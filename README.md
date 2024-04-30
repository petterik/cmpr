# CMPr

## Program in English!

Have you ever wanted to write your code in English and have an LLM compile it into a programming language like C or Python?
No?
Well, now you can.

The programming language code is treated as generated code, which we do not edit directly.

We're still programming, but now in English!

Programming in English has a learning curve, like any new programming language, but also some benefits.
The greatest difficulty in programming and debugging, especially in someone else's code, is understanding what is going on.
Reading a program in English makes this much easier.
When you read English code that the actual program code is generated from, you know that what you're reading reflects the actual state of the program.
You can also still read the implementation, of course, when necessary.

The difficult things about this approach are:

1. Writing English that is clear enough for GPT4 or another LLM to write correct code.
2. Determining whether the code that is written is correct.
3. Debugging it when it is not.

The programmer still has to work hard, perhaps harder than before.
However, it can also be quite enjoyable.

Here are some examples of code written in this style:

A 2048 implementation:

- [Here](https://inimino.github.io/2048/) is the playable game.
- Here is [all the English code for this implementation](doc/examples/2048.md); this is everything the LLM sees before generating the playable game above (more info in [the repo](https://github.com/inimino/2048/)).

Notice that the code (that is, the English code) is mostly focused on what makes 2048 2048.
For example, the English code mentions that when a new tile appears, 90% of the time it will be a 2.
If you changed this ratio the game would be a different game, maybe harder or easier.
The things that are left out of the English are the specifics of how to generate a 2 with probability .9 in JavaScript (or Python or whatever language) and these are the things that are not essential to the game.
You could use the English code for 2048 to test your JavaScript, HTML, and CSS knowledge.
If you can write the JS and CSS from the English description as well or better than GPT4, then you know JS and CSS pretty well.
On the other hand, so does GPT4, so knowing arcane details of a particular language now has less value relative to understanding algorithms and fundamental CS and programming principles.

This has some implications for the future of programming as a career.
If you can describe a program well enough that GPT4 can write it, that's valuable.
If you can take that same description and program it, that's not valuable, because GPT4 can do it cheaper and faster.
Programmers remain relevant by rising up the levels of abstraction as they become available to us.

Compare the English code with [the generated HTML, CSS, and JS](doc/examples/2048-gen.md).
Which one would you rather read?

A more complex example is [the cmpr code itself](https://github.com/inimino/cmpr/cmpr.c).
In constrast with 2048, this is not something the model has seen before, it's written in C, it's using a custom library which GPT must be told about, and it's a medium-sized program, not a toy.

## What's this then?

This is a tool to support the workflow of programming in English.

You can try it with a free ChatGPT account or any chat model with a web interface.
If you have access to GPT4, it generally writes the best code.

As of v6 you can also use gpt-3.5-turbo and gpt-4-turbo models by providing your own API key.
(Other APIs, local models, etc. coming very soon.)

## Cmpr vs Others

In comparison with Copilot, you are giving up control of the programming-language code (PL code) in order to move up a level of abstraction to the natural-language code (NL code).
The benefit of NL code is that it can be shorter, clearer, easier to understand a week or month later, and easier to maintain.
With Copilot, it's helping you edit the PL code, but it's not actually moving you up a level of abstraction to the NL level.

In comparison with tools like Devin, this workflow actually works.
All the code in cmpr is actually written by GPT4 using the tool; I do not believe Devin can make the same claim.
Until we have something much closer to AGI, we are not going to be able to hand over the entire task to an AI agent.
Programming requires problem solving skills that LLMs do not have.
A lot of companies are making unrealistic promises, and people who haven't worked with LLMs closely may believe that programming is obsolete, but nothing could be further from the truth.
Programmers are about to get a lot more effective, and good ones will be worth even more.

Are there other comparable tools that should be mentioned here?
Let us know!

## Blocks

All C code here is by GPT4, and all the English is by the human authors, which captures a key idea:

- Code is organized into blocks; each has a comment and then (usually) some code.
- You write the comment; the LLM writes the code.
- You iterate on the comment until the code works and meets your standard.

The cmpr tool itself runs on your machine and supports this workflow.
You use cmpr to navigate and search and manage your blocks, edit them in an editor, and get the LLM to rewrite the code parts of each block.
The tool also manages revisions, saving a copy every time or the LLM edit the codebase.
You can use another IDE or editor alongside cmpr if you like, but it works better if you adopt it as your primary interface, replacing the IDE or editor you use now.
(I still dip back into vim frequently; there's a lot of features we still lack.)

Features coming soon relate to other directions, such as from PL code to NL code (considerably harder for current LLMs), and treating with diffs (between natural language descriptions or code implementations; there are at least four interesting directions here that the LLM can potentially help with).

## Why blocks?

The block is the basic unit of interaction with the LLM.
We start with a "bootstrap block" that includes basic information about our codebase, libraries we're using, and so on (the stuff that a human programmer would learn from a readme file or onboarding resources).
See the 2048 demo codebase or the video walkthrough for more on this.

Current state-of-the-art LLMs like GPT4 can reliably generate a moderately-sized function from an English description that's sufficiently precise, along with this background information.
However, if you attempted to describe an entire large and complex program and get the all the code from a single prompt, today's LLMs will not be able to accomplish the task.
So the block size is determined by the amount of code that the LLM can write "in one go," and smaller blocks (like smaller functions) are easier to get right.

## Quick start:

0. Look at some sample code or watch the 2048 demo video to see if the idea appeals to you, and experiment with bare ChatGPT program generation first if you haven't already.
1. Get the code and build; assuming git repo at ~/cmpr and you have gcc, `cd cmpr; make` should do. Put the executable in your path with e.g. `sudo install cmpr/dist/cmpr /usr/local/bin`. You'll also need curl if you want to use GPT via API. (As of v7 we no longer depend on libcurl.)
2. Go to (or create) the directory for your project and run `cmpr --init`, this creates a `.cmpr/` directory (like git) with a conf file and some other stuff.
3. `export EDITOR=emacs` or whatever editor you use, or vi will be run by default.
4. Run `cmpr` in your project directory, and it will ask you some configuration questions.
   If you want to change the answers later, you can edit the .cmpr/conf file.
5. Right now things are rough and changing all the time, so stop by discord and ask if you hit any roadblocks!

### Bonus: cmpr in cmpr

1. We ship our own cmpr conf file, so run cmpr in the cmpr repo to see the code the way we do while building it.
   Use j/k to move around, ? to get help on keyboard commands, B to do a build, and q to quit.

## Caveats:

It's early days and there are bugs and many features still missing.

There's preliminary support for a "bootstrap" script with the ":bootstrap" ex command.
Basically you make a script that produces your bootstrap block on stdout, you put it in the conf file, and you use :bootstrap to run it.
For more details on this feature, stop by the discord and ask!
We also ship our own bootstrap.sh which you can look at as a template.

Developed on Linux; should work on Mac or Windows with WSL2.

The first time you use the 'r' or 'R' commands you will be prompted for the command to use to talk to the clipboard on your system.
For Mac you would use "pbcopy" and "pbpaste", on Linux we are using "xclip".

We support a small number of languages at the moment (this is mostly about how files get broken into blocks).
It's not hard to extend the support to other languages, just ask for what you want in the discord and it may happen soon!

To track progress, check out the TODO file in the repo.

## More

Development is sometimes [streamed on twitch](https://www.twitch.tv/inimino2).
Join [our discord](https://discord.gg/ekEq6jcEQ2).
