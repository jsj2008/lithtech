//-------------------------------------------------------------------
//
//   MODULE    : ILTDRAWPRIM.H
//
//   PURPOSE   : Defines engine interface ILTDrawPrimitive
//
//   CREATED   : On 8/11/00 At 10:40:21 AM
//
//-------------------------------------------------------------------

#ifndef __ILTDRAWPRIM_H__
#define __ILTDRAWPRIM_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __LTMODULT_H__
#include "ltmodule.h"
#endif

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


// screen near z -- use this value if you want your polygons to draw
// in front of everything

#ifdef _WIN32
#define SCREEN_NEAR_Z       0.0
#endif


//-----------------------------
// Vertex Primitive Definitions
//-----------------------------

// RGBA Colour def
/*!
  Contains the RGBA data for a vertex. Also provides the Compact
  operator to convert the RGBA data to a uint32. Gouraud shaded
  vertices use this structure. 
*/
struct LT_VERTRGBA
{
    // convert r,g,b,a to a uint32
    uint32 Compact() { 
        return (((uint32)a << 24) | ((uint32)r << 16) | 
               ((uint32)g << 8) | (uint32)b);
    }

    uint8 b, g, r, a;
};

// Textured gouraud shaded vertex
/*!
  Contains coordinate, color, and texture information for a textured,
  Gouraud shaded vertex.
*/
struct LT_VERTGT
{
    float                           x,y,z;

    LT_VERTRGBA                     rgba;
    float                           u, v;

	LT_VERTGT() : z(SCREEN_NEAR_Z) {}
};

// Textured flat shaded vertex
/*!
Contains coordinate and texture information for a textured, flat-shaded vertex.
*/
struct LT_VERTFT
{       
    float                           x,y,z;
    float                           u, v;

	LT_VERTFT() : z(SCREEN_NEAR_Z) {}
};

// Gouraud shaded vertex
/*!
Contains coordinate and color information for a Gouraud-shaded vertex.
*/
struct LT_VERTG
{       
    float                           x,y,z;
    LT_VERTRGBA                     rgba;

	LT_VERTG() : z(SCREEN_NEAR_Z) {}
};

// Flat shaded vertex
/*!
Contains coordinate information for a flat-shaded vertex.
*/
struct LT_VERTF
{       
    float                           x,y,z;

	LT_VERTF() : z(SCREEN_NEAR_Z) {}
};

//------------------------------
// Polygon Primitive Definitions
//------------------------------

// Textured gouraud shaded triangle
/*!
Contains vertex structure information for a textured, Gouraud shaded triangle.
*/
struct LT_POLYGT3
{
    LT_VERTGT                       verts[3];
};

// Textured flat shaded triangle
/*!
Contains vertex structure information and color information for a textured, flat-shaded triangle.
*/
struct LT_POLYFT3
{
    LT_VERTFT                       verts[3];
    LT_VERTRGBA                     rgba;
};

// Gouruad shaded triangle
/*!
Contains vertex structure information for a Gouraud shaded triangle.
*/
struct LT_POLYG3
{
    LT_VERTG                        verts[3];
};

// Flat shaded triangle
/*!
Contains vertex structure information and color informaiton for a flat-shaded triangle.
*/
struct LT_POLYF3
{
    LT_VERTF                        verts[3];
    LT_VERTRGBA                     rgba;
};

// Textured gouraud shaded quad
/*!
Contains vertex structure information for a textured, Gouraud shaded quadrilateral.
*/
struct LT_POLYGT4
{
    LT_VERTGT                       verts[4];
};

// Textured flat shaded quad
/*!
Contains vertex structure information and color information for a textured, flat-shaded triangle.
*/
struct LT_POLYFT4
{
    LT_VERTFT                       verts[4];
    LT_VERTRGBA                     rgba;
};

// Gouruad shaded quad
/*!
Contains vertex structure information for a Gouraud shaded quadrilateral.
*/
struct LT_POLYG4
{
    LT_VERTG                        verts[4];
};

// Flat shaded quad
/*!
Contains vertex structure information and color information for flat
shaded quadrilateral.
*/
struct LT_POLYF4
{
    LT_VERTF                        verts[4];
    LT_VERTRGBA                     rgba;
};

// Textured gouraud shaded line
/*!
Contains vertex structure information for a textured, Gouraud shaded line.
*/
struct LT_LINEGT
{
    LT_VERTGT                       verts[2];
};

