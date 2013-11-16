PREFIX ?= /usr/local

all: tag

tag: src/tag.c
	$(CC) $< -std=c99 -O3 -Wall -o $@

install: tag
	install tag $(PREFIX)/bin/tag

uninstall:
	rm -f $(PREFIX)/bin/tag

clean:
	rm -f tag