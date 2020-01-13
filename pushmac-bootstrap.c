/* 
   pushmac - a stack-based macro expander extendable with Guile Scheme

   Copyright Zach Flynn <zlflynn@gmail.com>

   This program is free software: you can redistribute it and/or modify it under
   the terms of version 3 of the GNU General Public License as published by the
   Free Software Foundation.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
   details.

   You should have received a copy of the GNU General Public License along with
   this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
;

/* parser state */
static int inquote = 0;
static int ignore = 0;

/* buffer stack */
struct buffer { char* text; int location; int size; };
struct buffer_stack { struct buffer* buf; int level; int n_pages; };

static struct buffer_stack stack;

/* macro table */
struct macro { char* name; char* value; };
struct macro_stack { struct macro* table; int n_pages; int n_macros; };

static struct macro_stack m;

/* characters not to print */
static char* dnp;
static int n_dnp = 0;

int
check_dnp (int c)
{
  for (int i=1; i < n_dnp; i++)
    if (dnp[i]==c)
      return 1;

  return 0;
}

/* pops buffer off stack */
struct buffer*
pop_buffer_stack ()
{
  stack.level--;
  return &stack.buf[stack.level];
}

/* writes character to buffer */
void
push_to_buffer (struct buffer* b, int c)
{
  if (b->size <= b->location)
    {
      b->text = realloc(b->text,
                        sizeof(char)*(b->size + PAGE_BUFFER));
      b->size = b->size + PAGE_BUFFER;
    }
  
  b->text[b->location] = c;
  b->location++;
}

/* init buffer */
void
init_buffer (struct buffer* b)
{
  b->text = malloc(sizeof(char)*PAGE_BUFFER);
  b->size = PAGE_BUFFER;
  b->location = 0;
}

/* copy buffer to string */
char*
copy_from_buffer (struct buffer* src)
{
  char* dest = malloc(sizeof(char)*(src->location+1));
  
  for (int i=0; i < src->location; i++)
    dest[i] = src->text[i];

  dest[src->location] = '\0';
  return dest;
}

/* pushes text to buffer or screen */
void
output (int c)
{
  if (stack.level == 0 && check_dnp(c))
    return;
  
  if (stack.level == 0)
    printf("%c", c);
  else
    push_to_buffer(&stack.buf[stack.level-1], c);
}

/* null terminate a buffer */
void
null_terminate (struct buffer* b)
{
  push_to_buffer(b, '\0');
  b->location--;
}

/* clean up macro table */
void
free_macro_table ()
{
  int i;
  for (i=0; i < m.n_macros; i++)
    {
      free(m.table[i].name);
      free(m.table[i].value);
    }
  free(m.table);
}

/* clean up stack */
void
free_stack ()
{
  int i;
  for (i=0; i < (PAGE_STACK*stack.n_pages); i++)
    free(stack.buf[i].text);

  free(stack.buf);
}

/* look up macro name */
int
look_up_name (char* name)
{
  int i;

  for (i = 0; i < m.n_macros; i++)
    if (strcmp(m.table[i].name, name)==0)
      return i;

  return -1;
}

/* add macro to table */
void
add_macro_to_table (char* name, char* value)
{
  int loc;
  loc = look_up_name(name);
  if (loc < 0)
    {
      if (m.n_macros >= (PAGE_MACRO*m.n_pages))
	{
	  m.n_pages++;
	  m.table = realloc(m.table,
			    sizeof(struct macro)*PAGE_MACRO*m.n_pages);
	}
      m.table[m.n_macros].name = name;
      m.table[m.n_macros].value = value;
      m.n_macros++;
    }
  else
    {
      free(m.table[loc].value);
      free(name);
      m.table[loc].value = value;
    }
}


/* add macro given top two elements in buffer */
void
push_macro ()

{
  struct buffer* value;
  struct buffer* name;

   value=pop_buffer_stack(); null_terminate(value);;
   name=pop_buffer_stack(); null_terminate(name);;

  add_macro_to_table(copy_from_buffer(name),
		     copy_from_buffer(value));
  
}

/* look at top three buffers. if third buffer is "yes", print second buffer. otherwise, print top buffer. */
void
ask_question ()
{
  struct buffer* answer, *yesreply, *noreply;

   noreply=pop_buffer_stack(); null_terminate(noreply);;
   yesreply=pop_buffer_stack(); null_terminate(yesreply);;
   answer=pop_buffer_stack(); null_terminate(answer);;

  if (strcmp(answer->text,"yes") == 0)
    for (int i=0; i < yesreply->location; i++)
      output(yesreply->text[i]);
  else
    for (int i=0; i < noreply->location; i++)
      output(noreply->text[i]);


}

/* append characters from buffer input to an array input, parsing
   escape seq, returns ending position */
