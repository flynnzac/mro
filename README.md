<!-- Creator     : groff version 1.22.4 -->
<!-- CreationDate: Fri Jan 10 11:59:24 2020 -->
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta name="generator" content="groff -Thtml, see www.gnu.org">
<meta http-equiv="Content-Type" content="text/html; charset=US-ASCII">
<meta name="Content-Style" content="text/css">
<style type="text/css">
       p       { margin-top: 0; margin-bottom: 0; vertical-align: top }
       pre     { margin-top: 0; margin-bottom: 0; vertical-align: top }
       table   { margin-top: 0; margin-bottom: 0; vertical-align: top }
       h1      { text-align: center }
</style>
<title>pushmac</title>

</head>
<body>

<h1 align="center">pushmac</h1>

<a href="#NAME">NAME</a><br>
<a href="#SYNOPSIS">SYNOPSIS</a><br>
<a href="#DESCRIPTION">DESCRIPTION</a><br>
<a href="#COMMANDS">COMMANDS</a><br>
<a href="#EXAMPLES">EXAMPLES</a><br>
<a href="#SEE ALSO">SEE ALSO</a><br>
<a href="#BUGS">BUGS</a><br>
<a href="#AUTHOR">AUTHOR</a><br>

<hr>


<h2>NAME
<a name="NAME"></a>
</h2>


<p style="margin-left:11%; margin-top: 1em">pushmac &minus;
a stack-based macro processor that can form macros based on
results returned by scheme functions</p>

<h2>SYNOPSIS
<a name="SYNOPSIS"></a>
</h2>



<p style="margin-left:11%; margin-top: 1em"><b>pushmac</b></p>

<h2>DESCRIPTION
<a name="DESCRIPTION"></a>
</h2>


<p style="margin-left:11%; margin-top: 1em"><b>pushmac</b>
is a stack-based macro processor. It reads from standard
input, evaluates the macros provided to standard input, and
writes the processed text to standard output. The macro
language is parsed by reading one character at a time from
left to right in a single pass.</p>

<p style="margin-left:11%; margin-top: 1em">The macro
language is centered around a stack. When you are on the
bottom of the stack, characters typed in are echoed to
standard output until a command is reached. You can move up
a level in the stack by using the PUSH command (by default:
#). Text you enter at higher levels of the stack will not be
put to standard output but can be accessed by <b>pushmac</b>
commands.</p>

<p style="margin-left:11%; margin-top: 1em">Macro
processing languages are used for text generation whether
that text is code, a website, or writing.</p>

<h2>COMMANDS
<a name="COMMANDS"></a>
</h2>


<p style="margin-left:11%; margin-top: 1em">The following
are the <b>pushmac</b> commands</p>

<table width="100%" border="0" rules="none" frame="void"
       cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="11%"></td>
<td width="9%">


<p><b>#</b></p></td>
<td width="2%"></td>
<td width="78%">


<p>PUSH command. All text written after the command is the
next level up on the stack.</p></td></tr>
<tr valign="top" align="left">
<td width="11%"></td>
<td width="9%">


<p><b>@</b></p></td>
<td width="2%"></td>
<td width="78%">


<p>DEFINE command. Pops the text off the top two levels of
the stack and assigns the text at the top of the text to the
macro name at the second level from the top.</p></td></tr>
<tr valign="top" align="left">
<td width="11%"></td>
<td width="9%">


<p><b>~</b></p></td>
<td width="2%"></td>
<td width="78%">


<p>REFERENCE command. Pops the text from the top of the
stack, looks up the name of the macro specified there, and
puts that text&rsquo;s value to the next level down in the
stack.</p> </td></tr>
<tr valign="top" align="left">
<td width="11%"></td>
<td width="9%">


<p><b>$</b></p></td>
<td width="2%"></td>
<td width="78%">


<p>EXPAND command. Expands the macros in the text on the
top level of the stack.</p></td></tr>
<tr valign="top" align="left">
<td width="11%"></td>
<td width="9%">


<p><b>|</b></p></td>
<td width="2%"></td>
<td width="78%">


<p>SHELL command. Evalutes the text on the top level of the
stack as a shell command, and outputs the output of the
command to the next level down on the stack.</p></td></tr>
<tr valign="top" align="left">
<td width="11%"></td>
<td width="9%">


<p><b>&rsquo;=&rsquo;</b></p></td>
<td width="2%"></td>
<td width="78%">


<p>PUSH2 command.</p></td></tr>
<tr valign="top" align="left">
<td width="11%"></td>
<td width="9%">


<p><b>&lsquo;text&rsquo;</b></p></td>
<td width="2%"></td>
<td width="78%">


<p>QUOTE text. Put the literal text to the current level of
the stack. This is how you escape the command
characters.</p> </td></tr>
</table>

<h2>EXAMPLES
<a name="EXAMPLES"></a>
</h2>


<p style="margin-left:11%; margin-top: 1em"><b>Section
header</b></p>

<p style="margin-left:11%; margin-top: 1em"><b>pushmac</b>
commands could be used to create a section header in a
standard way, like so:</p>

<p style="margin-left:22%;">#i=1@ <br>
#title=The Beginning@ <br>
#sec=&lsquo;Section #i~: #title~@</p>

<p style="margin-left:11%; margin-top: 1em">are expanded
to</p>

<p style="margin-left:22%;">Section 1: The Beginning <br>
Section 2: After the Beginning</p>

<p style="margin-left:11%;"><b>Boilerplate Guile
Code</b></p>

<p style="margin-left:11%; margin-top: 1em">Guile is a
Scheme language that is easy to use to extend programs.
Guile functions have to be registered in the C program that
wants to make use of them. <b>pushmac</b> makes this process
easy to generate.</p>

<p style="margin-left:22%;">#register= <br>
void* <br>
register_guile_functions (void* data) <br>
{@ <br>
#gfunc=&lsquo;#register##register~ <br>
scm_c_define_gsubr(&quot;#name~&quot;, #argnum~, 0, 0,
&amp;guile_#name~);@ <br>
SCM <br>
guile_#name~&rsquo;@ <br>
#regbuild=&lsquo;#register~ <br>
return NULL; <br>
}&rsquo;@</p>

<p style="margin-left:11%; margin-top: 1em">Then, we can
create guile functions like:</p>

<p style="margin-left:22%;">#name=myfunction@ <br>
#argnum=1@ <br>
##gfunc~$ (SCM file) { ... }</p>

<p style="margin-left:11%; margin-top: 1em">And then type
<b>##regbuild~$</b> to output the
<b>register_guile_functions</b> function that can be used
with Guile&rsquo;s <b>scm_with_guile</b> function.</p>

<h2>SEE ALSO
<a name="SEE ALSO"></a>
</h2>


<h2>BUGS
<a name="BUGS"></a>
</h2>


<p style="margin-left:11%; margin-top: 1em">Please report
as an issue to https://github.com/flynnzac/pushmac</p>

<h2>AUTHOR
<a name="AUTHOR"></a>
</h2>


<p style="margin-left:11%; margin-top: 1em">Zach Flynn
&lt;zlflynn@gmail.com&gt; 0.0.1</p>
<hr>
</body>
</html>
