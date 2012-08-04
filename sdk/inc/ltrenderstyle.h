// ltrenderstyle.h
//	Defines a render style. The renderer supports setting render styles for render
// objects.

#ifndef __LTRENDERSTYLE_H__
#define __LTRENDERSTYLE_H__

#define MAX_NUM_TEXTURES_PER_PASS			4

// RENDER STATE TYPES
/*!
Enumerates the z-buffering modes for model piece to which the render style
is applied.
\see RenderPassOp structure
*/
enum ERenStyle_ZBufferMode {
	/*!
	Read/write z-buffering enabled.
	*/
	RENDERSTYLE_ZRW,
	/*!
	Read-only z-buffering enabled.
	*/
	RENDERSTYLE_ZRO,
	/*!
	Z-buffering disabled.
	*/
	RENDERSTYLE_NOZ,

	//This must come last
	RENDERSTYLE_ZBUFFERMODE_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_ZBUFFERMODE_TYPE_FORCE_32BIT = 0x7fffffff
};

/*!
Enumerates the modes for alpha testing on model piece to which the render style is
applied. An alpha test compares the vertex alpha value of each pixel in the model
piece to the alpha value of the texture's pixel (texel). If the test is true, then
the pixel is rendered. If the test is false, the pixel does not render.
\see RenderPassOp structure
*/
enum ERenStyle_TestMode {
	/*!
	No test is conducted. All pixels are drawn.
	*/
	RENDERSTYLE_NOALPHATEST,
	/*!
	If the alpha value of the pixel is less than the alpha value of the texel,
	the pixel is drawn.
	*/
	RENDERSTYLE_ALPHATEST_LESS,
	/*!
	If the alpha value of the pixel is less than or equal to the alpha value
	of the texel, the pixel is drawn.
	*/
	RENDERSTYLE_ALPHATEST_LESSEQUAL,
	/*!
	If the alpha value of the pixel is greater than the alpha value of the texel,
	the pixel is drawn.
	*/
	RENDERSTYLE_ALPHATEST_GREATER,
	/*!
	If the alpha value of the pixel is greater than or equal to the alpha value
	of the texel, the pixel is drawn.
	*/
	RENDERSTYLE_ALPHATEST_GREATEREQUAL,
	/*!
	If the alpha value of the pixel is equal to the alpha value of the texel,
	the pixel is drawn.
	*/
	RENDERSTYLE_ALPHATEST_EQUAL,
	/*!
	If the alpha value of the pixel is not equal to the alpha value of the texel,
	the pixel is drawn.
	*/
	RENDERSTYLE_ALPHATEST_NOTEQUAL,

	//This must come last
	RENDERSTYLE_TESTMODE_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_TESTMODE_TYPE_FORCE_32BIT = 0x7fffffff
};

/*!
Enumerates the fill modes for the model piece to which the render style is applied.
\see RenderPassOp structure
*/
enum ERenStyle_FillMode {
    /*!
	Only a wire frame is drawn for the model piece.
	*/
    RENDERSTYLE_WIRE,
    /*!
	The model piece is filled with the appropriate color and texture.
	*/
    RENDERSTYLE_FILL,

	//This must come last
	RENDERSTYLE_FILLMODE_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_FILLMODE_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Enumerates the cull modes for the model piece to which the render style is applied.
\see RenderPassOp structure
*/
enum ERenStyle_CullMode {
    /*!
	No culling is done. The model piece is rendered regardless of winding order.
	*/
    RENDERSTYLE_CULL_NONE,
    /*!
	Model pieces with counter-clockwise winding orders relative to the camera
	are not rendered.
	*/
    RENDERSTYLE_CULL_CCW,
    /*!
	Model pieces with clockwise winding orders relative to the camera are
	not rendered.
	*/
	RENDERSTYLE_CULL_CW,

