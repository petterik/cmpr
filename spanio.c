/* #libraryintro

Introducing the spanio library.
We always use spanio methods when possible and never use null-terminated C strings (except when calling library functions or at other interface boundaries where there is no way around it, see s()).
When we say "print" we mean what prt() does, which is append output to the output span out.
You still must flush() before the output will be printed to stdout and be visible to the user.

- `empty(span)`: Checks if a span is empty (start and end pointers are equal).
- `len(span)`: Returns the length of a span. Always prefer this over manually subtracting.
- `init_spans()`: Initializes input, output, and comparison spans and their associated buffers.
- `prt2cmp()`, `prt2std()`: Swaps output and comparison spans.
- `prt(const char *, ...)`: Formats and appends a string to the output span, i.e. prints it. Pronounced as prt.
- `w_char(char)`: Writes a single character to the output span.
- `wrs(span)`: Writes the contents of a span to the output span.
- `bksp()`: Backspace, shortens the output span by one.
- `sp()`: Appends a space character to the output span, i.e. prints a space.
- `terpri()`: Appends a newline character to the output span.
- `flush()`, `flush_err()`, `flush_to(char*)`: Flushes the output span to standard output, standard error, or a specified file.
- `write_to_file_span(span content, span path, int clobber)`: Write a span to a file, optionally overwriting.
- `write_to_file(span, const char*)`: Deprecated.
- `read_file_into_span(char*, span)`: Reads the contents of a file into a span.
- `read_file_S_into_span(span, span)`: Ibid, but taking the filename as a span.
- `read_file_into_cmp(span)`: Filename as a span, returns contents as a span inside cmp space.
- `read_file_into_inp(span)`: Filename as a span, returns contents as a span inside inp space.
- `redir(span)`, `reset()`: Redirects output to a new span and resets it to the previous output span.
- `save()`, `push(span)`, `pop(span*)`, `pop_into_span()`: Manipulates a stack for saving and restoring spans.
- `advance1(span*)`, `advance(span*, int)`: Advances the start pointer of a span by one or a specified number of characters.
- `find_char(span, char)`: Searches for a character in a span and returns its first index or -1.
- `contains(span, span)`: Checks if one span TEXTUALLY contains another; abc b O(n) string search.
- `contains_ptr(span, span)`: Checks if one span PHYSICALLY contains another; [[]] O(1) pointer comparisons.
- `starts_with(span, span)`: Check if spans share $1 as a prefix; O(len($1)) when successful.
- `consume_prefix(span, span*)`: Shortens a span by a prefix if present, returning that prefix or nullspan().
- `take_n(int, span*)`: Returns as a new span the first n characters from a span, mutating it.
- `first_n(span, int)`: Returns n leading chars as a new span without mutating.
- `skip_n(span, int)`: Returns a span skipping initial chars.
- `skip_whitespace(span*)`: modifies a span, returning a prefix span of zero or removed whitespace.
- `next_line(span*)`: Extracts the next line (up to \n or .end) from a span and returns it as a new span.
- `span_eq(span, span)`, `span_cmp(span, span)`: Compares two spans for equality or lexicographical order.
- `S(char*)`: Creates a span from a null-terminated string.
- `char* s(char*,int,span)`: Creates a null-terminated string in a user-provided buffer of provided length from given span, which it also returns for convenience.
- `nullspan()`: Returns the empty span at address 0.
- `spans_alloc(int)`: Allocates a spans structure with a specified number of span elements.
- `span_arena_alloc(int)`, `span_arena_free()`, `span_arena_push()`, `span_arena_pop()`: Manages a memory arena for dynamic allocation of spans.
- `is_one_of(span, spans)`: Checks if a span is one of the spans in a spans.
- `spanspan(span, span)`: Finds the first occurrence of a span within another span and returns a span into haystack.
- `w_char_esc(char)`, `w_char_esc_pad(char)`, `w_char_esc_dq(char)`, `w_char_esc_sq(char)`, `wrs_esc()`: Write characters (or in the case of wrs, spans) to out, the output span, applying various escape sequences as needed.
- `trim(span)`: Gives the possibly smaller span with any isspace(3) trimmed on both sides.
- `json_s(span)`: prt a JSON string of $1
- `json_n(f64)`: prt a + or - inf, a NaN, or a floating point value or integer safely up to 2^53.
- `json_b(int)`: prt a true or false (only 0 is false).
- `json_0()`: prt a json_null value ("null").
- `json_o()`: prt an empty json object.
- `json_o_extend(json*,span,json)`: extends $1 with key $2 and value $3.
- `json_a()`: prt an empty json array.
- `json_a_extend(json*,json)`: extends $1 with key $2.
- `json_sp(json)`: 1 if $1 is a string (given that it is a json).
- `json_{s,n,b,0,o,a}p`: full list of json_?p predicate funcs.
- `json_key(span, json)`: gives a nullable json; object lookup.
- `json_index(int, json)`: gives a nullable json; array lookup.
- `json_parse(span)`: parse a span into a json and return it; may be shorter only by trimmed whitespace.
- `make_json(span)`: return a json wrapper of the span in O(1).
- note that all the json functions either return the json type (which they also prt, usually this is sent to cmp space) as in the _{s,n,b,0,o,a} constructors, or they return 0 or 1, or they return a nullable json (null representation: just a json containing the null span).
- all the json constructor functions trim whitespace, and all the predicate functions follow a pointer and examine one byte.
- the json parser and constant-time wrapper functions are the low-trust and high-trust ways to make a json from a string.

typedef struct { u8* buf; u8* end; } span; // the type of span

typedef struct { span *s; int n; } spans; // the type of spans, given by spans_alloc(n)

We have a generic array implementation using arena allocation.

- Call MAKE_ARENA(E,T,STACK_SIZE) to define array type T for elements of type E.
  - T has .a and .n on it of type E* and int resp.
- Use T_arena_alloc(N) and T_arena_free(), typically in main() or similar.
- T_alloc(N) returns an array of type T, with .n = .cap = N.
- T_arena_push() and T_arena_pop() manage arena allocation stack.

Note that spans uses .s for the array member, not .a like all of our generic array types.
This is because spans predates our generic arena implementation, and we should fix it.
In particular projfiles (state->files) uses .a as it is a generic array type.

Note that spans has only a .n not a separate capacity value.
Therefore spans_alloc returns a spans which has n equal to the number passed in.
Typically the caller will either fill the number requested exactly, or will shorten n if fewer are used.
These point into the spans arena which must be set up by calling span_arena_alloc() with some integer argument (in bytes).
A common idiom is to iterate over something once to count the number of spans needed, then call spans_alloc and iterate again to fill the spans.

A common idiom in functions returning span: use a span ret declared near the top, with buf pointing to one thing and end to something else found later or separately, they may be set anywhere in the function body as convenient, and the ret value is returned from one or more places.

Note that we NEVER write const in C, as this feature doesn't pull its weight.
There's some existing contamination around library functions but try to minimize the spread.

Sometimes we need a null-terminated C string for talking to library functions, we can use this pattern (with "2048" being replaced by some reasonably generous number appropriate to the specific use case): `char buf[2048] = {0}; s(buf,2048,some_span);`.
This: allocates (apparently on the stack, i.e. you don't have to think about freeing it) a nice buffer that's sufficiently large, copies the span contents into it or crashes your program if the buffer was too small, and then returns the safely null-terminated result (which was the first argument, but is also returned as a convenience).
This pattern makes it hard to pass a null-terminated string out of a function's scope, but that's perfect anyway, as we also minimize the spread of null-terminated strings.
Never assume anything is null-terminated.
Use spans everywhere.

Note that prt() has exactly the same function signature as printf, i.e. it takes a format string followed by varargs.
We never use printf, but always prt.
A common idiom when reporting errors is to call prt, flush, and exit.
We could also use flush, prt, flush_err, exit, but up to now we've been lazy about the distinction between stdout and stderr as we have mainly interactive use cases.

To prt a span x we use `%.*s` with len(x) and x.buf.

A common idiom is next_line() in a loop with !empty().

In main() or similar it is common to call init_spans and often also read_and_count_stdin.
*/
/* includes */

