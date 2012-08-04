
#include "bdefs.h"
#include "de_world.h"
#include "de_objects.h"
#include "geomroutines.h"
#include "ltbasedefs.h"


void obj_SetupWorldModelTransform(WorldModelInstance *pWorldModel)
{
	WorldBsp *pWorldBsp;

	pWorldBsp = (WorldBsp*)pWorldModel->m_pOriginalBsp;
	gr_SetupWMTransform(
		&pWorldBsp->m_WorldTranslation,
		&pWorldModel->GetPos(),
		&pWorldModel->m_Rotation,
		&pWorldModel->m_Transform,
		&pWorldModel->m_BackTransform);
}
