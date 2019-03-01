# mro
stack-based macro processor extensible with guile

The mro program processes macros.  It can also execute Guile Scheme code and use the values it returns in the text. The `mro` program is based on a simple stack-based macro language.  It has six commands and is less than 300 lines of code.

All of the program's commands are single characters.  The defaults are well-suited to producing Tex files which is mostly what I use the program for.  These characters can be changed by providing options to the `make` command. `mro` has the following commands:

- `#` is the default PUSH command.  It sets the buffer that the text is written to be the next level up.  For example, in the text

    `the quick brown fox # runs `

    "the quick brown fox" is written out and " runs" is written to the next buffer up

- `=` is the default PUSH2 command.  It can be used to PUSH if there is already an element in the buffer. Otherwise, it is treated like normal text.  It never has to be used, but it can make macro definition more readable.

- `@` is the default DEFINE command.  It pops the top two buffers and binds the value in the top buffer to the name in the second from the top buffer. Like so:

    `#username=flynnzac@`

    defines the macro "username" and binds the value "flynnzac" to it.

- `~` is the default REFERENCE command.  It pops the top element on the stack, reads it, and looks up the macro. It then replaces the text on the next lowest buffer in the stack with the macro's value.  Like so:

    `#username~`

    prints the value of the macro `username` (from above: "flynnzac").
    
- `TEXT' quotes TEXT and prints it literally, no matter what it contains.  It is the way to escape special characters.

- `%` comments out the rest of the line

- `;` executes the Guile Scheme on the top of the stack:

    `#(+ 1 2);`

    would print "3" out.

