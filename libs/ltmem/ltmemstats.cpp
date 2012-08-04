// ltmemstats.cpp 
//

#include "stdafx.h"
#include "bdefs.h"
#include "ltmem.h"
#include "ltmemheap.h"
#include "ltmemtrack.h"

// true if lt mem system is initialized
extern bool g_bLTMemInitialized;


// display console help
void LTMemConsoleHelp()
{
	dsi_ConsolePrint("\nltmem subsystem console commands\n\n");
	dsi_ConsolePrint("usage : mem <command> [parameters]\n\n");
	dsi_ConsolePrint("help - this help message\n");
	dsi_ConsolePrint("stats - current memory statistics\n");
	dsi_ConsolePrint("mark - marks all memory currently in the heap\n");
	dsi_ConsolePrint("unmark - unmarks all memory currently in the heap\n");
	dsi_ConsolePrint("dump - dumps the marked memory on the heap\n");
	dsi_ConsolePrint("log - logs all current allocations out to a file of the specified name in csv format\n");
	dsi_ConsolePrint("fulllog - creates a log similar to log, but doesn't collapse the allocations from the same line\n");
	dsi_ConsolePrint("ignore - ignore the currently marked memory (clears the marked flag)\n");
	dsi_ConsolePrint("\n");
}

// mark heap
void LTMemConsoleMark()
{
	uint32 nAllocationCount = 0;
	CMemTrackAllocInfo* pFinger = g_pTrackHeap;
	while (pFinger)
	{
		if ((pFinger->m_nFlags & CMemTrackAllocInfo::k_nFlag_Ignore) == 0)
		{
			pFinger->m_nFlags |= CMemTrackAllocInfo::k_nFlag_Marked;
			++nAllocationCount;
		}
		pFinger = pFinger->m_pNext;
	}

	dsi_ConsolePrint("%d blocks marked", nAllocationCount);
}


// un-mark heap
void LTMemConsoleUnMark()
{
	uint32 nMarkedCount = 0;
	CMemTrackAllocInfo* pFinger = g_pTrackHeap;
	while (pFinger)
	{
		if ((pFinger->m_nFlags & CMemTrackAllocInfo::k_nFlag_Marked) != 0)
		{
			pFinger->m_nFlags &= ~CMemTrackAllocInfo::k_nFlag_Marked;
			++nMarkedCount;
		}
		pFinger = pFinger->m_pNext;
	}

	dsi_ConsolePrint("%d blocks un-marked", nMarkedCount);
}


// mark heap
void LTMemConsoleDump()
{
	uint32 nMarkedCount = 0;
	uint32 nMarkedSize = 0;
	uint32 nKnownCount = 0;
	uint32 nKnownSize = 0;
	CMemTrackAllocInfo* pFinger = g_pTrackHeap;
	while (pFinger)
	{
		if (pFinger->m_nFlags & CMemTrackAllocInfo::k_nFlag_Marked)
		{
			bool bIsUnknown = 
				(pFinger->m_sFileName[0] == 0) ||
				(pFinger->m_nAllocationType == LT_MEM_TYPE_UNKNOWN);
			if (!bIsUnknown)
			{
				dsi_ConsolePrint("%s:%d, %d bytes", pFinger->m_sFileName, pFinger->m_nLineNum, pFinger->m_nRequestedSize);
				++nKnownCount;
				nKnownSize += pFinger->m_nRequestedSize;
			}
			++nMarkedCount;
			nMarkedSize += pFinger->m_nRequestedSize;
		}
		pFinger = pFinger->m_pNext;
	}

	dsi_ConsolePrint("%d (%dk) marked, %d (%dk) known, %d (%dk) unknown", 
		nMarkedCount, nMarkedSize / 1024, nKnownCount, nKnownSize / 1024,
		(nMarkedCount - nKnownCount), (nMarkedSize - nKnownSize) / 1024);
}