// Textured flat shaded line
/*!
Contains vertex structure information and color information for a textured, flat-shaded line.
*/
struct LT_LINEFT
{
    LT_VERTFT                       verts[2];
    LT_VERTRGBA                     rgba;
};

// Gouruad shaded line
/*!
Contains vertex structure information for a Gouraud shaded line.
*/
struct LT_LINEG
{
    LT_VERTG                        verts[2];
};

// Flat shaded line
/*!
Contains vertex structure information and color information for a flat
shaded line.
*/
struct LT_LINEF
{
    LT_VERTF                        verts[2];
    LT_VERTRGBA                     rgba;
};

//------------------
// ILTDrawPrim enums
//------------------

/*!
Enumerates the transform types in which a primitive is drawn.
\see ILTDrawPrim::SetTransformType()
*/
enum ELTTransformType
{ 
    /*!
    The primitive is drawn in camera space.
    */
    DRAWPRIM_TRANSFORM_CAMERA,
    /*!
    The primitive is drawn in screen space.
    */
    DRAWPRIM_TRANSFORM_SCREEN,
    /*!
    The primitive is drawn in world space.
    */
    DRAWPRIM_TRANSFORM_WORLD
} ;


// Function decoder : 
//  Cs - Source Color, As - Source Alpha, Cd - Dest Color, Ad - Dest Alpha
/*!
Enumerates the alpha blend modes with which a primitive is drawn. The blend mode defines how the source color (Cs), source alpha value(As), destination color(Cd), and destination alpha value(Ad) interact. PlayStation 2 (which does not support full collor modulate) supports the \b DRAWPRIM_BLEND_ADD, \b DRAWPRIM_BLEND_MOD_SRCALPHA, and \b DRAWPRIM_NOBLEND values.
\see ILTDrawPrim::SetAlphaBlendMode()
*/
enum  ELTBlendMode
{
    /*!
    No blending is done. The source color is drawn with no modification. This is the default value.
    */
    DRAWPRIM_NOBLEND,
    /*!
    The source color is added to the destination color (Cs + Cd).
    */
    DRAWPRIM_BLEND_ADD,                 
    /*!
    Cs * (1 - Cd) + Cd
    */
    DRAWPRIM_BLEND_SATURATE,            
    /*!
    Cs * As + Cd * (1 - As)
    */
    DRAWPRIM_BLEND_MOD_SRCALPHA,
    /*!
    Cs * Cs + Cd * (1 - Cs)
    */
    DRAWPRIM_BLEND_MOD_SRCCOLOR,    
    /*!
    Cs * Cd + Cd * (1 - Cd)
    */
    DRAWPRIM_BLEND_MOD_DSTCOLOR,        
    /*!
    Cs * Cs + Cd * Cd
    */
    DRAWPRIM_BLEND_MUL_SRCCOL_DSTCOL,   
    /*!
    Cs * As + Cd
    */
    DRAWPRIM_BLEND_MUL_SRCALPHA_ONE,
    /*!
    Cs * As
    */
    DRAWPRIM_BLEND_MUL_SRCALPHA,
    /*!
    Cs * Cs + Cd
    */
    DRAWPRIM_BLEND_MUL_SRCCOL_ONE,      
    /*!
    Cs * Cd
    */
    DRAWPRIM_BLEND_MUL_DSTCOL_ZERO      
};


/*!
Enumerates the z-buffering modes for drawing primitives.
\see ILTDrawPrim::SetZBufferMode()
*/
enum  ELTZBufferMode
{
    /*!
    Read/write z-buffering enabled.
    */
    DRAWPRIM_ZRW,
    /*!
    Read-only z-buffering enabled.
    */
    DRAWPRIM_ZRO,               
    /*!
    No z-buffering.
    */
    DRAWPRIM_NOZ
};

