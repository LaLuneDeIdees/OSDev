
uint8_t readBufferDisk[512];
uint32_t rootCellAddr=0;
uint8_t nowDisk=0;
uint32_t nowDirAdr=0;

void readSector(uint32_t addr){
read_sectors_lba(nowDisk,addr,1,&readBufferDisk[0]);
}

void INIT_FS(){
	for(int i =0;i<8;i++){
		nowDisk=i;
		readSector(0);
		rootCellAddr = *(uint32_t*)&readBufferDisk[0x1D6];
		readSector(rootCellAddr);
		if(*(uint32_t*)&readBufferDisk[4] == *(uint32_t*)(uint8_t*)("ROOT")){
			break;
		}else if(i==7){
			while(true);
		}
	}
	nowDirAdr=rootCellAddr;
}


void open(uint8_t* name, uint32_t adr){
	int i=0;uint32_t fileaddr=0;
	readSector(nowDirAdr);
	
	for(i =0;i<30;i++){
		if(readBufferDisk[16+i*16]!='F')continue;
		if(!equals(name,&readBufferDisk[16*(i+1)+8],8))continue;
		fileaddr= *(uint32_t*)&readBufferDisk[16*(i+1)+4];
		break;
	}
	
	if(i==30)return;
	
	while(true){
		readSector(fileaddr);
		for(int i=0;i<480;i++){
			mem[adr++]=readBufferDisk[i+16];
		}
		fileaddr=*(uint32_t*)&readBufferDisk[508];
		if(fileaddr==0xFFFFFFFF){
			break;
		}
	}
}
