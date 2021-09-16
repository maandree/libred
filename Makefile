.POSIX:

LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)


CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk


OBJ =\
	solar.o\
	blackbody.o

LOBJ = $(OBJ:.o=.lo)


all: libred.a libred.$(LIBEXT)
solar.o: libred.h
blackbody.o: 10deg-xy.i 10deg-rgb.i libred.h
generate-table.o: blackbody.c 10deg-xy.i libred.h

10deg-xy.i: 10deg
	sed -e 's/^/{/' -e 's/ /, /' -e 's/$$/},/' < 10deg | sed '$$s/,$$//' > $@

10deg-rgb.i: generate-table 10deg
	./generate-table > $@

.o:
	$(CC) -o $@ $< $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

libred.a: $(OBJ)
	-@rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) s $@

libred.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) -o $@ $(LOBJ) $(LDFLAGS)

install: libred.a libred.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man0"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- libred.a "$(DESTDIR)$(PREFIX)/lib"
	cp -- libred.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libred.$(LIBMINOREXT)"
	ln -sf -- libred.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libred.$(LIBMAJOREXT)"
	ln -sf -- libred.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libred.$(LIBEXT)"
	cp -- libred.h "$(DESTDIR)$(PREFIX)/include"
	cp -- libred.h.0 "$(DESTDIR)$(MANPREFIX)/man0"
	cp -- libred_check_timetravel.3 libred_get_colour.3 libred_solar_elevation.3 "$(DESTDIR)$(MANPREFIX)/man3"
	cp -- libred.7 "$(DESTDIR)$(MANPREFIX)/man7"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libred.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libred.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libred.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libred.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libred.h"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man0/libred.h.0"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libred_check_timetravel.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libred_get_colour.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libred_solar_elevation.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man7/libred.7"

clean:
	-rm -f -- generate-table *.i *.o *.a *.lo *.su *.so *.dll *.dylib

.SUFFIXES:
.SUFFIXES: .c .o .lo

.PHONY: all check install uninstall clean
