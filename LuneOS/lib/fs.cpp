
uint8_t readBufferDisk[512];
uint32_t rootCellAddr=0;
uint8_t nowDisk=0;
uint32_t nowDirAdr=0;

void readSector(uint32_t addr){
read_sectors_lba(nowDisk,addr,1,&readBufferDisk[0]);
}
void writeSector(uint32_t addr){
write_sectors_lba(nowDisk,addr,1,&readBufferDisk[0]);
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
	
	//while(true);
	
}
void createF(uint8_t* name, uint32_t addrin, uint8_t sizeInCell){
	readSector(nowDirAdr);
	uint8_t filecell=0;
	while(true){
		if(readBufferDisk[16*(filecell+1)]==0){
			break;
		}
		filecell++;
		if(filecell==30)return;
	}
	*(uint32_t*)&readBufferDisk[16*(filecell+1)] = *(uint32_t*)(uint8_t*)"FILE";
	for(int i =0;i<8;i++)readBufferDisk[16*(filecell+1)+8+i]=name[i];
	writeSector(nowDirAdr);
	uint32_t lastFileAddrLab = 0xFFFFFFFF;
	uint32_t FileAddrLab = nowDirAdr+1;
	
	readSector(FileAddrLab);
	while(readBufferDisk[3]==0xff){FileAddrLab++;readSector(FileAddrLab);}
	
	readSector(nowDirAdr);
	*(uint32_t*)&readBufferDisk[16*(filecell+1)+4]=FileAddrLab;
	writeSector(nowDirAdr);
	//return;
	//readSector(FileAddrLab);
	
	for(int i =0;i<sizeInCell;i++){
		readSector(FileAddrLab);
		*(uint32_t*)&readBufferDisk[0] = 0xFFFFFFFF;
		for(uint16_t k=0;k<480;k++)readBufferDisk[16+k]=mem[addrin++];
		lastFileAddrLab=FileAddrLab;
		writeSector(FileAddrLab);
		
		while(readBufferDisk[3]==0xff){FileAddrLab++;readSector(FileAddrLab);}
		
		readSector(lastFileAddrLab);
		*(uint32_t*)&readBufferDisk[508]=FileAddrLab;
		writeSector(lastFileAddrLab);
	}
	
	*(uint32_t*)&readBufferDisk[508]=0xFFFFFFFF;
	writeSector(lastFileAddrLab);
	
}

void removeF(uint8_t* name){
	uint32_t fileAddrLab;
	uint8_t i=0;
	
	readSector(nowDirAdr);
	for(i=0;i<30;i++){
		if(readBufferDisk[16+i*16]!='F')continue;
		if(!equals(name,&readBufferDisk[16*(i+1)+8],8))continue;
		fileAddrLab= *(uint32_t*)&readBufferDisk[16*(i+1)+4];
		break;
	}
	if(i==30)return;
	*(uint32_t*)&readBufferDisk[16*(i+1)]=0x0;
	writeSector(nowDirAdr);
	
	while(true){
		readSector(fileAddrLab);
		*(uint32_t*)&readBufferDisk[0]=0x0;
		writeSector(fileAddrLab);
		fileAddrLab=*(uint32_t*)&readBufferDisk[508];
		if(fileAddrLab==0xFFFFFFFF){
			break;
		}
	}
}

void createD(uint8_t* name){
	readSector(nowDirAdr);
	uint8_t filecell=0;
	while(true){
		if(readBufferDisk[16*(filecell+1)]==0){
			break;
		}
		filecell++;
		if(filecell==30)return;
	}
	*(uint32_t*)&readBufferDisk[16*(filecell+1)] = *(uint32_t*)(uint8_t*)"_DIR";
	for(int i =0;i<8;i++)readBufferDisk[16*(filecell+1)+8+i]=name[i];
	writeSector(nowDirAdr);
	uint32_t FileAddrLab = nowDirAdr+1;
	
	readSector(FileAddrLab);
	while(readBufferDisk[3]==0xff){FileAddrLab++;readSector(FileAddrLab);}
	
	*(uint32_t*)&readBufferDisk[0] = 0xFFFFFFFF;
	*(uint32_t*)&readBufferDisk[508]=0xFFFFFFFF;
	for(int i =0;i<30;i++){
		*(uint32_t*)&readBufferDisk[16*(i+1)]=0x0;
	}
	writeSector(FileAddrLab);
	
	readSector(nowDirAdr);
	*(uint32_t*)&readBufferDisk[16*(filecell+1)+4]=FileAddrLab;
	writeSector(nowDirAdr);
}

void removeD(uint8_t* name){
	uint32_t fileAddrLab;
	uint8_t i=0;
	
	readSector(nowDirAdr);
	for(i=0;i<30;i++){
		if(readBufferDisk[16+i*16]!='_')continue;
		if(!equals(name,&readBufferDisk[16*(i+1)+8],8))continue;
		fileAddrLab= *(uint32_t*)&readBufferDisk[16*(i+1)+4];
		break;
	}
	if(i==30)return;
	*(uint32_t*)&readBufferDisk[16*(i+1)]=0x0;
	writeSector(nowDirAdr);
	
	readSector(fileAddrLab);
	*(uint32_t*)&readBufferDisk[0]=0x0;
	writeSector(fileAddrLab);
	uint32_t lastDir=nowDirAdr;
	nowDirAdr=fileAddrLab;
	
	for(i=0;i<30;i++){
		if(readBufferDisk[16+i*16]==0)continue;
		if(readBufferDisk[16+i*16]=='_'){
			removeD(&readBufferDisk[16*(i+1)+8]);
		}else if(readBufferDisk[16+i*16]=='F'){
			removeF(&readBufferDisk[16*(i+1)+8]);
		}
		break;
	}
	
	nowDirAdr=lastDir;
	
}

void changeD(uint8_t* name){
	uint8_t i=0;
	
	readSector(nowDirAdr);
	for(i=0;i<30;i++){
		if(readBufferDisk[16+i*16]!='_')continue;
		if(!equals(name,&readBufferDisk[16*(i+1)+8],8))continue;
		nowDirAdr= *(uint32_t*)&readBufferDisk[16*(i+1)+4];
		break;
	}
	if(i==30)return;
}
void resetD(){
	nowDirAdr=rootCellAddr;
}

void srcD(uint8_t* FC){
	readSector(nowDirAdr);
		if(readBufferDisk[16+FC[9]*16]==0){
			FC[1]=0xFF;
			return;
		}
		for(int k=0;k<8;k++)FC[k+1]=readBufferDisk[16+FC[9]*16+8+k];
		if(readBufferDisk[16*(FC[9]+1)=='_'])FC[0]|=2;
		else FC[0]&=~2;
}

