CFLAGS=-O6 -Wall -Wstrict-prototypes -fPIC
#CFLAGS+=$(shell gcc -c -O6 tt.c && rm -f tt.o )
CC=gcc
LIBZAPSO=libzap.so.1.0
LIBZAPA=libzap.a
INSTALL_PREFIX=

OBJS=zap.o goertzel.o filtros.o demodulador.o serie.o baudot.o dpll.o tone_detect.o


all:	libzap.a libzap.so zaptest-incoming zaptest-outgoing zaptest-chunk2alsa usbfxstest answering_machine fxstest

$(LIBZAPA): $(OBJS)
	ar rcs $(LIBZAPA) $(OBJS)
	ranlib $(LIBZAPA)

$(LIBZAPSO): $(OBJS)
	$(CC) -shared -Wl,-soname,libzap.so.1 -o $(LIBZAPSO) $(OBJS) -lc -lm

libzap.so.1: $(LIBZAPSO)
	/sbin/ldconfig -n .

libzap.so: libzap.so.1
	rm -f libzap.so
	ln -sf libzap.so.1 libzap.so

mkfilter/mkfilter:
	make -C mkfilter 

mkcoef: mkfilter/mkfilter mkcoef.c
	$(CC) -o mkcoef mkcoef.c

coef:	mkcoef
	./mkcoef

install: all
	install -m 644 libzap.a $(INSTALL_PREFIX)/usr/lib
	install -m 644 zap.h $(INSTALL_PREFIX)/usr/include
	install -m 755 libzap.so.1 $(INSTALL_PREFIX)/usr/lib
	( cd $(INSTALL_PREFIX)/usr/lib ; ln -sf libzap.so.1 libzap.so )

tt:	tt.c  libzap.a
	cc tt.c -o tt -L. ./libzap.a -lm

zaptest-incoming: zaptest-incoming.c libzap.a
	cc -o zaptest-incoming -I. zaptest-incoming.c ./libzap.a -lm

zaptest-outgoing: zaptest-outgoing.c libzap.a
	cc -o zaptest-outgoing -I. zaptest-outgoing.c ./libzap.a -lm

zaptest-chunk2alsa: zaptest-chunk2alsa.c libzap.a
	cc -o zaptest-chunk2alsa -I. zaptest-chunk2alsa.c ./libzap.a -lm

usbfxstest: usbfxstest.c libzap.a
	cc -o usbfxstest -I. usbfxstest.c ./libzap.a -lm

answering_machine: answering_machine.c libzap.a
	cc -o answering_machine -I. answering_machine.c ./libzap.a -lm

fxstest: fxstest.c
	cc -o fxstest fxstest.c -ltonezone

clean:
	rm -f *.o
	rm -f libzap.a tt mkcoef p_tmp_ libzap.so*
	rm -f zaptest-outgoing zaptest-incoming zaptest-cw zaptest-chunk2alsa
	rm -f usbfxstest answering_machine fxstest
	rm -f *~ *.ul DEADJOE
	make -C mkfilter clean
