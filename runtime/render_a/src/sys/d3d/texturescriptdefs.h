//------------------------------------------------------------------
//
//   Module    : TextureScriptDefs
//
//   Purpose   : Provide definitions for constants that are used
//				 in the scripting system for textures.
//
//   Created   : On 9/23/2001 At 2:30:24 PM
//
//   Copyright : (C) 2001 Monolith Inc
//
//------------------------------------------------------------------

#ifndef __TEXTURESCRIPTDEFS_H__
#define __TEXTURESCRIPTDEFS_H__

#include "ltbasedefs.h"


//-----------------------------------------------------
//the IDs of the accessible variables for the script
//-----------------------------------------------------

//the working stack variable
#define TSVAR_STACK				0
#define TS_NUMSTACKVARS			16

//the first matrix variable, and the dimensions of the matrix (assumed square)
#define TSVAR_MAT				(TSVAR_STACK + TS_NUMSTACKVARS)
#define	TS_MATWIDTH				4
#define TS_NUMMATVARS			(TS_MATWIDTH * TS_MATWIDTH)

//the user defined variables
#define TSVAR_USER				(TSVAR_MAT + TS_NUMMATVARS)
#define TS_NUMUSERVARS			6

//the variables used for storing time information provided by the system
#define TSVAR_TIME				(TSVAR_USER + TS_NUMUSERVARS)
#define TSVAR_ELAPSED			(TSVAR_TIME + 1)

//the constant variables
#define TSVAR_CONSTANT			(TSVAR_ELAPSED + 1)
#define TS_NUMCONSTANTS			32

//the level offset variables
#define TSVAR_LEVELOFFSETX		(TSVAR_CONSTANT + TS_NUMCONSTANTS)
#define TSVAR_LEVELOFFSETY		(TSVAR_LEVELOFFSETX + 1)
#define TSVAR_LEVELOFFSETZ		(TSVAR_LEVELOFFSETX + 2)

//the total number of variables
#define TS_NUMVARS				(TSVAR_LEVELOFFSETZ + 1)

//---------------------------------------------------
// Opcodes for the script
//---------------------------------------------------
#define TSOP_ADD				0
#define TSOP_SUBTRACT			1
#define TSOP_MULTIPLY			2
#define TSOP_DIVIDE				3
#define TSOP_BIND				4
#define TSOP_FUNCTION			5
#define TSOP_NEGATE				6
#define TSOP_ASSIGN				7
#define TSOP_MIN				8
#define TSOP_MAX				9

#define TS_NUMOPS				10

//---------------------------------------------------
// Functions available for the script
//---------------------------------------------------
#define TSFUNC_SIN				0
#define TSFUNC_COS				1
#define TSFUNC_TAN				2
#define TSFUNC_SQRSIN			3
#define TSFUNC_SQRCOS			4
#define TSFUNC_DEGTORAD			5

#define TS_NUMFUNCTIONS			6

//---------------------------------------------------
// Available input modes
//---------------------------------------------------
#define TSINPUT_CSNORMAL		0
#define TSINPUT_CSPOS			1
#define TSINPUT_CSREFLECTION	2
#define TSINPUT_WSNORMAL		3
#define TSINPUT_WSPOS			4
#define TSINPUT_WSREFLECTION	5
#define TSINPUT_UV				6

#define TS_NUMINPUTS			7

//---------------------------------------------------
// Available ouput modes
//---------------------------------------------------
#define TSOUTPUT_2				0
#define TSOUTPUT_3				1
#define TSOUTPUT_3PROJ			2
#define TSOUTPUT_4PROJ			3

#define TS_NUMOUTPUTS			4

//---------------------------------------------------
// Flags to determine when this script needs to be
// reprocessed
//---------------------------------------------------
#define TSDIRTY_EVERYUPDATE		0x01
#define TSDIRTY_USERVARCHANGED	0x02

//---------------------------------------------------
// Types used for each op code variable
//---------------------------------------------------
typedef uint8					TSOp;

//---------------------------------------------------
// The current texture script version
//---------------------------------------------------
#define TS_CURR_VERSION			1

#endif