#define _GNU_SOURCE // for memmem
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <math.h>
/* convenient debugging macros */
#define dbgd(x) prt(#x ": %d\n", x),flush()
#define dbgx(x) prt(#x ": %x\n", x),flush()
#define dbgf(x) prt(#x ": %f\n", x),flush()
#define dbgp(x) prt(#x ": %p\n", x),flush()

typedef unsigned char u8;

/* #span

Our string type.

A span is two pointers.
Buf points to the first char included in the string.
End points to the first char excluded after the string's end.

If these two pointers are equal, the string is empty, but it still points to a location.

So two empty spans are not necessarily the same span, while two empty strings are.
Neither spans nor their contents are immutable, but everything depends on intended use.

These two pointers must point into some space that has been allocated somewhere.

The inp variable is the span which writes into input_space, and then is the immutable copy of stdin for the duration of the process.
The number of bytes of input is len(inp).
The output is stored in span out, which points to output_space.
Input processing is generally by reading out of the inp span or subspans of it.
The output spans are mostly written to with prt() and other IO functions.
The cmp_space and cmp span which points to it are used for analysis and model data, both reading and writing.
These are just the common conventions; your program may use them differently.

When writing output, we often see prt followed by flush.
Flush sends to stdout the contents of out (the output span) that have not already been sent.
Usually it is important to do this
- before any operation that blocks, when the user should see the output that we've already written,
- generally immediately after prt when debugging anything,
- after printing any error message and before exiting the program, and
- at the end of main before returning.

If you want to write to stderr, you can first do a flush, which will write any pending output to stdout, then do your prt and immediately flush_err() which also flushes from the output_space but to stderr.
*/

typedef struct {
  u8 *buf;
  u8 *end;
} span;

#define BUF_SZ (1 << 30)

u8 *input_space; // remains immutable once stdin has been read up to EOF.
u8 *output_space;
u8 *cmp_space;
span out, inp, cmp;

int empty(span);
int len(span);

void init_spans(); // main spanio init function

// basic spanio primitives

void prt2cmp();
void prt2std();
void prt(const char *, ...);
void w_char(char);
void wrs(span);
void bksp();
void sp();
void terpri();
void flush();
void flush_err();
void flush_to(char*);
void write_to_file(span content, const char* filename);
span read_file_into_span(char *filename, span buffer);
void redir(span);
span reset();
void w_char_esc(char);
void w_char_esc_pad(char);
void w_char_esc_dq(char);
void w_char_esc_sq(char);
void wrs_esc();
void save();
void push(span);
void pop(span*);
void advance1(span*);
void advance(span*,int);
span pop_into_span();
int find_char(span s, char c);
int contains(span, span);
span take_n(int, span*);
span next_line(span*);
span first_n(span, int);
int span_eq(span, span);
int span_cmp(span, span);
span S(char*);
span nullspan();

typedef struct {
  span *s; // array of spans (points into span arena)
  int n;   // length of array
} spans;

#define SPAN_ARENA_STACK 256

span* span_arena;
int span_arenasz;
int span_arena_used;
int span_arena_stack[SPAN_ARENA_STACK];
int span_arena_stack_n;

void span_arena_alloc(int);
void span_arena_free();
void span_arena_push();
void span_arena_pop();


/* input statistics on raw bytes; span basics */

int counts[256] = {0};

void read_and_count_stdin(); // populate inp and counts[]
int empty(span s) {
  return s.end == s.buf;
}


inline int len(span s) { return s.end - s.buf; }

u8 in(span s, u8* p) { return s.buf <= p && p < s.end; }

int out_WRITTEN = 0, cmp_WRITTEN = 0;

void init_spans() {
  input_space = malloc(BUF_SZ);
  output_space = malloc(BUF_SZ);
  cmp_space = malloc(BUF_SZ);
  out.buf = output_space;
  out.end = output_space;
  inp.buf = input_space;
  inp.end = input_space;
  cmp.buf = cmp_space;
  cmp.end = cmp_space;
}

void bksp() { out.end--; }

void sp() { w_char(' '); }

// we might have the same kind of redir_i() as we have redir() already, where we redirect input to come from a span and then use standard functions like take() and get rid of these special cases for taking input from streams or spans.

span head_n(int n, span *io) {
  span ret;
  ret.buf = io->buf;
  ret.end = io->buf + n;
  io->buf += n;
  return ret;
}