// Ignore marked memory
void LTMemConsoleIgnore()
{
	uint32 nIgnoredCount = 0;
	CMemTrackAllocInfo* pFinger = g_pTrackHeap;
	while (pFinger)
	{
		if (pFinger->m_nFlags & CMemTrackAllocInfo::k_nFlag_Marked)
		{
			pFinger->m_nFlags |= CMemTrackAllocInfo::k_nFlag_Ignore;
			pFinger->m_nFlags &= ~CMemTrackAllocInfo::k_nFlag_Marked;
			++nIgnoredCount;
		}
		pFinger = pFinger->m_pNext;
	}

	dsi_ConsolePrint("%d blocks ignored", nIgnoredCount);
}

// console command handler for "mem" console command
void LTMemConsole(int argc, char *argv[])
{
	// debugging information about parameters passed to this function
//	dsi_ConsolePrint("mem console command :\n");
//	for (int n = 0; n < argc; n++)
//	{
//		dsi_ConsolePrint("  argv[%i] = %s\n",n,argv[n]);
//	}

	// if the ltmem system is not being used then just display message and exit
	if (!g_bLTMemInitialized)
	{
		dsi_ConsolePrint("mem commands not available (ltmem system is not turned on in ltmem.cpp in this build)\n");
		return;
	}

#ifndef LTMEMTRACK
	dsi_ConsolePrint("mem commands not available (tracking in ltmem.h is not turned on in this build)\n");
	return;
#endif

	// if no parameters were passed then display help message and exit
	if (argc < 1)
	{
		LTMemConsoleHelp();
		return;
	}

	// get pointer to the command
	char* sCommand = argv[0];

	// make sure command is valid
	if (sCommand == NULL) 
	{
		LTMemConsoleHelp();
		return;
	}

	// is this the help command
	if (stricmp(sCommand, "help") == 0) 
	{
		LTMemConsoleHelp();
	}

	// is this the stats command
	else if (stricmp(sCommand, "stats") == 0) 
	{
		LTMemTrackPrintStats();
	}

	// is this the log command
	else if (stricmp(sCommand, "log") == 0) 
	{
		LTMemLog(argc, argv);
	}

	// is this the fulllog command
	else if (stricmp(sCommand, "fulllog") == 0) 
	{
		LTMemFullLog(argc, argv);
	}

	// is this the mark command
	else if (stricmp(sCommand, "mark") == 0) 
	{
		LTMemConsoleMark();
	}

	// is this the un-mark command
	else if (stricmp(sCommand, "unmark") == 0) 
	{
		LTMemConsoleUnMark();
	}

	// is this the dump command
	else if (stricmp(sCommand, "dump") == 0) 
	{
		LTMemConsoleDump();
	}

	// is this the ignore command
	else if (stricmp(sCommand, "ignore") == 0) 
	{
		LTMemConsoleIgnore();
	}

	// if command is unknown display help
	else LTMemConsoleHelp();
}

struct SMemRecord
{
	char	m_pszFilename[CMEMTRACKFILENAMESIZE];
	uint32	m_nLine;
	uint32	m_nAllocations;
	uint32	m_nMemorySize;
	uint32	m_nMemType;
};

