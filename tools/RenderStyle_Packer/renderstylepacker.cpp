// ------------------------------------------------------------------------
// file: renderstylepacker.cpp
// desc: functional file for packer.
//
// lithtech(c) 2001
// -----------------------------------------------------------------------


#include <d3dx9.h>
#include <stdio.h>
#include <ltinteger.h>
#include <ltbasedefs.h>
#include "ltamgr.h"
#include "ltb.h"
#include "Utilities.h"
#include "d3d_renderstyle.h"
#include "renderstylepacker.h"
#include "tdguard.h"



// globals
bool		g_VerboseMode		= false;
bool		g_bWin				= false;
bool		g_bNoAutoStart		= false;
string		g_InputFile[MAX_RENDERSTYLE_INPUTS], g_OutputFile;

char		g_StartingDirectory[MAX_PATH];
string		g_WorkingDir;

CLTADefaultAlloc	g_LTAAlloc;

E_LTB_FILE_TYPES g_PackType = LTB_D3D_RENDERSTYLE_FILE;

bool LoadLTARenderStyle		(CLTANode &root, const char *szFileName);
void SetPlatform			(const char *str );
void PackIt					(string szInputFile[], const char* szOutputFile);
bool LoadRenderStyle		(CD3DRenderStyle* pRenderStyle, const char* szFileName);
bool LoadRenderStyle		(CRenderStyle*, CLTANode *);

bool PackedRenderStyle		(CD3DRenderStyle* pRenderStyle, FILE* f, uint32 iOverallPass, uint32* pSize);
void ShutDownApp();

void SetProgress( uint32 );

// ------------------------------------------------------------------------
// Set the Current platform type from a string. If nothing valid, revert
// to default.
// ------------------------------------------------------------------------
void SetPlatform( const char *str )
{
	if( stricmp(str, "ps2" ) == 0 )
	{
		g_PackType = LTB_PS2_RENDERSTYLE_FILE ;
	}
	else if( stricmp( str, "d3d" ) == 0 )
	{
		g_PackType = LTB_D3D_RENDERSTYLE_FILE ;
	}
}


// ------------------------------------------------------------------------
// [true/false] LoadLTARenderStyle( root-node, filename )
// opens up lta ascii file, appends it to existing node tree.
// ------------------------------------------------------------------------
bool LoadLTARenderStyle( CLTANode &root, const char *szFileName  )
{
	CLTAReader InFile;
	InFile.Open(szFileName, CLTAUtil::IsFileCompressed(szFileName));

	if( InFile.IsValid() )
	{
		CLTANodeReader::LoadEntireFile( InFile, &root, &g_LTAAlloc);
		return true ;

	}
	return false ;
}

