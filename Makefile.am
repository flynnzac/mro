SUFFIXES = .pshm
.pshm.c:
	cat $< | pushmac > $@

bin_PROGRAMS = pushmac

if PUSHMAC
pushmac_SOURCES = pushmac.pshm
else
pushmac_SOURCES = pushmac-bootstrap.c
endif