int span_eq(span s1, span s2) {
  if (len(s1) != len(s2)) return 0;
  for (int i = 0; i < len(s1); ++i) if (s1.buf[i] != s2.buf[i]) return 0;
  return 1;
}

int span_cmp(span s1, span s2) {
  for (;;) {
    if (empty(s1) && !empty(s2)) return 1;
    if (empty(s2) && !empty(s1)) return -1;
    if (empty(s1)) return 0;
    int dif = *(s1.buf++) - *(s2.buf++);
    if (dif) return dif;
  }
}

span S(char *s) {
  span ret = {(u8*)s, (u8*)s + strlen(s) };
  return ret;
}

char* s(char* buf, int n, span s) {
  size_t l = (n - 1) < len(s) ? (n - 1) : len(s);
  memmove(buf, s.buf, l);
  buf[l] = '\0';
  return buf;
}

void read_and_count_stdin() {
  int c;
  while ((c = getchar()) != EOF) {
    //if (c == ' ') continue;
    assert(c != 0);
    counts[c]++;
    *inp.buf = c;
    inp.buf++;
    if (len(inp) == BUF_SZ) exit(1);
  }
  inp.end = inp.buf;
  inp.buf = input_space;
}

span saved_out[16] = {0};
int saved_out_stack = 0;

void redir(span new_out) {
  assert(saved_out_stack < 15);
  saved_out[saved_out_stack++] = out;
  out = new_out;
}

span reset() {
  assert(saved_out_stack);
  span ret = out;
  out = saved_out[--saved_out_stack];
  return ret;
}

// set if debugging some crash
const int ALWAYS_FLUSH = 0;

void swapcmp() { span swap = cmp; cmp = out; out = swap; int swpn = cmp_WRITTEN; cmp_WRITTEN = out_WRITTEN; out_WRITTEN = swpn; }
void prt2cmp() { /*if (out.buf == output_space)*/ swapcmp(); }
void prt2std() { /*if (out.buf == cmp_space)*/ swapcmp(); }

void prt(const char * fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char *buffer;
  // we used to use vsprintf here, but that adds a null byte that we don't want
  int n = vasprintf(&buffer, fmt, ap);
  memcpy(out.end, buffer, n);
  free(buffer);
  out.end += n;
  if (out.buf + BUF_SZ < out.end) {
    printf("OUTPUT OVERFLOW (%ld)\n", out.end - output_space);
    exit(7);
  }
  va_end(ap);
  if (ALWAYS_FLUSH) flush();
}

void terpri() {
  *out.end = '\n';
  out.end++;
  if (ALWAYS_FLUSH) flush();
}

void w_char(char c) {
  *out.end++ = c;
}

void w_char_esc(char c) {
  if (c < 0x20 || c == 127) {
    out.end += sprintf((char*)out.end, "\\%03o", (u8)c);
  } else {
    *out.end++ = c;
  }
}

void w_char_esc_pad(char c) {
  if (c < 0x20 || c == 127) {
    out.end += sprintf((char*)out.end, "\\%03o", (u8)c);
  } else {
    sp();sp();sp();
    *out.end++ = c;
  }
}

void w_char_esc_dq(char c) {
  if (c < 0x20 || c == 127) {
    out.end += sprintf((char*)out.end, "\\%03o", (u8)c);
  } else if (c == '"') {
    *out.end++ = '\\';
    *out.end++ = '"';
  } else if (c == '\\') {
    *out.end++ = '\\';
    *out.end++ = '\\';
  } else {
    *out.end++ = c;
  }
}

void w_char_esc_sq(char c) {
  if (c < 0x20 || c == 127) {
    out.end += sprintf((char*)out.end, "\\%03o", (u8)c);
  } else if (c == '\'') {
    *out.end++ = '\\';
    *out.end++ = '\'';
  } else if (c == '\\') {
    *out.end++ = '\\';
    *out.end++ = '\\';
  } else {
    *out.end++ = c;
  }
}

void wrs(span s) {
  for (u8 *c = s.buf; c < s.end; c++) w_char(*c);
}

void wrs_esc(span s) {
  for (u8 *c = s.buf; c < s.end; c++) w_char_esc(*c);
}

void flush() {
  if (out_WRITTEN < len(out)) {
    printf("%.*s", len(out) - out_WRITTEN, out.buf + out_WRITTEN);
    out_WRITTEN = len(out);
    fflush(stdout);
  }
}

void flush_err() {
  if (out_WRITTEN < len(out)) {
    fprintf(stderr, "%.*s", len(out) - out_WRITTEN, out.buf + out_WRITTEN);
    out_WRITTEN = len(out);
    fflush(stderr);
  }
}

void flush_to(char *fname) {
  int fd = open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0666);
  dprintf(fd, "%*s", len(out) - out_WRITTEN, out.buf + out_WRITTEN);
  //out_WRITTEN = len(out);
  // reset for constant memory usage
  out_WRITTEN = 0;
  out.end = out.buf;
  //fsync(fd);
  close(fd);
}

   /*
In write_to_file we open a file, which must not exist, and write the contents of a span into it, and close it.
If the file exists or there is any other error, we prt(), flush(), and exit as per usual.

In write_to_file_span we simply take the same two arguments but the filename is a span.
We build a null-terminated string and call write_to_file.
*/

void write_to_file_2(span, const char*, int);

void write_to_file(span content, const char* filename) {
  write_to_file_2(content, filename, 0);
}

void write_to_file_2(span content, const char* filename, int clobber) {
  // Attempt to open the file with O_CREAT and O_EXCL to ensure it does not already exist
  /* clobber thing is a manual fixup */
  int flags = O_WRONLY | O_CREAT | O_TRUNC;
  if (!clobber) flags |= O_EXCL;
  int fd = open(filename, flags, 0644);
  if (fd == -1) {
    if (clobber) {
      prt("Error opening %s for writing: File cannot be created or opened.\n", filename);
    } else {
      prt("Error opening %s for writing: File already exists or cannot be created.\n", filename);
    }
    flush();
    exit(EXIT_FAILURE);
  }

  // Write the content of the span to the file
  ssize_t written = write(fd, content.buf, len(content));
  if (written != len(content)) {
    // Handle partial write or write error
    prt("Error writing to file %s.\n", filename);
    flush();
    close(fd); // Attempt to close the file before exiting
    exit(EXIT_FAILURE);
  }

  // Close the file
  if (close(fd) == -1) {
    prt("Error closing %s after writing.\n", filename);
    flush();
    exit(EXIT_FAILURE);
  }
}

