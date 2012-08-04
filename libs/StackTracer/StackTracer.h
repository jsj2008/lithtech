#ifndef _STACK_TRACER_H_
#define _STACK_TRACER_H_

#include <vector>
#include "dbghelp.h"

typedef unsigned __int64 DWORD64;

struct _tStackTraceEntry
{
	DWORD64 ip;       // Instruction pointer (EIP on x86) 
	DWORD64 retAddr;  // Return address 
	DWORD64 ebp;       // Stack base pointer (EBP on x86)
};
typedef std::vector<_tStackTraceEntry> StackTrace;

struct _tSymbolDetails
{
	ULONG64		Address;
	ULONG		Size;
	char		Name[512];
	char		Filename[512];
	DWORD		LineNumber;
	bool		ErrorCode;
};
typedef _tSymbolDetails SymbolDetails;

class StackTracer
{
public:
	StackTracer();
	~StackTracer();

	void DoStackTrace(StackTrace& trace);
	void LoadSymbols(const char* pFilename);
	void LoadSymbolsInDir(const char* pDirectory, const char* pExtensions = "exe,dll");
	void GetSymbolByAddr(SymbolDetails &symDetails, DWORD64 addr);
	void UnloadSymbols();

protected:
	bool m_bInitialized;
	std::vector<DWORD64> m_vecModBase;

	void Init();
	void Term();
};


#endif