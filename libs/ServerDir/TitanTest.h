//////////////////////////////////////////////////////////////////////////////
// Testing code for playing with the Titan API

#ifndef __TITANTEST_H__
#define __TITANTEST_H__

#ifdef _FINAL

inline void TitanTest_Init() {}
inline void TitanTest_Term() {}

#else // _FINAL

void TitanTest_Init();
void TitanTest_Term();

#endif // _FINAL

#endif //__TITANTEST_H__