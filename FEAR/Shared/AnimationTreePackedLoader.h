// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationTreePackedLoader.h
//
// PURPOSE : AnimationTreePackedLoader class definition
//           Load a packed search tree from disk.
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __ANIMATION_TREE_PACKED_LOADER_H__
#define __ANIMATION_TREE_PACKED_LOADER_H__


//
// Forward declarations.
//

class	CAnimationTreePacked;


//
// CAnimationTreePackedLoader
//

class CAnimationTreePackedLoader
{
	public:
	
		 CAnimationTreePackedLoader();
		~CAnimationTreePackedLoader();

		bool	LoadAnimationTreePacked( CAnimationTreePacked* pAnimTree, const char* pszFilename );

	private:

		void	SetupAnimTreeData( CAnimationTreePacked* pAnimTree, uint32 nDataSize );
		void	FixUpAnimTreeData( CAnimationTreePacked* pAnimTree, const char* pszFilename );
};

#endif
