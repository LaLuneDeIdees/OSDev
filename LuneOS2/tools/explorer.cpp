#include <iostream>
#include <fstream>

using namespace std;

ifstream HHD,nowFile;
uint8_t readBufferDisk[512];
uint32_t rootCellAddr=0;
uint8_t nowDisk=0;
uint32_t nowDirAdr=0;
extern uint8_t datad[];

bool equals(uint8_t* str1,uint8_t* str2, uint8_t size){
	for(int i =0;i<size;i++){
		if(str1[i]!=str2[i])return false;
	}
	return true;
}

void readSector(uint32_t addr){
	for(int i =0;i<512;i++){
		readBufferDisk[i]=datad[addr*512+i];
	}
}
void writeSector(uint32_t addr){
	for(int i =0;i<512;i++){
		datad[addr*512+i]=readBufferDisk[i];
	}
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


void open(uint8_t* name){
	cout <<"\t===OPEN===\n";
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
			cout<<readBufferDisk[i+16];
		}
		fileaddr=*(uint32_t*)&readBufferDisk[508];
		if(fileaddr==0xFFFFFFFF){
			break;
		}
	}
	cout <<'\n';
	//while(true);
	
}
void createF(uint8_t* name){
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
	char tmp[1]={0};
	while(!nowFile.eof()){
		readSector(FileAddrLab);
		*(uint32_t*)&readBufferDisk[0] = 0xFFFFFFFF;
		for(uint16_t k=0;k<480;k++){
			if(!nowFile.eof())
				nowFile.read(tmp,1);
			else tmp[0]=0;
			readBufferDisk[16+k]=tmp[0];
		}
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

void srcD(){
	readSector(nowDirAdr);
	for(uint8_t i=0;i<30;i++){
		if(readBufferDisk[16+i*16]==0)continue;
		uint8_t str[8];
		for(int k=0;k<8;k++)str[k]=readBufferDisk[16+i*16+8+k];
		cout << &str[0]<<'\t'<<readBufferDisk[16*(i+1)]<<'\n';
	}
}



int main(){
	HHD.open("osdisk.iso",ios::binary);
	uint32_t cur=0;char tmp[1]={0};
	while(!HHD.eof()){
		HHD.read(tmp,1);
		datad[cur++]=tmp[0];
	}
	
	INIT_FS();
	
	while(true){
		string cmd;
		cout<<"-->";
		cin>>cmd;
		if(cmd=="ls"){
			cout<<"\t===DIR===\n";
			srcD();
			cout<<"\t===DIR==-\n";
		}
		if(cmd=="cd"){
			cin>>cmd;
			uint8_t name[8]={0,0,0,0,0,0,0,0};
			for(int i=0;i<cmd.length();i++){
				name[i]=cmd[i];
			}
			changeD(&name[0]);
		}
		if(cmd=="crd"){
			cin>>cmd;
			uint8_t name[8]={0,0,0,0,0,0,0,0};
			for(int i=0;i<cmd.length();i++){
				name[i]=cmd[i];
			}
			createD(&name[0]);
		}
		if(cmd=="rst"){
			resetD();
		}
		if(cmd=="crf"){
			cin>>cmd;
			nowFile.open(cmd,ios::binary);
			cin>>cmd;
			uint8_t name[8]={0,0,0,0,0,0,0,0};
			for(int i=0;i<cmd.length();i++){
				name[i]=cmd[i];
			}
			createF(&name[0]);
			nowFile.close();
		}
		if(cmd=="q"){
			break;
		}
		if(cmd=="rm"){
			cin>>cmd;
			uint8_t name[8]={0,0,0,0,0,0,0,0};
			for(int i=0;i<cmd.length();i++){
				name[i]=cmd[i];
			}
			removeF(&name[0]);
		}
		if(cmd=="rmd"){
			cin>>cmd;
			uint8_t name[8]={0,0,0,0,0,0,0,0};
			for(int i=0;i<cmd.length();i++){
				name[i]=cmd[i];
			}
			removeD(&name[0]);
		}
		if(cmd=="open"){
			cin>>cmd;
			uint8_t name[8]={0,0,0,0,0,0,0,0};
			for(int i=0;i<cmd.length();i++){
				name[i]=cmd[i];
			}
			open(&name[0]);
		}
	}
	ofstream HHDnew("osdisk.iso",ios::binary|ios::trunc);
	for(uint32_t tmpc=0;tmpc<cur;tmpc++){
		HHDnew<<datad[tmpc];
	}
}


uint8_t datad[0x6fffffff];