	//This must come last
	RENDERSTYLE_CULLMODE_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_CULLMODE_TYPE_FORCE_32BIT = 0x7fffffff
};


// TEXTURE STAGE TYPES
/*!
Enumerates the texture-to-color behaviors for model pieces to which the render
style is applied. This state is only useful for textured model pieces.
\see TextureStageOps structure
*/
enum ERenStyle_ColorOp {
	/*!
	The color of the texture is ignored and only the color of the model piece
	is drawn.
	*/
	RENDERSTYLE_COLOROP_DISABLE,
	/*!
	Select the Color Arg1.
	*/
	RENDERSTYLE_COLOROP_SELECTARG1,
	/*!
	Select the Color Arg2.
	*/
	RENDERSTYLE_COLOROP_SELECTARG2,
	/*!
	Each of the pixel colors of the model piece is multiplied with the pixel
	color of the texture.
	*/
	RENDERSTYLE_COLOROP_MODULATE,
	/*!
	Multiply the two arguments, then multiply by two.
	*/
	RENDERSTYLE_COLOROP_MODULATE2X,
	/*!
	Use the current alpha to modulate the two args (Arg1 * Alpha + Arg2 * (1-Alpha))
	*/
	RENDERSTYLE_COLOROP_MODULATEALPHA,
	/*!
	Use the tfactor alpha to modulate the two args (Arg1 * Alpha + Arg2 * (1-Alpha))
	*/
	RENDERSTYLE_COLOROP_MODULATETFACTOR,
	/*!
	Each of the pixel colors of the model piece is added to the pixel color of
	the texture. This value is not supported on the PlayStation 2 platform.
	*/
	RENDERSTYLE_COLOROP_ADD,
	/*!
	DotProduct3 the two args.
	*/
	RENDERSTYLE_COLOROP_DOTPRODUCT3,
	/*!
	Bump Environment Map.
	*/
	RENDERSTYLE_COLOROP_BUMPENVMAP,
	/*!
	Bump Environment Map with lumination.
	*/
	RENDERSTYLE_COLOROP_BUMPENVMAPLUM,
	/*!
	Color1 + Color2 - 0.5
	*/
	RENDERSTYLE_COLOROP_ADDSIGNED,
	/*!
	(Color1 + Color2 - 0.5) * 2
	*/
	RENDERSTYLE_COLOROP_ADDSIGNED2X,
	/*!
	Color1 - Color2
	*/
	RENDERSTYLE_COLOROP_SUBTRACT,
	/*!
	Color1 + Alpha1 * Color2
	*/
	RENDERSTYLE_COLOROP_ADDMODALPHA,
	/*!
	Color1 + (1 - Alpha1) * Color2
	*/
	RENDERSTYLE_COLOROP_ADDMODINVALPHA,
	/*!
	Use the current texture alpha to modulate the two args (Arg1 * Alpha + Arg2 * (1-Alpha))
	*/
	RENDERSTYLE_COLOROP_MODULATETEXALPHA,

	//This must come last
	RENDERSTYLE_COLOROP_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_COLOROP_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Color Argument types.
*/
enum ERenStyle_ColorArg {
	/*!
	Result of previous stage.
	*/
	RENDERSTYLE_COLORARG_CURRENT,
	/*!
	Diffuse component.
	*/
	RENDERSTYLE_COLORARG_DIFFUSE,
	/*!
	Texture component.
	*/
	RENDERSTYLE_COLORARG_TEXTURE,
	/*!
	Use TFactor color.
	*/
	RENDERSTYLE_COLORARG_TFACTOR,

