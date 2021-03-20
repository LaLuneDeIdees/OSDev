bool equals(uint8_t* str1,uint8_t* str2, uint8_t size){
	for(int i =0;i<size;i++){
		if(str1[i]!=str2[i])return false;
	}
	return true;
}