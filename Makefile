page_buffer=10000
page_stack=100
page_macro=100

define="'@'"
code="'!'"
shell="'|'"
comment_start="'%'"
comment_end="'\\n'"
push = "'\#'"
push2 = "'='"
ref = "'~'"
expand = 36
guile_include = /usr/include/guile/2.2
guile_lib = guile-2.2

mro: mro.expand.c expand
	cc -I $(guile_include) mro.expand.c -l$(guile_lib) -o mro \
	-DPAGE_MACRO=$(page_macro) \
	-DPAGE_BUFFER=$(page_buffer) \
	-DPAGE_STACK=$(page_stack)  \
	-DSHELL=$(shell) \
	-DDEFINE=$(define)  -DCOMMENT_START=$(comment_start) \
	-DCOMMENT_END=$(comment_end) -DPUSH=$(push) -DPUSH2=$(push2) \
	-DREF=$(ref) -DCODE=$(code) -DEXPAND=$(expand)  -Wall

expand: mro.c
	cat mro.c | mro > mro.expand.c

clean: mro
	rm mro.expand.c

doc: mro.mro.1
	cat mro.mro.1 | mro > mro.1
	groff -mandoc -Thtml mro.1 > README.md

install: mro
	cp mro /usr/local/bin/
