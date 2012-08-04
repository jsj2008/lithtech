//------------------------------------------------------------------
//
//	FILE	  : StdLith.h
//
//	PURPOSE	  : Includes all the useful files for your
//              programming pleasure.
//
//	CREATED	  : August 20 1996
//
//------------------------------------------------------------------

#ifndef __STDLITH_H__
	#define __STDLITH_H__


	// Includes....
	#include "StdLithDefs.h"
	#include "LithException.h"
	#include "Memory.h"

	#include "DynArray.h"
	
	#include "LinkList.h"
	#include "GoodLinkList.h"
	#include "MultiLinkList.h"
	#include "FastLinkList.h"
	#include "glink.h"

	#include "StringHolder.h"
	#include "MoRect.h"

	#include "AbstractIO.h"
	#include "FileIO.h"
	#include "MemoryIO.h"
	#include "CompressedIO.h"

	#include "ObjectBank.h"
	#include "object_bank.h"
	#include "struct_bank.h"

	#include "Helpers.h"



	#ifdef _DEBUG
		#pragma comment (lib, "StdLith.lib")
	#else
		#pragma comment (lib, "StdLith.lib")
	#endif


#endif
