# CMPr

## Program in English!

With LLMs like GPT4 we can now write code in English and have the LLM "compile" it into programming language code such as C or Python.
In this approach, the source code is actually the English (or other natural language) expression of the program's behavior, and the programming language code is treated as generated code, which we do not edit directly.

We're still programming, but now in English!

Programming in English, just like learning any new programming language, has a learning curve, but also many benefits, including more effective communication, increased security, and fewer bugs.
Getting onboarded to a new codebase is much easier when the code can be read in English or in C.

A common complaint about code comments is that they often don't reflect the state of the code and may be actively misleading and out of date.
When you read English code that the actual program code is generated from, you know that what you're reading reflects the actual state of the program.

Here are some examples of code written in this style:

A 2048 implementation:

- [Here](https://inimino.github.io/2048/) is the playable game.
- Here is [all the English code for this implementation](doc/examples/2048.md); this is everything the LLM sees before generating the playable game above (more info in [the repo](https://github.com/inimino/2048/)).

In contrast with tools that help you autocomplete code, here we are largely delegating the lower-level code to the LLM, so we are free to focus on the higher-level description.
However, the programmer remains responsible for the correctness of the generated code, and for debugging when necessary.

A more complex example is [the cmpr code itself](https://github.com/inimino/cmpr/cmpr.c).

## What's this then?

This is a tool to support the workflow of programming in English.

You can try it with a free ChatGPT account or any chat model.
If you have access to GPT4, it generally writes the best code.

We have early support for GPT4 and GPT3.5 if you provide your OpenAI API key.
(Coming soon: other APIs, local models, etc.)

This is a framework and represents a particular and opinionated approach to this workflow, and we are updating as we learn.

## Blocks

All C code here is by ChatGPT4, and all the English is by the human authors, which captures a key idea:

- Code is organized into blocks; each has a comment and then (usually) some code.
- You write the comment; the LLM writes the code.
- You iterate on the comment until the code works and meets your standard.

The cmpr tool itself runs on your machine and supports this workflow.
You use cmpr to navigate and search and manage your blocks, edit them in an editor, and get the LLM to rewrite the code parts of each block.
The tool also manages revisions, saving a copy every time or the LLM edit the codebase.
You can use another IDE or editor alongside cmpr if you like, or adopt cmpr as your primary tool to interact with your code.

Nearby features coming soon relate to other directions such from programming language code to natural language (considerably harder for current LLMs) and treating diffs (between natural language descriptions or code implementations).

## Why blocks?

The block is the basic unit of interaction with the LLM.
We start with a "bootstrap block" that includes basic information about our codebase, libraries we're using, and so on (the stuff that a human programmer would learn from a readme file or onboarding resources).

Current state-of-the-art LLMs like GPT4 can reliably generate a moderately-sized function from an English description that's sufficiently precise, along with this background information.
However, if you attempted to describe an entire large and complex program and get the all the code from a single prompt, today's LLMs will not be able to accomplish the task.
So the block size is determined by the amount of code that the LLM can write "in one go," and smaller blocks (like smaller functions) are easier to get right.

## Quick start:

1. Get the code and build; assuming git repo at ~/cmpr and you have gcc, `cd cmpr; make` should do. Currently we depend on libcurl, so you must have that and the headers installed too (e.g. `apt-get install libcurl libcurl-dev`). Put the executable in your path with e.g. `sudo install cmpr/dist/cmpr /usr/local/bin`.
2. Go to (or create) the directory for your project and run `cmpr --init`, this creates a `.cmpr/` directory (like git) with a conf file and some other stuff.
3. `export EDITOR=emacs` or whatever editor you use, or vi will be run by default.
4. Run `cmpr` in this directory, and it will ask you some configuration questions.
   If you want to change the answers later, you can edit the .cmpr/conf file.
5. Stop by discord and ask if you hit any roadblocks!

### Bonus: cmpr in cmpr

1. We ship our own cmpr conf file, so run cmpr in `~/cmpr` to see the code the way we do while building it.

## Caveats:

It's early days and there are bugs and many features still missing.
Feedback and contributions welcome!
You don't need to know C well, but you do need to be able to read it (you can't trust the code from GPT4 without close examination).

There's preliminary support for a "bootstrap" script with the ":bootstrap" ex command.
For more details on this feature, stop by the discord.

Developed on Linux, please report any bugs on other platforms!

The first time you use the 'r' or 'R' commands you will be prompted for the command to use to talk to the clipboard on your system.
For Mac you would use "pbcopy" and "pbpaste", on Linux we are using "xclip".

We support a small number of languages at the moment (this is mostly about how files get broken into blocks).
It's not hard to extend the support to other languages, just ask for what you want in the discord and it may happen soon!

To track progress, check out the TODO file in the repo.

## More

Development is being [streamed on twitch](https://www.twitch.tv/inimino2).
Join [our discord](https://discord.gg/ekEq6jcEQ2).
