#include "strbuf.h"
#include <string.h>
int main(int argc, const char *argv[])
{
    
    strbuf str = STRBUF_INIT;
    strbuf_init(&str,100);
    strcpy(str.buf,"   hello world   ");
    printf("len:%d , field %d \n",strlen(str.buf),str.len);
    printf("init:%s\n",str.buf);
    strbuf_ltrim(&str);
    printf("ltrim:%s\n",str.buf);
    strbuf_rtrim(&str);
    printf("%s\n",str.buf);
    return 0;
}
