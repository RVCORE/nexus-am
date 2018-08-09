#include <am-x86.h>
#include <stdarg.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

int _asye_init(_Context *(*handler)(_Event, _Context *)) {
  static GateDesc idt[NR_IRQ];
  ioapic_enable(IRQ_KBD, 0);

  // init IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i]  = GATE(STS_TG32, KSEL(SEG_KCODE), irqall, DPL_KERN);
  }
  // --------------------- exceptions --------------------------
  idt[0]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec0,   DPL_KERN);
  idt[1]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec1,   DPL_KERN);
  idt[2]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec2,   DPL_KERN);
  idt[3]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec3,   DPL_KERN);
  idt[4]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec4,   DPL_KERN);
  idt[5]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec5,   DPL_KERN);
  idt[6]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec6,   DPL_KERN);
  idt[7]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec7,   DPL_KERN);
  idt[8]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec8,   DPL_KERN);
  idt[9]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec9,   DPL_KERN);
  idt[10]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec10,  DPL_KERN);
  idt[11]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec11,  DPL_KERN);
  idt[12]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec12,  DPL_KERN);
  idt[13]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec13,  DPL_KERN);
  idt[14]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec14,  DPL_KERN);
  // --------------------- interrupts --------------------------
  idt[32]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq0,   DPL_KERN);
  idt[33]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq1,   DPL_KERN);
  idt[46]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq14,  DPL_KERN);
  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_USER);
  set_idt(idt, sizeof(idt));

  user_handler = handler; // global (unique) interrupt/exception handler

  return 0;
}

static void panic_on_return() {
  panic("kernel context returns");
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *ctx = (_Context *)stack.start;
  ctx->eax = ctx->ebx = ctx->ecx = ctx->edx = 0;
  ctx->esi = ctx->edi = ctx->ebp = ctx->esp3 = 0;

  ctx->ss0 = 0; // only used for ring3 procs
  ctx->esp0 = (uint32_t)stack.end;
  ctx->cs = KSEL(SEG_KCODE);
  ctx->ds = ctx->es = ctx->ss = KSEL(SEG_KDATA);
  ctx->eip = (uint32_t)entry;
  ctx->eflags = FL_IF;

  uint32_t **esp = (uint32_t **)&ctx->esp0;
  *(*esp -= 1) = (uint32_t)arg; // argument
  *(*esp -= 1) = (uint32_t)panic_on_return; // return address
  return ctx;
}

void _yield() {
  asm volatile("int $0x80" : : "a"(-1));
}

int _intr_read() {
  return (get_efl() & FL_IF) != 0;
}

void _intr_write(int enable) {
  if (enable) {
    sti();
  } else {
    cli();
  }
}

#define IRQ    T_IRQ0 + 
#define MSG(m) : ev.msg = m;