void write_to_file_span(span content, span filename_span, int clobber) {
  char filename[filename_span.end - filename_span.buf + 1];
  memcpy(filename, filename_span.buf, filename_span.end - filename_span.buf);
  filename[filename_span.end - filename_span.buf] = '\0';
  write_to_file_2(content, filename, clobber);
}

// Function to print error messages and exit (never write "const" in C)
void exit_with_error(char *error_message) {
  perror(error_message);
  exit(EXIT_FAILURE);
}

span read_file_S_into_span(span filename, span buffer) {
  char path[2048];
  s(path,2048,filename);
  return read_file_into_span(path, buffer);
}

span read_file_into_span(char* filename, span buffer) {
  // Open the file
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    prt("Failed to open %s\n", filename);
    flush();
    exit(1);
  }

  // Get the file size
  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1) {
    close(fd);
    prt("Failed to get file size for %s\n", filename);
    flush();exit(1);
  }

  // Check if the file's size fits into the provided buffer
  size_t file_size = statbuf.st_size;
  if (file_size > len(buffer)) {
    close(fd);
    exit_with_error("File content does not fit into the provided buffer");
  }

  // Read file contents into the buffer
  ssize_t bytes_read = read(fd, buffer.buf, file_size);
  if (bytes_read == -1) {
    close(fd);
    exit_with_error("Failed to read file contents");
  }

  // Close the file
  if (close(fd) == -1) {
    exit_with_error("Failed to close file");
  }

  // Create and return a new span that reflects the read content
  span new_span = {buffer.buf, buffer.buf + bytes_read};
  return new_span;
}

u8 *save_stack[16] = {0};
int save_count = 0;

void save() {
  push(out);
}

span pop_into_span() {
  span ret;
  ret.buf = save_stack[--save_count];
  ret.end = out.end;
  return ret;
}

void push(span s) {
  save_stack[save_count++] = s.buf;
}

void pop(span *s) {
  s->buf = save_stack[--save_count];
}

   /*
take_n is a mutating function which takes the first n chars of the span into a new span, and also modifies the input span to remove this same prefix.
After a function call such as `span new = take_n(x, s)`, it will be the case that `new` contatenated with `s` is equivalent to `s` before the call.
*/

span take_n(int n, span *io) {
  span ret;
  ret.buf = io->buf;
  ret.end = io->buf + n;
  io->buf += n;
  return ret;
}

void advance1(span *s) {
    if (!empty(*s)) s->buf++;
}

void advance(span *s, int n) {
    if (len(*s) >= n) s->buf += n;
    else s->buf = s->end; // Move to the end if n exceeds span length
}

int contains(span haystack, span needle) {
  /*
  prt("contains() haystack:\n");
  wrs(haystack);terpri();
  prt("needle:\n");
  wrs(needle);terpri();
  */
  if (len(haystack) < len(needle)) {
    return 0; // Needle is longer, so it cannot be contained
  }
  void *result = memmem(haystack.buf, haystack.end - haystack.buf, needle.buf, needle.end - needle.buf);
  return result != NULL ? 1 : 0;
}

int contains_ptr(span a, span b) {
  return a.buf <= b.buf && b.end <= a.end;
}

int starts_with(span a, span b) {
  return len(b) <= len(a) && 0 == memcmp(a.buf, b.buf, len(b));
}

span first_n(span s, int n) {
  span ret;
  if (len(s) < n) n = len(s); // Ensure we do not exceed the span's length
  ret.buf = s.buf;
  ret.end = s.buf + n;
  return ret;
}

span skip_n(span s, int n) {
  if (len(s) <= n) return (span){s.end, s.end};
  return (span){s.buf + n, s.end};
}

void skip_whitespace(span *s) {
  while (isspace(*s->buf)) s->buf++;
}

int find_char(span s, char c) {
  for (int i = 0; i < len(s); ++i) {
    if (s.buf[i] == c) return i;
  }
  return -1; // Character not found
}

span trim(span s) {
  while (len(s) && isspace((unsigned char)*s.buf)) s.buf++;
  while (len(s) && isspace((unsigned char)*(s.end - 1))) s.end--;
  return s;
}
/* next_line(span*) shortens the input span and returns the first line as a new span.
The newline is consumed and is not part of either the returned span or the input span after the call.
I.e. the total len of the shortened input and the returned line is one less than the len of the original input.
If there is no newline found, then the entire input is returned.
In this case the input span is mutated such that buf now points to end.
This makes it an empty span and thus a null span in our nomenclature, but it is still an empty span at a particular location.
This convention of empty but localized spans allows us to perform comparisons without needing to handle them differently in the case of an empty span.
*/

span next_line(span *input) {
  if (empty(*input)) return nullspan();
  span line;
  line.buf = input->buf;
  while (input->buf < input->end && *input->buf != '\n') {
    input->buf++;
  }
  line.end = input->buf;
  if (input->buf < input->end) { // If '\n' found, move past it for next call
    input->buf++;
  }
  return line;
}

/* 
In consume_prefix(span*,span) we are given a span which is typically something being parsed and another span which is expected to be a prefix of it.
If the prefix is found, we return it and modify the span that is being parsed to remove the prefix.
Otherwise we leave that span unmodified and return nullspan().
Typical use is in an if statement to either identify and consume some prefix and then continue on to handle what follows it, or otherwise to skip the if and continue parsing the unmodified input.
We return the span that points into the input in case the caller has some use for it.
*/

span consume_prefix(span prefix, span *input) {
  if (len(*input) < len(prefix) || !span_eq(first_n(*input, len(prefix)), prefix)) {
    return nullspan();
  }
  span ret = {buf: input->buf};
  input->buf += len(prefix);
  ret.end = input->buf;
  return ret;
}
/*
The spans arena implementation.
*/

spans spans_alloc(int n);
int bool_neq(int, int);
span spanspan(span haystack, span needle);
int is_one_of(span x, spans ys);

