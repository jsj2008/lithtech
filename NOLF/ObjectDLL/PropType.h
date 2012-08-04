// ----------------------------------------------------------------------- //
//
// MODULE  : PropType.h
//
// PURPOSE : Model PropType - Definition
//
// CREATED : 4/26/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROP_TYPE_H__
#define __PROP_TYPE_H__

#include "Prop.h"
#include "PropTypeMgr.h"

class PropType : public Prop
{
	public :

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	private :

		void ReadProp(ObjectCreateStruct *pData);
};

class CPropTypePlugin : public CPropPlugin
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

  protected :

	  	  CPropTypeMgrPlugin m_PropTypeMgrPlugin;
};

#endif // __PROP_H__