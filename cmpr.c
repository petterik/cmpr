/*
Below we have shell functions which we used for building in the first days of the project.
These can gradually be cleaned up as we implement the functionality directly or in other ways.
We export these functions into a separate file so that they can be sourced at the shell.
e.g. `. cmpr/functions.sh` brings the current functions into scope.
Then we use `build` to do a build, and so on.

SH_FN_START

F=cmpr/cmpr.c

prepare_commit() {
  # the .? on the line below is so this doesn't end a C block comment...
  awk 'BEGIN {block=0} /^\/\*.?/ {block++; if (block==2) exit} {print}' < cmpr/cmpr.c | tail -n +3 > cmpr/README.md
  ls -lh cmpr/README.md
}

export_functions() {
  awk 'BEGIN{EMIT=0} /^SH_FN_END$/{EMIT=0} {if(EMIT)print} /^SH_FN_START$/{EMIT=1}' <"$F" >cmpr/functions.sh;
}

spanio_pre() {
  echo "Here's our library code:"
  echo
  echo '```c'
}

spanio_post() {
  echo '```'
  cat <<EOF
You are an expert code writer, writing complete and accurate code yourself, NOT telling someone else how to write the code.
Your code will be integrated into an existing system, so just reply with the code that was asked for.
Never silently consume errors, either don't test for something and assume that it's correct, or if you must test for it and it is not correct, then fail loudly and exit.
Answer briefly and focus on code.
Usually you will be asked to write a function that is described by a comment.
In this case just reply with a single function.
Feel free to write function declarations for any helper or library funtions that we need but don't already have.
Don't implement the later helper functions, as we will do that in a subsequent step.
EOF
}

spanio_prompt() {
  ( spanio_pre
    awk '/END LIBRARY CODE/ {exit} {print}' "$F"
    spanio_post
  ) | xclip -i -selection clipboard
}

code_blocks() {
  awk '/^\/\*$/ { start = NR; } /^}$/ { if (start != 0) { print start "," NR; start = 0; } }'
}

code_prompt() {
  ( echo "Here's our code so far. Reply with \"OK\"."
    echo '```'
    awk '/END LIBRARY CODE/,0' "$F" | tail -n +2
    echo '```'
  ) | xclip -i -selection clipboard
}

index() {
  ( cd cmpr
    ctags "$F"
    code_blocks < "$F" > blocks
  )
}

build() {
  gcc -o cmpr/dist/cmpr -g cmpr/cmpr.c -lm
}

clipboard_copy() {
  xclip -i -selection clipboard
}

clipboard_paste() {
  xclip -o -selection clipboard
}

SH_FN_END

just stashed here for now:

Reply only with code. Do not simplify, but include working code. If necessary, you may define a helper function which we will implement later, but do not include comments leaving out key parts. Your code must be complete as written, but calling functions that do not yet exist is allowed.

*/

/* import our library code */

#include "spanio.c"

#define flush_exit(n) flush(); exit(n)
/* #langtable #NaturalLanguageTabularProgramming #replywithok
Here we have a table of languages that we support.

(The columns are written as numbered items, for easier editing in text form.)

1. Supported Language:

C, Python, JavaScript

2. Filename extension:

C: .c
Python: .py
JavaScript: .js

3. Find blocks implementation:

C: find_blocks_by_type_c
Python: find_blocks_by_type_python
JavaScript: find_blocks_by_type_c

4. Find blocks description string:

C: Blocks start with a C-style block comment at the beginning of a line (no leading whitespace).
Python: Blocks start with a triple-quoted string, also at the beginning of a line.
JavaScript: Uses the same rules as C (block comment flush left starts a block).

5. Block comment end description:

C: Comment part ends with a C-style block comment that can end anywhere on a line.
Python: The triple-quote end also has to be at the start of a line.
JavaScript: Same as C.

6. Default block comment to prompt pattern:

C: "```c\n" $COMMENT$ "```\n\nWrite the code. Reply only with code. Do not include comments.\n"
Python: "```python\n" $COMMENT$ "```\n\nWrite the code. Reply only with code. Avoid code_interpreter.\n"
JavaScript: "```js\n" $COMMENT$ "```\n\nWrite the code. Reply only with code. Do not include comments.\n"
*/
/* #config_fields

We define CONFIG_FIELDS including all our known config settings, as they are used in several places (with X macros).

The known config settings are:

- projdir, location of the project directory that we are working with as a span
- revdir, a span containing a relative or absolute path to where we store our revisions
- tmpdir, another path for temp files that we use for editing blocks
- buildcmd, the command to do a build (e.g. by the "b" key)
- bootstrap, a command that creates an initial prompt (see README)
- cbcopy, the command to pipe data to the clipboard on the user's platform
- cbpaste, the same but for getting data from the clipboard
*/

#define CONFIG_FIELDS \
X(projdir) \
X(revdir) \
X(tmpdir) \
X(buildcmd) \
X(bootstrap) \
X(cbcopy) \
X(cbpaste)
/* #projfiles

A project can contain multiple files.

We have a projfile type which contains for each file:

- the path as a span
- the language, also a span
- the contents of the file, also a span

Here we have a typedef for the projfile, and we also call our generic macro to make a corresponding array type called projfiles, choosing 256 for the stack size.
*/

typedef struct {
    span path;
    span language;
    span contents;
} projfile;

MAKE_ARENA(projfile, projfiles, 256)
/* #ui_state

We define a struct, ui_state, which we can use to store any information about the UI that we can then pass around to various functions as a single entity.
This includes, so far:
- files, a projfiles array of files in the project
- the current language, a span, used by the file/language config handler functions
- the blocks, a spans
- the current_index into them
- a marked_index, which represent the "other end" of a selected range
- the search span which will contain "/" followed by some search if in search mode, otherwise will be empty()
- the previous search span, used for n/N
- the ex_command which similarly contains ":" if in ex command entry mode, otherwise empty()
- the config file path as a span
- terminal_rows and _cols which stores the terminal dimensions
- scrolled_lines, the number of physical lines that have been scrolled off the screen upwards

Additionally, we include a span for each of the config files, with an X macro inside the struct, using CONFIG_FIELDS defined above.

Below the ui_state struct/typedef, we declare a global ui_state* state, which will be initialized below in main().
*/

typedef struct ui_state {
    projfiles files;
    span current_language;
    spans blocks;
    int current_index;
    int marked_index;
    span search;
    span previous_search;
    span ex_command;
    span config_file_path;
    int terminal_rows;
    int terminal_cols;
    int scrolled_lines;
    #define X(name) span name;
    CONFIG_FIELDS
    #undef X
} ui_state;

ui_state* state;
/* #all_functions #replywithok
*/

void print_config();
void parse_config();
void cmpr_init();
void main_loop();
void handle_keystroke(char);
void keyboard_help();
void reset_stdin_to_terminal();
void edit_current_block();
void rewrite_current_block_with_llm();
void compile();
void replace_code_clipboard();
void toggle_visual();

void start_search();
void perform_search();
void finalize_search();

void search_forward();
void search_backward();

void start_ex();
void handle_ex_command();

void bootstrap();
void addfile(span);
void addlib(span);
void ex_help();

void settings_mode();
void ensure_conf_var(span*, span, span);
span block_comment_part(span block);
void print_block(int);
void print_comment(int);
void print_code(int);
int find_block(span);
int count_blocks();
span comment_to_prompt(span comment);
void send_to_clipboard(span prompt);
void print_physical_lines(span, int);
int print_matching_physical_lines(span, span);
void page_down();
void page_up();
span count_physical_lines(span, int*);
void print_multiple_partial_blocks(int,int);
void print_single_block_with_skipping(int,int);
int add_projfile(span); // helper for check_conf_vars
char* tmp_filename();
int launch_editor(char* filename);
void handle_edited_file(char* filename);
spans find_blocks(span);
spans find_blocks_by_type(span file, span language);
int file_for_block(span block);
void replace_block_code_part(span new_code);
void new_rev(char*, int);
void handle_args(int argc, char **argv);
void get_code();
void get_revs();
void check_conf_vars();
void rev_decr();

span pipe_cmd_cmp(span); // should probably be a library method

// stubs

void rev_decr() {}
/* #main

In main,

First we call init_spans, since all our i/o relies on it.

We call projfiles_arena_alloc near the top and and _free before we exit, with room for 1 << 14 (2^14) elements.

We call span_arena_alloc(), and at the end we call span_arena_free() just for clarity even though it doesn't matter anyway since we're exiting the process.
We allocate a spans arena of 1 << 20 or a binary million spans.

Above (in the ui state block), we have declared a global ui_state* called state, which allows us to not pass around the ui_state singleton all over our program.
After declaring our ui_state variable in main, which we initialize to {0}, we will set this global pointer to it.
We set config_file_path on the state to the default configuration file path which is ".cmpr/conf", i.e. always relative to the CWD.
We projfiles_alloc room for 1024 files on the state, and set the ".n" to zero, so we can use the _push() pattern.

We call a function handle_args to handle argc and argv.
This function will also read our config file (if any).
We call check_conf_vars() once after this; we also call it in the main loop but we need it before trying to get the code.

Next we call a function get_code().
This function either handles reading standard input or if we are in "project directory" mode then it reads the files indicated by our config file.
In either case, once this returns, inp is populated and any other initial code indexing work is done.

Next we call get_revs(), which handles reading and indexing all of our revisions (in revdir).

Then we call main_loop().

The main loop reads input in a loop and probably won't return, but just in case, we always call flush() before we return so that our buffered output from prt and friends will be flushed to stdout.
*/

int main(int argc, char** argv) {
    init_spans();
    projfiles_arena_alloc(1 << 14);
    span_arena_alloc(1 << 20);

    ui_state local_state = {0};
    state = &local_state;
    state->config_file_path = S(".cmpr/conf");
    state->files = projfiles_alloc(1024);
    state->files.n = 0;

    handle_args(argc, argv);
    check_conf_vars();
    get_code();
    main_loop();

    flush();
    projfiles_arena_free();
    span_arena_free();
    return 0;
}

/*
Debugging helper
*/

void print_config() {
  prt("config:\n");
}

/* #handle_args #argtable

In handle_args we handle any command-line arguments.

We present the supported arguments and flags in a tabular form (as with langtable previously).

Command syntax summary:

cmpr [--conf <filepath>] [--print-conf|--help|--init|--version] [(--print-block|--print-code|--print-comment) <index>] [find-block <search>] [--count-blocks]

Command argument and flag table:

1. Supported arguments and flags:

--conf <filepath>
--print-conf
--help
--init
--version
--print-block <index>
--print-comment <index>
--print-code <index>
--find-block <search>
--count-blocks

2. Behavior of arguments and flags:

conf:
  Use an alternate configuration file given by <filepath>; otherwise proceed normally.

print-conf:
  Print configuration settings and exit.

help:
  Print usage summary, help on command-line flags and exit.

init:
  Initialize .cmpr/ in the current directory.

version:
  Print the version number and build timestamp and exit.

print-{block,comment,code}:
  Print the revelant part (or whole) of the block given by the one-based index.

3. Implementation notes:

conf:
  update config_file_path on the state; we must do this before calling parse_config

print-conf:
  we print the configuration (print_config()) and exit; we must have called parse_config (and set an alt conf file if any) prior

help:
  we just prt a short usage summary, flush(), and exit(0)
  we define a macro flush_exit(n) to reduce typing
  we include argv[0] as usual and summarize everything we support.

init:
  we just call cmpr_init; we can't combine this with --conf, because --init creates the configuration file and we don't want to create configuration files in random places (the user can still move the file themselves and use --conf later if they are doing something exotic)

version:
  we prt "Version: $VERSION$" here; the dollar-delimited variable-looking thing is replaced by a build step

--help, --init, --version:
  these three flags all are "action args"; if one is provided, any other flags will have no effect
  if more than one is given, any one of them may take effect (we don't care which), but not more than one

print-{code,comment,block}, count-blocks, find-block:
  all of these require the code be loaded, which normally happens after we are called
  so if any of these flags are used we call get_code() first, then we call the appropriate function, then flush and exit successfully
  we always use one-based indexes for anything user-visible, so we must add or subtract one when calling our functions (find_block, print_block, print_comment, print_code)

4. Help strings:

conf:
  Use alternate configuration file <filepath>.

print-conf:
  Print the current configuration settings.

init:
  Initialize a new directory for use with cmpr.

help:
  Display this help message.

version:
  Display the version number / build string.

print-block, -comment, -code:
  Print a complete block (or comment or code part) given by index.

find-block:
  Print index of first block matching search string by full-text search, or -1.

count-blocks:
  Print number of blocks in project.

--- end argtable

Below we implement void handle_args(int argc, char **argv);

Here are some random further implementation notes on that:

There are things that have to be handled in specific orderings.
Our basic technique here is to set indicators (0- or 1-valued ints) in our arg-handling loop.
These are used directly in if statements, like `if (ind_print_block) { ...`.
Below the loop we then handle the necessaries in the correct order.
We use "int ind_*" for these variables so they don't conflict with functions or anything else we already have.
We also have an int "action_arg" which tracks whether one of the action flags has been set, char pointers for string arguments like conf filepath or find-block search, and a block index which is shared by print-{block,comment,code}.

None of these flags can be combined: print-block, print-comment, print-code, find-block, count-blocks.
If more than one is set, we print an error message and exit.
If any of these are set then we exit successfully, but if none of them is, then we will return from this function and enter our main loop.

Because --init is used to set up the config file, it cannot be combined with --conf, if it is, we also print an error and exit.
Also, if we are doing an --init, we need to not try to parse the config file, because that will definitely fail.

If "--conf <alternate-config-file>" is passed, we update config_file_path on the state.
Once we know the conf file to read from, we call parse_config before we do anything else.

If "--print-conf" is passed in, we print our configuration settings and exit.
This is only OK to do once we have already called parse_config, so the configuration settings have already been read in from the file.

This function will always call parse_config, always before printing the config if "--print-conf" is used, and always after updating the config file if "--conf" is used.
In particular, even if no alternate conf file was set, we still need to read the default conf file.
*/

