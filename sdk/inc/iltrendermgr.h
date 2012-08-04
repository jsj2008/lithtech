#ifndef __ILTRENDERMGR_H__
#define __ILTRENDERMGR_H__

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

enum ERenderTargetFormat
{
	RTFMT_A8R8G8B8 = 0,
	RTFMT_X8R8G8B8,
	RTFMT_R16F,
	RTFMT_R32F,
};

enum EStencilBufferFormat
{
	STFMT_UNKNOWN = 0,
	STFMT_D24S8,
	STFMT_D24X8,
	STFMT_D24X4S4,
	STFMT_D32,
	STFMT_D15S1,
	STFMT_D16,
};

/*!  
The ILTRenderMgr interface.  

Define a holder to get this interface like this:
\code
define_holder(ILTRenderMgr, your_var);
\endcode  
*/

class ILTRenderMgr : public IBase {
public:
    interface_version(ILTRenderMgr, 0);	

	virtual void Init() = 0;
	virtual void Term() = 0;

	/*!
	\param  pFileName	Filename of the Effect file.
	\param  EffectShaderID	Effect Shader ID to create.
	\param  pVertexElements	\b D3DVERTEXELEMENT9 array pointer.
	\param  VertexElementsSize	Size in bytes, of the \b D3DVERTEXELEMENT9 array.
	\param  EffectPoolID ID handle of the HEFFECTPOOL for this effect. If none is desired, pass in NO_EFFECT_POOL.
	\return \b LT_INVALIDPARAMS if the filename pointer is invalid, returns \b LT_NOTFOUND if the file is not found, returns \b LT_ERROR for any other problem otherwise, returns \b LT_OK.

	Create a new EffectShader

	*/
	virtual LTRESULT AddEffectShader (const char *pFileName, 
										int EffectShaderID, 
										const uint32 *pVertexElements, 
										uint32 VertexElementsSize, 
										HEFFECTPOOL EffectPoolID) = 0;

	/*!
	\param  EffectShaderID	Effect Shader ID to find.
	\return Valid \b LTEffectShader pointer if the EffectShader is found, otherwise returns \b NULL.

	Find an EffectShader in the database.

	*/
	virtual LTEffectShader* GetEffectShader(int EffectShaderID) = 0;

	/*!
	\param  EffectPoolID	EffectPool ID to create.
	\return \b LT_OK if successful, LT_ERROR on failure.

	Create an EffectPool.

	*/
	virtual LTRESULT CreateEffectPool (HEFFECTPOOL EffectPoolID) = 0;

	/*!
	\param  nWidth Width of the render target. (in pixels)
	\param  nHeight Height of the render target. (in pixels)
	\param  eRenderTargetFormat Format of the render target.
	\param  eStencilBufferFormat Format of the depth/stencil buffer. Pass in STFMT_UNKNOWN to use the default depth stencil buffer format.
	\param  hRenderTarget ID of the render target.
	\return \b LT_OK if successful, LT_ERROR on failure.

	Create a new RenderTarget.

	*/
	virtual LTRESULT CreateRenderTarget(uint32 nWidth, uint32 nHeight, ERenderTargetFormat eRenderTargetFormat, EStencilBufferFormat eStencilBufferFormat, HRENDERTARGET hRenderTarget) = 0;

	/*!
	\param  hRenderTarget	Render target handle.
	\return \b LT_OK if successful, LT_ERROR on failure.

	Set the current render target.

	*/
	virtual LTRESULT InstallRenderTarget(HRENDERTARGET hRenderTarget) = 0;

	/*!
	\param  hRenderTarget	Render target handle.
	\return \b LT_OK if successful, LT_ERROR on failure.

	Delete this render target.

	*/
	virtual LTRESULT RemoveRenderTarget(HRENDERTARGET hRenderTarget) = 0;

	/*!
	\param  hRenderTarget	Render target handle.
	\return \b LT_OK if successful, LT_ERROR on failure.

	Use StretchRect() to copy the render target surface to the back buffer. 

	*/
	virtual LTRESULT StretchRectRenderTargetToBackBuffer(HRENDERTARGET hRenderTarget) = 0;

	/*!
	\param  hRenderTarget	Render target handle.
	\param  nWidth Holds the result for the width of the render target.
	\param  nHeight Holds the result for the height of the render target.
	\return \b LT_OK if successful, LT_ERROR on failure.

	Get the dimensions (in pixels) of the render target surface.

	*/
	virtual LTRESULT GetRenderTargetDims(HRENDERTARGET hRenderTarget, uint32& nWidth, uint32 nHeight) = 0;

	/*!
	\return \b LT_OK if successful, LT_ERROR on failure.
	
	Store the current render target. This should be used at the beginning of a render frame before other render targets are applied.

	*/
	virtual LTRESULT StoreDefaultRenderTarget() = 0;

	/*!
	\return \b LT_OK if successful, LT_ERROR on failure.

	Restore the default render target. This should be done before ILTClient::End3d() is called. This will restore the back buffer.

	*/
	virtual LTRESULT RestoreDefaultRenderTarget() = 0;

	/*!
	\return \b LT_OK if successful, LT_ERROR on failure.

	Take the current contents of the back buffer and save them to a texture.  This can be used for effects such a refraction.

	*/
	virtual LTRESULT SnapshotCurrentFrame() = 0;

	/*!
	\return \b LT_OK if successful, LT_ERROR on failure.

	Take the current snapshot and save it for the next frame to use. This \b MUST be done after SnapshotCurrentFrame() is called.

	*/
	virtual LTRESULT SaveCurrentFrameToPrevious() = 0;

	/*!
	\param  pEffect	LTEffectShader pointer.
	\param  szParam Parameter name to use to pass into the EffectShader.
	\return \b LT_OK if successful, LT_ERROR on failure.

    This will call SetTexture on the ID3DXEffect using szParam as the constant name.  This is usually done right after a call to SnapshotCurrentFrame().

	*/
	virtual LTRESULT UploadCurrentFrameToEffect(LTEffectShader* pEffect, const char* szParam) = 0;

	/*!
	\param  pEffect	LTEffectShader pointer.
	\param  szParam Parameter name to use to pass into the EffectShader.
	\return \b LT_OK if successful, LT_ERROR on failure.

	 This will call SetTexture on the ID3DXEffect using szParam as the constant name.  The previous frame \b MUST be saved on a previous frame or the function will fail.

	*/
	virtual LTRESULT UploadPreviousFrameToEffect(LTEffectShader* pEffect, const char* szParam) = 0;

};


#endif  //! __ILTRENDERMGR_H__

