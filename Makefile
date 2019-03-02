stack=10
macros=100
noprint=100
page=10000

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
	cc -I $(guile_include) mro.c -l$(guile_lib) -o mro -DPAGE=$(page) \
	-DMAXSTACK=$(stack) -DMAXMACROS=$(macros) -DMAXNOPRINT=$(noprint) \
	-DDEFINE=$(define)  -DCOMMENT_START=$(comment_start) \
	-DCOMMENT_END=$(comment_end) -DPUSH=$(push) -DPUSH2=$(push2) \
	-DREF=$(ref) -DCODE=$(code) -DEXPAND=$(expand)  -Wall 


doc: README.mro.html
	cat README.mro.html | mro > README.md

install: mro
	cp mro /usr/local/bin/
