#ifndef __RENDERSTYLELOOKUPTABLES_H__
#define __RENDERSTYLELOOKUPTABLES_H__

#ifndef __LTRENDERSTYLE_H__
#	include "ltrenderstyle.h"
#endif

//a utility class that handles managing all the lookup tables required to remove the logic from the
//setting of render styles

class CRenderStyleLookupTables
{
public:

	//constructs the object and initializes all of the tables
	CRenderStyleLookupTables();

	//conversion tables for the color and alpha operations
	uint32	m_nColorOp[RENDERSTYLE_COLOROP_TYPE_COUNT];
	uint32	m_nAlphaOp[RENDERSTYLE_ALPHAOP_TYPE_COUNT];

	//conversion tables for the argument types
	uint32	m_nColorArg[RENDERSTYLE_COLORARG_TYPE_COUNT];
	uint32	m_nAlphaArg[RENDERSTYLE_ALPHAARG_TYPE_COUNT];

	//conversions for fill and cull modes
	uint32	m_nFillMode[RENDERSTYLE_FILLMODE_TYPE_COUNT];
	uint32	m_nCullMode[RENDERSTYLE_CULLMODE_TYPE_COUNT];

	//conversions fo Z buffer type
	bool	m_bZWriteEnable[RENDERSTYLE_ZBUFFERMODE_TYPE_COUNT];
	bool	m_bZReadEnable[RENDERSTYLE_ZBUFFERMODE_TYPE_COUNT];

	//blend mode
	uint32	m_nSrcBlendMode[RENDERSTYLE_BLENDMODE_TYPE_COUNT];
	uint32	m_nDstBlendMode[RENDERSTYLE_BLENDMODE_TYPE_COUNT];

	//test modes
	uint32	m_nAlphaTest[RENDERSTYLE_TESTMODE_TYPE_COUNT];

	//UV source parameters
	uint32	m_nUVSource[RENDERSTYLE_UVSOURCE_TYPE_COUNT];
	bool	m_bUVSourceStage[RENDERSTYLE_UVSOURCE_TYPE_COUNT];
	bool	m_bUVSourceWorldSpace[RENDERSTYLE_UVSOURCE_TYPE_COUNT];

	//texture addressing
	uint32	m_nAddress[RENDERSTYLE_UVADDRESS_TYPE_COUNT];

	//texture filtering
	uint32	m_nMinFilter[RENDERSTYLE_TEXFILTER_TYPE_COUNT];
	uint32	m_nMagFilter[RENDERSTYLE_TEXFILTER_TYPE_COUNT];
	uint32	m_nMipFilter[RENDERSTYLE_TEXFILTER_TYPE_COUNT];
};

#endif
