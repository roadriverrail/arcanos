/*
 * stringformat.c
 *
 *  Created on: Aug 30, 2010
 *      Author: rhett
 */

#include <inc/stringformat.h>
#include <inc/console.h>
#include <inc/string.h>

void reverse(char* buffer) {
	int buflen = 0;
	int begin;
	int end;
	
	begin = 0;
	end = strlen(buffer)-1;
	
	if (end<0) return;
	if (end == begin) return;
	
	while(begin < end) {
		char swap;
		swap = buffer[begin];
		buffer[begin] = buffer[end];
		buffer[end] = swap;
		
		begin++;
		end--;
	}
	
}

void print_int_dec(unsigned int i) {
	int radix = 10;
	//Buffer size is based on a guess at something larger than
	//a maxint val
	char buffer [(sizeof(int)*3)+1];
	int c=0;
	do {
		buffer[c++] = i % radix + '0';
		i /= radix;
	} while (i>0);

	buffer[c] = '\0';
	reverse(buffer);
	print_string(buffer);
}

void print_int_hex(unsigned int i) {
	int radix = 16;
	//Buffer size is based on a guess at something larger than
	//a maxint val
	char buffer [(sizeof(int)*3)+1];
	int c=0;
	do {
		int hexit = i%radix;
		if (hexit<10) {
			buffer[c++] = hexit + '0';
		} else {
			hexit = hexit-10;
			buffer[c++] = hexit + 'a';
		}
		i /= radix;
	} while (i>0);

	buffer[c] = '\0';
	reverse(buffer);
	print_string(buffer);

}

void print_substring(char* c) {
	print_string(c);
}

void print_escape_char(char c) {
	switch(c) {
	case '\n':
		next_line();
		update_cursor();
		break;
	default:
		printchar(c);
	}
}

void _kern_print(const char* fmt_string, ...) {
	int i;
	int value;
	char* s;
	va_list args;

	va_start(args,fmt_string);

	for (i=0; fmt_string[i] != '\0'; i++) {
		if (fmt_string[i] == '%') {
			i++;
			switch (fmt_string[i]) {
			case 'd':
			case 'u':
				value = va_arg(args, int);
				print_int_dec(value);
				break;
			case 's':
				s = va_arg(args, char*);
				print_substring(s);
				break;
			case 'x':
				value = va_arg(args, int);
				print_int_hex(value);
				break;
			case '%':
				print_escape_char(fmt_string[i]);
				break;
			}
		} else {
			print_escape_char(fmt_string[i]);
		}
	}
	update_cursor();

	va_end(args);
}
