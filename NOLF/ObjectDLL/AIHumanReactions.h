// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIHUMANREACTIONS_H__
#define __AIHUMANREACTIONS_H__

extern REACTIONSTRUCT g_aAIHumanReactions[];
extern int g_cAIHumanReactions;

#define DEFINE_REACTIONS_AIHUMAN() \
		protected : \
		virtual void DoReaction(HSTRING hstrReaction, CAISense* pAISense, LTBOOL bIndividual);\
		void AppendReturnString();\
		void SoundAlarm(CAISense* pAISense, LTBOOL bIndividual);\
		void GetBackup(CAISense* pAISense, LTBOOL bIndividual);\
		void TrainingFailure(CAISense* pAISense, LTBOOL bIndividual);\
		void HitSwitch(CAISense* pAISense, LTBOOL bIndividual);\
		void GoForCover(CAISense* pAISense, LTBOOL bIndividual);\
		void Panic(CAISense* pAISense, LTBOOL bIndividual);\
		void Distress(CAISense* pAISense, LTBOOL bIndividual);\
		void Surrender(CAISense* pAISense, LTBOOL bIndividual);\
		void Charge(CAISense* pAISense, LTBOOL bIndividual);\
		void DrawWeaponAndAttack(CAISense* pAISense, LTBOOL bIndividual);\
		void DrawWeaponAndAttackFromCover(CAISense* pAISense, LTBOOL bIndividual);\
		void DrawWeaponAndAttackFromVantage(CAISense* pAISense, LTBOOL bIndividual);\
		void Attack(CAISense* pAISense, LTBOOL bIndividual);\
		void AttackStay(CAISense* pAISense, LTBOOL bIndividual);\
		void AttackFromCover(CAISense* pAISense, LTBOOL bIndividual);\
		void AttackFromCoverStay(CAISense* pAISense, LTBOOL bIndividual);\
		void AttackFromCoverAlwaysRetry(CAISense* pAISense, LTBOOL bIndividual);\
		void AttackFromCoverRetryOnce(CAISense* pAISense, LTBOOL bIndividual);\
		void AttackOnSight(CAISense* pAISense, LTBOOL bIndividual);\
		void AttackFromVantage(CAISense* pAISense, LTBOOL bIndividual);\
		void CallOut(CAISense* pAISense, LTBOOL bIndividual);\
		void ShineFlashlight(CAISense* pAISense, LTBOOL bIndividual);\
		void LookAt(CAISense* pAISense, LTBOOL bIndividual);\
		void BecomeAlert(CAISense* pAISense, LTBOOL bIndividual);\
		void InvestigateAndSearch(CAISense* pAISense, LTBOOL bIndividual);\
		void InvestigateAndStay(CAISense* pAISense, LTBOOL bIndividual);\
		void InvestigateAndReturn(CAISense* pAISense, LTBOOL bIndividual);\
		void PassOut(CAISense* pAISense, LTBOOL bIndividual);\

#endif

