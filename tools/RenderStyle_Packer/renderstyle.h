// renderstyle.h
//	Defines a render style. The renderer supports setting render styles for render
// objects.

#ifndef __RENDERSTYLE_H__
#define __RENDERSTYLE_H__

#include <string>

using namespace std;

#define MAX_NUM_TEXTURES_PER_PASS			4

// RENDER STATE TYPES
enum ERenStyle_ZBufferMode {
	RENDERSTYLE_ZRW,						// Z Read/Write
	RENDERSTYLE_ZRO,						// Z Read Only
	RENDERSTYLE_NOZ };						// No Z

enum ERenStyle_TestMode {
	RENDERSTYLE_NOALPHATEST,
	RENDERSTYLE_ALPHATEST_LESS,
	RENDERSTYLE_ALPHATEST_LESSEQUAL,
	RENDERSTYLE_ALPHATEST_GREATER,
	RENDERSTYLE_ALPHATEST_GREATEREQUAL,
	RENDERSTYLE_ALPHATEST_EQUAL,
	RENDERSTYLE_ALPHATEST_NOTEQUAL };

enum ERenStyle_FillMode {
    RENDERSTYLE_WIRE,
    RENDERSTYLE_FILL };

enum ERenStyle_CullMode {
    RENDERSTYLE_CULL_NONE,					// Don't CULL
    RENDERSTYLE_CULL_CCW,					// CULL Counter Clockwise
	RENDERSTYLE_CULL_CW };					// CULL Clockwise

// TEXTURE STAGE TYPES
enum ERenStyle_ColorOp
{
	RENDERSTYLE_COLOROP_DISABLE,
	RENDERSTYLE_COLOROP_SELECTARG1,
	RENDERSTYLE_COLOROP_SELECTARG2,
	RENDERSTYLE_COLOROP_MODULATE,
	RENDERSTYLE_COLOROP_MODULATE2X,
	RENDERSTYLE_COLOROP_MODULATEALPHA,
	RENDERSTYLE_COLOROP_MODULATETFACTOR,
	RENDERSTYLE_COLOROP_ADD,
	RENDERSTYLE_COLOROP_DOTPRODUCT3,
	RENDERSTYLE_COLOROP_BUMPENVMAP,
	RENDERSTYLE_COLOROP_BUMPENVMAPLUM,
//OLD PS2	RENDERSTYLE_COLOROP_DECAL,
//	RENDERSTYLE_COLOROP_HIGHLIGHT,
//	RENDERSTYLE_COLOROP_HIGHLIGHT2,
	RENDERSTYLE_COLOROP_ADDSIGNED,
	RENDERSTYLE_COLOROP_ADDSIGNED2X,
	RENDERSTYLE_COLOROP_SUBTRACT,
	RENDERSTYLE_COLOROP_ADDMODALPHA,
	RENDERSTYLE_COLOROP_ADDMODINVALPHA,
	RENDERSTYLE_COLOROP_MODULATETEXALPHA
};

enum ERenStyle_ColorArg
{
	RENDERSTYLE_COLORARG_CURRENT,
	RENDERSTYLE_COLORARG_DIFFUSE,
	RENDERSTYLE_COLORARG_TEXTURE,
	RENDERSTYLE_COLORARG_TFACTOR
};

enum ERenStyle_AlphaOp
{
	RENDERSTYLE_ALPHAOP_DISABLE,
	RENDERSTYLE_ALPHAOP_SELECTARG1,
	RENDERSTYLE_ALPHAOP_SELECTARG2,
	RENDERSTYLE_ALPHAOP_MODULATE,
	RENDERSTYLE_ALPHAOP_MODULATEALPHA,
	RENDERSTYLE_ALPHAOP_MODULATETFACTOR,
	RENDERSTYLE_ALPHAOP_ADD,
	RENDERSTYLE_ALPHAOP_ADDSIGNED,
	RENDERSTYLE_ALPHAOP_ADDSIGNED2X,
	RENDERSTYLE_ALPHAOP_SUBTRACT,
	RENDERSTYLE_ALPHAOP_MODULATETEXALPHA
};

enum ERenStyle_AlphaArg {
	RENDERSTYLE_ALPHAARG_CURRENT,
	RENDERSTYLE_ALPHAARG_DIFFUSE,
	RENDERSTYLE_ALPHAARG_TEXTURE,
	RENDERSTYLE_ALPHAARG_TFACTOR };

