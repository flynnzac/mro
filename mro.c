#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libguile.h>

/* 
   macro language defaults:
   push: #
   define: @
   reference: ~
   quote text without parsing: `TEXT'
   comment a line: %
   execute guile scheme code: <
*/

/* parser state */
static int inquote = 0;
static int incomment = 0;


/* buffer stack */
struct buffer { char text[MAXBUFFER]; int location; };
struct buffer_stack { struct buffer buf[MAXSTACK]; int n_buf;  };

struct buffer_stack stack;

/* macro table */
struct macro { char* name; char* value; };
static struct macro macro_table[MAXMACROS];
static int n_macros = 0;

/* characters not to print */
static char* dnp;
static int n_dnp = 0;

/* do not print functions */
void
add_to_dnp (int c)
{
  dnp = realloc(dnp, sizeof(char)*(n_dnp+1));
  dnp[n_dnp] = c;
  n_dnp++;
}

int
check_dnp (int c)
{
  int i;
  for (i=1; i < n_dnp; i++)
    {
      if (dnp[i]==c)
        return 1;
    }
  return 0;
}

/* manipulate stack */

struct buffer*
pop_buffer_stack ()
{
  stack.n_buf--;
  return &stack.buf[stack.n_buf];
}

/* write to and from buffers */

void
push_to_buffer (struct buffer* b, int c)
{
  b->text[b->location] = c;
  b->location = b->location + 1;
}

void
copy_from_buffer (struct buffer* src, char** dest)
{
  int i;
  *dest = malloc(sizeof(char)*(src->location+1));
  
  for (i=0; i < src->location; i++)
    {
      (*dest)[i] = src->text[i];
    }
  (*dest)[src->location] = '\0';
}

void
output (int c)
{
  if (check_dnp(c))
      return;

  
  if (stack.n_buf == 0)
    {
      printf("%c", c);
    }
  else
    {
      push_to_buffer(&stack.buf[stack.n_buf-1], c);
    }
}

void
null_terminate (struct buffer* b)
{
  b->text[b->location] = '\0';
}


/* clean up functions */

void
free_macro_table ()
{
  int i;
  for (i=0; i < n_macros; i++)
    {
      free(macro_table[i].name);
      free(macro_table[i].value);
    }
}

/* macro manipulation */
void
push_macro ()

{
  struct buffer* value = pop_buffer_stack();
  struct buffer* name = pop_buffer_stack();
  
  copy_from_buffer(name, &(macro_table[n_macros].name));
  copy_from_buffer(value, &(macro_table[n_macros].value));

  n_macros++;
  
}

int
look_up_name (const struct buffer name)
{
  int i;
  for (i = 0; i < n_macros; i++)
    {
      if (strcmp(macro_table[i].name, name.text)==0)
        {
          return i;
        }
    }
  return -1;
}

/* guile integration */

SCM
guile_add_to_dnp (SCM ch)
{
  char* str = scm_to_locale_string(ch);
  add_to_dnp(str[0]);
  free(str);
  return scm_from_locale_string("");
}

SCM
guile_printall ()
{
  dnp = realloc(dnp, sizeof(char));
  dnp[0] = '\0';
  n_dnp = 1;

  return scm_from_locale_string("");
}

SCM
guile_defsec ()
{
  add_to_dnp('\n');
  add_to_dnp(' ');
  return scm_from_locale_string("");
}

void*
register_guile_functions (void* data)
{
  scm_c_define_gsubr("add-to-dnp", 1, 0, 0, &guile_add_to_dnp);
  scm_c_define_gsubr("printall", 0, 0, 0, &guile_printall);
  scm_c_define_gsubr("defsec", 0, 0, 0, &guile_defsec);

  return NULL;
}
  

/* main program */
int
main (int argc, char** argv)
{

  int c;
  int loc;
  int i;
  struct buffer* buf;
  char* guile_str;
  SCM guile_ret;
  
  stack.n_buf = 0;

  dnp = malloc(sizeof(char));
  dnp[0] = '\0';
  n_dnp = 1;

  scm_with_guile(&register_guile_functions, NULL);

  while ((c = getc(stdin)) != EOF)
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
              if (stack.n_buf == 0)
                {
                  output(c);
                  break;
                }
            case PUSH:
              {
                stack.buf[stack.n_buf].location = 0;
                stack.n_buf++;
              }
              break;
            case DEFINE:
              if (stack.n_buf >= 2)
                  push_macro();
              else
                  output(c);
              break;
            case REF:
              if (stack.n_buf >= 1)
                {
                  buf = pop_buffer_stack();
                  null_terminate(buf);
                  loc = look_up_name(*buf);
                  if (loc >= 0)
                    {
                      for (i=0; i < strlen(macro_table[loc].value); i++)
                        {
                          output(macro_table[loc].value[i]);
                        }
                    }
                }
              else
                output(c);
              break;
            case CODE:
              if (stack.n_buf >= 1)
                {
                  buf = pop_buffer_stack();
                  null_terminate(buf);
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
                    }
                }
              break;
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

  free_macro_table();
  return 0;

}
