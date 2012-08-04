// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AICMD_H__
#define __AICMD_H__

#include "ltengineobjects.h"

enum Move
{
	eMoveRun,
	eMoveSwim,
	eMoveWalk,
};

enum Posture
{
	ePostureCrouch,
	ePostureStand,
};

enum Mood
{
	eMoodHappy,
	eMoodAngry,
	eMoodSad,
	eMoodTense,
	eMoodAgree,
	eMoodDisagree,
};

enum Task
{
	eTaskWait,
	eTaskClipboard,
	eTaskDust,
	eTaskSweep,
	eTaskWipe,
	eTaskTicket,
};

class AICmd : public BaseClass
{
	public :

		// Ctors/Dtors/etc

		AICmd();
		virtual ~AICmd();

		// Engine

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		char			m_szString[1024];
		HSTRING			m_hstrName;
};

class AICmdIdle : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdIdle();
		virtual ~AICmdIdle();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :
};

class AICmdAware : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAware();
		virtual ~AICmdAware();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :
};

class AICmdLookAt : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdLookAt();
		virtual ~AICmdLookAt();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTVector	m_vPosition;
};

class AICmdTail : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdTail();
		virtual ~AICmdTail();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		enum Constants
		{
			kMaxNodes = 12,
		};

	protected :

		HSTRING		m_ahstrNodes[kMaxNodes];
};

class AICmdFollowFootprint : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdFollowFootprint();
		virtual ~AICmdFollowFootprint();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTBOOL		m_bSearch;
};

class AICmdInvestigate : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdInvestigate();
		virtual ~AICmdInvestigate();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrEnemy;
		SenseType	m_stSenseType;
		LTVector	m_vPosition;
		LTBOOL		m_bSearch;
};

class AICmdCheckBody : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdCheckBody();
		virtual ~AICmdCheckBody();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrBody;
		LTBOOL		m_bSearch;
};

class AICmdSearch : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdSearch();
		virtual ~AICmdSearch();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTBOOL		m_bEngage;
		LTBOOL		m_bFace;
		LTBOOL		m_bPause;
};

class AICmdCharge : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdCharge();
		virtual ~AICmdCharge();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTFLOAT		m_fAttackDist;
		LTFLOAT		m_fYellDist;
		LTFLOAT		m_fStopDist;
};

class AICmdChase : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdChase();
		virtual ~AICmdChase();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :
};

class AICmdPanic : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdPanic();
		virtual ~AICmdPanic();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
		LTBOOL		m_bCanActivate;
};

class AICmdDistress : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdDistress();
		virtual ~AICmdDistress();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTBOOL		m_bCanActivate;
};

class AICmdDrowsy : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdDrowsy();
		virtual ~AICmdDrowsy();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :
};

class AICmdUnconscious : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdUnconscious();
		virtual ~AICmdUnconscious();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTBOOL		m_bAware;
		LTFLOAT		m_fTime;
};

class AICmdStunned : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdStunned();
		virtual ~AICmdStunned();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :
};

class AICmdAttack : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAttack();
		virtual ~AICmdAttack();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTBOOL		m_bChase;
		LTFLOAT		m_fChaseDelay;
		Posture		m_ePosture;
};

class AICmdDraw : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdDraw();
		virtual ~AICmdDraw();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :
};

class AICmdAttackFromCover : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAttackFromCover();
		virtual ~AICmdAttackFromCover();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
		uint32		m_nRetries;
};

class AICmdAttackFromVantage : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAttackFromVantage();
		virtual ~AICmdAttackFromVantage();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :
};

class AICmdAttackFromView : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAttackFromView();
		virtual ~AICmdAttackFromView();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
};

class AICmdAttackOnSight : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAttackOnSight();
		virtual ~AICmdAttackOnSight();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTFLOAT		m_fChaseDelay;
};

class AICmdAssassinate : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAssassinate();
		virtual ~AICmdAssassinate();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
		Move		m_eMove;
		LTBOOL		m_bIgnoreVisibility;
};

class AICmdCover : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdCover();
		virtual ~AICmdCover();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
		uint32		m_nRetries;
};

class AICmdPatrol : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdPatrol();
		virtual ~AICmdPatrol();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		enum Constants
		{
			kMaxNodes = 12,
		};

	protected :

		HSTRING		m_ahstrNodes[kMaxNodes];
		Task		m_eTask;
		LTBOOL		m_bFace;
		LTBOOL		m_bLoop;
		LTBOOL		m_bCircle;
};

class AICmdGoto : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdGoto();
		virtual ~AICmdGoto();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		enum Constants
		{
			kMaxNodes = 12,
		};

	protected :

		HSTRING		m_ahstrNodes[kMaxNodes];
		Move		m_eMove;
		LTBOOL		m_bFace;
		LTBOOL		m_bLoop;
};

class AICmdFlee : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdFlee();
		virtual ~AICmdFlee();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrDanger;
};

class AICmdFollow : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdFollow();
		virtual ~AICmdFollow();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		LTFLOAT		m_fRange;
		LTFLOAT		m_fRangeTime;
		Move		m_eMove;
};

class AICmdGetBackup : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdGetBackup();
		virtual ~AICmdGetBackup();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
		Move		m_eMove;
};

class AICmdUseObject : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdUseObject();
		virtual ~AICmdUseObject();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
		Move		m_eMove;
};

class AICmdPickupObject : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdPickupObject();
		virtual ~AICmdPickupObject();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrNode;
		Move		m_eMove;
};

class AICmdTalk : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdTalk();
		virtual ~AICmdTalk();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		Mood		m_eMood;
		HSTRING		m_hstrFace;
		LTFLOAT		m_fFaceTime;
};

class AICmdAnimate : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAnimate();
		virtual ~AICmdAnimate();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrAnimation;
		LTBOOL		m_bLoop;
};

class AICmdAttackProp : public AICmd
{
	typedef AICmd super;

	public :

		// Ctors/Dtors/etc

		AICmdAttackProp();
		virtual ~AICmdAttackProp();

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pData);

		// ToString

		virtual const char* ToString();

		// Verify

		virtual void Verify();

	protected :

		HSTRING		m_hstrProp;
};
/*
class AICmdCome : public AICmd
{
	protected :


};

class AICmdParaDive : public AICmd
{
	protected :


};

class AICmdParaShoot : public AICmd
{
	protected :


};

class AICmdParaDie : public AICmd
{
	protected :


};

class AICmdParaEscape : public AICmd
{
	protected :


};

class AICmdHeliAttack : public AICmd
{
	protected :


};

class AICmdScotBox : public AICmd
{
	protected :


};

class AICmdIngeSing : public AICmd
{
	protected :


};
*/
#endif