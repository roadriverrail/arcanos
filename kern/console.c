#include <inc/types.h>
#include <inc/x86.h>
#include <inc/memlayout.h>

#include <inc/console.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define X 0
#define Y 1

volatile uint8_t* textmode_base = (volatile uint8_t*)(0xB8000);
uint8_t current_color = (uint8_t)0x2a;
uint8_t current_loc[2] = {0,0};
char screen_buffer[SCREEN_WIDTH*SCREEN_HEIGHT];

void blank_screen() {
	int i=0;
	for (i=0; i<(SCREEN_WIDTH*SCREEN_HEIGHT*2); i=i+2) {
		*(textmode_base+i) = ' ';
		*(textmode_base+i+1) = current_color;
		screen_buffer[i/2] = ' ';
	}
	current_loc[X] = 0;
	current_loc[Y] = 0;
}

void buffer_to_screen() {
	int i=0;
	for (i=0; i<(SCREEN_WIDTH*SCREEN_HEIGHT*2); i=i+2) {
		*(textmode_base+i) = screen_buffer[i/2];
		*(textmode_base+i+1) = current_color;
	}
}

void console_init() {
	blank_screen();
	update_cursor();
}

void printchar(char c) {
	unsigned int offset = ((current_loc[Y]*SCREEN_WIDTH)+current_loc[X])*2;
	*(textmode_base+offset) = c;
	screen_buffer[(current_loc[Y]*SCREEN_WIDTH)+current_loc[X]] = c;
	*(textmode_base+offset+1) = current_color;
	current_loc[X]++;
	if (current_loc[X] >= SCREEN_WIDTH) {
		next_line();
	}
}

void next_line() {
	current_loc[X] = 0;
	current_loc[Y] = current_loc[Y] + 1;
	if (current_loc[Y] == SCREEN_HEIGHT) {
		int x=0;
		int y=0;
		//copy everything in the buffer one line down
		move_to(0,0);
		for (y=1; y<=SCREEN_HEIGHT; y++) {
			for (x=0; x<SCREEN_WIDTH; x++) {
				if (y == SCREEN_HEIGHT) {
					screen_buffer[((y-1)*SCREEN_WIDTH)+x] = ' ';
				} else {
					screen_buffer[((y-1)*SCREEN_WIDTH)+x] = screen_buffer[(y*SCREEN_WIDTH)+x];
				}
			}
		}
		//draw the screen from buffer
		buffer_to_screen();
		//set current_loc[Y] to the last line
		current_loc[X] = 0;
		current_loc[Y] = SCREEN_HEIGHT-1;
	}
}

void update_cursor() {
    unsigned int offset=((current_loc[Y]*SCREEN_WIDTH)+current_loc[X]);

    // This is a hack and a half.  We need to actually find the base port
    //for the VGA system by reading it from the BIOS
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(offset&0xFF));
    // cursor HIGH port to vga INDEX register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint16_t)((offset>>8)&0xFF));
}

int8_t move_to (uint8_t x, uint8_t y) {
	if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
		return -1;
	}
	current_loc[X] = x;
	current_loc[Y] = y;
	return 0;
}



void print_string_helper(char* string) {
	while (*string != '\0') {
		printchar(*string);
		string++;
	}
}

void print_string(char* string) {
	print_string_helper(string);
	update_cursor();
}

void print_line(char* string) {
	print_string_helper(string);
	next_line();
	update_cursor();
}
