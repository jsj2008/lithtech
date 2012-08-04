	
// This header file includes all the server-side DirectEngine header files.
#ifndef __DE_SERVER_H__
#define __DE_SERVER_H__

	#include "basedefs_de.h"

	#ifdef COMPILE_WITH_C
		#include "engineobjects_de.h"
		#include "server_de.h"
		#include "servershell_de.h"
	#else
		#include "cpp_engineobjects_de.h"
		#include "cpp_servershell_de.h"
	#endif

	#include "serverobj_de.h"


#endif  // __DE_SERVER_H__


