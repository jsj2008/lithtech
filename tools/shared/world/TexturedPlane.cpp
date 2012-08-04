#include "bdefs.h"
#include "texturedplane.h"
#include "ltamgr.h"
#include "ltasaveutils.h"
#include "de_world.h"

#ifdef DIRECTEDITOR_BUILD
	#include "texture.h"
	#include "edithelpers.h"
#endif

CTexturedPlane::CTexturedPlane()
{
#ifdef DIRECTEDITOR_BUILD
	m_pTextureFile			= NULL;
	m_pDetailTextureFile	= NULL;
#endif

	m_pTextureName			= "Default";

	P.Init( 1.0f, 0.0f, 0.0f );
	Q.Init( 0.0f, 0.0f, 1.0f );
	O.Init( 0.0f, 0.0f, 0.0f );

}

CTexturedPlane::CTexturedPlane(const CTexturedPlane& rhs)
{
#ifdef DIRECTEDITOR_BUILD
	m_pTextureFile			= NULL;
	m_pDetailTextureFile	= NULL;
#endif

	m_pTextureName			= "Default";

	P.Init( 1.0f, 0.0f, 0.0f );
	Q.Init( 0.0f, 0.0f, 1.0f );
	O.Init( 0.0f, 0.0f, 0.0f );

	CopyTextureAttributes(&rhs, NULL);
}

CTexturedPlane::~CTexturedPlane()
{
}

void CTexturedPlane::CopyTextureAttributes(const CTexturedPlane *pOther, CStringHolder *pStringHolder )
{
	ASSERT(pOther);

	O = pOther->O;
	P = pOther->P;
	Q = pOther->Q;

	if (pStringHolder)
	{
		m_pTextureName=pStringHolder->AddString(pOther->m_pTextureName);
	}
	else
	{
		m_pTextureName=pOther->m_pTextureName;
	}	

#ifdef DIRECTEDITOR_BUILD
	m_pTextureFile			= pOther->m_pTextureFile;
	m_pDetailTextureFile	= pOther->m_pDetailTextureFile;
#endif

}

void CTexturedPlane::LoadTextureInfoTBW( CAbstractIO& InFile, CStringHolder* pStringHolder)
{
	//read in the OPQ vectors
	InFile >> O.x;
	InFile >> O.y;
	InFile >> O.z;

	InFile >> P.x;
	InFile >> P.y;
	InFile >> P.z;

	InFile >> Q.x;
	InFile >> Q.y;
	InFile >> Q.z;

	char pszTempString[_MAX_PATH];
	InFile.ReadString(pszTempString, sizeof(pszTempString));
	m_pTextureName = pStringHolder->AddString(pszTempString);
}

void CTexturedPlane::SaveTextureInfoTBW( CAbstractIO& OutFile )
{
	//save out the OPQ vectors
	OutFile << O.x;
	OutFile << O.y;
	OutFile << O.z;

	OutFile << P.x;
	OutFile << P.y;
	OutFile << P.z;

	OutFile << Q.x;
	OutFile << Q.y;
	OutFile << Q.z;

	//now write out the texture name
	OutFile.WriteString(m_pTextureName);
}

void CTexturedPlane::LoadTextureInfoLTA( CLTANode* pNode, CStringHolder* pStringHolder)
{
	CLTANode* pTemp;
	pTemp = pNode->GetElement(1);// PairCdrNode( shallow_find_list(pTextureInfo, "O") );
	O.x = GetFloat( pTemp->GetElement(0) );
	O.y = GetFloat( pTemp->GetElement(1) );
	O.z = GetFloat( pTemp->GetElement(2) );
	pTemp = pNode->GetElement(2); // PairCdrNode( shallow_find_list(pTextureInfo, "P") );
	P.x = GetFloat( pTemp->GetElement(0) );
	P.y = GetFloat( pTemp->GetElement(1) );
	P.z = GetFloat( pTemp->GetElement(2) );
	pTemp = pNode->GetElement(3); //PairCdrNode( shallow_find_list(pTextureInfo, "Q") );
	Q.x = GetFloat( pTemp->GetElement(0) );
	Q.y = GetFloat( pTemp->GetElement(1) );
	Q.z = GetFloat( pTemp->GetElement(2) );

	m_pTextureName = pStringHolder->AddString( PairCdr( pNode->GetElement(5) /*shallow_find_list(pTextureInfo, "name")*/ ) );
}

