//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : WorldNode.cpp
//
//	PURPOSE	  : Implements the CWorldNode class.
//
//	CREATED	  : January 19 1996
//
//
// ------------------------------------------------------------------ //

// Includes....
#include "bdefs.h"
#include "worldnode.h"
#include "editpoly.h"
#include "editbrush.h"
#include "geomroutines.h"



// ------------------------------------------------------------------ //
// Unique ID management helpers.
// ------------------------------------------------------------------ //

// ID reference counts..
static CMoArray<WORD> g_IDRefCounts(1000);

// Free ID list.
static CMoArray<uint32> g_FreeIDs(400);

/************************************************************************/
static void AddUniqueID(uint32 id)
{
	uint32 nAdded, index;
	
	// Create as many new IDs as we have to.
	nAdded = 0;
	while((id+1) > g_IDRefCounts.GetSize())
	{
		g_IDRefCounts.Append(0);
		g_FreeIDs.Append(g_IDRefCounts.LastI());
		++nAdded;
	}

	if(nAdded > 0)
	{
		g_FreeIDs.Pop();
	}
	else
	{
		index = g_FreeIDs.FindElement(id);
		if(index != BAD_INDEX)
			g_FreeIDs.Remove(index);
	}

	g_IDRefCounts[id]++;
}

/************************************************************************/
static uint32 GenerateUniqueID()
{
	uint32 ret;

	if(g_FreeIDs > 0)
	{
		ret = g_FreeIDs.Last();
		g_FreeIDs.Pop();
		
		g_IDRefCounts[ret] = 1;
		return ret;
	}
	else
	{
		g_IDRefCounts.Append(1);
		return g_IDRefCounts.LastI();
	}
}

/************************************************************************/
static void ReleaseUniqueID(uint32 id)
{
	if(id < g_IDRefCounts)
	{
		--g_IDRefCounts[id];
		if(g_IDRefCounts[id] == 0)
			g_FreeIDs.Append(id);
	}
}

/************************************************************************/
void CWorldNode::CommonConstructor( )
{
	m_pParent = NULL;
	m_Type = Node_Null;
	m_Flags = 0;
	m_lpszLabel=NULL;

#ifdef DIRECTEDITOR_BUILD	
	m_hItem = NULL;
#endif // DIRECTEDITOR_BUILD
}

/************************************************************************/
// Constructor
CWorldNode::CWorldNode()
{
	CommonConstructor();

	m_TypeName[0] = 0;
	m_UniqueID = GenerateUniqueID();

#ifdef DIRECTEDITOR_BUILD
	m_hItem = NULL;	
#endif
}

/************************************************************************/
// Destructor
CWorldNode::~CWorldNode()
{
	Term();

	ReleaseUniqueID(m_UniqueID);
}


void CWorldNode::Term()
{
	if (m_lpszLabel)
	{
		delete m_lpszLabel;
		m_lpszLabel=NULL;
	}	

	GDeleteAndRemoveElements(m_Children);

	m_pParent = NULL;
#ifdef DIRECTEDITOR_BUILD
	if (m_lpszLabel)
	{
		delete m_lpszLabel;
		m_lpszLabel=NULL;
	}	
	m_hItem = NULL;
#endif // DIRECTEDITOR_BUILD
}

/************************************************************************/
void CWorldNode::DoCopy(CWorldNode *pOther)
{
	m_Flags = pOther->m_Flags;
	
	//AddUniqueID(pOther->m_UniqueID);
	//m_UniqueID = pOther->m_UniqueID;

	strncpy(m_TypeName, pOther->m_TypeName, sizeof(m_TypeName)-1);
	m_PropList.CopyValues(&pOther->m_PropList);

#ifdef DIRECTEDITOR_BUILD
	SetNodeLabel(pOther->GetNodeLabel());
#endif // DIRECTEDITOR_BUILD
}

/************************************************************************/
// Sets the node label
void CWorldNode::SetNodeLabel( const char *szLabel )
{ 		
	// Delete the current label
	if (m_lpszLabel)
	{
		delete []m_lpszLabel;
		m_lpszLabel=NULL;
	}

	// Check to make sure that the new label is valid
	if (szLabel && strlen(szLabel) > 0)
	{
		// Allocate memory for the new label
		int nBufferSize=strlen(szLabel)+1;
		m_lpszLabel = new char[nBufferSize];
		memset(m_lpszLabel, 0, nBufferSize);
		strcpy(m_lpszLabel, szLabel);	
	}	
}

