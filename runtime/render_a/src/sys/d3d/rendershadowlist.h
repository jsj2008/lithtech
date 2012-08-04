#ifndef __RENDERSHADOWLIST_H__
#define __RENDERSHADOWLIST_H__

class ModelInstance;
class CRelevantLightList;
class D3DShadowTexture;
class ViewParams;

class CRenderShadowList
{
public:

	//singleton access
	static CRenderShadowList& GetSingleton();

	//given a model instance and it's light list, this will add all appropriate shadows to the list
	//of shadows to be rendered
	void	QueueShadows(ModelInstance* pInstance, const CRelevantLightList& LightList);

	//this will render all shadows in the queue to the world and flush it
	void	RenderQueuedShadows(const ViewParams& Params);

	//this will empty all shadows out of the list
	void	FlushShadows();

	//call this to free all memory associated with the shadow list. This will require it to grow
	//again, but is a good chance to clean up after some level that used a lot of models
	//and caused it to grow too large
	void	FreeListMemory();

	//forces all the associated shadow textures to be invalidated
	void	InvalidateShadowTextures();

private:

	//frees all shadow textures
	void	FreeShadowTextures();

	//updates the texture list to reflect any changed console variables
	void	UpdateTextureList();

	//after the textures have been rendered into the appropriate
	//slots in the texture list, this function will render those textures on the world
	//using the data
	void	RenderTexturesOnWorld(const ViewParams& Params, uint32 nStart, uint32 nEnd, bool bBlurred);

	//renders the specified shadow into the specified texture
	bool	RenderShadowTexture(uint32 nShadow, uint32 nTexture);

	//blurs the specified texture to the other texture, assumes that the blur shader is valid
	bool	BlurShadowTexture(uint32 nSrcTex, uint32 nDestTex);

	//handles inserting a shadow into the list
	void	QueueShadow(ModelInstance* pInstance, const LTVector& vModelPos, const LTVector& vPos, const LTVector& vColor, bool bOrtho);

	//we don't want any instances other than the singleton
	CRenderShadowList();
	CRenderShadowList(const CRenderShadowList&);

	struct SQueuedShadow
	{
		ModelInstance*	m_pInstance;
		LTVector		m_vModelPos;
		LTVector		m_vColor;
		LTVector		m_vLightPos;
		bool			m_bOrtho;

		//parameters for sorting based upon the score
		bool operator<(const SQueuedShadow& rhs) const
		{
			return m_fScore > rhs.m_fScore;
		}

		float			m_fScore;
	};

	//calculates the score for a queued shadow
	float	CalcScore(const ViewParams& Params, const SQueuedShadow& rhs);

	typedef vector<SQueuedShadow>		TShadowList;
	TShadowList							m_cShadowList;

	//the list of shadow textures we have
	typedef vector<D3DShadowTexture*>	TTextureList;
	TTextureList						m_cTextureList;

	//the size of the textures in our list
	uint32								m_nTextureRes;

	//flag indicating if we failed to load this pixel shader prior
	static bool							s_bFailedPSHandle;

};

#endif
