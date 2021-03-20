#include <fstream>
#include <iostream>

using namespace std;

int main(){
	ifstream bin("out.bin",ios::binary);
	ofstream out("mem.cpp",ios::trunc);
	out<<"uint8_t mem[0x1000000]={\n\t";
	for(int i =0;i<0x800;i++)out<<"0,";
	char tmp[]={0};
	int c=0;
	out<<"\n";
	while(!bin.eof()){
		bin.read(tmp,1);
		int i = int(tmp[0]);
		if(i<0)i=256+i;
		out<<i<<',';
		c++;
		if(c==10){
			out<<"\n\t";c=0;
		}
	}
	out<<"\n}\n\n";
	return 0;
}