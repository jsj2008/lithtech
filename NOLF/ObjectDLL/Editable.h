// ----------------------------------------------------------------------- //
//
// MODULE  : CEditable.h
//
// PURPOSE : CEditable class definition
//
// CREATED : 3/10/99
//
// ----------------------------------------------------------------------- //

#ifndef __EDITABLE_H__
#define __EDITABLE_H__

#include "iaggregate.h"
#include "ltengineobjects.h"
#include "TemplateList.h"

class CPropDef
{
	public :

		CPropDef();
		~CPropDef();

		enum	PropType { PT_UNKNOWN_TYPE, PT_FLOAT_TYPE, PT_DWORD_TYPE, PT_BYTE_TYPE, PT_BOOL_TYPE, PT_VECTOR_TYPE };

        LTBOOL   Init(char* pName, PropType eType, void* pAddress);
		char*	GetPropName();
        LTBOOL   GetStringValue(CString & str);

        LTBOOL   SetValue(char* pPropName, char* pValue);

	private :

		HSTRING		m_strPropName;
		PropType	m_eType;
		void*		m_pAddress;

        LTBOOL   GetFloatValue(LTFLOAT & fVal);
        LTBOOL   GetDWordValue(uint32 & dwVal);
        LTBOOL   GetByteValue(uint8 & nVal);
        LTBOOL   GetBoolValue(LTBOOL & bVal);
        LTBOOL   GetVectorValue(LTVector & vVal);
};


class CEditable : public IAggregate
{
	public :

		CEditable();
		virtual ~CEditable();

        void AddFloatProp(char* pPropName, LTFLOAT* pPropAddress);
        void AddDWordProp(char* pPropName, uint32* pPropAddress);
        void AddByteProp(char* pPropName, uint8* pPropAddress);
        void AddBoolProp(char* pPropName, LTBOOL* pPropAddress);
        void AddVectorProp(char* pPropName, LTVector* pPropAddress);

	protected :

        uint32 ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void	TriggerMsg(LPBASECLASS pObject, HOBJECT hSender, const char* szMsg);
		void	EditProperty(LPBASECLASS pObject, char* pPropName, char* pPropValue);
		void	ListProperties(LPBASECLASS pObject);

	private :

		CTList<CPropDef*>		m_propList;
};

#endif // __EDITABLE_H__