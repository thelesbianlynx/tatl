SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, out/%.o, $(SOURCES))

NAME = catnip

$(NAME): $(OBJECTS)
	gcc $(OBJECTS) -o $(NAME) -lncurses

out/%.o: src/%.c | out
	gcc $< -c -o $@ -Wall -g

out:
	mkdir -p out

.PHONY: clean
clean:
	rm -r out
	rm $(NAME)
