// lta2abcpc.cpp : Defines the entry point for the console application.
//

// engine/sdk
#include "bdefs.h"
#include <iltstream.h>
#include <sysstreamsim.h>
#include <model.h>
#include "ltaModel.h"
#include "ltaSave.h"
#include "lta2ltb_d3d.h"
#include "tdguard.h"

// lta-parse
#include <ltamgr.h>

// c++
#include <iostream>
#include <fstream>

#include "commandline_parser.h"

#define LT_VERSION "-[Jupiter]-"

CommandLineParser g_cmdLine;	// The Command Line...

bool g_bProgress;

LTRESULT LoadChildCallBack(ModelLoadRequest *pRequest, Model **ppModel);

// ------------------------------------------------------------------------
// the output to std out call back 
// ------------------------------------------------------------------------
static 
void VerboseOutputCB( const char *str )
{
	cout << str ;
}

// Possible return values:
enum EReturnValue
{
	RET_SUCCESS				= 0,
	RET_INVALID_PARAMS		= 1,
	RET_FILE_OPEN_FAILED	= 2,
	RET_FILE_READ_FAILED	= 3,
	RET_FILE_WRITE_FAILED	= 4
};


const char *szAnimCompressionType [] ={ 
	"None" , "Relevant (RLE)", "Relevant Compressed (RLE 16bit)",
		"(RLE 16bit) for Player View Models" };

