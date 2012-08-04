// ----------------------------------------------------------------------- //
//
// MODULE  : Intelligence.h
//
// PURPOSE : An Intelligence object
//
// CREATED : 9/14/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTELLIGENCE_H__
#define __INTELLIGENCE_H__

#include "ltengineobjects.h"
#include "Prop.h"

class Intelligence : public Prop
{
	public :

		Intelligence();
		~Intelligence();

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void	ReadProp(ObjectCreateStruct *pData);
		void	PostPropRead(ObjectCreateStruct* pData);

	private :

		void	HandleGadgetMsg(HOBJECT hSender, ConParse & parse);
		void	DoActivate(HOBJECT hSender);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		LTBOOL		m_bShowPopup;
        LTBOOL		m_bPhotoOnly;
		HSTRING		m_hstrPickedUpCmd;	// Command to send when intelligence is picked up / photographed

		uint8		m_nPlayerTeamFilter;
		uint32		m_nIntelId;
		uint32		m_nInfoId;

		LTBOOL		m_bStartHidden;
		LTBOOL		m_bFirstUpdate;
        LTFLOAT		m_fRespawnDelay;            // How quickly we respawn
		HSTRING		m_hstrRespawnSoundFile;		// Sound to play when item respawns


		LTBOOL		m_bSkipUpdate; 

        void InitialUpdate();
        void CacheFiles();
        LTBOOL Update();

};

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

  protected :

	  	  CIntelMgrPlugin m_IntelMgrPlugin;
};

#endif // __INTELLIGENCE_H__