#ifndef __PROCESSING_H__
#define __PROCESSING_H__


	class CPreWorld;
	class CEditBrush;
	class CBspHullMaker;
	class CWorldNode;
	class IPackerOutput;


	// ------------------------------------------------------ //
	// Here's all the functions and variables the 
	// system-dependent stuff gets to handle.
	// ------------------------------------------------------ //

	#define MAX_THREADS					40

	class CProcessorGlobs
	{
	public:

		// Maximum lightmap size.  Comes from the world info string or DEFAULT_LIGHTMAP_PIXELS.
		PReal	m_MaxLMSize; 

		// Set to TRUE if it's making a world model (so the debug output can
		// be toned down a little).
		bool	m_bMakingWorldModel;

		// These should be set by the caller.
		char	**m_Argv;
		uint32	m_Argc;
		char	m_InputFile[MAX_PATH]; // Input filename (like worlds\test1.lta).
		char	m_OutputFile[MAX_PATH]; // Full output filename (like worlds\test1.dat).

		char	m_ProjectDir[MAX_PATH];

		//determines if the supersampling algorithm should be used
		bool	m_bLMSuperSample;

		//determine if the lightmaps should be expanded by a pixel on the right and 
		//bottom sides in order to have a little bit higher resolution (this
		//tends to fix continuity issues across corners)
		bool	m_bLMOverSample;

		//if the super sampling algorithm is used, this specifies the number
		//of samples to be taken along a side
		uint32	m_nLMNumSamplesOnSide;

		//center world around origin?
		bool	m_bCenterWorldAroundOrigin;

		// Use portals?
		bool	m_bPortals; 
		bool	m_bObjectsOnly;
		
		bool	m_bIgnoreHidden;
		bool	m_bIgnoreFrozen;

		PReal	m_SplitWeight, m_BalanceWeight;
		
		bool	m_bLight, m_bShadows, m_bLightAnimations;

		//only light vertices (no lightmaps)
		bool	m_bVerticesOnly;

		// Enable shadow meshing
		bool	m_bShadowMesh;

		// Volumetric ambient grid calculation
		bool	m_bVolumetricAmbient;

		//when taking lighting samples, this determines how much to extrude the points
		//along the polygon normal to prevent self intersection with its base polygon
		PReal	m_fLMSampleExtrude;

		//see if we are logging to a file
		bool	m_bLogFile;

		//see if we are logging to an error file
		bool	m_bErrorLog;

		//number of threads to use
		uint32	m_nThreads;

		//see if we want to skip the starting dialog
		bool	m_bSkipDialog;

		//where the output will be directed
		IPackerOutput*	m_pIOutput;

		// The processor will fill these in.
		uint32	m_nInputPolies, m_nInputVertices, m_nInputObjects;
		uint32	m_nOutputPolies, m_nOutputVertices, m_TreeDepth;
		uint32	m_LMDataSize;
	};

	extern CProcessorGlobs *g_pGlobs;


	// ------------------------------------------------------ //
	// Functions the Processing module implements.
	// ------------------------------------------------------ //

	//different types of messages that can be displayed
	enum EStatusTextType
	{
		eST_Error,		//used to signal errors that would indicate bugs in the level
		eST_Warning,	//used to indicate items that should be brought to the users attention but does not invalidate level
		eST_Normal		//used for displaying what the processor is doing
	};

	// Initializes the global structure
	void InitProcessOptions(CProcessorGlobs *pGlobs, const char* pszInputFile);

	// Do all the processing.
	void DoProcessing(CProcessorGlobs *pGlobs);

	void DrawStatusText(EStatusTextType eType, const char *pMsg, ...);
	// Draws text if not working on a world model.
	void DrawStatusTextIfMainWorld(EStatusTextType eType, const char *pMsg, ...); 

	// Makes a surface in pWorld for each brush poly.
	void GenerateSurfaces(GenList<CEditBrush*> &brushes, CPreWorld *pWorld);

	//given a node, it will look at the processor settings and determine if it should be ignored
	bool ShouldIgnoreNode(const CWorldNode* pNode);


	// ------------------------------------------------------ //
	// Functions the system-dependent stuff implements.
	// ------------------------------------------------------ //

	// Functions for the back end user interface see IPackerOutput for more details
	void	ActivateTask(const char* pszTaskName);
	void	ActivateSubTask(const char* pszSubTaskName);
	void	SetProgressBar(float fPercent);

	// Question types and return values.
	#define QUES_YES		1
	#define QUES_NO			2
	#define QUES_CANCEL		4

	// Type is a combination of the above flags.
	// Returns one of the above flags.
	uint32 AskQuestion(const char *pQuestion, uint32 type);

	// Does a union on all brush geometry and converts to world geometry.
	bool MergeBrushes(
		GenList<CEditBrush*> &brushes,
		CPreWorld *pWorld);

	// Make a BSP.
	bool MakeBsp(CPreWorld *pWorld);

#endif  // __PROCESSING_H__
