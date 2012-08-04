// ----------------------------------------------------------------------- //
//
// MODULE  : Intelligence.h
//
// PURPOSE : An Intelligence object
//
// CREATED : 9/14/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTELLIGENCE_H__
#define __INTELLIGENCE_H__

#include "ltengineobjects.h"
#include "Prop.h"
#include "IntelMgr.h"

LINKTO_MODULE( Intelligence );

class Intelligence : public Prop
{
	public :

		Intelligence();
		~Intelligence();

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void	ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);

	private :

		void	HandleGadgetMsg(HOBJECT hSender);
		void	DoActivate(HOBJECT hSender);

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		LTBOOL		m_bShowPopup;
        LTBOOL		m_bPhotoOnly;
		HSTRING		m_hstrPickedUpCmd;	// Command to send when intelligence is picked up / photographed

		uint8		m_nIntelId;
		uint32		m_nTextId;
		uint8		m_nPopupId;

		LTBOOL		m_bIsIntel;
		LTBOOL		m_bAddToList;

		LTBOOL		m_bStartHidden;
        LTFLOAT		m_fRespawnDelay;    // How quickly we respawn
		HSTRING		m_hstrRespawnSnd;	// Sound to play when item respawns
		HSTRING		m_hstrPickupSnd;	// Sound to play when item is pickedup

		LTBOOL		m_bSkipUpdate; 

        void InitialUpdate();
        LTBOOL Update();

};

#ifndef __PSX2
class CIntelPlugin : public CPropPlugin
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

	  	  CIntelMgrPlugin m_IntelMgrPlugin;
		  CCommandMgrPlugin m_CommandMgrPlugin;
};
#endif

#endif // __INTELLIGENCE_H__