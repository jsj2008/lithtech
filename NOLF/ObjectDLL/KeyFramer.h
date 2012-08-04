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
#include "iltlightanim.h"

class KeyframerLight;


#define ADD_KEYFRAMER_PROPERTIES(groupflag) \
	ADD_STRINGPROP_FLAG(ObjectName, "", groupflag) \
	ADD_STRINGPROP_FLAG(BaseKeyName, "", groupflag) \
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
	ADD_STRINGPROP(RotationWave, WAVESTR_SINE)\
	PROP_DEFINEGROUP(Waveform, (groupflag == 0 ? PF_GROUP1 : (groupflag<<1))) \
		ADD_REALPROP_FLAG(TotalPathTime, 0.0f, (groupflag == 0 ? PF_GROUP1 : (groupflag<<1))) \
        ADD_BOOLPROP_FLAG(Linear, LTTRUE, (groupflag == 0 ? PF_GROUP1 : (groupflag<<1))) \
        ADD_BOOLPROP_FLAG(Sine, LTFALSE, (groupflag == 0 ? PF_GROUP1 : (groupflag<<1))) \
        ADD_BOOLPROP_FLAG(SlowOff, LTFALSE, (groupflag == 0 ? PF_GROUP1 : (groupflag<<1))) \
        ADD_BOOLPROP_FLAG(SlowOn, LTFALSE, (groupflag == 0 ? PF_GROUP1 : (groupflag<<1))) \
	PROP_DEFINEGROUP(Lighting, (groupflag == 0 ? PF_GROUP2 : (groupflag<<2))) \
		ADD_STRINGPROP_FLAG(ShadowLights, "", (groupflag == 0 ? PF_GROUP2 : (groupflag<<2))) \
        ADD_BOOLPROP_FLAG(ShadowMaps,LTFALSE, PF_HIDDEN|(groupflag == 0 ? PF_GROUP2 : (groupflag<<2)))

struct KEYNODE
{
	KEYNODE()	{ pPrev = NULL; pNext = NULL; }

	KeyData		keyData;
	KEYNODE*	pPrev;
	KEYNODE*	pNext;
};

struct PREWORLDMODEL
{
    LTVector     m_vPos;
    LTRotation   m_rRot;
	char		m_WorldModelName[64];
};

enum KFWaveType
{
	KFWAVE_LINEAR,
	KFWAVE_SINE,
	KFWAVE_SLOWOFF,
	KFWAVE_SLOWON
};


// Implements the IObjectPlugin interface to generate light animations
// in the preprocessor.
class KeyframerPlugin : public IObjectPlugin
{
public:
    virtual LTRESULT PreHook_Light(
        ILTPreLight *pInterface,
		HPREOBJECT hObject);
};

class KeyFramer : public BaseClass
{
	friend class CinematicTrigger;

	public :

		KeyFramer();
		virtual ~KeyFramer();

	protected :

		enum		KFDirection { KFD_FORWARD, KFD_BACKWARD };

        void        GoActive(LTBOOL bReset=LTTRUE);
		void		GoInActive();
		void		TriggerMsg(HOBJECT hSender, const char* pMsg);

		void		ProcessCurrentKey();

		void		On();
		void		Off();
		void		Pause();
		void		Resume();
		void		Forward();
		void		Backward();
		void		ToggleDir();
		void		DeferCommand(ConParse parse);

		void		GoToKey(char* pKeyName);
		void		MoveToKey(ConParse parse);
        KEYNODE*    FindKey(char* pKeyName, KEYNODE* pTest=LTNULL, LTBOOL* pbAtOrBefore=LTNULL);

		void		HandleLinkBroken(HOBJECT hLink);
        void        UpdateObjects(LTBOOL bInterpolate=LTTRUE, LTBOOL bTeleport=LTFALSE);

        uint32      EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32      ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void		SetObjectName(char* pName);
		void		SetTarget(char* pName);

		virtual	void ReachedDestination();

		KFDirection m_eDirection;

		HSTRING		m_hstrObjectName;
		HSTRING		m_hstrTargetName;
		HSTRING		m_hstrBaseKeyName;
		HSTRING		m_hstrActiveSnd;
		HSTRING		m_hstrDestCmd;

        LTBOOL       m_bStartActive;
        LTBOOL       m_bStartPaused;
        LTBOOL       m_bLooping;
        LTBOOL       m_bActive;
        LTBOOL       m_bPaused;
        LTBOOL       m_bFinished;
        uint8       m_nNumKeys;

		HOBJECT		m_hTargetObject;
        LTVector     m_vTargetOffset;

		ObjectList*	m_pObjectList;

        CDynArray<LTVector>   m_pOffsets;
        CDynArray<LTRotation> m_pRotations;

		WaveType	m_RotationWave;

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

		HOBJECT		m_hActiveSndObj;
        HLTSOUND    m_hActiveSnd;
        LTFLOAT      m_fSoundRadius;
		char*		m_pCommands;

		HCLASS		m_hKeyframerLightClass;

		HLIGHTANIM	m_hLightAnim;

        LTFLOAT      m_fEarliestGoActiveTime;

		LTBOOL		m_bPausedOnLoad;

	private :

		void InitialUpdate();
		void Update();

        LTBOOL ReadProps();

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTBOOL CreateKeyList();
		void  CreateObjectList();
		void  CalculateVelocityInfo();


		// Updates the lightmaps/shadowmaps for ShadowObjects.
		void  UpdateShadowObjects();

		// Updates the animation of a KeyframerLight.
        void  UpdateKeyframerLight(KeyframerLight *pLight, LTVector vLightPos);

		// Get the index of a KEYNODE into our m_pKeys list.
        uint32 GetKeyIndex(KEYNODE *pNode);

        LTBOOL CalcCurPos(LTBOOL & bAtKey);

		// Calculate the lightmap animation indices to blend between, and the percentage between them
		void CalcLMAnimFrames(uint32 &uFrame1, uint32 &uFrame2, float &fPercent);
};



#endif // __KEYFRAMER_H__