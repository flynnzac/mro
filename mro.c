#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libguile.h>

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

/* pops buffer off stack */

struct buffer*
pop_buffer_stack ()
{
  stack.n_buf--;
  return &stack.buf[stack.n_buf];
}

/* writes character to buffer */

void
push_to_buffer (struct buffer* b, int c)
{
  b->text[b->location] = c;
  b->location = b->location + 1;
}

/* copy buffer to string */

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

/* pushes text to buffer or screen */

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

/* null terminate a buffer */

void
null_terminate (struct buffer* b)
{
  b->text[b->location] = '\0';
}


/* clean up macro table */

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

/* add macro given top two elements in buffer */
void
push_macro ()

{
  struct buffer* value = pop_buffer_stack();
  struct buffer* name = pop_buffer_stack();
  
  copy_from_buffer(name, &(macro_table[n_macros].name));
  copy_from_buffer(value, &(macro_table[n_macros].value));

  n_macros++;
  
}

/* look up macro name */
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

void
expand_macros (FILE* f)
{
  int c;
  int loc;
  int i;
  struct buffer* buf;
  char* guile_str;
  SCM guile_ret;


  while ((c = fgetc(f)) != EOF)
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
              if (stack.n_buf != 1)
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

  
}


/* guile: add to do not print list */

SCM
guile_add_to_dnp (SCM ch)
{
  char* str = scm_to_locale_string(ch);
  add_to_dnp(str[0]);
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

/* guile: start a definition section by adding \n and ' ' to do not print
   list */

SCM
guile_defsec ()
{
  add_to_dnp('\n');
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
  scm_c_define_gsubr("add-to-dnp", 1, 0, 0, &guile_add_to_dnp);
  scm_c_define_gsubr("printall", 0, 0, 0, &guile_printall);
  scm_c_define_gsubr("defsec", 0, 0, 0, &guile_defsec);

  scm_c_define_gsubr("source", 1, 0, 0, &guile_source);

  return NULL;
}


/* main program */
int
main (int argc, char** argv)
{
  
  stack.n_buf = 0;

  dnp = malloc(sizeof(char));
  dnp[0] = '\0';
  n_dnp = 1;

  scm_with_guile(&register_guile_functions, NULL);

  expand_macros(stdin);

  free_macro_table();
  return 0;

}
