CC=gcc

OPT=-O2
DEBUG=-g

CFLAGS=-fPIC $(OPT) $(DEBUG) -Wall `gtk-config --cflags` `xmms-config --cflags`
LIBS=`gtk-config --libs` `xmms-config --libs`

PLUGIN=libmpris2.so
PLUGINDIR=`xmms-config --general-plugin-dir`

all : $(PLUGIN)

install : $(PLUGIN)
	mkdir -p $(PLUGINDIR)
	install $(PLUGIN) $(PLUGINDIR)

$(PLUGIN) : mpris2.o mpris-object.o
	$(CC) $(LIBS) -shared $? -o $@

.c.o :
	gdbus-codegen --interface-prefix org.mpris. --c-namespace Mpris --generate-c-code mpris-object mpris2.xml
	$(CC) $(CFLAGS) -c $? -o $@

clean :
	rm -f $(PLUGIN) *.o
	rm -f *-obj*.c
	rm -f *-obj*.h