void span_arena_alloc(int sz) {
  span_arena = malloc(sz * sizeof *span_arena);
  span_arenasz = sz;
  span_arena_used = 0;
  span_arena_stack_n = 0;
}
void span_arena_free() {
  free(span_arena);
}
void span_arena_push() {
  assert(span_arena_stack_n < SPAN_ARENA_STACK);
  span_arena_stack[span_arena_stack_n++] = span_arena_used;
}
void span_arena_pop() {
  assert(0 < span_arena_stack_n);
  span_arena_used = span_arena_stack[--span_arena_stack_n];
}
spans spans_alloc(int n) {
  assert(span_arena);
  spans ret = {0};
  ret.s = span_arena + span_arena_used;
  ret.n = n;
  span_arena_used += n;
  assert(span_arena_used < span_arenasz);
  return ret;
}

/* Generic arrays.

Here we have a macro that we can call with two type names (i.e. typedefs) and a number.
One is an already existing typedef and another will be created by the macro, and the number is the size of a stack, described below.

For example, to create the spans type we might call this macro with span and spans as the names.
We call these the element type and array type names resp.
(We may use "E" and "T" as variables in documentation to refer to them.)

This macro will create a typedef struct with that given name that has a pointer to the element type called "a", a number of elements, which is always called "n", and a capacity "cap", which are size_t's.

We don't implement realloc for simplicity and improved memory layout.
We use an arena allocation pattern.
Because we don't realloc, a common pattern is to make an initial loop over something to count some required number of elements and then to call the alloc function to get an array of the precise size and then to have a second loop where we populate that array.

For every generic array type that we make, we will have:

- A setup function T_arena_alloc(N) for the arena, which takes a number (as int) and allocates (using malloc) enough memory for that many of the element type, where T is the array type name.
- A corresponding T_arena_free().
- A pair T_arena_push() and T_arena_pop().
- A function T_alloc(N) which returns T, always having "n" already set.
- T_push(E) which increments n, complains and exits if cap is reached, and stores the element provided.

The implementation makes a single global struct (both the typedef and the singleton instance) that holds the arena state for the array type.
This includes the arena pointer, the arena size in elements, the number of allocated elements, and a stack of such numbers.
The stack size is also an argument to the macro.
We do not support realloc on the entire arena, rather the programmer needs to choose a big enough value and if we exceed at runtime we will always crash.
The programmer has to call the T_arena_alloc(N) and _free methods themselves, usually in a main() function or similar, and if the function is not called the arena won't be initialized and T_alloc() will always complain and crash (using prt, flush, exit as usual).

The main entry point is the MAKE_ARENA(E,T) macro, which sets up everything and must be called before any references to T in the source code.
Then the arena alloc and free functions must be called somewhere, and everything is ready to use.
*/

#define MAKE_ARENA(E, T, STACK_SIZE) \
typedef struct { \
    E* a; \
    size_t n, cap; \
} T; \
\
static struct { \
    E* arena; \
    size_t arena_size, allocated, stack[STACK_SIZE], stack_top; \
} T##_arena = {0}; \
\
void T##_arena_alloc(int N) { \
    T##_arena.arena = (E*)malloc(N * sizeof(E)); \
    if (!T##_arena.arena) { \
        prt("Failed to allocate arena for " #T "\n", 0); \
        flush(); \
        exit(1); \
    } \
    T##_arena.arena_size = N; \
    T##_arena.allocated = 0; \
    T##_arena.stack_top = 0; \
} \
\
void T##_arena_free() { \
    free(T##_arena.arena); \
} \
\
void T##_arena_push() { \
    if (T##_arena.stack_top == STACK_SIZE) { \
        prt("Exceeded stack size for " #T "\n", 0); \
        flush(); \
        exit(1); \
    } \
    T##_arena.stack[T##_arena.stack_top++] = T##_arena.allocated; \
} \
\
void T##_arena_pop() { \
    if (T##_arena.stack_top == 0) { \
        prt("Stack underflow for " #T "\n", 0); \
        flush(); \
        exit(1); \
    } \
    T##_arena.allocated = T##_arena.stack[--T##_arena.stack_top]; \
} \
\
T T##_alloc(size_t N) { \
    if (T##_arena.allocated + N > T##_arena.arena_size) { \
        prt("Arena overflow for " #T "\n", 0); \
        flush(); \
        exit(1); \
    } \
    T result; \
    result.a = T##_arena.arena + T##_arena.allocated; \
    result.n = N; \
    result.cap = N; \
    T##_arena.allocated += N; \
    return result; \
} \
\
void T##_push(T* t, E e) { \
    if (t->n == t->cap) { \
        prt("Capacity reached for " #T "\n", 0); \
        flush(); \
        exit(1); \
    } \
    t->a[t->n++] = e; \
}
/*
Other stuff.
*/

span nullspan() {
  return (span){0, 0};
}

int bool_neq(int a, int b) { return ( a || b ) && !( a && b); }

/* #json

JSON library

*/

typedef struct {
  span s;
} json;

int json_is_null(json);

// constructors
json json_s(span);
json json_n(double);
json json_b(int);
json json_0();
json json_o();
json json_a();
json nulljson();

// inverse
span json_s2s(json,span*,u8*);

// extend
void json_o_extend(json*,span,json);
void json_a_extend(json*,json);

// predicates
int json_sp(json);
int json_np(json);
int json_bp(json);
int json_0p(json);
int json_op(json);
int json_ap(json);

// lookups
json json_key(span, json);
json json_index(int, json);

// from spans
json json_parse(span);
json make_json(span);
json json_parse_prefix(span*);
json json_parse_prefix_string(span*);
json json_parse_prefix_number(span*);
json json_parse_prefix_littok(span*);

// implementation

int json_is_null(json j) { return !j.s.buf; }

json json_s(span s) {
  json ret = {0};
  ret.s.buf = out.end;
  prt("\"");
  for (u8* p=s.buf;p<s.end;p++) {
    switch (*p) {
      case '\b':
        prt("\\b");
      case '\f':
        prt("\\f");
      case '\n':
        prt("\\n");
        break;
      case '\r':
        prt("\\r");
      case '\t':
        prt("\\t");
      case '"':
        prt("\\\"");
        break;
      case '\\':
        prt("\\\\");
        break;
      default:
        if (iscntrl(*p)) {
          prt("\\u%04X", *p);
        }
        w_char(*p);
    }
  }
  prt("\"");
  ret.s.end = out.end;
  return ret;
}

