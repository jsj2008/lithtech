
#ifndef __CREATEPHYSICSBSP_H__
#define __CREATEPHYSICSBSP_H__

	class CEditRegion;
	class CWorldModelDef;
	class CPreWorld;

	// Generate the physics BSP for the world.
	bool CreatePhysicsBSP(
		CEditRegion		*pRegion,
		CWorldModelDef	*pWorldModel,
		CPreWorld		*pWorld,
		bool			bMainWorld);


#endif