void CTexturedPlane::SaveTextureInfoLTA( CLTAFile* pFile, uint32 level )
{
	PrependTabs(pFile, level);
	pFile->WriteStr("( textureinfo ");
		PrependTabs(pFile, level+1);
		pFile->WriteStrF("( %f %f %f )", O.x, O.y, O.z );
		PrependTabs(pFile, level+1);
		pFile->WriteStrF("( %f %f %f )", P.x, P.y, P.z );
		PrependTabs(pFile, level+1);
		pFile->WriteStrF("( %f %f %f )", Q.x, Q.y, Q.z );
		PrependTabs(pFile, level+1);
		pFile->WriteStrF("( sticktopoly %d )", 1 );
		PrependTabs(pFile, level+1);
		if( strlen(m_pTextureName) )
		{
			pFile->WriteStrF("( name \"%s\" )", m_pTextureName );
		}
		else
		{
			ASSERT( 0 );	// the texture must have a name, something is clearing it for some reason
			pFile->WriteStr("( name \"Default\" )" );
		}
	PrependTabs(pFile, level);
	pFile->WriteStr(")");
}

void CTexturedPlane::SaveFlagsLTA(CLTAFile* pFile, uint32 level)
{
	PrependTabs(pFile, level);
	pFile->WriteStr("( flags ) " );
}

void CTexturedPlane::SaveShadeLTA(CLTAFile* pFile, uint32 level)
{
	PrependTabs(pFile, level);
	pFile->WriteStrF("( shade 0 0 0 ) ");
}

void CTexturedPlane::SavePhysicsMaterialLTA( CLTAFile* pFile, uint32 level )
{
	PrependTabs(pFile, level);
	pFile->WriteStrF("( physicsmaterial \"Default\" )" );
}

void CTexturedPlane::SetTextureSpace(const LTVector& N, const LTVector& newO, const LTVector& newP, const LTVector& newQ)
{
	O = newO;
	P = newP;
	Q = newQ;

	//see if they are close to being perpendicular to each other
	//if they are we don't need to do anything, but otherwise
	//we need to reset them so they are perpindicular. This fixes
	//the bug where through continual realigning, the PQ vectors
	//would collapse on each other -JohnO
	//
	// Also, we need to preserve texture alignments, so the textures
	// don't get messed up whenever we re-orthogonalize the P and Q. -David C.

	if( fabs(P.Dot(Q)) > 0.001f )
	{	
		// Make sure P and Q are perpendicular, while preserving the texture alignment

		const float dot = P.Dot(Q); // dot product of P and Q

		float t; // scaling amount

		float a, b, c; // variables in quadratic formula

		// What we're going to do here is move the P and Q vectors along the line of the 
		// surface normal until they become perpendicular.  Which direction we move them
		// depends on whether the angle they make is acute or oblique.

		if (dot < 0.0f) // oblique angle
		{ 

			a = (N.x * N.x) + (N.y * N.y) + (N.z * N.z);

			b = (P.x * N.x) + (Q.x * N.x) + 
				(P.y * N.y) + (Q.y * N.y) +
				(P.z * N.z) + (Q.z * N.z) ;

			c = (P.x * Q.x) + (P.y * Q.y) + (P.z * Q.z);
		}
		if (dot > 0.0f) // acute angle
		{ 

			a = -(N.x * N.x) - (N.y * N.y) - (N.z * N.z);

			b = (P.x * N.x) - (Q.x * N.x) + 
				(P.y * N.y) - (Q.y * N.y) +
				(P.z * N.z) - (Q.z * N.z) ;

			c = (P.x * Q.x) + (P.y * Q.y) + (P.z * Q.z);
		}

		// quadratic formula

		t = (-b + sqrtf((b * b) - (4 * a * c))) / (2 * a);


		if (dot < 0.0f) 
		{
			P = P + (N * t);
			Q = Q + (N * t);
		}

		if (dot > 0.0f) 
		{
			P = P - (N * t);
			Q = Q + (N * t);
		}	
	}
}


