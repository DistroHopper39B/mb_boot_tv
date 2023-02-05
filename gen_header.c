#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sha1.h"

void shax(unsigned char *result, unsigned char *data, unsigned int len)
{
	struct SHA1Context context;
	SHA1Reset(&context);
	SHA1Input(&context, (unsigned char *)&len, 4);
	SHA1Input(&context, data, len);
	SHA1Result(&context,result);	
}

int main(void) {
	FILE *kernel;
	FILE *header;
	long lKernel;
	unsigned char *szKernel;
	unsigned char sha_Message_Digest[SHA1HashSize];
	int i;

	kernel = fopen("bzImage", "r");
	if(!kernel) {
		printf("Kernel image bzImage not fount\n");
		return 1;
	}

	header = fopen("bzimage.h", "w");
	fseek(kernel, 0L, SEEK_END);
	lKernel = ftell(kernel);
	fseek(kernel, 0L, SEEK_SET);

	szKernel = (unsigned char *)malloc(lKernel);	

	fread(szKernel, lKernel, 1, kernel);

	fclose(kernel);

	fprintf(header, "#ifndef _BZIMAGE_\n");
	fprintf(header, "#define _BZIMAGE_\n\n\n");

	fprintf(header, "long lKernel = %d;\n", lKernel);
	
#if 0
	fprintf(header, "unsigned char szKernel[] = {\n");

	for(i = 0; i < lKernel; i++) {
		if((i % 20) == 0) {
			fprintf(header, "\n");
		}
		fprintf(header, "0x%02X, ", szKernel[i]);
	}

	fprintf(header, "};\n");
#else
	fprintf(header, "unsigned char szKernel[] = \n");


{
	int i=0;
	int n;

          fprintf (header,"\"");
	for(n = 0; n < lKernel; n++) {
            {
		int c=szKernel[n];

              if ((c == '"') || (c == '\\') || (c < 32) || (c > 126))
                {
                  fprintf(header, "\\%03o", c);
                }
              else
                {
                  fprintf(header, "%c", c);
                }
		i++;
              if (i == 64)
                {
                  i = 0;
                  fprintf(header,"\"\n\"");
                }
            }
        }
         fprintf (header,"\"\n;\n");
}
#endif


       	shax(&sha_Message_Digest[0], szKernel , lKernel);
	
	fprintf(header, "unsigned char szKernelSHA1[] = {\n");

	printf("Kernel SHA1 hash : ");
	for(i = 0; i < SHA1HashSize; i++) {
		printf("%02X", sha_Message_Digest[i]);
		fprintf(header, "0x%02X, ", sha_Message_Digest[i]);
	}
	printf("\n");

	fprintf(header, "};\n");

	fprintf(header, "\n\n#endif\n");

       	shax(&sha_Message_Digest[0], &szKernel[(szKernel[0x1F1] + 1) * 512] , lKernel - ((szKernel[0x1F1] + 1) * 512));

	printf("Kernel 1 SHA1 hash : ");
	for(i = 0; i < SHA1HashSize; i++) {
		printf("%02X", sha_Message_Digest[i]);
	}
	printf("\n");

	fclose(header);
	free(szKernel);

	return 0;
}
