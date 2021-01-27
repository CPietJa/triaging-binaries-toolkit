AR?=ar
AS?=as
RANLIB?=ranlib
CC?=gcc
LD?=gcc
CFLAGS?=-Wall -Wextra  -c -I./libbele -O3
LDFLAGS?=

LIB=libelf.a

OFILES=\
	elf.o\
	print.o\
	str.o\

HFILES=\
	dat.h\
	elf.h\
	fns.h\

default: deps $(LIB)
$(LIB): $(OFILES) $(HFILES)
	$(AR) r $(LIB) $(OFILES)
	$(RANLIB) $(LIB)

deps:
	@cd libbele/ && $(MAKE)

cleandeps:
	@cd libbele/ && $(MAKE) nuke

%.o: %.c
	$(CC) $(CFLAGS) $*.c

clean:
	rm -f *.o

nuke: clean cleandeps
	rm -f $(LIB)
