
// This file contains all the build options.  Sometimes the makefile is just
// setup to set the options automatically, but the option should be listed in here.

#ifndef __BUILDOPTIONS_H__
#define __BUILDOPTIONS_H__


// Define this to have it track memory usage.  Takes 4 bytes extra per
// memory allocation.  It always tracks allocation counts, and is currently
// set to track full memory usage in debug builds.
#ifdef _DEBUG
//	#define TRACK_MEMORY_USAGE	1
#endif


#endif  // __BUILDOPTIONS_H__



