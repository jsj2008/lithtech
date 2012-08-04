//-----------------------------------------------------------------------------
// PolyTrailFX.h 
//
// A collection of samples taken from moving geometry that is then rendered in
// a path like manner in order to create a visualization of movement, such as
// blur created by swinging a sword, or a trail following debris.
//
//-----------------------------------------------------------------------------

#ifndef __POLYTRAILFX_H__
#define __POLYTRAILFX_H__

#ifndef __BASEFX_H__
#	include "BaseFx.h"
#endif

#ifndef __POLYTRAILPROPS_H__
#	include "PolyTrailProps.h"
#endif

#ifndef __ILTCUSTOMRENDER_H__
#	include "iltcustomrender.h"
#endif

//forward declarations
class CMemoryPage;

//--------------------------------------------------------------------------------
// CTrailSample
//
// This class represents a sampling of the tracked nodes used on the trail. Note
// that this has room for the maximum number of samples, however, these are ONLY
// valid if the number of tracked nodes includes those samples (i.e. if the number
// of tracked nodes is 2, only samples 0 and 1 are valid 3 is NOT)
//--------------------------------------------------------------------------------
class CTrailSample
{
public:

	//note no constructor or destructor as these objects are created in place in
	//memory!

	//the next sample in this list
	CTrailSample*	m_pNextSample;

	//the V coordinate to use for this sample
	float			m_fVTexCoord;

	//the lifetime of this sample
	float			m_fLifetime;

	//information stored by a node sample
	struct SNodeSample
	{
		//the position in space of this node
		LTVector	m_vPos;

		//the normal of this point
		LTVector	m_vNormal;

		//the tangent of this point
		LTVector	m_vTangent;
	};

	//our node samples
	SNodeSample		m_Nodes[CPolyTrailProps::knMaxTrackedNodes];
};

//--------------------------------------------------------------------------------
// CPolyTrailFX
//--------------------------------------------------------------------------------
class CPolyTrailFX : 
	public CBaseFX
{
public:

	CPolyTrailFX();
	~CPolyTrailFX();

	//Overrides from CBaseFX
	virtual bool	Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps);
	virtual bool	Update(float tmFrameTime);
	virtual bool	SuspendedUpdate(float tmFrameTime);
	virtual void	Term();
	virtual void	EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData);

	virtual bool	IsVisibleWhileSuspended();
	virtual bool	IsFinishedShuttingDown();


private:

	//called to examine the properties and setup our tracked node information for this effect
	void	InitTrackedNodes();

	//called to update the current positions of the nodes and store them within the head of the
	//sample list
	void	UpdateCurrentSamplePositions(const LTVector& vObjectPos);

	//called to update the current samples that we have and also the visibility for this object
	void	UpdateSamples(float fElapsed, const LTVector& vObjectPos);

	//utility function that calculates the distance between the head sample and the next sample
	//for the node that is selected to be the node used for distance tracking
	float	CalcHeadSegmentDistance() const;

	//utility function that will update the normals and tangents of the head and adjacent sample
	void	UpdateHeadSampleNormals();

	//function called to calculate the tangent space for the specified node, and store it within
	//that node. This takes in the sample and node within the sample to calculate, and also the 
	//samples that should be used for the horizontal support and indices to use for the vertical support
	bool	GenerateTangentSpace(	CTrailSample* pCalcSample, uint32 nCalcNode, 
									const CTrailSample* pPrevSample, const CTrailSample* pNextSample,
									uint32 nPrevNode, uint32 nNextNode);

	//called to determine whether or not a new head sample should be created
	bool	ShouldCreateNewSample() const;

	//function to allocate a new sample. This will remove it from the free list, and set this up so
	//that the list refers to this as the new head
	bool	AllocateHeadSample();

	//called to free the nodes that are in the list that fall after the one provided
	void	FreeSamplesAfter(CTrailSample& Node);

	//called to allocate a new memory page for this trail and add it to the free list
	bool	AddNewPageToFreeList();

	//hook for the custom render object, this will just call into the render function
	static void CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser);

	//function that handles the custom rendering
	void RenderPolyTrail(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera);

	//called to access the lighting properties
	const CPolyTrailProps*			GetProps() const	{ return (const CPolyTrailProps*)m_pProps; }

	//this structure stores the data needed to access the nodes or sockets that we are tracking
	struct STrackedNode
	{
		//the object that this node or socket belongs to
		HOBJECT		m_hObject;

		//is this a node? If not it is a socket
		bool		m_bIsNode;

		//the socket or node that we are tracking
		union
		{
		public:
			HMODELSOCKET	m_hSocket;
			HMODELNODE		m_hNode;
		};
	};

	//our nodes that we are tracking (in addition to the main position)
	STrackedNode	m_TrackedNodes[CPolyTrailProps::knMaxTrackedNodes - 1];

	//the first sample in our list, and the one that is updated with the most up to date
	//changes
	CTrailSample*	m_pHeadSample;

	//the head of our free list where we take allocations from
	CTrailSample*	m_pFreeListAlloc;

	//the tail of our free list where freed entities are added
	CTrailSample*	m_pFreeListFree;

	//the listing of memory pages that we are using
	CMemoryPage*	m_pPageHead;

	//our custom render object
	HOBJECT			m_hObject;

	//this is the distance that we have accumulated, not including the current sample, used for generating
	//V coordinates
	float			m_fAccumulatedDist;

	//the number of nodes that we are tracking
	uint32			m_nNumTrackedNodes;

	//the pitch of our trail samples in bytes
	uint32			m_nTrailSamplePitch;

	//the current number of trail samples that we have
	uint32			m_nNumSamples;

	//the total distance that we have covered, used for texture coordinate generation
	float			m_fTotalDistance;

	//the time elapsed between generating samples
	float			m_fSampleElapsed;

	//the total amount of time that this effect has been around
	float			m_fTotalElapsed;
};

#endif