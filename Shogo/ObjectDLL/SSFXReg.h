
// This file manages the special FX registry on the server.  Just derive your class
// from SAutoSpecialFX, call the REGISTER_SSFX macro in the source file, and the
// IMPLEMENT_SSFX macro inside the class.

#ifndef __SSFXREG_H__
#define __SSFXREG_H__


	#include "cpp_engineobjects_de.h"

	
	class SSFXReg;


	class SAutoSpecialFX : public BaseClass
	{
	public:
		virtual	~SAutoSpecialFX() {}
		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		virtual int GetAutoSFXID() {return 0xFFFF;}

		virtual void WriteMessage(HMESSAGEWRITE hWrite) {}
	};


	typedef SAutoSpecialFX* (*ASSFXCreateFn)();


	// The global list of registered SFX classes.
	extern SSFXReg *g_SSFXRegHead;

	class SSFXReg
	{
	public:

						SSFXReg(char *pClassName)
						{
							// Get our ID.
							m_ID = g_SSFXRegHead ? g_SSFXRegHead->m_ID+1 : 0;
							m_SFXName = pClassName;
							m_pNext = g_SSFXRegHead;
							g_SSFXRegHead = this;
						}

		DDWORD			m_ID; // ID the server refers to it as.
		char			*m_SFXName;
		SSFXReg			*m_pNext;
	};


	#define REGISTER_SSFX(sfxName, className) \
		static SSFXReg __##className##_Reg(#sfxName); \
		##className::GetAutoSFXID() {return (int)__##className##_Reg.m_ID;}

	#define IMPLEMENT_SSFX() GetAutoSFXID();


#endif



