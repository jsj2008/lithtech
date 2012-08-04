// Utilities.cpp

#include "stdafx.h"
#include "Utilities.h"

void OutputMsg(LPSTR fmt, ...)
{
    char buff[256]; va_list va;

    va_start( va, fmt ); wvsprintf( buff, fmt, va );
    va_end( va ); lstrcat(buff, "\r\n");
    MessageBox(NULL,buff,"Message Box",MB_OK);
}

void dprintf(LPSTR fmt, ...)
{
    char buff[256]; va_list va;

    va_start( va, fmt ); wvsprintf( buff, fmt, va );
    va_end( va ); lstrcat(buff, "\r\n");
    OutputDebugString(buff);
}

void DisplayLastWndError()
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);
	MessageBox(NULL,(LPCTSTR)lpMsgBuf,"Error",MB_OK|MB_ICONINFORMATION);
	LocalFree(lpMsgBuf);
}

void wwrite(unsigned short data, FILE* f)
{
    unsigned char l = data & 0xFF;
    unsigned char h = data >> 8;
    fputc(l,f);
    fputc(h,f);
}

#define UL_TGA_LR 0x10									// TGA Defines...
#define UL_TGA_BT 0x20
bool SaveImageFile_TGA(const char* szFilename, unsigned char* pData, uint32 Width, uint32 Height)
{
	FILE*	f				= NULL;
	int		format			= 0;
	int		src_depth		= 32;
	int		dst_depth		= 32;
	int		bytes_per_pixel	= src_depth/8;
	uint32	desc			= UL_TGA_BT;

	if ((f = fopen(szFilename, "wb")) == NULL)			{ return false; }

	fputc(0,f);				// id_length
	fputc(0,f);				// color_map_type
	fputc(2,f);				// type;
	wwrite(0,f);			// cm_index;
	wwrite(0,f);			// cm_length;
	fputc(0,f);				// cm_entry_size;

	wwrite(0,f);			// x_org;
	wwrite(0,f);			// y_org;
	wwrite((unsigned short)Width,f);
	wwrite((unsigned short)Height,f);

	fputc(src_depth,f);
	fputc(desc,f);			// desc

	int hxw					= Width * Height;
	int right				= desc & UL_TGA_LR;
	int top					= desc & UL_TGA_BT;
	unsigned char* temp_dp	= pData;
	unsigned long* swap		= 0;

	if (!top) {			// swap back around
		swap = (unsigned long*) new unsigned long[hxw];
		memcpy(swap, temp_dp, hxw * sizeof(unsigned long));
		unsigned long * src, * dest;   
		unsigned long * src_data = (unsigned long *)temp_dp;
		for (uint32 i = 0; i < Height; i++) {
			src  = &swap[(Height - i - 1) * Width];
			dest = &src_data[i * Width];
			memcpy(dest, src, Width * sizeof(unsigned long)); }
        
		pData = (unsigned char*)swap; }					// Use the swapped are to write out the pData
    
	if (src_depth == dst_depth && format == 0) {
		fwrite(pData, hxw, bytes_per_pixel, f); }
	else { assert(0); fclose(f); if (swap) delete[] swap; return false; }

	fclose(f);
    if (swap) delete[] swap;
    
	return true;
}