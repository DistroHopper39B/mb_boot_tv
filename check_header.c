#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sha1.h"
#include "bzimage.h"

void shax(unsigned char *result, unsigned char *data, unsigned int len)
{
	struct SHA1Context context;
	SHA1Reset(&context);
	SHA1Input(&context, (unsigned char *)&len, 4);
	SHA1Input(&context, data, len);
	SHA1Result(&context,result);	
}

int main(void) {
	unsigned char sha_Message_Digest[SHA1HashSize];
	int i;

       	shax(&sha_Message_Digest[0], szKernel , lKernel);
	
	printf("Kernel SHA1 hash : ");
	for(i = 0; i < SHA1HashSize; i++) {
		printf("%02X", sha_Message_Digest[i]);
	}
	printf("\n");
	if(memcmp(szKernelSHA1, sha_Message_Digest, SHA1HashSize) != 0) {
		printf("Kernel SHA1 missmatch !!!!!!\n");
	}

       	shax(&sha_Message_Digest[0], &szKernel[(szKernel[0x1F1] + 1) * 512] , lKernel - ((szKernel[0x1F1] + 1) * 512));

	printf("Kernel 1 SHA1 hash : ");
	for(i = 0; i < SHA1HashSize; i++) {
		printf("%02X", sha_Message_Digest[i]);
	}
	printf("\n");

	return 0;
}
