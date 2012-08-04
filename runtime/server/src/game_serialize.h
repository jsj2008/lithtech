
// This module defines all the server routines to load and save the game
// and object state.

#ifndef __GAME_SERIALIZE_H__
#define __GAME_SERIALIZE_H__

class CServerMgr;
struct ObjectList;
class ILTStream;

// Save object states into a stream.
void sm_SaveObjects(ILTStream *pStream, ObjectList *pList, uint32 dwParam, uint32 flags);

// Restore an object list from a stream.
LTRESULT sm_RestoreObjects(ILTStream *pStream, uint32 dwParam, uint32 flags);


#endif  // __GAME_SERIALIZE_H__