// ------------------------------------------------------------------------
// lta packer
// 
// open up the lta file parse it in, interpret into model, call model save 
// with export name... 
// ------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		return 1;
	}


	const bool bLoadChildModelsAsRefs = true ;
	LAllocCount AllocCount(&g_DefAlloc);
	EReturnValue RetVal = RET_SUCCESS; // Return value. Set to indicate error.
	bool bVerbose = false ;
	// default number of quad slots in a render patch (ps2)
    uint32 iMaxPatchMemory = 500; // quad slots 
	Model  *pNewModel;
	const char *pInFileName; char *pOutFileName ;

	g_cmdLine.SetCommandLine(argc,argv); 
	bool bInvalidParams = false;
	
	if (!g_cmdLine.GetProcessCmd())	
	{ 
		cerr << "No compile output format specified" << endl; 
		OutputDebugString("No compile output format specified"); 
		bInvalidParams = true; 
	}
	if (!g_cmdLine.GetInFile())		
	{  
		cerr << "No input file specified" << endl; 
		OutputDebugString("No input file specified"); 
		bInvalidParams = true; 
	}
	if (!g_cmdLine.InPlace() && !g_cmdLine.GetOutFile() )	
	{ 
		cerr << "No output file specified" << endl; 
		OutputDebugString("No output file specified"); 
		bInvalidParams = true; 
	}
	
	if (bInvalidParams) {
		RetVal = RET_INVALID_PARAMS;
		cout << "Usage:" << endl;
		cout << "Model_Packer CONVERSION_DIRECTIVE -input filename.lta -output outputfilename.ltb -verbose" << endl;
		cout << "Model_Packer CONVERSION_DIRECTIVE -inplace filename.lta -verbose" << endl;
		cout << "Packer Conversion Directives :" << endl;
		
		cout << "\nd3d : "<< endl;
		cout << "\tCompiles lta to an ltb with d3d (DirectX) optimizations."<<endl;
		cout << "\tOptions : " << endl;
		cout << "\tMaxBonesPerVert, MaxBonesPerTri,\n MinBoneWeight, ReIndexBones" << endl;
		cout << "\tUseMatrixPalettes " << endl;
		
			
		cout << "\t-inplace will change the lta/ltc into ltb as the out put name." << endl;
		cout << "\t-ac0     forces packer to not compress." << endl;
		cout << "\t-ac1     compress animations, saving only relevant info." << endl;
		cout << "\t-ac2     compress animations even more. Possibly losing float precision."<<endl;
		cout << "\t-acpv    animation compression 16 bit (for rotations only), plus rle. " <<endl;
		cout << "\t-nogeom  don't put geometry into ltb file. (space saving)."<<endl;

		cout << "\n\tnote: don't forget to convert child models as well." << endl;
		cout << endl;
		
		cout << endl<< __DATE__ << "\t\t" << LT_VERSION << endl;
		
		return RetVal; 
	}
	
	
	if( g_cmdLine.hasVerbose() )
	{
		CLTATranslStatus::SetLoadLogCB(VerboseOutputCB);
		bVerbose = true ;
	}

		// open input file 
	if( bVerbose ) {
		cout <<"processing : " << g_cmdLine.GetInFile() ;
		if( !g_cmdLine.InPlace() )
		 cout << " --> " << g_cmdLine.GetOutFile() << endl;
		else cout << endl;
	}

	pInFileName = g_cmdLine.GetInFile();
	if( g_cmdLine.InPlace() )
	{
		uint32 size = strlen(pInFileName);
		pOutFileName = strdup(pInFileName);
		pOutFileName[ size -1 ] = 'b';
	}
	else {
		pOutFileName = (char*)g_cmdLine.GetOutFile();
	}

	pNewModel = ltaModelLoad( pInFileName, AllocCount ,bLoadChildModelsAsRefs );
	if( pNewModel == NULL )
	{
		cerr << "could open or process " << g_cmdLine.GetInFile() << " exiting " << endl;
		RetVal = RET_FILE_READ_FAILED;
		return RetVal ;
	}

	// Go through the known process commands...
	if (stricmp(g_cmdLine.GetProcessCmd(),"compile_ltb_d3d")==0  ||
	   (stricmp(g_cmdLine.GetProcessCmd(),"d3d")==0)) 
	{
		// Get some params...
		int    iMaxBonesPerVert					= 4;
        int    iMaxBonesPerTri					= 99; 
		bool   bUseMatrixPalettes				= false;
		float  fMinWeightPerBone				= 0.05f;
		bool   bReIndexBones					= true;
		bool   bExportJustPieceLOD				= false;
		bool   bExportGeom						= g_cmdLine.ExportGeom();
		EANIMCOMPRESSIONTYPE AnimCompressionType= g_cmdLine.GetAnimCompressionType();

		// model over-rides.
		if(!g_cmdLine.GetAnimCompressionTypeOverride())
		{
			AnimCompressionType = (EANIMCOMPRESSIONTYPE)pNewModel->m_CompressionType ;
		}
		if(!g_cmdLine.ExportGeomOverride()) 
		{		
			bExportGeom         = !pNewModel->m_bExcludeGeom ;
		}
 
		string szPieceLODName;
		uint32 StreamData[4];

		if (g_cmdLine.GetParamVal("MaxBonesPerVert"))
            iMaxBonesPerVert   = atoi(g_cmdLine.GetParamVal("MaxBonesPerVert"));

		if (g_cmdLine.GetParamVal("MaxBonesPerTri"))
            iMaxBonesPerTri    = atoi(g_cmdLine.GetParamVal("MaxBonesPerTri"));

		if (g_cmdLine.GetParamVal("MinBoneWeight"))
            fMinWeightPerBone  = (float)atof(g_cmdLine.GetParamVal("MinBoneWeight"));

		if (g_cmdLine.GetParamVal("UseMatrixPalettes")) {
            bUseMatrixPalettes = atoi(g_cmdLine.GetParamVal("UseMatrixPalettes")) ?  true : false; }

		if (g_cmdLine.GetParamVal("ReIndexBones")) {
			bReIndexBones = atoi(g_cmdLine.GetParamVal("ReIndexBones")) ? true : false; }

		if (g_cmdLine.GetParamVal("ExportJustRenderObject")) {
			bExportJustPieceLOD = true;
			szPieceLODName = g_cmdLine.GetParamVal("ExportJustRenderObject"); 
		}
		


		if ( bUseMatrixPalettes ) iMaxBonesPerVert = MIN(iMaxBonesPerVert,4);	// Always force it - even if they say not to - because these are the real limits...
		else iMaxBonesPerTri  = MIN(iMaxBonesPerTri,4);

		for (uint32 i = 0; i < 4; ++i) { StreamData[i] = g_cmdLine.GetSteamFlags(i); }

		

		C_LTB_D3D_File MyFile;
		
		if (!MyFile.OpenFile(pOutFileName)) {
			cout << "Error opening file." << pOutFileName << endl; 
		}
		else 
		{
			if (bVerbose ) 
			{	
				cerr << "MaxBonesPerVert "<< iMaxBonesPerVert << endl;
				cerr << "MaxBonesPerTri "<< iMaxBonesPerTri << endl;
				cerr << "MinBoneWeight "<< fMinWeightPerBone << endl;
				cerr << "UseMatrixPalettes "<< bUseMatrixPalettes << endl; 
				cerr << "ReIndexBones" << bReIndexBones << endl; 
				if( !bExportGeom ) cerr << "Excluding Geometry"<< endl;
				cerr << "AnimCompressionType : " << szAnimCompressionType[ int(AnimCompressionType) ] << endl;
			}


			if (bExportJustPieceLOD) // export just the pieces.
			{
				if (!MyFile.ExportD3DFile_JustPieceLOD(
													pNewModel,
													szPieceLODName.c_str(),
													iMaxBonesPerVert,
													iMaxBonesPerTri,
													bUseMatrixPalettes,
													fMinWeightPerBone,
													bReIndexBones,
													StreamData)) 
				{
					cerr << "Error render object file." << endl; 
					RetVal = RET_FILE_OPEN_FAILED; 
				} 
				
			}
			else // export the whole file
			{
				if (!MyFile.ExportD3DFile(	pNewModel, 
											iMaxBonesPerVert,
											iMaxBonesPerTri,
											bUseMatrixPalettes,
											fMinWeightPerBone,
											bReIndexBones,
											StreamData,
											AnimCompressionType,
											bExportGeom)) 
				{
					cerr << "Error exporting file." << endl; 
					RetVal = RET_FILE_OPEN_FAILED; 
				} 
			} 
		}
	}
	else {
		cerr << g_cmdLine.GetProcessCmd() << " is an unknown command " << endl;
	}

	if( bVerbose )
	{
		extern void GetAnimOutputReport( ostream & );
		GetAnimOutputReport(cout);
	}
	
	// see if the user wants to stall ( this is for when ModelEdit calls 
	// the packer so that the user can see the output )
	if( g_cmdLine.hasStall() )
	{
		cout << endl << "Press Enter to continue." << endl;
		while( cin.peek() == EOF )
			;
	}

	return RetVal; 
}