	//This must come last
	RENDERSTYLE_COLORARG_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_COLORARG_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Alpha Argument types.
*/
enum ERenStyle_AlphaOp {
	/*!
	Disable this operand (output is zero).
	*/
	RENDERSTYLE_ALPHAOP_DISABLE,
	/*!
	Select Alpha Arg1.
	*/
	RENDERSTYLE_ALPHAOP_SELECTARG1,
	/*!
	Select Alpha Arg2.
	*/
	RENDERSTYLE_ALPHAOP_SELECTARG2,
	/*!
	Out = AlphaArg1 * AlphaArg2
	*/
	RENDERSTYLE_ALPHAOP_MODULATE,
	/*!
	Use current alpha to modulate. Out = AlphaArg1 * Alpha + AlphaArg2 * (1 - Alpha)
	*/
	RENDERSTYLE_ALPHAOP_MODULATEALPHA,
	/*!
	Use TFactor alpha to modulate. Out = AlphaArg1 * Alpha + AlphaArg2 * (1 - Alpha)
	*/
	RENDERSTYLE_ALPHAOP_MODULATETFACTOR,
	/*!
	Out = AlphaArg1 + AlphaArg2
	*/
	RENDERSTYLE_ALPHAOP_ADD,
	/*!
	Out = AlphaArg1 + AlphaArg2 - 0.5
	*/
	RENDERSTYLE_ALPHAOP_ADDSIGNED,
	/*!
	Out = (AlphaArg1 + AlphaArg2 - 0.5) * 2
	*/
	RENDERSTYLE_ALPHAOP_ADDSIGNED2X,
	/*!
	Out = AlphaArg1 - AlphaArg2
	*/
	RENDERSTYLE_ALPHAOP_SUBTRACT,
	/*!
	Use current texture alpha to modulate. Out = AlphaArg1 * Alpha + AlphaArg2 * (1 - Alpha)
	*/
	RENDERSTYLE_ALPHAOP_MODULATETEXALPHA,

	//This must come last
	RENDERSTYLE_ALPHAOP_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_ALPHAOP_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Alpha Arguments
*/
enum ERenStyle_AlphaArg {
	/*!
	Use alpha from previous stage.
	*/
	RENDERSTYLE_ALPHAARG_CURRENT,
	/*!
	Use the diffuse alpha.
	*/
	RENDERSTYLE_ALPHAARG_DIFFUSE,
	/*!
	Use the texture alpha.
	*/
	RENDERSTYLE_ALPHAARG_TEXTURE,
	/*!
	Use the TFactor alpha.
	*/
	RENDERSTYLE_ALPHAARG_TFACTOR,

	//This must come last
	RENDERSTYLE_ALPHAARG_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_ALPHAARG_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Source for UV co-ords.
*/
enum ERenStyle_UV_Source {
	/*!
	ModelData UV Set 1.
	*/
	RENDERSTYLE_UVFROM_MODELDATA_UVSET1,
	/*!
	ModelData UV Set 2.
	*/
	RENDERSTYLE_UVFROM_MODELDATA_UVSET2,
	/*!
	ModelData UV Set 3.
	*/
	RENDERSTYLE_UVFROM_MODELDATA_UVSET3,
	/*!
	ModelData UV Set 4.
	*/
	RENDERSTYLE_UVFROM_MODELDATA_UVSET4,
	/*!
	The camera space normal.
	*/
	RENDERSTYLE_UVFROM_CAMERASPACENORMAL,
	/*!
	The camera space position.
	*/
	RENDERSTYLE_UVFROM_CAMERASPACEPOSITION,
	/*!
	The camera space reflection vector.
	*/
	RENDERSTYLE_UVFROM_CAMERASPACEREFLTVECT,
	/*!
	The world space normal vector.
	*/
	RENDERSTYLE_UVFROM_WORLDSPACENORMAL,
	/*!
	The world space position.
	*/
	RENDERSTYLE_UVFROM_WORLDSPACEPOSITION,
	/*!
	The world space reflection vector.
	*/
	RENDERSTYLE_UVFROM_WORLDSPACEREFLTVECT ,

	//This must come last
	RENDERSTYLE_UVSOURCE_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_UVSOURCE_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Set which texture to use.
*/
enum ERenStyle_TextureParam {
	/*!
	No Texture.
	*/
	RENDERSTYLE_NOTEXTURE,
	/*!
	Use the first texture in the list.
	*/
	RENDERSTYLE_USE_TEXTURE1,
	/*!
	Use the second texture in the list.
	*/
	RENDERSTYLE_USE_TEXTURE2,
	/*!
	Use the third texture in the list.
	*/
	RENDERSTYLE_USE_TEXTURE3,
	/*!
	Use the fourth texture in the list.
	*/
	RENDERSTYLE_USE_TEXTURE4,

