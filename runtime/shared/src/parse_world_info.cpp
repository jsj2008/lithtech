#include "bdefs.h"

#include "conparse.h"
#include "parse_world_info.h"


bool ParseAmbientLight(const char *pStr, LTVector *pAmbient) {
    ConParse parse;

    parse.Init(pStr);
    pAmbient->Init();

    if (parse.ParseFind("AmbientLight", LTFALSE, 4)) {
        // <AmbientLight> <r> <g> <b>
        // colors are 0-255
        pAmbient->x = (float)atof(parse.m_Args[1]);
        pAmbient->y = (float)atof(parse.m_Args[2]);
        pAmbient->z = (float)atof(parse.m_Args[3]);
        VEC_CLAMP(*pAmbient, 0.0f, 255.0f);
        return LTTRUE;
    }
    
    return LTFALSE;
}


