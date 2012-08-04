// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterHitBox.h
//
// PURPOSE : Character hit box object class definition
//
// CREATED : 01/05/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_HIT_BOX_H__
#define __CHARACTER_HIT_BOX_H__

#include "GameBase.h"
#include "ModelButeMgr.h"
#include "TemplateList.h"

class CProjectile;
class CAttachments;
struct IHitBoxUser;

LINKTO_MODULE( CharacterHitBox );

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

	LTObjRef hModel;
	ModelNode eNode;
};

class CCharacterHitBox : public GameBase
{
	public :

		CCharacterHitBox();
		virtual ~CCharacterHitBox();

		LTBOOL   Init(HOBJECT hModel, IHitBoxUser* pUser);
        void  SetOffset(LTVector vOffset) { m_vOffset = vOffset; }
		const LTVector& GetOffset() const { return m_vOffset; }

		void  Update();

        LTBOOL HandleImpact(CProjectile* pProj, IntersectInfo & iInfo,
            LTVector vDir, LTVector & vFrom);

        LTBOOL DidProjectileImpact(CProjectile* pProjectile);

		HOBJECT	GetModelObject() const { return m_hModel; }

		LTBOOL CanActivate() const { return m_bCanActivate; }
		void SetCanActivate(LTBOOL b) { m_bCanActivate = b; }

		bool CanBeSearched() const { return m_bCanBeSearched; }
		void SetCanBeSearched(bool b);

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		void	SetAnimControllingDims( bool bControls, HMODELANIM hAni ) { m_bAnimControlsDims = bControls; m_hControllingAnim = hAni; }
		bool	IsAnimControllingDims() { return m_bAnimControlsDims; }

		void	SetAnimControllingOffset( bool bControls, HMODELANIM hAni ) { m_bAnimControlsOffset = bControls; m_hControllingAnim = hAni; }	
		bool	IsAnimControllingOffset() { return m_bAnimControlsOffset; }

		void	SetDimsToModel();
		void	GetDefaultModelDims( LTVector &vDims );
		void	EnlargeAndSetDims( LTVector &vDims );

	protected :

		LTBOOL	GetNodeTransform(const char* const pszNodeName,LTransform &NodeTransform);
        virtual	LTVector GetBoundingBoxColor();

		ModelNode FindHitNode( LTVector const& vDir, LTVector const& vFrom );

        LTBOOL  HandleVectorImpact(CProjectile* pProj, IntersectInfo& iInfo, LTVector& vDir,
            LTVector& vFrom, ModelNode& eModelNode);

        LTFLOAT			GetNodeRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode);

		IHitBoxUser* m_pHitBoxUser;
		LTObjRef	m_hModel;
        LTVector	m_vOffset;
		
		bool		m_bAnimControlsDims;
		bool		m_bAnimControlsOffset;

		HMODELANIM	m_hControllingAnim;

	private :

		// Debugging helper...
		CTList<NodeRadiusStruct*>	m_NodeRadiusList;

		LTBOOL	m_bCanActivate;
		bool	m_bCanBeSearched;
		
		void CreateNodeRadiusModels();
		void RemoveNodeRadiusModels();
		void UpdateNodeRadiusModels();

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);
};


#endif  // __CHARACTER_HIT_BOX_H__
