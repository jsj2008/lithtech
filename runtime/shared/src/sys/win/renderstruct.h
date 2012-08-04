#ifndef __RENDERSTRUCT_H__
#define __RENDERSTRUCT_H__

#ifndef __PIXELFORMAT_H__
#include "pixelformat.h"
#endif

#ifndef __D3D9_H__
#include <d3d9.h>
#define __D3D9_H__
#endif

#ifndef __D3DDDSTRUCTS_H__
#include "d3dddstructs.h"
#endif

#ifndef __LIGHTMAPDEFS_H__
#include "lightmapdefs.h"
#endif

class SharedTexture;
class TextureData;
class Attachment;
struct RenderInfoStruct;

#ifndef __RENDEROBJECT_H__
#include "renderobject.h"
#endif

#ifndef NUM_MIPMAPS
    #define NUM_MIPMAPS 8
#endif

#define OPTIMIZE_NO_TRANSPARENCY    0xFFFFFFFF

// Version checking..
#define LTRENDER_VERSION    3421

// Values.
#define RENDER_OK           0
#define RENDER_ERROR        1


// Blit flags.
#define BLIT_TRANSPARENT    1   // Transparent blit.

struct RenderContext;
typedef RenderContext* HRENDERCONTEXT;
typedef void* HLTPARAM;
typedef void* HLTBUFFER;


// Draw modes.
#define DRAWMODE_NORMAL     1   // Render normally.
#define DRAWMODE_OBJECTLIST 2   // Only render the objects in m_pObjectList.

struct IDirect3DDevice9;

// This is passed in when rendering a scene.
struct SceneDesc
{
    // How to draw.  One of the DRAWMODE_ defines above.
    int             m_DrawMode;

	// Rendering statistics counters
	uint32			*m_pTicks_Render_Objects;
	uint32			*m_pTicks_Render_Models;
	uint32			*m_pTicks_Render_Sprites;
	uint32			*m_pTicks_Render_WorldModels;
	uint32			*m_pTicks_Render_ParticleSystems;
    
    LTVector        m_GlobalModelLightAdd;      // Global model light addition.
    
    // The context gotten from CreateContext.
    HRENDERCONTEXT  m_hRenderContext;

    LTVector        m_GlobalLightScale;     // RGB 0-1, scale / darken the light.
    LTVector        m_GlobalLightAdd;       // RGB 0-1, add / brighten the light.  This
                                            // draws a poly over the screen if one of these
                                            // is nonzero (so it's slower than scale).

    float           m_FrameTime;            // Used for updating things like sprites and texture scripts
	float			m_fActualFrameTime;		// The actual elapsed time. Used for performance measurement.

    // Sky definition and objects.
    SkyDef          m_SkyDef;
    LTObject        **m_SkyObjects;
    int             m_nSkyObjects;

    // Viewport rectangle.
    LTRect          m_Rect;

    // Field of view.
    float           m_xFov, m_yFov;

    // Position info.
    LTVector        m_Pos;
    LTRotation      m_Rotation;
    
    // Objects to draw, if mode is DRAWMODE_OBJECTSONLY.
    LTObject        **m_pObjectList;
    int             m_ObjectListSize;

    // If ModelHookFn is set, then the renderer will call it before drawing each model.
    void            (*m_ModelHookFn)(ModelHookData *pData, void *pUser);
    void            *m_ModelHookUser;
};


struct RenderStructInit
{
    int     m_RendererVersion;  // The renderer MUST set this to LTRENDER_VERSION.
    RMode   m_Mode;     // What mode we want to use.
                        // The renderer fills in the actual mode that it set.
    void    *m_hWnd;    // The main window.
};


// A blit command.
class BlitRequest
{
public:

	BlitRequest()
	{
		m_hBuffer = NULL;
		m_BlitOptions = 0;
		m_pSrcRect = NULL;
		m_pDestRect = NULL;
		m_Alpha = 1.0f;
		m_bUseOld = false;
	}


public:

	HLTBUFFER      m_hBuffer;          // The buffer to blit.
	uint32         m_BlitOptions;      // Combination of the BLIT_ flags above.
	GenericColor   m_TransparentColor; // Transparent color.
	LTRect         *m_pSrcRect;        // Source (m_hBuffer) rectangle.
	LTRect         *m_pDestRect;       // Destination rectangle.
	float          m_Alpha;            // Alpha value (0-1).

	LTWarpPt       *m_pWarpPts;
	int            m_nWarpPts;
	bool				m_bUseOld;
};



struct RenderStruct
{
		RenderStruct() : m_bInitted(false), m_bLoaded(false), m_nIn3D(0), m_nInOptimized2D(0) {}

    // Functions LithTech implements.

        // Processes the attachment.  Returns the child object if it exists.
        LTObject*   (*ProcessAttachment)(LTObject *pParent, Attachment *pAttachment);

