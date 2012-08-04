//-----------------------------------------------------------------
// LTASaveUtils.h
//
// Provides common functions for saving out lists to a node
// builder for LTA saving. This is not part of the standard LTA
// library since most of these are somewhat domain specific
// to loading and saving of LTAs in the LT tools.
//
// Author: John O'Rorke
// Created: 1/30/01
// Modification History:
//
//-----------------------------------------------------------------
#ifndef __LTASAVEUTILS_H__
#define __LTASAVEUTILS_H__

//each one of these is for printing out a small list, containing only
//the item specified. This list can be named by using the ...Named
//variety, or it just prints out the value.
void SaveStringLTANamed(const char* name,CLTANodeBuilder& lb, const char* data);
void SaveStringLTA(CLTANodeBuilder& lb, const char* data);
void SaveFloatLTANamed(const char* name,CLTANodeBuilder& lb, float data);
void SaveFloatLTA(CLTANodeBuilder& lb, float data);
void SaveIntLTANamed(const char* name,CLTANodeBuilder& lb, int data);
void SaveIntLTA(CLTANodeBuilder& lb, int data);
void SaveUIntLTANamed(const char* name,CLTANodeBuilder& lb, uint32 data);
void SaveUIntLTA(CLTANodeBuilder& lb, uint32 data);
void SaveBoolLTANamed(const char* name,CLTANodeBuilder& lb, int data);
void SaveBoolLTA(CLTANodeBuilder& lb, int data);
void SaveFVecLTANamed(const char* name, CLTANodeBuilder& lb, PVector &vec);
void SaveFVecLTA(CLTANodeBuilder& lb, PVector &vec);
void SaveLTVecLTA(CLTANodeBuilder& lb, LTVector &vec);
void SaveLTVecLTANamed(const char* name, CLTANodeBuilder& lb, LTVector &vec);

//this prints out a string of tabs of the specified length, preceded by a newline
void PrependTabs(CLTAFile* pFile, uint32 level);


//---------------------------------
// Inlines
//
inline void SaveIntLTA(CLTANodeBuilder& lb, int data)
{
	lb.AddValue((int32)data);
}

inline void SaveIntLTANamed(char* name,CLTANodeBuilder& lb, int data)
{
	lb.Push();
		lb.AddValue(name);
		SaveIntLTA(lb, data);
	lb.Pop();
}

inline void SaveUIntLTA(CLTANodeBuilder& lb, uint32 data)
{
	lb.AddValue((int32)data);
}

inline void SaveUIntLTANamed(char* name,CLTANodeBuilder& lb, uint32 data)
{
	lb.Push();
		lb.AddValue(name);
		SaveUIntLTA(lb, data);
	lb.Pop();
}

inline void SaveFVecLTA(CLTANodeBuilder& lb, PVector &vec)
{
	lb.Push();
		lb.AddValue(vec.x); 
		lb.AddValue(vec.y); 
		lb.AddValue(vec.z); 
	lb.Pop();
}

inline void SaveFVecLTANamed(char* name, CLTANodeBuilder& lb, PVector &vec)
{
	lb.Push();
		lb.AddValue(name);
		SaveFVecLTA(lb, vec);
	lb.Pop();
}

inline void SaveLTVecLTA(CLTANodeBuilder& lb, LTVector &vec)
{
	lb.Push();
		lb.AddValue(vec.x); 
		lb.AddValue(vec.y); 
		lb.AddValue(vec.z); 
	lb.Pop();
}

inline void SaveLTVecLTANamed(const char* name, CLTANodeBuilder& lb, LTVector &vec)
{
	lb.Push(name);
		SaveLTVecLTA(lb, vec);
	lb.Pop();
}

inline void SaveBoolLTA(CLTANodeBuilder& lb, int data)
{
	data ? lb.AddValue((int32)1) : lb.AddValue((int32)0);
}

inline void SaveBoolLTANamed(const char* name,CLTANodeBuilder& lb, int data)
{
	lb.Push(name);
		SaveBoolLTA(lb, data);
	lb.Pop();
}

inline void SaveStringLTA(CLTANodeBuilder& lb, const char* data)
{
	lb.AddValue(data, 1); 
}

inline void SaveStringLTANamed(const char* name,CLTANodeBuilder& lb, const char* data)
{
	lb.Push(name);
		SaveStringLTA(lb, data);
	lb.Pop();
}

inline void SaveFloatLTA(CLTANodeBuilder& lb, float data)
{
	lb.AddValue(data); 
}

inline void SaveFloatLTANamed(const char* name,CLTANodeBuilder& lb, float data)
{
	lb.Push(name);
		SaveFloatLTA(lb, data);
	lb.Pop();
}

//writes out a string of tabs to the specified 
inline void PrependTabs(CLTAFile* pFile, uint32 level)
{
	//sanity check
	ASSERT(pFile);

	//end the previous line
	pFile->WriteStr("\n");

	//write out the tab string
	for( uint32 i = 0; i < level; i++ )
	{
		pFile->WriteStr("\t");
	}
}



#endif