void LTMemLog(uint32 nArgC, char** ppArgV)
{
	//make sure that the parameters are correct
	if(nArgC < 2)
	{
		LTMemConsoleHelp();
		return;
	}

	//the second parameter is the filename
	const char* pszFilename = ppArgV[1];

	//try and open up the file
	FILE* pOutFile = fopen(pszFilename, "wt");

	if(!pOutFile)
	{
		dsi_ConsolePrint("Error opening file %s for memory log", pszFilename);
		return;
	}

	//we now need to build up our data list, note that we allocate an additional item. This
	//is actually for the allocation that we are doing right now
	SMemRecord* pRecordList;
	LT_MEM_TRACK_ALLOC(pRecordList = new SMemRecord[g_nMemTrackAllocationCount - g_nMemTrackFreeCount + 1], LT_MEM_TYPE_MEM);

	//check the allocation
	if(!pRecordList)
	{
		dsi_ConsolePrint("Error allocating memory for memory log.");
		fclose(pOutFile);
		return;
	}

	//alright, now filter all the records into the list
	uint32 nNumRecords = 0;

	CMemTrackAllocInfo* pInfo = g_pTrackHeap;
	while (pInfo)
	{
		//figure out this allocation size
		uint32 nSize = pInfo->m_nActualSize - sizeof(CMemTrackAllocInfo);

		bool bFoundMatch = false;

		//now run through and see if this is a new record or not
		for(uint32 nCurrRecord = 0; nCurrRecord < nNumRecords; nCurrRecord++)
		{
			SMemRecord& Record = pRecordList[nCurrRecord];

			if((Record.m_nLine == pInfo->m_nLineNum) && (strcmp(Record.m_pszFilename, pInfo->m_sFileName) == 0))
			{
				//same line allocation
				Record.m_nAllocations++;
				Record.m_nMemorySize += nSize;
				bFoundMatch = true;
				break;
			}
		}

		//if we didn't match it, we need to add a new one on
		if(!bFoundMatch)
		{
			SMemRecord& Record = pRecordList[nNumRecords];
			LTStrCpy(Record.m_pszFilename, pInfo->m_sFileName, CMEMTRACKFILENAMESIZE);
			Record.m_nAllocations		= 1;
			Record.m_nMemorySize		= nSize;
			Record.m_nLine				= pInfo->m_nLineNum;
			Record.m_nMemType			= pInfo->m_nAllocationType;

			nNumRecords++;
		}
	
		pInfo = pInfo->m_pNext;
	}

	fprintf(pOutFile, "MemType, Filename, Line, Allocations, Memory\n");

	//alright, we have our list of objects, now lets write them out to a file
	for(uint32 nCurrRecord = 0; nCurrRecord < nNumRecords; nCurrRecord++)
	{
		SMemRecord& Record = pRecordList[nCurrRecord];

		//figure out the name of this memory type
		CMemTrackTypeToString* pType = LTMemTrackGetPointerFromType(Record.m_nMemType);

		const char* pszTypeName = (pType) ? pType->m_sName : "Unknown";
		
		fprintf(pOutFile, "%s, %s, %d, %d, %d\n", pszTypeName, Record.m_pszFilename, Record.m_nLine, Record.m_nAllocations, Record.m_nMemorySize);
	}

	//success
	dsi_ConsolePrint("Memory log %s successfully created", pszFilename);

	//clean up
	delete [] pRecordList;
	fclose(pOutFile);
}

void LTMemFullLog(uint32 nArgC, char** ppArgV)
{
	//make sure that the parameters are correct
	if(nArgC < 2)
	{
		LTMemConsoleHelp();
		return;
	}

	//the second parameter is the filename
	const char* pszFilename = ppArgV[1];

	//try and open up the file
	FILE* pOutFile = fopen(pszFilename, "wt");

	if(!pOutFile)
	{
		dsi_ConsolePrint("Error opening file %s for memory log", pszFilename);
		return;
	}

	fprintf(pOutFile, "MemType, Filename, Line, Memory\n");

	CMemTrackAllocInfo* pInfo = g_pTrackHeap;
	while (pInfo)
	{
		//figure out this allocation size
		uint32 nSize = pInfo->m_nActualSize - sizeof(CMemTrackAllocInfo);

		//figure out the name of this memory type
		CMemTrackTypeToString* pType = LTMemTrackGetPointerFromType(pInfo->m_nAllocationType);
		const char* pszTypeName = (pType) ? pType->m_sName : "Unknown";

		fprintf(pOutFile, "%s, %s, %d, %d\n", pszTypeName, pInfo->m_sFileName, pInfo->m_nLineNum, nSize);
	
		pInfo = pInfo->m_pNext;
	}


	//success
	dsi_ConsolePrint("Full memory log %s successfully created", pszFilename);

	//clean up
	fclose(pOutFile);
}

//given a memory count, it will return a floating point version in megs
static float ConvertToMegs(uint32 nBytes)
{
	return (float)nBytes / (float)(1024 * 1024);
}