	//This must come last
	RENDERSTYLE_TEXTUREPARAM_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_TEXTUREPARAM_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Selects the UV address mode.
*/
enum ERenStyle_UV_Address {
	/*!
	Wrap (Repeat) UV address.
	*/
	RENDERSTYLE_UVADDR_WRAP,
	/*!
	Clamp to edge of texture.
	*/
	RENDERSTYLE_UVADDR_CLAMP,
	/*!
	Mirror on wrap.
	*/
	RENDERSTYLE_UVADDR_MIRROR,
	/*!
	Mirror once on wrap.
	*/
	RENDERSTYLE_UVADDR_MIRRORONCE,

	//This must come last
	RENDERSTYLE_UVADDRESS_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_UVADDRESS_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
Texture filter mode.
*/
enum ERenStyle_TexFilter {
	/*!
	BiLinear filter (TexFilter: Point, MipFilter: None).
	*/
	RENDERSTYLE_TEXFILTER_POINT,
	/*!
	BiLinear filter (TexFilter: Linear, MipFilter: Point).
	*/
	RENDERSTYLE_TEXFILTER_BILINEAR,
	/*!
	BiLinear filter (TexFilter: Linear, MipFilter: Linear).
	*/
	RENDERSTYLE_TEXFILTER_TRILINEAR,
	/*!
	BiLinear filter (TexFilter: Anisotropic, MipFilter: Linear).
	*/
	RENDERSTYLE_TEXFILTER_ANISOTROPIC,
	/*!
	BiLinear filter (TexFilter: Point, MipFilter: Point).
	*/
	RENDERSTYLE_TEXFILTER_POINT_PTMIP,

	//This must come last
	RENDERSTYLE_TEXFILTER_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_TEXFILTER_TYPE_FORCE_32BIT = 0x7fffffff
};


// RENDER PASS TYPES
/*!
Enumerates the alpha blend modes with which the model piece is drawn. The blend mode
defines how the source color (Cs), source alpha value (As), destination color (Cd),
and destination alpha value (Ad) interact. PlayStation 2 (which does not support
full collor modulate) supports the \b RENDERSTYLE_BLEND_ADD,
\b RENDERSTYLE_BLEND_MOD_SRCALPHA, and \b RENDERSTYLE_NOBLEND values.
\see RenderPassOp structure
*/
enum ERenStyle_BlendMode {
	/*!
	No blending is done. The source color is drawn with no modification.
	This is the default value.
	*/
	RENDERSTYLE_NOBLEND,					// Cs
	/*!
	The source color is added to the destination color (Cs + Cd).
	*/
	RENDERSTYLE_BLEND_ADD,
	/*!
	Cs * (1 - Cd) + Cd
	*/
	RENDERSTYLE_BLEND_SATURATE,
	/*!
	Cs * As	+ Cd * (1 - As)
	*/
	RENDERSTYLE_BLEND_MOD_SRCALPHA,
	/*!
	Cs * Cs + Cd * (1 - Cs)
	*/
	RENDERSTYLE_BLEND_MOD_SRCCOLOR,
	/*!
	Cs * Cd	+ Cd * (1 - Cd)
	*/
	RENDERSTYLE_BLEND_MOD_DSTCOLOR,
	/*!
	Cs * Cs	+ Cd * Cd
	*/
	RENDERSTYLE_BLEND_MUL_SRCCOL_DSTCOL,
	/*!
	Cs * Cs	+ Cd
	*/
	RENDERSTYLE_BLEND_MUL_SRCCOL_ONE,
	/*!
	CS * As
	*/
	RENDERSTYLE_BLEND_MUL_SRCALPHA_ZERO,
    /*!
    Cs * As + Cd
    */
	RENDERSTYLE_BLEND_MUL_SRCALPHA_ONE,
	/*!
	Cs * Cd
	*/
	RENDERSTYLE_BLEND_MUL_DSTCOL_ZERO,

	//This must come last
	RENDERSTYLE_BLENDMODE_TYPE_COUNT,