json json_n(double n) {
  json ret = {s: {buf: out.end }};
  prt("%F", n);
  ret.s.end = out.end;
  return ret;
}

json json_b(int b) {
  json ret = {s: {buf: out.end }};
  if (b) prt("true"); else prt("false");
  ret.s.end = out.end;
  return ret;
}

json json_0() {
  json ret = {s: {buf: out.end }};
  prt("null");
  ret.s.end = out.end;
  return ret;
}

json json_o() {
  json ret = {s: {buf: out.end }};
  prt("{}");
  ret.s.end = out.end;
  return ret;
}

void json_o_extend(json *j, span key, json val) {
  u8* keybuf = malloc(len(key));
  u8* valbuf = malloc(len(val.s));
  memcpy(keybuf, key.buf, len(key));
  memcpy(valbuf, val.s.buf, len(val.s));
  span key2 = {keybuf, keybuf + len(key)};
  span val2 = {valbuf, valbuf + len(val.s)};
  out.end = j->s.end;
  bksp();
  if (*(out.end - 1) != '{') prt(",");
  //wrs(key2);
  json_s(key2);
  prt(":");
  wrs(val2);
  prt("}");
  j->s.end = out.end;
  free(keybuf);
  free(valbuf);
}

json json_a() {
  json ret = { s: {buf: out.end }};
  prt("[]");
  ret.s.end = out.end;
  return ret;
}

void json_a_extend(json *a, json val) {
  out.end = a->s.end;
  bksp();
  if (*(out.end - 1) != '[') prt(",");
  wrs(val.s);
  prt("]");
  a->s.end = out.end;
}

json nulljson() { return (json) {nullspan()}; }

int json_sp(json j) { return j.s.buf && *j.s.buf == '"'; }
int json_np(json j) {
  if (!j.s.buf) return 0;
  switch(*j.s.buf) {
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return 1;
    default:
      return 0;
  }
}
int json_bp(json j) { return j.s.buf && (*j.s.buf == 't' || *j.s.buf == 'f'); }
int json_0p(json j) { return j.s.buf && *j.s.buf == 'n'; }
int json_op(json j) { return j.s.buf && *j.s.buf == '{'; }
int json_ap(json j) { return j.s.buf && *j.s.buf == '['; }

json json_index(int n, json a) {
  json ret = {0};
  a.s.buf++;
  while (*a.s.buf != ']') {
    skip_whitespace(&a.s);
    ret = json_parse_prefix(&a.s);
    if (json_is_null(ret)) return nulljson();
    if (!n--) return ret;
    skip_whitespace(&a.s);
    if (*a.s.buf != ',') return nulljson();
    a.s.buf++;
  };
  return nulljson();
}

json json_key(span s, json o) {
  o.s.buf++;
  while (*o.s.buf != '}') {
    skip_whitespace(&o.s);
    json key = json_parse_prefix(&o.s);
    if (json_is_null(key)) return key;
    skip_whitespace(&o.s);
    if (*(o.s.buf++) != ':') return nulljson();
    skip_whitespace(&o.s);
    json value = json_parse_prefix(&o.s);
    if (json_is_null(value)) return nulljson();
    span key_s = json_s2s(key, &cmp, cmp_space + BUF_SZ);
    if (span_eq(key_s, s)) return value;
    skip_whitespace(&o.s);
    if (*(o.s.buf++) != ',') return nulljson();
    skip_whitespace(&o.s);
  }
}

json make_json(span s) { return (json){s}; }

/*
json_parse returns a json if the span contains valid JSON with whitespace stripped
otherwise it returns a null json

ChatGPT4's idea, but see next block:

1. **Function Prototype**:
   The `json_parse` function will take a `span` representing the JSON data and will return a `json` type object. The exact structure of `json` isn't defined here, so I'll assume a generic structure that can handle JSON primitives (strings, numbers, booleans, null) and structures (arrays, objects).

2. **Error Handling**:
   Given the lack of exceptions in C, error handling should be robust, returning a nullable `json` (possibly `nullspan()` to indicate parsing failure). This will allow us to handle malformed input gracefully.

3. **Buffer Management**:
   Ensure that all operations adhere to the span-oriented operations, minimizing reliance on null-terminated strings unless interfacing with external libraries that require them.

4. **Incremental Parsing**:
   Implement a state machine or a recursive descent parser to handle JSON's hierarchical structure. This would involve functions to parse objects, arrays, strings, numbers, and other primitives. The parser should be able to handle nested objects and arrays recursively or iteratively.

5. **Whitespace and Format Handling**:
   As JSON allows for whitespace between tokens, the parser should correctly skip whitespace where appropriate. Utilize `trim()` to manage spaces around tokens.

6. **Utilities and Helpers**:
   Develop helper functions such as `parse_number`, `parse_string`, `parse_array`, and `parse_object`. These functions would be called as needed when the main parsing function encounters the corresponding opening characters ('{', '[', etc.).

7. **Unicode and Escaping**:
   JSON string parsing must handle escaped characters and unicode symbols correctly. Implement or utilize existing functions like `w_char_esc` to decode these sequences.

8. **Performance Considerations**:
   Since JSON parsing can be computationally intensive, especially for large strings, ensure that the parser is optimized for speed and memory usage, perhaps by using span manipulations effectively to avoid unnecessary copying.

9. **Integration with Span Functions**:
   Utilize span-related functions like `find_char`, `next_line`, and `contains` to implement parsing logic, ensuring that the parser remains consistent with the `spanio` library's methodology.

10. **Testing**:
   Implement rigorous unit tests to cover typical JSON structures as well as edge cases like deeply nested objects, large arrays, and malformed JSON to ensure that the parser is robust.

Here’s a skeletal structure for the function:

```c
json json_parse(span input) {
    trim(input);  // Remove leading/trailing whitespace
    if (empty(input)) {
        return nullspan();  // Return null json for empty input
    }

    // Depending on the first non-space character, call the appropriate function
    switch (*input.buf) {
        case '{':
            return parse_object(input);
        case '[':
            return parse_array(input);
        case '\"':
            return parse_string(input);
        case 't': case 'f':  // true or false
            return parse_boolean(input);
        case 'n':  // null
            return parse_null(input);
        default:
            if (isdigit(*input.buf) || *input.buf == '-' || *input.buf == '+') {
                return parse_number(input);
            }
            break;
    }

    return nullspan();  // Return null if none of the valid JSON starting characters are found
}
```
*/
/*
The `json_parse` function takes a `span` representing the JSON data and returns a `json` type object, unless the parse failed or did not consume the entire input (excepting whitespace) in which case it returns nulljson().
*/

