// ----------------------------------------------------------------------- //
//
// MODULE  : KeyFramer.h
//
// PURPOSE : KeyFramer definition
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#ifndef __KEYFRAMER_H__
#define __KEYFRAMER_H__

#include "ltengineobjects.h"
#include "CommonUtilities.h"
#include "iobjectplugin.h"
#include "GameBase.h"

LINKTO_MODULE( KeyFramer );

//forward declarations
struct KeyData;

//the listing of the various velocity ramping methods that we support
enum EKFWaveType
{
	eKFWT_Linear,
	eKFWT_Sine,
	eKFWT_SlowOff,
	eKFWT_SlowOn,

	//note that this must come last as it counts the number of wave types
	eKFWT_NumWaveTypes,
};

//the listing of the various interpolation methods that we support
enum EKFInterpType
{
	eKFIT_Linear,
	eKFIT_Bezier,

	//note that this must come last as it counts the number of interpolation types
	eKFIT_NumInterpolationTypes,
};

#define KEYFRAMER_BLINDOBJECTID		0x6aaf0884



// Implements the IObjectPlugin interface for the keyframer
class KeyframerPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList( const char *szRezPath, 
											 const char *szPropName,
											 char **aszStrings,
											 uint32 *pcStrings,
											 const uint32 cMaxStrings,
											 const uint32 cMaxStringLength );
};

class KeyFramer : public GameBase
{
	public :

		KeyFramer();
		virtual ~KeyFramer();

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	private:

		//this structure holds all data necessary for an object that is keyframed, including
		//a handle to the object and the relative transform
		struct STrackedObject
		{
			STrackedObject() :
				m_hObject(NULL)
			{}

			//note: we use a straight handle because we create manual object links
			//making an LTObjRef unnecessary
			HOBJECT				m_hObject;

			//the relative transform for when maintain offsets is enabled
			LTRigidTransform	m_tRelativeTrans;
		};

		enum		KFDirection { KFD_FORWARD, KFD_REVERSE };

		void		GoActive(bool bReset);
		void		GoInActive();

		void		ProcessCurrentKey();

		void		On();
		void		Off();
		void		Pause();
		void		Resume();
		void		Forward();
		void		Reverse();
		void		ToggleDir();

		//called to teleport the keyframer to the designated key
		void		GoToKey(const char* pKeyName);

		//called to have the keyframer move to the designated key
		void		MoveToKey(const CParsedMsg &cMsg);

		const KeyData*	FindKey(const char* pKeyName, const KeyData* pTest=NULL, bool* pbAtOrBefore=NULL);

		//called to handle when a link is broken to an object we are holding onto so we can clean
		//up our reference
		void		HandleLinkBroken(HOBJECT hLink);
		void		UpdateObjects(bool bTeleport);

		uint32		EngineMessageFn(uint32 messageID, void *pData, float fData);

		//called to set the target to a specific object given its name
		void		SetTarget(const char* pName);

		//this function is called whenever the destination key is reached
		void		ReachedDestination();

		//given a valid key, this will return the index of the key
		uint32		GetKeyIndex(const KeyData* pKey) const;

		//given an index, this will return a key
		const KeyData*	GetKeyFromIndex(uint32 nIndex) const;

		//given a key, this will get the key immediately before it (this assumes that
		//the passed in key is not the first key)
		const KeyData*	GetPrevKey(const KeyData* pKey) const;

		//given a key, this will get the key immediately after it (this assumes that the
		//passed in key is not the last key)
		const KeyData*	GetNextKey(const KeyData* pKey) const;

		//called to obtain the ending time of the path
		float	GetPathEndTime() const			{ return (m_bUseDistance) ? m_fDistancePathTime : m_fEndTime; }

		//given two keys and a time value that lies between them, this will compute the percentage that
		//the time value is between those keys
		float	GetKeyPercentage(const KeyData* pKey1, const KeyData* pKey2, float fTimeVal, float fTotalTime) const;

		//given a key, this will convert it to linear time, even if it is stored as a distance
		float	GetKeyTime(const KeyData* pKey) const;

		//given a key that has been passed, this will handle triggering any messages and will
		//detect if this is the destination key, and return whether or not it should continue moving
		//beyond this key
		bool	HandleKeyEvents(const KeyData* pKey);

		//called to stop the active sound. This will only release the sound handle,
		//but will not clear out any of the stored sound data (attached object, radius, name)
		void	StopActiveSound();

		//handles moving the keyframer forward given a specified interval of time
		void	MoveKeyframerForward(float fFrameTime, float fTotalTime);

