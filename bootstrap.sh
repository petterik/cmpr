#!/bin/sh

cat <<EOF
We are writing cmpr, which is a tool to interact with LLMs and is written in C.
EOF

#cat <<EOF
#Here is information about spanio, the library we are using:
#
#Note that spanio is already included, so don't include header files (ever, for it or anything else).
#EOF
#
#cmpr --print-block $(cmpr --find-block "#libraryintro")

#cat <<EOF
#Here's our ctags:
#
#\`\`\`
#EOF
#
#ctags cmpr/spanio.c cmpr/cmpr.c
#
#cat cmpr/tags
#
#cat <<EOF
#
#\`\`\`

cat <<EOF

We have a few globals; to those already part of spanio (inp, out, cmp, etc.) we add state, a pointer to the ui_state struct, automatically available everywhere:
EOF

cmpr --print-block $(cmpr --find-block "#config_fields")
cmpr --print-code $(cmpr --find-block "#ui_state")
cmpr --print-block $(cmpr --find-block "#projfiles")

echo "Here's all the functions we have implemented so far:\n\n\`\`\`c"

cmpr --print-code $(cmpr --find-block "#all_functions")

echo "\`\`\`"
echo

cat <<EOF
Here is information about spanio, the library we are using:

Note that spanio is already included, so don't include header files (ever, for it or anything else).
EOF

cmpr --print-block $(cmpr --find-block "#libraryintro")

cat <<EOF

The projfiles type is created by MAKE_ARENA and has the methods defined for our generic array types (with element type projfile).
In particular, we use projfiles_alloc with a sufficient capacity in main() and then we use projfiles_push() whenever we populate a project file.

In the library intro above there are some idioms and advice given at the very end. Extremely briefly, summarize these in bullet form, starting with "- spans uses .s not .a".
EOF
