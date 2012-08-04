//////////////////////////////////////////////////////////////////////////////
// PC-Specific rendering vertex implementation

#include "bdefs.h"

#include "pcrendervert.h"
#include "pcfileio.h"

CAbstractIO &operator<<(CAbstractIO &file, const CPCRenderVert2T &cVert)
{
	file << cVert.m_vPos;
	file << cVert.m_fU0;
	file << cVert.m_fV0;
	file << cVert.m_fU1;
	file << cVert.m_fV1;
	file << cVert.m_nColor;
	file << cVert.m_vNormal;
	file << cVert.m_vTangent;
	file << cVert.m_vBinormal;
	return file;
}