static void PrintMemStat(const char* pszName, uint32 nBytes, uint32 nNumAllocations, uint32 nTotalAllocations)
{
	dsi_ConsolePrint("  %s = %.2f MB        (%u allocations, %u total)\n", pszName, ConvertToMegs(nBytes), nNumAllocations, nTotalAllocations);
}

// print out memory stats
void LTMemTrackPrintStats()
{
	//the current number of outstanding allocations
	uint32 nCurrAllocations = g_nMemTrackAllocationCount - g_nMemTrackFreeCount;

	dsi_ConsolePrint("memory stats :\n");
	dsi_ConsolePrint("  current allocations = %u\n",nCurrAllocations);
	dsi_ConsolePrint("  total allocations = %u\n",g_nMemTrackAllocationCount);
	dsi_ConsolePrint("  total frees = %u\n",g_nMemTrackFreeCount);
	dsi_ConsolePrint("  average bytes per allocation = %u\n", g_nMemTrackTotalAllocated / nCurrAllocations);

	dsi_ConsolePrint("  peak allocations = %u\n", g_nMemTrackPeakAllocations);
	dsi_ConsolePrint("  peak memory usage = %.2f", ConvertToMegs(g_nMemTrackPeakMemAllocated));

	// clear counts in all types
	CMemTrackTypeToString* pList = g_pLTMemTrackTypeToStringList;
	while (pList != NULL)
	{
		pList->m_nMemoryAllocated = 0;
		pList->m_nNumAllocations  = 0;

		pList = pList->m_pNext;
	}

	uint32 nUnregisteredMemory = 0;
	uint32 nUnregisteredAllocations = 0;

	//keep track of the smallest and largest blocks we have allocated
	uint32 nSmallestBlock = 0xFFFFFFFF;
	uint32 nLargestBlock = 0;

	// walk heap to count all types of memory
	CMemTrackAllocInfo* pInfo = g_pTrackHeap;
	while (pInfo != NULL)
	{
		uint32 nSize = pInfo->m_nActualSize - sizeof(CMemTrackAllocInfo);
		CMemTrackTypeToString* pType = LTMemTrackGetPointerFromType(pInfo->m_nAllocationType);
		if (pType == NULL) 
		{
			nUnregisteredMemory += nSize;
			nUnregisteredAllocations++;
		}
		else 
		{
			pType->m_nMemoryAllocated += nSize;
			pType->m_nNumAllocations++;
		}

		if(nSize > nLargestBlock)
			nLargestBlock = nSize;
		if(nSize < nSmallestBlock)
			nSmallestBlock = nSize;
	
		pInfo = pInfo->m_pNext;
	}

	//display the largest and smallest allocation sizes
	dsi_ConsolePrint("  largest allocation = %u\n",nLargestBlock);
	dsi_ConsolePrint("  smallest allocation = %u\n",nSmallestBlock);

	// print out totals from all types of memory
	dsi_ConsolePrint("\n");
	PrintMemStat("total allocated", g_nMemTrackTotalAllocated, nCurrAllocations, g_nMemTrackAllocationCount);
	dsi_ConsolePrint("\n");

	uint32 nAccountedAllocations = 0;

	pList = g_pLTMemTrackTypeToStringList;
	while (pList != NULL)
	{
		uint32 nTotalAllocations = g_nMemTypeTrackAllocationCount[pList->m_nType];

		//only bother printing out the categories that actually have memory in them
		if(pList->m_nMemoryAllocated || pList->m_nNumAllocations || nTotalAllocations)
		{
			PrintMemStat(pList->m_sName, pList->m_nMemoryAllocated, pList->m_nNumAllocations, nTotalAllocations);
			nAccountedAllocations += nTotalAllocations;
		}

		pList = pList->m_pNext;
	}
	PrintMemStat("unregistered", nUnregisteredMemory, nUnregisteredAllocations, g_nMemTrackAllocationCount - nAccountedAllocations);
}
