CFLAGS=-O2 -fno-builtin -fsigned-char -fsingle-precision-constant -I../include

OBJECTS=brd.o hdlcd.o mem.o pfpu.o slowout.o snd.o time.o tmu.o vga.o

all: libmmemulation.a

libmmemulation.a: $(OBJECTS)
	$(AR) clrs libmmemulation.a $(OBJECTS)

.PHONY: clean depend

depend:
	makedepend -Y -- $(CFLAGS) -- *.c

clean:
	rm -f *.o libmmemulation.a .*~ *~ Makefile.bak
# DO NOT DELETE

brd.o: ../include/board.h ../include/hal/brd.h
hdlcd.o: ../include/hal/hdlcd.h
mem.o: ../include/hal/mem.h
pfpu.o: ../include/hal/pfpu.h ../include/hw/pfpu.h
slowout.o: ../include/hal/slowout.h
snd.o: ../include/hal/snd.h ../include/hw/ac97.h
time.o: ../include/hal/time.h
tmu.o: ../include/hal/tmu.h ../include/hw/tmu.h
vga.o: ../include/hal/vga.h
