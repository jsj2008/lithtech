// This is used to add all key information used by KeyFramer objects to
// blind data chunks within the world.  Key data can come from either
// Key objects, or from a seperate key data file.

#include "bdefs.h"
#include "editregion.h"
#include "preworld.h"
#include "proputils.h"
#include "processing.h"
#include "node_ops.h"
#include "ltamgr.h"
#include "geomroutines.h"
#include "lteulerangles.h"


#define KEYFRAMER_BLINDOBJECTID		0x6aaf0884		// just a random, but constant 32-bit ID


class KeyData
{
public:
	KeyData( CWorldNode* key, const LTMatrix& invRotOffset );
	KeyData( CLTANode* keyNode, const LTMatrix& offset, const LTMatrix& invRotOffset, const LTMatrix& rotOffset );
	~KeyData();

	uint32 Size();
	uint8* Write( uint8* dest );

	CWorldNode* WorldNode() { return worldNode; }

private:
	LTVector pos;			// position
	LTVector rot;			// rotation
	LTVector bezIn;			// in tangent
	LTVector bezOut;		// out tangent
	float soundRadius;		// sound radius
	float timeStamp;		// time since last key
	char* sound;			// sound name
	char* command;			// command string

	uint32 soundLen;
	uint32 commandLen;
	bool bezInUsed;
	bool bezOutUsed;
	bool valid;

	CWorldNode* worldNode;
};


KeyData::KeyData( CWorldNode* key, const LTMatrix& invRotOffset )
{
	worldNode = key;

	CRotationProp* rotProp;
	CVectorProp* vecProp;
	CStringProp* stringProp;
	CRealProp* realProp;

	CPropList* propList = key->GetPropertyList();

	if( vecProp = (CVectorProp*)propList->GetMatchingProp( "Pos", PT_VECTOR ) )
		pos = vecProp->m_Vector;
	else
		pos.Init();

	if( rotProp = (CRotationProp*)propList->GetMatchingProp( "Rotation", PT_ROTATION ) )
		rot = rotProp->GetEulerAngles();
	else
		rot.Init();

	LTRotation quat( rot.x, rot.y, rot.z );
	LTMatrix rotMat;
	quat.ConvertToMatrix( rotMat );

	rotMat = rotMat * invRotOffset;

	EulerAngles ea = Eul_FromMatrix( rotMat, EulOrdYXZr );

	rot.y = ea.x;
	rot.x = ea.y;
	rot.z = ea.z;

	if( realProp = (CRealProp*)propList->GetMatchingProp( "TimeStamp", PT_REAL ) )
		timeStamp = realProp->m_Value;
	else
		timeStamp = 0.0f;

	sound = NULL;
	soundLen = 0;
	if( stringProp = (CStringProp*)propList->GetMatchingProp( "SoundName", PT_STRING ) )
	{
		const char* tmpStr = stringProp->GetString();
		if( tmpStr && (soundLen = strlen( tmpStr )) )
		{
			sound = new char[soundLen+1];
			strcpy( sound, tmpStr );
		}
	}

	if( realProp = (CRealProp*)propList->GetMatchingProp( "SoundRadius", PT_REAL ) )
		soundRadius = realProp->m_Value;
	else
		soundRadius = 0.0f;

	command = NULL;
	commandLen = 0;
	if( stringProp = (CStringProp*)propList->GetMatchingProp( "Command", PT_STRING ) )
	{
		const char* tmpStr = stringProp->GetString();
		if( tmpStr && (commandLen = strlen( tmpStr )) )
		{
			command = new char[commandLen+1];
			strcpy( command, tmpStr );
		}
	}

	if( vecProp = (CVectorProp*)propList->GetMatchingProp( "BezierPrev", PT_VECTOR ) )
	{
		bezIn = vecProp->m_Vector;
		bezInUsed = (bezIn.MagSqr() > 0.0f);
	}
	else
	{
		bezIn.Init();
		bezInUsed = false;
	}

	if( vecProp = (CVectorProp*)propList->GetMatchingProp( "BezierNext", PT_VECTOR ) )
	{
		bezOut = vecProp->m_Vector;
		bezOutUsed = (bezOut.MagSqr() > 0.0f);
	}
	else
	{
		bezOut.Init();
		bezOutUsed = false;
	}
}


