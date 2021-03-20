#include <lib/io.cpp>
#include <lib/disk_ide.cpp>
#include <lib/std.cpp>
#include <lib/fs.cpp>

uint32_t* A =         (uint32_t*)&mem[0x000];
uint8_t*  SR =        (uint8_t* )&mem[0x004];
uint32_t* PC =        (uint32_t*)&mem[0x005];
uint8_t*  DSC =       (uint8_t* )&mem[0x009];
uint8_t*  FSC =       (uint8_t* )&mem[0x00A];
uint8_t*  datastack = (uint8_t* )&mem[0x100];
uint8_t*  funcstack = (uint8_t* )&mem[0x200];
uint8_t*  videomem =  (uint8_t* )&mem[0x300];

bool running;

void startVM();

void VM_RUN(){
	//disable nmi
	outb ( 0x70 , inb ( 0x70 )  |  0x80 ) ; 
	//		INIT start state
	*PC = 0x00010000;
	*A =  0;
	*SR = 0;
	*DSC =0;
	*FSC =0;
	running=true;
	for(uint16_t i=0;i<256;i++){
		datastack[i]=0;
		funcstack[i]=0;
	}	
	
	//init video start state
	outb(0x3c8,0); //set start address for set color pallete
	for(int i =0;i<256;i++){  //set all 256 colors
		int r,g,b;
		r = (i&0b11100000)>>2; //calculate r,g and b
		g = (i&0b11100)<<1;
		b = (i&0b11)<<4;
		outb(0x3c9,r); //set
		outb(0x3c9,g);
		outb(0x3c9,b);
	}
	for(uint16_t i=0;i<64000;i++){ //clear screen
		videomem[i]=0;
	}
	
	//init filesystem
	INIT_FS();
	//read kernel file to memory
	uint8_t kernelname[8]={'k','e','r','n','e','l',0,0};
	open(kernelname,*PC);


	//test for debuging
	uint8_t* screen = (uint8_t*)0xA0000;
	for(int i=0;i<64000;i++){
		screen[i]=(mem[0x10000+i%320]);
	}
	
	startVM();
}
//struct for cmd data
struct Args{
	uint8_t cmd=0;
	uint8_t type=0;
	uint8_t mode=0;
	uint32_t oper=0;
};

//function read next 5 bytes and calculate cmd,type,mode and oper
Args getNextContext(){
	Args ret; //create struct
	ret.cmd = mem[*PC]; //load cmd description
	ret.type = (ret.cmd&0b11000000)>>6; //get type
	ret.mode = (ret.cmd&0b00100000)>>5; //get mode
	ret.cmd = ret.cmd & 0b00011111; //get cmd
	ret.oper = *(uint32_t*)&mem[*PC+1]; // first level operand
	for(uint8_t i = 0;i < ret.type; i++){ //if levels>0 go to claster
		ret.oper = *(uint32_t*)&mem[ret.oper]; //read next claster
	}
	*PC=*PC+5; //incriment program counter
	return ret;
}
void LDA(Args arg){
	*A = arg.oper;
	if(arg.mode==0)*A=*A&0xFF;
}
void STA(Args arg){
	if(arg.mode==0){
		mem[arg.oper] = *A&0xFF;
	}else{
		*(uint32_t*)&mem[arg.oper] = *A;
	}
}
void INB(Args arg){
	*A=inb(arg.oper);
}
void OUTB(Args arg){
	outb(arg.oper,*A&0xFF);
}