// Pixels pass the AlphaTest if they satisfy the AlphaTest specified
//  alpha test function - from the list below. AlphaRef Value is 0.5f.
/*!
Enumerates the modes for alpha testing on primitives. An alpha test compares the vertex alpha value of each pixel in the primitive to the alpha value fo the texture. If the test is true, then the pixel is rendered. If the test is false, the pixel does not render.
\see ILTDrawPrim::SetTransformType()
*/
enum  ELTTestMode
{
    /*!
    No test is conducted. All pixels in the primitive are drawn.    
    */
    DRAWPRIM_NOALPHATEST,
    /*!
    If the alpha value of the pixel is less than the alpha value of the texel, the pixel is drawn.
    */
    DRAWPRIM_ALPHATEST_LESS,
    /*!
    If the alpha value of the pixel is less than or equal to the alpha value of the texel, the pixel is drawn.
    */
    DRAWPRIM_ALPHATEST_LESSEQUAL,
    /*!
    If the alpha value of the pixel is greater than the alpha value of the texel, the pixel is drawn.
    */
    DRAWPRIM_ALPHATEST_GREATER,
    /*!
    If the alpha value of the pixel is greater than or equal to the alpha value of the texel, the pixel is drawn.
    */
    DRAWPRIM_ALPHATEST_GREATEREQUAL,
    /*!
    If the alpha value of the pixel is equal to the alpha value of the texel, the pixel is drawn.
    */
    DRAWPRIM_ALPHATEST_EQUAL,
    /*!
    If the alpha value of the pixel is not equal to the alpha value of the texel, the pixel is drawn.
    */
    DRAWPRIM_ALPHATEST_NOTEQUAL
};

/*!
Enumerates the texture-to-color behaviors for primitives. This state is only useful for textured primitives. 
\see ILTDrawPrim::SetColorOp()
*/
enum ELTColorOp
{
    /*!
    The color of the texture is ignored and only the color of the primitive is drawn.   
    */
    DRAWPRIM_NOCOLOROP,
    /*!
    Each of the pixel colors of the primitive is multiplied with the pixel color of the texture.
    */
    DRAWPRIM_MODULATE,
    /*!
    Each of the pixel colors of the primitive is added to the pixel color of the texture. This value is not supported on the PlayStation 2 platform.
    */
    DRAWPRIM_ADD,
    /*!
    The color of the primitive is ignored and only the color of the texture is drawn.
    */
    DRAWPRIM_DECAL,
};

/*!
Enumerates the clip modes for drawing primitives.
\see ILTDrawPrim::SetClipMode()
*/
enum ELTClipMode
{
    /*!
    The primitive is not clipped if it penetrates the guardband.    
    */
    DRAWPRIM_NOCLIP,
    /*!
    If any portion of the primitive lies outside the guardband it will be rejected and no portion of the primitive will be drawn.
    */
    DRAWPRIM_FASTCLIP,
    /*!
    Portions of the primitive that lie outside the guardband will not be rendered.
    */
    DRAWPRIM_FULLCLIP
} ;

/*!
Enumerates the fill modes for drawing primitives.
\see ILTDrawPrim::SetFillMode()
*/
enum ELTDPFillMode
{
    /*!
    Only a wire frame is drawn for the primitive.
    */
    DRAWPRIM_WIRE,
    /*!
    The primitive is filled with the appropriate color and texture. 
    */
    DRAWPRIM_FILL
};

/*!
Enumerates the cull modes for drawing primitives.
\see ILTDrawPrim::SetCullMode()
*/
enum ELTDPCullMode
{
    /*!
    No culling is done. All primitives are rendered regardless of winding order.
    */
    DRAWPRIM_CULL_NONE,                 // Don't CULL
    /*!
    Primitives with counter-clockwise winding orders relative to the camera are not rendered.   
    */
    DRAWPRIM_CULL_CCW,                  
    /*!
    Primtives with clockwise winding orders relative to the camera are not rendered.
    */
    DRAWPRIM_CULL_CW                    // CULL Clockwise
};

/*!
The ILTDrawPrim is a low level polygon based drawing/rendering interface.

Define a holder to get this interface like this:
\code
define_holder(ILTDrawPrim, your_var);
\endcode
*/
class ILTDrawPrim : public IBase
{
    public :
        interface_version(ILTDrawPrim, 0);

		/*!
        Called to begin a draw primitive block. This is for optimizing draw primitive
		calls. Calls to have the engine render anything not through the draw primitive
		cannot be made inside this block.
        
        Used for: Rendering.
        */
		virtual LTRESULT BeginDrawPrim() = 0;

		/*!
        Called to end a draw primitive block. 
        
        Used for: Rendering.
        */
		virtual LTRESULT EndDrawPrim() = 0;

