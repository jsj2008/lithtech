#ifndef __INTERLINK_H__
#define __INTERLINK_H__


class CServerMgr;


// Link types.
#define LINKTYPE_INTERLINK  0   // InterLinks.
#define LINKTYPE_CONTAINER  1   // Container->object and object->container.
#define LINKTYPE_SOUND      2   // object->sound


// An actual interlink.
struct InterLink
{
    uint32          m_Type; // Link type.

    LTObject        *m_pOwner;
    void            *m_pOther;

    LTLink          *m_pOwnerLink;
    LTLink          *m_pOtherLink;
};


// Disconnects and frees all links between two objects without notifying 
// the objects of the links being broken.
void DisconnectLinks(LTObject *pObject, void *pOther, LTBOOL bDisconnectAll=LTTRUE, LTBOOL bNotify=LTFALSE);

// Disconnects all of pServerMgr's links, notifying the owner of each of the link breaking.
void BreakInterLinks(LTObject *pObj, uint32 linkType, LTBOOL bNotify);

// Create an inter-link between 2 objects.
LTRESULT CreateInterLink(LTObject *pOwner, void *pOther, uint32 linkType);

#endif


