#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void main()
{
	FILE *in = fopen("WONStatus.h","r");
	FILE *out = fopen("WONStatus.cpp","w");

	fprintf(out,"#define __WON_MASTER_CPP__\n\n");
	fprintf(out,"#include \"WONStatus.h\"\n");
	fprintf(out,"#include <stdio.h>\n");
	fprintf(out,"\n");
	fprintf(out,"///////////////////////////////////////////////////////////////////////////////\n");
	fprintf(out,"///////////////////////////////////////////////////////////////////////////////\n");
	fprintf(out,"\n");
	fprintf(out,"const char* WONAPI::WONStatusToString(WONStatus theError)\n");
	fprintf(out,"{\n");
	fprintf(out,"\tswitch(theError)\n");
	fprintf(out,"\t{\n");

	bool inEnum = false;

	while(!feof(in))
	{
		char aBuf[1024], anEnum[1024];
		aBuf[0] = '\0';
		fgets(aBuf,1000,in);
		if(!inEnum && strstr(aBuf,"enum WONStatus")!=NULL)
			inEnum = true;
		else if(inEnum && strchr(aBuf,'}')!=NULL)
			inEnum = false;
		else if(inEnum && sscanf(aBuf,"#%s",anEnum)==1)
		{
			fprintf(out,"%s",aBuf);
		}
		else if(sscanf(aBuf,"%s",anEnum)==1 /*&& strchr(aBuf,'=')!=NULL*/ && strlen(aBuf)>3 && !strncmp(anEnum,"WS_",3))
		{
			char *aPtr = strchr(anEnum,',');
			if(aPtr!=NULL)
				*aPtr = '\0';

			fprintf(out,"\t\tcase %s: return \"%s\";\n",anEnum,anEnum);
		}
	}

	fprintf(out,"\t\tdefault: { static char aBuf[1024]; if(theError<=-2900 && theError>=-2999) sprintf(aBuf,\"DBProxyServ_GameSpecificError%%d\",theError); else sprintf(aBuf,\"%%d\",theError); return aBuf; }\n");
	fprintf(out,"\t}\n");
	fprintf(out,"}\n");

	fclose(in);
	fclose(out);
}