#ifdef DIRECTEDITOR_BUILD

#	define DETAIL_TEXTURE_TAG		"DETAILTEX"
#	define DETAIL_TEXTURE_TAG_LEN	9				//the length of the above string

	//determines the name of the detail texture 
	static bool GetDetailTextureFilename(const char* pszCommandString, CString& sFilename)
	{
		//create a copy of the command string
		CString sCommandString(pszCommandString);

		//uppercase it, for caseless comparisons
		sCommandString.MakeUpper();

		int nFindPos = sCommandString.Find(DETAIL_TEXTURE_TAG);

		if(nFindPos == -1) 
		{
			//no name was found
			return false;
		}

		//strip off the left hand side
		sCommandString = sCommandString.Mid(nFindPos + DETAIL_TEXTURE_TAG_LEN);

		//now we have the filename from here to the end of the string or a semicolon
		int nEndPos = sCommandString.Find(';');

		if(nEndPos != -1)
		{
			sCommandString = sCommandString.Left(nEndPos);
		}

		//now just do some clean up
		sCommandString.TrimLeft();
		sCommandString.TrimRight();

		//success
		sFilename = sCommandString;
		return true;
	}

	static BOOL IsSpriteFilename(char *pName)
	{
		if(strlen(pName) > 3)
			return stricmp(&pName[strlen(pName)-3], "spr") == 0;
		else
			return FALSE;
	}

	void CTexturedPlane::UpdateTextureID()
	{
		CMoFileIO file;
		DWORD i, nFrames, frameRate, bTransparent, bTranslucent, key;
		char str[256];

		m_pTextureFile			= NULL;
		m_pDetailTextureFile	= NULL;

		if(m_pTextureName)
		{
			if(IsSpriteFilename(m_pTextureName))
			{
				// Get the first frame for the sprite.
				if(dfm_OpenFileRelative(GetFileMgr(), m_pTextureName, file))
				{
					file >> nFrames;
					file >> frameRate;
					file >> bTransparent;
					file >> bTranslucent;
					file >> key;

					if(nFrames > 0)
					{
						if(file.ReadString(str, 256))
						{
							dfm_GetFileIdentifier(GetFileMgr(), str, &m_pTextureFile);
						}
					}
					
					file.Close();
				}
				return;
			}
			else
			{
				dfm_GetFileIdentifier(GetFileMgr(), m_pTextureName, &m_pTextureFile);
			}
		}

		// load detail texture		(YF 1/10/01)
		if( m_pTextureFile )
		{
			CTexture* pTexture = dib_GetDibTexture( m_pTextureFile );

			if( pTexture )
			{
				CString sFilename;
				if( GetDetailTextureFilename( pTexture->m_Header.m_CommandString, sFilename ) )
				{
					dfm_GetFileIdentifier( GetFileMgr(), sFilename, &m_pDetailTextureFile );
				}
			}
		}
	}

	//used to get the dimensions of the texture
	uint32 CTexturedPlane::GetBaseTextureWidth()
	{
		//get the dimensions of the texture
		CTexture* pTexture = dib_GetDibTexture(m_pTextureFile);

		if(pTexture)
		{
			return pTexture->m_pDib->GetWidth();
		}

		return 0;
	}

	uint32 CTexturedPlane::GetBaseTextureHeight()
	{
		//get the dimensions of the texture
		CTexture* pTexture = dib_GetDibTexture(m_pTextureFile);

		if(pTexture)
		{
			return pTexture->m_pDib->GetHeight();
		}

		return 0;
	}

#endif // DIRECTEDITOR_BUILD