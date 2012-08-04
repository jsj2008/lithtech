#include "AdvSound.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"

BEGIN_CLASS(AdvSound)
	ADD_LONGINTPROP(SoundFlags, 0)
	ADD_STRINGPROP(SoundFile, "")
	ADD_LONGINTPROP(VoiceType, 0 )
	ADD_LONGINTPROP(Priority, 0 )
	ADD_REALPROP_FLAG(Radius, 300.0f, PF_RADIUS)
	ADD_LONGINTPROP(Volume, 100.0f)
	ADD_REALPROP(FrequencyScale, 1.0f)
	ADD_LONGINTPROP(Pan, 0.0f)
	ADD_LONGINTPROP(ConeAngle, 360)
	ADD_LONGINTPROP(OutsideConeVolume, 100)
	ADD_VECTORPROP(RotateXYZPeriod)
END_CLASS_DEFAULT(AdvSound, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::AdvSound()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AdvSound::AdvSound() : BaseClass(OT_NORMAL)
{
	m_dwFlags = 0;
	m_hstrSoundFile = DNULL;
	m_nVoiceType = 0;
	m_nPriority = 0;
	m_fRadius = 100.0f;
	m_nVolume = 100;
	m_fFrequencyScale = 1.0f;
	m_iPan = 0;
	m_wConeAngle = 360;
	m_nOutsideConeVolume = 100;
	m_bRotate = DFALSE;
	VEC_SET( m_vRotateXYZPeriod, 0.0f, 0.0f, 0.0f );
	m_fXRotVel = 0.0;
	m_fYRotVel = 0.0;
	m_fZRotVel = 0.0;
	m_fLastTime		= 0.0f;
	m_fPitch		= 0.0f;
	m_fYaw			= 0.0f;
	m_fRoll			= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::~AdvSound()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

AdvSound::~AdvSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSoundFile)
	{
		pServerDE->FreeString(m_hstrSoundFile);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD AdvSound::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == 1.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate(fData);
			break;
		}

		default : break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL AdvSound::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	DFLOAT fFloatVal;
	long nLongVal;

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';

	pServerDE->GetPropLongInt("SoundFlags", &nLongVal);
	m_dwFlags = nLongVal;
	m_dwFlags &= ( ~PLAYSOUND_GETHANDLE & ~PLAYSOUND_GETHANDLE );
	m_dwFlags |= PLAYSOUND_ATTACHED;

	buf[0] = '\0';
	pServerDE->GetPropString("SoundFile", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSoundFile = pServerDE->CreateString(buf);
	
	pServerDE->GetPropLongInt("VoiceType", &nLongVal);
	m_nVoiceType = (unsigned short)nLongVal;

	pServerDE->GetPropLongInt("Priority", &nLongVal);
	m_nPriority = (unsigned long)nLongVal;

	pServerDE->GetPropReal("Radius", &m_fRadius);

	pServerDE->GetPropReal("Volume", &fFloatVal);
	m_nVolume = (DBYTE) fFloatVal;

	pServerDE->GetPropReal("FrequencyScale", &m_fFrequencyScale);

	pServerDE->GetPropLongInt("Pan", &nLongVal);
	m_iPan = (signed char) nLongVal;

	pServerDE->GetPropLongInt("ConeAngle", &nLongVal);
	m_wConeAngle = (D_WORD) nLongVal;

	pServerDE->GetPropLongInt("OutsideConeVolume", &nLongVal);
	m_nOutsideConeVolume = (DBYTE) nLongVal;

	pServerDE->GetPropVector("RotateXYZPeriod", &m_vRotateXYZPeriod);

	float mag = VEC_MAGSQR( m_vRotateXYZPeriod );
	if( mag > 0.0001 )
	{
		m_bRotate = DTRUE;
		
		if( m_vRotateXYZPeriod.x > 0.0f )
			m_fXRotVel = MATH_CIRCLE / m_vRotateXYZPeriod.x;
		if( m_vRotateXYZPeriod.y > 0.0f )
			m_fYRotVel = MATH_CIRCLE / m_vRotateXYZPeriod.y;
		if( m_vRotateXYZPeriod.z > 0.0f )
			m_fZRotVel = MATH_CIRCLE / m_vRotateXYZPeriod.z;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

DBOOL AdvSound::InitialUpdate(DFLOAT fInfo)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	if (fInfo == INITIALUPDATE_SAVEGAME) return DFALSE;

	DRotation rRot;
	DVector vU, vR;
	PlaySoundInfo playSoundInfo;

	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = m_dwFlags;
	strncpy( playSoundInfo.m_szSoundName, g_pServerDE->GetStringData( m_hstrSoundFile ), 100);
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_fOuterRadius = m_fRadius;
	playSoundInfo.m_fInnerRadius = m_fRadius * 0.2f;
	playSoundInfo.m_nVolume = m_nVolume;
	pServerDE->GetObjectPos( m_hObject, &playSoundInfo.m_vPosition );
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	pServerDE->PlaySound( &playSoundInfo );

	if( m_bRotate )
		pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);
	else
		pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.0f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

DBOOL AdvSound::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.001);

	DFLOAT fTime = pServerDE->GetTime();
	DFLOAT fDeltaTime = fTime - m_fLastTime;

	DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);

	if( m_fXRotVel )
		m_fPitch += m_fXRotVel * fDeltaTime;
	if( m_fYRotVel )
		m_fYaw += m_fYRotVel * fDeltaTime;
	if( m_fZRotVel )
		m_fRoll += m_fZRotVel * fDeltaTime;

	pServerDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
	pServerDE->SetObjectRotation(m_hObject, &rRot);	

	m_fLastTime = fTime;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AdvSound::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_dwFlags);
	pServerDE->WriteToMessageDWord(hWrite, m_nVoiceType);
	pServerDE->WriteToMessageDWord(hWrite, m_nPriority);
	pServerDE->WriteToMessageDWord(hWrite, m_iPan);
	pServerDE->WriteToMessageWord(hWrite, m_wConeAngle);
	pServerDE->WriteToMessageByte(hWrite, m_nVolume);
	pServerDE->WriteToMessageByte(hWrite, m_nOutsideConeVolume);
	pServerDE->WriteToMessageByte(hWrite, m_bRotate);
	pServerDE->WriteToMessageFloat(hWrite, m_fRadius);
	pServerDE->WriteToMessageFloat(hWrite, m_fFrequencyScale);
	pServerDE->WriteToMessageFloat(hWrite, m_fXRotVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fYRotVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fZRotVel);
	pServerDE->WriteToMessageFloat(hWrite, m_fPitch);
	pServerDE->WriteToMessageFloat(hWrite, m_fYaw);
	pServerDE->WriteToMessageFloat(hWrite, m_fRoll);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastTime);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSoundFile);
	pServerDE->WriteToMessageVector(hWrite, &m_vRotateXYZPeriod);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AdvSound::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AdvSound::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_dwFlags				= pServerDE->ReadFromMessageDWord(hRead);
	m_nVoiceType			= (unsigned short) pServerDE->ReadFromMessageDWord(hRead);
	m_nPriority				= (unsigned long) pServerDE->ReadFromMessageDWord(hRead);
	m_iPan					= (signed char) pServerDE->ReadFromMessageDWord(hRead);
	m_wConeAngle			= pServerDE->ReadFromMessageWord(hRead);
	m_nVolume				= pServerDE->ReadFromMessageByte(hRead);
	m_nOutsideConeVolume	= pServerDE->ReadFromMessageByte(hRead);
	m_bRotate				= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_fRadius				= pServerDE->ReadFromMessageFloat(hRead);
	m_fFrequencyScale		= pServerDE->ReadFromMessageFloat(hRead);
	m_fXRotVel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fYRotVel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fZRotVel				= pServerDE->ReadFromMessageFloat(hRead);
	m_fPitch				= pServerDE->ReadFromMessageFloat(hRead);
	m_fYaw					= pServerDE->ReadFromMessageFloat(hRead);
	m_fRoll					= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastTime				= pServerDE->ReadFromMessageFloat(hRead);
	m_hstrSoundFile			= pServerDE->ReadFromMessageHString(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vRotateXYZPeriod);

	// Now that all our data members are set, start the sound...

	InitialUpdate(INITIALUPDATE_NORMAL);
}