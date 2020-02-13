NAME    := jcompress
BIN_DIR := /usr/local/bin

all: build

build: $(NAME).c
	gcc -Wall -Wextra -o $(NAME) $< -lm -ljpeg

test: build
	./$(NAME) test/sonnet.txt

memcheck:
	gcc -o $(NAME) $(NAME).c -lm -ljpeg -g -O0
	valgrind --tool=memcheck --leak-check=yes $(NAME) test/sonnet.txt

install: build
	install -m 0755 $(NAME) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

clean:
	rm -rf $(NAME)