	/*!
	Force this enumerated type to use 32 bit value.
	*/
	RENDERSTYLE_BLENDMODE_TYPE_FORCE_32BIT = 0x7fffffff
};


/*!
FourFloatVector
*/
struct FourFloatVector {					// Four float vector...
	FourFloatVector()		{ }
	FourFloatVector(float nx, float ny, float nz, float nw)	{ x = nx; y = ny; z = nz; w = nw; };
	float					x,y,z,w;
};

/*!
FourFloatColor
*/
struct FourFloatColor {						// Four float color...
	FourFloatColor()		{ }
	FourFloatColor(float nr, float ng, float nb, float na)	{ r = nr; g = ng; b = nb; a = na; }
	float					r,g,b,a;
};


/*!
Contains the set of operations that the render pass applies to the texture of
a model piece.
\see RenderPassOp structure
*/
struct TextureStageOps {
	/*!
	The \b ERenStyle_TextureParam value for this \TextureStageOps.
	*/
	ERenStyle_TextureParam			TextureParam;
	/*!
	The \b ERenStyle_ColorOp value for this \TextureStageOps.
	*/
	ERenStyle_ColorOp				ColorOp;
	/*!
	The \b ERenStyle_ColorArg value for this \TextureStageOps.
	*/
	ERenStyle_ColorArg				ColorArg1,ColorArg2;
	/*!
	The \b ERenStyle_AlphaOp value for this \TextureStageOps.
	*/
	ERenStyle_AlphaOp				AlphaOp;
	/*!
	The \b ERenStyle_AlphaArg value for this \TextureStageOps.
	*/
	ERenStyle_AlphaArg				AlphaArg1,AlphaArg2;
	/*!
	The \b ERenStyle_UV_Source value for this \TextureStageOps.
	*/
	ERenStyle_UV_Source				UVSource;
	/*!
	The \b ERenStyle_UV_Address value for this \TextureStageOps.
	*/
	ERenStyle_UV_Address			UAddress;
	/*!
	The \b ERenStyle_UV_Address value for this \TextureStageOps.
	*/
	ERenStyle_UV_Address			VAddress;
	/*!
	The \b ERenStyle_TexFilter value for this \TextureStageOps.
	*/
	ERenStyle_TexFilter				TexFilter;
	/*!
	Enable matrix transform on UV Co-ords.
	*/
	bool							UVTransform_Enable;
	/*!
	The matrix transform.
	*/
	float							UVTransform_Matrix[16];
	/*!
	Enable projection of uv co-ords (divide by last UV element).
	*/
	bool							ProjectTexCoord;
	/*!
	The number of texture coordinates that will be output by the texture transform matrix
	*/
	uint32							TexCoordCount;

};

/*!
Contains the set of operations that are applied to the model piece for a given
render pass. Each Render Style has one or more render passes.
\see AddRenderPass(), SetRenderPass(), GetRenderPass()								= 0;
*/
struct RenderPassOp
{
	/*!
	An array of \b TextureStageOps for this \b RenderPassOp.
	*/
	TextureStageOps					TextureStages[4];
	/*!
	The \b ERenStyle_BlendMode value for this \b RenderPassOp.
	*/
	ERenStyle_BlendMode				BlendMode;
	/*!
	The \b ERenStyle_ZBufferMode value for this \b RenderPassOp.
	*/
	ERenStyle_ZBufferMode			ZBufferMode;
	/*!
	The \b ERenStyle_CullMode value for this \b RenderPassOp.
	*/
	ERenStyle_CullMode				CullMode;
	/*!
	ARGB value that can be used in the texture stages.
	*/
	uint32							TextureFactor;
	/*!
	Alpha Ref values (for alphatesting)...
	*/
	uint32							AlphaRef;
	/*!
	Enable dynamic lighting...
	*/
	bool							DynamicLight;
	/*!
	The Z comparison function that will be used for this render pass
	*/
	ERenStyle_TestMode				ZBufferTestMode;
	/*!
	The \b ERenStyle_TestMode value for this \b RenderPassOp.
	*/
	ERenStyle_TestMode				AlphaTestMode;
	/*!
	The \b ERenStyle_FillMode value for this \b RenderPassOp.
	*/
	ERenStyle_FillMode				FillMode;
	/*!
	Enable BumpEnvMap
	*/
	bool							bUseBumpEnvMap;			// BumpEnvMap Params...
	/*!
	Set the BumpEnvMap Stage...
	*/
	uint32							BumpEnvMapStage;
	/*!
	BumpEnvMap scale.
	*/
	float							fBumpEnvMap_Scale;
	/*!
	BumpEnvMap offset.
	*/
	float							fBumpEnvMap_Offset;
};

/*!
Light Material values (attenuation values for dynamic lighting).
*/
struct LightingMaterial {					// Lighting Materials...
	/*!
	Ambient attenuation.
	*/
	FourFloatColor					Ambient;
	/*!
	Diffuse attenuation.
	*/
	FourFloatColor					Diffuse;
	/*!
	Emissive attenuation.
	*/
	FourFloatColor					Emissive;
	/*!
	Specular attenuation.
	*/
	FourFloatColor					Specular;
	/*!
	Specular Power.
	*/
	float							SpecularPower;
};

/*!
*/
struct RSD3DOptions					// Platform options: Direct3D...
{						
	bool							bUseEffectShader;
	int								EffectShaderID;
};

/*!
Contains render pass options for Direct3d platforms.
\see SetRenderPass_D3DOptions(), GetRenderPass_D3DOptions()
*/
struct RSD3DRenderPass
{
//! indicates whether or not to use a vertex shader
	bool							bUseVertexShader;
//! Id of a loaded vertex shader
	int								VertexShaderID;

//! indicates whether or not to use a vertex shader
	bool							bUsePixelShader;
//! Id of a loaded pixel shader
	int								PixelShaderID;
};

/*!
Manages settings for a render style.
*/
class CRenderStyle {
public:
	CRenderStyle()					{ m_iRefCnt = 0; m_pFilename = NULL ; }
	virtual ~CRenderStyle()			{ if(m_pFilename) delete [] m_pFilename ; }

