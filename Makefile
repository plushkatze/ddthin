CC = gcc
LD_FLAGS = -Wl,-z,relro,-z,now

.PHONY = clean all default

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

default: ddthin
all: default

ddthin.o: main.c
	$(CC) $(LD_FLAGS) -c main.c -o ddthin.o

ddthin: ddthin.o
	$(CC) $(LD_FLAGS) ddthin.o -o ddthin

install: ddthin
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 ddthin $(DESTDIR)$(PREFIX)/bin/

clean:
	-rm -f ddthin.o
	-rm -f ddthin
