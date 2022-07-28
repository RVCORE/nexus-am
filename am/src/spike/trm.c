#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include "htif.h"

#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

int main(const char *args);

void _trm_init() {
  int ret = main(mainargs);
  _halt(ret);
}

void _putc(char ch) {
  htif_console_putchar(ch);
}

void _halt(int code) {
  printf("Exit with code = %d\n\n", code);
  htif_poweroff();

  // should not reach here
  while (1);
}