void handle_args(int argc, char **argv) {
    int ind_conf = 0, ind_print_conf = 0, ind_help = 0, ind_init = 0, ind_version = 0;
    int ind_print_block = 0, ind_print_comment = 0, ind_print_code = 0, ind_find_block = 0, ind_count_blocks = 0;
    int action_arg = 0;
    char *conf_filepath = NULL, *find_block_search = NULL;
    int block_index = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--conf") == 0 && i + 1 < argc) {
            conf_filepath = argv[++i];
            ind_conf = 1;
        } else if (strcmp(argv[i], "--print-conf") == 0) {
            ind_print_conf = 1;
            action_arg = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            ind_help = 1;
            action_arg = 1;
        } else if (strcmp(argv[i], "--init") == 0) {
            ind_init = 1;
            action_arg = 1;
        } else if (strcmp(argv[i], "--version") == 0) {
            ind_version = 1;
            action_arg = 1;
        } else if (strcmp(argv[i], "--print-block") == 0 && i + 1 < argc) {
            block_index = atoi(argv[++i]) - 1;
            ind_print_block = 1;
        } else if (strcmp(argv[i], "--print-comment") == 0 && i + 1 < argc) {
            block_index = atoi(argv[++i]) - 1;
            ind_print_comment = 1;
        } else if (strcmp(argv[i], "--print-code") == 0 && i + 1 < argc) {
            block_index = atoi(argv[++i]) - 1;
            ind_print_code = 1;
        } else if (strcmp(argv[i], "--find-block") == 0 && i + 1 < argc) {
            find_block_search = argv[++i];
            ind_find_block = 1;
        } else if (strcmp(argv[i], "--count-blocks") == 0) {
            ind_count_blocks = 1;
        }
    }

    if (action_arg) {
        if (ind_conf && ind_init) {
            prt("Error: --conf and --init cannot be combined.\n");
            flush_exit(1);
        }
        if (ind_help) {
            prt("Usage: cmpr [--conf <filepath>] [--print-conf|--help|--init|--version] [(--print-block|--print-code|--print-comment) <index>] [find-block <search>] [--count-blocks]\n");
            flush_exit(0);
        }
        if (ind_version) {
            prt("Version: $VERSION$\n");
            flush_exit(0);
        }
        if (ind_init) {
            cmpr_init();
            flush_exit(0);
        }
    } else {
        if (ind_conf) {
            state->config_file_path = S(conf_filepath);
        }
        parse_config();
        if (ind_print_conf) {
            print_config();
            flush_exit(0);
        }
        if (ind_print_block + ind_print_comment + ind_print_code + ind_find_block + ind_count_blocks > 1) {
            prt("Error: --print-block, --print-comment, --print-code, --find-block, and --count-blocks cannot be combined.\n");
            flush_exit(1);
        }
        get_code();
        if (ind_print_block) {
            print_block(block_index);
            flush_exit(0);
        } else if (ind_print_comment) {
            print_comment(block_index);
            flush_exit(0);
        } else if (ind_print_code) {
            print_code(block_index);
            flush_exit(0);
        } else if (ind_find_block) {
            int index = find_block(S(find_block_search));
            prt("%d\n", index + 1);
            flush_exit(0);
        } else if (ind_count_blocks) {
            int count = count_blocks();
            prt("%d\n", count);
            flush_exit(0);
        }
    }
}

/*
In clear_display() we clear the terminal by printing some escape codes (with prt and flush as usual).
*/


void clear_display() {
    prt("\033[2J\033[H"); // Escape codes to clear the screen and move the cursor to the top-left corner
    flush();
}

/* #block_sanity_check
 
In block_sanity_check, we are given a file span and blocks spans, and we check certain invariants.

First we handle the special case where the file is empty, in this case there must be exactly one empty block and nothing else, and we early exit.
In all other cases, blocks are never empty.

Then we ensure with a simple loop, that all the blocks returned together tile the file, and that none are empty.
This means:
The first block begins where our input span begins.
The last block ends where our input ends.
If either of these conditions fails, the sanity test fails.
For every other block, .buf is equal to the .end of the previous.
No block is empty (i.e. len() > 0 in every case).
If this sanity check fails, we complain and crash as usual (prt, flush, exit).
*/

void block_sanity_check(span file, spans blocks) {
    if (empty(file)) {
        if (blocks.n != 1 || !empty(blocks.s[0])) {
            prt("Error: Empty file must have exactly one empty block.\n");
            flush();
            exit(EXIT_FAILURE);
        }
        return; // Early exit for empty file
    }

    // Check if the first block begins where the input span begins
    if (blocks.s[0].buf != file.buf) {
        prt("Error: The first block does not start where input begins.\n");
        flush();
        exit(EXIT_FAILURE);
    }

    // Check if the last block ends where the input ends
    if (blocks.s[blocks.n - 1].end != file.end) {
        prt("Error: The last block does not end where input ends.\n");
        flush();
        exit(EXIT_FAILURE);
    }

    // Ensure all blocks tile the file and none are empty
    for (int i = 1; i < blocks.n; ++i) {
        if (blocks.s[i].buf != blocks.s[i - 1].end || empty(blocks.s[i])) {
            prt("Error: Blocks do not properly tile the file or a block is empty.\n");
            flush();
            exit(EXIT_FAILURE);
        }
    }
}

/*
In find_all_blocks, we find the blocks in each file.

We are going to need to know how many blocks there are in all the files, and to store all of the blocks so that we can allocate a single spans to hold all of them at the end.
Therefore we first allocate an array of the `spans` type, one per file.

Then for each of the projfiles, we call find_blocks_by_type() to find the blocks in that file (by language, actually, the second argument to this function), and stash those temporarily on our array.
We also call block_sanity_check on the returned blocks for each file.

Once we have all of the blocks for each of the files, we then can construct a new spans (using spans_alloc) of the right size, and we can copy all the blocks into this, which we store on the state.
(We don't store the blocks per file anywhere, since we can always determine which file a block belongs to by comparing the block's span with the contents span on the projfile.)
*/

void find_all_blocks() {
    spans* file_blocks = (spans*)malloc(state->files.n * sizeof(spans));
    if (!file_blocks) {
        perror("Failed to allocate memory for file_blocks");
        exit(EXIT_FAILURE);
    }

    size_t total_blocks = 0;
    for (int i = 0; i < state->files.n; ++i) {
        file_blocks[i] = find_blocks_by_type(state->files.a[i].contents, state->files.a[i].language);
        block_sanity_check(state->files.a[i].contents, file_blocks[i]);
        total_blocks += file_blocks[i].n;
    }

    spans all_blocks = spans_alloc(total_blocks);
    size_t index = 0;
    for (int i = 0; i < state->files.n; ++i) {
        for (int j = 0; j < file_blocks[i].n; ++j) {
            all_blocks.s[index++] = file_blocks[i].s[j];
        }
    }

    state->blocks = all_blocks;
    free(file_blocks);
}
/* #get_code

In get_code, we get the code into the input buffer.

For each of the projfiles:

- we read this file into inp by read_file_S_into_span with inp_compl() as the span argument, always advancing inp as usual in this pattern so we don't overwrite the contents
- we store the contents on the projfile

Then we call find_all_blocks(), which sets up the blocks according to each file's contents and language.
*/

void get_code() {
    for (int i = 0; i < state->files.n; i++) {
        state->files.a[i].contents = read_file_S_into_span(state->files.a[i].path, inp_compl());
        inp.end = state->files.a[i].contents.end; // Advance inp to not overwrite contents
    }

    find_all_blocks();
}

/* #get_revs
*/

void get_revs() {}
/*
In find_blocks_by_type_python, we get a span containing a file.

We write two loops.
In the first one we count the blocks, then we spans_alloc our return value with the correct number, and in the second loop we assign the spans.

For a Python file, a block starts with triple double-quote at the beginning of a line.
It contains another triple double-quote at the beginning of a line somewhere in the middle of the block, ending the comment part, which we must skip over.
It ends where the next block starts (true for all blocks regardless of language).

There are some special cases:

If the file is empty, we return a single empty block.

If a file does not begin with the triple quote, then the first block will just be from the beginning of the file to the second triple quote at the beginning of a line.

The last block always goes to the end of the file.

Before the first loop we copy our argument into a new span `copy` so that we can consume it while counting, but then still have access to the original full span for the second loop.

The block-finding loop:
- While the remaining input is not empty, we get the next line. (Note that next_line will return everything left if there is no newline).
- If this line starts with the pattern, or if it is the first line in the file, then it begins a block (regardless of whether the line is empty), otherwise we simply do nothing and continue the loop. (If it starts with the pattern and we have seen an odd number of them so far, then we skip it because the pattern appears two times in each Python block.)
- If we are counting blocks, we increment our counter, otherwise we assign .buf of a span for this block, and if we had a previous block, we assign .end of that block to the same offset.
- When we reach the end of the input we will assign .end of the last block to the end of the input.

(To determine if we are at the start of the input, we can compare .buf of the line with that of the input.)

At the end of the function, we ensure with a simple loop, that all the blocks returned together tile the file, and that none are empty.
This means:
The first block begins where our input span begins.
The last block ends where our input ends.
If either of these conditions fails, the sanity test fails.
For every other block, .buf is equal to the .end of the previous.
If this sanity check fails, we complain and crash as usual (prt, flush, exit).
*/

spans find_blocks_by_type_python(span file) {
    int block_count = 0;
    span copy = file;
    span line;
    int quote_count = 0;

    // First loop: count blocks
    while (!empty(copy)) {
        line = next_line(&copy);
        if (starts_with(line, S("\"\"\"")) || copy.buf == file.buf) {
            quote_count++;
            // Skip the ending quote of a block
            if (quote_count % 2 == 0) continue;
            block_count++;
        }
    }

    spans blocks = spans_alloc(block_count);
    copy = file; // Reset copy for second loop
    span* previous_block = NULL;
    int index = 0;
    quote_count = 0;

    // Second loop: assign spans
    while (!empty(copy)) {
        line = next_line(&copy);
        if (starts_with(line, S("\"\"\"")) || copy.buf == file.buf) {
            quote_count++;
            if (quote_count % 2 == 0) continue;
            if (previous_block != NULL) {
                previous_block->end = line.buf;
            }
            blocks.s[index].buf = line.buf;
            previous_block = &blocks.s[index++];
        }
    }
    if (previous_block != NULL) {
        previous_block->end = file.end;
    }

    // Sanity check
    for (int i = 0; i < blocks.n; ++i) {
        if (i == 0 && blocks.s[i].buf != file.buf) {
            prt("Error: First block does not start where input begins.\n");
            flush();
            exit(EXIT_FAILURE);
        }
        if (i == blocks.n - 1 && blocks.s[i].end != file.end) {
            prt("Error: Last block does not end where input ends.\n");
            flush();
            exit(EXIT_FAILURE);
        }
        if (i > 0 && blocks.s[i].buf != blocks.s[i - 1].end) {
            prt("Error: Block start does not match previous block end.\n");
            flush();
            exit(EXIT_FAILURE);
        }
    }

    return blocks;
}

/*
In find_blocks_by_type_c, we get a span containing a file.

We write two loops.
In the first one we count the blocks, then we spans_alloc our return value with the correct number, and in the second loop we assign the spans.

For a C file, a block starts with the slash-star pattern at the beginning of a line.
It ends where the next block starts.

There are some special cases:

If the file is empty, we return a single empty block.
We handle this as a special case, since it doesn't really work with our two-loop approach.
We also care about where the blocks are, even if they are empty, so in this case we ensure that the empty block we return is the same as the empty span of the file itself (i.e. .buf and .end are equal to each other, and to those of the file: we can NOT use nullspan() here).

If a file does not begin with a block comment, then the first block will just be from the beginning of the file to the first block comment.

The last block always goes to the end of the file.

Before the first loop we copy our argument into a new span `copy` so that we can consume it while counting, but then still have access to the original full span for the second loop.

The block-finding loop:
- While the remaining input is not empty, we get the next line. (Note that next_line will return everything left if there is no newline).
- If this line starts with the pattern, or if it is the first line in the file, then it begins a block (regardless of whether the line is empty), otherwise we simply do nothing and continue the loop.
- If we are counting blocks, we increment our counter, otherwise we assign .buf of a span for this block, and if we had a previous block, we assign .end of that block to the same offset.
- When we reach the end of the input we will assign .end of the last block to the end of the input.

(To determine if we are at the start of the input, we can compare .buf of the line with that of the input.)
*/

