
#include "windows.h"
#include "string.h"
#include "stdio.h"


SHORT g_pEnglishCharToScanCodeTable[] =
	{
		-1,	30,	48,	0,	32,	18,	33,	34,	14,	15,	28,	37,	38,	28,	49,	24,	
		25,	16,	19,	31,	20,	22,	47,	17,	45,	21,	44,	26,	43,	27,	7,	12,	
		57,	2,	40,	4,	5,	6,	8,	40,	10,	11,	9,	13,	51,	12,	52,	53,	
		11,	2,	3,	4,	5,	6,	7,	8,	9,	10,	39,	39,	51,	13,	52,	53,	
		3,	30,	48,	46,	32,	18,	33,	34,	35,	23,	36,	37,	38,	50,	49,	24,	
		25,	16,	19,	31,	20,	22,	47,	17,	45,	21,	44,	26,	43,	27,	7,	12,	
		41,	30,	48,	46,	32,	18,	33,	34,	35,	23,	36,	37,	38,	50,	49,	24,	
		25,	16,	19,	31,	20,	22,	47,	17,	45,	21,	44,	26,	43,	27,	41,	14,	
		3,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	
		-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	
		-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	
		-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	
		-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	
		-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	
		-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	
		-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1
	};


void ConvertKeyToCurrentKeyboard(char* sNewKey, char* sOldKey, int nNewKeyMaxSize)
{
	if (strlen(sOldKey) == 1)
	{
		HKL nDefaultKeyboard = GetKeyboardLayout(0);

		SHORT nCharCode = sOldKey[0];

		if ((nCharCode > 0) && (nCharCode <= 255))
		{
			SHORT nScanCode = g_pEnglishCharToScanCodeTable[nCharCode];

			if (nScanCode != -1)
			{
				SHORT nKeyCode2 = MapVirtualKeyEx(nScanCode & 0xff, 1, nDefaultKeyboard);

				if (nKeyCode2 != -1)
				{
					UINT nNewChar = MapVirtualKeyEx( nKeyCode2 & 0xff, 2, nDefaultKeyboard);

					if (nNewChar != 0)
					{
						sNewKey[0] = (char)nNewChar;
						sNewKey[1] = '\0';
						return;
					}
				}
			}
		}
	}

	strncpy(sNewKey, sOldKey, nNewKeyMaxSize);
	sNewKey[nNewKeyMaxSize-1] = '\0';
}



/*  This version requires that the english key table is available in windows
void ConvertKeyToCurrentKeyboard(char* sNewKey, char* sOldKey, int nNewKeyMaxSize)
{
	if (strlen(sOldKey) == 1)
	{
		HKL nDefaultKeyboard = GetKeyboardLayout(0);

		char sEnglishIdentifier[9];
		sprintf(sEnglishIdentifier,"0000%04X",MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
		HKL nEnglishKeyboard = LoadKeyboardLayout(sEnglishIdentifier,0); // "00000409",0);
		if (nEnglishKeyboard != NULL)
		{
			SHORT nKeyCode = VkKeyScanEx(sOldKey[0], nEnglishKeyboard);

			if (nKeyCode != -1)
			{
				SHORT nScanCode = MapVirtualKeyEx(nKeyCode & 0xff, 0, nEnglishKeyboard);

				if (nScanCode != -1)
				{
					SHORT nKeyCode2 = MapVirtualKeyEx(nScanCode & 0xff, 1, nDefaultKeyboard);

					if (nKeyCode2 != -1)
					{
						UINT nNewChar = MapVirtualKeyEx( nKeyCode2 & 0xff, 2, nDefaultKeyboard);

						if (nNewChar != 0)
						{
							sNewKey[0] = (char)nNewChar;
							sNewKey[1] = '\0';
							return;
						}
					}
				}
			}
		}
	}

	strncpy(sNewKey, sOldKey, nNewKeyMaxSize);
	sNewKey[nNewKeyMaxSize-1] = '\0';
}
*/