KeyData::KeyData( CLTANode* keyNode, const LTMatrix& offset, const LTMatrix& invRotOffset, const LTMatrix& rotOffset )
{
	// only used when creating keys from Key objects
	worldNode = NULL;

	CLTANode* curNode;

	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "pos" )) && (curNode->GetNumElements() == 4) )
	{
		pos.x = GetFloat( curNode->GetElement( 1 ) );
		pos.y = GetFloat( curNode->GetElement( 2 ) );
		pos.z = GetFloat( curNode->GetElement( 3 ) );
	}
	else
		pos.Init();
	offset.Apply( pos );

	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "rot" )) && (curNode->GetNumElements() == 4) )
	{
		rot.x = GetFloat( curNode->GetElement( 1 ) );
		rot.y = GetFloat( curNode->GetElement( 2 ) );
		rot.z = GetFloat( curNode->GetElement( 3 ) );
	}
	else
		rot.Init();

	LTRotation quat( rot.x, rot.y, rot.z );
	LTMatrix rotMat;
	quat.ConvertToMatrix( rotMat );

	rotMat = rotOffset * rotMat * invRotOffset;

	EulerAngles ea = Eul_FromMatrix( rotMat, EulOrdYXZr );

	rot.y = ea.x;
	rot.x = ea.y;
	rot.z = ea.z;

	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "time" )) && (curNode->GetNumElements() == 2) )
		timeStamp = GetFloat( curNode->GetElement( 1 ) );
	else
		timeStamp = 0.0f;

	sound = NULL;
	soundLen = 0;
	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "sound" )) && (curNode->GetNumElements() == 2) )
	{
		const char* tmpStr = GetString( curNode->GetElement( 1 ) );
		if( tmpStr && (soundLen = strlen( tmpStr )) )
		{
			sound = new char[soundLen+1];
			strcpy( sound, tmpStr );
		}
	}

	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "soundradius" )) && (curNode->GetNumElements() == 2) )
		soundRadius = GetFloat( curNode->GetElement( 1 ) );
	else
		soundRadius = 0.0f;

	command = NULL;
	commandLen = 0;
	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "command" )) && (curNode->GetNumElements() == 2) )
	{
		const char* tmpStr = GetString( curNode->GetElement( 1 ) );
		if( tmpStr && (commandLen = strlen( tmpStr )) )
		{
			command = new char[commandLen+1];
			strcpy( command, tmpStr );
		}
	}

	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "bezierprev" )) && (curNode->GetNumElements() == 4) )
	{
		bezIn.x = GetFloat( curNode->GetElement( 1 ) );
		bezIn.y = GetFloat( curNode->GetElement( 2 ) );
		bezIn.z = GetFloat( curNode->GetElement( 3 ) );
		bezInUsed = (bezIn.MagSqr() > 0.0f);
	}
	else
	{
		bezIn.Init();
		bezInUsed = false;
	}

	if( (curNode = CLTAUtil::ShallowFindList( keyNode, "beziernext" )) && (curNode->GetNumElements() == 4) )
	{
		bezOut.x = GetFloat( curNode->GetElement( 1 ) );
		bezOut.y = GetFloat( curNode->GetElement( 2 ) );
		bezOut.z = GetFloat( curNode->GetElement( 3 ) );
		bezOutUsed = (bezOut.MagSqr() > 0.0f);
	}
	else
	{
		bezOut.Init();
		bezOutUsed = false;
	}
}


KeyData::~KeyData()
{
	delete [] command;
	delete [] sound;
}


uint32 KeyData::Size( void )
{
	static uint32 fltSz = sizeof(float);
	static uint32 vecSz = 3*fltSz;

	uint32 size = 0;

	size += 2;			// control word
	size += 1;			// sound length
	size += 1;			// command length
	size += vecSz;		// position
	size += vecSz;		// rotation
	size += fltSz;		// time stamp
	size += fltSz;		// sound radius
	size += soundLen;	// sound string (optional)
	size += commandLen;	// command string (optional)
	if( bezInUsed )
		size += vecSz;	// in tangent
	if( bezOutUsed )
		size += vecSz;		// out tangent

	return size;
}


