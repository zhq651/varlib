#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"


struct tl{
    char s1[256];
    char s2[256];
};


static bool hash_str_cmp(const void * s1,const void *s2)
{
    return strcmp(((struct tl*)s1)->s1,((struct tl*)s2)->s1)==0 ;
}

static size_t hash_fun(const void * s,size_t t)
{
    return hash32_str(((struct  tl*)s)->s1,HASH32_STR_INIT)%t;
}

struct tl* make_tl(char * s)
{
    struct tl* tmp=(struct tl *)malloc(sizeof( struct tl)) ;
    memset(tmp,0,sizeof(struct tl)) ;
    char *p=s ;
    int n=0;
    while(*p !=' ')
    {
        p++ ;
    }
    memcpy(tmp->s1,s,p-s);

    while(*p==' ')
    {
        p++ ;
    }
    strcpy(tmp->s2,p);
	printf("make %s %s\n",tmp->s1,tmp->s2);
    return tmp ;
}

extern void
hash_print (const Hash_table *table);
int main(int argc ,char ** argv)
{

    Hash_table * myhash=hash_initialize (100,NULL,hash_fun,hash_str_cmp,free);
    char str[256];
    struct tl * ptl=NULL ;
    FILE * fp=fopen("c:\\test.log","r");
    while(NULL != fgets(str,256,fp))
    {
        ptl=make_tl(str);
        hash_insert(myhash,ptl);
        //      printf("%s 44444 %s  ",ptl->s1,ptl->s2);
    }
    fclose(fp);
	hash_print(myhash);
    printf("-----------------------------------------------------\n");
    struct tl dd ;
    struct tl* s;
    strcpy(dd.s1,"888033");
    s=hash_lookup (myhash,&dd);
    printf("%s the value %s  ",dd.s1,s->s2);
    hash_print_statistics(myhash,stdout);
    return 0;
}

