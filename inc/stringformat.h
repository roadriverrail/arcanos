/*
 * stringformat.h
 *
 *  Created on: Aug 30, 2010
 *      Author: rhett
 */

#ifndef STRINGFORMAT_H_
#define STRINGFORMAT_H_

#include <inc/stdarg.h>

void reverse(char*);

void print_int_dec(unsigned int i);
void print_int_hex(unsigned int i);
void print_substring(char* i);
void print_escape_char(char c);

void _kern_print(const char* fmt_string, ...);
#endif /* STRINGFORMAT_H_ */
