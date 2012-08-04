//------------------------------------------------------------------------------------------
// ClientFXInstance.h
//
// Provides the definition for the classes that are needed for maintaining and updating a
// currently playing effect instance and referencing it. An effect instance is mainly
// a collection of effect keys and a current time position within a timeline and handles
// updating the keys appropriately as time advances.
//------------------------------------------------------------------------------------------
#ifndef __CLIENTFXINSTANCE_H__
#define __CLIENTFXINSTANCE_H__

//forward declarations
class CClientFXInstance;

#ifndef __BASEFX_H__
#	include "BaseFx.h"
#endif


//------------------------------------------------------------------------------------------
// CClientFXLink
//
// Utility class that represents an smart link to an effect instance. This will be broken
// when the effect goes away, and each effect can only have a single outstanding link to
// itself.
//------------------------------------------------------------------------------------------
class CClientFXLink
{
public:

	CClientFXLink() : m_pInstance(NULL) {}
	~CClientFXLink()	{ ClearLink(); }

	//called to set the effect that this link points to
	void				SetLink(CClientFXInstance* pInstance);

	//determines if this link is connected to an object
	bool				IsValid() const			{ return m_pInstance != NULL; }

	//gets the object the link is connected to
	CClientFXInstance*	GetInstance()			{ return m_pInstance; }

	//clears the connection and breaks the link
	void				ClearLink();

private:

	CClientFXInstance* m_pInstance;

	//we cannot allow copying since that will lead to the possibility that items
	//are deleted twice
	PREVENT_OBJECT_COPYING(CClientFXLink);
};

//------------------------------------------------------------------------------------------
// CClientFXLinkNode
//
// a node for a linked list of CLIENTFX_LINKs
//------------------------------------------------------------------------------------------
class CClientFXLinkNode
{
public:

	CClientFXLinkNode() : m_pNext(NULL)		{}
	~CClientFXLinkNode()						{ DeleteList(); }

	void AddToEnd(CClientFXLinkNode* pNode)
	{
		if(m_pNext)
			m_pNext->AddToEnd(pNode);
		else
			m_pNext = pNode;
	}

	CClientFXLink* GetElement(uint32 nElement)
	{
		if(nElement == 0)
			return &m_Link;
		if(m_pNext == NULL)
			return NULL;
		return m_pNext->GetElement(nElement - 1);
	}

	void DeleteList()
	{
		//just to avoid any possibility of cyclic deleting
		CClientFXLinkNode* pToDel = m_pNext;
		m_pNext = NULL;

		debug_delete(pToDel);
	}

	CClientFXLink			m_Link;
	CClientFXLinkNode		*m_pNext;

private:

	//we cannot allow copying since that will lead to a corrupt list
	PREVENT_OBJECT_COPYING(CClientFXLinkNode);
};


//------------------------------------------------------------------------------------------
// CClientFXInstance
// 
// Represents an actual instance of an effect which has a current time in a timeline, and
// a collection of keys that are the actual effects
//------------------------------------------------------------------------------------------

class CClientFXInstance :
	public ILTObjRefReceiver
{
public:

	// Member Functions

	CClientFXInstance();
	virtual ~CClientFXInstance();

	// Implementing classes will have this function called
	// when HOBJECT ref points to gets deleted.
	virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	//freezes all the FX associated with this instance
	void	Suspend();

	//unfreezes all the FX associated with this instance
	void	Unsuspend();

	//called to update the parent information for this effect, and all the effect keys contained.
	//This can be in node/socket space, object space, or world space depending upon which parameters
	//are provided. The transform is in the corresponding space. This will update even fixed
	//effect positions.
	void	SetParent(HOBJECT hObject, HMODELNODE hNode, HMODELSOCKET hSocket, const LTRigidTransform& tTransform);

	//same as above, but will set the parent of the effects to the provided physics rigid body. This will
	//hold onto those rigid bodies until the effects are destroyed or have a new parent set
	void	SetParent(HPHYSICSRIGIDBODY hParent, const LTRigidTransform& tTransform);

	//called to shutdown the effect, removing finished effects, and placing appropriate
	//effects into a shutdown state so that they can properly finish.
	void	Shutdown();

	bool	ExistFX(CBaseFX *pFX);

	//are all FX inactive?
	bool	IsDone();

	//is this suspended
	bool	IsSuspended() const;

	void	Hide();
	void	Show();

	void	ClearLink();

	//this will delete a single effect from its list of effects. Note that this will invalidate
	//the node, so the next pointer should have been cached previously if iterating
	void	DeleteFX(CBaseFX* pFX);

	void	RemoveAllEffects();

	//Given an instance and an effect that has just finished shutting down, it will take 
	//the appropriate course of action. Note that this will invalidate the node that is passed into it
	void	HandleShutdownEffect(CBaseFX* pFX);

	//Given an instance and a time interval, this will appropriately update
	//all effects contained within that interval
	void	UpdateInterval(float fStartInterval, float fEndInterval);

	//Updates the suspended status of the instance and returns that status
	bool	UpdateSuspended();

	// Member Variables
	LTList<CBaseFX*>	m_ActiveFXList;
	float				m_tmElapsed;
	float				m_fDuration;
	float				m_tmSuspended;			//time that this object was frozen (used when unfreezing to find the delta)
	LTObjRefNotifier	m_hParent;
	LTObjRefNotifier	m_hTarget;
	bool				m_bLoop;
	bool				m_bSmoothShutdown;
	bool				m_bShutdown;
	bool				m_bSuspended;
	bool				m_bShow;

	LTLink<CClientFXInstance*>	m_FXListLink;
	CClientFXLink*				m_pLink;

#ifndef _FINAL
	//for development builds, this saves the parameters that were used to create this effect. This
	//is disabled in development builds due to the increased memory usage and the fact that this
	//exists only for restarting of effects. Note that the parent should not be used as it could have
	//gone away

	//called to set the associated client fx creation structure
	void							SetCreateStruct(const CLIENTFX_CREATESTRUCT& cs);
	const CLIENTFX_CREATESTRUCT&	GetCreateStruct() const { return m_CreateStruct; }

private:

	CLIENTFX_CREATESTRUCT	m_CreateStruct;
#endif

private:

	//we don't support copying because of the lists and references to key instances
	PREVENT_OBJECT_COPYING(CClientFXInstance);
};


#endif