//------------------------------------------------------------------------------
//
//  CWorldNode::RemoveFromTree()
//
//  Purpose:	Removes a node from a tree.  Removed from children of object's 
//				parent.  Object's Children's parent set to object's parent.
//
//------------------------------------------------------------------------------
void CWorldNode::RemoveFromTree()
{
	GPOS pos;
	CWorldNode *pChild;

	ASSERT( m_pParent );

	// Give object's children to their grandparent...
	for(pos=m_Children; pos; )
	{
		pChild = m_Children.GetNext(pos);

		m_Children.RemoveAt(pChild);
		pChild->SetParent(m_pParent);
		m_pParent->m_Children.Append(pChild);
	}
	
	// Remove object from parent's children...
	m_pParent->RemoveChild( this );

	// Orphan...
	SetParent( NULL );
}

/************************************************************************/
void CWorldNode::RemoveChild( CWorldNode *pChild )
{
	ASSERT(m_Children.FindElement(pChild) != BAD_INDEX);

	m_Children.RemoveAt(pChild);
}

/************************************************************************/
BOOL CWorldNode::FindNodeInChildren( CWorldNode *pNode )
{
	GPOS pos;

	if( this == pNode )
		return TRUE;

	for(pos=m_Children; pos; )
		if(m_Children.GetNext(pos)->FindNodeInChildren(pNode))
			return TRUE;

	return FALSE;
}


#ifdef DIRECTEDITOR_BUILD

/***********************************************************************/
//handles toggling the viewing of the model
void CWorldNode::ShowModel(BOOL bShowModel, BOOL bIncludeChildren)
{
	if(bShowModel)
	{
		EnableFlag(NODEFLAG_SHOWMODEL);
	}
	else
	{
		ClearFlag(NODEFLAG_SHOWMODEL);
	}

	if(bIncludeChildren)
	{
		GPOS ChildPos = m_Children;

		while(ChildPos)
		{
			m_Children.GetNext(ChildPos)->ShowModel(bShowModel, TRUE);
		}
	}
}

/************************************************************************/
// Hide the node
void CWorldNode::HideNode(BOOL bHideChildren)
{
	m_Flags |= NODEFLAG_HIDDEN;

	// Hide the children
	if (bHideChildren)
	{
		GPOS pos=m_Children;
		while (pos)
		{
			m_Children.GetNext(pos)->HideNode(bHideChildren);
		}
	}
}

/************************************************************************/
// Show the node
void CWorldNode::ShowNode(BOOL bShowChildren)
{
	if (m_Flags & NODEFLAG_HIDDEN)
	{
		m_Flags &= ~NODEFLAG_HIDDEN;
	}

	// Show the children
	if (bShowChildren)
	{
		GPOS pos=m_Children;
		while (pos)
		{
			m_Children.GetNext(pos)->ShowNode(bShowChildren);
		}
	}
}

/************************************************************************/
// Toggle the hidden status of the node
void CWorldNode::ToggleHideNode(BOOL bHideChildren)
{		
	if (m_Flags & NODEFLAG_HIDDEN)
	{
		ShowNode();
	}
	else
	{
		HideNode();
	}
	
	// Toggle the children
	if (bHideChildren)
	{
		GPOS pos=m_Children;
		while (pos)
		{
			m_Children.GetNext(pos)->ToggleHideNode(bHideChildren);
		}
	}
}

//notification that all properties have been changed
void CWorldNode::RefreshAllProperties( const char *pModifiers )
{
	if( m_Type == Node_PrefabRef )
	{
		OnPropertyChanged( m_PropList.GetAt(0), true, pModifiers );
		return;
	}

	for(uint32 nCurrProp = 0; nCurrProp < m_PropList.GetSize(); nCurrProp++)
	{
		OnPropertyChanged( m_PropList.GetAt(nCurrProp), true, pModifiers );
	}
}

#endif // DIRECTEDITOR_BUILD

//------------------------------------------------------------------------------------
//
// CWorldNode::FindParentNodes
//
// Finds the parent nodes within a set.
//
//------------------------------------------------------------------------------------
void CWorldNode::FindParentNodes( CMoArray< CWorldNode * > &nodes )
{
	uint32 i, j;
	CWorldNode *pNode, *pTestNode;

	// Find all the topmost nodes...
	for( i = 0; i < nodes.GetSize( ); i++ )
	{
		pNode = nodes[i];
		ASSERT( pNode );

		for( j = 0; j < nodes.GetSize( ); j++ )
		{
			if( i == j )
				continue;

			pTestNode = nodes[j];
			ASSERT( pTestNode );

			if( pNode->FindNodeInChildren( pTestNode ))
			{
				nodes.Remove( nodes.FindElement( pTestNode ));
				if( j < i )
					i--;
				j--;
			}
		}
	}
}

/************************************************************************/
// Finds the closest parent container node in relation to this node.  The root is returned
// if a suitable container isn't found
CWorldNode *CWorldNode::FindParentContainer()
{
	CWorldNode *pCurrentParent=GetParent();

	// Go up the node tree
	while (pCurrentParent)
	{
		// Check this node
		if (pCurrentParent->GetType() == Node_Null)
		{
			return pCurrentParent;
		}

		// Move up to the next parent
		pCurrentParent=pCurrentParent->GetParent();
	}

	// Didn't find a parent container node
	return NULL;
}

