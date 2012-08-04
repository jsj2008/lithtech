
// This module implements the PCX loading routines.

#ifndef __LOAD_PCX_H__
#define __LOAD_PCX_H__

    
#ifndef __ILTSTREAM_H__
#include "iltstream.h"
#endif

#ifndef __PIXELFORMAT_H__
#include "pixelformat.h"
#endif


// PCX header structures.
struct REZ_PCXRGB
{
    uint8 r, g, b;
};

struct REZ_PCXHDR
{
    int8            manufacturer;
    int8            version;
    int8            encoding;
    uint8           bitsPerPixel;
    int16           x0, y0, x1, y1;
    int16           xDPI, yDPI;
    REZ_PCXRGB      pal16[16];
    int8            reserved;
    int8            planes;
    int16           bytesPerLine;
    int16           paletteInfo;
    int16           xScreenSize, yScreenSize;
    int8            filler[54];
};



// TGA header.
#pragma pack(1)
    class TGAHeader
    {
    public:
        uint8   m_IDLength;
        uint8   m_ColorMapType;
        uint8   m_ImageType;
        uint16  m_CMapStart;
        uint16  m_CMapLength;
        uint8   m_CMapDepth;
        uint16  m_XOffset;
        uint16  m_YOffset;
        uint16  m_Width;
        uint16  m_Height;
        uint8   m_PixelDepth;
        uint8   m_ImageDescriptor;
    };
#pragma pack()


// This is a PCX file loaded into memory.. 
// LoadedBitmaps are just malloc()'d into one big block of memory.
class LoadedBitmap
{
public:

                    LoadedBitmap();
    void            Term();

    uint8&          Pixel(uint32 x, uint32 y) {return m_Data[y*m_Pitch+x];}


    // Tells what format the loaded PCX was.
    PFormat         m_Format;
    RPaletteColor   m_Palette[256];

    unsigned long   m_Width, m_Height, m_Pitch;
    CMoArray<uint8> m_Data;
};


LoadedBitmap* pcx_Create(ILTStream *pStream);
LTBOOL pcx_Create2(ILTStream *pStream, LoadedBitmap *pBitmap);
void pcx_Destroy(LoadedBitmap *pBitmap);    


LTBOOL tga_Create2(ILTStream *pStream, LoadedBitmap *pBitmap);


#endif  // __LOAD_PCX_H__
    
