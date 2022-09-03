#include <rvh_test.h>

bool (*entry)() = NULL;

static const char *tests[256] = {
  ['a'] = "two-stage address translation",
};

int main(const char *args) {
  reset_state();
  switch (args[0]) {
    CASE('a', two_stage_translation);
    case 'H':
    default:
      printf("Usage: make run mainargs=*\n");
      for (int ch = 0; ch < 256; ch++) {
        if (tests[ch]) {
          printf("  %c: %s\n", ch, tests[ch]);
        }
      }
  }
  return 0;
}
