#define VGA_WIDTH 80;
#define VGA_HEIGHT 25;
 
uint16_t* terminal_buffer;
 
void terminal_initialize(void) 
{
	//clear screen and set addres of screen buffer
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < 2000; y++) {
			terminal_buffer[y] = ' ' | 0x0F<<8;
	}
	
	//set timeout for keyboard
	outb(0x64,0xA7);
	
	//disable cursor
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}