// ------------------------------------------------------------------------
// IsLtaFile
// does this filename have an lta extention, (case insensitive)
// ------------------------------------------------------------------------
static inline 
int IsFilenameLTAExt( const char *filename )
{
	string pathName ;

	// not enough chars in the name  
	if( strlen( filename ) < 4 )
		return 0;

	pathName = filename ; 
	int pos= pathName.size() - 4;
	string szFileExt( pathName, pos , pos + 4)  ;

	//  tolower() on the string
	for( int i = 0 ; i < (int)szFileExt.size() ; i ++ )
	{
		szFileExt[i] = tolower( szFileExt[i] );
	}

	if( szFileExt == ".lta" )
		return 1;
	else return 0;

}

// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
Model* LoadChildModel(ModelLoadRequest *pRequest, char *pBaseName, char *pChildFilename)
{
	char dirName[256], newFilename[256];
	Model *pModel;

	
	CHelpers::ExtractNames(pBaseName, dirName, NULL, NULL, NULL);
	if (dirName[0] == 0)
	{
		sprintf(newFilename, "%s", pChildFilename);
	}
	else
	{
		sprintf(newFilename, "%s\\%s", dirName, pChildFilename);
	}

	int base_is_lta = IsFilenameLTAExt( pBaseName ) ;
	int child_is_lta= IsFilenameLTAExt( pChildFilename );

	if( base_is_lta && child_is_lta )
	{
		cerr << "Programer Error Report this as a bug.\r\nCannot Load LTA Child Models From ME";
		cerr << "Error in ChildModel Load" << endl;
		pModel = NULL ;

			return pModel;

	}

	return NULL;
}

// ------------------------------------------------------------------------
// Load Child Call back 
// called by Model::Load 
// ------------------------------------------------------------------------
LTRESULT LoadChildCallBack(ModelLoadRequest *pRequest, Model **ppModel)
{
	return LT_NOCHANGE ;

	*ppModel = LoadChildModel(pRequest, (char*)pRequest->m_pLoadFnUserData, pRequest->m_pFilename);
	if (*ppModel)
		return LT_OK;
	else
		return LT_NOCHANGE;
}



// ------------------------------------------------------------------------
// convert-modelchild-filename( input-string, output-string, error-string)
//
// converts incomming child model filename to name-root + ".ltb"
// 
// returns 1 on ok, 0 fail, stuffs error in err_string 
// ------------------------------------------------------------------------
int convert_modelchild_filename( char *filename, string & result, string & err_str  )
{
	string str = filename;
	string new_ext = ".ltb";

	// if the string is shorter than 4 and does not lead with a '.' then we're ok
	// otherwise it's an error.
	if( str.length() <= 4 )
	{
		
		if( str.length()== 0 )
		{
			cout << " 0 len case " << endl;
			err_str = filename ;
			err_str += " is not a filename " ;
			return 0;
		}
		
		if( str[0] != '.' )
		{
			if( str[0] == '\"' && str[1] == '\"' )
			{
				// because of a bug in the lta reader, a filename can be litterally "" so
				// turns the name to a empty string.
				result = "";
				err_str = "Empty filename for child model ";
				return 0;
			}

			// we have a regular string that is less then or equal to four chars long. 
			result = str ;
			result += new_ext ;
			return 1 ;
		}
		
		// if we've failed all ifs then we're in an error state.

		err_str = "\"";
		err_str += filename ;
		err_str += "\" isn't formatted correctly, must have no extension and be a letter or more long." ;
		return 0;
	
	
	}

	// look for lta/ltb/abc extensions
	// older lta files or files that have been tweaked by hand 
	// may have extensions in the filenames of child-model node.
	// LTA
	if( str.substr( str.length() - 4, str.length() ).compare( ".lta" ) == 0 )
	{
		result = str.substr( 0, str.length() - 4 );
		result += new_ext ;
		return 1;
	}
	// LTB
	if( str.substr( str.length() - 4, str.length() ).compare( ".ltb" ) == 0 )
	{
		cout << "WARNING: ltb file extension for child-model file name: " << str<< endl;
		result = str.substr( 0, str.length() - 4 );
		result += new_ext ;
		return 1;
	}
	// ABC
	if( str.substr( str.length() - 4, str.length() ).compare( ".abc" ) == 0 )
	{
		cout << "WARNING: abc file extension for child-model file name: " << str <<  endl;
		result = str.substr( 0, str.length() - 4 );
		result += new_ext ;
		return 1;
	}

	result = str ;
	result += new_ext ;
	return 1;
}

// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
	return malloc(size);
}

void DefStdlithFree(void *ptr)
{
	free(ptr);
}




