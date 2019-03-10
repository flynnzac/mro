<h1>mro</h1>
stack-based macro processor extensible with guile

<p>The mro program processes macros.  It can also execute Guile Scheme code and insert the value that code returns into the text. The <code>mro</code> program is based on a simple stack-based macro language.  It has seven basic commands and is less than 500 lines of code.</p>

<p>All of the program's commands are single characters.  The default command characters are well-suited to producing Tex files and HTML files which is mostly what I use the program for.  These characters can be changed by providing options to the <code>make</code> command.</p>

<p>The program reads macros from standard input and writes to standard output, so the usual command for executing <code>mro</code> is:</p>

<p><code>cat file.mro.html | mro > file.html</code></p>

<h2>Installation</h2>

<code>mro</code> depends on guile and the standard C library.  I made no effort to avoid using GNU C library-specific features so the program might use those.

To build the library, type <code>make</code>.  You can change the default guile include and linking expressions by setting the <code>guile_lib</code> and <code>guile_include</code> variables.

<h2>Commands</h2>
<code>mro</code> has the following commands:

<ul>
<li> # is the default PUSH command.  It sets the buffer that the text is written to be the next level up.  For example, in the text

<p><code>the quick brown fox # runs </code>

<p>"the quick brown fox" is written out and " runs" is written to the next buffer up

<li> = is the default PUSH2 command.  It can be used to PUSH if there is exactly one layer to the stack (i.e. it can be used if one and only PUSH command has been used before). Otherwise, it is treated like normal text.  It never has to be used, but it can make macro definitions more readable.

<li> @ is the default DEFINE command.  It pops the top two buffers and binds the value in the top buffer to the name in the second from the top buffer. Like so:

<p><code>#username=flynnzac@</code>

<p>defines the macro "username" and binds the value "flynnzac" to it.

<li> ~ is the default REFERENCE command.  It pops the top element on the stack, reads it, and looks up the macro. It then replaces the text on the next lowest buffer in the stack with the macro's value.  Like so:

<p><code>#username~</code>

<p>prints the value of the macro `username` (from above: "flynnzac").
    
<li> `TEXT' quotes TEXT and prints it literally, no matter what it contains.  It is the way to escape special characters.

<li> % comments out the rest of the line

<li> | executes the shell command on the top level of the stack if there is at least one buffer there and returns the output to the next level down on the stack.

<li> ! executes the Guile Scheme on the top of the stack.  For example,

<p><code>#(+ 1 2)!</code>

<p>would print "3" to the buffer below it.
  
<li> $ expands the macros in the top of the stock if there is anything on the stack.  For example,
    
<p><code>#code=number`#i~'@</code>
<p><code>#i=1@</code>
<p><code>##code~$</code>
<p><code>#i=2@</code>
<p><code>##code~$</code>

<p> Would print out two lines.  The first would be "number1" and the second would be "number2".
</ul>

<h2>Use Cases</h2>

<p>mro can be used for the sorts of problem that m4 or the C pre-processor is used for, but it is simple and allows access to a full programming language if necessary.  I use it for my personal website -- see the code at <a href="https://github.com/flynnzac/flynnzac.github.io">https://github.com/flynnzac/flynnzac.github.io</a> and the website at <a href="http://zflynn.com">http://zflynn.com</a>.   I also use it in the source code for <code>mro</code>itself to generate boilerplate code making C functions callable from Scheme.  It can be used in this way to do meta-programming.</p>

<h3>Example: boilerplate code generation</h3>

Define the macros:
<pre>
  <code>
  #register=
  void*
  register_guile_functions (void* data)
  {@
  #gfunc=`#register##register~
  scm_c_define_gsubr("#name~", #argnum~, 0, 0, &guile_#name~);@%
  SCM
  guile_#name~'@
  #regbuild=`#register~

  return NULL;
  }'@
  </code>
</pre>

Then, can create guile functions like:
<pre>
  <code>
    #name=source@
    #argnum=1@
    ##gfunc~$ (SCM file) { ... }
  </code>
</pre>

After all the functions have been created, typing, <code>##regbuild~$</code> will write out the necessary <code>register_guile_functions</code> function.
    

<h3>Example: automatic section numbering</h3>

<pre>
<code>#i=1@%
#sec=`Section #i~#i##(+ #i~ 1)!@'@%
##sec~$
##sec~$
</code>
</pre>

The code above would print out Section 1 and then Section 2 and so on.

<h2>Built-in Scheme commands</h2>

The following are built-in scheme commands available:

<ul>
  <li> <code>#(source "file.mro")!</code> treats the file as if it were included in the text.
  <li> <code>#(add_to_dnp "\n")!</code> ignores newlines in all subsequent code.  Replace "\n" with other characters to ignore those.
  <li> <code>#(printall)!</code> removes all characters from the do-not-print list that were added with <code>add-to-dnp</code>
</ul>

