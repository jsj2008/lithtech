// pma.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <conio.h>
#include "tdguard.h"

// TGA header.
#pragma pack(1)
	class TGAHeader
	{
	public:
		BYTE	m_IDLength;
		BYTE	m_ColorMapType;
		BYTE	m_ImageType;
		WORD	m_CMapStart;
		WORD	m_CMapLength;
		BYTE	m_CMapDepth;
		WORD	m_XOffset;
		WORD	m_YOffset;
		WORD	m_Width;
		WORD	m_Height;
		BYTE	m_PixelDepth;
		BYTE	m_ImageDescriptor;
	};
#pragma pack()


int PrintUsage(char *pError, ...)
{
	va_list marker;
	char str[512];

	if(pError)
	{
		va_start(marker, pError);
		vsprintf(str, pError, marker);
		va_end(marker);

		printf("Error: %s\n", str);
	}
	
	printf("PreMultiplied Alpha tool - unpremultiplies the specified TGA file.\n");
	printf("Usage: PMA <filename.tga>\n");
	return 0;
}


int main(int argc, char* argv[])
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		return 1;
	}

	char idField[256], *pFilename;
	DWORD x, y, lineSize, dataSize;
	BYTE *pData, *pLineData;
	TGAHeader hdr;
	FILE *fp;
	DWORD r, g, b;
	float div;


	if(argc < 2)
	{
		return PrintUsage(NULL);
	}

	pFilename = argv[1];
	fp = fopen(pFilename, "rb");
	if(fp)
	{	
		fread(&hdr, sizeof(hdr), 1, fp);

		// We only support:
		// 32-bit data
		// Uncompressed image data
		// No palette
		if(hdr.m_PixelDepth != 32 || 
			hdr.m_ImageType != 2 || 
			hdr.m_ColorMapType != 0)
		{
			fclose(fp);
			return PrintUsage("%s - not a valid TGA file (not 32-bit, wrong image type..)", pFilename);
		}

		// Read whatever ID stuff there is.
		fread(idField, hdr.m_IDLength, 1, fp);

		lineSize = (DWORD)hdr.m_Width * sizeof(DWORD);
		dataSize = lineSize * hdr.m_Height;
		
		pData = new BYTE[dataSize];
		if(!pData)
		{
			fclose(fp);
			return PrintUsage("%s - can't allocate %d bytes", pFilename, dataSize);
		}

		for(y=0; y < hdr.m_Height; y++)
		{
			pLineData = &pData[(hdr.m_Height-y-1) * lineSize];
			fread(pLineData, lineSize, 1, fp);
		}
		fclose(fp);


		// Save the file back out..
		// 0 = blue
		// 1 = green
		// 2 = red
		// 3 = alpha
		fp = fopen(pFilename, "wb");
		if(fp)
		{
			fwrite(&hdr, sizeof(hdr), 1, fp);
			fwrite(idField, hdr.m_IDLength, 1, fp);

			for(y=0; y < hdr.m_Height; y++)
			{
				pLineData = &pData[(hdr.m_Height-y-1) * lineSize];
				for(x=0; x < hdr.m_Width; x++)
				{
					if(pLineData[3] != 0)
					{
						div = (float)pLineData[3] / 255.0f;
						
						r = (DWORD)((float)pLineData[0] / div);
						g = (DWORD)((float)pLineData[1] / div);
						b = (DWORD)((float)pLineData[2] / div);

						if(r > 255) r = 255;
						if(g > 255) g = 255;
						if(b > 255) b = 255;

						pLineData[0] = (BYTE)r;
						pLineData[1] = (BYTE)g;
						pLineData[2] = (BYTE)b;
					}

					pLineData += sizeof(DWORD);
				}

				pLineData = &pData[(hdr.m_Height-y-1) * lineSize];
				fwrite(pLineData, lineSize, 1, fp);
			}
			
			fclose(fp);
		}
		else
		{
			delete pData;
			return PrintUsage("%s - can't open for writing", pFilename);
		}

		
		delete pData;
		return 1;
	}
	else
	{
		return PrintUsage("%s - can't open file for reading", pFilename);
	}
}