spans find_blocks_by_type_c(span file) {
    if (empty(file)) {
        // Handle special case for empty file
        spans single_empty_block = spans_alloc(1);
        single_empty_block.s[0].buf = file.buf;
        single_empty_block.s[0].end = file.end;
        return single_empty_block;
    }

    int block_count = 0;
    span copy = file;
    int is_first_line = 1;

    // First loop: count blocks
    while (!empty(copy)) {
        span line = next_line(&copy);
        if (is_first_line || starts_with(line, S("/*"))) {
            block_count++;
            is_first_line = 0;
        }
    }

    spans blocks = spans_alloc(block_count);
    copy = file; // Reset copy for second loop
    span* previous_block = NULL;
    int index = 0;
    is_first_line = 1;

    // Second loop: assign spans
    while (!empty(copy)) {
        span line = next_line(&copy);
        if (is_first_line || starts_with(line, S("/*"))) {
            if (previous_block != NULL) {
                previous_block->end = line.buf;
            }
            blocks.s[index].buf = line.buf;
            previous_block = &blocks.s[index++];
            is_first_line = 0;
        }
    }
    if (previous_block != NULL) {
        previous_block->end = file.end;
    }

    return blocks;
}
/*
In `spans find_blocks_by_type(span,span)`, we take a span (a file's contents) and a language, which is any of the supported languages.

We dispatch to another function that handles that language appropriately, and return the resulting spans.

We dispatch according to the "find blocks by language" implementation given by #langtable col. 3., above.

If the language is not known, we prt, flush, exit as per usual.
*/

spans find_blocks_by_type(span file, span language) {
    if (span_eq(language, S("C"))) {
        return find_blocks_by_type_c(file);
    } else if (span_eq(language, S("Python"))) {
        return find_blocks_by_type_python(file);
    } else if (span_eq(language, S("JavaScript"))) {
        return find_blocks_by_type_c(file); // JavaScript uses the same rule as C for finding blocks.
    } else {
        prt("Error: Unsupported language '%.*s'.\n", len(language), language.buf);
        flush();
        exit(1);
    }
}
/*
Function to read a single character without echoing it to the terminal.
*/

char getch(void) {
  char buf = 0;
  struct termios old = {0}, new = {0};
  if (tcgetattr(0, &old) < 0) perror("tcgetattr()");
  new = old;
  new.c_lflag &= ~(ICANON | ECHO);
  new.c_cc[VMIN] = 1;  // Set to block until at least one character is read
  new.c_cc[VTIME] = 0; // Disable the timeout

  if (tcsetattr(0, TCSANOW, &new) < 0) perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0) perror("read()");
  if (tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");

  return buf;
}

/*
In reset_stdin_to_terminal, we use the technique of opening /dev/tty for direct keyboard input with dup2 to essentially "reset" stdin to the terminal, even if it was originally redirected from a file.
This approach allows us to switch back to reading from the terminal without having to specifically manage a separate file descriptor for /dev/tty in the rest of the program.
First we open the terminal device, dup2 to stdin (fd 0), and finally close the tty fd as it's no longer needed.
*/

void reset_stdin_to_terminal() {
  int tty_fd = open("/dev/tty", O_RDONLY);
  if (tty_fd < 0) {
    perror("Failed to open /dev/tty");
    exit(EXIT_FAILURE);
  }

  if (dup2(tty_fd, STDIN_FILENO) < 0) {
    perror("Failed to duplicate /dev/tty to stdin");
    close(tty_fd);
    exit(EXIT_FAILURE);
  }

  close(tty_fd);
}

/*
In main_loop, we initialize:

- the current index to 0,
- the marked index to -1 indicating that we are not in visual selection mode.

In our loop, we call check_conf_vars(), which just handles the case where some essential configuration variables aren't set.

Next we clear the terminal, then print the current block or blocks.
Then we will wait for a single keystroke of keyboard input using our getch() above.
Just before we call this function, we also call flush(), which just prevents us having to call it an a lot of other places all over the code.

We also define a helper function print_current_blocks; we include just the declaration for that function below.
(Reminder: we never write `const` in C.)

Once we have a keystroke, we will call another function, handle_keystroke, which takes the char that was entered.
*/

void check_conf_vars();
void print_current_blocks();
void handle_keystroke(char keystroke);

void main_loop() {
    state->current_index = 0;
    state->marked_index = -1;

    while (1) {
        check_conf_vars(); // Ensure essential configuration variables are set

        clear_display(); // Clear the terminal screen
        print_current_blocks(); // Print the current block or blocks
        flush(); // Flush the output before waiting for input

        char input = getch(); // Wait for a single keystroke
        handle_keystroke(input); // Handle the input keystroke
    }
}
/* span count_physical_lines(span, int*)

In this function we are given a span and a (pointer to a) maximum number of physical lines to print or count.

The span may be of arbitrary length, which is why we do not simply count the lines in the span; we do not want unbounded runtime.

Definitions:
A logical line is an actual line terminated by newline or by the end of the span.
A physical line is a row of terminal output, which is reached either when terminal_cols characters of output reaches the end of the line, causing wrapping, or when a literal newline is printed (or both).
The physical line is defined such that after printing a physical line (starting at the beginning of a terminal row), the next character printed to the terminal will appear on the subsequent terminal row.

We begin counting physical lines off of the input span according to this definition.
Specifically, we count off either terminal_cols chars, followed by a newline (which will have no effect if it comes after exactly that many chars, i.e. after counting off terminal_cols chars, we need to check if the *next* char is a newline, because if it is we should skip over it), or we count off a shorter line followed by an actual newline.

We decrement the int of remaining lines each time we have counted off a physical line.
Our return value is the span of physical lines that we have counted off.
The int will go to zero if the span passed in is long enough, otherwise the span returned will contain the entire input and the int may remain positive.

(The intended typical use of this function is to provide a span and a desired number of physical lines of terminal space to occupy, then to print the returned span, and perhaps to prepare a suffix of the original span to keep printing, and perhaps to use the int to track remaining lines of terminal output to fill or similar.)

(Note: I don't think the below is correct when a logical line has exactly terminal_cols chars in it, however I don't want to iterate further or test right now.)
*/

span count_physical_lines(span input, int *max_physical_lines) {
    span result = input;
    int line_count = 0;
    int chars_in_line = 0;

    while (!empty(input) && line_count < *max_physical_lines) {
        if (*input.buf == '\n' || chars_in_line == state->terminal_cols) {
            line_count++;
            if (*input.buf == '\n') input.buf++;
            chars_in_line = 0;
        } else {
            input.buf++;
            chars_in_line++;
        }
    }

    *max_physical_lines -= line_count;
    result.end = input.buf;
    return result;
}
/* page_down() and page_up()

Here we implement dual functions that handle pagination within the current block.

We define content_rows as the number of terminal_rows minus two, since we always have a header line and a ruler line reserved at the top and bottom of the screen resp.

We simply increment or decrement state->scrolled_lines by content_rows, except that we always want to fill the screen.
For example, if a block has 23 physical lines and the terminal has 24 rows, then our content area is 22 rows, and when we paginate downwards we will show the last 22 lines of content (skipping only the first line).
Redrawing is handled in the main loop, so all we do here is update scrolled_lines as needed, returning void.

If we are already scrolled to the bottom, scrolling down will have no effect (similarly if scrolled_lines = 0 for scrolling up).

We have a helper function (count_physical_lines) that counts physical lines up to a maximum.
It updates the int passed to it by reference to indicate the remaining number of lines (<= the maximum before the call) that have not been printed (will only be non-zero if the block was short of content).
In page_down, we first call this with scrolled_lines and get a span back which is the part that is already "scrolled off" the top of the screen as the return value.

We make a copy of the block (blocks indexed by current_block, both on state).
We update .buf of this copy to the .end of the scrolled-off part, thus getting the part of the block currently visible on the screen as well as anything "below" the screen.

We then call the helper function again on this remainder content with terminal_rows as the number, to get the number of lines occupied by the currently displayed content, up to a full screen's worth.
If there is less than one full screen's worth currently displayed, then we reduce scrolled_lines by the remaining number, so that the screen becomes full.

(Note that as count_physical_lines decrements the remaining physical lines to print while it is counting off lines, we need to subtract to get the actual number of physical lines of content that would be printed.)

Otherwise, we increase scrolled_lines by a full screenfull, and then we call the helper function a third time.
Now, again, we can check if it will print a full screenfull, and if not, we can again reduce scrolled_lines such that the result will be a full screen of content ending with the last physical line of the block.

The page_up function is a bit simpler, as we can always unconditionally scroll up by a full page of lines, so we simply decrease scrolled_lines by a screenful (with a minimum of zero, obviously).
*/

void page_down() {
    int lines_to_skip = state->scrolled_lines;
    int content_rows = state->terminal_rows - 2;
    span block_copy = state->blocks.s[state->current_index];
    span scrolled_off = count_physical_lines(block_copy, &lines_to_skip);

    block_copy.buf = scrolled_off.end;
    int lines_for_screen = content_rows;
    span remainder = count_physical_lines(block_copy, &lines_for_screen);

    if (lines_for_screen > 0) {
        state->scrolled_lines -= lines_for_screen;
    } else {
        state->scrolled_lines += content_rows;
        lines_to_skip = state->scrolled_lines;
        block_copy = state->blocks.s[state->current_index];
        scrolled_off = count_physical_lines(block_copy, &lines_to_skip);

        /* *** manual fixup *** */
        block_copy.buf = scrolled_off.end;
        lines_for_screen = content_rows;
        count_physical_lines(block_copy, &lines_for_screen);

        if (lines_for_screen > 0) {
            state->scrolled_lines -= lines_for_screen;
        }
    }
}

void page_up() {
    state->scrolled_lines -= (state->terminal_rows - 2);
    if (state->scrolled_lines < 0) {
        state->scrolled_lines = 0;
    }
}
/* Helper function to print the first n lines of a block. */

void print_first_n_lines(span block, int n) {
  int line_count = 0;
  span copy = block; // Make a copy to avoid modifying the original span
  while (!empty(copy) && line_count < n) {
    span line = next_line(&copy);
    wrs(line); // Write the line to the output span
    terpri();  // Append a newline character
    line_count++;
  }
}

/*
In toggle_visual, we test if we are in visual selection mode.
If the marked index is not -1 then we are in visual mode, and we leave the mode (by setting it to -1).
Otherwise we enter it by setting marked index to be the current index.
In either case we then reflect the new state in the display by calling print_current_blocks()
*/

void toggle_visual() {
    if (state->marked_index != -1) {
        // Leave visual mode
        state->marked_index = -1;
    } else {
        // Enter visual mode
        state->marked_index = state->current_index;
    }
    // Reflect the new state in the display
    print_current_blocks();
}

/*
In print_current_blocks, we print either the current block, if we are in normal mode, or the set of selected blocks if we are in visual mode.

Before this, we write a helper function that gets the screen dimensions (rows and cols) from the terminal.
This function will update the state directly.

If marked_index != -1 and != current_index, then we have a "visual" selected range of more than 1 block.

First we determine which of marked_index and current_index is lower and make that our block range start, and then one past the other is our block range end (considered as an exclusive endpoint).

The difference between the two is then the number of selected blocks.
At this point we know how many blocks we are displaying.
Finally we pass state, inclusive start, and exclusive end of range to another function handles rendering.

Helper functions:

- render_block_range(int,int) -- also supports rendering a single block (if range includes only one block).
*/

void get_screen_dimensions() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  state->terminal_rows = w.ws_row;
  state->terminal_cols = w.ws_col;
}

void render_block_range(int, int);

void print_current_blocks() {
  get_screen_dimensions();

  if (state->marked_index != -1 && state->marked_index != state->current_index) {
    int start = state->current_index < state->marked_index ? state->current_index : state->marked_index;
    int end = state->current_index > state->marked_index ? state->current_index + 1 : state->marked_index + 1;

    render_block_range(start, end);
  } else {
    // Normal mode or visual mode with only one block selected.
    render_block_range(state->current_index, state->current_index + 1);
  }
}

/*
In render_block_range, we get the state and a range of blocks (first endpoint inclusive, second exclusive).

The state tells us (with terminal_rows and _cols) what the dimensions of the terminal are.

First we calculate the length in lines of each of the blocks that we have.

More specifically, we calculate a mapping from logical lines to physical lines.
First we identify the logical lines by finding the newlines in the block, and we store the offset of each (as an offset from .buf on the block).
We find out how many "physical" lines (i.e. terminal rows) each logical line uses by taking the ceil of quotient of line length by terminal cols.

Then we decide, based on the number of blocks, the layout information, and the terminal dimensions, how many lines from each we can fit on the screen.
For this we declare a separate helper function which we will write later.
We call this function (with the ui state and our logical-physical mapping) and then we free our own resources.

We include a forward declaration for our helper functions.
*/

//* *** manual fixup ***/

void render_block_range(int start, int end) {

    if (end - start == 1) {
      print_single_block_with_skipping(start, state->scrolled_lines);
    } else {
      print_multiple_partial_blocks(start, end);
    }
}

