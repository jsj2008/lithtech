#include "bdefs.h"
#include "PreProcPackerImpl.h"
#include "IPackerUI.h"
#include "PackerProperty.h"
#include "PackerPropList.h"
#include "Processing.h"
#include "tdguard.h"
#include <string.h>

//several categories that the UI fields can go into
#define PROPTAB_COMMON		"Common"
#define PROPTAB_ADVANCED	"Advanced"

CPreProcPackerImpl g_GlobalPacker;

//file used to associate this packer with the specified filename
extern "C"  //disable name mangling
{
	__declspec(dllexport) IPackerImpl* AssociatePacker(const char* pszFilename)
	{
		return &g_GlobalPacker;
	}
}

CPreProcPackerImpl::CPreProcPackerImpl()
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
	}
}

CPreProcPackerImpl::~CPreProcPackerImpl()
{
}

//---------------------
// General Information

//this is called after the packer has been selected, and is used to get a unique
//name for this packer. This name is used in the user interface and also for
//saving settings. It should be human readable. The name must be copied into the
//passed in buffer, and must not exceed buffer size (including ending \0)
bool CPreProcPackerImpl::GetPackerName(char *pszBuffer, int nBufferSize)
{
#ifdef _DEBUG
	LTStrCpy(pszBuffer, "Debug World Processor", nBufferSize);
#else
	LTStrCpy(pszBuffer, "World Processor", nBufferSize);
#endif

	return true;
}

//---------------------
// User Interface

