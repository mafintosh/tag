# tag

A small C program that tags messages piped to stdin.

## Usage

tag is useful for tagging various streams (including adding time stamps to log files).
It has a very small memory footprint making it ideal for long running processes

```
echo world | tag hello     # prints hello world
echo world | tag %d hello  # prints [date stamp] hello world
echo world | tag %h hello  # prints [hostname] hello world
echo world | tag %l %h     # prints [line-number] [hostname]
```

## Installation

Either clone the repo and do

	make install

Or install via clib

	clib install mafintosh/tag.c

## License

MIT