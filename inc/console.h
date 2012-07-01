#include <inc/types.h>

void blank_screen();

void buffer_to_screen();

void console_init();

void printchar(char c);

void next_line();

void update_cursor();
int8_t move_to (uint8_t x, uint8_t y);


void print_string_helper(char* string);

void print_string(char* string);

void print_line(char* string);