enum ERenStyle_UV_Source
{
	RENDERSTYLE_UVFROM_MODELDATA_UVSET1,
	RENDERSTYLE_UVFROM_MODELDATA_UVSET2,
	RENDERSTYLE_UVFROM_MODELDATA_UVSET3,
	RENDERSTYLE_UVFROM_MODELDATA_UVSET4,
	RENDERSTYLE_UVFROM_CAMERASPACENORMAL,
	RENDERSTYLE_UVFROM_CAMERASPACEPOSITION,
	RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT,
	RENDERSTYLE_UVFROM_WORLDSPACENORMAL,
	RENDERSTYLE_UVFROM_WORLDSPACEPOSITION,
	RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT
};

enum ERenStyle_TextureParam {
	RENDERSTYLE_NOTEXTURE,
	RENDERSTYLE_USE_TEXTURE1,
	RENDERSTYLE_USE_TEXTURE2,
	RENDERSTYLE_USE_TEXTURE3,
	RENDERSTYLE_USE_TEXTURE4 };

enum ERenStyle_UV_Address {
	RENDERSTYLE_UVADDR_WRAP,
	RENDERSTYLE_UVADDR_CLAMP,
	RENDERSTYLE_UVADDR_MIRROR,
	RENDERSTYLE_UVADDR_MIRRORONCE };

enum ERenStyle_TexFilter {
	RENDERSTYLE_TEXFILTER_POINT,
	RENDERSTYLE_TEXFILTER_BILINEAR,
	RENDERSTYLE_TEXFILTER_TRILINEAR,
	RENDERSTYLE_TEXFILTER_ANISOTROPIC,
	RENDERSTYLE_TEXFILTER_POINT_PTMIP };

struct FourFloatVector {					// Four float vector...
	FourFloatVector()		{ }
	FourFloatVector(float nx, float ny, float nz, float nw)	{ x = nx; y = ny; z = nz; w = nw; }
	FourFloatVector(D3DVECTOR v)							{ x = v.x; y = v.y; z = v.z; w = 1.0f; }
	float					x,y,z,w; };

struct FourFloatColor {						// Four float color...
	FourFloatColor()		{ }
	FourFloatColor(float nr, float ng, float nb, float na)	{ r = nr; g = ng; b = nb; a = na; }
	float					r,g,b,a; };

// RENDER PASS TYPES
enum ERenStyle_BlendMode {					//	Cs - Source Color, As - Source Alpha, Cd - Dest Color, Ad - Dest Alpha
	RENDERSTYLE_NOBLEND,					// Cs
	RENDERSTYLE_BLEND_ADD,					// Cs			 + Cd
	RENDERSTYLE_BLEND_SATURATE,				// Cs * (1 - Cd) + Cd
	RENDERSTYLE_BLEND_MOD_SRCALPHA,			// Cs * As		 + Cd * (1 - As)
	RENDERSTYLE_BLEND_MOD_SRCCOLOR,			// Cs * Cs		 + Cd * (1 - Cs)
	RENDERSTYLE_BLEND_MOD_DSTCOLOR,			// Cs * Cd		 + Cd * (1 - Cd)
	RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL,	// Cs * Cs		 + Cd * Cd
	RENDERSTYLE_BLEND_MUL_SRCCOL_ONE,		// Cs * Cs		 + Cd
	RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO,	// Cs * As
	RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE,		// Cs * As		 + Cd
	RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO };	// Cs * Cd

struct TextureStageOps
{
	// Each Render Pass has a set of ops it'll do to apply the texture - this struct defines them.
	ERenStyle_TextureParam			TextureParam;
	ERenStyle_ColorOp				ColorOp;
	ERenStyle_ColorArg				ColorArg1,ColorArg2;
	ERenStyle_AlphaOp				AlphaOp;
	ERenStyle_AlphaArg				AlphaArg1,AlphaArg2;
	ERenStyle_UV_Source				UVSource;
	ERenStyle_UV_Address			UAddress;
	ERenStyle_UV_Address			VAddress;
	ERenStyle_TexFilter				TexFilter;
	bool							UVTransform_Enable;
	float							UVTransform_Matrix[16];
	bool							ProjectTexCoord;
	uint32							TexCoordCount;
};