/*
In print_multiple_partial_blocks, we get a state and we should print as much as we can of the blocks.
Currently, we just print the number of blocks that there are.
*/

// Placeholder for print_multiple_partial_blocks, assuming it's defined elsewhere
void print_multiple_partial_blocks(int start_block, int end_block) {
  prt("%d blocks (printing multiple blocks coming soon!)\n", end_block - start_block);
}

/* #printsingle

In printsingle, we get the ui state and a limit of lines to print.
We call printsinglehelper(), which takes:

- the ui state,
- a max number of lines to print phy_row_lim,
- a number of lines to skip phy_row_off,
- an indicator (1) to print or (0) to count the lines,
- an indicator (1) to split or (0) favor the 
- the address of a decrement counter for lines remaining (non-negative),
- the address of a span of unprinted input.

We call an offset (the number of rows to skip) perfect if it perfectly fills the limit, which means that a call to the helper sends the lines remaining to zero and the len of unprinted input to zero on the same call.
In the helper, if 

The limit must always be less than or equal to state.terminal_rows, otherwise it will be ignored, since we are a paging output mode.
The block to print from is given by the second argument, always a zero-based integer < state->blocks.n.

If we are in splitting mode, then we first need to determine whether we have more than

Specifically:

- block_row_limit sets a hard limit of vertical lines to be occupied by the printing of the block
- block_row_offset skips a number of vertical lines of the block before it begins printing at the beginning of a (physical) line.

The caller sets block_row_limit to a desired limit before the call to this function.
If block_row_limit is zero, we return 0 (the number of lines we have successfully printed, as always).
If it is negative we prt() a short complaint with newline and exit_failure().

We copy the block into a local span for convenience.

In our first loop, we count physical lines until we know that we have skipped block_row_offset lines.
We perform this by calling a function, span decrement_physical_skip(int*,int,span), described later.
The arguments to this function are, in order, the address of block_row_limit, terminal_cols, and our copy of the block, and we assign the return value back to our copy of the block as the block copy always contains data not yet printed (or in this loop, skipped).
If the len() of the remaining block contents goes to zero in this loop, then we return zero.

We then skip more physical lines until we know whether we can overfill block_row_limit.

If the number of physical lines printed equals the block_row_limit, and the block_row_limit was positive, we can say the offset is "perfect".

*/

/* #handle_keystroke

In handle_keystroke, we support the following single-char inputs:

- j/k Go up or down one block. If we are at the first or last block, these are no-ops.
- g/G Go to the first or last block resp.
- e, Edit the current block in $EDITOR (or vi by default)
- r, Request an LLM rewrite the code part of the block based on the comment part; silently updates clipboard
- R, kin to "r", which reads current clipboard contents back into the block, replacing the code part
- u, undo
- space/b, paginate down or ("back") up within a block
- B, do a build by running the build command you provide
- v, sets the marked point to the current index, switching to "visual" selection mode, or leaves visual mode if in it (currently commented out)
- /, switches to search mode
- :, switches to ex command line
- n/N, repeat search in forward/backward direction
- S, (likely to change) goes into settings mode
- ?, display brief help about the keyboard shortcuts available
- q, exits (with prt("goodbye\n"); flush(); exit(0))

We call terpri() on the first line of this function (just to separate output from any handler function from the ruler line).

Implemented inline: j,k,g,G,q
All others call helper functions already declared, e.g.:
B -> compile()
u -> rev_decr()
R -> replace_code_clipboard()
? -> keyboard_help()

Any of j,k,g,G that change the current_index must also reset pagination (scrolled_lines -> 0).
*/

void handle_keystroke(char input) {
    terpri();
    switch (input) {
        case 'j':
            if (state->current_index < count_blocks() - 1) {
                state->current_index++;
                state->scrolled_lines = 0;
            }
            break;
        case 'k':
            if (state->current_index > 0) {
                state->current_index--;
                state->scrolled_lines = 0;
            }
            break;
        case 'g':
            state->current_index = 0;
            state->scrolled_lines = 0;
            break;
        case 'G':
            state->current_index = count_blocks() - 1;
            state->scrolled_lines = 0;
            break;
        case 'e':
            edit_current_block();
            break;
        case 'r':
            rewrite_current_block_with_llm();
            break;
        case 'R':
            replace_code_clipboard();
            break;
        case 'u':
            rev_decr();
            break;
        case ' ':
            page_down();
            break;
        case 'b':
            page_up();
            break;
        case 'B':
            compile();
            break;
        case 'v':
            //toggle_visual();
            break;
        case '/':
            start_search();
            break;
        case ':':
            start_ex();
            break;
        case 'n':
            search_forward();
            break;
        case 'N':
            search_backward();
            break;
        case 'S':
            settings_mode();
            break;
        case '?':
            keyboard_help();
            break;
        case 'q':
            prt("goodbye\n");
            flush();
            exit(0);
            break;
        default:
            break;
    }
}

/* #keyboard_help

To get the help text we basically copy the lines in #handle_keystrokes above, except formatted nicely for terminal output.
We split out j/k and g/G onto their own lines though.
Include all relevant details about usage that might be non-obvious, e.g.:
- r "puts a prompt on the clipboard to rewrite the code part based on comment part"
Include mnemonic hints where given (e.g. "back" for b).

We call clear_display first, and flush, getch after, so the user has time to read the help (we prompt them about this).
*/

void keyboard_help() {
    clear_display();
    prt("Keyboard shortcuts:\n");
    prt("j    - Go down one block\n");
    prt("k    - Go up one block\n");
    prt("g    - Go to the first block\n");
    prt("G    - Go to the last block\n");
    prt("e    - Edit the current block in $EDITOR\n");
    prt("r    - Rewrite code part based on comment part; clipboard updated\n");
    prt("R    - Replace code part with clipboard contents\n");
    prt("u    - Undo\n");
    prt("space- Paginate down within a block\n");
    prt("b    - Paginate up (\"back\") within a block\n");
    prt("B    - Build project with provided command\n");
    prt("v    - Toggle visual selection mode\n");
    prt("/    - Enter search mode\n");
    prt(":    - Enter ex command line\n");
    prt("n    - Repeat search forward\n");
    prt("N    - Repeat search backward\n");
    prt("S    - Enter settings mode\n");
    prt("?    - Display this help\n");
    prt("q    - Quit\n");
    prt("\nPress any key to return...\n");
    flush();
    getch();
}

/* #start_search

To support search mode, we have a static buffer of length 256 which can be used to search and which the user types into when in search mode.

In start_search(), to indicate that we are in search mode, we set the search on the ui state to be equal to a span which points at this static buffer.
And we set the span to contain the slash "/" that was typed.
We can do this using S(), but we prefer to just directly construct the span.
When we first go into search mode, we point the span at the start of the static buffer, we make it length 1 and the static buffer always starts with slash.

Then we enter our own loop where we call getch and handle basic line editing.
On any backspace character we will simply shorten the span (.end--) and on any other input at all we will extend it.
However, if we've deleted the initial slash, that means the user doesn't want to be in search mode any more.
Therefore, if the search span on the ui state has zero length, this means search mode is off.
So after every backspace, if the search span length goes to zero then we call print_current_blocks() to update the display and then return.

Every time the contents of the line changes, or when we first enter search mode, we will call another function, perform_search.
This will implement search and also displays the search results and indicates that we're in search mode to the user.
If we hit enter, we call another helper to finish the search successfully.

helper functions:

- perform_search()
- finalize_search()

OF COURSE, we use getch() which we carefully defined above, NEVER getchar().

We write declarations for the helper functions (which we define below).
*/

void start_search() {
    static char search_buffer[256] = {"/"}; // Static buffer for search, pre-initialized with "/"
    state->search = (span){.buf = search_buffer, .end = search_buffer + 1}; // Initialize search span to contain just "/"

    perform_search(); // Perform initial search display/update

    char input;
    while ((input = getch()) != '\n') { // Continue until Enter is pressed
        if (input == '\b' || input == 127) { // Handle backspace (ASCII DEL on some systems)
            if (state->search.buf < state->search.end) {
                state->search.end--; // Shorten the span
                if (state->search.end == state->search.buf) {
                    // If we've deleted the initial "/", exit search mode
                    print_current_blocks();
                    return;
                }
            }
        } else if ((state->search.end - state->search.buf) < sizeof(search_buffer) - 1) {
            // Ensure there's space for more characters
            *state->search.end++ = input; // Extend the span
        }

        perform_search(); // Update search results after each modification
    }

    finalize_search(); // Finalize search on Enter
}

/* #start_ex

To support ex commands, we have a static buffer (declared in start_ex() below) of length 256 which is used to hold the ex command while the user is typing it.
The first byte of this buffer is always ":".

In start_ex, to indicate that we are going into ex mode, we set ex_command on the state to a span that points at this static buffer, and includes only the ":", since this is what was just typed to get into ex command mode.
We can do this using S(), but we prefer to just directly construct the span.
Then we print the line at the bottom starting with ":" (which is currently the only thing on it) as described below, to indicate to the user that we're now in ex command entry mode.

However, if we've deleted the initial colon, that means the user doesn't want to be in ex command mode any more.
If the state element is empty it always means we are not in the mode.
If this happens we call print_current_blocks() to refresh the display, and then return.

As with search mode, we implement our own input handling with a getch() loop, handling both kinds of backspace, and if enter is hit, we will call handle_ex_command().
On any backspace character we will simply shorten the span (.end--) and on any other input at all we will extend it.
In particular, we want to transparently support UTF-8 input, so if the byte is not a backspace byte we simply append it without any further testing.

In our loop, unlike with search mode, we are not continuously changing what's displayed on the main area of the screen while the user is typing, so we can print an ANSI escape code to move to and clear the last row of the screen and then write our ex_command buffer on this last terminal row (including the ":").
(Obviously, we use prt() just like everywhere else in this codebase, not randomly printf for no reason.)
*/

void start_ex() {
    static char ex_buf[256] = ":";
    state->ex_command = (span){(u8*)ex_buf, (u8*)ex_buf + 1};

    prt("\033[%d;1H\033[K", state->terminal_rows);
    prt("%.*s", len(state->ex_command), state->ex_command.buf);
    flush();

    char ch;
    while ((ch = getch()) != '\n') {
        if (ch == '\b' || ch == 127) { // Handle backspace
            if (state->ex_command.end > state->ex_command.buf + 1) {
                state->ex_command.end--;
            } else { // Exit ex mode if only ":" is left
                state->ex_command = nullspan();
                print_current_blocks();
                return;
            }
        } else { // Append non-backspace input, including UTF-8
            if (state->ex_command.end < state->ex_command.buf + sizeof(ex_buf)) {
                *(state->ex_command.end++) = ch;
            }
        }
        // Move to and clear the last row of the screen
        prt("\033[%d;1H\033[K", state->terminal_rows);
        // Write ex_command buffer on this last terminal row
        prt("%.*s", len(state->ex_command), state->ex_command.buf);
        flush();
    }
    handle_ex_command();
}

/* #extable #handle_ex_command

Once the ex_command on the state is set up, this function actually handles it, and clears ex_command to leave ex command entry mode.
We put all the commands in spans first, so we can conveniently use starts_with for dispatching and len() with skip_n().
Note that we don't implement ex_help here, as it is already implemented; on :help we just call it.

The ex commands are defined as a table:

1. Supported ex commands:

:bootstrap, :addfile, :addlib, :help

2. Implementation functions:

bootstrap(), addfile(span), addlib(span), ex_help()

3. Arguments if any:

:addfile, :addlib:
  file path to add.

4. Help text:

bootstrap:
  Run the user-provided bootstrap command.

addfile, addlib:
  Add a file or library to the project (adds file: or lib: line to conf).

help:
  Print short help on available ex commands.
*/

// stubbed for now (manually)
void addfile(span s) {}
void addlib(span s) {}
void ex_help() {}

void handle_ex_command() {
    span ex_cmd = state->ex_command;
    if (starts_with(ex_cmd, S(":bootstrap"))) {
        bootstrap();
    } else if (starts_with(ex_cmd, S(":addfile"))) {
        addfile(skip_n(ex_cmd, 8));
    } else if (starts_with(ex_cmd, S(":addlib"))) {
        addlib(skip_n(ex_cmd, 7));
    } else if (starts_with(ex_cmd, S(":help"))) {
        ex_help();
    }
    state->ex_command = nullspan();
}

/* #bootstrap

Here we make sure that the conf variable bootstrap is set, and then run it.

The message is "The bootstrap command you provide should generate your initial prompt on stdout. It will be sent to the clipboard for you. See README for details.".

This is similar to #compile above.

However, instead of just running it and letting the output go to the terminal, we capture the output in a span using pipe_cmd_cmp(), which returns a span with the piped data.

Then we send this span to the clipboard with send_to_clipboard().
*/

void bootstrap() {
    ensure_conf_var(&state->bootstrap, S("The bootstrap command generates your initial prompt on stdout. See README for details."), nullspan());
    
    char buf[2048] = {0};
    s(buf, sizeof(buf), state->bootstrap);
    prt("Running bootstrap command: %s\n", buf);
    flush();

    span output = pipe_cmd_cmp(S(buf));
    send_to_clipboard(output);
}