        /*!
        \param hCamera  The handle of the camera.
        
        Set the camera relative to which the primitive is drawn.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetCamera(const HOBJECT hCamera) = 0;

        /*!
        \param hTexure  The handle of the texture.
        
        Set the texture used by the primitive. If a texture is not set using this function, all textured polygons will be rendered with random texture data.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetTexture(const HTEXTURE hTexture) = 0;

        /*!
        \param eType    An \b ELTTransformType enum value.
        
        Set the transform type in which the primitive is drawn.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetTransformType(const ELTTransformType eType) = 0;
        
        /*!
        \param eColorOp An \b ELTColorOp enum value.
        
        Set the texture-to-color behavior of the primitive.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetColorOp(const ELTColorOp eColorOp) = 0;

        /*!
        \param eBlendMode An \b ELTBlendMode enum value.
        
        Set the alpha blend mode of the primitive.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetAlphaBlendMode(const ELTBlendMode eBlendMode) = 0;
        
        /*!
        \param eZBufferMode An \b ELTZBufferMode enum value.
        
        Set the z-buffering mode for drawing primitives.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetZBufferMode(const ELTZBufferMode eZBufferMode) = 0;
        
        /*!
        \param eTestMode An \b ELTTestMode enum value.
        
        Set the alpha test mode for drawing primitives.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetAlphaTestMode(const ELTTestMode eTestMode) = 0;
        
        /*!
        \param eClipMode An \b ELTClipMode enum value.
        
        Set the clip mode for drawing primitives.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetClipMode(const ELTClipMode eClipMode) = 0;
        
        /*!
        \param efillmode An \b ELTDPFillMode enum value.
        
        Set the fill mode for drawing primitives.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetFillMode(ELTDPFillMode efillmode) = 0;
        
        /*!
        \param ecullmode An \b ELTDPCullMode enum value.
        
        Set the cull mode for drawing primitives.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetCullMode(ELTDPCullMode ecullmode) = 0;

		/*!
        \param bFogEnable An \b boolean indicating whether or not fog should be enabled
        
        Set the fog enabled status for drawing primitives.
        
        Used for: Rendering.
        */
        virtual LTRESULT SetFogEnable(bool bFogEnable) = 0;

		/*!
        \param bReallyClose - Specifies whether or not the draw primitive will be rendered in the really close range.
        
        Specifiy whether or not to be in really close space for rendering
        
        Used for: Rendering.
        */
        virtual LTRESULT SetReallyClose(bool bReallyClose) = 0;

		/*!
		\param nEffectShaderID - Specifies the effect shader ID.

		Specify the Effect Shader to use when rendering this prim.

		Used for: Rendering.
		*/
		virtual LTRESULT SetEffectShaderID(uint32 nEffectShaderID) = 0;
    
        /*!
		Store off the current viweport

		Used for: Rendering.
		*/
		virtual void SaveViewport() = 0;

		/*!
		Restore the saved viewport

		Used for: Rendering.
		*/
		virtual void RestoreViewport() = 0;

		/*!
        \param pPrim        A pointer to an array of polygon structures to draw.
        \param nCount       The number of polygons (default value is 1) in the array identified by the \b pPrim parameter.

        Draw one or more polgons of one type (line, triangle, or quadrilateral). This overloaded function supports the following structures: \b LT_LINEF, \b LT_LINEFT, \b LT_LINEG, \b LT_LINEGT, \b LT_POLYF3, \b LT_POLYFT3, \b LT_POLYG3, \b LT_POLYGT3, \b LT_POLYF4, \b LT_POLYFT4, \b LT_POLYG4, and\b LT_POLYGT4.
  
        
        Used for: Rendering.
        */
        virtual LTRESULT DrawPrim (LT_POLYGT3 *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_POLYFT3 *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_POLYG3 *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_POLYF3 *pPrim, 
                                   const uint32 nCount = 1) = 0;

        virtual LTRESULT DrawPrim (LT_POLYGT4 *pPrim, 
                                   const uint32 nCount = 1) = 0;

		// special version added by adam s. for optimized wide font rendering
		virtual LTRESULT DrawPrim (LT_POLYGT4 **ppPrim, 
                                   const uint32 nCount = 1) = 0;

        virtual LTRESULT DrawPrim (LT_POLYFT4 *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_POLYG4 *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_POLYF4 *pPrim, 
                                   const uint32 nCount = 1) = 0;