struct RenderPassOp {						// Each Render Style has one or more render passes. Each pass has a different set of textures and blend modes...
	TextureStageOps					TextureStages[4];
	ERenStyle_BlendMode				BlendMode;
	ERenStyle_ZBufferMode			ZBufferMode;
	ERenStyle_CullMode				CullMode;
	uint32							TextureFactor;
	uint32							AlphaRef;
	bool							DynamicLight;
	ERenStyle_TestMode				ZBufferTestMode;
	ERenStyle_TestMode				AlphaTestMode;
	ERenStyle_FillMode				FillMode;

	bool							bUseBumpEnvMap;			// BumpEnvMap Params...
	uint32							BumpEnvMapStage;
	float							fBumpEnvMap_Scale;
	float							fBumpEnvMap_Offset;
};

struct LightingMaterial {					// Lighting Materials...
	FourFloatColor					Ambient;
	FourFloatColor					Diffuse;
	FourFloatColor					Emissive;
	FourFloatColor					Specular;
	float							SpecularPower; };

struct RSD3DOptions					// Platform options: Direct3D...
{						
	bool							bUseEffectShader;
	int								EffectShaderID;
};

struct RSD3DRenderPass {					// Platform Render Pass options: Direct3D...
	bool							bUseVertexShader;
	string							VertexShaderFilename;
	bool							bExpandForSkinning;
	int32							ConstVector_ConstReg1;	// Should be -1 if not used (same goes for all const regs)...
	FourFloatVector					ConstVector_Param1;
	int32							ConstVector_ConstReg2;
	FourFloatVector					ConstVector_Param2;
	int32							ConstVector_ConstReg3;
	FourFloatVector					ConstVector_Param3;
	int32							WorldViewTransform_ConstReg;
	uint32							WorldViewTransform_Count;
	int32							ProjTransform_ConstReg;
	int32							WorldViewProjTransform_ConstReg;
	int32							ViewProjTransform_ConstReg;
	int32							CamPos_MSpc_ConstReg;
	uint32							Light_Count;
	int32							LightPosition_MSpc_ConstReg;
	int32							LightPosition_CSpc_ConstReg;
	int32							LightColor_ConstReg;
	int32							LightAtt_ConstReg;
	int32							Material_AmbDifEm_ConstReg;
	int32							Material_Specular_ConstReg;
	int32							AmbientLight_ConstReg;
	int32							PrevWorldViewTrans_ConstReg;
	uint32							PrevWorldViewTrans_Count;
	int32							Last_ConstReg;

	bool							bDeclaration_Stream_Position[4];	// Declaration flags...
	bool							bDeclaration_Stream_Normal[4];
	bool							bDeclaration_Stream_UVSets[4];
	bool							bDeclaration_Stream_BasisVectors[4];
	int32							Declaration_Stream_UVCount[4];

	int								VertexShaderID;
	bool							bUsePixelShader;
	int								PixelShaderID;
};

class CRenderStyle {
public:
	CRenderStyle()					{ m_iRefCnt = 0; }
	virtual ~CRenderStyle()			{ }

	// Lighting Material...
	virtual bool					SetLightingMaterial(LightingMaterial& LightMaterial)	= 0;
	virtual bool					GetLightingMaterial(LightingMaterial* pLightMaterial)	= 0;
	// RenderPasses...
	virtual bool					AddRenderPass(RenderPassOp& Renderpass)					= 0;
	virtual bool					RemoveRenderPass(uint32 iPass)							= 0;
	virtual bool					SetRenderPass(uint32 iPass,RenderPassOp& RenderPass)	= 0;
	virtual bool					GetRenderPass(uint32 iPass,RenderPassOp* pRenderPass)	= 0;
	virtual uint32					GetRenderPassCount()									= 0;
	// Platform Options: Direct3D...
	virtual bool					SetDirect3D_Options(RSD3DOptions& Options)				{ return false; }
	virtual bool					GetDirect3D_Options(RSD3DOptions* pOptions)				{ pOptions = NULL; return false; }
	virtual bool					SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)	{ return false; }
	virtual bool					GetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)	{ pD3DRenderPass = NULL; return false; }

	// Helper Functions...
	virtual bool					Compile()												= 0;	// May need to be compiled if it has been changed (will automattically be done if used - but you may choose to do it at a particular time since it may take some time).
	virtual void					SetDefaults()											= 0;
	virtual uint32					GetRefCount()											{ return m_iRefCnt; }

protected:
	uint32							m_iRefCnt;												// Used to figure out if/when we can get rid of this guy...
};

#endif