// ------------------------------------------------------------------------
// PackIt
// entry point
// ------------------------------------------------------------------------
void PackIt(string szInputFile[], const char* szOutputFile)
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return;
	}

	FILE* f = NULL;
	uint32 i = 0, iRenderStyleCount = 0, iPass = 0, iTotalSize = 0;
	LTB_Header Header;

	// count up the imput files
	if (szInputFile[0].empty())								{ OutputMsg("Error: Invalid input file."); goto BAIL_AND_EXIT; }
	if (!szOutputFile || !szOutputFile[0])					{ OutputMsg("Error: Invalid output file."); goto BAIL_AND_EXIT; }
	while (!szInputFile[iRenderStyleCount].empty() && (iRenderStyleCount < MAX_RENDERSTYLE_INPUTS)) { ++iRenderStyleCount; }

	switch( g_PackType )
	{
	case LTB_D3D_RENDERSTYLE_FILE :
	{
		CD3DRenderStyle RenderStyle;
		uint32 rs_cnt ;

		// Start writing the packed file...
		f = fopen(szOutputFile,"wb");
		if (!f) {
			OutputMsg("Error: Couldn't save output file %s",szOutputFile);
			goto BAIL_AND_EXIT;
		}

		// Write out the header...
		Header.m_iFileType	= LTB_D3D_RENDERSTYLE_FILE;
		Header.m_iVersion	= RENDERSTYLE_D3D_VERSION;
		fwrite(&Header,1,sizeof(Header),f);


		// load up all the renderstyles into an lta node tree
		CLTANode lta_root ;

		// we load all the renderstyles into one node tree.
		for( rs_cnt = 0 ; rs_cnt < iRenderStyleCount; ++rs_cnt )
		{
			if( !LoadLTARenderStyle( lta_root, szInputFile[rs_cnt].c_str() ) )
			{
				OutputMsg("Error: Couldn't load input file %s",szInputFile[i].c_str());
				goto BAIL_AND_EXIT;
			}

		}

		// if we got anything in the tree..
		if( lta_root.GetNumElements() > 0 )
		{
			// create an array of renderstyles
			vector< CRenderStyle*> vRenderStylesFromLTA ;
			vector<CLTANode*> RenderStyles;

			// find all the renderstyles in the lta tree.
			CLTAUtil::FindAll( RenderStyles,&lta_root,"renderstyle");

			// if there's none, bail
			if (RenderStyles.empty()) { goto BAIL_AND_EXIT ;}

			// allocate renderstyles & convert the lta-rs into in-core rs
			for( rs_cnt = 0 ; rs_cnt < RenderStyles.size() ; rs_cnt++ )
			{
				CD3DRenderStyle *pRenderStyle = new CD3DRenderStyle ;
				vRenderStylesFromLTA.push_back( pRenderStyle );
				pRenderStyle->SetDefaults();
				// load the lta-node rs into in-core rs.
				LoadRenderStyle( pRenderStyle,RenderStyles[rs_cnt] );
			}

			// Just figure out the size on the first pass, then write the file
			// in the second.
			for (iPass = 0; iPass < 2; ++iPass) {
				if (iPass==1) {
					fwrite(&iTotalSize,1,sizeof(iTotalSize),f);
					fwrite(&iRenderStyleCount,1,sizeof(iRenderStyleCount),f);
				}
				// for every in core rs, write it out as bin
				for (i = 0; i < vRenderStylesFromLTA.size(); ++i) {

					SetProgress((uint32)(iPass ? ((float)i * (50.0f/(float)iRenderStyleCount) + (25.0f/(float)iRenderStyleCount)) + 50.0f : ((float)i * (50.0f/(float)iRenderStyleCount) + (25.0f/(float)iRenderStyleCount))));
					if (!PackedRenderStyle( (CD3DRenderStyle*)vRenderStylesFromLTA[i],f,iPass,&iTotalSize)) {
						OutputMsg("Error: Couldn't save output file %s",szOutputFile);
						goto BAIL_AND_EXIT;
					}
					SetProgress((uint32)(iPass ? ((float)i * (50.0f/(float)iRenderStyleCount) + (50.0f/(float)iRenderStyleCount)) + 50.0f : ((float)i * (50.0f/(float)iRenderStyleCount) + (50.0f/(float)iRenderStyleCount))));
				}
			}

		}else
		{
			OutputMsg("Warning: No data in files... " );
			goto BAIL_AND_EXIT ;
		}

		lta_root.Free(&g_LTAAlloc);
	};
	break;

	default:
		{
			char buf[256];
			sprintf(buf, "ltb file type : %i", g_PackType );
			MessageBox(NULL, buf, "ERROR, UNKNOWN EXPORT TARGET TYPE", MB_OK);
		break;

		}

	} // switch

BAIL_AND_EXIT:
	if (f) { fclose(f); f = NULL; }
	ShutDownApp();
}

char* ScanParam(char* szString, char* szOutParam)
{
	uint32 iCh = 0; uint32 iOutCh = 0;					// Strip white space...
	while (szString[iCh]==' ') { ++iCh; }

	szOutParam[0] = NULL;								// Copy over as much as you can...
	while (szString[iCh]!=',' && szString[iCh]!=NULL && szString[iCh]!='>' && iOutCh < 31) {
		szOutParam[iOutCh] = szString[iCh]; ++iCh; ++iOutCh; }
	szOutParam[iOutCh] = NULL; if (iOutCh > 0) --iOutCh;

	if (szString[iCh] == ',') ++iCh;					// Skip over that comma...

	while ((iOutCh > 0) && (szOutParam[iOutCh]==' ')) {	// Strip off end white space...
		szOutParam[iOutCh] = NULL; --iOutCh; }

	return (szString + iCh);
}