        virtual LTRESULT DrawPrim (LT_LINEGT *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_LINEFT *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_LINEG *pPrim, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrim (LT_LINEF *pPrim, 
                                   const uint32 nCount = 1) = 0;

        virtual LTRESULT DrawPrimPoint (LT_VERTGT *pVerts, 
                                   const uint32 nCount = 1) = 0;
        virtual LTRESULT DrawPrimPoint (LT_VERTG *pVerts, 
                                   const uint32 nCount = 1) = 0;

        /*!
        \param pVerts       A pointer to an array structures that identify the vertices of the fan. Three or more vertices are required. The first vertex structure in this array defines the source vertex for the fan. This overloaded functions supports the following vertex structures: \b LT_VERTF, \b LT_VERTFT, \b LT_VERTG, and\b LT_VERTGT.
        \param nCount       The number of vertex structures in the array identified by the \b pVerts parameter.
        \param rgba         The color for flat shaded fans.

        Draw a primitive triangle fan. Each vertex in the array creates a triangle with the previous vertex in the arary and the source vertex.

        
        Used for: Rendering.
        */
        virtual LTRESULT DrawPrimFan (LT_VERTGT *pVerts, 
                                      const uint32 nCount) = 0;
        virtual LTRESULT DrawPrimFan (LT_VERTFT *pVerts, 
                                      const uint32 nCount, 
                                      LT_VERTRGBA rgba) = 0;
        virtual LTRESULT DrawPrimFan (LT_VERTG *pVerts, 
                                      const uint32 nCount) = 0;
        virtual LTRESULT DrawPrimFan (LT_VERTF *pVerts, 
                                      const uint32 nCount, 
                                      LT_VERTRGBA rgba) = 0;

        /*!
        \param pVerts       A pointer to an array of structures that identify the vertices of the strip. Three or more vertices are required. The first vertex structure in this array defines the source vertex for the strip. This overloaded functions supports the following vertex structures: \b LT_VERTF, \b LT_VERTFT, \b LT_VERTG, and\b LT_VERTGT.
        \param nCount       The number of vertex structures in the array identified by the \b pVerts parameter.
        \param rgba         The color for flat shaded strips.

        Draw a primitive triangle strip. Each vertex in the array creates a triangle with the previous two vertices in the array. 

        
        Used for: Rendering.
        */
        virtual LTRESULT DrawPrimStrip (LT_VERTGT *pVerts, 
                                        const uint32 nCount) = 0;
        virtual LTRESULT DrawPrimStrip (LT_VERTFT *pVerts, 
                                        const uint32 nCount, 
                                        LT_VERTRGBA rgba) = 0;
        virtual LTRESULT DrawPrimStrip (LT_VERTG *pVerts, 
                                        const uint32 nCount) = 0;
        virtual LTRESULT DrawPrimStrip (LT_VERTF *pVerts, 
                                        const uint32 nCount, 
                                        LT_VERTRGBA rgba) = 0;

        /*!
        \param pPrim (return) A pointer to the polygon structure to which the coordinate parameters \b x0, \b y0, \b x1, \b y1, \b x2, \b y2, \b x3, and \b y3 are applied. This overloaded function supports the \b LT_POLYF4, \b LT_POLYFT4, \b LT_POLYG4, and \b LT_POLYGT4 structures.
        \param x0   The x-coordinate of the first vertex of the quadrilateral. All eight of the coordinate parameters are applied to the quadrilateral in winding order.
        \param y0   The y-coordinate of the first vertex of the quadrilateral.
        \param x1   The x-coordinate of the second vertex of the quadrilateral.
        \param y1   The y-coordinate of the second vertex of the quadrilateral.
        \param x2   The x-coordinate of the third vertex of the quadrilateral.
        \param y2   The y-coordinate of the third vertex of the quadrilateral.
        \param x3   The x-coordinate of the fourth vertex of the quadrilateral.
        \param y3   The y-coordinate of the fourth vertex of the quadrilateral.

        Set vertex coordinates for a primitive quadrilateral.

        
        Used for: Rendering.
        */
        virtual void SetXY4 (LT_POLYGT4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3) = 0;

        virtual void SetXY4 (LT_POLYFT4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3) = 0;
        
        virtual void SetXY4 (LT_POLYG4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3) = 0;

        virtual void SetXY4 (LT_POLYF4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3) = 0;
        
        /*!
        \param pPrim (return) A pointer to the polygon structure for which the vertices is set.
        \param x    The x-coordinate of the upper left vertex of the quadrilateral. 
        \param y    The y-coordinate of the upper left vertex of the quadrilateral.
        \param w    The width of the quadrilateral.
        \param h    The height the quadrilateral.

        Set vertex coordinates for a primitive quadrilateral give a source vertex, width, and height. The x and y coordinates for the upper left vertex are set with the \b x and \b y parameters. The other three vertices are calculated using the width and height (\b w and \b h parameters, respectively).

        
        Used for: Rendering.
        */
        virtual void SetXYWH (LT_POLYGT4 *pPrim,
                             float x, float y,
                             float w, float h) = 0;

        virtual void SetXYWH (LT_POLYFT4 *pPrim,
                             float x, float y,
                             float w, float h) = 0;

        virtual void SetXYWH (LT_POLYG4 *pPrim,
                             float x, float y,
                             float w, float h) = 0;
        
        virtual void SetXYWH (LT_POLYF4 *pPrim,
                             float x, float y,
                             float w, float h) = 0;
        
        /*!
        \param pPrim (return) A pointer to the polygon structure for which the texture data is set. This overloaded function supports the \b LT_POLYGT4 and \b LT_POLYFT4 structures.
        \param u0   The u texture value for the upper left vertex of the quadrilateral.
        \param v0   The v texture value for the upper left vertex of the quadrilateral.
        \param u1   The u texture value for the second vertex of the quadrilateral.
        \param v1   The v texture value for the second vertex of the quadrilateral.
        \param u2   The u texture value for the third vertex of the quadrilateral.
        \param v2   The v texture value for the third vertex of the quadrilateral.
        \param u3   The u texture value for the fourth vertex of the quadrilateral.
        \param v3   The v texture value for the fourth vertex of the quadrilateral.

        Set the texture data for each vertex in a quadrilateral structure.
        
        Used for: Rendering.
        */
        virtual void SetUV4 (LT_POLYGT4 *pPrim,
                             float u0, float v0,
                             float u1, float v1,
                             float u2, float v2,
                             float u3, float v3) = 0;

        virtual void SetUV4 (LT_POLYFT4 *pPrim,
                             float u0, float v0,
                             float u1, float v1,
                             float u2, float v2,
                             float u3, float v3) = 0;
                
        /*!
        \param pPrim (return) A pointer to the polygon structure for which the texture data is set. This overloaded function supports the \b LT_POLYGT4 and \b LT_POLYFT4 structures.
        \param u    The u texture value for the upper left vertex of the quadrilateral.
        \param v    The v texture value for the upper left vertex of the quadrilateral.
        \param w    The width of the texture.
        \param h    The height of the texture.
        
        Set the texture data for each vertex in a quadrilateral structure.  The upper left vertex is set explicity by the \b u and \b v parameters. The other three vertices are calculated using the width and height (\b w and \b h parameters, respectively).
        
        Used for: Rendering.
        */
        virtual void SetUVWH (LT_POLYGT4 *pPrim,
                             float u, float v,
                             float w, float h) = 0;

		virtual void SetUVWH (LT_POLYGT4 *pPrim, HTEXTURE pTex,
                             float u, float v,
                             float w, float h) = 0;

        virtual void SetUVWH (LT_POLYFT4 *pPrim,
                             float u, float v,
                             float w, float h) = 0;

        
        /*!
        \param pPrim (return) A pointer to the polygon structure for which the color data is set. This overloaded function supports the \b LT_POLYF4, \b LT_POLYG4, \b LT_POLYGT4 and \b LT_POLYFT4 structures.
        \param color    The RGB color of all vertices in the quadrilateral.
        
        Set a single RGB color for all vertices in a quadrilateral structure.
        
        Used for: Rendering.
        */
        virtual void SetRGB(LT_POLYGT4 *pPrim, uint32 color) = 0;
        virtual void SetRGB(LT_POLYFT4 *pPrim, uint32 color) = 0;
        virtual void SetRGB(LT_POLYG4 *pPrim, uint32 color) = 0;
        virtual void SetRGB(LT_POLYF4 *pPrim, uint32 color) = 0;
        
        /*!
        \param pPrim (return) A pointer to the polygon structure for which the color data is set. This overloaded function supports the \b LT_POLYF4, \b LT_POLYG4, \b LT_POLYGT4 and \b LT_POLYFT4 structures.
        \param color    The RGBA color of all vertices in the quadrilateral.
        
        Set a single RGBA color for all vertices in a quadrilateral structure.
        
        Used for: Rendering.
        */
        virtual void SetRGBA(LT_POLYGT4 *pPrim, uint32 color) = 0;
        virtual void SetRGBA(LT_POLYFT4 *pPrim, uint32 color) = 0;
        virtual void SetRGBA(LT_POLYG4 *pPrim, uint32 color) = 0;
        virtual void SetRGBA(LT_POLYF4 *pPrim, uint32 color) = 0;
        
 
        // ------------------------------------------
        
        // Note: it makes no sense to have a SetRGB4 on flat shaded polys

        /*!
        \param pPrim (return) A pointer to the polygon structure for which the color data is set. This overloaded function supports the \b LT_POLYG4 and \b LT_POLYGT4 structures.
        \param color0   The RGB color of the first (upper left) vertex of the quadrilateral.
        \param color1   The RGB color of the second vertex of the quadrilateral.
        \param color2   The RGB color of the third vertex of the quadrilateral.
        \param color3   The RGB color of the fourth vertex of the quadrilateral.
        
        Set the RGB color for each vertex in a quadrilateral.
        
        Used for: Rendering.
        */
        virtual void SetRGB4(LT_POLYGT4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3) = 0;


        virtual void SetRGB4(LT_POLYG4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3) = 0;
        
        /*!
        \param pPrim (return) A pointer to the polygon structure for which the color data is set. This overloaded function supports the \b LT_POLYG4 and \b LT_POLYGT4 structures.
        \param color0   The RGBA color of the first (upper left) vertex of the quadrilateral.
        \param color1   The RGBA color of the second vertex of the quadrilateral.
        \param color2   The RGBA color of the third vertex of the quadrilateral.
        \param color3   The RGBA color of the fourth vertex of the quadrilateral.
        
        Set the RGBA color for each vertex in a quadrilateral.
        
        Used for: Rendering.
        */
        virtual void SetRGBA4(LT_POLYGT4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3) = 0;

        virtual void SetRGBA4(LT_POLYG4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3) = 0;

        /*!
        \param pPrim (return) A pointer to the polygon structure for which the color data is set. This overloaded function supports the \b LT_POLYF4, \b LT_POLYFT4, \b LT_POLYG4 and \b LT_POLYGT4 structures.
        \param alpha The alpha value for the entire quadrilateral.
        
        Set the alpha value for a quadrilateral.
        
        Used for: Rendering.
        */
        virtual void SetALPHA(LT_POLYGT4 *pPrim, uint8 alpha) = 0;
        virtual void SetALPHA(LT_POLYG4 *pPrim, uint8 alpha)  = 0;
        virtual void SetALPHA(LT_POLYFT4 *pPrim, uint8 alpha) = 0;
        virtual void SetALPHA(LT_POLYF4 *pPrim, uint8 alpha)  = 0;
        
        /*!
        \param pPrim (return) A pointer to the polygon structure for which the alpha value is set. This overloaded function supports the \b LT_POLYG4 and \b LT_POLYGT4 structures.
        \param alpha0   The alpha value of the first (upper left) vertex of the quadrilateral.
        \param alpha1   The alpha value of the second vertex of the quadrilateral.
        \param alpha2   The alpha value of the third vertex of the quadrilateral.
        \param alpha3   The alpha value of the fourth vertex of the quadrilateral.
        
        Set the alpha value for each vertex in a quadrilateral.
        
        Used for: Rendering.
        */
        virtual void SetALPHA4(LT_POLYGT4 *pPrim, 
                             uint8 alpha0, uint8 alpha1,
                             uint8 alpha2, uint8 alpha3) = 0;
        virtual void SetALPHA4(LT_POLYG4 *pPrim, 
                             uint8 alpha0, uint8 alpha1,
                             uint8 alpha2, uint8 alpha3) = 0;
};

// For now just expose this to everyone to use.
// extern ILTDrawPrim* gDrawPrim;

#endif // __ILTDRAWPRIM_H_

