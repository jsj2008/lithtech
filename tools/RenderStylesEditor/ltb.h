// ltb.h 
// header for ltb files.
#ifndef __LTB_H__
#define __LTB_H__

// File types...
enum E_LTB_FILE_TYPES {
	LTB_D3D_MODEL_FILE			= 1,
	LTB_PS2_MODEL_FILE			= 2,
	LTB_XBOX_MODEL_FILE			= 3,
	LTB_ABC_MODEL_FILE			= 4,
	LTB_D3D_RENDERSTYLE_FILE	= 5,
	LTB_PS2_RENDERSTYLE_FILE    = 6,
	LTB_D3D_RENDEROBJECT_FILE	= 7
};

// LTB_Header should be the first thing in all TLB Files...
struct LTB_Header {
	LTB_Header()							{ m_iFileType = 0; m_iVersion = 0; }
	uint8  m_iFileType;						// Should be one of E_LTB_FILE_TYPES
	uint16 m_iVersion;						// Version number...
	uint8  m_iReserved1;					// Reserved for stuff we thing of later...
	uint32 m_iReserved2;
	uint32 m_iReserved3;
	uint32 m_iReserved4;
};

#endif
