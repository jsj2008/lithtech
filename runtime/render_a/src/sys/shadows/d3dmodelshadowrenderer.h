#ifndef __D3D_MODELSHADOWRENDERER_H_
#define __D3D_MODELSHADOWRENDERER_H_

#include "imodelshadowrenderer.h"

//forward declarations
class ShadowLightInfo;
class IShadowTexture;
class ModelInstance;


//the maximum number of shadows that can be projected from any single model
#define MAX_SHADOWS		16


//---------------------------------
//functions for handling the global shadow textures

//sets all shadow textures as dirty so they will have to be regenerated on the next
//get
void			InvalidateShadowTextures();

//gets the shadow for shadow N that is of the appropriate size
IShadowTexture* GetShadowTexture( uint32 nSizeX, uint32 nSizeY, uint32 nShadow );

//! D3DModelShadowRenderer

class D3DModelShadowRenderer : public IModelShadowRenderer
{
public:
	D3DModelShadowRenderer();
	virtual ~D3DModelShadowRenderer();

	virtual bool Init();
	virtual bool Term();
	virtual bool RenderModelShadowTextures(ModelDraw* md);
	virtual bool RenderModelShadowsOnWorld(ModelDraw* md);

protected:

	//finds the bounding sphere for the associated model and its attachments
	void	FindBoundingInfo(ModelInstance* pModel, const LTVector& vLightPos, LTVector& vFinalDir, LTVector& vFinalPos, float& fFinalRadius);

	void	DrawModelShadows(ModelDraw* md );
	void	ClampMaxShadows(uint32& nMaxShadows);
	void	GatherRelevantLightSources(ModelDraw* md, uint32 nMaxShadows, uint32& nShadows);
	void	ComputeShadowTexSize(uint32& iShadowTexSize);
	void	BuildLightRefFrame(LTVector& lightDir, LTVector& lightUp, LTVector& lightRight);
	void	BuildShadowLightInfo(ShadowLightInfo& info, float fShadowRadius, float fMaxShadowDist, bool bPerspective);
	void	RenderModelShadow(	ModelDraw* md, const LTVector& lightPos, const LTVector& lightDir, 
								const LTVector& lightUp, const LTVector& lightColor, 
								float fAlpha, float fShadTexSize,
								float fProjSizeX, float fProjSizeY,
								bool bPerspective);
};

#endif // __D3D_MODELSHADOWRENDERER_H_