void irq_handle(struct TrapFrame *tf) {
  // Saving processor context
  _Context ctx = {
    .eax = tf->eax, .ebx = tf->ebx, .ecx = tf->ecx, .edx = tf->edx,
    .esi = tf->esi, .edi = tf->edi, .ebp = tf->ebp, .esp3 = 0,
    .eip = tf->eip, .eflags = tf->eflags,
    .cs  = tf->cs,  .ds = tf->ds,   .es = tf->es,   .ss = 0,
    .ss0 = 0, .esp0 = 0,
  };

  if (tf->cs & DPL_USER) { // interrupt at user code
    ctx.esp0 = (uint32_t)(tf + 1); // tf is verything saved on the stack
    ctx.ss = tf->ss;
    ctx.esp3 = tf->esp;
    ctx.ss0 = KSEL(SEG_KDATA);
  } else { // interrupt at kernel code
    // tf (without ss0/esp0) is everything saved on the stack
    ctx.ss0 = KSEL(SEG_KDATA);
    ctx.esp0 = ((uint32_t)(tf + 1)) - sizeof(uint32_t) * 2;
  }

  // Sending end-of-interrupt
  if (IRQ 0 <= tf->irq && tf->irq < IRQ 32) {
    lapic_eoi();
  }

  // Creating an event
  _Event ev = {
    .event = _EVENT_NULL,
    .cause = 0, .ref = 0,
    .msg = "(no message)",
  };
  
  switch (tf->irq) {
    case IRQ 0 MSG("timer interrupt (lapic)")
      ev.event = _EVENT_IRQ_TIMER; break;
    case IRQ 1 MSG("I/O device IRQ1 (keyboard)")
      ev.event = _EVENT_IRQ_IODEV; break;
    case EX_SYSCALL MSG("int $0x80 trap: _yield() or system call")
      if ((int32_t)tf->eax == -1) {
        ev.event = _EVENT_YIELD;
      } else {
        ev.event = _EVENT_SYSCALL;
      }
      break;
    case EX_DIV MSG("divide by zero")
      ev.event = _EVENT_ERROR; break;
    case EX_UD MSG("UD #6 invalid opcode")
      ev.event = _EVENT_ERROR; break;
    case EX_NM MSG("NM #7 coprocessor error")
      ev.event = _EVENT_ERROR; break;
    case EX_DF MSG("DF #8 double fault")
      ev.event = _EVENT_ERROR; break;
    case EX_TS MSG("TS #10 invalid TSS")
      ev.event = _EVENT_ERROR; break;
    case EX_NP MSG("NP #11 segment/gate not present")
      ev.event = _EVENT_ERROR; break;
    case EX_SS MSG("SS #12 stack fault")
      ev.event = _EVENT_ERROR; break;
    case EX_GP MSG("GP #13, general protection fault")
      ev.event = _EVENT_ERROR; break;
    case EX_PF MSG("PF #14, page fault, @cause: _PROT_XXX")
      ev.event = _EVENT_PAGEFAULT;
      if (tf->err & 0x1) ev.cause |= _PROT_NONE;
      if (tf->err & 0x2) ev.cause |= _PROT_WRITE;
      else               ev.cause |= _PROT_READ;
      ev.ref = get_cr2();
      break;
    default MSG("unrecognized interrupt/exception")
      ev.event = _EVENT_ERROR;
      ev.cause = tf->err;
      break;
  }

  // Call user handlers (registered in _asye_init)
  _Context *ret_ctx = &ctx;
  if (user_handler) {
    _Context *next = user_handler(ev, &ctx);
    if (!next) {
      panic("return to a null context");
    }
    ret_ctx = next;
  }

  // Return to context @ret_ctx
#define REGS_KERNEL(_) \
  _(eflags) _(cs) _(eip) _(ds) _(es) \
  _(eax) _(ecx) _(edx) _(ebx) _(esp0) _(ebp) _(esi) _(edi)
#define REGS_USER(_) \
  _(ss) _(esp3) REGS_KERNEL(_)
#define push(r) "push %[" #r "];"      // -> push %[eax]
#define def(r)  , [r] "m"(ret_ctx->r)  // -> [eax] "m"(ret_ctx->eax)
 
  if (ret_ctx->cs & DPL_USER) { // return to user
    cpu_setustk(ret_ctx->ss0, ret_ctx->esp0);
    asm volatile goto(
      "movl %[esp], %%esp;" // move stack
      REGS_USER(push)       // push reg context onto stack
      "jmp %l[iret]"        // goto iret
    : : [esp] "m"(ret_ctx->esp0)
        REGS_USER(def) : : iret );
  } else { // return to kernel
    asm volatile goto(
      "movl %[esp], %%esp;" // move stack
      REGS_KERNEL(push)     // push reg context onto stack
      "jmp %l[iret]"        // goto iret
    : : [esp] "m"(ret_ctx->esp0)
        REGS_KERNEL(def) : : iret );
  }
iret:
  asm volatile(
    "popal;"     // restore context
    "popl %es;"
    "popl %ds;"
    "iret;"      // interrupt return
  );
}