// ----------------------------------------------------------------------- //
//
// MODULE  : NamedObjectList.h
//
// PURPOSE : Class definition for an aggregate used to include a list
//           of named objects as a member of another object.
//
// CREATED : 07/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _NAMED_OBJECT_LIST_H_
#define _NAMED_OBJECT_LIST_H_

//
// Includes...
//

#include "iaggregate.h"


//
// Defines...
//

#define ADD_NAMED_OBJECT_LIST_AGGREGATE( GroupName, Group, Object, GroupDesc, PropDesc ) \
	PROP_DEFINEGROUP( GroupName, Group, GroupDesc ) \
		ADD_STRINGPROP_FLAG(Object##1, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##2, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##3, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##4, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##5, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##6, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##7, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##8, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##9, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##10, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##11, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##12, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##13, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##14, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##15, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##16, "", Group|PF_OBJECTLINK, PropDesc) \

#define ADD_NAMED_OBJECT_LIST_AGGREGATE0( GroupName, Group, Object, GroupDesc, PropDesc ) \
	PROP_DEFINEGROUP( GroupName, Group, GroupDesc ) \
		ADD_STRINGPROP_FLAG(Object##0, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##1, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##2, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##3, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##4, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##5, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##6, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##7, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##8, "", Group|PF_OBJECTLINK, PropDesc) \
		ADD_STRINGPROP_FLAG(Object##9, "", Group|PF_OBJECTLINK, PropDesc) \

//
// Typedefs...
//

typedef std::vector< LTObjRef, LTAllocator<LTObjRef, LT_MEM_TYPE_OBJECTSHELL> > HOBJECT_LIST;


class CNamedObjectList : public IAggregate
{
	public :

		// Ctors/Dtors/etc

		CNamedObjectList();
		virtual ~CNamedObjectList();

		// Engine calls to the aggregate...

		uint32			EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float lData);

		void			ReadProp(const GenericPropList *pProps, const char* pszObjectType);

		// Initialization

		void			InitNamedObjectList( HOBJECT hOwner );

		// Save/Load

		void			Save(ILTMessage_Write *pMsg, uint8 nType);
		void			Load(ILTMessage_Read *pMsg, uint8 nType);

		// Query.

		bool				DoesContain( HOBJECT hObject );

		// Data access

		void				ClearStrings() { m_lstObjectNames.resize( 0 ); }
		void				ClearObjectHandle( uint32 iIndex );
		uint32				GetNumObjectNames() const { return m_lstObjectNames.size(); }
		uint32				GetNumObjectHandles() const { return m_lstObjectHandles.size(); }
		const char*			GetObjectName( uint32 iIndex ) const;
		HOBJECT				GetObjectHandle( uint32 iIndex ) const;
		const HOBJECT_LIST*	GetObjectHandles() const { return &m_lstObjectHandles; }	

	protected :

		LTObjRef		m_hObject;
		StringArray		m_lstObjectNames;
		HOBJECT_LIST	m_lstObjectHandles;
};

//-----------------------------------------------------------------

#endif // _NAMED_OBJECT_LIST_H_