void INW(Args arg){
	*A=inw(arg.oper);
}
void OUTW(Args arg){
	outw(arg.oper,*A&0xFFFF);
}
void JMP(Args arg){
	*PC = arg.oper;
}
void JIT(Args arg){
	if(*SR&1)
	*PC = arg.oper;
}
void JSR(Args arg){
	*(uint32_t*)&funcstack[*FSC] = *PC;
	*PC=arg.oper;
	*FSC = *FSC+4;
}
void RST(Args arg){
	*FSC = *FSC-4;
	*PC = *(uint32_t*)&funcstack[*FSC];
}
void ADD(Args arg){
	if(arg.mode==0)*A=*A&0xFF;
	uint32_t lasta = *A;
	*A=*A+arg.oper;
	bool setp=false;
	if(arg.mode==0){
		setp=*A>0xFF;
	}else{
		setp=(lasta>=*A)&&(arg.oper!=0);
	}
	if(setp){
		if(arg.mode==0)*A&=0xFF;
		*SR|=3;
	}else{
		*SR&=~3;
	}
}
void ADC(Args arg){
	if(arg.mode==0)*A=*A&0xFF;
	uint32_t lasta = *A;
	*A=*A+arg.oper+((*SR&2)>>1);
	bool setp=false;
	if(arg.mode==0){
		setp=*A>0xFF;
	}else{
		setp=(lasta>=*A)&&(arg.oper+((*SR&2)>>1)!=0);
	}
	if(setp){
		if(arg.mode==0)*A&=0xFF;
		*SR|=3;
	}else{
		*SR&=~3;
	}
}
void SUB(Args arg){
	if(arg.mode==0)*A=*A&0xFF;
	uint32_t lasta = *A;
	*A=*A-arg.oper;
	if((lasta<=*A)&&(arg.oper!=0)){
		if(arg.mode==0)*A&=0xFF;
		*SR|=3;
	}else{
		*SR&=~3;
	}
}

void SBC(Args arg){
	if(arg.mode==0)*A=*A&0xFF;
	uint32_t lasta = *A;
	*A=*A-arg.oper-((*SR&2)>>1);
	if((lasta<=*A)&&(arg.oper+((*SR&2)>>1)!=0)){
		if(arg.mode==0)*A&=0xFF;
		*SR|=3;
	}else{
		*SR&=~3;
	}
}
void OR(Args arg){
	*A|=arg.oper;
	if(arg.mode==0)*A&=0xFF;
	if(*A){
		*SR|=1;
	}else{
		*SR&=~1;
	}
}
void AND(Args arg){
	*A&=arg.oper;
	if(arg.mode==0)*A&=0xFF;
	if(*A){
		*SR|=1;
	}else{
		*SR&=~1;
	}
}
void XOR(Args arg){
	*A^=arg.oper;
	if(arg.mode==0)*A&=0xFF;
	if(*A){
		*SR|=1;
	}else{
		*SR&=~1;
	}
}
void NOT(Args arg){
	*A=~*A;
	if(arg.mode==0)*A&=0xFF;
	if(*A){
		*SR|=1;
	}else{
		*SR&=~1;
	}
}
void PUSH(Args arg){
	if(arg.mode==0){
		datastack[*DSC]=*A&0xFF;
		*DSC+=1;
	}else{
		*(uint32_t*)&datastack[*DSC]=*A;
		*DSC+=4;
	}
}
void POP(Args arg){
	if(arg.mode==0){
		*DSC-=1;
		*A=datastack[*DSC];
	}else{
		*DSC-=4;
		*A=*(uint32_t*)&datastack[*DSC];
	}
}
void startVM(){
	uint8_t* screen = (uint8_t*)0xA0000; //create real video memory
	while(running){//if we run now
		Args arg = getNextContext(); //read next cmd
		
		switch(arg.cmd){ //call cmd
			case 0: //BRK stop program
				running=false;
				break;
			case 1:
				LDA(arg);
				break;
			case 2:
				STA(arg);
				break;
			case 3:
				INB(arg);
				break;
			case 4:
				OUTB(arg);
				break;
			case 5:
				INW(arg);
				break;
			case 6:
				OUTW(arg);
				break;
			case 7:
				JMP(arg);
				break;
			case 8:
				JSR(arg);
				break;
			case 9:
				RST(arg);
				break;
			case 10:
				ADD(arg);
				break;
			case 11:
				ADC(arg);
				break;
			case 12:
				SUB(arg);
				break;
			case 13:
				SBC(arg);
				break;
			case 14:
				NOT(arg);
				break;
			case 15:
				OR(arg);
				break;
			case 16:
				AND(arg);
				break;
			case 17:
				XOR(arg);
				break;
			case 18:
				JIT(arg);
				break;
			case 19:
				POP(arg);
				break;
			case 20:
				PUSH(arg);
				break;
			default:
				break;
		}
		if(*SR&4){
			for(int i =0;i<64000;i++){//update screen
				screen[i]=videomem[i];
			}
			*SR&=~4;
		}
	}
	while(true);
}