	// Lighting Material...
/*!
\param LightMaterial The address of the \b LightingMaterial structure for this render style.
\return A Boolean value indicating whether or not the function is successful.

Sets the lighting material for this render style.
*/
	virtual bool					SetLightingMaterial(LightingMaterial& LightMaterial)	= 0;

/*!
\param pLightMaterial [Return parameter] A pointer to the \LightingMaterial structure set for this
render style.
\return A Boolean value indicating whether or not the function is successful.

Retrieves the lighting material set for this render style.
*/
	virtual bool					GetLightingMaterial(LightingMaterial* pLightMaterial)	= 0;

	// RenderPasses...
/*!
\param Renderpass The address of the \b RenderPassOp structure to add to this
render style.
\return A Boolean value indicating whether or not the function is successful.

Adds a \b RenderPassOp structure to this render style. A \b CRenderStyle instance
may have any number of \b RenderPassOp structures applied to it. The
\b CRenderStyle instance keeps a 0-based list of these structures.

\see CRenderStyle::RemoveRenderPass()
*/
	virtual bool					AddRenderPass(RenderPassOp& Renderpass)					= 0;

/*!
\param iPass An index into the \b CRenderStyle instance's list of \b RenderPassOp
structures.
\return A Boolean value indicating whether or not the function is successful.

Removes a \b RenderPassOp structure from this render style.

\see CRenderStyle::AddRenderPass()
*/
	virtual bool					RemoveRenderPass(uint32 iPass)							= 0;

/*!
\param iPass An index into the \b CRenderStyle instance's list of \b RenderPassOp
structures.
\param RenderPass The address of the \b RenderPassOp structure.
\return A Boolean value indicating whether or not the function is successful.

Sets a \RenderPassOp structure for this render style.

\see CRenderStyle::GetRenderPass()
*/
	virtual bool					SetRenderPass(uint32 iPass,RenderPassOp& RenderPass)	= 0;

/*!
\param iPass An index into the \b CRenderStyle instance's list of \b RenderPassOp
structures.
\param pRenderPass [Return parameter] A pointer to the \b RenderPassOp structure.

Retrieves one of the \b RenderPassOp structures set for this render style.

\see CRenderStyle::SetRenderPass()
*/
	virtual bool					GetRenderPass(uint32 iPass,RenderPassOp* pRenderPass)	= 0;

/*!
\return A uint32 value identifying how many \b RenderPassOp structures apply to this
render style.

Returns the count of \b RenderPassOp structures applied to this render style.
*/
	virtual uint32					GetRenderPassCount()									= 0;

