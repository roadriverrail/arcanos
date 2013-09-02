#include <syscall.h>

void syscall() {
  __asm__("int $0x80");
}
