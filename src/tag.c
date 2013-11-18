#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct format {
	int offset;
	void (*format)(int offset);
	struct format *next;
};

static char stamp[1024];
static int stamp_len = 0;
static struct format *fmt = NULL;

static int line_count = 0;

void
format_line_count (int offset) {
	char last = stamp[offset+5];
	sprintf(stamp+offset, "%05d", line_count++);
	stamp[offset+5] = last;
}

void
format_date (int offset, bool utc) {
	time_t ltime;
	struct tm *tm;

	time(&ltime);
	tm = (utc ? gmtime : localtime)(&ltime);

	char last = stamp[offset+19];
	sprintf(stamp+offset, "%04d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year+1900,
		tm->tm_mon+1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec
	);
	stamp[offset+19] = last;
}

void
format_utc_date (int offset) {
	format_date(offset, true);
}

void
format_local_date (int offset) {
	format_date(offset, false);
}

void
apply_format () {
	struct format *next = fmt;
	while (next != NULL) {
		next->format(next->offset);
		next = next->next;
	}
}

int
compile_format (int len, char format[]) {
	bool formatting = false;
	struct format *next = fmt;

	for (int i = 0; i < len; i++) {
		char ch = format[i];

		if (ch == '%') {
			formatting = true;
			continue;
		}
		if (!formatting) {
			stamp[stamp_len++] = ch;
			continue;
		}

		formatting = false;

		if (ch == 'h') {
			char hostname[1024];
			gethostname(hostname, 1024);
			for (int j = 0; hostname[j] != '\0'; j++) stamp[stamp_len++] = hostname[j];
			continue;
		}

		if (next) next = next->next = malloc(sizeof(struct format));
		else fmt = next = malloc(sizeof(struct format));

		next->offset = stamp_len;
		next->next = NULL;

		if (ch == 'd') {
			next->format = format_utc_date;
			stamp_len += 19;
			continue;
		}
		if (ch == 'D') {
			next->format = format_local_date;
			stamp_len += 19;
			continue;
		}
		if (ch == 'l') {
			next->format = format_line_count;
			stamp_len += 5;
			continue;
		}

		return -1;
	}

	stamp[stamp_len++] = ' ';
	stamp[stamp_len++] = '\0';
	return 0;
}

void
error(char *msg) {
	write(2, msg, strlen(msg));
	exit(1);
}

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		error(
			"\n"
			"  Usage: tag [tags]\n"
			"  - tag all lines piped to stdin with a message\n"
			"\n"
			"  Examples:\n"
			"    tag hello-world  # -> hello-world\n"
			"    tag %h [%d]      # -> hostname [YYYY-MM-DD HH:mm:ss UTC]\n"
			"    tag %D           # -> YYYY-MM-DD HH:mm:ss in local time\n"
			"    tag %l test      # -> line-number test\n"
			"\n"
		);
	}

	for (int i = 1; i < argc; i++) {
		if (compile_format(strlen(argv[i]), argv[i]) < 0) error("Invalid format\n");
	}

	char prev = '\n';
	char buf[1024];
	int len;

	while ((len = read(0, &buf, 1024)) > 0) {
		int offset = 0;

		for (int i = 0; i < len; i++) {
			if (prev == '\n') {
				apply_format();
				if (write(1, stamp, stamp_len) < 0) exit(1);
			}
			if (buf[i] == '\n') {
				if (write(1, buf+offset, i-offset+1) < 0) exit(1);
				offset = i+1;
			}
			prev = buf[i];
		}

		if (offset < len) {
			if (write(1, buf+offset, len-offset) < 0) exit(1);
		}
	}

	return 0;
}