        // Get a shared texture from a file name  (returns NULL on failure)
        SharedTexture*  (*GetSharedTexture)(const char *pFilename);

        // Gets the texture in memory (guaranteed to be in memory until the next 
        // call to GetTexture).  
        TextureData*    (*GetTexture)(SharedTexture *pTexture);

		// Gets the texture's file name
		const char *	(*GetTextureName)(const SharedTexture *pTexture);

        // Force it to free this texture.
        void            (*FreeTexture)(SharedTexture *pTexture);

        // Runs a string in the console.  The render drivers usually use this
        // so they can get HLTPARAMs right away and not have to check for
        // them all the time.
        void            (*RunConsoleString)(char *pString);

        // Print a string in the console.           
        void            (*ConsolePrint)(char *pMsg, ...);

        // Gets a parameter from the game (something that can be set from the console).
        // Returns NULL if the parameter doesn't exist.
        HLTPARAM        (*GetParameter)(char *pName);
        
        // Gets the value of a parameter .. returns 0/NULL if you pass in NULL.
        float           (*GetParameterValueFloat)(HLTPARAM hParam);
        char*           (*GetParameterValueString)(HLTPARAM hParam);

        // Increments the object frame code.  This is needed for portals.
        uint32          (*IncObjectFrameCode)();
        uint32          (*GetObjectFrameCode)();
        
        // Returns a texture frame code that is guaranteed to not be set in 
        // any SharedTextures.  The renderer is expected to set this frame code on
        // any SharedTextures that it uses while rendering so the engine can know
        // if a texture was viewed or not.
        uint16          (*IncCurTextureFrameCode)();

    // The engine maintains these variables.

        uint32          m_Width;
        uint32          m_Height;
        int             m_bInitted;
        int             m_bLoaded;
		uint32			m_nIn3D;
		uint32			m_nInOptimized2D;


    // The renderer maintains these.

        uint32  m_SystemTextureMemory;      // How much memory the renderer is using for textures.


    // Functions implemented by the render driver.
		
        int             (*Init)(RenderStructInit *pInit);   // Returns RENDER_OK for success, or an error code.
        void            (*Term)(bool bFullTerm);

        IDirect3DDevice9* (*GetD3DDevice)(); // Note: In spring the renderer will link directly with the engine.
                            //  RenderStruct will go away - the renderer will be the only thing that
                            //  needs d3d. The DrawPrim interface lives in the engine for now (and it needs the Device).

        // Any textures you expect the renderer to use must be bound and unbound.
        // If bTextureChanged is TRUE, the renderer should reinitialize its data for the texture
        // even if it's already bound.
        void            (*BindTexture)(SharedTexture *pTexture, bool bTextureChanged);
        void            (*UnbindTexture)(SharedTexture *pTexture);
        D3DFORMAT       (*GetTextureDDFormat1)(BPPIdent BPP, uint32 iFlags);
        bool            (*QueryDDSupport)(PFormat* Format);
        bool            (*GetTextureDDFormat2)(BPPIdent BPP, uint32 iFlags, PFormat* pFormat);
        bool            (*ConvertTexDataToDD)(uint8* pSrcData, PFormat* SrcFormat, uint32 SrcWidth, uint32 SrcHeight, uint8* pDstData, PFormat* DstFormat, BPPIdent eDstType, uint32 nDstFlags, uint32 DstWidth, uint32 DstHeight);

		//called to set a texture for the draw primitive
		void            (*DrawPrimSetTexture)(SharedTexture *pTexture);
        void            (*DrawPrimDisableTextures)();

        // You render through a context.  Note: LithTech frees all of its lightmap data
        // after calling this because it assumes you converted it all into a more suitable format.
        HRENDERCONTEXT  (*CreateContext)();
        void            (*DeleteContext)(HRENDERCONTEXT hContext);

        // Clear a section of the screen.  Flags are from CLEARSCREEN_ flags in de_codes.h.
        void            (*Clear)(LTRect *pRect, uint32 flags, LTRGBColor& ClearColor);

        // Used around render calls.
        bool            (*Start3D)();
        bool            (*End3D)();
        bool            (*IsIn3D)();

        bool            (*StartOptimized2D)();
        void            (*EndOptimized2D)();
        bool            (*IsInOptimized2D)();
        bool            (*SetOptimized2DBlend)(LTSurfaceBlend blend);
        bool            (*GetOptimized2DBlend)(LTSurfaceBlend &blend);
        bool            (*SetOptimized2DColor)(HLTCOLOR color);
        bool            (*GetOptimized2DColor)(HLTCOLOR &color);
        
        // Render a scene.
        int             (*RenderScene)(SceneDesc *pScene);

        // Handle a command from the console.
        void            (*RenderCommand)(int argc, char **argv);

