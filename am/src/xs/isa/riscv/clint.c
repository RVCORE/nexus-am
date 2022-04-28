#include <xs.h>


typedef struct {
    uintptr_t mtimecmp;
    uintptr_t time_inc;
    uintptr_t temp[3];
} ClintInfo;

ClintInfo timer_handle;

#ifdef __ARCH_RISCV64_NOOP
#define CLINT_MMIO (RTC_ADDR - 0xbff8)
#define TIME_INC 0x80000
#else
#define CLINT_MMIO 0xa2000000
#define TIME_INC 0x800
#endif
#define CLINT_MTIMECMP (CLINT_MMIO + 0x4000)

/*
 * Note that timer interrupt is always triggered under machine mode
 * Machine mode interrupt handler redirects timer interrupt to supervisor
 */

/*
 * set timer increase value
 */
void set_timer_inc(uintptr_t inc) {
    timer_handle.time_inc = inc;
}

/*
 * timer initialize
 * set interrupt handler
 */
void init_timer() {
    timer_handle.mtimecmp = CLINT_MTIMECMP;
    set_timer_inc(TIME_INC);
    asm volatile("csrw mscratch, %0" : : "r"(&timer_handle));
}
/*
 * enable machine mode timer interrupt
 */
void enable_timer() {
  // set machine timer interrupt
  asm volatile("csrs mie, %0" : : "r"((1 << 7) | (1 << 1)));
}

/*
 * disable machine mode timer interrupt
 */
void disable_timer() {
  // unset machine timer interrupt
  asm volatile("csrc mie, %0" : : "r"((1 << 7) | (1 << 1)));
}