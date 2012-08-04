#ifndef __RAGDOLLNODE_H__
#define __RAGDOLLNODE_H__

class CRagDoll;

class CRagDollNode
{
public:

	CRagDollNode() :
		m_hModelNode(INVALID_MODEL_NODE),
		m_pRagDoll(NULL),
		m_fBSphereRadius(0.0f),
		m_fWeight(1.0f)
	{
	}

	~CRagDollNode()
	{
	}

	//the model node that this node corresponds to
	HMODELNODE		m_hModelNode;

	//the previous and current position of this node
	LTVector		m_vPosition[2];

	//the radius of the bounding sphere (0 indicates don't perform collisions)
	float			m_fBSphereRadius;

	//the parent model. I know this is a cyclic dependency, but it is opaque to the rag doll node
	//and is intended for use inside of the node controller
	CRagDoll*		m_pRagDoll;

	//this is the weight of the node
	float			m_fWeight;

private:

};

#endif
