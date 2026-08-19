SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)

OBJECTS = $(SOURCES:src/%.c=out/%.o) #$(patsubst src/%.c, out/%.o, $(SOURCES))
DEPS    = $(SOURCES:src/%.c=out/%.d)

NAME = tatl

PREFIX ?= /usr/local
BINDIR ?= $(DESTDIR)$(PREFIX)/bin

$(NAME): $(OBJECTS)
	gcc $(OBJECTS) -o $(NAME) -lm -lncurses

out/%.o: src/%.c | out
	gcc $< -std=gnu11 -c -o $@ -Wall -Wextra -Wno-sign-compare -Wno-unused -Wshadow -g -MMD -MP

out:
	mkdir -p out

.PHONY: clean install uninstall
clean:
	rm -r out
	rm $(NAME)

install: $(NAME)
	mkdir -p $(BINDIR)
	cp -f $(NAME) $(BINDIR)
	chmod 755 $(BINDIR)/$(NAME)

uninstall:
	rm -f $(BINDIR)/$(NAME)

-include $(DEPS)
