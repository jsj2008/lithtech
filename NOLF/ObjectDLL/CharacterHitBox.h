// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.h
//
// PURPOSE : Character hit box object class definition
//
// CREATED : 01/05/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_HIT_BOX_H__
#define __CHARACTER_HIT_BOX_H__

#include "GameBase.h"
#include "ModelButeMgr.h"
#include "TemplateList.h"

class CProjectile;
class CAttachments;

struct NodeRadiusStruct
{
	NodeRadiusStruct()
	{
        hModel = LTNULL;
		eNode = eModelNodeInvalid;
	}

	~NodeRadiusStruct()
	{
		if (hModel)
		{
            g_pLTServer->RemoveObject(hModel);
		}
	}

	HOBJECT hModel;
	ModelNode eNode;
};

class CCharacterHitBox : public GameBase
{
	public :

		CCharacterHitBox();
		virtual ~CCharacterHitBox();

        LTBOOL Init(HOBJECT hModel);
        void  SetOffset(LTVector vOffset) { m_vOffset = vOffset; }

		void  Update();

        LTBOOL HandleImpact(CProjectile* pProj, IntersectInfo & iInfo,
            LTVector vDir, LTVector & vFrom);

        LTBOOL DidProjectileImpact(CProjectile* pProjectile);

		HOBJECT	GetModelObject() const { return m_hModel; }

		LTBOOL CanActivate() const { return m_bCanActivate; }
		void SetCanActivate(LTBOOL b) { m_bCanActivate = b; }

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	protected :

        virtual	LTVector GetBoundingBoxColor();

        LTBOOL  HandleVectorImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector& vDir,
            LTVector& vFrom, ModelNode& eModelNode);

        LTBOOL  UsingHitDetection();
		void	SetModelNodeLastHit(ModelNode eModelNode);

		ModelSkeleton	GetModelSkeleton();
		CAttachments*	GetAttachments();
        LTFLOAT			GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		HOBJECT		m_hModel;
        LTVector	m_vOffset;

	private :

		// Debugging helper...
		CTList<NodeRadiusStruct*>	m_NodeRadiusList;

		LTBOOL m_bCanActivate;

		void CreateNodeRadiusModels();
		void RemoveNodeRadiusModels();
		void UpdateNodeRadiusModels();

		void Save(HMESSAGEREAD hRead);
		void Load(HMESSAGEWRITE hWrite);
};


#endif  // __CHARACTER_HIT_BOX_H__
