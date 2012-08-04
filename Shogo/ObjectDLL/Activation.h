// ----------------------------------------------------------------------- //
//
// MODULE  : CActivation.h
//
// PURPOSE : CActivation class definition
//
// CREATED : 12/14/97
//
// ----------------------------------------------------------------------- //

#ifndef __ACTIVATION_H__
#define __ACTIVATION_H__

#include "cpp_aggregate_de.h"
#include "cpp_engineobjects_de.h"

// Use ADD_ACTIVATION_AGGREGATE() in your class definition to enable
// the following properties in the editor.  For example:
//
//BEGIN_CLASS(CMyCoolObj)
//	ADD_ACTIVATION_AGGREGATE()
//	ADD_STRINGPROP(Filename, "")
//  ...
//

#define ADD_ACTIVATION_AGGREGATE() \
	ADD_STRINGPROP(ActivateCondition, "")


class CActivation : public Aggregate
{
	public :

		CActivation();
		virtual ~CActivation();
	
		DBOOL	IsActive()	const { return m_bIsActive; }

	protected :

		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void Save(LPBASECLASS pObject, HMESSAGEWRITE hWrite);
		void Load(LPBASECLASS pObject, HMESSAGEREAD hRead);

		void ReadProp(LPBASECLASS pObject, ObjectCreateStruct* pStruct);
		void InitialUpdate(LPBASECLASS pObject, DFLOAT fInfo);

		HSTRING m_hstrActivateCondition;
		DBOOL	m_bIsActive;
};

#endif // __ACTIVATION_H__

