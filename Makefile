CC ?= gcc
LDFLAGS ?= -Wl,-z,relro,-z,now
PREFIX ?= /usr/local

VERSION ?= ddthin-debug

.PHONY = clean all default

default: ddthin
all: default

ddthin: ddthin.o

install: ddthin
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 ddthin $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)$(PREFIX)/share/man/man1/
	install -m 644 ddthin.1 $(DESTDIR)$(PREFIX)/share/man/man1/

release: ddthin.c LICENSE ddthin.1 PKGBUILD README.md Makefile
	mkdir $(VERSION)
	cp ddthin.c $(VERSION)
	cp LICENSE $(VERSION)
	cp ddthin.1 $(VERSION)
	cp PKGBUILD $(VERSION)
	cp README.md $(VERSION)
	cp Makefile $(VERSION)
	tar cfz $(VERSION).tar.gz $(VERSION)

clean:
	-rm -f ddthin.o
	-rm -f ddthin