/* #perform_search

In perform_search(), we update the display after the search string has been updated.

The search string (span state.search) will always start with a slash.
We remove this (there is no library method for this so just directly construct the span) and take the rest of it as the actual string to search for.
We iterate through the blocks and use spanspan to find the first block that matches, along with the number of other blocks that match.
If the search span is empty (as when only "/" was typed) then we match every block, so we can use empty() on the result of spanspan to detect a match, but we also match if the search span is empty().
(This will store the empty span at the beginning of the first block as the match span, which gives the behavior we want when printing the match later.)
We store both the index of the first block that matched and a copy of the span given by spanspan for this first block only, as we will need both of them later.

This tells us where the first block was that matched, and how many total blocks matched, and also the location in the span that contains the match.

Once we have our search results, we call clear_display().

Also at the top, we declare a local variable of remaining lines from the top of the terminal window, since we want to print something at the bottom later.
Every time we print a line or multiple lines, we decrement this value with the number of lines we printed, no more and no less.
In particular, we never decrement this "in advance," for lines to printed later, as that would violate the invariant that the number of actual remaining lines is the number in our variable.

Next, we print "Block N:" on a line (adding one as usual).
Then we decide how many initial lines of the block to print.
Without changing the terminal lines remaining value (since we indeed want our last line to be on the last line) we subtract 8 from this remaining number (leaving space for other output) and divide this by two to get the number of initial block physical lines to print.
Then we print this many physical lines of the block, by calling a function print_physical_lines(span,int).
(Note: physical lines means terminal rows used, as opposed to logical lines actually ending in newline.)

If nothing matched, then we do not print this part, but we still print the "N blocks matched" part later (0 in that case, of course) and the search string itself on the last line.

After the first lines of the block, we print a blank line, then "Match:" on a line, and then the line that contains the matched span.
We call a helper function, print_matching_physical_lines, which takes the block and the actual matched span (which we have from before), and handles finding and printing the match, and returns the number of physical lines that it used.

We print another blank line and then "N blocks matched".

Finally, we will add empty lines until we are at the bottom of the screen as indicated by terminal_rows on the state.
On the last line we will print the entire search string (including the slash).
(We can do this with wrs(), we don't need to add a newline as we are already on the last line of the window anyway.)
Before returning from the function we call flush() as we are responsible for updating the display.
*/

void perform_search() {
    int remaining_lines = state->terminal_rows;
    span search_span = {state->search.buf + 1, state->search.end};
    int match_count = 0;
    int first_match_index = -1;
    span first_match_span = nullspan();

    for (int i = 0; i < state->blocks.n; i++) {
        span match = spanspan(state->blocks.s[i], search_span);
        if (!empty(match) || empty(search_span)) {
            if (first_match_index == -1) {
                first_match_index = i;
                first_match_span = match;
            }
            match_count++;
        }
    }

    clear_display();

    if (first_match_index != -1) {
        prt("Block %d:\n", first_match_index + 1);
        remaining_lines -= 1;

        int initial_lines_to_print = (remaining_lines - 8) / 2;
        print_physical_lines(state->blocks.s[first_match_index], initial_lines_to_print);
        remaining_lines -= initial_lines_to_print;

        prt("\n");
        remaining_lines -= 1;

        prt("Match:\n");
        remaining_lines -= 1;

        int lines_printed = print_matching_physical_lines(state->blocks.s[first_match_index], first_match_span);
        remaining_lines -= lines_printed;

        prt("\n");
        remaining_lines -= 1;
    }

    prt("%d blocks matched\n", match_count);
    remaining_lines -= 1;

    while (remaining_lines > 1) {
        terpri();
        remaining_lines -= 1;
    }

    wrs(state->search);
    flush();
}
/*
In next_line_limit(span* s, int n) we are given a span and a length limit.
We return a span which is a prefix of s.
If the first n characters do not contain newline, and the n+1th character is also not a newline, then we return a span, not ending in newline, of n characters.
If the first n characters do not contain newline but n+1 does, we include n+1 characters, including the newline.
Otherwise, there is a newline within the first n characters, we return n or fewer characters with the last one being the newline.
We also set s->buf to contain the rest of the span not in the returned prefix.
*/


span next_line_limit(span* s, int n) {
    span result = {s->buf, s->buf}; // Initialize result span to start of input span
    if (s->buf == NULL || n <= 0) return result; // Check for null pointer or non-positive length

    int i;
    for (i = 0; i < n && (s->buf + i) < s->end; i++) {
        if (s->buf[i] == '\n') {
            result.end = s->buf + i + 1; // Include the newline in the result
            break;
        }
    }

    if (i == n) { // Reached the length limit without encountering a newline
        if ((s->buf + n) < s->end && s->buf[n] == '\n') {
            result.end = s->buf + n + 1; // Include the next character if it is a newline
        } else {
            result.end = s->buf + n; // Just include up to the limit
        }
    }

    s->buf = result.end; // Advance the start of the input span past the returned prefix
    return result;
}

/*
In print_ruler we use prt to show

- the number of blocks,
- the currently selected block,
- the scrolled lines plus one (i.e. the one-based index of the top visible line)
- the filename of the current block (get the block by state->current_index, the file by file_for_block, and the file path from the projfile at that index on state). Note that 0 <= current_index < state->blocks.n is an invariant, and we don't need to check for it, but rather simply assume it here, indexing into blocks directly.

all on a line without a newline.

Our output reads as "Block n/N, Line L, File <path>", using the `%.*s` pattern for the filepath.
*/

void print_ruler() {
    span current_block = state->blocks.s[state->current_index];
    int file_index = file_for_block(current_block);
    projfile current_file = state->files.a[file_index];
    char path_buf[2048] = {0}; // Assuming path lengths won't exceed 2047 characters + null terminator
    s(path_buf, 2048, current_file.path);

    prt("Block %d/%d, Line %d, File %s", state->current_index + 1, state->blocks.n, state->scrolled_lines + 1, path_buf);
}
/*
In print_single_block_with_skipping we get a block index and a pagination index in the form of a number of lines already "scrolled off" above the top of the screen (skipped_lines).

First, we call count_physical_lines, which gives us a span of skipped lines and alters an int, subtracting the number of physical lines which this span represents.
We make a copy of the block and adjust this copy to the suffix which is meant to be aligned to the top of our content area, by setting the .buf of the copy to the .end of the scrolled-off span.

We set a variable remaining_rows which we initialize with state->terminal_rows and decrement as we print lines of output.
First we print a line "Block N" and decrement this variable by one.

We have a ruler line at the bottom that we need to leave room for, so we make another variable, remaining_content_lines, that is one less than remaining lines, and call count_physical_lines again with this variable, letting us determine how many lines are actually printed, and more importantly, giving us a span of the appropriate content to at-most fill the screen.

We then print this content by wrs().
We then ...
Finally, we call print_ruler to handle the last line of the terminal.
*/

void print_single_block_with_skipping(int block_index, int skipped_lines) {
    span block = state->blocks.s[block_index];
    int physical_lines = skipped_lines;
    span skipped_span = count_physical_lines(block, &physical_lines);
    span block_suffix = block;
    block_suffix.buf = skipped_span.end;

    int remaining_rows = state->terminal_rows;
    prt("Block %d\n", block_index + 1);
    --remaining_rows;

    int remaining_content_lines = remaining_rows - 1;
    span content_to_print = count_physical_lines(block_suffix, &remaining_content_lines);
    wrs(content_to_print);

    /* *** manual fixup *** totally failed to get GPT4 to write this */
    while (remaining_content_lines-- > 0) {
        terpri();
    }

    print_ruler();
}

/*
In print_physical_lines, we get a span, and a number of lines.
We can use next_line on the span to get each logical line, and then we use the terminal_cols on the state to determine the number of physical lines that each one will require.
However, note that a blank line will still require one physical line (because we will print an empty line).
If the physical lines would be more than we need, then we only print enough characters (with wrapping) to fill the lines.
Otherwise we print the full line, followed by a newline, and then we decrement the number of lines that we still need appropriately.
*/


void print_physical_lines(span block, int lines_to_print) {
    while (!empty(block) && lines_to_print > 0) {
        span line = next_line(&block); // Get the next logical line from the block

        // Handle blank lines
        if (line.end == line.buf) {
            if (lines_to_print > 0) {
                terpri(); // Print a newline for a blank logical line
                lines_to_print--;
            }
            continue; // Move to the next line
        }

        // Calculate the number of physical lines required for this logical line
        int line_length = line.end - line.buf;
        int physical_lines_needed = (line_length / state->terminal_cols) + (line_length % state->terminal_cols != 0);

        if (physical_lines_needed <= lines_to_print) {
            // If the entire logical line fits within the remaining physical lines
            for (int i = 0; i < line_length; i += state->terminal_cols) {
                int chars_to_print = (i + state->terminal_cols > line_length) ? (line_length - i) : state->terminal_cols;
                prt("%.*s\n", chars_to_print, line.buf + i); // Print a segment of the logical line
            }
            lines_to_print -= physical_lines_needed;
        } else {
            // If the logical line does not fit entirely, print parts of it to fit in the remaining lines
            for (int i = 0; i < lines_to_print * state->terminal_cols; i += state->terminal_cols) {
                int chars_to_print = (i + state->terminal_cols > line_length) ? (line_length - i) : state->terminal_cols;
                prt("%.*s\n", chars_to_print, line.buf + i);
            }
            lines_to_print = 0; // We've filled the remaining lines
        }
    }
}
/*
In print_matching_physical_lines, we get the span of a block and of a match.

We must print the physical lines from the block which contain the match, and then return the number of physical lines that we have printed.

We happen to know, because of how our search currently works, that a match will never cross a logical line boundary (since we don't have a way of entering newlines in the search mode).

We will loop over the logical lines using next_line as usual.

When we find the logical line that contains the match (which we can do by comparing .buf and .end), then we determine where to start printing the physical lines.
(Note: we compare .buf and .end because we already have found a match, we do not use contains() which would search the strings again!)

(A logical line contains physical lines, which are runs of characters that fill the terminal_cols (on state), and therefore cause wrapping.)

Specifically, we want to skip any physical lines at the start of the logical line until we get to the first physical line that contains the match.
Then we print this physical line, and keep printing physical lines until we have reached the end of the match.
Whether or not we have printed the full logical line, we then print a newline.

We return the number of physical lines that we have printed.
*/

int print_matching_physical_lines(span block, span match) {

    int physical_lines_printed = 0;
    int terminal_width = state->terminal_cols;

    while (!empty(block)) {
        span line = next_line(&block);

        if ((match.buf >= line.buf) && (match.end <= line.end)) {
            int start_offset = match.buf - line.buf;
            int match_length = len(match);
            int start_physical_line = start_offset / terminal_width;
            int end_physical_line = (start_offset + match_length) / terminal_width;

            for (int i = start_physical_line; i <= end_physical_line; ++i) {
                int line_start = i * terminal_width;
                int line_end = (i + 1) * terminal_width;
                if (line_end > len(line)) {
                    line_end = len(line);
                }
                prt("%.*s\n", line_end - line_start, line.buf + line_start);
                physical_lines_printed++;
            }
            break;
        }
    }

    return physical_lines_printed;
}
/* #finalize_search

In finalize_search(), we update the current_index to point to the first result of the search given in the search string.
We ignore the first character of state.search which is always slash, and find the first block which contains the rest of the search string (using contains()).
Then we set current_index to that block, also resetting scrolled_lines.
We then reset state.search to an empty span to indicate that we are not in search mode any more, but we put the search on state.previous_search so that 'n' and 'N' can work.
*/

void finalize_search() {
    span search_term = skip_n(state->search, 1); // Skip the slash
    int found = -1;
    for (int i = 0; i < state->blocks.n && found == -1; i++) {
        if (contains(state->blocks.s[i], search_term)) {
            found = i;
        }
    }
    if (found != -1) {
        state->current_index = found;
        state->scrolled_lines = 0;
        state->previous_search = state->search;
        state->search = nullspan();
    }
}

/* #search_forward, #search_backward

In these two functions (used by n/N) we first get the sequence of blocks which match state.previous_search.
If this is empty, we do nothing.

As this includes the leading slash, we first strip that (using skip_n), then use contains() to find the blocks which match.

We then find either the lowest block greater than current_index which contains a match, for a forward search, or the highest matching block lower than current_index for a backward search.
In either case we set current_index to this other matching block.

If there is no other matching block, we do nothing.

If there are other matching blocks, but none which is higher/lower, then instead of wrapping around, we do nothing.

Later we can add match information (count and current) to the ruler, but we don't handle that yet.
*/

void search_forward() {
    if(empty(state->previous_search)) return;
    span search_term = skip_n(state->previous_search, 1);
    int match_index = -1;
    for (int i = 0; i < state->blocks.n; ++i) {
        if (i > state->current_index && contains(state->blocks.s[i], search_term)) {
            match_index = i;
            break;
        }
    }
    if (match_index != -1) {
        state->current_index = match_index;
    }
}

void search_backward() {
    if(empty(state->previous_search)) return;
    span search_term = skip_n(state->previous_search, 1);
    int match_index = -1;
    for (int i = state->blocks.n - 1; i >= 0; --i) {
        if (i < state->current_index && contains(state->blocks.s[i], search_term)) {
            match_index = i;
            break;
        }
    }
    if (match_index != -1) {
        state->current_index = match_index;
    }
}