// Called by the application when it needs to retrieve a list
// of options that the user can set
bool CPreProcPackerImpl::RequestUserOptions(IPackerUI* pUI)
{
	//------------------------------------------------------------------
	//Common
	//project settings
	pUI->CreateProperty(CPackerInterfaceProperty("Process Settings", CPackerInterfaceProperty::TEXT_LEFT), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerInterfaceProperty("Separator", CPackerInterfaceProperty::SEPARATOR), PROPTAB_COMMON);

	pUI->CreateProperty(CPackerStringProperty("ProjectPath", "", true, true, "DEdit Project Files(*.dep)|*.dep|All Files(*.*)|*.*||", "Specifies the location of the project resources"), PROPTAB_COMMON);

	//log settings
	pUI->CreateProperty(CPackerBoolProperty("LogToFile", false, "Logs all output directly to a file. Useful for debugging"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("ErrorLog", false, "Logs all errors to a file"), PROPTAB_COMMON);

	//process settings
	pUI->CreateProperty(CPackerBoolProperty("ObjectsOnly", false, "Determines if only objects should be processed"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("IgnoreHidden", false, "If all hidden nodes should be ignored while processing"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("IgnoreFrozen", false, "If all frozen nodes should be ignored while processing"), PROPTAB_COMMON);


	//lighting settings
	//pUI->CreateProperty(CPackerInterfaceProperty("Blank", CPackerInterfaceProperty::BLANK), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerInterfaceProperty("Lighting Settings", CPackerInterfaceProperty::TEXT_LEFT), PROPTAB_COMMON);
	pUI->CreateReference("Separator", PROPTAB_COMMON);

	pUI->CreateProperty(CPackerBoolProperty("ApplyLighting", true, "If lighting should be calculated on a level"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("VolumetricAmbient", true, "Applies volumetric ambient to the level from the brush ambient values"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("LightAnimations", true, "If the game code should be used to create lightmap animations"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("Shadows", true, "If lights should be blocked by objects, causing shadows in a level"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("VertexLightingOnly", false, "If only lighting values should be taken at vertices. This effectivly disables all lightmapping"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerBoolProperty("SuperSample", true, "Specifies that lighting should be done through supersampling, causing multiple samples to be taken per lightmap texel"), PROPTAB_COMMON);
	pUI->CreateProperty(CPackerRealProperty("NumSamples", 1, true, 0, 9, "Specifies the number of samples that should be taken per lightmap texel"), PROPTAB_COMMON);

	//------------------------------------------------------------------
	//Advanced

	//the heading
	pUI->CreateProperty(CPackerInterfaceProperty("Advanced", CPackerInterfaceProperty::TEXT_LEFT), PROPTAB_ADVANCED);
	pUI->CreateReference("Separator", PROPTAB_ADVANCED);

	//output file (primarily for scripting)
	pUI->CreateProperty(CPackerStringProperty("OutFile", "", false, false, "DAT Files(*.dat)|*.dat|All Files(*.*)|*.*||", "Specifies where the output file will be created"), PROPTAB_ADVANCED);

	//number of threads that we can use
	pUI->CreateProperty(CPackerRealProperty("NumThreads", 0, true, 0, 40, "Specifies how many threads that processor can utilize"), PROPTAB_ADVANCED);

	//the BSP weight used
	pUI->CreateProperty(CPackerRealProperty("BalanceWeight", 0.9f, false, "Specifies the emphasis to place on balancing the tree, with 0 being none, and 1 being all."), PROPTAB_ADVANCED);

	//lighting settings
	pUI->CreateProperty(CPackerRealProperty("LMSampleExtrude", 0.05f, false, "Specifies the amount to displace samples along the polygon normal"), PROPTAB_ADVANCED);
	pUI->CreateProperty(CPackerBoolProperty("LMOverSample", true, "Determines if oversampling should be used on lightmaps"), PROPTAB_ADVANCED);

	//centering the world around the origin
	pUI->CreateProperty(CPackerBoolProperty("CenterWorld", true, "This will center the world around the origin in order to reduce numerical accuracy errors"), PROPTAB_ADVANCED);


	return true;
}

// Called when a property is changed by the user. This is where
// different options can be validated/invalidated, etc. Note: This function
// should not add any extra options to through the interface, merely enable
// or disable, or change values. It should not change the form of any of the
// properties and this will be undefined. For example, switching a string to a
// filename will have unknown results. It is given the property that was
// changed (note that this can be null) along with the list of properties
// and the UI which the packer can modify the enabled status of properties through.
bool CPreProcPackerImpl::PropertyChanged(CPackerProperty* pProperty, CPackerPropList* pPropList, IPackerUI* pUI)
{
	//if replace textures is checked, nothing else can be enabled

	//------------------------------------------------------------
	//geometry settings


	bool bObjectsOnly = pPropList->GetBool("ObjectsOnly");

	pPropList->EnableProperty("BalanceWeight", !bObjectsOnly);
	pPropList->EnableProperty("MaxDetailLevel", !bObjectsOnly);

	//------------------------------------------------------------
	//lighting settings

	pPropList->EnableProperty("ApplyLighting", !bObjectsOnly);

	bool bLight	= !bObjectsOnly && pPropList->GetBool("ApplyLighting");
	bool bGenLMaps = !pPropList->GetBool("VertexLightingOnly") && bLight;

	pPropList->EnableProperty("VolumetricAmbient", bLight);
	pPropList->EnableProperty("Shadows", bLight);
	pPropList->EnableProperty("VertexLightingOnly", bLight);
	pPropList->EnableProperty("ImportTextureFlags", bLight);
	pPropList->EnableProperty("LMSampleExtrude", bLight);
	pPropList->EnableProperty("LightAnimations", bLight);

	pPropList->EnableProperty("LMOverSample", bGenLMaps);
	pPropList->EnableProperty("SuperSample", bGenLMaps);

	pPropList->EnableProperty("NumSamples", pPropList->GetBool("SuperSample") && bGenLMaps);

	return true;
}

//---------------------
// Execution

// Called when the user has chosen to begin processing the level. The output
// object is passed in so that all messages can be directed to it as they
// arise. In addition, the property list is passed for the preprocessor
// to retrieve its settings from
bool CPreProcPackerImpl::Process(const char* pszFilename, CPackerPropList* pPropList, IPackerOutput* pOutput)
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return false;
	}

	//need to first setup the global settings structure

	//the data structure
	CProcessorGlobs Globs;

	//setup the strucutre
	InitProcessOptions(&Globs, pszFilename);

	// These should be set by the caller.
	LTStrCpy(Globs.m_InputFile, pszFilename, MAX_PATH);

	if(strlen(pPropList->GetString("OutFile")) > 0)
	{
		strncpy(Globs.m_OutputFile, pPropList->GetString("OutFile"), MAX_PATH);
	}

	LTStrCpy(Globs.m_ProjectDir, pPropList->GetString("ProjectPath"), MAX_PATH);

	//verify that the project path is setup correctly, first make sure slashes face the same way
	uint32 nProjPathLen = strlen(Globs.m_ProjectDir);
	for(uint32 nCurrChar = 0; nCurrChar < nProjPathLen; nCurrChar++)
	{
		if(Globs.m_ProjectDir[nCurrChar] == '/')
			Globs.m_ProjectDir[nCurrChar] = '\\';
	}

	//and make sure it doesn't end with a slash
	if(nProjPathLen && (Globs.m_ProjectDir[nProjPathLen - 1] == '\\'))
		Globs.m_ProjectDir[nProjPathLen - 1] = '\0';

	//determines if the supersampling algorithm should be used
	Globs.m_bLMSuperSample = pPropList->GetBool("SuperSample");

	//determine if the lightmaps should be expanded by a pixel on the right and
	//bottom sides in order to have a little bit higher resolution (this
	//tends to fix continuity issues across corners)
	Globs.m_bLMOverSample = pPropList->GetBool("LMOverSample");

	//if the super sampling algorithm is used, this specifies the number
	//of samples to be taken along a side
	Globs.m_nLMNumSamplesOnSide = (uint32)pPropList->GetReal("NumSamples");

	Globs.m_bObjectsOnly = pPropList->GetBool("ObjectsOnly");
	Globs.m_bIgnoreHidden = pPropList->GetBool("IgnoreHidden");
	Globs.m_bIgnoreFrozen = pPropList->GetBool("IgnoreFrozen");
	Globs.m_BalanceWeight = (PReal)pPropList->GetReal("BalanceWeight");

	Globs.m_bCenterWorldAroundOrigin = pPropList->GetBool("CenterWorld");
	Globs.m_bVolumetricAmbient = pPropList->GetBool("VolumetricAmbient");

	//make sure that balance weight is in range
	Globs.m_BalanceWeight = LTMAX((PReal)0.0, LTMIN((PReal)1.0, Globs.m_BalanceWeight));

	Globs.m_bLight = pPropList->GetBool("ApplyLighting");
	Globs.m_bShadows = pPropList->GetBool("Shadows");
	Globs.m_bLightAnimations = pPropList->GetBool("LightAnimations");

	//only light vertices (no lightmaps)
	Globs.m_bVerticesOnly = pPropList->GetBool("VertexLightingOnly");

	//when taking lighting samples, this determines how much to extrude the points
	//along the polygon normal to prevent self intersection with its base polygon
	Globs.m_fLMSampleExtrude = pPropList->GetReal("LMSampleExtrude");

	//see if we are logging to a file
	Globs.m_bLogFile = pPropList->GetBool("LogToFile");
	Globs.m_bErrorLog = pPropList->GetBool("ErrorLog");

	//number of threads to use
	Globs.m_nThreads = (uint32)pPropList->GetReal("NumThreads");

	//make sure to save the back end for the rest of the packer to access
	Globs.m_pIOutput = pOutput;

	//now go ahead and do the processing
	DoProcessing(&Globs);

	return true;
}