json json_parse(span s) {
  skip_whitespace(&s);
  json ret = json_parse_prefix(&s);
  skip_whitespace(&s);
  if (empty(s)) return ret;
  return nulljson();
}
/*
The json_parse_prefix function takes a span and parses as much of it as it can as a json, then leaves the rest, and returns a json which is null only if the parse failed.

In fact, the span is passed in by reference and we modify it, shortening it from the front.
If the parse fails the input span may be modified.

We declare a json return value `ret`.
This will include the first non-whitespace byte that we consume up to the end of the complete JSON value by the time we return it, or we will indicate failure and not return it.

First we strip any whitespace by calling skip_whitespace on the input span.
Then we set ret.s.buf from the input as if we succeed this is the only value it can have.
After this point, if we return successfully we will always set ret.s.end to be the same as .buf of the input span when we are returning.
I.e. the json that we return always covers the prefix that we have parsed up to what is left in the input.

Then we switch on the first non-ws char of the input, and then we either:

- call json_parse_prefix_string
- call json_parse_prefix_number
- directly parse a true/false/null
- directly parse an object or array

To directly parse an object, we first consume the "{", then call json_parse_prefix_string.
If this returns a null json it means we failed and we return the null json.
Otherwise we continue by skipping whitespace, consuming the ":" and then recursively calling json_parse_prefix to consume the value.
Once again, if the value is the null json then we failed and return the null json.
Otherwise, we consume (whitespace and) either another comma, going around the loop again, we exit the loop, and then outside that consume (any whitespace and) the final "}" and return successfully.

To directly parse an array we do something similar but without the keys, just handling the commas and values.

To directly parse true/false/null, we call consume_prefix with the appropriate string.
This returns a span, which we put in a variable; if it is the null span we return nulljson().
Otherwise, it will be a span pointing into the span that we are parsing, for this reason we return the same span returned from consume_prefix, just wrapping it with a call to make_json first.
*/

json json_parse_prefix(span *input) {
    json ret = {0};
    //skip_whitespace(input);
    ret.s.buf = input->buf;

    char first_char = *input->buf;
    switch (first_char) {
        case '\"':
            ret = json_parse_prefix_string(input);
            break;
        case '-':
        case '0' ... '9':
            ret = json_parse_prefix_number(input);
            break;
        case 't':
        case 'f':
        case 'n':
            ret = json_parse_prefix_littok(input);
            break;
        case '{':
            input->buf++; // consume '{'
            skip_whitespace(input);
            while (*input->buf != '}') {
                json key = json_parse_prefix_string(input);
                if (key.s.buf == NULL) return nulljson();
                skip_whitespace(input);
                if (*input->buf != ':') return nulljson();
                input->buf++; // consume ':'
                skip_whitespace(input);
                json value = json_parse_prefix(input);
                if (value.s.buf == NULL) return nulljson();
                skip_whitespace(input);
                if (*input->buf == ',') input->buf++; // consume ','
                skip_whitespace(input);
            }
            if (*input->buf == '}') input->buf++; // consume '}'
            else return nulljson();
            break;
        case '[':
            input->buf++; // consume '['
            skip_whitespace(input);
            while (*input->buf != ']') {
                json value = json_parse_prefix(input);
                if (value.s.buf == NULL) return nulljson();
                skip_whitespace(input);
                if (*input->buf == ',') input->buf++; // consume ','
                skip_whitespace(input);
            }
            if (*input->buf == ']') input->buf++; // consume ']'
            else return nulljson();
            break;
        default:
            return nulljson();
    }
    ret.s.end = input->buf;
    return ret;
}

/*

In json_s2s we get a json with .s being a JSON string, and a span pointer to a buffer area, which we will extend, and a max u8* giving the end of the buffer region.

First we assert that the string starts with the double quote, which we advance past.

Then we iterate over the input until it ends or we reach the closing quote.

We unescape the input into the buffer, advancing the end of the buffer.

We handle all the escaping in JSON strings.

Specifically, we can see a backslash followed by:

- b,f,n,r,t
- ",\,/
- u followed by four hex digits with either A-F or a-f

In every case we write the unescaped character into the buffer, advancing .end, and we also copy all non-escaped characters over directly.
In the case of \u we encode as UTF-8.

Finally we return a span covering the area that .end advanced over.
I.e. the length of the returned span is also the length that was added to the buffer span.

If we would ever advance the buf past the max we also crash the program (prt, flush, exit).
*/

// Utility to convert a hex digit to its integer value
int hex_to_int(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + c - 'a';
    if ('A' <= c && c <= 'F') return 10 + c - 'A';
    return -1; // Error case, should never happen if input is correct
}

// Function to parse unicode sequence and write as UTF-8
void write_utf8_from_hex(u8 **buf, char *hex) {
    int codepoint = (hex_to_int(hex[0]) << 12) | (hex_to_int(hex[1]) << 8) |
                    (hex_to_int(hex[2]) << 4) | hex_to_int(hex[3]);
    if (codepoint < 0x80) {
        *(*buf)++ = codepoint;
    } else if (codepoint < 0x800) {
        *(*buf)++ = 192 + (codepoint >> 6);
        *(*buf)++ = 128 + (codepoint & 63);
    } else if (codepoint < 0x10000) {
        *(*buf)++ = 224 + (codepoint >> 12);
        *(*buf)++ = 128 + ((codepoint >> 6) & 63);
        *(*buf)++ = 128 + (codepoint & 63);
    } else {
        *(*buf)++ = 240 + (codepoint >> 18);
        *(*buf)++ = 128 + ((codepoint >> 12) & 63);
        *(*buf)++ = 128 + ((codepoint >> 6) & 63);
        *(*buf)++ = 128 + (codepoint & 63);
    }
}

