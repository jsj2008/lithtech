#ifndef __RENDERMODELPIECELIST_H__
#define __RENDERMODELPIECELIST_H__

class CRelevantLightList;
class CDeviceLightList;
class CRenderStyleMap;

class CRenderModelPieceList
{
public:

	//adds a piece to be rendered
	void QueuePiece(ModelInstance* pInstance, ModelPiece* pPiece, CDIModelDrawable* pLOD, DDMatrix* pTransforms, CDeviceLightList* pLightList, bool bTexture, const ModelHookData* pHookData);

	//Renders all the queued pieces and flushes the list. This takes an alpha
	//that can be applied to all pieces in the list
	void RenderPieceList(float fAlpha);

	//call this to free all memory associated with the piece list. This will require it to grow
	//again, but is a good chance to clean up after some level that used a lot of models
	//and caused it to grow too large
	void FreeListMemory();

	//singleton access
	static CRenderModelPieceList& GetSingleton();

	//remaps all the render styles using the specified map
	void RemapRenderStyles(const CRenderStyleMap& Map);

private:

	//we don't want any instances other than the singleton
	CRenderModelPieceList();
	CRenderModelPieceList(const CRenderModelPieceList&);

	struct SQueuedPiece
	{
		//standard STL constructors and operators
		SQueuedPiece();
		SQueuedPiece(const SQueuedPiece& rhs);
		~SQueuedPiece();

		SQueuedPiece& operator=(const SQueuedPiece& rhs);
		bool operator==(const SQueuedPiece& rhs) const;
		bool operator<(const SQueuedPiece& rhs) const;

		//the list of textures for the piece
		SharedTexture*			m_TextureList[MAX_PIECE_TEXTURES];

		//the render style associated with the piece
		CD3DRenderStyle*		m_pRenderStyle;

		//whether or not this is a really close piece
		bool					m_bReallyClose;

		//the priority of this piece
		uint8					m_nRenderPriority;

		//the lights effecting this piece
		CDeviceLightList*		m_pLightList;

		//the actual piece that is to be rendered
		CDIModelDrawable*		m_pRenderPiece;

		//the model instance that this piece belongs to
		ModelInstance*			m_pInstance;

		//the transforms for this model
		DDMatrix*				m_pTransforms;

		//the model hook data associated with the model instance
		const ModelHookData*	m_pHookData;
	};

	typedef vector<SQueuedPiece>		TPieceList;
	TPieceList							m_cPieceList;
};

#endif