        // Show the backbuffer.
        void            (*SwapBuffers)(uint flags );

        // Get the screen pixel format.
        bool            (*GetScreenFormat)(PFormat *pFormat);

        
        HLTBUFFER       (*CreateSurface)(int width, int height);
        void            (*DeleteSurface)(HLTBUFFER hSurf);

        void            (*GetSurfaceInfo)(HLTBUFFER hSurf, uint32 *pWidth, uint32 *pHeight);
        
        void*           (*LockSurface)(HLTBUFFER hSurf, uint32& Pitch);
        void            (*UnlockSurface)(HLTBUFFER hSurf);

        // Set transparentColor to OPTIMIZE_NO_TRANSPARENCY to not use transparency.
        bool            (*OptimizeSurface)(HLTBUFFER hSurf, uint32 transparentColor);
        void            (*UnoptimizeSurface)(HLTBUFFER hSurf);

        // Note: whenever you do this, you stall all async performance!
        bool            (*LockScreen)(int left, int top, int right, int bottom, void **pData, long *pPitch);
        void            (*UnlockScreen)();

        // Blit a surface to the screen.
        // No clipping is performed so it WILL crash if you don't clip the coordinates!
        // This function CAN be NULL if the driver doesn't want to support this, in
        // which case the engine will just lock the screen and do it itself.
        void            (*BlitToScreen)(BlitRequest *pRequest);
        bool            (*WarpToScreen)(BlitRequest *pRequest);

        // Make a screenshot file.
        void            (*MakeScreenShot)(const char *pFilename);

		//Generates a series of images that form a cubic environment map of the form Prefix[FW|BK|LF|RI|UP|DW].bmp
		//aligned along the world's basis space from the given position
		void			(*MakeCubicEnvMap)(const char* pszPrefix, uint32 nSize, const SceneDesc& InSceneDesc);

        // Reads in new console variable values.
        void            (*ReadConsoleVariables)();

        // Get the current render info
        void            (*GetRenderInfo)(RenderInfoStruct *pStruct);

        // Blit from the screen.

        void            (*BlitFromScreen)(BlitRequest *pRequest);
        
        // Creating RenderObjects...
        CRenderObject*  (*CreateRenderObject)(CRenderObject::RENDER_OBJECT_TYPES ObjectType);
        bool            (*DestroyRenderObject)(CRenderObject* pObject);

		// Load rendering data from the specified stream
		bool			(*LoadWorldData)(ILTStream *pStream);

		// Change the color of a lightgroup in the currently loaded world
		// Returns false if a world isn't loaded
		bool			(*SetLightGroupColor)(uint32 nID, const LTVector &vColor);

		// Change/query the state of an occluder in the currently loaded world
		// Returns LT_NOTFOUND if the ID isn't found or LT_NOTINWORLD if a world isn't loaded
		LTRESULT		(*SetOccluderEnabled)(uint32 nID, bool bEnabled);
		LTRESULT		(*GetOccluderEnabled)(uint32 nID, bool *pEnabled);

		// Accessing texture effect variables
		uint32			(*GetTextureEffectVarID)(const char* pszEffectGroup, uint32 nStage);
		bool			(*SetTextureEffectVar)(uint32 nVarID, uint32 nVar, float fValue);

		// Access to the different object groups
		bool			(*IsObjectGroupEnabled)(uint32 nGroup);
		void			(*SetObjectGroupEnabled)(uint32 nGroup, bool bEnable);
		void			(*SetAllObjectGroupEnabled)();

		// Access to the render style map used when rendering the glow effect
		bool			(*AddGlowRenderStyleMapping)(const char* pszSource, const char* pszMapTo);
		bool			(*SetGlowDefaultRenderStyle)(const char* pszFile);
		bool			(*SetNoGlowRenderStyle)(const char* pszFile);

        // This stuff MUST come last so it doesn't get zeroed out when switching res.

        int             m_DontClearMarker;

        LTVector		m_GlobalLightDir;
        LTVector		m_GlobalLightColor;
		float			m_GlobalLightConvertToAmbient;

		// Timing variables
		uint32			m_Time_Vis;
};



// This is what you use to select how you want to initialize the renderer..  Get a list of
// the modes it supports, then copy the desired mode and pass it into RenderStruct::Init().

// Get the list with GetSupportedModes() and pass that into FreeModeList() to free it.
typedef RMode* (*GetSupportedModesFn)();
typedef void (*FreeModeListFn)(RMode *pHead);


// To make a DirectEngine rendering DLL, make a DLL with the function 
// "RenderDLLSetup" that looks like this.  This function should init all the
// function pointers in the structure.
typedef void (*RenderDLLSetupFn)(RenderStruct *pStruct);


#endif  // __RENDERSTRUCT_H__

