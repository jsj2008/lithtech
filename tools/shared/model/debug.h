// Debug.h - Debugging utilities..

#ifndef __DEBUG_H__
#define __DEBUG_H__

void DebugTraceFn(const char *st, ...);
#ifdef _DEBUG
#define TRACE DebugTraceFn
#else
#define TRACE 1 ? (void)0 : DebugTraceFn
#endif

#endif // __DEBUG_H__