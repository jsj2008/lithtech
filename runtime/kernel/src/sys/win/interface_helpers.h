
// This module implements lots of helpful rectangle and 2d clipping
// routines used for interface stuff.

#ifndef __INTERFACE_HELPERS_H__
#define __INTERFACE_HELPERS_H__



#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __RENDER_H__
#include "render.h"
#endif


// Surface flags.
#define SURFFLAG_SCREEN         (1<<0)  // This is the screen surface..
#define SURFFLAG_OPTIMIZED      (1<<1)  // This surface is 'optimized' by the renderer.
#define SURFFLAG_OPTIMIZEDIRTY  (1<<2)  // This surface has been changed and needs to be
                                        // reoptimized.


// These are used to simplify routines.
class Pixel16
{
public:

    typedef uint16  type;

    void    operator=(uint8 *ptr)       {m_pPixel = (uint16*)ptr;}
    void    operator=(uint16 *ptr)      {m_pPixel = (uint16*)ptr;}
    void    operator=(Pixel16 &other)   {*m_pPixel = *other.m_pPixel;}
    void    operator=(uint16 val)       {*m_pPixel = val;}
    void    operator=(GenericColor val) {*m_pPixel = val.wVal;}
    void    operator++() {m_pPixel++;}
    LTBOOL  operator==(GenericColor &theColor) {return *m_pPixel == theColor.wVal;}
    LTBOOL  operator!=(GenericColor &theColor) {return *m_pPixel != theColor.wVal;}
    uint16  operator[](uint32 index)        {return m_pPixel[index];}
    static uint16 GetGenericColor(GenericColor &color) {return color.wVal;}

    uint16  *m_pPixel;
};

class Pixel32
{
public:

    typedef uint32  type;

    void    operator=(uint8 *ptr)       {m_pPixel = (uint32*)ptr;}
    void    operator=(uint32 *ptr)      {m_pPixel = (uint32*)ptr;}
    void    operator=(Pixel32 &other)   {*m_pPixel = *other.m_pPixel;}
    void    operator=(uint32 val)       {*m_pPixel = val;}
    void    operator=(GenericColor val) {*m_pPixel = val.dwVal;}
    void    operator++() {m_pPixel++;}
    LTBOOL  operator==(GenericColor &theColor) {return *m_pPixel == theColor.dwVal;}
    LTBOOL  operator!=(GenericColor &theColor) {return *m_pPixel != theColor.dwVal;}
    uint32  operator[](uint32 index)        {return m_pPixel[index];}
    static uint32 GetGenericColor(GenericColor &color) {return color.dwVal;}

    uint32  *m_pPixel;
};



// The font structure.
class LTFont : public CGLLNode
{
    public:

        HFONT   m_hFont;

};


// The surface structure.. 
class CisSurface : public CGLLNode
{
    public:

        // Used when restarting the renderer to backup the surface.
        uint8       *m_pBackupBuffer;
        
        // All the surface info you always wanted but were afraid to ask for.
        HLTBUFFER   m_hBuffer;
        uint32      m_Width, m_Height;
        long        m_Pitch;
        
        // Is it the screen or a normal surface, etc...
        uint32      m_Flags;

        // If this is NO_OPTIMIZED_TRANSPARENCY
        PValue      m_OptimizedTransparentColor;

        // User data..
        void        *m_pUserData;

        // Surface alpha value (only used if optimized).
        float       m_Alpha;
};

#define MAX_WARP_POINTS 10

#define WARP_FIXED              int32
#define WARP_FIXED_SHIFT        16
#define FLOAT_TO_WARP_FIXED(x)  (WARP_FIXED)(x * (float)(1<<WARP_FIXED_SHIFT))



typedef struct
{
    short       m_DestX;
    WARP_FIXED  m_SourceX, m_SourceY;   // Fixed point source X and Y.
} WarpCoords;


extern CisSurface g_ScreenSurface;
extern RenderStruct *g_pCisRenderStruct;
extern GenericColor g_TransparentColor, g_SolidColor;

extern PFormat g_ScreenFormat;
extern uint32 g_nScreenPixelBytes;


