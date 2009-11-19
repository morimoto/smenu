HOST	=
CROSS	=
LDFLAGS	= -static

CC	= $(CROSS)gcc
STRIP	= $(CROSS)strip

KOBJ	= kmain.o tevent.o
TOBJ	= tmain.o tevent.o

-include .config

ifneq ($(HOST),)
CROSS := ${HOST}-
endif

all: kmenu tmenu

kmenu: $(KOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(KOBJ)
	$(STRIP) $@

tmenu: $(TOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TOBJ)
	$(STRIP) $@

sh3 sh4:
	make HOST=$@-linux kmenu
	make HOST=$@-linux tmenu

clean:
	rm -fr kmenu
	rm -fr tmenu
	rm -fr *.o
