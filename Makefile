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

pushmac: pushmac.expand.c 
	cc -I $(guile_include) pushmac.expand.c -l$(guile_lib) -o pushmac \
	-DPAGE_MACRO=$(page_macro) \
	-DPAGE_BUFFER=$(page_buffer) \
	-DPAGE_STACK=$(page_stack)  \
	-DSHELL=$(shell) \
	-DDEFINE=$(define)  \
	-DPUSH=$(push) -DPUSH2=$(push2) \
	-DREF=$(ref) -DCODE=$(code) -DEXPAND=$(expand)  -Wall

pushmac.expand.c: pushmac.c
	cat pushmac.c | pushmac > pushmac.expand.c

clean: pushmac
	rm pushmac.expand.c

doc: pushmac.pushmac.1
	cat pushmac.pushmac.1 | pushmac > pushmac.1
	groff -mandoc -Thtml pushmac.1 > README.md

install: pushmac
	cp pushmac /usr/local/bin/