/* Settings.

When the tool starts, we ask about specific configuration settings that must exist.
Otherwise, for settings like the buildcmd we only ask the first time the feature is used.

As settings are changed, we write a configuration file, which is always .cmpr/conf in the current working directory.

The contents of this file will be similar to RFC822-style headers.
We will have a key name followed by colon, space and then a value to the end of the line.

We have functions to read and write this format, which we do in the cmp space.

In handle_conf_{language,file} we are called with a span in each case containing either a pathname or a language, in the order that these config keys occur in the config file.

The idea is that when a language key occurs, it sets the language which is then used on subsequent files.
However, as the one exception to this pattern, when files are included before any language is given, then we will add the files without a language.

Therefore:

- when we see a language line, we will set the current_language on the state
- when we see a file line, we set the language on the file to current_language and add the file to the state
- only in the case where we see a language line and the language was not already set, then we know that this is the first language line in the file; in this case we iterate over any files that we already added, and set the language on them.

Below we write both functions handle_conf_{language,file}.
*/

void handle_conf_language(span language) {
    state->current_language = language;
    // Set the language for all previously added files if they have no language set
    if (state->files.n > 0 && empty(state->files.a[0].language)) {
        for (int i = 0; i < state->files.n; i++) {
            if (empty(state->files.a[i].language)) {
                state->files.a[i].language = language;
            }
        }
    }
}

void handle_conf_file(span file_path) {
    projfile file = { .path = file_path, .language = state->current_language, .contents = nullspan() };
    projfiles_push(&state->files, file);
}

/*
In the first function, parse_config, we read the contents of our config file (at state->config_file_path) into the cmp space, parse it, and set on the ui_state all the appropriate values.

We have a library method cmp_compl() which gives us the complement of cmp in cmp_space, which is the space that we can safely read into.
We'll read our configuration file into that space with read_file_S_into_span().

After we read the file into cmp, we get a span back from our library method containing the file contents.
We still have to manually update cmp.end to match the end of this span so that later uses of the cmp space don't clobber our configs.

We then can use next_line() in a loop to process them one by one.
First we look for ":" with find_char, and if it is not found, we skip the line.
After the colon we'll skip any whitespace with an isspace() loop and then consider the rest of the line to be the value.
(Note that we don't trim whitespace off the end, something maybe worth documenting elsewhere for config file users).
Next_line has already stripped the newline from the end so we don't need to do that.

Next, we compare the key part before the colon with our list of configuration values above, and if it matches any of them, we set the corresponding member of the ui state, which is always named the same as the config key name.
The values of the config keys will always be spans.
We have CONFIG_FIELDS defined above, we use that here with an X macro.

However, there is an exception to the handling of config fields for some values that might be handled specially.
These are not included in our CONFIG_FIELDS, but instead we handle them with custom code here.
These are:

- language
- file

These are handled by custom code, so we have functions handle_conf_{language,file} (already written above) that we call with the value span for either of these each time they occur in the config file.
*/

void parse_config() {
    span cmp_free_space = cmp_compl();
    span config_content = read_file_S_into_span(state->config_file_path, cmp_free_space);
    cmp.end = config_content.end; // Update cmp to avoid overwriting config

    while (!empty(config_content)) {
        span line = next_line(&config_content);
        int pos = find_char(line, ':');
        if (pos < 0) continue; // Skip line if no colon found

        span key = {line.buf, line.buf + pos};
        span value = {line.buf + pos + 1, line.end};

        // Skip initial whitespace in the value
        while (value.buf < value.end && isspace(*value.buf)) value.buf++;

        // Handle special keys
        if (span_eq(key, S("language"))) {
            handle_conf_language(value);
        } else if (span_eq(key, S("file"))) {
            handle_conf_file(value);
        } else {
            // Handle general configuration keys
            #define X(name) \
                if (span_eq(key, S(#name))) { \
                    state->name = value; \
                    continue; \
                }
            CONFIG_FIELDS
            #undef X
        }
    }
}

void settings_mode(){}
/*
In read_line, we get a span pointer to some space that we can use to store input from the user, and a default value.

Just to be sure, we always assert(len(buffer)) which helps catch some programming errors that have happened in the past.

We first print our prompt, which is "> ", followed by the default if any.

We use prt() as always.

We handle some basic line editing using our getch() in a loop, until enter is hit.
At that point we return a span containing the user's input.
The span which was passed in will have been shortened such that .end of our return value is now the .buf of the passed-in span.
(In this way, the caller can perform other necessary adjustments, or use the remaining buffer area in a loop etc.)

The line editing we support:

- all kinds of backspaces shorten the span by one and redraw the line (using ANSI escapes and \r).
- enter returns.
- everything else just gets appended to our span, which it extends.
*/

span read_line(span *buffer, span default_value) {
    assert(len(*buffer) > 0); // Ensure buffer is not empty
    span line = { .buf = buffer->buf, .end = buffer->buf }; // Initialize line span to empty
    if (!empty(default_value)) { // If default value is provided
        memcpy(buffer->buf, default_value.buf, len(default_value)); // Copy default into buffer
        line.end += len(default_value); // Adjust end of line span
    }
    prt("> %.*s", len(line), line.buf); // Print prompt and current line content
    flush(); // Ensure output is visible
    char ch;
    while ((ch = getch()) != '\n') { // Read input until enter is hit
        if (ch == '\b' || ch == 127) { // Handle backspace (ASCII DEL or backspace)
            if (line.buf < line.end) { // Check if there's a character to delete
                line.end--; // Shorten the span by one
                prt("\033[D \033[D"); // Move cursor back, clear character, move back again
            }
        } else { // For all other characters
            *line.end++ = ch; // Append character to span
            w_char(ch); // Print character
        }
        flush(); // Ensure output is visible
    }
    *buffer = (span){ .buf = line.end, .end = buffer->end }; // Adjust input buffer span to exclude the read line
    return (span){ .buf = line.buf, .end = line.end }; // Return the span containing user input
}

/* save_conf_files()

Here we write the language and file lines into the conf file.

The structure of this data in the conf file is that a language line should be included every time the language of the next file is different from the previous file.
For example, if there are three files, with languages C, Python, Python, then we would have a language line of C, then the first file, then language Python and both the other files.

Therefore, we maintain a local variable indicating the last-written language, initially empty of course.
For each file, if the language is already equal to this, then we just print the file line, otherwise we print a language line first.
When printing a language line, we put a blank line first, since these usually group related files together.

As mentioned elsewhere, each conf line includes the key, a colon, space and the value, followed by newline.
*/

void save_conf_files() {
    span last_written_language = nullspan();
    for (int i = 0; i < state->files.n; i++) {
        if (!span_eq(last_written_language, state->files.a[i].language)) {
            last_written_language = state->files.a[i].language;
            prt("\nlanguage: %.*s\n", len(last_written_language), last_written_language.buf);
        }
        prt("file: %.*s\n", len(state->files.a[i].path), state->files.a[i].path.buf);
    }
}
/*
In save_conf(), we simply rewrite the conf file to reflect any settings that may have been changed.

First we will write into the cmp space the current configuration.
Next, we will write that into the file named by state.config_file_path.
Finally we can shorten the cmp space back to what it was.

First we store a span that has .buf pointing to the current cmp.end.
Then we call prt2cmp() and use prt to print a line for each config var as described below.
Then we call prt2std().
Next we set the .end of that span to be the current cmp.end.

Then we call write_to_file_span with the span and the configuration file name.
Finally we shorten cmp back to what it was, since the contents have been written out we no longer need them around.

To print a config var, we print the name, a colon and single space, and then the value itself followed by newline.
(We currently assume that none of our conf vars contain newlines (a safe assumption, as if they did we'd also have no way to read them in).)
Our X macro handles the "normal" config fields, but then we call another function, save_conf_files, that handles the file and language lines that are special.
*/

void save_conf() {
    span original_cmp_end = {cmp.end, cmp.end};
    prt2cmp();

    #define X(name) prt(#name ": %.*s\n", len(state->name), state->name.buf);
    CONFIG_FIELDS
    #undef X

    save_conf_files();
    prt2std();
    original_cmp_end.end = cmp.end;

    write_to_file_span(original_cmp_end, state->config_file_path, 1);
    cmp.end = original_cmp_end.buf;
}

/* #add_projfile(span)

Here we add a file to the projfiles and ensure that the file exists on disk.
This is a helper function for check_conf_vars.

We add the file to the projfiles, but do not set a language on it (since that will be handled next by that function.)

If it is not creatable, writable by the user, etc, we will report the error, wait for a keystroke, and then return 0, otherwise we return 1.
*/

int add_projfile(span file_path_span) {
    char file_path[2048] = {0};
    s(file_path, 2048, file_path_span);

    FILE *file = fopen(file_path, "a+");
    if (file == NULL) {
        prt("Error: Cannot create or write to file %s.\nPress any key to continue...\n", file_path);
        flush();
        getch();
        return 0;
    }
    fclose(file);

    projfile new_file = {.path = file_path_span, .language = nullspan(), .contents = nullspan()};
    projfiles_push(&state->files, new_file);
    return 1;
}
/*
In check_conf_vars() we test the state for all essential configuration variables and if any is missing we prompt the user to set that.

The essential configuration variables, along with the reason why each is required, are:

revdir: We save a revision every time you edit a block, this is where they will be stored (.cmpr/revs is a good choice).
tmpdir: Location for temporary files for editing a block in the editor, a good choice is .cmpr/tmp.

TODO: probably all the above "config-related metadata" should be in one place in a table for all the conf vars; we'll do that later.
For now, a local macro cleans up this code quite a bit.

For each missing variable in the order given, we print the string after the colon above as a prompt to the user (and include the configuration parameter name).
(We end the prompt with colon and newline, so the read_line shows up under it.)

We then call read_line to get the new value from the user.
For the buffer space to use we will first call cmp_compl() to get the complement of cmp space as a span.
After read_line returns we will always set cmp.end to point to the end of the RETURNED span; this makes sure nothing else uses that space later.
NOTE: THIS MEANS THE VALUE THAT IS ****RETURNED**** BY THE READ_LINE FUNCTION.

Additional to the "normal" conf vars, we also have the files, and the language setting for each file.

First, it's a requirement that there be at least one file, even if it is empty, otherwise we have nowhere to put any blocks and no way to get things off the ground.
Therefore if there are no files on the state, we will prompt the user to name one file that can be added to the project.
Our prompt for this is "Enter at least one project file (e.g. main.c or main.py or index.js) where your blocks will be stored.".
We then create this as an empty file if it does not already exist by calling add_projfile() with the path.
Note that this function may fail, in which case it will return 0.
If it returns 0 (i.e. false), we loop until it returns positive, as there's no going forward without a place to store blocks.

Next, if there is a file in projfiles that doesn't have a language set, then we tell the user that this is required and set the language on the projfile in this case.
We tell them that the language must be one of our supported languages (see #langtable above), and determines how blocks start and also where the comment part ends and code part starts.
Our prompt for this is "Please specify a language for <file>, one of <list of languages>.", where the file is the path and the list of languages is from #langtable.

Once we have set all the required conf vars, if any of them were missing, including any languages on the projfiles, or adding the first projfile, then we call save_conf() which just rewrites the conf file.
To handle all of this we declare some int indicator(s) at the top of the function, set to 0 initially and use this to call save_conf() only if we changed some conf var.

[TODO: use the ensure_conf_var thing that we added after this (?)]
*/

void check_conf_vars() {
    int confChanged = 0;
    span cmpComplement = cmp_compl();

    #define CHECK_SET(var, prompt, defaultValue) \
    if (empty(state->var)) { \
        prt(prompt); \
        flush(); \
        span input = read_line(&cmpComplement, defaultValue); \
        cmp.end = input.end; \
        state->var = empty(input) ? defaultValue : input; \
        confChanged = 1; \
    }

    CHECK_SET(revdir, "Enter the directory to save revisions (e.g., .cmpr/revs):\n", S(".cmpr/revs"))
    CHECK_SET(tmpdir, "Enter the directory for temporary files (e.g., .cmpr/tmp):\n", S(".cmpr/tmp"))

    if (state->files.n == 0) {
        while (1) {
            prt("Enter at least one project file (e.g. main.c or main.py or index.js) where your blocks will be stored:\n");
            flush();
            span input = read_line(&cmpComplement, nullspan());
            cmp.end = input.end;
            if (add_projfile(input)) {
                confChanged = 1;
                break;
            }
        }
    }

    for (int i = 0; i < state->files.n; i++) {
        if (empty(state->files.a[i].language)) {
            prt("Please specify a language for %.*s, one of C, Python, JavaScript:\n", len(state->files.a[i].path), state->files.a[i].path.buf);
            flush();
            span input = read_line(&cmpComplement, nullspan());
            cmp.end = input.end;
            state->files.a[i].language = input;
            confChanged = 1;
        }
    }

    if (confChanged) {
        save_conf();
    }

    #undef CHECK_SET
}

/*
In ensure_conf_var() we are given a span, which must be one of the conf vars on the state, a message for the user to explain what the conf setting does and why it is required, and a default or current value that we can pass through to read_line.

If the conf var is not empty, we return immediately.

We call read_line to get the new value from the user.
For the buffer space to use we will first call cmp_compl() to get the complement of cmp space as a span.
After read_line returns we will always set cmp.end to point to the end of the returned span; this makes sure nothing else uses that space later.

Then we call save_conf() which just rewrites the conf file.
*/

void ensure_conf_var(span* var, span message, span default_value) {
    if (!empty(*var)) return; // If the configuration variable is already set, return immediately

    prt("%.*s\n", len(message), message.buf); // Print the message explaining the configuration setting
    if (!empty(default_value)) {
        prt("Default: %.*s\n", len(default_value), default_value.buf); // Show default value if provided
    }

    span buffer = cmp_compl(); // Get complement of cmp space as a span for input
    *var = read_line(&buffer, default_value); // Read new value from user

    cmp.end = buffer.buf; // Update cmp.end to the end of the returned span from read_line

    save_conf(); // Rewrite the configuration file with the updated setting
}

/*
To edit the current block we first write it out to a file, which we do with a helper function write_to_file(span, char*).

Once the file is written, we then launch the user's editor of choice on that file, which is handled by another helper function.

That function will wait for the editor process to exit, and will indicate to us by its return value whether the editor exited normally.

If so, then we call another function which will then read the edited file contents back in and handle storing the new version.

If not, we print a short message to let the user know their changes were ignored.
*/

void edit_current_block() {
  if (state->current_index >= 0 && state->current_index < state->blocks.n) {
    char* filename = tmp_filename();
    write_to_file(state->blocks.s[state->current_index], filename);
    int editor_exit_code = launch_editor(filename);

    if (editor_exit_code == 0) {
      handle_edited_file(filename);
    } else {
      prt("Editor exited abnormally. Changes might not be saved.\n");
    }

    free(filename);
  }
}

/*
To generate a tmp filename for launching the user's editor, we return a string starting with state->tmpdir.
For the filename part, we construct a timestamp in a compressed ISO 8601-like format, as YYYYMMDD-hhmmss with just a single dash as separator.
We append a file extension: we use file_for_block and current_block to get the language for the current block and add the appropriate extension, switching on the language and adding the appropriate filename extension from #langtable above.
We are assure here that the language will be already set for every file, so if it is not recognized, it's acceptable to either crash (as this is a programming error) or simply to do nothing and carry on with no extension on added.
(The only thing we don't want to do is add something ridiculous like ".txt".)
We return a char* which the caller must free.
Since state->tmpdir may or may not include a trailing slash we test for and handle both cases.
*/

char* tmp_filename() {
    time_t now = time(NULL);
    struct tm *tm_struct = localtime(&now);

    char timestamp[20]; // YYYYMMDD-hhmmss
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm_struct);

    char extension[5]; // Up to 4 chars + null terminator
    span language = state->current_language;
    if (span_eq(language, S("C"))) {
        snprintf(extension, sizeof(extension), ".c");
    } else if (span_eq(language, S("Python"))) {
        snprintf(extension, sizeof(extension), ".py");
    } else if (span_eq(language, S("JavaScript"))) {
        snprintf(extension, sizeof(extension), ".js");
    } else {
        extension[0] = '\0'; // No recognized language, no extension.
    }

    int tmpdir_len = len(state->tmpdir);
    int needs_slash = (state->tmpdir.buf[tmpdir_len - 1] != '/');
    int filename_len = tmpdir_len + needs_slash + sizeof(timestamp) + sizeof(extension);
    char *filename = malloc(filename_len);
    if (needs_slash) {
        snprintf(filename, filename_len, "%.*s/%s%s", tmpdir_len, state->tmpdir.buf, timestamp, extension);
    } else {
        snprintf(filename, filename_len, "%.*s%s%s", tmpdir_len, state->tmpdir.buf, timestamp, extension);
    }

    return filename;
}
/*
In launch_editor, we are given a filename and must launch the user's editor of choice on that file, and then wait for it to exit and return its exit code.

We look in the env for an EDITOR environment variable and use that if it is present, otherwise we will use "vi".

As always, we never write const anywhere in C.
*/

int launch_editor(char* filename) {
    char* editor = getenv("EDITOR");
    if (editor == NULL) {
        editor = "vi"; // Default to vi if EDITOR is not set
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        execlp(editor, editor, filename, (char*)NULL);
        // If execlp returns, it means it failed
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status); // Return the exit status of the editor
        } else {
            return -1; // Editor didn't exit normally
        }
    }
}
/* int file_for_block(span)

Here we're given the span of a block and we must find out the index of the file that contains that block.

If .buf of the span is >= .buf of the contents of the file (recall that file contents are spans into inp and so are blocks), and the .end of the block is similarly <= .end of the file, then the file contains the block.

If the block overlaps a file boundary, something has gone badly wrong and we complain as exit as usual (prt, flush, exit).

Similarly if it's outside all the files.
In short, we always return an int n, 0 <= n <= state->files.n, or don't return at all.
*/