uint8* KeyData::Write( uint8* dest )
{
	static uint32 fltSz = sizeof(float);
	uint32 i;

	uint8* startDest = dest;

	// write the control word
	uint16 control = 0;
	if( bezInUsed )
		control |= (1<<0);
	if( bezOutUsed )
		control |= (1<<1);
	*((uint16*)dest) = control;
	dest += 2;

	// write the sound and command lengths
	*(dest++) = soundLen;
	*(dest++) = commandLen;

	// write the postition
	*((float*)dest) = pos.x;
	dest += fltSz;
	*((float*)dest) = pos.y;
	dest += fltSz;
	*((float*)dest) = pos.z;
	dest += fltSz;

	// write the rotation
	*((float*)dest) = rot.x;
	dest += fltSz;
	*((float*)dest) = rot.y;
	dest += fltSz;
	*((float*)dest) = rot.z;
	dest += fltSz;

	// write the time stamp
	*((float*)dest) = timeStamp;
	dest += fltSz;

	// write the sound radius
	*((float*)dest) = soundRadius;
	dest += fltSz;

	// write the sound string (optional)
	for( i = 0; i < soundLen; i++ )
	{
		*(dest++) = sound[i];
	}

	// write the command string (optional)
	for( i = 0; i < commandLen; i++ )
	{
		*(dest++) = command[i];
	}

	// write the in tangent (optional)
	if( bezInUsed )
	{
		*((float*)dest) = bezIn.x;
		dest += fltSz;
		*((float*)dest) = bezIn.y;
		dest += fltSz;
		*((float*)dest) = bezIn.z;
		dest += fltSz;
	}

	// write the out tangent (optional)
	if( bezOutUsed )
	{
		*((float*)dest) = bezOut.x;
		dest += fltSz;
		*((float*)dest) = bezOut.y;
		dest += fltSz;
		*((float*)dest) = bezOut.z;
		dest += fltSz;
	}

	ASSERT( uint32(dest - startDest) == Size() );

	return dest;
}


static uint32 FindObjectsWithBaseName( CEditRegion* region, const char* baseName, std::vector<CWorldNode*>& objects )
{
	objects.clear();

	uint32 baseNameLen = strlen( baseName );

	for( uint32 curObj = 0; curObj < region->m_Objects; curObj++ )
	{
		CBaseEditObj* pObj = region->m_Objects[curObj];

		if( pObj->GetType() != Node_Object )
			continue;

		if( const char* objName = pObj->GetName() )
		{
			if( strnicmp( objName, baseName, baseNameLen ) == 0 )
				objects.push_back( pObj );
		}
	}

	return objects.size();
}


static uint32 FindObjectsOfType( CEditRegion* region, const char* objectType, std::vector<CWorldNode*>& objects )
{
	objects.clear();

	for( uint32 curObj = 0; curObj < region->m_Objects; curObj++ )
	{
		CBaseEditObj* pObj = region->m_Objects[curObj];

		if( pObj->GetType() != Node_Object )
			continue;

		if( const char* className = pObj->GetClassName() )
		{
			if( strcmp( className, objectType ) == 0 )
				objects.push_back( pObj );
		}
	}

	return objects.size();
}


static CWorldNode* FindKey( std::vector<CWorldNode*>& keys, const char* baseKeyName, uint32 keyNum )
{
	char keyName[256];

	if( keyNum > 0 && keyNum < 10 )
		sprintf( keyName, "%s0%d", baseKeyName, keyNum );
	else
		sprintf( keyName, "%s%d", baseKeyName, keyNum );

	std::vector<CWorldNode*>::iterator it = keys.begin();

	for( ; it != keys.end(); it++ )
	{
		CWorldNode* curNode = *it;
		if( stricmp( curNode->GetName(), keyName ) == 0 )
			return curNode;
	}

	return NULL;
}


static CLTANode* FindKeyFromFile( CLTANode* keyList, const char* baseKeyName, uint32 keyNum )
{
	ASSERT( keyList && baseKeyName );

	char keyName[256];

	if( keyNum > 0 && keyNum < 10 )
		sprintf( keyName, "%s0%d", baseKeyName, keyNum );
	else
		sprintf( keyName, "%s%d", baseKeyName, keyNum );

	CLTANode* keyNode = CLTAUtil::ShallowFindList( keyList, keyName );

	return keyNode;
}


