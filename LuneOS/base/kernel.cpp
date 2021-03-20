#include <stddef.h>
#include <stdint.h>
#include <lib/std.cpp>
#include <lib/io.cpp>
#include <lib/term.cpp>
#include <lib/disk_ide.cpp>
extern uint8_t mem[];
#include <lib/fs.cpp>
#include <lib/vm_ldi.cpp>

extern "C" void kernel_main(void) 
{	
	terminal_initialize();
	ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
	INIT_FS();
	VM_INIT();
	
	VM_START();
	
	terminal_buffer[1999]='@'|0x1F<<8;
}


#include <lib/mem.cpp>