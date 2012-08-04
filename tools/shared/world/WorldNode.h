//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : WorldNode.h
//
//	PURPOSE	  : Defines the CWorldNode class.
//
//	CREATED	  : January 19 1996
//
//
//------------------------------------------------------------------

#ifndef __WORLDNODE_H__
#define __WORLDNODE_H__

#ifdef DIRECTEDITOR_BUILD
#	include "commctrl.h"
#endif

#include "proplist.h"

// World Node stuff.
typedef enum
{

	Node_Null,
	Node_Brush,
	Node_Object,
	Node_PrefabRef

} WorldNodeType;



#define TYPENAME_LEN			30


#define INVALID_NODE_ID		((uint32)0xFFFFFFFF)


// Selection modes.
#define		SINGLE_SELECTION		0
#define		MULTI_SELECTION			1


// Node flags.
enum ENodeFlags
{
	NODEFLAG_SELECTED		= (1 << 0),
	NODEFLAG_HIDDEN			= (1 << 1),
	NODEFLAG_PATH			= (1 << 2),
	NODEFLAG_WORLDROOT		= (1 << 3),
	NODEFLAG_EXPANDED		= (1 << 4),
	NODEFLAG_FROZEN			= (1 << 5),
	NODEFLAG_SHOWMODEL		= (1 << 6),
	NODEFLAG_USER			= (1 << 7)
};


class CEditBrush;
class CBaseEditObj;

class CWorldNode : public CGLLNode
{
	public:
		enum					{ MAX_LABEL=64 };

								CWorldNode();
		virtual					~CWorldNode();

		virtual void			Term();
		virtual CWorldNode*		AllocateSameKind()	{return new CWorldNode;}
		virtual void			DoCopy(CWorldNode *pOther);


		void					RemoveFromTree();
		void					RemoveChild( CWorldNode *pChild );

		BOOL					FindNodeInChildren( CWorldNode *pNode );


		// Return the node type
		WorldNodeType			GetType( )	const	{ return m_Type; }			
		
		char					*GetName();						// Returns the name			
		void					SetName(const char *lpszName);	// Sets the name

		virtual LTVector		GetPos();						// Returns the position
		virtual void			SetPos(const LTVector &v);		// Sets the position
		virtual LTVector		GetOr();						// Returns the rotation
		virtual void			SetOr(const LTVector &v);		// Sets the rotation

		// Rotate this node about a point using a rotation matrix
		// Note : Both of these parameters should be const, but LTMatrix is non-const friendly
		virtual	void			Rotate(LTMatrix &mMatrix, LTVector &vCenter);

		// Returns the class type
		const char*				GetClassName() const				{ return m_TypeName; }
		void					SetClassName(const char* pszName);

		//this function must be called whenever the class name is updated
		virtual void			UpdateClassName()					{}
		
		//gets the whole flag variable
		uint32					GetFlags( )					const	{ return m_Flags; }

		//sets the whole flag variable
		void					SetFlags(uint32 dwFlags)			{ m_Flags=dwFlags; }

		//determines if a specific flag is set 
		BOOL					IsFlagSet( uint32 dwFlag )	const	{ return m_Flags & dwFlag; }

		//clears the specified flag
		void					ClearFlag(uint32 dwFlag)			{ m_Flags &= ~dwFlag; }

		//turns on the specified flag
		void					EnableFlag(uint32 dwFlag)			{ m_Flags |= dwFlag; }

		//will either disable or enable the specified flag based upon the boolean
		void					SetFlag(uint32 dwFlag, bool bVal)	{ if(bVal) { EnableFlag(dwFlag); } else { ClearFlag(dwFlag); } }

		//toggles the specified flag
		void					ToggleFlag(uint32 dwFlag)			{ SetFlag(dwFlag, !IsFlagSet(dwFlag)); }
		
		const char *			GetNodeLabel( )				const	{ return m_lpszLabel; }
		void					SetNodeLabel( const char *szLabel );

#ifdef DIRECTEDITOR_BUILD

		//handles toggling the viewing of the model
		void					ShowModel(BOOL bShowModel, BOOL bIncludeChildren);

		void					SetItem( HTREEITEM hItem )			{ m_hItem = hItem; }
		HTREEITEM				GetItem( )					const	{ return m_hItem; }

		// Hide/Show nodes
		void					HideNode(BOOL bHideChildren=FALSE);
		void					ShowNode(BOOL bShowChildren=FALSE);
		void					ToggleHideNode(BOOL bHideChildren=FALSE);

		//notification that all properties have been changed
		void					RefreshAllProperties( const char *pModifiers );

		// Notification that a property of this object has changed
		virtual void			OnPropertyChanged(CBaseProp* pProperty, bool bNotifyGame, const char *pModifiers ) {}

#endif // DIRECTEDITOR_BUILD

		void					SetParent( CWorldNode *pNode )		{ m_pParent = pNode; }
		CWorldNode *			GetParent( )				const	{ return m_pParent; }
		static void				FindParentNodes( CMoArray< CWorldNode * > &nodes );

		// Finds the closest parent container node in relation to this node.  The root is returned
		// if a suitable container isn't found.
		CWorldNode				*FindParentContainer();

		// Get/Set its unique ID.
		uint32					GetUniqueID() {return m_UniqueID;}
		
		// Forces the ID to be the one from pNode.. highly unrecommended.. this
		// is only used in the undo stuff in which new node IDs must match their
		// old ID.
		void					ForceUniqueID(CWorldNode *pNode);
	

		// Returns the upper left corner of the node as it would appear in DEdit.
		// For objects this is the position and for brushes this is the upper left of the
		// bounding box for the brush.
		virtual CVector			GetUpperLeftCornerPos();

		// Returns the property list
		CPropList				*GetPropertyList()					{ return &m_PropList; }			

		CEditBrush*				AsBrush() const
		{
			ASSERT(GetType() == Node_Brush);
			return (CEditBrush*)this;
		}

		CBaseEditObj*			AsObject() const
		{
			ASSERT(GetType() == Node_Object);
			return (CBaseEditObj*)this;
		}

		CWorldNode*				AsNode()
		{
			return this;
		}

	public:

		// All the properties and the type name for this node.
		CPropList				m_PropList;


		CGLinkedList<CWorldNode*>	m_Children;

		// Tells what type of node this is.
		WorldNodeType			m_Type;

		// Flags on this node.
		uint32					m_Flags;

#ifdef DIRECTEDITOR_BUILD
		HTREEITEM				m_hItem;
#endif // DIRECTEDITOR_BUILD

	protected:
		// The label
		char					*m_lpszLabel;

	private:
		char					m_TypeName[TYPENAME_LEN + 1];

		uint32					m_UniqueID;

		CWorldNode				*m_pParent;

		void					CommonConstructor( );
};
#endif  // __WORLDNODE_H__

