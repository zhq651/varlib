#include <string.h>
#include <stdio.h>
#include "md5.h"
int Hex2Str(char *str,char *hex,int len)
{
	int i;

	for(i = 0;i < len;i++)
		sprintf(str + i*2, "%02X", (unsigned char)hex[i]);

	return 0;
}
int main(int argc, const char *argv[])
{
    MD5_CTX md5;
	int i;
    char hello[] = "hello world";
	char final[17]={0};
	char str[100];
    MD5Init(&md5);
	MD5Update(&md5,(const unsigned char*)hello,strlen(hello));
	MD5Final(&md5,(unsigned char*)final);
	Hex2Str(str,final,16);
	printf("%s\n",str);
    return 0;
}

