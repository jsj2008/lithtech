//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : WadTypes.h
//
//	PURPOSE	  : Defines data structures used during WAD file loading.
//
//	CREATED	  : October 24 1996
//
//
//------------------------------------------------------------------

#ifndef __WADTYPES_H__
#define __WADTYPES_H__

	
	#pragma pack(1)


	class CWadDirEntry
	{
		public:

			DWORD		m_ResPos;
			DWORD		m_ResLen;
			char		m_ResName[8];
	
	};

	class CDoomVertex
	{
		public:
			
			SWORD		x, y;

	};

	class CDoomLinedef
	{
		public:

			short		m_From;
			short		m_To;
			short		m_Attribs;
			short		m_Type;
			short		m_Tag;
			short		m_RightSidedef;
			short		m_LeftSidedef;

	};

	class CDoomSidedef
	{
		public:

			WORD		m_xOffset;
			WORD		m_yOffset;

			char		m_UpperTex[8];
			char		m_LowerTex[8];
			char		m_NormalTex[8];

			SWORD		m_Sector;

	};

	class CDoomSector
	{
		public:

			SWORD		m_FloorHeight;
			SWORD		m_CeilHeight;

			char		m_FloorTex[8];
			char		m_CeilTex[8];

			SWORD		m_LightLevel;
			SWORD		m_SpecialSector;
			SWORD		m_Tag;

	};


	#pragma pack()


#endif  // __WADTYPES_H__