span json_s2s(json j, span *buffer, u8 *max) {
    u8 *buf = buffer->end;
    span ret = { buf, buf };

    if (*j.s.buf != '\"') {
        prt("Expected starting quote in JSON string\n");
        flush();
        exit(1);
    }

    for (u8 *s = j.s.buf + 1; s < j.s.end && *s != '\"'; s++) {
        if (buf >= max) {
            prt("Buffer overflow detected\n");
            flush();
            exit(1);
        }
        if (*s == '\\') {
            s++;
            switch (*s) {
                case 'b': *buf++ = '\b'; break;
                case 'f': *buf++ = '\f'; break;
                case 'n': *buf++ = '\n'; break;
                case 'r': *buf++ = '\r'; break;
                case 't': *buf++ = '\t'; break;
                case '\"': case '\\': case '/': *buf++ = *s; break;
                case 'u':
                    if (s + 4 >= j.s.end) {
                        prt("Incomplete unicode escape in JSON string\n");
                        flush();
                        exit(1);
                    }
                    write_utf8_from_hex(&buf, (char *)(s + 1));
                    s += 4;
                    break;
                default:
                    prt("Unknown escape sequence in JSON string\n");
                    flush();
                    exit(1);
            }
        } else {
            *buf++ = *s;
        }
    }
    ret.end = buf;
    buffer->end = buf;
    return ret;
}

/*
In json_parse_prefix_string we consume a JSON string from the input span and wrap it as a json.

We handle all the escaping in JSON strings.

Specifically, we can see a backslash followed by:

- b,f,n,r,t
- ",\,/
- u followed by four hex digits with either A-F or a-f

Here we just consume the initial and final double quotes, parse all the escaping to be sure it is a valid JSON string, and return a json with .s that points to the string (including the quotes) or nulljson() if there is any parsing error.
If we return successfully ret.s.end and input->buf will be equal at the end.
*/

json json_parse_prefix_string(span *input) {
    if (empty(*input) || *input->buf != '\"') return nulljson();
    advance1(input);
    span start = *input;
    while (!empty(*input) && *input->buf != '\"') {
        if (*input->buf == '\\') {
            advance1(input);
            if (empty(*input)) return nulljson();
            if (*input->buf == 'u') {
                for (int i = 0; i < 4; i++) {
                    advance1(input);
                    if (empty(*input) || !isxdigit(*input->buf)) return nulljson();
                }
            } else if (strchr("bfnrt\"\\/", *input->buf) == NULL) {
                return nulljson();
            }
        }
        advance1(input);
    }
    if (empty(*input)) return nulljson();
    advance1(input);
    return make_json((span){start.buf - 1, input->buf});
}

/*
In json_parse_prefix_number, we handle the JSON number format, namely:

We either return nulljson() if we could not parse the number for any reason or a json which includes the number that was parsed.

Manually written.
*/

json json_parse_prefix_number(span *input) {
  json ret = {0};
  ret.s.buf = input->buf;

  if (*input->buf == '-') advance1(input);

  if (!isdigit(*input->buf)) return nulljson();
  while (isdigit(*input->buf)) input->buf++;

  if (*input->buf == '.') {
    advance1(input);
    if (!isdigit(*input->buf)) return nulljson();
    while(isdigit(*input->buf)) input->buf++;
  }
  if (*input->buf == 'e' || *input->buf == 'E') {
    advance1(input);
    if (*input->buf == '+' || *input->buf == '-') {
      advance1(input);
    }
    if (!isdigit(*input->buf)) return nulljson();
    while (isdigit(*input->buf)) {
      advance1(input);
    }
  }

  ret.s.end = input->buf;
  return ret;
}
/*
Manually written for now.
*/

json json_parse_prefix_littok(span *input) {
  span inner;
  if (!empty(inner = consume_prefix(S("true"), input))) return (json){inner};
  if (!empty(inner = consume_prefix(S("false"), input))) return (json){inner};
  if (!empty(inner = consume_prefix(S("null"), input))) return (json){inner};
  return nulljson();
}
/* 
We do not use null-terminated strings but instead rely on the explicit end point of our span type.
Here we have spanspan(span,span) which is equivalent to strstr or memmem in the C library but for spans rather than C strings or void pointers respectively.
We implement spanspan with memmem under the hood so we get the same performance.
Like strstr or memmem, the arguments are in haystack, needle order, so remember to call spanspan with the thing you are looking for as the second arg.
We return either the empty span at the end of haystack or the span pointing to the first location of needle in haystack.
If needle is empty we return the empty span at the beginning of haystack.
(This allows you to distinguish between a non-found needle and an empty needle, as both give an empty match.)
Maybe we should actually return the nullspan here; considering the obvious extension to regexes or more powerful patterns, the difference between a matching empty span and a non-match would become significant.
Examples:

spanspan "abc" "b" -> "b"
spanspan "abc" "x" -> "" (located after "c")
spanspan "abc" ""  -> "" (located at "a")
*/

span spanspan(span haystack, span needle) {
  if (empty(needle)) return (span){haystack.buf, haystack.buf};

  if (len(needle) > len(haystack)) return nullspan();

  void *result = memmem(haystack.buf, len(haystack), needle.buf, len(needle));

  if (!result) return (span){haystack.end, haystack.end};

  return (span){result, result + len(needle)};
}

// Checks if a given span is contained in a spans.
// Returns 1 if found, 0 otherwise.
// Actually a more useful function would return an index or -1, so we don't need another function when we care where the thing is.
int is_one_of(span x, spans ys) {
  for (int i = 0; i < ys.n; ++i) {
    if (span_eq(x, ys.s[i])) {
      return 1; // Found
    }
  }
  return 0; // Not found
}

/*
Library function inp_compl() returns a span that is the complement of inp in the input_space.
The input space is defined by the pointer input_space and the constant BUF_SZ.
As the span inp always represents the content of the input (which has been written so far, for example by reading from stdin), the complement of inp represents the portion of the input space after inp.end which has not yet been written to.

We have cmp_compl() and out_compl() methods which do the analogous operation for the respective spaces.
*/

span inp_compl() {
  span compl;
  compl.buf = inp.end;
  compl.end = input_space + BUF_SZ;
  return compl;
}

span cmp_compl() {
  span compl;
  compl.buf = cmp.end;
  compl.end = cmp_space + BUF_SZ;
  return compl;
}

span out_compl() {
  span compl;
  compl.buf = out.end;
  compl.end = output_space + BUF_SZ;
  return compl;
}

