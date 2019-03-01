buffer=100
stack=10
macros=100
noprint=100

define="'@'"
code="';'"
comment_start="'%'"
comment_end="'\\n'"
push = "'\#'"
push2 = "'='"
ref = "'~'"

.FORCE:
mro: .FORCE mro.c
	cc -I /usr/include/guile/2.2 -lguile-2.2 -o mro -DMAXBUFFER=$(buffer) \
	-DMAXSTACK=$(stack) -DMAXMACROS=$(macros) -DMAXNOPRINT=$(noprint) \
	-DDEFINE=$(define)  -DCOMMENT_START=$(comment_start) \
	-DCOMMENT_END=$(comment_end) -DPUSH=$(push) -DPUSH2=$(push2) \
	-DREF=$(ref) -DCODE=$(code) -Wall mro.c


doc: README.mro
	cat README.mro | ./mro > README.md

install: mro
	cp mro /usr/local/bin/