// add key data to the blind data
static bool AddKeyData( CEditRegion* region, CPreMainWorld* world, CWorldNode* keyFramer, std::vector<KeyData*>& keyData )
{
	std::vector<KeyData*>::iterator it;

	// determine the size of the blind data
	uint32 blindDataSize = 4;			// start off with enough space for the number of keys
	uint32 numKeys = 0;

	for( it = keyData.begin(); it != keyData.end(); it++ )
	{
		blindDataSize += (*it)->Size();
		numKeys++;
	}

	ASSERT( numKeys == keyData.size() );

	// allocate space for the blind data
	uint8* blindData = new uint8[blindDataSize];
	uint8* curBlindDataPtr = blindData;

	// add the number of keys to the blind data
	*((uint32*)curBlindDataPtr) = numKeys;
	curBlindDataPtr += 4;

	// add each key to the blind data, delete the temporary data, and remove the key object (if any) from the world
	for( it = keyData.begin(); it != keyData.end(); it++ )
	{
		KeyData* curKeyData = *it;

		// add the key to the blind data
		curBlindDataPtr = curKeyData->Write( curBlindDataPtr );

		// remove the key object (if any) from the world
		if( curKeyData->WorldNode() )
			no_DestroyNode( region, curKeyData->WorldNode(), false );

		// delete the intermediate data
		delete curKeyData;
	}

	ASSERT( (uint32(curBlindDataPtr - blindData) == blindDataSize) );

	// add the blind data to the world
	CPreBlindData* blindDataObject = new CPreBlindData( blindData, blindDataSize, KEYFRAMER_BLINDOBJECTID );
	world->m_BlindObjectData.push_back( blindDataObject );

	// tag the keyframer with the index of the blind data containing its key data
	CRealProp* prop = (CRealProp*)keyFramer->GetPropertyList()->GetMatchingProp( "KeyDataIndex", PT_REAL );

	if( !prop )
	{
		DrawStatusText(eST_Error, "Found out of date object.  Please resave this level before reprocessing.");
		return false;
	}

	prop->SetValue( (float)(world->m_BlindObjectData.size() - 1) + 0.1f );

	return true;
}


// convert key objects to blind keyframer data
static bool ConvertKeyFramerObjects( CEditRegion* region, CPreMainWorld* world, CWorldNode* keyFramer, const char* baseKeyName, std::vector<CWorldNode*>& keys )
{
	std::vector<KeyData*> keyData;
	keyData.reserve( keys.size() );

	uint32 curKeyNum = 0;
	CWorldNode* curKey = NULL;

	// build up the transform needed to tweak potentially rotated keyframers
	// this assumes that the keys are rotated as much as the keyframer, which is a good assumption unless the LD is loopy
	LTVector kfRot = keyFramer->GetOr();
	LTRotation quat( kfRot.x, kfRot.y, kfRot.z );
	LTMatrix kfInvRotOffset;
	quat.ConvertToMatrix( kfInvRotOffset );
	kfInvRotOffset.Inverse();

	// extract the keyframe info for the keys used by this keyframer
	while( curKey = FindKey( keys, baseKeyName, curKeyNum ) )
	{
		KeyData* newKeyData = new KeyData( curKey, kfInvRotOffset );
		keyData.push_back( newKeyData );
		curKeyNum++;
	}

	// warn the user if not all the keys were used
	if( curKeyNum != keys.size() )
		DrawStatusText(eST_Warning, "There were unused keys (duplicates or out of sequence) on KeyFramer: %s", keyFramer->GetName() );

	// add the key data to the world
	if( !AddKeyData( region, world, keyFramer, keyData ) )
		return false;

	return true;
}


