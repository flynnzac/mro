macros=1000
noprint=100
page_buffer=10000
page_stack=100

define="'@'"
code="';'"
comment_start="'%'"
comment_end="'\\n'"
push = "'\#'"
push2 = "'='"
ref = "'~'"
expand = 36
guile_include = /usr/include/guile/2.2
guile_lib = guile-2.2

.FORCE:
mro: .FORCE mro.c
	cc -I $(guile_include) mro.c -l$(guile_lib) -o mro \
	-DPAGE_BUFFER=$(page_buffer) \
	-DPAGE_STACK=$(page_stack) -DMAXMACROS=$(macros) \
	-DMAXNOPRINT=$(noprint) \
	-DDEFINE=$(define)  -DCOMMENT_START=$(comment_start) \
	-DCOMMENT_END=$(comment_end) -DPUSH=$(push) -DPUSH2=$(push2) \
	-DREF=$(ref) -DCODE=$(code) -DEXPAND=$(expand)  -Wall 


doc: README.mro.html
	cat README.mro.html | mro > README.md
	cat mro.mro.1 | mro > mro.1

install: mro
	cp mro /usr/local/bin/
