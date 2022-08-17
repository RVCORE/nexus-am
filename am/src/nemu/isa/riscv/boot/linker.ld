ENTRY(_start)

SECTIONS {
  . = 0x80000000;
  .text.init : { *(.text.init) }
  . = ALIGN(0x1000);
  .text : {
    *(entry)
    *(.text)
  }
  . = ALIGN(0x1000);
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  . = ALIGN(0x1000);
  .data : {
    *(.data)
  }
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  }
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x8000;
  _stack_pointer = .;
  end = .;
  _end = .;
  _heap_start = ALIGN(0x1000);
}