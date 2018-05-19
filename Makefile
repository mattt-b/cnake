SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)
DEPENDS=$(wildcard src/*.d)

SHELL=/bin/sh
CFLAGS=-Wall -Wextra -pedantic -g -MD -std=c99
LDFLAGS=-lSDL2

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
INSTALL=install
INSTALL_PROGRAM=$(INSTALL) -D -s

.PHONY: all clean install uninstall

all: snake

clean:
	$(RM) snake $(OBJS) $(DEPENDS)

install: snake
	$(INSTALL_PROGRAM) snake $(DESTDIR)$(bindir)/snake

uninstall:
	$(RM) $(DESTDIR)$(bindir)/snake

snake: $(OBJS)
	$(CC) -o snake $(OBJS) $(CFLAGS) $(LDFLAGS)

-include $(OBJS:.o=.d)