/************************************************************************/
void CWorldNode::ForceUniqueID(CWorldNode *pNode)
{
	uint32 id;

	id = pNode->GetUniqueID();

	ReleaseUniqueID(m_UniqueID);
	
	ASSERT(id < g_IDRefCounts);
	ASSERT(g_IDRefCounts[id] != 0);

	g_IDRefCounts[id]++;

	m_UniqueID = id;
}

/************************************************************************/
// Returns the upper left corner of an object/brush
CVector CWorldNode::GetUpperLeftCornerPos()
{
	// This should be called in a derived class only
	ASSERT(FALSE);

	CVector v;
	v.x=0.0f;
	v.y=0.0f;
	v.z=0.0f;

	return v;
}

void CWorldNode::SetClassName(const char* pszName)
{
	ASSERT(strlen(pszName) < TYPENAME_LEN);
	strncpy(m_TypeName, pszName, TYPENAME_LEN); 
	UpdateClassName();
}

/************************************************************************/
// Returns the name
char *CWorldNode::GetName()	
{
	if (GetType() == Node_Null)
	{
		return g_DummyString.m_String;
	}

	CStringProp *pProp;

	pProp = (CStringProp*)m_PropList.GetProp(g_NameName, TRUE);
	if(pProp)
		return pProp->m_String;
	else
		return g_DummyString.m_String;
}

/************************************************************************/
// Sets the name
void CWorldNode::SetName(const char *lpszName)
{
	CStringProp *pProp;

	pProp = (CStringProp*)m_PropList.GetProp(g_NameName, TRUE);
	if(pProp)
	{
		pProp->SetString(lpszName);
#ifdef DIRECTEDITOR_BUILD
		OnPropertyChanged(pProp, true, NULL);
#endif
	}
}

/************************************************************************/
// Returns the position
LTVector CWorldNode::GetPos()
{
	CVectorProp *pProp;
	
	pProp = (CVectorProp*)m_PropList.GetProp(g_PosName, TRUE);
	if(pProp)
		return pProp->m_Vector;
	else
		return g_DummyPos;
}

/************************************************************************/
// Sets the position
void CWorldNode::SetPos(const LTVector &v)
{
	CVectorProp *pProp;
	
	pProp = (CVectorProp*)m_PropList.GetProp(g_PosName, TRUE);
	if(pProp)
	{
		pProp->m_Vector=v;
	}				
}

/************************************************************************/
// Returns the rotation
LTVector CWorldNode::GetOr()
{ 
	CRotationProp *pProp;
	
	pProp = (CRotationProp*)m_PropList.GetProp(g_AnglesName, TRUE);
	if(pProp)
		return pProp->GetEulerAngles();
	else
		return g_DummyRotation;
}

/************************************************************************/
// Sets the rotation
void CWorldNode::SetOr(const LTVector &v)
{ 
	CRotationProp *pProp;
	
	pProp = (CRotationProp*)m_PropList.GetProp(g_AnglesName, TRUE);
	if(pProp)
	{
		pProp->SetEulerAngles(v);
#ifdef DIRECTEDITOR_BUILD
		OnPropertyChanged(pProp, true, NULL);
#endif
	}
}

/************************************************************************/
// Rotates the node about a point
void CWorldNode::Rotate(LTMatrix &mMatrix, LTVector &vCenter)
{
	LTVector vPt = GetPos();

	// Rotate point around rotation center...
	vPt -= vCenter;
	mMatrix.Apply( vPt );
	vPt += vCenter;

	SetPos(vPt);

	// below here is for updating the euler angles
	LTVector vRot = GetOr();
	LTMatrix mat;
	gr_SetupMatrixEuler( vRot, mat.m );
	mat = mMatrix * mat;

	LTVector up, right, forward;
	mat.GetBasisVectors( &right, &up, &forward );

	// recover yaw
	float yaw;
	yaw = atan2f( forward.x, forward.z );
	vRot.y = yaw;

	// recover pitch
	float base = sqrtf( (forward.x * forward.x) + (forward.z * forward.z) );
	float pitch;
	pitch = -atan2f( forward.y, base );
	vRot.x = pitch;

	// recover roll
	LTMatrix reversePitch;
	LTMatrix reverseYaw;
	reversePitch.SetupRot( LTVector(1.0f,0.0f,0.0f), -pitch );
	reverseYaw.SetupRot( LTVector(0.0f,1.0f,0.0f), -yaw );
	LTMatrix reverse = reversePitch * reverseYaw;
	up = reverse * up;
	float roll = -atan2f( up.x, up.y );
	vRot.z = roll;

	SetOr(vRot);
}
