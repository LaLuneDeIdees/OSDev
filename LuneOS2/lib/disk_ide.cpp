#define CMD_READ 0x20
#define CMD_WRITE 0x30
#define BSY_FLAG 0x80
 
// 24bit LBA (128GB max addressing)
// Reads up to 255 sectors at once into dest starting at start_lba (logical block address aka sector index)
void read_sectors_lba(uint8_t drive_num, uint32_t start_lba, uint8_t sector_count, uint8_t *dest)
{
    uint16_t index, max, base, *buffer = (uint16_t *)dest;
    uint8_t  drive = 0x40;
 
    switch(drive_num){
        case 0: case 1: base = 0x1F0; break;
        case 2: case 3: base = 0x170; break;
        case 4: case 5: base = 0x1E8; break;
        case 6: case 7: base = 0x168; break;
        default:
            // error
            return;
    }
    if(drive_num % 2){
        drive |= 0x10;
    }
	
	while(inb(base + 7) & BSY_FLAG);
 
    outb(base + 2, sector_count);
    outb(base + 3, (uint8_t)((start_lba & 0x000000FF)      ));
    outb(base + 4, (uint8_t)((start_lba & 0x0000FF00) >>  8));
    outb(base + 5, (uint8_t)((start_lba & 0x00FF0000) >> 16));
    outb(base + 6, (uint8_t)((start_lba & 0x0F000000) >> 24) | drive);
    outb(base + 7, CMD_READ);
 
    while(inb(base + 7) & BSY_FLAG);
 
    max = (sector_count * (512 / 2));
    for(index = 0; index < max; index++){
        *buffer++ = inw(base + 0);
    }
}