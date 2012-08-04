//------------------------------------------------------------------
//
//	FILE	  : PreprocessorBase.h
//
//	PURPOSE	  : Defines all the base stuff for the preprocessor.
//
//	CREATED	  : December 10 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PREPROCESSOR_BASE_H__
#define __PREPROCESSOR_BASE_H__

	#ifdef PRE_FLOAT
		typedef float				PReal;
	#else
		typedef double				PReal;
	#endif

	typedef TVector3<PReal>		PVector;

	class PVertex : public TVector3<PReal>
	{
		public:
			void operator=(const PVertex &other) {TVector3<PReal>::operator=(other);}
	};

	typedef CMoArray<PVertex>	PVertexArray;

#endif  // __PREPROCESSOR_BASE_H__

