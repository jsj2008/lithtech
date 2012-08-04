// ----------------------------------------------------------------------- //
//
// MODULE  : KeyItem.h
//
// PURPOSE : An KeyItem object
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __KEY_ITEM_H__
#define __KEY_ITEM_H__

#include "ltengineobjects.h"
#include "Prop.h"
#include "KeyMgr.h"

#define KEY_MAX_CONTROLOBJS	20

LINKTO_MODULE( KeyItem );

class KeyItem : public Prop
{
	public :

		KeyItem();
		~KeyItem();

		uint16	GetKeyId() const { return m_nKeyId;	}

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void	ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);

	private :

		void	DoActivate(HOBJECT hSender);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		HSTRING		m_hstrPickedUpCmd;	// Command to send when key item is picked up / photographed

		uint16		m_nKeyId;

		LTBOOL		m_bStartHidden;
		LTBOOL		m_bFirstUpdate;

		LTBOOL		m_bSkipUpdate; 

        void InitialUpdate();
        LTBOOL Update();
		void AllObjectsCreated();

		HSTRING		m_hstrCtrlObjName[KEY_MAX_CONTROLOBJS];

};

#ifndef __PSX2
class CKeyItemPlugin : public CPropPlugin
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

	virtual LTRESULT PreHook_PropChanged(
			const char *szObjName,
			const char *szPropName,
			const int nPropType,
			const GenericProp &gpPropValue,
			ILTPreInterface *pInterface,
			const char *szModifiers );

  protected :

	  	  CKeyMgrPlugin m_KeyMgrPlugin;
		  CCommandMgrPlugin m_CommandMgrPlugin;
};
#endif

#endif // __KEY_ITEM_H__