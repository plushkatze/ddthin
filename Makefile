CC = gcc
LD_FLAGS = -Wl,-z,relro,-z,now

.PHONY = clean all default

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

default: ddthin ddthin.1.gz
all: default

ddthin.o: main.c
	$(CC) $(LD_FLAGS) -c main.c -o ddthin.o

ddthin: ddthin.o
	$(CC) $(LD_FLAGS) ddthin.o -o ddthin

ddthin.1.gz: ddthin.1
	gzip -k ddthin.1

install: ddthin
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 ddthin $(DESTDIR)$(PREFIX)/bin/
	install -m 644 ddthin.1.gz $(DESTDIR)/usr/share/man/man1/

clean:
	-rm -f ddthin.o
	-rm -f ddthin
	-rm -f ddthin.1.gz
