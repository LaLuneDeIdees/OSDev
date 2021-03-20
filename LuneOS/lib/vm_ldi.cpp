/////////////////////////////////////
uint32_t PC = 0x1000;
uint8_t* START = &mem[0x1000];
uint8_t* A = &mem[0];
uint8_t* P = &mem[1];
uint32_t* PCM = (uint32_t*)&mem[2];
//video
uint8_t* VC = &mem[0x16];
uint16_t* VM = (uint16_t*)&mem[0x17];
//input
uint8_t* IC = &mem[6];
//fileio
uint8_t* FC = &mem[8];
uint8_t* FNAME=&mem[9];
uint32_t* FADDR = (uint32_t*)&mem[17];
uint8_t* FNUM = &mem[21];

bool run = true;
/////////////////////////////////////


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
void S(){if(P[0]&4)return;*PCM=PC;}
uint32_t arg(uint8_t cmd){
	uint32_t otv=0;
	cmd=cmd>>5;
	if(cmd){
		for(int i =0;i<4;i++)otv+=mem[PC++]<<(i*8);S();cmd--;
		uint32_t tmp=0;
		for(int i =0;i<cmd;i++){
			tmp=otv;otv=0;
			for(int i =0;i<4;i++)otv+=mem[tmp++]<<(i*8);
		}
	}
	return otv;
}
void BRK(uint32_t oper){run=false;}
void NOP(uint32_t oper){for(int i =0;i<oper;i++)asm("nop");}
void LDA(uint32_t oper){A[0]=uint8_t(oper%256);}
void STA(uint32_t oper){mem[oper]=A[0];}
void ADD(uint32_t oper){
	uint8_t last = A[0];A[0]+=oper;
	if(last>=A[0] && (oper%256)!=0)P[0]|=3;else P[0]&=~3;
}
void ADC(uint32_t oper){
	uint8_t last = A[0];A[0]+=oper%256+((P[0]&2)>>1);
	if(last>=A[0] && ((oper%256)!=0 | ((P[0]&2)>>1)))P[0]|=3;else P[0]&=~3;
}
void SUB(uint32_t oper){
	uint8_t last = A[0];A[0]-=oper%256;
	if(last<=A[0] && (oper%256)!=0)P[0]|=3;else P[0]&=~3;
}
void SBC(uint32_t oper){
	uint8_t last = A[0];A[0]-=oper%256+((P[0]&2)>>1);
	if(last<=A[0] && ((oper%256)!=0 | ((P[0]&2)>>1)))P[0]|=3;else P[0]&=~3;
}
void NOT(uint32_t oper){A[0]=~A[0];if(A[0])P[0]|=1;else P[0]&=~1;}
void OR(uint32_t oper){A[0]=A[0]|(oper%256);if(A[0])P[0]|=1;else P[0]&=~1;}
void AND(uint32_t oper){A[0]=A[0]&(oper%256);if(A[0])P[0]|=1;else P[0]&=~1;}
void XOR(uint32_t oper){A[0]=A[0]^(oper%256);if(A[0])P[0]|=1;else P[0]&=~1;}
void JMP(uint32_t oper){PC=oper;S();}
void JIT(uint32_t oper){if(P[0]&1)JMP(oper);}
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void VM_INIT(){
	for(int i =0;i<2000;i++)
		VM[i]=' '|0x0F<<8;
	S();
	
	if(ide_devices[0].Reserved){
	VM[1998]='1'|0x1F<<8;
	}if(ide_devices[1].Reserved){
	VM[1997]='2'|0x1F<<8;
	}if(ide_devices[2].Reserved){
	VM[1996]='3'|0x1F<<8;
	}if(ide_devices[3].Reserved){
	VM[1995]='4'|0x1F<<8;
	}
	
	
	uint8_t newname[8] = {'k','e','r','n','e','l',0,0};
	open(&newname[0],0x1000);
}
void VM_START(){
	while(run){
		uint8_t cmd=mem[PC++];S();
		uint32_t oper = arg(cmd);
		cmd=cmd & ~(0b111<<5);
		if(cmd==0)BRK(oper);
		else if(cmd==1)NOP(oper);
		else if(cmd==2)LDA(oper);
		else if(cmd==3)STA(oper);
		else if(cmd==4)ADD(oper);
		else if(cmd==5)ADC(oper);
		else if(cmd==6)SUB(oper);
		else if(cmd==7)SBC(oper);
		else if(cmd==8)NOT(oper);
		else if(cmd==9)OR(oper);
		else if(cmd==10)AND(oper);
		else if(cmd==11)XOR(oper);
		else if(cmd==12)JMP(oper);
		else if(cmd==13)JIT(oper);
		
		//video
		if(VC[0] & 1){
			for(int i =0;i<2000;i++)terminal_buffer[i]=VM[i];
			VC[0]&=~1;
		}
		//input
		if(IC[0] & 1){
			uint8_t nowkey=0xff;
			uint8_t state = inb(0x64);
			if(state & 1)nowkey=inb(0x60);
			IC[1]=nowkey;IC[0]&=~1;
		}else if(IC[0] & 2){
			uint8_t nowkey=0xff;
			uint8_t state = inb(0x64);
			while(state & 1){nowkey=inb(0x60);state = inb(0x64);}
			IC[1]=nowkey;IC[0]&=~2;
		}
		
		//fileio
		if(FC[0]&1){
			uint8_t mode = FC[0]>>4;
			if(mode==1)open(FNAME,*FADDR);
			else if(mode==2)createF(FNAME,*FADDR,*FNUM);
			else if(mode==3)removeF(FNAME);
			else if(mode==4)createD(FNAME);
			else if(mode==5)removeD(FNAME);
			else if(mode==6)changeD(FNAME);
			else if(mode==7)resetD();
			else if(mode==8)srcD(FC);
			FC[0]&=~1;
		}
	}
}