int file_for_block(span block) {
    for (int i = 0; i < state->files.n; i++) {
        if (block.buf >= state->files.a[i].contents.buf && block.end <= state->files.a[i].contents.end) {
            return i;
        }
    }
    prt("Error: Block does not belong to any file.\n");
    flush();
    exit(EXIT_FAILURE);
}
/* handle_edited_file

Here we are given a tmp file containing new contents of a block.

Every block belongs to a projfile, and the blocks are in order of the projfiles (set up in the conf file).

What we must do is replace the existing block contents with the new contents of that tmp file, and then write everything out to a new file on disk called a rev for the particular projfile that contains that block.

We already have on state all the current values of the blocks, which are spans that point into inp.
First we want to fix inp to reflect the new reality, and then we will write out the new disk file from inp (our global input span).
We will also fix the contents spans of all of the files starting with this one, and including all later ones.

We get the span corresponding to the current block, which is also the edited block, in a local variable for convenience.
We can do the same for the .contents of the file, calling file_for_block to get the index.

The contents of inp are already correct, up to the start of this block, which is what we want to replace.
Now we get the size of the tmp file and compare it to the len of the existing block (recall that len(span) exists and is one of our most commonly used library methods).
If it is larger, we need to move the contents after it (in inp) to the right in memory, if smaller, to the left.
We can do this using memmove.

Once we have done this, we have everything correct in inp except for the contents of the file itself, and we have a space that's sufficient for the file contents to be read into.
We construct a span that represents this space.
This span starts at the .buf of the original block, but has the length of the file.

We have a library function, span read_file_into_span(char*,span), which takes a span and reads a file into a prefix of that span, and then returns that prefix as another span.
We can pass our "gap" span representing the part of inp that we want to replace to this function, along with the filename.
We can confirm that the span that is returned is in fact the identical span as the buffer we passed in, since we are expecting that the file length has not changed.

As usual, if this expectation is violated we will complain and exit (using our exit_with_error(char*) convenience function).

Now inp is representing the new state of the project.

However, the file[i].contents for the files including and subsequent to this one may be incorrect.
Specifically, the difference in length of this block must be added to the .end of the file contents span for this file, and to both the .buf and .end of every subsequent file.

As a sanity check, after this step, we could validate that the .end of the last file is equal to the .end of inp itself.

We need to update the blocks, since any blocks after and including this one may have moved, so we call find_all_blocks().

Then we call a helper function, new_rev, which takes the filename and the file index for the projfile that was altered.
This function is responsible for storing a new rev, cleaning up the tmp file, and any reporting to the user that we might do.
*/

void handle_edited_file(char* filename) {
    int file_index = file_for_block(state->blocks.s[state->current_index]);
    span original_block = state->blocks.s[state->current_index];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        prt("Error: Failed to open edited file.\n");
        flush();
        exit(EXIT_FAILURE);
    }
    off_t new_size = lseek(fd, 0, SEEK_END);
    ssize_t size_diff = new_size - (original_block.end - original_block.buf);
    close(fd);

    // Adjusting the memory in inp for new content size
    memmove(original_block.buf + new_size, original_block.end, inp.end - original_block.end);
    inp.end += size_diff;

    // Reading the new block content into the adjusted space in inp
    span gap = {original_block.buf, original_block.buf + new_size};
    span result = read_file_into_span(filename, gap);
    if (len(result) != new_size) {
        prt("Error: Unexpected file size after reading edited content.\n");
        flush();
        exit(EXIT_FAILURE);
    }

    // Adjusting file contents spans for this and subsequent files
    state->files.a[file_index].contents.end += size_diff;
    for (int i = file_index + 1; i < state->files.n; ++i) {
        state->files.a[i].contents.buf += size_diff;
        state->files.a[i].contents.end += size_diff;
    }

    // Sanity check
    if (state->files.a[state->files.n - 1].contents.end != inp.end) {
        prt("Error: Inconsistent state after updating file contents.\n");
        flush();
        exit(EXIT_FAILURE);
    }

    // Updating the blocks representation
    find_all_blocks();

    // Creating a new revision and cleaning up
    new_rev(filename, file_index);
}

/* #new_rev

Here we store a new revision, given a filename which contains a block that was edited and the index of the projfile that contains that block.
The file contents have already been processed, we just get the name so that we can clean up the file when done.

Our conf var revdir tells us where to put the new file that we are going to write.
We don't assume that the conf var ends in a slash, so we test for that and handle both cases.

First we construct a path starting with revdir and ending with an ISO 8601-style compact timestamp like 20240501-210759.
We then write the contents of the projfile (which has already been updated) into this file (the "rev") using write_to_file().

We create a copy of this file at the path for the projfile.
The file there which may have been edited and contain unsaved changes by some other process.
So we have a helper function, update_projfile, which takes the projfile index, the path of the rev file, and handles all of this.

Finally we unlink the filename that was passed in, since we have now fully processed it.
The filename is now optional, since we sometimes also are processing clipboard input, so if it is NULL we skip this step.

Reminder: we never write `const` in C as it only brings pain.
*/

void update_projfile(int, char*);

