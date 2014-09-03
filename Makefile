NAME    := jcompress
BUILD   := build
EXEC    := $(BUILD)/$(NAME)
BIN_DIR := /usr/local/bin

all: build

build: $(NAME).c
	mkdir -p $(BUILD)
	gcc -Wall -Wextra -o $(EXEC) $< -lm -ljpeg

test: build
	./$(EXEC) test/sonnet.txt

memcheck:
	gcc -o $(NAME) $(NAME).c -lm -ljpeg -g -O0
	valgrind --tool=memcheck --leak-check=yes ./$(NAME) test/sonnet.txt

install: build
	install -m 0755 $(EXEC) $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/$(NAME)

clean:
	rm -rf $(BUILD)
