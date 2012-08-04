#ifndef __LOADSTATUS_H__
#define __LOADSTATUS_H__


//
//This status enum is used by the many functions that are called 
//during the world loading process.
//

enum ELoadWorldStatus {
    LoadWorld_Ok=0,
    LoadWorld_InvalidVersion=1,
    LoadWorld_InvalidFile=2,
    LoadWorld_InvalidParams,
    LoadWorld_Error // Generic error
};




#endif

