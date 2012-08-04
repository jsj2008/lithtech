
// The special FX registry.  Makes it a little easier to make new special FX.
// Just use the REGISTER_SFX macro to register your class and ID and it'll 
// automatically handle creating it and adding it to the SFX lists.  You also
// must derive your class from CAutoSpecialFX and implement the InitAuto
// function.

#ifndef __SFXREG_H__
#define __SFXREG_H__


	#include "SpecialFX.h"
#include "iltmessage.h"


	class SFXReg;


	// Derive your special effect from this.
	class CAutoSpecialFX : public CSpecialFX
	{
	public:
		
		virtual ~CAutoSpecialFX() {}
		
		// Initialize from the special FX message.
		virtual LTBOOL	InitAuto(HOBJECT hServerObj, ILTMessage_Read* hRead)
		{
			m_hServerObject = hServerObj;
			return LTTRUE;
		}
	};	


	typedef CAutoSpecialFX* (*ASFXCreateFn)();


	// The global list of registered SFX classes.
	extern SFXReg *g_SFXRegHead;


	class SFXReg
	{
	public:

						SFXReg(char *pClassName, ASFXCreateFn fn)
						{
							m_ID = 0xFFFFFFFF;
							m_SFXName = pClassName;
							m_Fn = fn;
							m_pNext = g_SFXRegHead;
							g_SFXRegHead = this;
						}

		uint32			m_ID; // ID the server refers to it as.
		char			*m_SFXName;
		ASFXCreateFn	m_Fn;
		SFXReg			*m_pNext;
	};


	#define REGISTER_SFX(sfxName, className) \
		static CAutoSpecialFX* __##className##SFXCreateFn() {return new className;} \
		static SFXReg __##className##_Reg(#sfxName, __##className##SFXCreateFn);


	SFXReg* FindSFXReg(uint32 id);
	SFXReg* FindSFXRegByName(char *pName);


#endif




