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
#include "CDynArray.h"
#include "KeyData.h"
#include "commonutilities.h"
#include "iobjectplugin.h"
#include "GameBase.h"

LINKTO_MODULE( KeyFramer );


#define ADD_KEYFRAMER_PROPERTIES(groupflag) \
	ADD_STRINGPROP_FLAG(ObjectName, "", groupflag) \
	ADD_STRINGPROP_FLAG(BaseKeyName, "", groupflag) \
	ADD_STRINGPROP_FLAG(ExternalKeyFile, "", groupflag) \
	ADD_REALPROP_FLAG(KeyDataIndex, -1.0f, groupflag | PF_HIDDEN) \
    ADD_BOOLPROP_FLAG(PushObjects, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(StartActive, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(StartPaused, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(Looping, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(AlignToPath, LTFALSE, groupflag) \
    ADD_BOOLPROP_FLAG(IgnoreOffsets, LTFALSE, groupflag) \
	ADD_STRINGPROP_FLAG(TargetName, "", groupflag) \
	ADD_VECTORPROP_VAL_FLAG(TargetOffset, 0.0f, 0.0f, 0.0f, groupflag) \
	ADD_STRINGPROP_FLAG(ActiveSound, "", groupflag) \
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, groupflag) \
	PROP_DEFINEGROUP(Waveform, PF_GROUP(groupflag + 1)) \
		ADD_REALPROP_FLAG(TotalPathTime, 0.0f, PF_GROUP(groupflag + 1)) \
		ADD_STRINGPROP_FLAG(Wavetype, "Linear", PF_STATICLIST | PF_GROUP(groupflag + 1)) \

struct KEYNODE
{
	KEYNODE()	{ pPrev = NULL; pNext = NULL; }

	KeyData		keyData;
	KEYNODE*	pPrev;
	KEYNODE*	pNext;
};

enum KFWaveType
{
	KFWAVE_LINEAR,
	KFWAVE_SINE,
	KFWAVE_SLOWOFF,
	KFWAVE_SLOWON,
	KFWAVE_MAX = KFWAVE_SLOWON
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

	protected :

		enum		KFDirection { KFD_FORWARD, KFD_BACKWARD };

        void        GoActive(LTBOOL bReset=LTTRUE);
		void		GoInActive();

		void		ProcessCurrentKey();

		void		On();
		void		Off();
		void		Pause();
		void		Resume();
		void		Forward();
		void		Backward();
		void		ToggleDir();
		void		DeferCommand(const CParsedMsg &cMsg);

		void		GoToKey(const char* pKeyName);
		void		MoveToKey(const CParsedMsg &cMsg);
        KEYNODE*    FindKey(const char* pKeyName, KEYNODE* pTest=LTNULL, LTBOOL* pbAtOrBefore=LTNULL);

		void		HandleLinkBroken(HOBJECT hLink);
        void        UpdateObjects(LTBOOL bInterpolate=LTTRUE, LTBOOL bTeleport=LTFALSE);

        uint32      EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool		OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void		SetObjectName(char* pName);
		void		SetTarget(const char* pName);

		virtual	void ReachedDestination();

		KFDirection m_eDirection;

		HSTRING		m_hstrObjectName;
		HSTRING		m_hstrTargetName;
		HSTRING		m_hstrBaseKeyName;
		HSTRING		m_hstrActiveSnd;
		HSTRING		m_hstrDestCmd;
		uint32		m_nKeyDataIndex;

        LTBOOL       m_bStartActive;
        LTBOOL       m_bStartPaused;
        LTBOOL       m_bLooping;
        LTBOOL       m_bActive;
        LTBOOL       m_bPaused;
        LTBOOL       m_bFinished;
        uint16       m_nNumKeys;

		LTObjRef	m_hTargetObject;
        LTVector     m_vTargetOffset;

		ObjectList*	m_pObjectList;

        CDynArray<LTVector>   m_pOffsets;
        CDynArray<LTRotation> m_pRotations;

		KEYNODE*	m_pKeys;
		KEYNODE*	m_pCurKey;
		KEYNODE*	m_pPosition1;
		KEYNODE*	m_pPosition2;
		KEYNODE*	m_pLastKey;
		KEYNODE*	m_pDestinationKey;

        LTFLOAT      m_fCurTime;
        LTFLOAT      m_fEndTime;
        LTBOOL       m_bFirstUpdate;
        LTBOOL       m_bUseVelocity;
        LTBOOL       m_bAlignToPath;
        LTBOOL       m_bPushObjects;
        LTBOOL       m_bIgnoreOffsets;

		KFWaveType	m_eWaveform;
        LTFLOAT      m_fTotalPathTime;
        LTFLOAT      m_fTotalDistance;
        LTFLOAT      m_fVelocity;
        LTVector     m_vCurPos;
        LTFLOAT      m_fKeyPercent;

		LTObjRef	m_hActiveSndObj;
        HLTSOUND    m_hActiveSnd;
        LTFLOAT      m_fSoundRadius;
		char*		m_pCommands;

        LTFLOAT      m_fEarliestGoActiveTime;

		LTBOOL		m_bPausedOnLoad;

	private :

		void InitialUpdate();
		void Update();

        LTBOOL ReadProps();

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTBOOL CreateKeyList();
		void  CalculateVelocityInfo();


		// Get the index of a KEYNODE into our m_pKeys list.
        uint32 GetKeyIndex(KEYNODE *pNode);

        LTBOOL CalcCurPos(LTBOOL & bAtKey);
};



#endif // __KEYFRAMER_H__