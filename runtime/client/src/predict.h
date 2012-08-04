
// Defines the main motion prediction routines.
#ifndef __PREDICT_H__
#define __PREDICT_H__


class CClientShell;
class LTObject;

// Called the first time we get a server update packet.
void pd_InitialServerUpdate(CClientShell *pShell, float gameTime);

// Called when an object's position comes in an update packet.	
void pd_OnObjectMove(CClientShell *pShell, LTObject *pObject, LTVector *pNewPos, LTVector *pNewVel, bool bNew, bool bTeleport);

// Called when an object's position comes in an update packet.	
void pd_OnObjectRotate(CClientShell *pShell, LTObject *pObject, LTRotation *pNewRot, bool bNew, bool bSnap);

// Called every frame after reading network input.
void pd_Update(CClientShell *pShell);


#endif  // __PREDICT_H__