// Pre-Process the shaderfile code (expand skin macros and stuff like that)...
bool PreProcessShaderFile(const char* szFileName, string& OutputCode, uint32 iType)
{
	bool bRigid = ((iType==0) ? true : false);
	FILE* f = fopen(szFileName,"r"); if (!f) return false;
	char ch[2]; ch[1] = NULL; char* pCh = NULL;			// Read in the whole file...
	while ((ch[0] = fgetc(f))!=EOF) { OutputCode += ch; }
	fclose(f);

	// LT_MACRO_IFRIGID Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_IFRIGID<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_IFRIGID<")) {		// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_IFRIGID<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();
		if (bRigid) {
			OutputCode.erase(iEndCh,1);
			OutputCode.erase(iStartCh,strlen("LT_MACRO_IFRIGID<")); }
		else {
			OutputCode.erase(iStartCh,iEndCh-iStartCh+1); } }

	// LT_MACRO_IFSKIN Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_IFSKIN<")) {		// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_IFSKIN<")) {		// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_IFSKIN<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();
		if (!bRigid) {
			OutputCode.erase(iEndCh,1);
			OutputCode.erase(iStartCh,strlen("LT_MACRO_IFSKIN<")); }
		else {
			OutputCode.erase(iStartCh,iEndCh-iStartCh+1); } }

	// LT_MACRO_RIGIDTRANS4 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_RIGIDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_RIGIDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_RIGIDTRANS4<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_RIGIDTRANS4<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (bRigid) {
			sprintf(szBuff,"dp4 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.w, %s, c%d\n",OutPos,InPos,iMatConst+3);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_RIGIDTRANS3 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_RIGIDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_RIGIDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_RIGIDTRANS3<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_RIGIDTRANS3<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (bRigid) {
			sprintf(szBuff,"dp3 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_SKINTRANS4 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINTRANS4<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINTRANS4<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) {
			sprintf(szBuff,"dp4 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp4 %s.w, %s, c%d\n",OutPos,InPos,iMatConst+3);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_SKINTRANS3 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINTRANS3<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],MatConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINTRANS3<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,MatConst);		if (strlen(MatConst)==0)	return false;
		uint32 iMatConst	= atoi(&MatConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) {
			sprintf(szBuff,"dp3 %s.x, %s, c%d\n",OutPos,InPos,iMatConst+0);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.y, %s, c%d\n",OutPos,InPos,iMatConst+1);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
			sprintf(szBuff,"dp3 %s.z, %s, c%d\n",OutPos,InPos,iMatConst+2);			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); } }

	// LT_MACRO_LIGHT_ATT Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_LIGHT_ATT<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_LIGHT_ATT<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_LIGHT_ATT<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char OutVal[16],Light[16],NormalR[16],NormalS[16],Tmp1[16],Tmp2[16],AttVec[16],szBuff[128];
		pCh += strlen("LT_MACRO_LIGHT_ATT<");
		pCh = ScanParam(pCh,OutVal);		if (strlen(OutVal)==0)		return false;
		pCh = ScanParam(pCh,Light);			if (strlen(Light)==0)		return false;
		pCh = ScanParam(pCh,NormalR);		if (strlen(NormalR)==0)		return false;
		pCh = ScanParam(pCh,NormalS);		if (strlen(NormalS)==0)		return false;
		pCh = ScanParam(pCh,Tmp1);			if (strlen(Tmp1)==0)		return false;
		pCh = ScanParam(pCh,Tmp2);			if (strlen(Tmp2)==0)		return false;
		pCh = ScanParam(pCh,AttVec);		if (strlen(AttVec)==0)		return false;
		uint32 iAttVec	= atoi(&AttVec[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		sprintf(szBuff,"dp3 %s.w, %s, %s\n",Tmp1,Light,Light);						OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"rsq %s.w, %s.w\n",Tmp2,Tmp1);								OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"mul %s, %s, %s.w\n",Light,Light,Tmp2);						OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"dst %s, %s.w, %s.w\n",Tmp1,Tmp1,Tmp2);						OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"dp3 %s.w, %s.xyz, %s.xyz\n",Tmp1,Tmp1,AttVec);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"rcp %s.w, %s.w\n",Tmp2,Tmp1);								OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		if (bRigid) { sprintf(szBuff,"dp3 %s.x, %s, %s\n",Tmp1,NormalR,Light); }
		else { sprintf(szBuff,"dp3 %s.x, %s, %s\n",Tmp1,NormalS,Light); }			OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
		sprintf(szBuff,"mul %s, %s.w, %s.x\n",OutVal,Tmp2,Tmp1);					OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff); }

	// LT_MACRO_SKINBLENDTRANS4 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINBLENDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINBLENDTRANS4<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINBLENDTRANS4<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],Weights[16],Indicies[16],TmpReg[16],TmpReg2[16],StartConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINBLENDTRANS4<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,Weights);		if (strlen(Weights)==0)		return false;
		pCh = ScanParam(pCh,Indicies);		if (strlen(Indicies)==0)	return false;
		pCh = ScanParam(pCh,TmpReg);		if (strlen(TmpReg)==0)		return false;
		pCh = ScanParam(pCh,TmpReg2);		if (strlen(TmpReg2)==0)		return false;
		pCh = ScanParam(pCh,StartConst);	if (strlen(StartConst)==0)	return false;
		uint32 iStartConst	= atoi(&StartConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) { 									// Expand the sucka...
			switch (iType) {
			case 1 :									// Two Bones...
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 2 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 3 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.zzzz, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.w, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+4);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break; } } }

	// LT_MACRO_SKINBLENDTRANS3 Macro...
	while (pCh = strstr(OutputCode.c_str(),"//LT_MACRO_SKINBLENDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),";LT_MACRO_SKINBLENDTRANS3<")) {	// Erase the commented out ones...
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		OutputCode.erase((pCh - OutputCode.c_str()),(pEndCh - OutputCode.c_str())-(pCh - OutputCode.c_str())+1); }
	while (pCh = strstr(OutputCode.c_str(),"LT_MACRO_SKINBLENDTRANS3<")) {
		char* pEndCh = strstr(pCh,">"); if (!pEndCh) return false;
		int32 iStartCh = pCh    - OutputCode.c_str();
		int32 iEndCh   = pEndCh - OutputCode.c_str();

		// Scan in the params...
		char InPos[16],OutPos[16],Weights[16],Indicies[16],TmpReg[16],TmpReg2[16],StartConst[16],szBuff[128];
		pCh += strlen("LT_MACRO_SKINBLENDTRANS3<");
		pCh = ScanParam(pCh,OutPos);		if (strlen(OutPos)==0)		return false;
		pCh = ScanParam(pCh,InPos);			if (strlen(InPos)==0)		return false;
		pCh = ScanParam(pCh,Weights);		if (strlen(Weights)==0)		return false;
		pCh = ScanParam(pCh,Indicies);		if (strlen(Indicies)==0)	return false;
		pCh = ScanParam(pCh,TmpReg);		if (strlen(TmpReg)==0)		return false;
		pCh = ScanParam(pCh,TmpReg2);		if (strlen(TmpReg2)==0)		return false;
		pCh = ScanParam(pCh,StartConst);	if (strlen(StartConst)==0)	return false;
		uint32 iStartConst	= atoi(&StartConst[1]);		// Figure out the const values...

		OutputCode.erase(iStartCh,iEndCh-iStartCh+1);	// Erase the macro...

		if (!bRigid) { 									// Expand the sucka...
			switch (iType) {
			case 1 :									// Two Bones...
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 2 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break;
			case 3 :
				sprintf(szBuff,"mul %s.x, %s.x, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mul %s, %s, %s.xxxx\n",OutPos,TmpReg,Weights);				OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.y, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.yyyy, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.z, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.zzzz, %s\n",OutPos,TmpReg,Weights,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);

				sprintf(szBuff,"mul %s.x, %s.w, c[%d].w\n",TmpReg,Indicies,iStartConst);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mov a0.x, %s\n",TmpReg);									OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.x, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+1);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.y, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+2);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp3 %s.z, %s, c[a0.x + %d]\n",TmpReg,InPos,iStartConst+3);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"dp4 %s.w, %s.xyzw, c[%d].yyyx\n",TmpReg2,Weights,iStartConst);		OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				sprintf(szBuff,"mad %s, %s, %s.wwww, %s\n",OutPos,TmpReg,TmpReg2,OutPos);	OutputCode.insert(iStartCh,szBuff);		iStartCh += strlen(szBuff);
				break; } } }

	return true;
}

