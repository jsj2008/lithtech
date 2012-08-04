// ----------------------------------------------------------------------- //
//
// MODULE  : TextureFX.h
//
// PURPOSE : Handlers for texture effect parameters
//
// CREATED : 10/03/01 
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEXTUREFX_H__
#define __TEXTUREFX_H__

LINKTO_MODULE( TextureFX );

#include "GameBase.h"

class TextureFX : public GameBase
{
public:
	enum {	NUM_STAGES	= 2,
			NUM_VARS	= 6
		};


	TextureFX();
	virtual ~TextureFX();
protected:
	uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	LTBOOL ReadProp(ObjectCreateStruct *pOCS);
	LTBOOL PostReadProp(ObjectCreateStruct *pOCS);

	bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
	
private:
    void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
    void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
	void InitialUpdate();

	// Send the current state of the light to the clients
	void UpdateClients();

	//writes the stage information to a specified message 
	void WriteStageInfo(ILTMessage_Write& Msg) const;


	class CStage
	{
	public:
		uint32		m_nID;
		uint32		m_nChanged;
		float		m_fVars[NUM_VARS];
	};

	//the stages
	CStage	m_Stages[NUM_STAGES];

	// Are we waiting to send the client an update?
	bool m_bClientNeedsUpdate;
};


#endif //__LIGHTGROUP_H__