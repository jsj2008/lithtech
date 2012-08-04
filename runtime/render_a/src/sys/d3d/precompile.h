
// This file includes the main headers for d3drender.

#ifndef __PRECOMPILE_H__
#define __PRECOMPILE_H__

#define DIRECT3D_VERSION        0x0900

#ifndef __BDEFS_H__
#include "bdefs.h"
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __D3D9_H__
#include <d3d9.h>
#define __D3D9_H__
#endif

#ifndef __D3D9TYPES_H__
#include <d3d9types.h>
#define __D3D9TYPES_H__
#endif

#ifndef __D3DX9_H__
#include <d3dx9.h>
#define __D3DX9_H__
#endif


#ifndef __D3D9CAPS_H__
#include <d3d9caps.h>
#define __D3D9CAPS_H__
#endif

#ifndef __D3D_DEFS_H__
#include "d3d_defs.h"
#endif

 
#define FASTCALL __fastcall

 
// STL
#pragma warning(disable : 4786)
#include <vector>
#include <list>
#ifndef __CODEWARRIOR
using namespace std;
#endif
#endif  // __PRECOMPILE_H__