// convert a key data file to blind keyframer data
static bool ConvertKeyFramerFile( CEditRegion* region, CPreMainWorld* world, CWorldNode* keyFramer, const char* baseKeyName )
{
	// get the name of the external key file
	CPropList* propList = keyFramer->GetPropertyList();
	CStringProp* prop = (CStringProp*)(propList->GetMatchingProp( "ExternalKeyFile", PT_STRING ));
	const char* keyFileName = prop->GetString();

	// keyframer just doesn't have any key data
	if( !keyFileName || !keyFileName[0] )
		return true;

	// get the keyframer transform (key transforms are relative to the keyframer object)
	LTVector kfPos = keyFramer->GetPos();
	LTVector kfRot = keyFramer->GetOr();

	// build up the various transforms needed to tweak potentially rotated and translated keyframers
	LTMatrix kfOffset;

	LTRotation quat( kfRot.x, kfRot.y, kfRot.z );
	quat.ConvertToMatrix( kfOffset );

	LTMatrix transMat;
	transMat.Identity();
	transMat.SetTranslation( kfPos );

	kfOffset = transMat * kfOffset;

	LTMatrix kfRotOffset;
	quat.ConvertToMatrix( kfRotOffset );

	LTMatrix kfInvRotOffset = kfRotOffset;
	kfInvRotOffset.Inverse();

	CLTAReader reader;
	std::vector<KeyData*> keyData;

	// get the complete path to the key data file
	char fullName[MAX_PATH];
	sprintf( fullName, "%s\\%s", g_pGlobs->m_ProjectDir, keyFileName );

	// open the lta file for reading
	if( reader.Open( fullName, CLTAUtil::IsFileCompressed( fullName ) ) )
	{
		// grab the keys root level node
		CLTADefaultAlloc Allocator;

		CLTANode* root = CLTANodeReader::LoadNode( &reader, "keys", &Allocator );
		CLTANode* keyList;

		// grab the list of keys
		if( root && root->IsList() )
			keyList = root->GetElement( 1 ); 

		if( root && keyList && keyList->IsList() )
		{
			CLTANode* curKeyNode;
			int curKeyNum = 0;

			// allocate enough for all the keys in the file (even though they may not all be for this keyframer)
			keyData.reserve( keyList->GetNumElements() );

			// keep grabbing keys until one isn't found
			while( curKeyNode = FindKeyFromFile( keyList, baseKeyName, curKeyNum ) )
			{
				KeyData* newKeyData = new KeyData( curKeyNode, kfOffset, kfInvRotOffset, kfRotOffset );
				keyData.push_back( newKeyData );
				curKeyNum++;
			}
		}

		if( root )
			Allocator.FreeNode( root );
	}
	else
	{
		DrawStatusText(eST_Error, "Failed to open keyframer file %s for reading.", fullName);
		return false;
	}

	reader.Close();

	// add the key data to the world
	if( !AddKeyData( region, world, keyFramer, keyData ) )
		return false;

	return true;
}


static bool ConvertKeyFramer( CEditRegion* region, CPreMainWorld* world, CWorldNode* keyFramer )
{
	std::vector<CWorldNode*> keys;

	// get the base key name
	CStringProp* prop = (CStringProp*)(keyFramer->GetPropertyList()->GetMatchingProp( "BaseKeyName", PT_STRING ));

	// initialize the keyframe data index to point to no data
	CRealProp* indexProp = (CRealProp*)keyFramer->GetPropertyList()->GetMatchingProp( "KeyDataIndex", PT_REAL );

	if( !prop || !indexProp )
	{
		DrawStatusText(eST_Error, "Found out of date object.  Please update objects before reprocessing.");
		return false;
	}

	indexProp->SetValue( -1.0f );

	// find any objects that match the base key name
	uint32 numKeys = FindObjectsWithBaseName( region, prop->GetString(), keys );

	if( numKeys )
	{
		// found valid keys, convert them to blind data
		return ConvertKeyFramerObjects( region, world, keyFramer, prop->GetString(), keys );
	}
	else
	{
		// try to convert a key data file to blind data
		return ConvertKeyFramerFile( region, world, keyFramer, prop->GetString() );
	}
}


// given an edit region, it will go through finding keyframers
// Key data for these keyframers will be packed into
// blind object data chunks, and all key objects will be
// removed from the edit region
bool ConvertKeyData( CEditRegion* region, CPreMainWorld* world )
{
	std::vector<CWorldNode*> keyFramers;

	FindObjectsOfType( region, "KeyFramer", keyFramers );

	std::vector<CWorldNode*>::iterator it = keyFramers.begin();

	for( ; it != keyFramers.end(); it++ )
	{
		if( !ConvertKeyFramer( region, world, *it ) )
			return false;
	}

	return true;
}
