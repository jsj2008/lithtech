#pragma warning(disable:4786)
#include <stdio.h>

void main()
{
	FILE *aFile = fopen("d:\\titan\\kver.pub","rb");
	FILE *out = fopen("kver.txt","w");
	int aCount = 0;

	fprintf(out,"static char VerifierKeyBuf[] = \n");
	fprintf(out,"\t\"");
	while(!feof(aFile))
	{
		int aChar = fgetc(aFile);
		if(aChar>=0)
		{
			fprintf(out,"\\x%02x",aChar);
			aCount++;
			if(aCount>20)
			{
				fprintf(out,"\"\\\n\t\"");
				aCount=0;
			}
		}
	}

	fprintf(out,"\";");

	fclose(aFile);
	fclose(out);

}
