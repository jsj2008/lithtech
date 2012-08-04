// ----------------------------------------------------------------------- //
//
// MODULE  : SearchProp.h
//
// PURPOSE : Searchable Prop - Definition
//
// CREATED : 12/21/2001
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCH_PROP_H__
#define __SEARCH_PROP_H__

#include "PropType.h"
#include "Searchable.h"

LINKTO_MODULE( SearchProp );


class SearchProp : public PropType
{
	public :
		SearchProp();

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

		CSearchable		m_search;


};



#ifndef __PSX2
class CSearchPropPlugin : public CPropTypePlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims);
  
  protected:

	  CSearchItemPlugin		m_SearchItemPlugin;


};
#endif


#endif // __SEARCH_PROP_H__