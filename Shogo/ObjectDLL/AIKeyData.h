// ----------------------------------------------------------------------- //
//
// MODULE  : CAIKeyData.h
//
// PURPOSE : CAIKeyData definition for Keyframer class
//
// CREATED : 2/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __AI_KEY_DATA_H__
#define __AI_KEY_DATA_H__

#include "cpp_server_de.h"

#define MAX_AIKEY_NAME_LENGTH 20

class CAIKeyData
{
	public :

		CAIKeyData();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		DBOOL Copy(HOBJECT pKey);

		DVector	m_vPos;							// Key Position
		char m_Name[MAX_AIKEY_NAME_LENGTH+1];	// Key Name
};

#endif // __AI_KEY_DATA_H__

