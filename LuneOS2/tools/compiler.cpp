#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

vector<string> lablename;
vector<uint32_t> lableaddr;
vector<vector<string>> macros;

ofstream bin;
ifstream code;

uint32_t PC=0x10000;

#include "../lib/cmdlist.cpp"
uint8_t findcmd(string cmd){
	for(int i =0;i<cmd.length();i++)cmd[i]=toupper(cmd[i]);
	for(int i =0;i<32;i++){
		if(cmds[i]==cmd){
			return i;
		}
	}
	return 0;
}
void parse(string tmp){
	if(tmp[0]=='='){
		tmp = tmp.substr(1,tmp.length());
		int loop= stoi(tmp);
		for(int i=0;i<loop;i++){
			bin<<(uint8_t)0;
			PC++;
		}
	}else if(tmp[0]=='/'){
		tmp = tmp.substr(1,tmp.length());
		lablename.push_back(tmp);
		lableaddr.push_back(PC);
	}else if(tmp[0]=='.'){
		string name = tmp.substr(1,tmp.find('<')-1);
		string data = tmp.substr(tmp.find('<')+1,tmp.length());
		while(tmp[tmp.length()-1]!='>'){
			code>>tmp;
			data+=" "+tmp;
		}
		data = data.substr(0,data.length()-1);
		vector<string> tmpv;
		tmpv.push_back(name);
		tmpv.push_back(data);
		macros.push_back(tmpv);
	}else if(tmp[0]==','){
		tmp = tmp.substr(1,tmp.length());
		for(int i =0;i<macros.size();i++){
			if(macros[i][0] == tmp){
				tmp = macros[i][1];
				break;
			}
			if(i==macros.size()-1)return;
		}
		stringstream ss;
		ss<<tmp;
		while(!ss.eof()){
			string tmpp;
			ss >> tmpp;
			if(tmpp=="")continue;
			if(tmpp[0]=='['){
				while(tmpp[tmpp.length()-1]!=']' && !ss.eof()){
					ss>>tmpp;
				}
				ss>>tmpp;
			}
			parse(tmpp);
		}
	}else if(tmp[0]=='-'){
		tmp = tmp.substr(1,tmp.length());
		ifstream includef(tmp);
		while(!includef.eof()){
			string tmpp;
			includef >> tmpp;
			if(tmpp=="")continue;
			if(tmpp[0]=='['){
				while(tmpp[tmpp.length()-1]!=']' && !includef.eof()){
					includef>>tmpp;
				}
				includef>>tmpp;
			}
			parse(tmpp);
		}
	}else{
		string cmd,type,mode,oper;
		int i=0;
		while(tmp[i]!='-')cmd+=tmp[i++];i++;
		while(tmp[i]!='-')type+=tmp[i++];i++;
		while(tmp[i]!='-')mode+=tmp[i++];i++;
		oper = tmp.substr(i,tmp.length());
		if(type == "")type="0";
		if(mode=="")mode="0";
		if(oper=="")oper="0";
		
		bin<< uint8_t(findcmd(cmd) | (stoi(type)<<6) | (stoi(mode) << 5));
		PC++;
		
		if(oper!=""){
			uint32_t operi=0;
			try{
				operi = stoi(oper);
			}catch(exception e){
				for(int i =0;i<lablename.size();i++){
					if(lablename[i]==oper){
						operi = lableaddr[i];
						break;
					}
					if(i==lablename.size()-1){
						for(int k=0;k<macros.size();k++){
							if(macros[k][0]==oper){
								try{
									operi = stoi(macros[k][1]);
								}catch(exception e){
									for(int q =0;q<lablename.size();q++){
										if(lablename[q]==macros[k][1]){
											operi = lableaddr[q];
											break;
										}
									}
								}
								break;
							}
						}
					}
				}
			}
			for(int i =0;i<4;i++){
				bin << uint8_t((operi>>(i*8))&0xFF);
				PC++;
			}
		}
	}
}

int main(int arg, char** argc){
	if(arg<3){
		cout<<"not arg\n";
		return 0;
	}
	code.open(argc[1]);
	bin.open(argc[2],ios::trunc|ios::binary);
	for(int i =0;i<2;i++){
		cout << "\t"<<i<<'\n';
		while(!code.eof()){
			string tmp;
			code >> tmp;
			if(tmp=="")continue;
			if(tmp[0]=='['){
				while(tmp[tmp.length()-1]!=']' && !code.eof()){
					code>>tmp;
				}
				code>>tmp;
			}
			cout << tmp <<'\n';
			parse(tmp);
		}
		code.clear();
		code.seekg(0, ios::beg);
		bin.clear();
		bin.seekp(0, ios::beg);
	}
	return 0;
}