// These take the base number and shift them left by 3 WITH sign extending.
typedef LTRESULT (DrawWarpFn)(CisSurface *pDest, CisSurface *pSrc, 
    WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY);


// ----------------------------------------------------------------- //
// Functions implemented in clientde_impl_sys.cpp.
// ----------------------------------------------------------------- //

CisSurface* cis_InternalCreateSurface(uint32 width, uint32 height);
LTRESULT cis_DeleteSurface(HSURFACE hSurface);

//gone now, use IClientFormatMgr interface.
//FormatMgr* cis_GetFormatMgr();

LTRESULT cis_DrawSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc, 
    LTRect *pSrcRect, int destX, int destY);


// ----------------------------------------------------------------- //
// Helper functions.
// ----------------------------------------------------------------- //

LTBOOL cis_RectIntersection(LTRect *pDest, LTRect *pRect1, LTRect *pRect2);

// Note: this clips a little differently than ClipRectsNonScaled.  It first
// CLAMPS the source coordinates to be inside the source rectangle, then clips
// the destination rectangle.
LTBOOL cis_ClipRectsScaled(
    int srcWidth, int srcHeight, int sLeft, int sTop, int sRight, int sBottom,
    int destWidth, int destHeight, int dLeft, int dTop, int dRight, int dBottom,
    LTRect *pSrcRect, LTRect *pDestRect);

LTBOOL cis_ClipRectsNonScaled(
    int srcWidth, int srcHeight, int sLeft, int sTop, int sRight, int sBottom,
    int destWidth, int destHeight, int dLeft, int dTop, LTRect *pSrcRect, LTRect *pDestRect);

void cis_GetWarpCoordinates(WarpCoords *pLeftCoords, WarpCoords *pRightCoords,
    LTWarpPt *pCoords, int nCoords, uint32 &outputMinY, uint32 &outputMaxY);

LTBOOL cis_Clip2dPoly(LTWarpPt* &pCoords, int &nCoords, float rectLeft, float rectTop,
    float rectRight, float rectBottom);

LTRESULT cis_DrawWarp(CisSurface *pDest, CisSurface *pSrc, 
    WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY);

LTRESULT cis_DrawWarpTransparent(CisSurface *pDest, CisSurface *pSrc, 
    WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY);

LTRESULT cis_DrawWarpSolidColor(CisSurface *pDest, CisSurface *pSrc, 
    WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY);


inline LTBOOL cis_IsScreenSurface(CisSurface *pSurface)
{
    return pSurface == &g_ScreenSurface;
}

inline void cis_SetDirty(CisSurface *pSurface)
{
    if (!pSurface)
        return;

    pSurface->m_Flags |= SURFFLAG_OPTIMIZEDIRTY;
}

inline void* cis_LockSurface(CisSurface *pSurface, long &pitch, LTBOOL bSetDirty=LTFALSE)
{
    void *pData;

    if (!g_pCisRenderStruct)
        return LTNULL;

    pitch = 0;

    if (cis_IsScreenSurface(pSurface))
    {
        if (g_pCisRenderStruct->LockScreen(
            0, 0, g_ScreenSurface.m_Width, g_ScreenSurface.m_Height, &pData, &pitch))
        {
            return pData;
        }
        else
        {
            return LTNULL;
        }
    }
    else
    {
        ASSERT(pSurface->m_hBuffer); uint32 iPitch = 0;
        //pitch = pSurface->m_Pitch;

        pData = g_pCisRenderStruct->LockSurface(pSurface->m_hBuffer,iPitch);
        pitch = iPitch;
        if (bSetDirty && pData) {
            cis_SetDirty(pSurface); }

        return pData;
    }
}

inline void cis_UnlockSurface(CisSurface *pSurface)
{
    if (cis_IsScreenSurface(pSurface))
    {
        g_pCisRenderStruct->UnlockScreen();
    }
    else
    {
        g_pCisRenderStruct->UnlockSurface(pSurface->m_hBuffer);
    }
}


inline LTBOOL IsColorTransparent(HLTCOLOR hColor)
{
    return !!(((uint32)hColor) & COLOR_TRANSPARENCY_MASK);
}

    
#endif  // __INTERFACE_HELPERS_H__