		//handles moving the keyframer backward given a specified interval of time
		void	MoveKeyframerBackward(float fFrameTime, float fTotalTime);

		//called to handle immediately after the object is created NOT from a save file
		void	InitialUpdate();

		//called to handle a typical object update
		void	Update();

		//called to read in the engine properties into our data
		bool	ReadProps(const GenericPropList *pProps);

		//handles serialization of this object to a save file
		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// Create object list.
		bool	CreateObjectList();

		// Creates the list of keys.
		bool	CreateKeyList();

		//called to free all of our keys and all the associated data
		void	FreeKeys();

		//called once all objects have been created to handle setting up our object list
		void	AllObjectsCreated();

		//called to calculate the current transform of the keyframer using the two position keys
		//and the percentage between the keys
		LTRigidTransform CalcCurTransform(const KeyData* pKey1, const KeyData* pKey2, float fKeyInterp) const;

		//called to calculate the distance between the specified keys
		float CalcKeyDistance(const KeyData* pKey1, const KeyData* pKey2) const;

		//the current direction of our keyframer
		KFDirection m_eDirection;

		//the name of the objects we will keyframe
		std::string	m_sObjectName;

		//the name of our target object
		std::string	m_sTargetName;

		//the base string for all of our keys
		std::string	m_sBaseKeyName;

		//the name of the active sound record that we should play for this keyframer
		std::string	m_sActiveSndRecord;

		//the command that should be sent when we reach our destination
		std::string	m_sDestCmd;

		//the index into the blind object data to obtain our key data
		uint32		m_nKeyDataIndex;

		//whether or not we should start the keyframer active or not
		bool		m_bStartActive;

		//whether or not we should start the keyframer paused or not (if paused it starts active as well)
		bool		m_bStartPaused;

		//whether or not this keyframer should loop when it hits the end of the track
		bool		m_bLooping;

		//are we currently active? 
		bool		m_bActive;

		//are we currently active, but paused?
		bool		m_bPaused;

		//the object that we are targeting
		LTObjRef	m_hTargetObject;

		//an offset from the position of the object in world space
        LTVector    m_vTargetOffset;

		//this is the list of the objects that we are tracking and any associated data needed
		//to track these objects
		typedef std::vector<STrackedObject, LTAllocator<STrackedObject, LT_MEM_TYPE_OBJECTSHELL> > TTrackedObjectList;
		TTrackedObjectList	m_TrackedObjects;

		//the current keys that we are between. Previous always is <= next in the key ordering
		const KeyData*	m_pNextKey;
		const KeyData*	m_pPrevKey;

		//the key that we are trying to move to
		const KeyData*	m_pDestinationKey;

		//the current amount of elapsed time in the track
		float		m_fCurTime;

		//wheter or not we should use distance based movement or time based movement
		bool		m_bUseDistance;

		//should we push other objects out of the way?
		bool		m_bPushObjects;

		//if we need to preserve relative transforms from the keyframed objects
		bool		m_bMaintainOffsets;

		//for distance based movement, this is the waveform that should be applied to blend movement
		EKFWaveType		m_eWaveform;

		//the interpolation method that is used in between keys
		EKFInterpType	m_eInterpType;

		//the size of a key in bytes
		uint32			m_nKeyStride;

		//this is the total time that it should take to cover the path when using distance
		float		m_fDistancePathTime;

		//the total linear distance along the path
		float		m_fTotalDistance;

		//the percentage that our current time is between the previous and next keys
		float		m_fKeyPercent;

		//the currently active sound object
		LTObjRef	m_hActiveSndObj;
        HLTSOUND    m_hActiveSnd;

		//-----------------------------------------
		//key information that is gathered from the key blind object data and is not saved or loaded

		//our command buffer that keys index into
		const char*		m_pCommandBuffer;

		//our listing of keys
		const KeyData*	m_pKeys;

		//a pointer to the last key in our list
		const KeyData*	m_pLastKey;

		//the total amount of time used by the keys
		float			m_fEndTime;

		//the number of keys in our list
		uint32			m_nNumKeys;

		//determines if we need to setup object references once the objects become available
		bool			m_bInitializedObjRefs;

		// Message Handlers...
		DECLARE_MSG_HANDLER( KeyFramer, HandleOnMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleOffMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandlePauseMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleResumeMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleForwardMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleReverseMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleToggleDirMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleGoToMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleMoveToMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleTargetMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleClearTargetMsg );
		DECLARE_MSG_HANDLER( KeyFramer, HandleTargetOffsetMsg );
};



#endif // __KEYFRAMER_H__