bool PackedRenderStyle(CD3DRenderStyle* pRenderStyle, FILE* f, uint32 iOverallPass, uint32* pSize)
{
	uint32 iSize = 0;

	assert(f);

	if (!f) 
	{
		return false;		// Figure out the size on the first pass
	}

	for (uint32 iPass = 0; iPass < 2; ++iPass) 
	{
		if (iPass==1)		
		{
			if (!iOverallPass) 
			{
				*pSize += iSize; return true; 
			}
			fwrite(&iSize,1,sizeof(iSize),f);
		}

		// Lighting Material...
		if (iPass)			
		{ 
			fwrite(&pRenderStyle->m_LightingMaterial,1,sizeof(pRenderStyle->m_LightingMaterial),f); 
		}
		else
		{ 
			iSize += sizeof(pRenderStyle->m_LightingMaterial); 
		}

		// Render Passes...
		uint32 iRenderPasses = pRenderStyle->m_RenderPasses.size();
		if (iPass)
		{
			fwrite(&iRenderPasses,1,sizeof(iRenderPasses),f);
		}
		else
		{
			iSize += sizeof(iRenderPasses);
		}


		for (list<CD3DRenderPass>::iterator it = pRenderStyle->m_RenderPasses.begin(); it != pRenderStyle->m_RenderPasses.end(); ++it)
		{
			if (iPass)
			{
				fwrite(&(it->RenderPass),1,sizeof(it->RenderPass),f);
			}
			else
			{
				iSize += sizeof(it->RenderPass);
			}

			uint8 iHasRSD3DRenderPass = it->pD3DRenderPass ? 1 : 0;

			if (iPass)
			{
				fwrite(&iHasRSD3DRenderPass,1,sizeof(iHasRSD3DRenderPass),f);
			}
			else
			{
				iSize += sizeof(iHasRSD3DRenderPass);
			}

			if (iHasRSD3DRenderPass)
			{
				// usevertexshader
				if (iPass)
				{
					fwrite(&it->pD3DRenderPass->bUseVertexShader,1,sizeof(it->pD3DRenderPass->bUseVertexShader),f);
				}
				else
				{
					iSize += sizeof(it->pD3DRenderPass->bUseVertexShader);
				}

				// vertexshaderid
				if (iPass)
				{
					fwrite(&it->pD3DRenderPass->VertexShaderID,1,sizeof(it->pD3DRenderPass->VertexShaderID),f);
				}
				else
				{
					iSize += sizeof(it->pD3DRenderPass->VertexShaderID);
				}

				// usepixelshader
				if (iPass)
				{
					fwrite(&it->pD3DRenderPass->bUsePixelShader,1,sizeof(it->pD3DRenderPass->bUsePixelShader),f);
				}
				else
				{
					iSize += sizeof(it->pD3DRenderPass->bUsePixelShader);
				}

				// pixelshaderid
				if (iPass)
				{
					fwrite(&it->pD3DRenderPass->PixelShaderID,1,sizeof(it->pD3DRenderPass->PixelShaderID),f);
				}
				else
				{
					iSize += sizeof(it->pD3DRenderPass->PixelShaderID);
				}
			}
		}
	}

	RSD3DOptions rsD3DOptions;
	bool bRet = pRenderStyle->GetDirect3D_Options(&rsD3DOptions);	
	fwrite(&rsD3DOptions, 1, sizeof(RSD3DOptions), f);

	return true;
}

