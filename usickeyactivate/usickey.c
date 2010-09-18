#include <crypt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void print_arr(char *buf,int n)
{
    int i=0;
    for (;i<n;i++) printf("%c",buf[i]);
}

/* This expands n symbols of inbuf to binary numerical representation in outbuf */
void expand(char *inbuf,char *outbuf,int n)
{
    int i=0;
    for (;i<n;i++)
    {
	char c=inbuf[i];
	int j=0;
	for (;j<8;j++)
	{
	    outbuf[i*8+j]=c&(1<<(8-j-1))?1:0;
	}
    }
}

/* Not really base64 but very alike */
void printable(char *inbuf,char *outbuf,int n)
{
    int i=0;
    char *ptr=inbuf;
    for (;i<n/4;i++)
    {
	char num=ptr[3]*8+ptr[2]*4+ptr[1]*2+ptr[0];
	ptr+=4;
	outbuf[i]=num+'a';
    }
}

/* Returns nearest 8-divisible number, greater than given */
int nearest8(int n)
{
    return (n/8+1)*8; /* NB: a/b is INTEGER divistion */
}

int main(int argc,char **argv)
{
    /* Setting encryption key */
    errno=0;
    char key[]="davyjones";
    char key_bin[8*8+1];
    expand(key,key_bin,8);
    key_bin[8*8]=0;
    setkey(key_bin);
    
    char key_ascii[16];
    
    char *cryptogramm=NULL;
    
    if (argc>=2)
    {
	/* Allocating the buffer with size divisible by 8 nearest and greater to given */
	int n=nearest8(strlen(argv[1]));
	char *name=calloc(n+1,sizeof(char));
	/* Copying the name */
	strcpy(name,argv[1]);
	/* Creating bit representation of message */
	char *bigbuf=calloc(n*8+1,sizeof(char));
	expand(name,bigbuf,n);
	int i=0;
	/* Loading portions of 8 bytes to encryptor */
	char *ptr=bigbuf;
	for (;i<n/8;i++)
	{
	    encrypt(ptr,0);
	    ptr+=64;    
	}
	cryptogramm=calloc(n*2+1,sizeof(char));
	printable(bigbuf,cryptogramm,n*8);
	if (argc==2) print_arr(cryptogramm,n*2);
    }
    if (argc==3) 
    {
	if (!strcmp(argv[2],cryptogramm)) return 0;
	else return 1;
    }
    return 0;
}
