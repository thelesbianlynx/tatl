SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, out/%.o, $(SOURCES))

NAME = tatl

PREFIX ?= /usr/local
BINDIR ?= $(DESTDIR)$(PREFIX)/bin

$(NAME): $(OBJECTS)
	gcc $(OBJECTS) -o $(NAME) -lm -lncurses

out/%.o: src/%.c | out
	gcc $< -c -o $@ -Wall -g

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
