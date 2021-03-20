#include <stddef.h>
#include <stdint.h>
extern uint8_t mem[];
#include <lib/vm_ldi.cpp>

extern "C" void kernel_main(void) 
{	
	VM_RUN();
}


#include <lib/mem.cpp>