void new_rev(char* filename, int file_index) {
    char rev_path[1024];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    int need_slash = state->revdir.end[-1] != '/';
    snprintf(rev_path, sizeof(rev_path), "%.*s%s%04d%02d%02d-%02d%02d%02d", 
             (int)(state->revdir.end - state->revdir.buf), state->revdir.buf,
             need_slash ? "/" : "",
             tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
             tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

    write_to_file(state->files.a[file_index].contents, rev_path);

    update_projfile(file_index, rev_path);

    if (filename) {
        unlink(filename);
    }
}

void update_projfile(int file_index, char* rev_path) {
    char projfile_path[1024];
    snprintf(projfile_path, sizeof(projfile_path), "%.*s", 
             (int)(state->files.a[file_index].path.end - state->files.a[file_index].path.buf), 
             state->files.a[file_index].path.buf);

    // Check if the projfile exists and handle accordingly
    struct stat statbuf;
    if (lstat(projfile_path, &statbuf) == 0) {
        if (S_ISREG(statbuf.st_mode)) {
            // If it's a regular file, we might want to back it up before overwriting
            char backup_path[1024];
            snprintf(backup_path, sizeof(backup_path), "%s.bak", projfile_path);
            rename(projfile_path, backup_path);
        }
    }

    // Create or overwrite the projfile with the new revision
    link(rev_path, projfile_path);
}
/*
SH_FN_START

update_symlink() {
  rm -r cmpr/cmpr.c; ln -s $(cd cmpr; find revs | sort | tail -n1; ) cmpr/cmpr.c
}

SH_FN_END
*/

/* #rewrite_current_block_with_llm

Here we call a helper function with the block contents which returns the top part of the block, which is usually a comment, stripping the rest which is usually code.
It calls another function to turn this comment part into a prompt.

We then call another function which writes that span into a file and calls a user-provided command that places the contents of that file on the clipboard.

Helper functions:
span block_comment_part(span);
span comment_to_prompt(span);
void send_to_clipboard(span);
*/

void rewrite_current_block_with_llm() {
  if (state->current_index < 0 || state->current_index >= state->blocks.n) {
    fprintf(stderr, "Invalid block index.\n");
    return;
  }

  // Extract the comment part of the current block
  span current_block = state->blocks.s[state->current_index];
  span comment = block_comment_part(current_block);

  if (comment.buf == NULL || len(comment) == 0) {
    fprintf(stderr, "No comment found in the current block.\n");
    return;
  }

  // Convert the comment to a prompt
  span prompt = comment_to_prompt(comment);
  if (prompt.buf == NULL || len(prompt) == 0) {
    fprintf(stderr, "Failed to create a prompt from the comment.\n");
    fflush(stderr);
    exit(1);
    return;
  }

  // Send the prompt to the clipboard
  send_to_clipboard(prompt);
}

/* #block_comment_part

To split out the block_comment_part of a span, we first write two helper functions, one for C and one for Python.
Others will be added but these are the only ones we've needed so far.

Then we dispatch based on the language on the projfile which we lookup using file_for_block(), and .language on the file in the state.
Previously, if this span contained "Python" we called the Python version, and otherwise we called the C version (which was therefore our default).
Now, however, we look up the language in the #langtable above and call the find block comment end implementation.
For example, the third language we support is JS, which uses the same implementation as C.

The helper function always takes a span and returns an index offset to the location of the "block comment part terminator".
For Python this is the second occurrence of the triple doublequote in the block, and for C it is star slash.

In the main function block_comment_part, we return the span up to and including the comment part terminator, and also including any newlines and whitespace after it.
This means each of the helper functions returns the offset at which the comment terminator ends, and the main function includes a while isspace loop to advance past any whitespace.
(Among other things, this ensures that "R" does not unduly change the amount of whitespace mid-block.)

All three functions are written below.
*/

static int find_block_comment_end_c(span block) {
    for (int i = 0; i < len(block) - 1; i++) {
        if (block.buf[i] == '*' && block.buf[i + 1] == '/') {
            return i + 2; // Return the index just past the terminator
        }
    }
    return len(block); // If not found, return the length of the block
}

static int find_block_comment_end_python(span block) {
    int count = 0;
    for (int i = 0; i < len(block) - 2; i++) {
        if (block.buf[i] == '"' && block.buf[i + 1] == '"' && block.buf[i + 2] == '"') {
            count++;
            if (count == 2) { // Second occurrence
                return i + 3; // Return the index just past the terminator
            }
            i += 2; // Skip past this triple-quote
        }
    }
    return len(block); // If not found, return the length of the block
}

span block_comment_part(span block) {
    int index = file_for_block(block);
    span language = state->files.a[index].language;
    int end;
    if (span_eq(language, S("Python"))) {
        end = find_block_comment_end_python(block);
    } else { // Default to C, includes C and JavaScript
        end = find_block_comment_end_c(block);
    }
    
    while (end < len(block) && isspace(block.buf[end])) {
        end++; // Advance past whitespace
    }

    span result = {block.buf, block.buf + end};
    return result;
}

/* #print_block #print_comment #print_code #count_blocks

In these print_* implementations we print block contents completely or partially.

We use block_comment_part to get a span containing the comment part of a block.
We can use this pattern to get the code part: span code_part = block; code_part.buf = comment_part.end.

TODO: add this as a spanio method (complement of span in span).

count_blocks() is a trivial wrapper around state.blocks.n.
*/

void print_comment(int index) {
    if (index < 0 || index >= state->blocks.n) return;
    span block = state->blocks.s[index];
    span comment_part = block_comment_part(block);
    wrs(comment_part);
    terpri();
}

void print_code(int index) {
    if (index < 0 || index >= state->blocks.n) return;
    span block = state->blocks.s[index];
    span comment_part = block_comment_part(block);
    span code_part = block;
    code_part.buf = comment_part.end;
    wrs(code_part);
    terpri();
}

void print_block(int index) {
    if (index < 0 || index >= state->blocks.n) return;
    span block = state->blocks.s[index];
    wrs(block);
    terpri();
}

int count_blocks() {
    return state->blocks.n;
}

/* #find_block

This is similar to the search implementation: we iterate over all the blocks, find the first block which contains the literal search text provided, and return the index of that block (or -1 if none matches).
*/

int find_block(span search_text) {
    for (int i = 0; i < state->blocks.n; i++) {
        if (contains(state->blocks.s[i], search_text)) {
            return i;
        }
    }
    return -1;
}

/*
In comment_to_prompt, we use the cmp space to construct a prompt around the given block comment.

First we should create our return span and assign .buf to the cmp.end location.

Then we call prt2cmp.

Next we prt the a literal string (such as "```c\n").
Then we use wrs to write the span passed in as our argument.
Finally we write "```\n\n" and an instruction.

The above three elements are given in #langtable above as 6., block comment part to prompt pattern.

Next we call prt2std to go back to the normal output mode.

Then we can get the new end of cmp and make that the end of our return span so that we return everything written into the cmp space.
*/

span comment_to_prompt(span comment) {
    span ret;
    ret.buf = cmp.end;
    prt2cmp();
    if (state->current_language.buf[0] == 'C') {
        prt("```c\n");
    } else if (state->current_language.buf[0] == 'P') { // Python
        prt("```python\n");
    } else if (state->current_language.buf[0] == 'J') { // JavaScript
        prt("```js\n");
    }
    wrs(comment);
    if (state->current_language.buf[0] == 'C') {
        prt("```\n\nWrite the code. Reply only with code. Do not include comments.\n");
    } else if (state->current_language.buf[0] == 'P') { // Python
        prt("```\n\nWrite the code. Reply only with code. Avoid code_interpreter.\n");
    } else if (state->current_language.buf[0] == 'J') { // JavaScript
        prt("```\n\nWrite the code. Reply only with code. Do not include comments.\n");
    }
    prt2std();
    ret.end = cmp.end;
    return ret;
}
/*
In send_to_clipboard, we are given a span and we must send it to the clipboard using a user-provided method, since this varies quite a bit between environments.

There is a global ui_state* variable "state" with a span cbcopy on it.

Before we do anything else we ensure this is set by calling ensure_conf_var with the message "The command to pipe data to the clipboard on your system. For Mac try \"pbcopy\", Linux \"xclip -i -selection clipboard\", Windows please let me know and I'll add something here".

We run this as a command and pass the span data to its stdin.
We use the `s()` library method and heap buffer pattern to get a null-terminated string for popen from our span conf var.

We complain and exit if anything goes wrong as per usual.
*/

void send_to_clipboard(span content) {
    ensure_conf_var(&(state->cbcopy), S("The command to pipe data to the clipboard on your system. For Mac try \"pbcopy\", Linux \"xclip -i -selection clipboard\", Windows please let me know and I'll add something here"), S(""));

    char command[2048];
    snprintf(command, sizeof(command), "%.*s", (int)(state->cbcopy.end - state->cbcopy.buf), state->cbcopy.buf);

    FILE* pipe = popen(command, "w");
    if (!pipe) {
        perror("Failed to open pipe for clipboard command");
        exit(EXIT_FAILURE);
    }

    fwrite(content.buf, sizeof(char), content.end - content.buf, pipe);

    if (pclose(pipe) != 0) {
        perror("Failed to execute clipboard command");
        exit(EXIT_FAILURE);
    }
}
/* #compile()

In compile(), we take the state and execute buildcmd, which is a config parameter.

First we call ensure_conf_var(state->buildcmd), since we are about to use that setting.

Next we print the command that we are going to run and flush, so the user sees something before the compiler process, which may be slow to produce output.

Then we use system(3) on a 2048-char buf which we allocate and statically zero.
Our s() interface (s(char*,int,span)) lets us set buildcmd as a null-terminated string beginning at buf.

We wait for another keystroke before returning if the compiler process fails, so the user can read the compiler errors (later we'll handle them better).
(Remember to call flush() before getch() so the user sees the prompt (which is "Build failed, press any key to continue...").)

On the other hand, if the build succeeds, we don't need the extra keystroke and go back to the main loop after a 1s delay so the user has time to read the success message before the main loop refreshes the current block.
In this case we prt "Build succeeded" on a line.

(We aren't doing this yet, but later we'll put something on the state to provide more status info to the user.)
*/

void compile() {
    ensure_conf_var(&state->buildcmd, S("The build command will be run every time you hit 'B' and should build the code you are editing (typically in projfile)"), nullspan());
    
    char buf[2048] = {0};
    s(buf, sizeof(buf), state->buildcmd);
    
    prt("Running command: %s\n", buf);
    flush();
    
    int status = system(buf);
    
    if (status != 0) {
        prt("Build failed, press any key to continue...\n");
        flush();
        getch();
    } else {
        prt("Build succeeded\n");
        flush();
        sleep(1); // Give time for the user to read the message
    }
}
/* #replace_code_clipboard

In replace_code_clipboard, we pipe in the result of running state->cbpaste.

First we call ensure_conf_var with the message "Command to get text from the clipboard on your platform (Mac: pbpaste, Linux: try xclip -o -selection clipboard, Windows: ?)"

This will contain a command like "xclip -o -selection clipboard" (our default) or "pbpaste" on Mac, and comes from our conf file.

Then we call pipe_cmd_cmp(span) and returns a span of piped in data from the clipboard, which we pass on to replace_block_code_part().
*/

span pipe_cmd_cmp(span);

void replace_code_clipboard() {
    ensure_conf_var(&state->cbpaste, S("Command to get text from the clipboard on your platform (Mac: pbpaste, Linux: try xclip -o -selection clipboard, Windows: ?)"), S("xclip -o -selection clipboard"));
    span new_content = pipe_cmd_cmp(state->cbpaste);
    replace_block_code_part(new_content);
}

/* #pipe_cmd_cmp()

We get a command (as a span) and we run it, sending the output into the complement of cmp in cmp_space.

Use s() and a char[2048] to prep the popen() call.

We use the cmp buffer to store this data, starting from cmp.end (which is always somewhere before the end of the cmp space big buffer).

The space that we can use is the difference between cmp_space + BUF_SZ, which locates the end of the cmp_space, and cmp.end, which is always less than this limit.

We then create a span, which is pointing into cmp, capturing the new data we just captured, which is our return value.

Note: obviously we need the value of fread() to know the length of the incoming data

Note: we can assert that the bytes read fits in the cmp space
*/

span pipe_cmd_cmp(span cmd) {
    char cmd_str[2048];
    s(cmd_str, 2048, cmd);
    FILE *pipe = popen(cmd_str, "r");
    assert(pipe != NULL);

    size_t space_available = (cmp_space + BUF_SZ) - cmp.end;
    size_t bytes_read = fread(cmp.end, 1, space_available, pipe);
    assert(bytes_read <= space_available);

    span result = {cmp.end, cmp.end + bytes_read};
    cmp.end += bytes_read;

    pclose(pipe);
    return result;
}

/* #replace_block_code_part(span)

Here we get a span (into cmp space) which contains a new code part for the current block.

Note: To get the length of a span, use len().
Note: Do this every time, not the just the first time.

Similar to handle_edited_file() above, we are given a span (instead of a file) and we must update the current block, moving data around in memory as necessary.

We put the original block's span in a local variable for convenience.

The end result of inp should contain:
- the contents of inp currently, up to the start (the .buf) of the original block.
- the comment part of the current block, which we can get from block_comment_part on the current block.
- up to two newlines unless the block comment part already ends with them
- the code part coming from the clipboard in our second argument
- one newline after the code part and before the next block
- current contents of inp from the .end of the original block to the inp.end of original input.

We check whether we are adding zero, one, or two newlines between the comment part and the new code part.
If the comment part is empty or is less than len 2, we don't do the check and don't add any newlines.
We get the index of the projfile from file_for_block.
Then we check the length of what the new block will be (comment part + opt. newlines + new part + newline).
We then compare this to the old block length and do a memmove if necessary on the "rest" of inp, so that we have a gap to accommodate the new block's len.
Then we simply copy any newlines and the new code into inp.
(We do not need to copy the comment part, as it is already there in the original block.)

We then must update the .end of the current file contents, and both the .buf and .end of all subsequent projfiles, since the block length may have changed and therefore the file contents lengths will have also changed.

As before we then find the current locations of the blocks.

Once all this is done, we call new_rev, passing NULL for the filename argument, since there's no filename here.
*/

void replace_block_code_part(span new_code) {
    int file_index = file_for_block(state->blocks.s[state->current_index]);
    span original_block = state->blocks.s[state->current_index];
    span comment_part = block_comment_part(original_block);

    int newlines_needed = 2;
    if (len(comment_part) > 0 && comment_part.end[-1] == '\n') {
        newlines_needed = 1;
        if (comment_part.end - comment_part.buf > 1 && comment_part.end[-2] == '\n') {
            newlines_needed = 0;
        }
    }

    int newlines_needed_after_code = 1;

    size_t new_block_length = len(comment_part) + newlines_needed + len(new_code) + newlines_needed_after_code;
    ssize_t size_diff = new_block_length - (original_block.end - original_block.buf);

    if (size_diff != 0) {
        memmove(original_block.end + size_diff, original_block.end, inp.end - original_block.end);
    }

    unsigned char* current_pos = original_block.buf + len(comment_part);
    for (int i = 0; i < newlines_needed; ++i) {
        *current_pos++ = '\n';
    }

    memcpy(current_pos, new_code.buf, len(new_code));

    *(current_pos + len(new_code)) = '\n';

    // Update inp.end to reflect the new size
    inp.end += size_diff;

    // Update the contents span of the current and subsequent files
    state->files.a[file_index].contents.end += size_diff;
    for (int i = file_index + 1; i < state->files.n; ++i) {
        state->files.a[i].contents.buf += size_diff;
        state->files.a[i].contents.end += size_diff;
    }

    // Re-find all the blocks since inp has changed
    find_all_blocks();

    // Store a new revision, no filename required
    new_rev(NULL, file_index);
}
/* cmpr_init
We are called without args and set up some configuration and empty directories to prepare the CWD for use as a cmpr project.

In sh terms:

- mkdir -p .cmpr/{,revs,tmp}
- touch .cmpr/conf

Note that if the CWD is already initialized this is a no-op i.e. the init is idempotent (up to file access times and similar).

TODO: can set revs and tmp already in conf, if not already set.
*/

void cmpr_init() {
    mkdir(".cmpr", 0755);
    mkdir(".cmpr/revs", 0755);
    mkdir(".cmpr/tmp", 0755);

    FILE *file = fopen(".cmpr/conf", "a");
    if (file != NULL) {
        fclose(file);
    }
}

