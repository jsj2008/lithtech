// ----------------------------------------------------------------------- //
//
// MODULE  : AICentralKnowledgeMgr.h
//
// PURPOSE : AICentralIntelligenceMgr class definition
//
// CREATED : 4/06/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AICENTRAL_KNOWLEDGE_MGR_H__
#define __AICENTRAL_KNOWLEDGE_MGR_H__

#include "AIClassFactory.h"
#include "LTObjRef.h"


// Forward declarations.
class CAICentralKnowledgeMgr;

extern CAICentralKnowledgeMgr *g_pAICentralKnowledgeMgr;

//
// ENUM: Types of knowledge.
//
enum EnumAICentralKnowledgeType
{
	kCK_InvalidType = 0,
	kCK_InvestigatingVolume,
	kCK_AlarmResponseVolume,
	kCK_ReservedVolume,
	kCK_AttackCroucher,
	kCK_NextDisappearTime,
	kCK_Attacking,
	kCK_AttackFromRoof,
	kCK_LastCombatSoundTime,
	kCK_CheckingBody,
	kCK_NextProneTime,
	kCK_AttackLunge,
	kCK_NextAlarmTime,
};


//
// CLASS: Record of knowledge.
//
class CAICentralKnowledgeRecord : public CAIClassAbstract
{
	public :
		DECLARE_AI_FACTORY_CLASS(CAICentralKnowledgeRecord);

		CAICentralKnowledgeRecord( );
		~CAICentralKnowledgeRecord( );

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

	public:

		EnumAICentralKnowledgeType	m_eKnowledgeType;	// Type of knowledge.

		LTObjRefNotifier	m_hAI;					// AI who this knowledge is related to.
		ILTBaseClass*		m_pAI;
		LTObjRefNotifier	m_hKnowledgeTarget;		// Target that knowledge is about.
		ILTBaseClass*		m_pKnowledgeTarget;
		LTFLOAT				m_fKnowledgeData;		// Generic float associated with knowledge.
		LTBOOL				m_bKnowledgeDataIsTime;	// Flags that the float data represents time.
		LTBOOL				m_bLinkKnowledge;		// Remove if AI or Target goes away.
};


//
// MAP: Map of all currently existing knowledge.
//
typedef std::multimap<EnumAICentralKnowledgeType, CAICentralKnowledgeRecord* > AICENTRAL_KNOWLEDGE_MAP;


//
// CLASS: Global manager for central Knowledge.
//
class CAICentralKnowledgeMgr : public ILTObjRefReceiver
{
	public : // Public methods

		 CAICentralKnowledgeMgr();
		~CAICentralKnowledgeMgr();

		void Init();
		void Term();

		virtual void Save(ILTMessage_Write *pMsg);
        virtual void Load(ILTMessage_Read *pMsg);

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Knowledge handling.

		void	RegisterKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget, LTBOOL bLinkKnowledge);
		void	RegisterKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget, LTBOOL bLinkKnowledge, LTFLOAT fData, LTBOOL bIsTime);

		void	RemoveKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget);
		void	RemoveKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI);
		
		void	ReplaceKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget, LTBOOL bLinkKnowledge, LTFLOAT fData, LTBOOL bIsTime);

		void	RemoveAllKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI);

		uint32	CountTargetMatches(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget);
		uint32	CountMatches(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget);
		uint32	CountMatches(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI);
		uint32	CountMatches(EnumAICentralKnowledgeType eKnowledgeType);

		LTFLOAT	GetKnowledgeFloat(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI);
		ILTBaseClass *	GetKnowledgeTarget(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI);

	protected:

		AICENTRAL_KNOWLEDGE_MAP		m_mapCentralKnowledge;
};

#endif
