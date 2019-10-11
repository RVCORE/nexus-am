#include <am.h>
#include <riscv64.h>
#include <klib.h>

#define MAINARGS_ADDR 0x80000000

extern char _heap_start;
extern char _heap_end;
int main(const char *args);
void __am_init_uartlite(void);
void __am_uartlite_putchar(char ch);

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

void _putc(char ch) {
  __am_uartlite_putchar(ch);
}

void _halt(int code) {
  __asm__ volatile("mv a0, %0; .word 0x0005006b" : :"r"(code));

  // should not reach here during simulation
  printf("Exit with code = %d\n", code);

  // should not reach here on FPGA
  while (1);
}

void _trm_init() {
  __am_init_uartlite();
  int ret = main((const char *)MAINARGS_ADDR);
  _halt(ret);
}