int
escape_and_add_chars (struct buffer* buf, char** array, int pos)
{
  /* make this UTF-8'able */
  *array = realloc(*array, sizeof(char)*(pos+buf->location));
  int isslash = 0;
  int j = pos;
  for (int i=0; i < buf->location; i++)
    {
      if (isslash)
	{
	  switch (buf->text[i])
	    {
	    case '\\':
	      (*array)[j] = '\\';
	      break;
	    case 'n':
	      (*array)[j] = '\n';
	      break;
	    case 't':
	      (*array)[j] = '\t';
	      break;
	    case 'r':
	      (*array)[j] = '\r';
	      break;
	    default:
	      (*array)[j] = '\\';
	      j++;
	      (*array)[j] = buf->text[i];
	      break;
	    }
	  j++;
	}
      else
	{
	  if (buf->text[i] == '\\')
	    isslash = 1;
	  else
	    {
	      (*array)[j] = buf->text[i];
	      j++;
	    }
	}
    }
  return j;
}






/* expands macros from a file stream */
void
expand_macros (FILE* f)
{
  int i,c,loc;
  struct buffer* buf;
  FILE* f2;

  while (((c = fgetc(f)) != EOF) && c != '\0')
    {
      if (inquote)
        {
          if (c == '\'') inquote = 0; else output(c);;
	}
      else if (ignore)
	{
	  if (c == '\n') ignore = 0;
	}
      else
        {
          switch (c)
            {
            case PUSH2:
              if (stack.level != 1)
                {
                  output(c);
                  break;
                }
            case PUSH:
              {
                if (stack.level >= (PAGE_STACK*stack.n_pages))
                  {
                    stack.n_pages++;
                    stack.buf = realloc(stack.buf,
                                        sizeof(struct buffer)*
                                        PAGE_STACK*stack.n_pages);
                    for (i=(stack.n_pages-1)*PAGE_STACK;
                         i < (PAGE_STACK*stack.n_pages); i++)
                      init_buffer(stack.buf+i);
                  }
                stack.buf[stack.level].location = 0;
                stack.level++;
              }
              break;
            case DEFINE:
              
              
              if (stack.level >= 2) { push_macro(); } else { output(c); } break;;
            case REF:
              
              ;
              if (stack.level >= 1) { 
              buf=pop_buffer_stack(); null_terminate(buf);;
              loc = look_up_name(buf->text);
              if (loc >= 0)
                for (i=0; i < strlen(m.table[loc].value); i++)
                  output(m.table[loc].value[i]);
               } else { output(c); } break;;
            case EXPAND:
              
              ;
              if (stack.level >= 1) { 
              buf=pop_buffer_stack(); null_terminate(buf);;
              f2 = fmemopen(buf->text, buf->size, "r");
              expand_macros(f2);
              fclose(f2); } else { output(c); } break;;
            case SHELL:
              
              ;
              if (stack.level >= 1) { 
              buf=pop_buffer_stack(); null_terminate(buf);;
              f2 = popen(buf->text, "r");
              while (((c=fgetc(f2)) != EOF) && c != '\0')
		output(c);
              pclose(f2); } else { output(c); } break;;
	    case QUESTION:
	      
	      
	      if (stack.level >= 3) { ask_question(); } else { output(c); } break;;
	    case IGNORE:
	      
	      ;
	      if (stack.level >= 1) { 
	      buf=pop_buffer_stack(); null_terminate(buf);; } else { output(c); } break;;
	      break;
	    case SILENCE:
	      
	      ;
	      if (stack.level >= 1) { 
	      buf=pop_buffer_stack(); null_terminate(buf);;
	      n_dnp = escape_and_add_chars(buf, &dnp, n_dnp);
	      dnp = realloc(dnp, sizeof(char)*n_dnp);
	       } else { output(c); } break;;
	      break;
	    case SPEAK:
	      
	      ;
	      if (stack.level >= 1) { 
	      buf=pop_buffer_stack(); null_terminate(buf);;
	      free(dnp);
	      dnp = malloc(sizeof(char));
	      dnp[0] = '\0';
	      n_dnp = 1;
	       } else { output(c); } break;;
	      break;
            case '`':
	      inquote = 1;
	      break;
            default:
              output(c);
              break;
            }
        }
    }
}

/* main program */
int
main (int argc, char** argv)
{
  int i;
  char* val;
  char* num;
  
  stack.level = 0;
  stack.n_pages = 1;
  stack.buf = malloc(sizeof(struct buffer)*PAGE_STACK);
  for (i=0; i < PAGE_STACK; i++)
    init_buffer(stack.buf+i);

  m.n_macros = 0;
  m.n_pages = 1;
  m.table = malloc(sizeof(struct macro)*PAGE_MACRO);

  dnp = malloc(sizeof(char));
  dnp[0] = '\0';
  n_dnp = 1;

  /* expand , , ... to the command line args */
  for (i=1; i < argc; i++)
    {
      num = malloc(sizeof(char)*((i / 10) + 1+1));
      sprintf(num, "%d", i-1);

      val = malloc(sizeof(char)*(strlen(argv[i])+1));
      strcpy(val, argv[i]);
      
      add_macro_to_table(num,val);
    }

  expand_macros(stdin);

  free_stack();
  free_macro_table();
  free(dnp);

  return 0;
}
