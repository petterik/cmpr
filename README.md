# CMPr

## Program in English!

People started out programming machines by directly writing the machine code, then we got assemblers and compilers, and we could write high-level languages instead.
Now we have LLMs, and the LLM is the new compiler.
We can write English and let the LLM turn that into code.
However, the programmer still has to be closely involved in the work, specifying the approach and checking the code.

## Key ideas

- By 2030, most code will be written by LLMs.
- However, we may simply change the meaning of "code".
- Expert programming experience is more valuable, not less in this new world.
- Programming in English, just like learning any new programming language, has a learning curve.
- Unexpected benefits include maintainability, security, and improved communication.

## What's this then?

This is a tool to support the LLM programming workflow.

Today we support ChatGPT, and interaction is by copy and paste.
This means you can try it with a free account.
If you have a paid account you can use GPT4, which writes better code.

(Coming soon: API usage, local models, competing LLMs, etc.)

This is a framework and represents a particular and opinionated approach to this workflow, and we are updating as we learn.
The first working version of cmpr was written in a day, and the first broadly useful version in a week, using the workflow itself.

## Blocks

All C code here is by ChatGPT4, and all the English is by the human authors, which captures the essential idea of the workflow:

- Code is organized in "blocks," a block has a comment on top and code below it.
- You write the comment; the LLM writes the code.
- You iterate on the comment until the code works and meets your standard.

The cmpr tool itself runs on your machine and supports this workflow.
You use cmpr to navigate and search and manage your blocks, edit them in an editor, and get the LLM to rewrite the code parts of each block.
You can use vi-like keybindings (j/k to navigate blocks, "/" for search, etc).
The tool also manages revisions, saving a copy every time you edit.

You can use another IDE or editor alongside cmpr if you like, or adopt cmpr as your primary tool to interact with your code.
Your code still lives in files as usual; blocks are just segments of a file (specific details depend on the programming language).

To get the LLM involved, when you're on a block you hit "r" and this puts a prompt into the clipboard for you.
(You won't see anything happen when you hit "r", but the clipboard has been updated.)
Then you switch over to your ChatGPT window, paste and hit Enter.
ChatGPT writes the code, you can click "copy code" in the ChatGPT window, and then hit "R" (uppercase) back in cmpr to replace the code part of the block with the clipboard contents.
Mnemonic: "r" gets the LLM to "rewrite" (or "replace") the code to match the comment (and "R" is the opposite of "r").
Then you can hit "B" (for "build") to compile your code, run your tests, or whatever you want, and iterate.
Try "?" for a short help on keyboard shortcuts and run "cmpr --help" for command-line options.

## Why blocks?

The block is the basic unit of interaction with the LLM.
We start with a "bootstrap block" that includes basic information about our codebase, libraries we're using, and so on (the stuff that a human programmer would learn from a readme file or onboarding resources).

Current state-of-the-art LLMs like GPT4 can reliably generate a moderately-sized function from an English description that's sufficiently precise, along with this background information.
However, if you attempted to describe an entire large and complex program and get the all the code from a single prompt, today's LLMs will not be able to accomplish the task.
So the block size is determined by the amount of code that the LLM can write in one go, and this determines how much iteration you do on the comment.
(We have some demo videos coming soon.)

## Quick start:

1. Get the code and build; assuming git repo at ~/cmpr and you have gcc, `(cd cmpr && make)` should do. Put the executable in your path with e.g. `sudo install cmpr/dist/cmpr /usr/local/bin`.
2. Go to the directory you want to work in and run `cmpr --init`, this creates a `.cmpr/` directory (similar to how git works) with a conf file and some other stuff.
3. `export EDITOR=emacs` or whatever editor you use, or vi will be run by default.
4. Run `cmpr` in this directory, and it will ask you some configuration questions.
   If you want to change the answers later, you can edit the .cmpr/conf file.
5. Stop by discord and ask if you hit any roadblocks!

### Bonus: cmpr in cmpr

1. We ship our own cmpr conf file, so run cmpr in `~/cmpr` to see the code the way we do while building it.

## Caveats:

The features we have today are mostly around editing a single block; we are currently adding support for more multi-block operations.

There's preliminary support for a "bootstrap" script, you create a script that generates a "start of day" prompt on stdout, and then use ":bootstrap" to run that script and put the output on the clipboard.
To support setting up a script like this, you can use the `--find-block`, `--print-block`, `--print-comment`, `--print-code` command-line flags.
For more details on this feature, which is likely to change, stop by the discord.

Developed on Linux, please report any bugs on other platforms!

The first time you use the 'r' or 'R' commands you will be prompted for the command to use to talk to the clipboard on your system.
For Mac you would use "pbcopy" and "pbpaste", on Linux we are using "xclip".

The tool is still new and light on features.
We support C, Python, and JavaScript; mostly this is around syntax of where blocks start (in C we use block comments, and triple-quoted strings in Python).
It's not hard to extend the support to other languages, just ask for what you want in the discord and it may happen soon!
It's not hard to contribute, you don't need to know C well, but you do need to be able to read it (you can't trust the code from GPT without close examination).

To track progress look at the TODO file in the repo and you can see what's changed between releases and what's coming up.

## More

Development is being [streamed on twitch](https://www.twitch.tv/inimino2).
Join [our discord](https://discord.gg/ekEq6jcEQ2).
