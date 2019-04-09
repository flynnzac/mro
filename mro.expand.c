/* 
   mro - a stack-based macro expander extendable with Guile Scheme

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
#include <libguile.h>
;

/* parser state */
static int inquote = 0;
static int incomment = 0;

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

/* do not print functions/macros */
;

int
check_dnp (int c)
{
  int i;

  for (i=1; i < n_dnp; i++)
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
  int i;
  char* dest;
  dest = malloc(sizeof(char)*(src->location+1));
  
  for (i=0; i < src->location; i++)
    dest[i] = src->text[i];

  dest[src->location] = '\0';
  return dest;
}

/* pushes text to buffer or screen */
void
output (int c)
{
  if (check_dnp(c))
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
  int loc;
  struct buffer* value;
  struct buffer* name;

   value=pop_buffer_stack(); null_terminate(value);;
   name=pop_buffer_stack(); null_terminate(name);;

  add_macro_to_table(copy_from_buffer(name),
		     copy_from_buffer(value));
  
}





/* expands macros from a file stream */
void
expand_macros (FILE* f)
{
  int i,c,loc;
  struct buffer* buf;
  char* guile_str;
  SCM guile_ret;
  FILE* f2;

  while (((c = fgetc(f)) != EOF) && c != '\0')
    {
      if (inquote)
        {
          if (c == '\'')
            inquote = 0;
          else
            output(c);
            }
      else if (incomment)
        {
          if (c == COMMENT_END)
            incomment = 0;
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
            case CODE:
              
              ;
	      
              if (stack.level >= 1) { 
              buf=pop_buffer_stack(); null_terminate(buf);;
              guile_ret = scm_c_eval_string(buf->text);
              if (!scm_is_eq(guile_ret, SCM_UNSPECIFIED))
                {
                  guile_str = scm_to_locale_string
                    (scm_object_to_string
                     (guile_ret,
                      scm_c_eval_string("display")));
                  for (i=0; i < strlen(guile_str); i++)
                    output(guile_str[i]);
            
                  free(guile_str);
                } } else { output(c); } break;;
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
            case '`':
              inquote = 1;
              break;
            case COMMENT_START:
              incomment = 1;
              break;
            default:
              output(c);
              break;
            }
        }
    }
}

/* define macros to add guile functions */

    
      ;

/* guile: add to do not print list */



    SCM
      guile_add_to_dnp (SCM ch)
{
  char* str = scm_to_locale_string(ch);
   
dnp = realloc(dnp, sizeof(char)*(n_dnp+1));
dnp[n_dnp] = str[0];
n_dnp++;

  free(str);
  return scm_from_locale_string("");
}

/* guile: clear do not print list */



    SCM
      guile_printall ()
{
  dnp = realloc(dnp, sizeof(char));
  dnp[0] = '\0';
  n_dnp = 1;

  return scm_from_locale_string("");
}

/* guile: start a definition section by adding \n to do not print
   list */



    SCM
      guile_defsec ()
{
   
dnp = realloc(dnp, sizeof(char)*(n_dnp+1));
dnp[n_dnp] = '\n';
n_dnp++;

  return scm_from_locale_string("");
}

/* guile: source a macro file as if it was entered along with text */


    SCM
      guile_source (SCM file)
{
  char* file_c = scm_to_locale_string(file);
  FILE* f = fopen(file_c, "r");
  expand_macros(f);
  fclose(f);
  free(file_c);
  return scm_from_locale_string("");
}


void*
register_guile_functions (void* data)
{
    scm_c_define_gsubr("add_to_dnp", 1, 0, 0, &guile_add_to_dnp);
    scm_c_define_gsubr("printall", 0, 0, 0, &guile_printall);
    scm_c_define_gsubr("defsec", 0, 0, 0, &guile_defsec);
    scm_c_define_gsubr("source", 1, 0, 0, &guile_source);

      return NULL;
}

/* main program */
int
main (int argc, char** argv)
{
  int i;
  int sz;
  char* val;
  char* num;
  
  FILE* f;

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
  scm_with_guile(&register_guile_functions, NULL);

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
