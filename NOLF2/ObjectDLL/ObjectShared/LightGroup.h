// ----------------------------------------------------------------------- //
//
// MODULE  : LightGroup.h
//
// PURPOSE : Grouping objects for lighting animations
//
// CREATED : 10/03/01 
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTGROUP_H__
#define __LIGHTGROUP_H__

LINKTO_MODULE( LightGroup );



class LightGroup : public Engine_LightGroup
{
public:
	LightGroup();
	virtual ~LightGroup();
protected:
	uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
	uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

	LTBOOL ReadProp(ObjectCreateStruct *pOCS);
	LTBOOL PostReadProp(ObjectCreateStruct *pOCS);

	void HandleTrigger( HOBJECT hSender, ILTMessage_Read *pMsg );
	
private:
    void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
    void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
	void InitialUpdate();

	// Send the current state of the light to the clients
	void UpdateClients();

	// The lightgroup's ID
	uint32 m_nID;

	// The current color, relative to the starting color
	LTVector m_vColor;
	// Is the light on?
	bool m_bOn;

	// Are we waiting to send the client an update?
	bool m_bClientNeedsUpdate;
};


#endif //__LIGHTGROUP_H__