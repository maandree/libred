.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

all: libred.a
solar.o: solar.c libred.h
blackbody.o: blackbody.c 10deg-xy.i 10deg-rgb.i libred.h

10deg-xy.i: 10deg
	sed -e 's/^/{/' -e 's/ /, /' -e 's/$$/},/' < 10deg | sed '$$s/,$$//' > $@

generate-table: generate-table.c blackbody.c 10deg-xy.i libred.h
	$(CC) -o $@ generate-table.c $(CPPFLAGS) $(CFLAGS) $(LDFLAGS)

10deg-rgb.i: generate-table 10deg
	./generate-table > $@

.c.o:
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

libred.a: solar.o blackbody.o
	$(AR) rc $@ $?
	$(AR) s $@

install: libred.a
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man0"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- libred.a "$(DESTDIR)$(PREFIX)/lib"
	cp -- libred.h "$(DESTDIR)$(PREFIX)/include"
	cp -- libred.h.0 "$(DESTDIR)$(MANPREFIX)/man0"
	cp -- libred_check_timetravel.3 libred_get_colour.3 libred_solar_elevation.3 "$(DESTDIR)$(MANPREFIX)/man3"
	cp -- libred.7 "$(DESTDIR)$(MANPREFIX)/man7"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libred.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libred.h"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man0/libred.h.0"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libred_check_timetravel.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libred_get_colour.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libred_solar_elevation.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man7/libred.7"

clean:
	-rm -f -- generate-table *.i *.o *.a *.lo *.su *.so

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all check install uninstall clean
