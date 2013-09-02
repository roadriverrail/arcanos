#include <syscall.h>

int main (int argc, char **argv)
{
  while (1)
  {
    syscall();
  }
}
