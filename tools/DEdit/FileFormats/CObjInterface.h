// CObjInterface.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegionView view


#ifndef __COBJINTERFACE_H__
#define __COBJINTERFACE_H__


	#include "editgrid.h"
	#include "editpoly.h"
	#include "editray.h"
	#include "navigator.h"
	#include "editbrush.h"
	#include "refs.h"
	#include "viewrender.h"
	#include "trackers\trackermgr.h"

	class CEditRegion;
	class CEditPoly;
	/************************************************************************/
	class CObjInterface
	{
	public:
		CObjInterface(void)					
		{ 
			m_vertexList.SetCacheSize(4096);
			m_texCoordList.SetCacheSize(4096);
			m_faceSetList.SetCacheSize(4096);
												
			m_flag = RAW; 
			m_from = F3DSMAX;
		}
		~CObjInterface(void)				{ ; }
	public:
		void	ImportObj(FILE* file, int flag = CObjInterface::RAW, int from = F3DSMAX);
		void	ExportObj(FILE* file, int flag = CObjInterface::RAW);
		int		FileSize( FILE* file );
		void	FileCopyToBuffer(FILE* file, char* buffer);
		void	FileGetLine(char* line, int n, FILE* file);
	public:
		enum
		{
			RAW,
			REVERSE,
			F3DSMAX,	// from 3DSMax
			FMAYA,		// from Maya
			FSOFT,		// from SOFT
		};

		CMoArray< LTVector >			m_vertexList;
		CMoArray< LTVector >			m_texCoordList;
		CMoArray< CMoArray< int > >		m_faceSetList;
		int								m_flag;
		int								m_from;

	private:
		void	ImportObjVertices( FILE* file );
		void	ImportObjTexCoords( FILE* file );
		void	ImportObjNormals( FILE* file );
		void	ImportObjObjects( FILE* file );
	};
	
#endif  // __COBJINTERFACE_H__