	// Platform Options: Direct3D...

/*!
\param Options The address of the \b RSD3DOptions structure to apply to this
render style.
\return A Boolean value indicating whether or not the function is successful.

Sets Direct3D options for this render style.

\see CRenderStyle::GetDirect3D_Options()
*/
	virtual bool					SetDirect3D_Options(RSD3DOptions& Options)				{ return false; }

/*!
\param pOptions [Return parameter] A pointer to the \b RSD3DOptions structure
set for this render style.
\return A Boolean value indicating whether or not the function is successful.

Retrieves the Direct3D options set for this render style.

\see CRenderStyle::SetDirect3D_Options()
*/
	virtual bool					GetDirect3D_Options(RSD3DOptions* pOptions)				{ pOptions = NULL; return false; }

/*!
\param iPass An index into the \b CRenderStyle instance's list of \b RSD3DRenderPassOp
structures.
\param pD3DRenderPass The address of the \b RSD3DRenderPass structure to apply
to this render style.
\return A Boolean value indicating whether or not the function is successful.

Sets a \b RSD3DRenderPass structure to this render style.

\see CRenderStyle::GetRenderPass_D3DOptions()
*/
	virtual bool					SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)	{ return false; }

/*!
\param iPass An index into the \b CRenderStyle instance's list of \b RSD3DRenderPassOp
structures.
\param pD3DRenderPass [Return parameter] A pointer to one of the \b RSD3DRenderPass
structures applied to this render style.
\return A Boolean value indicating whether or not the function is successful.

Retrieves one of the \b RSD3DRenderPass structures applied to this render style.

\see CRenderStyle::GetRenderPass_D3DOptions()
*/
	virtual bool					GetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass)	{ pD3DRenderPass = NULL; return false; }

	// Helper Functions...

/*!
\param pFileStream The LTB render style file.
\return A Boolean value indicating whether or not the function is successful.

Loads a compiled LTB render style file.
*/
	virtual bool					Load_LTBData(ILTStream* pFileStream)					= 0;	// Stream in the ltb file...

	const char *					GetFilename() const { return m_pFilename; }

/*!
\return A Boolean value indicating whether or not the function is successful.
*/
	virtual bool					Compile()												= 0;	// May need to be compiled if it has been changed (will automattically be done if used - but you may choose to do it at a particular time since it may take some time).

/*!
Set to default.
*/
	virtual void					SetDefaults()											= 0;

/*!
\param pSrcRenderStyle
\return A Boolean value indicating whether or not the function is successful.
*/
	virtual bool					CopyRenderStyle(CRenderStyle* pSrcRenderStyle)			= 0;	// Copy the render style data from a source RS and put it in this one...

/*!
Check for support on the current platform.
*/
	virtual bool					IsSupportedOnDevice()									= 0;	// Does the current device support this render style...

/*!
\return ref count.

Get the reference count.
*/
	virtual uint32					GetRefCount()											{ return m_iRefCnt; }

	// Note: Use these functions with care. DecRefCount will not free a render style (use the render style interface to do that)...
	virtual uint32					IncRefCount()											{ ++m_iRefCnt; return m_iRefCnt; }
	virtual uint32					DecRefCount()											{ assert(m_iRefCnt > 0); --m_iRefCnt; return m_iRefCnt; }

	void							SetFilename( const char *new_name )						{ delete [] m_pFilename ; LT_MEM_TRACK_ALLOC(m_pFilename = new char [ strlen( new_name ) + 1], LT_MEM_TYPE_RENDERER); strcpy( m_pFilename, new_name ); }
protected:
	// Used to figure out if/when we can get rid of this guy...
	uint32							m_iRefCnt;

private:
	char*							m_pFilename ;

};

#endif

