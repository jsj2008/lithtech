// ModelEditDlg.cpp : implementation file
//
// Changes:
//
// {BP 1/4/98} Switched to using DStream instead of MoFileIO

#include "precompile.h"
#include "modeledit.h"
#include "modeleditdlg.h"
#include "renamedlg.h"
#include "continuousdlg.h"
#include "renderwnd.h"
#include "keyframewnd.h"
#include "modelinfodlg.h"
#include "commandstringdlg.h"
#include "model_ops.h"
#include "model_cleanup.h"
#include "mmsystem.h"
#include "streamsim.h"
#include "dimensionsdlg.h"
#include "importanimation.h"
#include "animframeratedlg.h"
#include "keyframetimedlg.h"
#include "translationdlg.h"
#include "uvimportdlg.h"
#include "animnumberdlg.h"
#include "renamenodedlg.h"
#include "rotationdlg.h"
#include "aboutdlg.h"
#include "stringdlg.h"
#include "ltbasedefs.h"
#include "addsocketdlg.h"
#include "geomroutines.h"
#include "transformmaker.h"
#include "animmergedlg.h"
#include "newgenlod.h"
#include "addchildmodeldlg.h"
#include "regmgr.h"
#include "lteulerangles.h"
#include "socketedit.h"
#include "weightsetselectdlg.h"
#include "ExportD3D_Dlg.h"
#include "loadltadlg.h"
#include <process.h>
#include "importstringkeysdlg.h"
#include "piecematerialdlg.h"
#include "piecelodgendlg.h"
#include "piecelodgen.h"
#include "piecemerge.h"
#include "importloddlg.h"
#include "editobbdlg.h"
#include "DirDialog.h"
#include "ltwintreeitem.h"
#include "ltwintreeitemiter.h"
#include <set>
#include "invalidanimdimsdlg.h"


extern char g_szStartPath[_MAX_PATH];	// working directory at launch
extern char g_szExePath[_MAX_PATH];		// path to executable


// ------------------------------------------------------------------------
/// std c++
#include <fstream>

using namespace std;
   
/// lta parse...
#include "ltaModel.h"

// ------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Externs
extern TCHAR szRegKeyCompany[];
extern TCHAR szRegKeyApp[];
extern TCHAR szRegKeyVer[];

// ------------------------------------------------------------------------
// TempVarSetter
// change value of variable for the life time of the class instance.
//
// TempVarSetter<int> tmpVal( m_IntVal, 100 );
// m_IntVal is equal to 100 until tmpVal gets deconstructed. 
// ------------------------------------------------------------------------
template<class T>
class TempVarSetter
{
public:
			
			TempVarSetter(T *pVar, T newVal)
			{
				m_PrevVal = *pVar;
				m_pValue = pVar;
				*pVar = newVal;
			}

			~TempVarSetter()
			{
				*m_pValue = m_PrevVal;
			}
			 
private:
	T		*m_pValue;
	T		m_PrevVal;
};


// Model transition time (this is a console variable in the engine)..
int g_CV_ModelTransitionMS = 200;


#define MENUCHECK(test) ((test) ? MF_CHECKED : MF_UNCHECKED)

bool g_bUpdateNodeFlags = false;

// ------------------------------------------------------------------------
// IsLtaFile
// does this filename have an lta extention, (case insensitive)
// ------------------------------------------------------------------------
static inline 
int IsFilenameLTAExt( const char *filename )
{
	CString pathName ;

	// not enough chars in the name  
	if( strlen( filename ) < 4 )
		return 0;

	pathName = filename ; 
	CString szFileExt = pathName.Right(4);
	szFileExt.MakeLower();

	
	//check for either normal or compressed extensions
	if(( szFileExt == ".lta" ) || ( szFileExt == ".ltc" ))
	{
		return 1;
	}

	else return 0;

}
 
// Used for socket rotation and positioning.
float GetRotAngle(int delta)
{
	return ((delta * MATH_CIRCLE) / 360.0f) * 0.25f;
}

float GetPosAmount(int delta)
{
	return (float)delta * (1.0f / 64.0f);
}

float GetScaleAmount(int delta)
{
	return 1.0f + ((float)delta * (1.0f / 64.0f));
}

// ------------------------------------------------------------------------
// GetModelMaxLODDist
// get the largest largest dist specified in any piece of a model.
// ------------------------------------------------------------------------
float GetModelMaxLODDist( Model *pModel )
{
	float max_dist = 0.0f ;

	if( !pModel )
		return max_dist ;

	for( int i = 0 ;i < pModel->NumPieces() ; i++ )
	{
		ModelPiece *pPiece = pModel->GetPiece(i);
		float max = pPiece->GetMaxDist();
		if( max > max_dist )
			max_dist = max ;
	}
	return max_dist ;
}
// ------------------------------------------------------------------------
// AddTracker
// ui thing.
// ------------------------------------------------------------------------
void AddTracker(CModelEditDlg *pDlg, COLORREF color, UINT ctlID, RTType type, DWORD iAxis)
{
	RectTracker *pTracker;
	CWnd *pWnd;

	if(pTracker = new RectTracker)
	{
		pTracker->m_Color = color;
		pTracker->m_CtlID = ctlID;

		if(pWnd = pDlg->GetDlgItem(ctlID))
		{
			pWnd->GetClientRect(&pTracker->m_Rect);
			pWnd->ClientToScreen(&pTracker->m_Rect);
			pDlg->ScreenToClient(&pTracker->m_Rect);
		}

		if(pTracker->m_Wnd.Create(
			NULL, 
			"RectTracker", 
			WS_CHILD | WS_VISIBLE, 
			pTracker->m_Rect, 
			pDlg, 
			-1, 
			NULL))
		{
			pTracker->m_Wnd.SetColor(color);
			pTracker->m_Wnd.SetForward(TRUE);
		}

		pTracker->m_Type = type;
		pTracker->m_Axis = iAxis;
		pDlg->m_Trackers.Append(pTracker);
	}
}

///////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------
// MEAnimInfo 
// ------------------------------------------------------------------------

MEAnimInfo::MEAnimInfo()
{
	m_bAnimPlayingForward = FALSE;
	m_dwNumTagged = 0;
}


void MEAnimInfo::Reset(Model *pModel)
{
	m_Wnd.SetActive(FALSE);
	m_Wnd.SetAnim(NULL);
	m_Tracker.m_TimeRef.Init(pModel);
}


void MEAnimInfo::StartPlayback()
{
	StopPlayback();
	m_bAnimPlayingForward = TRUE;
}


void MEAnimInfo::StopPlayback()
{
	m_bAnimPlayingForward = FALSE;
	m_Wnd.ForceNearestKeyframe();
}


BOOL MEAnimInfo::IsValid()
{
	return m_Tracker.m_TimeRef.IsValid();	
}


void MEAnimInfo::SetKeyframeWindowTime()
{
	m_Wnd.SetTime(m_Tracker.m_TimeRef.m_Cur.m_Time);
}


ModelAnim* MEAnimInfo::PrevAnim()
{
	if(IsValid())
		return m_Tracker.m_TimeRef.m_pModel->GetAnim(PrevAnimIndex());
	else
		return NULL;
}


ModelAnim* MEAnimInfo::CurAnim()
{
	if(IsValid())
		return m_Tracker.m_TimeRef.m_pModel->GetAnim(CurAnimIndex());
	else
		return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CModelEditDlg dialog

CModelEditDlg::CModelEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModelEditDlg::IDD, pParent),
	m_AllocCount(&g_DefAlloc),
	m_pDimensionDlg(NULL)
{
	DWORD i;
	MEAnimInfo *pInfo;
	LTAnimTracker *animTrackerPointers[NUM_ANIM_INFOS];
	UINT animButtonIDs[NUM_ANIM_INFOS] =
	{
		IDC_ANIMATION1,
		IDC_ANIMATION2,
		IDC_ANIMATION3,
		IDC_ANIMATION4
	};

	//{{AFX_DATA_INIT(CModelEditDlg)
	m_bDrawSkeleton = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pModel = NULL;
	m_bEndDialog = FALSE;
	m_bTracking = FALSE;
	m_bInStringChange = FALSE;

	m_LastIdleTime = timeGetTime();

	m_nCurrentLOD = PIECELOD_BASE;
	m_CurrentLODDist = 0.0f ;

	m_bChangesMade = FALSE;
	m_hAccel = NULL;

	m_RenderWnd.m_pModelEditDlg = this;
	m_bFillingNodeList = FALSE;

	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = GetAnimInfo(i);

		m_LastAnimActiveFlags[i] = 0;
		animTrackerPointers[i] = &pInfo->m_Tracker;
		pInfo->m_Tracker.m_Flags = AT_PLAYING | AT_LOOPING;
		pInfo->m_AnimButtonID = animButtonIDs[i];
		pInfo->m_Wnd.m_iWnd = i;
	}

	m_RenderWnd.InitAnims(animTrackerPointers);

	m_RenderWnd.ResetLocator();	
	
	m_pImportStringKeyDlg = NULL;	

	// get the project dir
		// get the dep proj dir stuff it into the model db.
	CRegMgr regMgr;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		CString csOpenDir ;
		UINT32 dwSize = 256;
		regMgr.Get("dep_file_location", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		m_szCurProjectPath =  csOpenDir ;
	}

	// should we require user confirmation of the load dialog
	if( regMgr.Init( szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Options", HKEY_CURRENT_USER ) )
	{
		m_bConfirmLoadDlg = regMgr.Get( "confirm_load", 1 ) > 0;
	}
}


CModelEditDlg::~CModelEditDlg()
{
	if (m_pImportStringKeyDlg)
	{
		m_pImportStringKeyDlg->DestroyWindow();
		delete m_pImportStringKeyDlg;
		m_pImportStringKeyDlg = NULL;
	}
	
	for( int i = 0; i < m_Trackers; i++ )
	{
		delete m_Trackers[i];
	}

	DeleteModel();
}

void CModelEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModelEditDlg)
	DDX_Control(pDX, IDC_ANIMLIST, m_AnimList);
//	DDX_Control(pDX, IDC_PIECES, m_PieceList);
	DDX_Control(pDX, IDC_NODELIST, m_NodeList);
	DDX_Control(pDX, IDC_GLOBAL_SPACE, m_GlobalSpace);
	DDX_Control(pDX, IDC_SOCKETLIST, m_SocketList);
	DDX_Control(pDX, IDC_FRAMETIME, m_EditFrameTime);
	DDX_Control(pDX, IDC_FRAMESTRING, m_EditFrameString);
	DDX_Control(pDX, IDC_CHILDMODELS, m_ChildModelList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CModelEditDlg, CDialog)
	//{{AFX_MSG_MAP(CModelEditDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOADMODEL, OnLoadModel)
	ON_MESSAGE(WM_STARTIDLE, OnStartIdle)
	ON_BN_CLICKED(ID_HELP_ABOUT, OnHelpAbout)
	ON_BN_CLICKED(IDC_FPLAY, OnPlayForward)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_MOVEDOWN, OnAnimMoveDown)
	ON_BN_CLICKED(IDC_MOVEUP, OnAnimMoveUp)
	ON_BN_CLICKED(IDC_DELETE, OnAnimDelete)
	ON_BN_CLICKED(IDC_NEXTKEY, OnNextKeyframe)
	ON_BN_CLICKED(IDC_PREVKEY, OnPrevKeyframe)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_IMPORT_LODS, OnImportLODs)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_SAVEAS, OnSaveAs)
	ON_BN_CLICKED(IDC_CREATE_SINGLE_FRAME, OnCreateSingleFrame)
	ON_BN_CLICKED(IDC_RENAME, OnRenameAnim)
	ON_BN_CLICKED(IDC_WIREFRAME, OnWireframe)
	ON_BN_CLICKED(IDC_CONTINUOUS, OnContinuous)
	ON_BN_CLICKED(IDC_CAMERA_FOLLOW, OnCameraFollow)
	ON_BN_CLICKED(IDC_DIMS, OnDims)
	ON_BN_CLICKED(IDC_ANIM_BOX, OnAnimBox)
	ON_WM_HSCROLL()
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_MODELINFO, OnModelInfo)
	ON_BN_CLICKED(IDC_COMMANDSTRING_BUTTON, OnCommandString)
	ON_BN_CLICKED(IDC_DIMENSIONS_BUTTON, OnDimensions)
	ON_BN_CLICKED(IDC_ANIMATION_FRAMERATE, OnAnimationFramerate)
	ON_BN_CLICKED(IDC_ANIMATIONLENGTH, OnAnimationLength)
	ON_BN_CLICKED(IDC_ANIMATIONINTERPOLAION, OnAnimationInterpolation)
	ON_BN_CLICKED(IDC_TRANSLATION_BUTTON, OnTranslationButton)
	ON_BN_CLICKED(IDC_ROTATION_BUTTON, OnRotationButton)
	ON_BN_CLICKED(IDC_SET_PROJ_DIR, OnSetProjectDir)
	ON_BN_CLICKED(IDC_XSUB, OnXsub)
	ON_BN_CLICKED(IDC_XADD, OnXadd)
	ON_BN_CLICKED(IDC_YSUB, OnYsub)
	ON_BN_CLICKED(IDC_YADD, OnYadd)
	ON_BN_CLICKED(IDC_ZSUB, OnZsub)
	ON_BN_CLICKED(IDC_ZADD, OnZadd)
	ON_BN_CLICKED(IDC_NUMBER, OnNumberAnim)
	ON_BN_CLICKED(IDC_DUPLICATEANIM, OnDuplicateAnim)
	ON_BN_CLICKED(ID_ANIMATION_CREATEANIMFROMBINDPOSE, OnCreateAnimFromBindPose)
	ON_BN_CLICKED(IDC_RENAMENODE, OnRenameNode)
	ON_BN_CLICKED(IDC_UNROTATE_TOP_NODE, OnUnrotateTopNode)
	ON_BN_CLICKED(ID_GENERATE_VERTEX_NORMALS, OnGenerateVertexNormals)
	ON_BN_CLICKED(ID_REVERSE_ANIMATION, OnReverseAnimation)
	ON_BN_CLICKED(ID_SELECT_NULL_NODES, OnSelectNullNodes)
	ON_BN_CLICKED(ID_MODEL_REMOVENODE, OnRemoveNode)
	ON_BN_CLICKED(ID_PIECE_SET_TEXTURE, OnSetTexture)
	ON_BN_CLICKED(IDC_DRAWSKELETON, OnDrawSkeleton)
	ON_BN_CLICKED(ID_SHOW_SOCKETS, OnShowSockets)
	ON_COMMAND(ID_SHOW_TEXTURES, OnShowTextures)
	ON_BN_CLICKED(ID_SHOW_ATTACHMENTS, OnShowAttachments)
	ON_BN_CLICKED(ID_SOLID_ATTACHMENTS, OnSolidAttachments)
	ON_BN_CLICKED(IDC_MOVEMENT_ENCODING, OnMovementEncoding)
	ON_BN_CLICKED(ID_ADDCHILDMODEL, OnAddChildModel)
	ON_BN_CLICKED(ID_REMOVECHILDMODEL, OnRemoveChildModel)
	ON_BN_CLICKED(ID_ADD_SOCKET, OnAddSocket)
	ON_BN_CLICKED(ID_REMOVE_SOCKET, OnRemoveSocket)
	ON_BN_CLICKED(ID_RENAME_SOCKET, OnRenameSocket)
	ON_BN_CLICKED(ID_REBUILDCHILDMODELTREE, OnRebuildChildModelTree)
	ON_BN_CLICKED(ID_SHOW_NORMALS, OnShowNormals)
	ON_BN_CLICKED(ID_SHOW_NORMALREF, OnShowNormalRef)
	ON_BN_CLICKED(ID_OPTIONS_PROFILE, OnProfile)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(ID_MODEL_MOOD, OnMood)
	ON_BN_CLICKED(ID_INTERNAL_RADIUS, OnInternalRadius)
	ON_BN_CLICKED(ID_CALC_INTERNAL_RADIUS, OnCalcInternalRadius)
	ON_BN_CLICKED(ID_EDIT_WEIGHTS, OnEditWeights)
	ON_BN_CLICKED(IDC_CALC_OBB, OnReCalcOBBExtent)
	ON_BN_CLICKED(ID_PIECE_INFO, OnPieceInfo)
	ON_BN_CLICKED(ID_CREATE_NULL_LOD, OnCreateNullLOD)
	ON_BN_CLICKED(ID_PIECE_SELECT_NONE, OnPieceSelectNone)
	ON_BN_CLICKED(ID_PIECE_REMOVE_LODS, OnPieceRemoveLODs)
	ON_BN_CLICKED(ID_PIECE_BUILD_LODS, OnPieceBuildLODs)
	ON_BN_CLICKED(ID_PIECE_MERGE, OnPieceMerge)
	ON_COMMAND(ID_PIECEEDIT_EXPANDALL, OnPieceExpandAll)
	ON_COMMAND(ID_PIECEEDIT_COLLAPSEALL, OnPieceCollapseAll)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_NODELIST, OnEndlabeleditNodeList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_NODELIST, OnItemChangedNodeList)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_LTTREE_SELCHANGED, OnTreeSelChanged)
	ON_MESSAGE(WM_LTTREE_EDITTEXT, OnTreeEditText)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_ANIMLIST, OnBeginLabelEditAnimList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_ANIMLIST, OnEndLabelEditAnimList)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_ANIMLIST, OnItemChangingAnimList)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_ANIMLIST, OnBeginDragAnimList)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDC_SORTANIM_NAME, OnSortAnimName)
	ON_COMMAND(IDC_SORTANIM_RELATION, OnSortAnimRelation)
	ON_LBN_DBLCLK(IDC_SOCKETLIST, OnDblClkSocketList)
	ON_EN_CHANGE(IDC_FRAMESTRING, OnFrameStringChange)
	ON_COMMAND(ID_OPTIONS_NEWBACKGROUND, OnNewBgColor)
	ON_COMMAND(ID_OPTIONS_LOADCONFIRM, OnLoadConfirm)
	ON_COMMAND(ID_OPTIONS_SHOWORIGINALMODEL, OnOptionsShoworiginalmodel)
	ON_COMMAND(IDC_CHANGE_FOV, OnChangeFov)
	ON_COMMAND(IDC_COMPILE, OnCompile)
	ON_COMMAND(IDC_SAVE_AND_COMPILE, OnSaveAndCompile)
	ON_COMMAND(ID_EXPORTMODELSTRINGKEYS, OnExportModelStringKeys)
	ON_COMMAND(ID_IMPORTMODELSTRINGKEYS, OnImportModelStringKeys)
	ON_NOTIFY(HDN_ITEMCLICK, IDC_NODELIST, OnItemclickNodelist)
	ON_COMMAND(ID_OPTIONS_SHOWVERTEXWEIGHTING, OnOptionsShowVertexWeighting)
	ON_COMMAND(ID_NODEEDIT_EDITOBB, OnNodeeditEditobb)
	ON_COMMAND(ID_NODEEDIT_RENAME, OnNodeEditRename)
	ON_COMMAND(ID_NODEEDIT_LOOKAT, OnNodeEditLookAt)
	ON_COMMAND(ID_SOCKETEDIT_LOOKAT, OnSocketEditLookAt)
	ON_COMMAND(IDC_DELETEANIM, OnAnimDelete)
	ON_COMMAND(IDC_DUPLICATEANIM, OnDuplicateAnim)
	ON_COMMAND(IDC_CHANGE_FOV, OnChangeFov)
	ON_COMMAND(IDC_CHANGE_FOV, OnChangeFov)
	ON_COMMAND(ID_MODEL_EXPORTWEIGHTSETS, OnExportWeightSets)
	ON_COMMAND(ID_MODEL_IMPORTWEIGHTSETS, OnImportWeightSets)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CModelEditDlg public member functions


ModelSocket* CModelEditDlg::GetSelectedSocket()
{
	int curSel;

	if (!m_pModel)
		return NULL;

	curSel = m_SocketList.GetCurSel();
	if (curSel >= 0 && curSel < (int)m_pModel->NumSockets())
	{
		return m_pModel->GetSocket((DWORD)curSel);
	}
	else
	{
		return NULL;
	}
}


CButton* CModelEditDlg::GetTransformEdit(UINT id)
{
	CWnd *pWnd;

	pWnd = (CWnd*)GetDlgItem(id);
	if (pWnd /*&& pWnd->IsKindOf(RUNTIME_CLASS(CButton))*/)
		return (CButton*)pWnd;
	
	return NULL;
}


BOOL CModelEditDlg::IsButtonSelected(UINT id)
{
	CButton *pButton;

	if (pButton = GetTransformEdit(id))
	{
		return !!(pButton->GetState() & 0x3);
	}
	else
	{
		return FALSE;
	}
}


// The about dialog
void CModelEditDlg::OnHelpAbout()
{
	CAboutDlg dlg;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	dlg.DoModal();
}

void CModelEditDlg::InitTaggedArrays()
{
	uint32 i;

	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		InitTaggedArray(i);
	}
}

void CModelEditDlg::InitTaggedArray(uint32 iAnimInfo)
{
	GetAnimInfo(iAnimInfo)->m_Wnd.InitTaggedArray();
}

void CModelEditDlg::DeleteModel()
{
	if (m_pModel)
	{
		DeleteSocketAttachments();
		
		m_pModel->Term(TRUE); // We want it to delete the child models.
		delete m_pModel;
		m_pModel = NULL;
	
		ResetAnimInfos();
	}
}


void CModelEditDlg::DeleteSocketAttachments()
{
	DWORD i;
	ModelSocket *pSocket;

	
	if (m_pModel)
	{
		// Delete models attached to sockets.
		for (i=0; i < m_pModel->NumSockets(); i++)
		{
			pSocket = m_pModel->GetSocket(i);

			if (pSocket->m_pAttachment)
			{
				delete pSocket->m_pAttachment;
				pSocket->m_pAttachment = NULL;
			}
		}
	}
}


void CModelEditDlg::GetPlaybackActiveFlags(
	DWORD dwActive[NUM_ANIM_INFOS], 
	BOOL bAllIfNoKeysDown)
{
	DWORD i, nActive;

	// First clear the flags.
	for(i=0; i < NUM_ANIM_INFOS; i++)
		dwActive[i] = 0;

	// See what CTRL+1.. keys they have down.
	nActive = 0;
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		if((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
			(GetAsyncKeyState('1' + i) & 0x8000))
		{
			dwActive[i] |= PA_ACTIVE;
			++nActive;
		}
	}

	// If no special keys are down, apply the command to all animations.
	if(nActive == 0 && bAllIfNoKeysDown)
	{
		for(i=0; i < NUM_ANIM_INFOS; i++)
			dwActive[i] |= PA_ACTIVE;
	}

	// Set the valid flag.
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		if(GetAnimInfo(i)->IsValid())
			dwActive[i] |= PA_VALID;
	}
}


void CModelEditDlg::SetCurrentPosition(
	DWORD iAnimInfo,
	DWORD nKeyframe1, 
	DWORD nKeyframe2, 
	float nPercentBetween)
{
	AnimTimeRef *pRef;
	MEAnimInfo *pInfo;

	
	ASSERT(iAnimInfo < NUM_ANIM_INFOS);
	pInfo = &m_AnimInfos[iAnimInfo];
	
	pRef = &pInfo->m_Tracker.m_TimeRef;
	pRef->SetKeyframePosition(
		pRef->m_Prev.m_iAnim,
		nKeyframe1, 
		nKeyframe2, 
		nPercentBetween);

	if(iAnimInfo == ANIMINFO_MAIN)
	{
		UpdateEditFrameString( );
		UpdateEditFrameTime( );
	}
}
					

void CModelEditDlg::SetCurrentPosition(
	CKeyframeWnd *pWnd,
	DWORD nKeyframe1, 
	DWORD nKeyframe2, 
	float nPercentBetween)
{
	DWORD i;

	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		if(&m_AnimInfos[i].m_Wnd == pWnd)
		{
			SetCurrentPosition(i, nKeyframe1, nKeyframe2, nPercentBetween);
			break;
		}
	}
}
					

/////////////////////////////////////////////////////////////////////////////
// CModelEditDlg protected member functions

void CModelEditDlg::OnIdle()
{
	uint32 i;
	DWORD msDelta, dwTime;
	MEAnimInfo *pInfo;


	// Update the animation playback stuff.
	OnSelChangeAnimList();

	dwTime = timeGetTime();

	msDelta = dwTime - m_LastIdleTime;
	//msDelta = (DWORD)( (float)msDelta * .5f ) ;
	msDelta = (DWORD)( (float)msDelta ) ;
	msDelta = DCLAMP(msDelta, 1, 1000);

	m_LastIdleTime = dwTime;


	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = GetAnimInfo(i);

		if (pInfo->m_bAnimPlayingForward)
		{
			InterpForward(i, msDelta);
		}
	}

	DrawActiveAnimRects(FALSE);

	m_RenderWnd.Draw();
}

void CModelEditDlg::InterpForward(uint32 iAnimInfo, DWORD msDelta)
{
	AnimTimeRef *pTimeRef;
	MEAnimInfo *pInfo;

	
	pInfo = GetAnimInfo(iAnimInfo);
	pTimeRef = &pInfo->m_Tracker.m_TimeRef;

	// get the current time in the animation
	trk_Update(&pInfo->m_Tracker, msDelta);

	// tell the keyframe window where are are
	pInfo->SetKeyframeWindowTime();
	
	UpdateEditFrameString();
	UpdateEditFrameTime();
}

// {BP 1/20/97}
// Verifies that two model files are similar...
BOOL CModelEditDlg::VerifyModels( Model *pModel1, Model *pModel2 )
{
	if ( !RecurseAndVerifyModels( pModel1->GetRootNode(), pModel2->GetRootNode() ))
		return FALSE;

	return TRUE;
}

BOOL CModelEditDlg::RecurseAndVerifyModels( ModelNode* pNode1, ModelNode* pNode2 )
{
	if ( pNode1->m_Children.GetSize() != pNode2->m_Children.GetSize())
	{
		// models don't match
		return FALSE;
	}

	for (DWORD i = 0; i < pNode1->m_Children; i++)
	{
		if (!RecurseAndVerifyModels(pNode1->m_Children[i], pNode2->m_Children[i]))
		{
			return FALSE;
		}
	}

	return TRUE;
}


// ------------------------------------------------------------------------
// load_LTA_Using_FeedBackDialog( filename, memory-allocation-counter )
// helper function to load lta files and showing the load feed-back dialog.
// returns NULL if failed, the alloc'd model file if successful.
// ------------------------------------------------------------------------
Model* CModelEditDlg::load_LTA_Using_FeedBackDialog( const char *pFilename, bool isChildModel  )
{
	void LoadLTAThreadMain( void* pUser );

	m_AllocCount.ClearCounts();

	//creat the dialog that will exist on this thread to display
	//the status
	CLoadLTADlg LoadDlg( !m_bConfirmLoadDlg );

	//tell it that the thread will now be running
	CLoadLTADlg::SetLoadThreadDone(FALSE);
	CLoadLTADlg::ClearLoadLog();


	//create the parameters for the thread
	CLoadLTAThreadParams Params;
	Params.m_pAllocCount	= &m_AllocCount ;
	Params.m_pModel			= NULL;
	Params.m_bChildLoad     = isChildModel;
	Params.m_Filename       = pFilename ;

	// tell the lta translation status thingy what to call when updateing 
	CLTATranslStatus::SetLoadLogCB( CLoadLTADlg::AppendLoadLog );

	//spawn the new thread to handle the loading of the LTA
	unsigned long hThread = _beginthread( LoadLTAThreadMain, 0, &Params );

	if( hThread == -1 )
	{
		MessageBox("Failed to create thread to load LTA", "Error!");
		return FALSE;
	}

	//set the filename
	//LoadDlg.m_sFilename = pathName;
	LoadDlg.m_sFilename = pFilename ;

	//tell the dialog to take control. It will not return until
	//the LTA loading thread has finished and the user has closed
	//it
	LoadDlg.DoModal();

	//unload the parameters that have been modified by the thread
	return  Params.m_pModel;	
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

	CString puff;
	puff= pBaseName ;
	puff+=pChildFilename ;

	int base_is_lta = IsFilenameLTAExt( pBaseName ) ;
	int child_is_lta= IsFilenameLTAExt( pChildFilename );

	if( base_is_lta && child_is_lta )
	{
		MessageBox(NULL, "Programer Error Report this as a bug.\r\nCannot Load LTA Child Models From ME",
					"Error in ChildModel Load", MB_OK);
		pModel = NULL ;
		return pModel;
	}

	return NULL;
}


DRESULT ME_LoadChildFn(ModelLoadRequest *pRequest, Model **ppModel)
{
	*ppModel = LoadChildModel(pRequest, (char*)pRequest->m_pLoadFnUserData, pRequest->m_pFilename);
	if (*ppModel)
		return LT_OK;
	else
		return LT_NOCHANGE;
}


//this is the thread main function that handles the loading of an LTA file. The parameter
//is a stucture containing a handle to an ifstream, and a pointer to where the model
//will be stored.
void LoadLTAThreadMain( void* pUser )
{
	//get the parameters
	CLoadLTAThreadParams* pParams = (CLoadLTAThreadParams*)pUser;

	//verify the parameters are valid
	ASSERT(pParams);
	ASSERT(pParams->m_pAllocCount);

	//load up the model
	if( pParams->m_bChildLoad)
	{
		pParams->m_pModel = ltaLoadChildModel(pParams->m_Filename.c_str(), pParams->m_pAllocCount);
	}
	else
	{
		pParams->m_pModel = ltaModelLoad(pParams->m_Filename.c_str(), *(pParams->m_pAllocCount));
	}

	//tell the dialog we are done, and that it can exit
	CLoadLTADlg::SetLoadThreadDone(TRUE);

	return;
}

// ------------------------------------------------------------------------
// DoLoadModel( filenmae , bMessage )
// 
// ------------------------------------------------------------------------
BOOL CModelEditDlg::DoLoadModel(char *pFilename, BOOL bMessage)
{
	CString titleText;
	Model *pModel;
	ModelLoadRequest request;
	DWORD i;

	// Stop any animations...
	StopAllPlayback();

	BeginWaitCursor();

	// check for lta file. if so call ltaModelLoad 
	// else bomb out.
	if( IsFilenameLTAExt(pFilename )  )
	{
		// zap all the textures before loading up something new.
		m_RenderWnd.ReleaseAllTextures() ;

		pModel = load_LTA_Using_FeedBackDialog( pFilename  );

		if( pModel == NULL )
		{
			MessageBox("Failed to load lta file","load error");
		}
	}
	else 
	{
		// huh? don't know that extention ...
		MessageBox( "unknown extension... " , "load model error", MB_OK);
		EndWaitCursor();
		return FALSE;
	}
	
	// Get rid of the old one.
	if( pModel != NULL )
	{
		DeleteModel();
		m_pModel = pModel;
		m_strFilename = pFilename;
	}
	else
		return FALSE;

	//check and make sure animation dimensions are valid now
	CheckValidAnimationDims();

	// init dialog controls
	FillAnimList();
	FillNodeList();
	FillSocketList();
	FillChildModelList();
	FillPieceList( &m_PieceList, m_pModel );

	m_AnimList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	for (i = 1; i < (DWORD)m_AnimList.GetItemCount(); i++)
		m_AnimList.SetItemState((int)i, 0, LVIS_SELECTED);

	// Init animations..
	OnSelChangeAnimList();


	m_RenderWnd.AllocSelections( m_pModel->NumPieces(), m_pModel->NumNodes());

	// enable appropriate windows

	m_btnPrevKey.EnableWindow();
	m_btnStop.EnableWindow();
	m_btnFPlay.EnableWindow();
	m_btnNextKey.EnableWindow();
	m_btnMoveUp.EnableWindow();
	m_btnMoveDown.EnableWindow();
	m_btnDelete.EnableWindow();
	m_AnimList.EnableWindow();


	m_bChangesMade = FALSE;


	titleText.FormatMessage(IDS_TITLE_TEXT, pFilename);
	SetWindowText((LPCTSTR)titleText);

	m_pModel->SetFilename( pFilename );

	DVector temp;
	float mag;

	if (m_pModel->m_Anims > 0)
	{
		float fov = MATH_DEGREES_TO_RADIANS(m_RenderWnd.m_DrawStruct.m_FOV);
		
		static const float kfMaxFOV = 89.99f;

		//make sure that we never go beyond a valid FOV
		if( m_RenderWnd.m_DrawStruct.m_FOV > kfMaxFOV )
		{
			m_RenderWnd.m_DrawStruct.m_FOV = kfMaxFOV;
		}
		
		mag = m_pModel->m_GlobalRadius ;

		m_RenderWnd.m_Scale = mag / 200.0f;
		m_RenderWnd.m_Camera.Reset();
		m_RenderWnd.m_Camera.m_Position.z = ((mag) / (float)tan( fov ))  ; 
		m_RenderWnd.m_Camera.m_Position.z = (m_pModel->GetAnimInfo(0)->m_vDims.Mag() * 2);

		m_RenderWnd.m_Camera.Update();
	}
	else
	{
		m_RenderWnd.m_Scale = 1.0f;
	}

	UpdateEditFrameString( );
	UpdateEditFrameTime( );

	ProcessCommandString();

	SetCurrentLOD(0.0f);
	InitTheScrollBar(GetModelMaxLODDist(m_pModel));

	CheckChildModels(&request);

	// Load the sockets that are defined in the registry
	CRegMgr regMgr;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Attachments", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		CString csAttachment;
		for (i = 0; i < m_pModel->NumSockets(); i++)
		{
			ModelSocket *pSocket = m_pModel->GetSocket(i);
			if (!pSocket)
				continue;
			dwSize = 256;
			if (regMgr.Get(pSocket->GetName(), csAttachment.GetBufferSetLength(dwSize), dwSize) != NULL)
			{
				csAttachment.ReleaseBuffer(-1);

				if (pSocket->m_pAttachment)
					delete pSocket->m_pAttachment;
				pSocket->m_pAttachment = NULL;

				if (!csAttachment.IsEmpty())
				{
					const TCHAR* filename = csAttachment ;
					if( IsFilenameLTAExt( filename ) )
					{
						Model *pModel = load_LTA_Using_FeedBackDialog( filename, false  );
						if( pModel != NULL )
						{
							pSocket->m_pAttachment = pModel ;
						}
					}
					else 
					{
					// Load the attachment
					if (request.m_pFile = streamsim_Open(csAttachment, "rb"))
					{
						MessageBox("SOCKET LOAD CALLED OLD STYLE", "DEPRECATED", MB_OK);
						request.m_pFile->Release();
					}
					}
				}
			}
		}
	}
	
	// get the dep proj dir stuff it into the model db.
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		CString csOpenDir ;
		UINT32 dwSize = 256;
		regMgr.Get("dep_file_location", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		m_pModel->m_sProjectDir = csOpenDir ;
	}

	{
	// Ok, now we load up the texture maps, from the tools-info node... 
	Model::CTextureIndexMapIter it = m_pModel->m_TextureIndexMap.begin();
	Model::CTextureIndexMapIter end= m_pModel->m_TextureIndexMap.end();
	// export the index to texture name list
	for(  ; it != end ;	it++ )
	{
		uint32 index = (*it).first;
		string path = m_pModel->m_sProjectDir ;
		path += (*it).second;

		// if we fail to load its ok, 
		SetRenderTexture( path.c_str(), index );
	}

	}

	// [TMG] Update the dimensions dialog	
	if (m_pDimensionDlg)
	{
		m_pDimensionDlg->m_pModel = m_pModel;
	}

	EndWaitCursor();
	
	return TRUE;// Success !!
}


void CModelEditDlg::UpdateMenuChecks()
{
	CMenu *pMenu;

	pMenu = GetMenu();
	if (!pMenu)
		return;

	// Since the submenus don't have IDs....
	pMenu = pMenu->GetSubMenu(6);
	if (!pMenu)
		return;
 //t.f important
	pMenu->CheckMenuItem(IDC_CAMERA_FOLLOW, MENUCHECK(m_RenderWnd.m_bCameraFollow));
	pMenu->CheckMenuItem(IDC_WIREFRAME, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bWireframe));
	pMenu->CheckMenuItem(IDC_DIMS, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bDims));
	pMenu->CheckMenuItem(IDC_ANIM_BOX, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bAnimBox));
	pMenu->CheckMenuItem(IDC_DRAWSKELETON, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bDrawSkeleton));
	pMenu->CheckMenuItem(ID_OPTIONS_SHOWORIGINALMODEL, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bDrawOriginalModel));

	pMenu->CheckMenuItem(ID_SHOW_TEXTURES, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bDrawTextures));
	pMenu->CheckMenuItem(ID_SHOW_SOCKETS, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bDrawSockets));
	pMenu->CheckMenuItem(ID_SHOW_ATTACHMENTS, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bDrawAttachments));
	pMenu->CheckMenuItem(ID_SOLID_ATTACHMENTS, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bSolidAttachments));
	pMenu->CheckMenuItem(ID_SHOW_NORMALS, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bNormals));
	pMenu->CheckMenuItem(ID_SHOW_NORMALREF, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bShowNormalRef));
	pMenu->CheckMenuItem(ID_OPTIONS_PROFILE, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bProfile));

	pMenu->CheckMenuItem(ID_OPTIONS_SHOWVERTEXWEIGHTING, MENUCHECK(m_RenderWnd.m_DrawStruct.m_bDrawVertexWeights));

	pMenu->CheckMenuItem(ID_OPTIONS_LOADCONFIRM, MENUCHECK(m_bConfirmLoadDlg));
}


void CModelEditDlg::CheckChildModels(ModelLoadRequest *pRequest)
{
	Model *pModel;
	DWORD i;
	ChildInfo *pChild;
	CString str;

	
	pModel = m_pModel;
	if (!pModel)
		return;

	// Regenerate the trees.
	if (!pRequest->m_bTreesValid)
	{
		if (DoMessageBox(IDS_TREESINVALID, MB_YESNO) == IDYES)
		{
			for (i=0; i < pModel->NumChildModels(); i++)
			{
				pChild = pModel->GetChildModel(i);
				
				if (!pChild->m_bTreeValid)
				{
					str.FormatMessage(IDS_CANTCREATETREE, pChild->m_pFilename);
					DoMessageBox(str, MB_OK);
				}
			}
		}
	}

	// Now check to see if it's out of date with any of its child models.
	for (i=0; i < pModel->NumChildModels(); i++)
	{
		pChild = pModel->GetChildModel(i);

		// Was it unable to load?
		if (!pChild->m_pModel)
		{
			str.FormatMessage(IDS_CANT_LOAD_CHILD_MODEL, pChild->m_pFilename);
			DoMessageBox(str, MB_OK);
		}

		if (!pChild->m_bTreeValid)
			continue;
	}
}

// fill a piece list with a given models pieces
void CModelEditDlg::FillPieceList( CLTWinTreeMgr* tree, Model* model )
{
	DWORD i;
	char buf[256];

	tree->ClearSelection();
	tree->DeleteTree();

	if( !model )
		return;

	for( i = 0; i < model->NumPieces(); i++ )
	{
		// get the current piece
		ModelPiece* curPiece = model->GetPiece( i );
		if( !curPiece )
		{
			ASSERT(0);
			continue;
		}

		// add the parent to the tree control
		PieceListItem* item = new PieceListItem( curPiece->GetName() );
		item->m_LOD = -1;
		item->m_PieceNum = i;
		item->SetTextEditable( TRUE );
		CLTWinTreeKey parent = tree->AddItem( item, NULLITEM, FALSE );

		for( int j = 0; j < curPiece->NumLODs(); j++ )
		{
			sprintf( buf, "%.2f", curPiece->m_LODDists[j] );
			item = new PieceListItem( buf );
			item->m_LOD = j;
			item->m_PieceNum = i;
			item->SetTextEditable( FALSE );
			tree->AddItem( item, parent, FALSE );
		}
	}
}


void CModelEditDlg::FillNodeList()
{
	DWORD i;
	ModelNode *pNode;


	m_NodeList.DeleteAllItems();
	
	if (!m_pModel)
		return;

	g_bUpdateNodeFlags = false;

	m_bFillingNodeList = TRUE;

		for (i = 0; i < m_pModel->NumNodes(); i++)
		{
			pNode = m_pModel->GetNode(i);
			
			m_NodeList.InsertItem(i, pNode->GetName());
			m_NodeList.SetCheck((int)i, !!(pNode->m_Flags & MNODE_ROTATIONONLY));
		}

	g_bUpdateNodeFlags = true;

	m_bFillingNodeList = FALSE;
}


void CModelEditDlg::FillSocketList(BOOL bPreserveSel)
{
	DWORD i;
	ModelSocket *pSocket;
	int iLastSel;

	
	iLastSel = m_SocketList.GetCurSel();

	m_SocketList.ResetContent();

	if (!m_pModel)
		return;

	for (i=0; i < m_pModel->NumSockets(); i++)
	{
		pSocket = m_pModel->GetSocket(i);

		m_SocketList.AddString(pSocket->GetName());
	}

	if (bPreserveSel)
	{
		m_SocketList.SetCurSel(iLastSel);
	}
	else
	{
		m_SocketList.SetCurSel(0);
	}
}


void CModelEditDlg::FillAnimList()
{
	DWORD i;

	m_AnimList.DeleteAllItems();

	if (m_pModel)
	{
		for (i = 0; i < m_pModel->m_Anims; i++)
		{
			m_AnimList.InsertItem(i, m_pModel->GetAnim(i)->GetName());
			m_AnimList.SetCheck(i, (m_pModel->GetAnimInfo(i))->GetAnimOwner() == m_pModel);
		}
	}
}


void CModelEditDlg::FillChildModelList()
{
	ChildInfo *pChild;
	DWORD i;

	m_ChildModelList.ResetContent();

	if (m_pModel)
	{
		for (i=0; i < m_pModel->NumChildModels(); i++)
		{
			pChild = m_pModel->GetChildModel(i);

			if (pChild != m_pModel->GetSelfChildModel())
			{
				m_ChildModelList.AddString(pChild->m_pFilename);
			}
		}
	}
}


void CModelEditDlg::InitTheScrollBar(float max_dist )
{
	
	m_LODSlider.m_ScrollBar       = (CScrollBar*)GetDlgItem(IDC_LOD_SCROLL);

	m_LODSlider.m_CurrentValue = (CMyEdit*)GetDlgItem(IDC_CURRENT_DIST);
	m_LODSlider.m_MaxDist          = GetDlgItem(IDC_MAX_DIST);
	m_LODSlider.m_MinDist          = GetDlgItem(IDC_MIN_DIST);

	SCROLLINFO info;

	if (m_LODSlider.m_ScrollBar)
	{
		info.cbSize = sizeof(info);
		info.fMask = SIF_ALL;
		info.nMin = 0;
		info.nMax = (int)max_dist ;
		info.nPage = 1;
		info.nPos = info.nTrackPos = 0;
		
		m_LODSlider.m_ScrollBar->SetScrollInfo(&info);
	}

	m_LODSlider.SetMinDist(0);
	m_LODSlider.SetMaxDist(max_dist);
	m_LODSlider.SetCurrentValue(0);

}



/////////////////////////////////////////////////////////////////////////////
// CModelEditDlg message handlers

LONG CModelEditDlg::OnStartIdle (UINT, LONG)
{
	static BOOL bFirst=TRUE;

	// Handle startup options (we do it in the idle handler so we can do MessageBox).
	if (bFirst)
	{
		bFirst = FALSE;

		if (__argc > 1)
		{
			DoLoadModel(__argv[1], FALSE);
		}

		if (__argc > 2)
		{
			SetRenderTexture(__argv[2]);
		}
	}

	MSG msg;
	if (!PeekMessage(&msg, GetSafeHwnd(), 0,0, PM_NOREMOVE))
		OnIdle();
	m_cWinIdle.NextIdle();

	return 0;
}


BOOL CModelEditDlg::OnInitDialog()
{
	CButton *pButton;
	UINT keyframeWndIDs[NUM_ANIM_INFOS] =
	{
		IDC_KEYFRAME1,
		IDC_KEYFRAME2,
		IDC_KEYFRAME3,
		IDC_KEYFRAME4
	};
	UINT animActiveRectIDs[NUM_ANIM_INFOS] =
	{
		IDC_ANIMACTIVE1,
		IDC_ANIMACTIVE2,
		IDC_ANIMACTIVE3,
		IDC_ANIMACTIVE4
	};
	uint32 i;
	MEAnimInfo *pInfo;


	// Run in a low-priority thread so we're friendly in the background..
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);


	CDialog::OnInitDialog();

	CRect rect;

	// Set up the node view
	m_NodeList.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	m_NodeList.GetWindowRect(&rect);
	m_NodeList.InsertColumn(0, "Nodes", LVCFMT_LEFT, rect.Width() -17);

	// Set up the piece view
	m_PieceList.SubclassDlgItem( IDC_PIECES, this );
	m_PieceList.EnableMultiSelect( TRUE );
	m_PieceList.EnableEditText( TRUE );

	// Set up the animation list
	m_AnimList.SetDropNotify(this, (FDragListCallback)OnDropAnimList);
	m_AnimList.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	m_AnimList.GetWindowRect(&rect);
	m_AnimList.InsertColumn(0, "Name", LVCFMT_LEFT, rect.Width() - 17);

	// Setup the trackers.
	AddTracker(this, RGB(255,0,0), IDC_SOCKETROTRIGHT, RT_ROTATION, 0);
	AddTracker(this, RGB(0,255,0), IDC_SOCKETROTUP, RT_ROTATION, 1);
	AddTracker(this, RGB(0,0,255), IDC_SOCKETROTFORWARD, RT_ROTATION, 2);
	AddTracker(this, RGB(255,0,0), IDC_SOCKETPOSRIGHT, RT_POSITION, 0);
	AddTracker(this, RGB(0,255,0), IDC_SOCKETPOSUP, RT_POSITION, 1);
	AddTracker(this, RGB(0,0,255), IDC_SOCKETPOSFORWARD, RT_POSITION, 2);
	AddTracker(this, RGB(255,0,0), IDC_SOCKETSCLRIGHT, RT_SCALE, 0);
	AddTracker(this, RGB(0,255,0), IDC_SOCKETSCLUP, RT_SCALE, 1);
	AddTracker(this, RGB(0,0,255), IDC_SOCKETSCLFORWARD, RT_SCALE, 2);


	if (pButton = GetTransformEdit(IDC_SOCKET_EDIT))
	{
		pButton->SetCheck(TRUE);
	}

	if (pButton = (CButton*)GetDlgItem(IDC_DIMS_EDIT))
	{
		pButton->SetCheck(TRUE);
	}


	m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	SetFocus();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// Initialize the Bitmap Buttons

	m_btnPrevKey.AutoLoad (IDC_PREVKEY, this);
	m_btnStop.AutoLoad (IDC_STOP, this);
	m_btnFPlay.AutoLoad (IDC_FPLAY, this);
	m_btnNextKey.AutoLoad (IDC_NEXTKEY, this);
	m_btnMoveUp.AutoLoad (IDC_MOVEUP, this);
	m_btnMoveDown.AutoLoad (IDC_MOVEDOWN, this);
	m_btnNumber.AutoLoad (IDC_NUMBER, this);
	m_btnDelete.AutoLoad (IDC_DELETE, this);

	CheckDlgButton (IDC_CAMERA_FOLLOW, TRUE);

	// create the render window

	CWnd* pStaticWnd = GetDlgItem (IDC_RENDER);
	CRect rcStatic;
	pStaticWnd->GetWindowRect (rcStatic);
	ScreenToClient (rcStatic);
	if (!m_RenderWnd.Create (NULL, "Render Window", WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, rcStatic, this, -1, NULL))
	{
		MessageBox ("Could not create render window", NULL, MB_OK);
		EndDialog (-1);
	}

	/*
	m_RenderWnd.SetWireframe (IsDlgButtonChecked (IDC_WIREFRAME));
	m_RenderWnd.SetCameraFollow (IsDlgButtonChecked (IDC_CAMERA_FOLLOW));
	m_RenderWnd.DrawDims (IsDlgButtonChecked (IDC_DIMS));
	m_RenderWnd.DrawModelBox (IsDlgButtonChecked (IDC_MODEL_BOX));
	m_RenderWnd.DrawAnimBox (IsDlgButtonChecked (IDC_ANIM_BOX));
	*/

	// t.f important
	m_RenderWnd.m_bCameraFollow = TRUE;
	m_RenderWnd.m_DrawStruct.m_bWireframe = FALSE;
	m_RenderWnd.m_DrawStruct.m_bNormals = FALSE;
	m_RenderWnd.m_DrawStruct.m_bDims = FALSE;
	m_RenderWnd.m_DrawStruct.m_bAnimBox = FALSE;
	m_RenderWnd.m_DrawStruct.m_bDrawOriginalModel = FALSE;
	m_RenderWnd.m_DrawStruct.m_bDrawVertexWeights = FALSE;
	m_RenderWnd.m_DrawStruct.m_bMovementEncoding = false;

	m_RenderWnd.m_DrawStruct.m_bDrawTextures = TRUE;
	m_RenderWnd.m_DrawStruct.m_bDrawSockets = FALSE;
	m_RenderWnd.m_DrawStruct.m_bDrawAttachments = FALSE;
	m_RenderWnd.m_DrawStruct.m_bSolidAttachments = TRUE;
	m_RenderWnd.m_DrawStruct.m_bShowNormalRef = FALSE;
	m_RenderWnd.m_DrawStruct.m_bProfile = FALSE;
	UpdateMenuChecks();

	m_RenderWnd.m_DrawStruct.m_FOV = CalcFOV();
	// Update the registry with the FOV, just in case it's not there..
	SetFOV(m_RenderWnd.m_DrawStruct.m_FOV);
	
	InitTheScrollBar(0);

	
	
	// create the keyframe windows.
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = GetAnimInfo(i);

		// Create the keyframe window.
		if(pStaticWnd = GetDlgItem (keyframeWndIDs[i]))
		{
			pStaticWnd->GetWindowRect (rcStatic);
			ScreenToClient (rcStatic);
			if (!pInfo->m_Wnd.Create (NULL, "Keyframe Window", WS_CHILD | WS_VISIBLE, rcStatic, this, -1, NULL))
			{
				MessageBox ("Could not create keyframe window", NULL, MB_OK);
				EndDialog (-1);
			}
		}

		// Create the flags window.
		if(pStaticWnd = GetDlgItem(animActiveRectIDs[i]))
		{
			pStaticWnd->GetWindowRect (rcStatic);
			ScreenToClient (rcStatic);
			if (!pInfo->m_ActiveFlagsWnd.Create (NULL, "Anim Active Flags", WS_CHILD | WS_VISIBLE, rcStatic, this, -1, NULL))
			{
				MessageBox ("Could not create anim flags window", NULL, MB_OK);
				EndDialog (-1);
			}
		}	

		m_AnimInfos[i].m_Wnd.EnableWindow(TRUE);
	}

	// disable appropriate windows 

	m_btnPrevKey.EnableWindow (FALSE);
	m_btnStop.EnableWindow (FALSE);
	m_btnFPlay.EnableWindow (FALSE);
	m_btnNextKey.EnableWindow (FALSE);
	m_btnMoveUp.EnableWindow (FALSE);
	m_btnMoveDown.EnableWindow (FALSE);
	m_btnDelete.EnableWindow (FALSE);
	m_AnimList.EnableWindow (FALSE);

	// Start the idle loop (Note : It is currently set to not idle more quickly than every 20ms)
	m_cWinIdle.StartIdle(GetSafeHwnd(), WM_STARTIDLE, 0,0, 20);
	m_cWinIdle.NextIdle();

	UpdateEditFrameString( );
	UpdateEditFrameTime( );

	// Create the import string keys dialog

	m_pImportStringKeyDlg = new CImportStringKeysDlg;
	m_pImportStringKeyDlg->Create(IDD_IMPSTRINGKEYS, this);	

	return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CModelEditDlg::OnPaint() 
{
	CRect rect;
	CBrush brush;
	CPaintDC dc(this); // device context for painting


	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CModelEditDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CModelEditDlg::OnLoadModel() 
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	// Look up the open directory in the registry
	CRegMgr regMgr;
	CString csOpenDir;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		regMgr.Get("Model", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		csOpenDir.ReleaseBuffer(-1);
	}

	csOpenDir += "*.lta;*.ltc";

	// get the filename
	CString sFilter;
	sFilter.LoadString(IDS_LOADMODELFILTER);

	CFileDialog dlg (TRUE, "lta", (csOpenDir.GetLength()) ? (LPCTSTR)csOpenDir : NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, sFilter, this);

	if (dlg.DoModal() != IDOK)
	{ 
		return;
	}

	// Store the open directory in the registry
	csOpenDir = dlg.GetPathName();
	csOpenDir = csOpenDir.Left(csOpenDir.GetLength() - dlg.GetFileName().GetLength());
	regMgr.Set("Model", (LPCTSTR)csOpenDir);
	
	CString pathName = dlg.GetPathName() ;

	m_sModelPath = pathName;
	DoLoadModel((char*)(LPCTSTR)pathName);
}


// import custom lods from another model file
void CModelEditDlg::OnImportLODs()
{
	if( !m_pModel )
	{
		DoMessageBox( "A model must be loaded before attempting to import LODs.", MB_OK|MB_ICONERROR );
		return;
	}

	IdleChanger idle( &m_cWinIdle, IDLE_DIALOG_DELAY );

	// Look up the open directory in the registry
	CRegMgr regMgr;
	CString openDir;
	if( regMgr.Init( szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER ) )
	{
		UINT32 dwSize = 256;
		regMgr.Get( "Model", openDir.GetBufferSetLength( dwSize ), dwSize );
		openDir.ReleaseBuffer( -1 );
	}
	openDir += "*.lta;*.ltc";

	// get the filename
	CString filter;
	filter.LoadString(IDS_LOADMODELFILTER);
	CFileDialog fileDlg( TRUE, "lta", (openDir.GetLength()) ? (LPCTSTR)openDir : NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, filter, this );
	if( fileDlg.DoModal() != IDOK )
		return;
	CString pathName = fileDlg.GetPathName() ;

	// load the model
	Model* model;
	CLTAReader inFile;

	BeginWaitCursor();

	inFile.Open( (char*)(LPCSTR)pathName, CLTAUtil::IsFileCompressed( (char*)(LPCSTR)pathName ) );
	
	if( inFile.IsValid() )
	{
		model = ltaModelLoad( inFile, m_AllocCount, false );

		if( !model )
		{
			DoMessageBox( "Error loading model.", MB_OK|MB_ICONERROR );
			return;
		}
	}

	EndWaitCursor();

	// launch the import lods dialog
	CImportLODDlg dlg;

	dlg.m_ImportModel = model;
	dlg.m_CurrentModel = m_pModel;
	dlg.m_ModelEditDlg = this;

	if( dlg.DoModal() != IDOK )
	{
		delete model;
		return;
	}

	// process the selected import LODs
	const vector<PieceLODInfo>& selection = dlg.m_Selection;
	int numSelected = selection.size();

	int numGoodAdds = 0;		// number of imported LODs that were added successfully
	int numBadMatches = 0;		// number of imported LODs that don't match an existing piece by name
	int numBadVA = 0;			// number of imported LODs that don't match piece types (VA->Skel/Skel->VA)
	int numBadNodes = 0;		// number of imported LODs that failed to find matching nodes
	int numBadAdds = 0;			// number of imported LODs that failed to import for some other reason

	for( int i = 0; i < numSelected; i++ )
	{
		ASSERT( selection[i].m_ModelPiece && selection[i].m_PieceLOD );

		// find the matching piece by name
		char* pieceName = selection[i].m_ModelPiece->GetName();
		ModelPiece* curPiece = m_pModel->FindPiece( pieceName );

		// didn't find a matching piece by name
		if( !curPiece )
		{
			numBadMatches++;
			continue;
		}

		// pieces don't match animation types
		if( curPiece->m_isVA != selection[i].m_ModelPiece->m_isVA )
		{
			numBadVA++;
			continue;
		}

		// create a new LOD and isolate its vertex weights
		PieceLOD newLOD;
		newLOD = *(selection[i].m_PieceLOD);

		for( int curVert = 0; curVert < newLOD.m_Verts; curVert++ )
		{
			int numWeights = newLOD.m_Verts[curVert].m_nWeights;

			if( numWeights > 0 )
			{
				newLOD.m_Verts[curVert].m_Weights = new NewVertexWeight[numWeights];
				for( int curWeight = 0; curWeight < numWeights; curWeight++ )
				{
					newLOD.m_Verts[curVert].m_Weights[curWeight] = selection[i].m_PieceLOD->m_Verts[curVert].m_Weights[curWeight];
				}
			}
		}

		// attemp to remap the node indices on the imported LOD
		if( !newLOD.RemapNodeIndices( m_pModel ) )
		{
			numBadNodes++;
			continue;
		}

		newLOD.m_pModel = m_pModel;

		// attempt to add the LOD
		float dist = selection[i].m_ModelPiece->GetLODDist( selection[i].m_LODNum );

		uint32 addSuccess = curPiece->AddLOD( newLOD, dist );

		if( addSuccess == 0 )
		{
			numBadAdds++;
			continue;
		}

		numGoodAdds++;
	}

	delete model;

	if( numSelected == 0 )
		return;

	// report on the import operation
	CString importReport;
	CString tmpReport;

	tmpReport.Format( "Successfully added %d Piece LOD%s to the model.", numGoodAdds, (numGoodAdds == 1) ? "" : "s" );
	importReport += tmpReport;

	if( numBadMatches > 0 )
	{
		tmpReport.Format( "\nFailed to add %d Piece LOD%s because no pieces matched by name.", numBadMatches, (numBadMatches == 1) ? "" : "s" );
		importReport += tmpReport;
	}

	if( numBadVA > 0 )
	{
		tmpReport.Format( "\nFailed to add %d Piece LOD%s because they didn't have matching animation types.", numBadVA, (numBadVA == 1) ? "" : "s" );
		importReport += tmpReport;
	}

	if( numBadNodes > 0 )
	{
		tmpReport.Format( "\nFailed to add %d Piece LOD%s because matching nodes couldn't be found.", numBadNodes, (numBadNodes == 1) ? "" : "s" );
		importReport += tmpReport;
	}

	if( numBadAdds > 0 )
	{
		ASSERT(0);
		tmpReport.Format( "\nFailed to add %d Piece LOD%s for an unknown reason.", numBadAdds, (numBadAdds == 1) ? "" : "s" );
		importReport += tmpReport;
	}

	// update the piece list if any LODs were added
	if( numGoodAdds > 0 )
	{
		// reload the piece tree
		FillPieceList( &m_PieceList, m_pModel);

		// adjust the LOD slider
		float maxLODDist = GetModelMaxLODDist( m_pModel );
		InitTheScrollBar( maxLODDist );
		SetCurrentLOD( 0.0f );

		SetChangesMade();
	}

	DoMessageBox( importReport, MB_OK );
}


// ------------------------------------------------------------------------
// OnImport stuff from other data files.
// The lta part could be vastly improved by accessing the parts of the
// lta that are needed instead of loading up the whole model and tossing 
// unwanted stuff out. For now this will stand
// ------------------------------------------------------------------------
void CModelEditDlg::OnImport() 
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	UVImportDlg uvImportDlg;
	char msg[256];
	DWORD i, nFound;
    uint32 j;
	ModelAnim *pNewAnim, *pImportAnim;
	ModelLoadRequest request;
	AnimInfo *pAnimInfo, *pOtherAnimInfo;
	ModelSocket *pSocket, *pOtherSocket;
	WeightSet *pWeightSet, *pOtherWeightSet;
	uint32 iWeightSetIndex;
	BuildLODRequest LODRequest;
//	LODRequestInfo LODInfo;

	// Make sure we have a model loaded

	if (!m_pModel)
	{
		int nRet = AfxMessageBox("No model currently loaded, load one ?", MB_YESNO | MB_ICONEXCLAMATION);
		if (nRet == IDYES)
		{
			// Let the user load a model
			
			OnLoadModel();

			if (!m_pModel)
			{
				// No model loaded
				
				return;
			}

			AfxMessageBox("Model loaded, continuing to Import");
		}
		else
		{
			return;
		}
	}

	// Look up the open directory in the registry
	CRegMgr regMgr;
	CString csOpenDir;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		regMgr.Get("Model", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		csOpenDir.ReleaseBuffer(-1);
	}

	//load strings needed for the open file dialog
	CString sImportFilter;
	sImportFilter.LoadString(IDS_IMPORTFILTER);

	CString sImportDefExt;
	sImportDefExt.LoadString(IDS_IMPORTDEFEXT);

	// Strip the extension...
	char* pExt = _tcsrchr(csOpenDir,'.'); 
	if (pExt) 
	{ 
		csOpenDir.TrimRight(pExt); 
	}
	csOpenDir += "*.lta;*.ltc";

	// get the filename

	CImportAnimation dlg(TRUE, sImportDefExt, (csOpenDir.GetLength()) ? (LPCTSTR)csOpenDir : NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, sImportFilter, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	POSITION startPos = dlg.GetStartPosition();

	if (m_pImportStringKeyDlg)
	{
		m_pImportStringKeyDlg->Clear();
		m_pImportStringKeyDlg->ShowWindow(SW_SHOW);
	}

	// Store the open directory in the registry
	csOpenDir = dlg.GetPathName();
	csOpenDir = csOpenDir.Left( csOpenDir.GetLength() - dlg.GetFileName().GetLength() );
	regMgr.Set("Model", (LPCTSTR)csOpenDir);

	while (startPos)
	{
//		Model import;
		Model *pModel ;			

		bool bSuccessfulLoad = false;

		CString sFileName = dlg.GetNextPathName(startPos);

		m_pImportStringKeyDlg->AddMsg("----------------------------------------\r\n", 128, 0, 0);
		CString sMsg;
		sMsg.Format("Loading %s\r\n", (char *)(LPCSTR)sFileName);
		m_pImportStringKeyDlg->AddMsg(sMsg, 0, 0, 0);
		
		// Test to see whether file is an LTA.

		if (IsFilenameLTAExt(sFileName))
		{		
			CLTAReader InFile;
			InFile.Open((char *)(LPCSTR)sFileName, CLTAUtil::IsFileCompressed((char *)(LPCSTR)sFileName));

			if (InFile.IsValid())
			{
				pModel = ltaModelLoad(InFile, m_AllocCount, false);

				//pModel = ltaModelLoad("", NULL);
				if (pModel)
				{
					bSuccessfulLoad = true;
					//pModel = load_LTA_Using_FeedBackDialog((char*)(LPCTSTR)sFileName);
			//		import.CopyModel( pModel );
			//		delete pModel;
				}
			}
		}
		else  
		{
			// Load the ABC
			MessageBox("ABC LOAD ATTEMPTED IN ON_IMPORT()", "ERROR " , MB_OK );	
		}
		
		if (bSuccessfulLoad)
		{
			Model& import = *pModel;

			// these import options require matching models
			if( !VerifyModels( m_pModel, &import ) )
			{
				if (m_pImportStringKeyDlg)
				{
					CString sMsg;
					sMsg.Format("%s model geometry does not match, cannot import.\r\n");
					m_pImportStringKeyDlg->AddMsg(msg, 0, 0, 0);
				}

				//MessageBox ("Model geometry does not match.  Cannot import from model.", "Error", MB_OK | MB_ICONEXCLAMATION);
				//return;
			}
			else
			{
				// now allocate new array for anims
				if ( dlg.m_bImportAnimations )
				{
					for (i=0; i < import.m_Anims; i++)
					{
						pNewAnim = new ModelAnim(m_pModel);
						pImportAnim = import.GetAnim(i);

						pNewAnim->CopyAnim(pImportAnim);
						m_pModel->AddAnimToSelf(pNewAnim);
					}


					// add the animations to the listbox
					FillAnimList();


					// re-allocate the tagged array
					InitTaggedArrays();

					// let the user know

//					if (import.m_Anims > 1)
//					{
//						MessageBox ("New animations have been added to end of listbox", "Model Editor", MB_OK | MB_ICONINFORMATION);
//					}
//					else
//					{
//						MessageBox ("New animation has been added to end of listbox", "Model Editor", MB_OK | MB_ICONINFORMATION);
//					}

					if (m_pImportStringKeyDlg)
					{
						CString sMsg;
						sMsg.Format("Anims from %s imported\r\n", (char *)(LPCSTR)sFileName);
						m_pImportStringKeyDlg->AddMsg(sMsg, 0, 0, 0);
					}

					m_bChangesMade = TRUE;
				}

				if ( dlg.m_bUseUVCoords )
				{
					if (m_pModel->CopyUVs(&import))
					{
						if (m_pImportStringKeyDlg)
						{
							CString sMsg;
							sMsg.Format("UV's from %s imported\r\n", (char *)(LPCSTR)sFileName);
							m_pImportStringKeyDlg->AddMsg(sMsg, 0, 0, 0);
						}

//						MessageBox ("UV Coordinates imported.", "Model Editor", MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						if (m_pImportStringKeyDlg)
						{
							m_pImportStringKeyDlg->AddMsg("Geometry does not match, cannot import UV's\r\n", 0, 0, 0);
						}

//						MessageBox ("Model geometry does not match.  Cannot import UV coords.", "Error", MB_OK | MB_ICONEXCLAMATION);
					}
				}

				if (dlg.m_bImportTranslations)
				{
					BeginWaitCursor();

					nFound = 0;
					for (i=0; i < import.m_Anims; i++)
					{
						pAnimInfo = import.GetAnimInfo(i);
						pOtherAnimInfo = m_pModel->FindAnimInfo(pAnimInfo->m_pAnim->GetName(), m_pModel);

						if (pOtherAnimInfo)
						{
							++nFound;
							pOtherAnimInfo->m_vTranslation = pAnimInfo->m_vTranslation;
						}
					}

					sprintf(msg, "Imported translations from %d animations.\r\n", nFound);
//					MessageBox(msg, "ModelEdit", MB_OK);
					if (m_pImportStringKeyDlg)
					{
						m_pImportStringKeyDlg->AddMsg(msg, 0, 0, 0);
					}

					EndWaitCursor();
				}

				// Import the weight sets
				if (dlg.m_bImportWeightSets)
				{
					BeginWaitCursor();
					nFound = 0;
					for (i=0; i < import.NumWeightSets(); i++)
					{
						pWeightSet = import.GetWeightSet(i);
						pOtherWeightSet = m_pModel->FindWeightSet(pWeightSet->GetName(), &iWeightSetIndex);

						if (pOtherWeightSet)
							m_pModel->RemoveWeightSet(iWeightSetIndex);
						pOtherWeightSet = new WeightSet(m_pModel);
						if (!pOtherWeightSet)
							continue;
						pOtherWeightSet->SetName(pWeightSet->GetName());
						if (!(pOtherWeightSet->m_Weights.CopyArray(pWeightSet->m_Weights)))
						{
							delete pOtherWeightSet;
							continue;
						}
						if (!m_pModel->AddWeightSet(pOtherWeightSet))
						{
							delete pOtherWeightSet;
							continue;
						}
						++nFound;
					}

					sprintf(msg, "Imported %d weight sets.\r\n", nFound);
					if (m_pImportStringKeyDlg)
					{
						m_pImportStringKeyDlg->AddMsg(msg, 0, 0, 0);
					}
//					MessageBox(msg, "ModelEdit", MB_OK);
					
					EndWaitCursor();
				}
			}

			// Import user dims (doesn't matter if models don't match).
			if (dlg.m_bImportUserDims)
			{
				BeginWaitCursor();

				nFound = 0;
				for (i=0; i < import.m_Anims; i++)
				{
					pAnimInfo = import.GetAnimInfo(i);
					pOtherAnimInfo = m_pModel->FindAnimInfo(pAnimInfo->m_pAnim->GetName(), m_pModel);

					if (pOtherAnimInfo)
					{
						++nFound;
						pOtherAnimInfo->m_vDims = pAnimInfo->m_vDims;
					}
				}

				sprintf(msg, "Imported user dims from %d animations.\r\n", nFound);
				if (m_pImportStringKeyDlg)
				{
					m_pImportStringKeyDlg->AddMsg(msg, 0, 0, 0);
				}
//				MessageBox(msg, "ModelEdit", MB_OK);

				EndWaitCursor();
			}

			// Import sockets (doesn't matter if models don't match)
			if (dlg.m_bImportSockets)
			{
				BeginWaitCursor();
				nFound = 0;
				for (i=0; i < import.NumSockets(); i++)
				{
					pSocket = import.GetSocket(i);
					pOtherSocket = m_pModel->FindSocket(pSocket->GetName());
					
					if (pOtherSocket)
					{
						++nFound;
						pOtherSocket->m_Pos = pSocket->m_Pos;
						pOtherSocket->m_Rot = pSocket->m_Rot;
					}
					else if (m_pModel->FindNode(import.GetNode(pSocket->m_iNode)->GetName(), &j))
					{
						pOtherSocket = new ModelSocket;
						pOtherSocket->m_iNode = j;
						pOtherSocket->SetName(pSocket->GetName());
						pOtherSocket->m_Pos = pSocket->m_Pos;
						pOtherSocket->m_Rot = pSocket->m_Rot;
						if (m_pModel->AddSocket(pOtherSocket))
							++nFound;
						else
							delete pOtherSocket;											
					}
				}
				if (nFound)
					FillSocketList(TRUE);
				sprintf(msg, "Imported %d sockets.\r\n", nFound);
				if (m_pImportStringKeyDlg)
				{
					m_pImportStringKeyDlg->AddMsg(msg, 0, 0, 0);
				}
//				MessageBox(msg, "ModelEdit", MB_OK);

				EndWaitCursor();
			}

			delete pModel;
		}
	}
}



// ------------------------------------------------------------------------
// OnSetProjectDir
// user asked to change the project dir.
// set the new project dir in the registry for later.
// ------------------------------------------------------------------------
void CModelEditDlg::OnSetProjectDir()
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
//
	//// Look up the open directory in the registry
	CRegMgr regMgr;
	CString csOpenDir;
//	
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER)) 
	{
		UINT32 dwSize = 256;
		regMgr.Get("dep_file_location", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		csOpenDir.ReleaseBuffer(-1); 

	}

	CDirDialog dlg;

	dlg.m_strTitle = "Select Project Directory";
	dlg.m_strSelDir = csOpenDir ;

	if( dlg.DoBrowse() )
	{
		csOpenDir = dlg.m_strPath ;
		// make sure the project path ends with a backslash
		if( csOpenDir.GetLength() )
		{
			if( csOpenDir[csOpenDir.GetLength()-1] != '\\' )
				csOpenDir += '\\';
		}
		regMgr.Set("dep_file_location", csOpenDir );
		m_szCurProjectPath = csOpenDir;

		// change the window title.. 
		CString titleText;
		if( m_pModel )
		titleText.FormatMessage("ModelEdit: %1!s! %2!s!", m_pModel->GetFilename(),m_szCurProjectPath);
		else{
		titleText.FormatMessage("ModelEdit - %1!s! ",m_szCurProjectPath);
		}
		SetWindowText((LPCTSTR)titleText);
	}
	else// canceled..
	{
		// just set it what's in the registry
		m_szCurProjectPath = csOpenDir;
	}
}


// ------------------------------------------------------------------------
// Save, then call external packer with d3d bound args
// ------------------------------------------------------------------------
void CModelEditDlg::OnSaveAndCompile()
{
	CompileD3D( true );
}

// ------------------------------------------------------------------------
// Call external packer with d3d bound args
// ------------------------------------------------------------------------
void CModelEditDlg::OnCompile()
{
	CompileD3D( false );
}




void CModelEditDlg::CompileD3D( bool saveBeforeCompile )
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	bool	bForceCustomTransform = false;
	uint32	iMaxBonesPerVert = 4;
	uint32	iMaxBonesPerTri  = 4;
	float fMinBoneWeight;

	// default to the currently opened models filename with an .ltb extension
	CString csOpenDir = m_strFilename;
	int extPos = csOpenDir.ReverseFind( '.' );
	if( extPos >= 0 )
		csOpenDir = csOpenDir.Left( extPos ) + ".ltb";

	// Get the filename
	char szFilter[] = "LTB Files (*.ltb)|*.ltb|All Files (*.*)|*.*||";
	CExportD3D_Dlg dlg (FALSE, "ltb", (csOpenDir.GetLength()) ? (LPCTSTR)csOpenDir : NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);
	
	
	dlg.m_bUseMatrixPalettes	= bForceCustomTransform;
	dlg.m_iMaxBonesPerVert		= iMaxBonesPerVert;
	dlg.m_bMaxBonesPerVert		= true;
	dlg.m_iMaxBonesPerTri		= iMaxBonesPerTri;
	dlg.m_bMaxBonesPerTri		= false;
	dlg.m_fMinBoneWeight		= 0.05f;
	dlg.m_bPauseAfterCompile	= false;
	// get the next values from the model database.
	dlg.m_bExcludeGeometry      = (m_pModel)?m_pModel->m_bExcludeGeom:false;
	dlg.m_CompressionType      = (CExportD3D_Dlg::EAnimCompressionType)((m_pModel)?m_pModel->m_CompressionType:2);
	
	if (dlg.DoModal() != IDOK) 
		{ return; }

	// get user input.
	if(m_pModel)
	{
		m_pModel->m_CompressionType = dlg.m_CompressionType ;
		m_pModel->m_bExcludeGeom    = dlg.m_bExcludeGeometry;
	}

	if( saveBeforeCompile )
		OnSave();

	
	bForceCustomTransform					= dlg.m_bUseMatrixPalettes;
	
	iMaxBonesPerVert						= dlg.m_iMaxBonesPerVert;
	if (!dlg.m_bMaxBonesPerVert) 
		iMaxBonesPerVert = 99;
	
	iMaxBonesPerTri							= dlg.m_iMaxBonesPerTri;
	if (!dlg.m_bMaxBonesPerTri)  
		iMaxBonesPerTri = 99;

	fMinBoneWeight		= dlg.m_fMinBoneWeight;

	// get the path from the dialog
	csOpenDir = dlg.GetPathName();
	csOpenDir = csOpenDir.Left( csOpenDir.GetLength() - dlg.GetFileName().GetLength() );
	
	// Export the file...
	LithTry {
		// Save off a TEMP LTA File...
		CString szTmpFileName = csOpenDir; 

		//make sure it has a trailing slash
		if(!szTmpFileName.IsEmpty() && (szTmpFileName[szTmpFileName.GetLength() - 1] != '\\'))
			szTmpFileName += '\\';

		szTmpFileName += "TMP_ModelEditLTAFile.lta";
		if( !DoSave(szTmpFileName) )
		{
			MessageBox( "Error: Could not save intermediate lta file", "File Error", MB_OK | MB_ICONEXCLAMATION );
			return;
		}

		// check for a model_packer in the launch directory
		CString szRun = g_szStartPath;
		szRun += "\\Model_Packer.exe";
		CFileStatus fileStatus;
		if( !CFile::GetStatus( szRun, fileStatus ) )
		{
			// couldn't find it there, check the directory that modeledit is in
			szRun = g_szExePath;
			szRun += "\\Model_Packer.exe";
			if( !CFile::GetStatus( szRun, fileStatus ) )
			{
				MessageBox( "Error: Could not locate Model_Packer.exe\nCheck that Model_Packer.exe is located in\nthe ModelEdit launch dir,\nor in the same directory as ModelEdit.exe", "File Error", MB_OK | MB_ICONEXCLAMATION );
				return;
			}
		}

		// Call the Model_Packer to convert it to LTB...
		CString szProcessCmd = "d3d";
		CString szInput1  = "-input";
		CString szInput2  = '\"' + szTmpFileName + '\"';
		CString szOutput1 = "-output";
		CString szOutput2 = '\"' + csOpenDir + dlg.GetFileName() + '\"';
		CString szMaxBonesPerVert1 = "-MaxBonesPerVert";
		CString szVerbose = "-verbose";
		char szMaxBonesPerVert2[8]; sprintf(szMaxBonesPerVert2,"%d",iMaxBonesPerVert);
		CString szMaxBonesPerTri1  = "-MaxBonesPerTri";
		char szMaxBonesPerTri2[8];  sprintf(szMaxBonesPerTri2,"%d",iMaxBonesPerTri);
		CString szForceCustomTransform1 = "-UseMatrixPalettes";
		char szForceCustomTransform2[8]; sprintf(szForceCustomTransform2,"%d",bForceCustomTransform?1:0);
		CString szStall = "";
		if( dlg.m_bPauseAfterCompile )
			szStall = "-stall";
		CString szNoGeom = "";
		if( dlg.m_bExcludeGeometry )
			szNoGeom = "-nogeom";

		// [TMG] [10/31/2000] Added min bone weight
		
		CString szMinBoneWeight1 = "-MinBoneWeight";
		char szMinBoneWeight2[16];
		sprintf(szMinBoneWeight2, "%4.4f", fMinBoneWeight);
 
		CString szCompressionType ;
		switch(dlg.m_CompressionType){
			case CExportD3D_Dlg::NONE : szCompressionType = "-ac0"; break;
			case CExportD3D_Dlg::RELEVANT : szCompressionType = "-ac1" ; break ;
			case CExportD3D_Dlg::RELEVANT_16bit : szCompressionType = "-ac2" ; break ;
			case CExportD3D_Dlg::REL_PV16 : szCompressionType = "-acpv" ; break ;
			default : szCompressionType =  "-ac0" ;
		}

		char szStream0OptionsBuffer[256];
		memset(szStream0OptionsBuffer, 0, 256);

		if(dlg.m_bStreamOptionsPosition)
		{
			strcat(szStream0OptionsBuffer, "position ");
		}

		if(dlg.m_bStreamOptionsNormal)
		{
			strcat(szStream0OptionsBuffer, "normal ");
		}

		if(dlg.m_bStreamOptionsColor)
		{
			strcat(szStream0OptionsBuffer, "color ");
		}

		if(dlg.m_bStreamOptionsTangent)
		{
			strcat(szStream0OptionsBuffer, "basisvectors ");
		}

		if(dlg.m_bStreamOptionsUV1)
		{
			strcat(szStream0OptionsBuffer, "uv1 ");
		}
	
		if(dlg.m_bStreamOptionsUV2)
		{
			strcat(szStream0OptionsBuffer, "uv2 ");
		}

		if(dlg.m_bStreamOptionsUV3)
		{
			strcat(szStream0OptionsBuffer, "uv3 ");
		}

		if(dlg.m_bStreamOptionsUV4)
		{
			strcat(szStream0OptionsBuffer, "uv4 ");
		}

		char szStream0Options[512];

		if(szStream0OptionsBuffer[0] != '\0')
		{

			sprintf(szStream0Options, "-stream0 %s", szStream0OptionsBuffer);

		}


		if (_spawnl(_P_WAIT,szRun,"Model_Packer.exe",
					szProcessCmd,
					szInput1,
					szInput2,
					szOutput1,
					szOutput2,
					szMaxBonesPerVert1,
					szMaxBonesPerVert2,
					szMaxBonesPerTri1,
					szMaxBonesPerTri2,
					szForceCustomTransform1,
					szForceCustomTransform2,
					szMinBoneWeight1,
					szMinBoneWeight2,
					szVerbose,
					szCompressionType,
					szNoGeom,
					szStream0Options,
					szStall,
					
					NULL) < 0) 
		{
			MessageBox ("Error: Could not launch Model_Packer.exe", "File Error", MB_OK | MB_ICONEXCLAMATION);
			return; 
		} 

	

		DeleteFile(szTmpFileName);			// Delete that temp LTA file...
	}
	LithCatch(CLithException &exception) {
		exception = exception;
		MessageBox ("Error: Could not export file", "File Error", MB_OK | MB_ICONEXCLAMATION);
		return; }

}


int CModelEditDlg::DoMessageBox(UINT idString, int style)
{
	CString str, caption;

	str.LoadString(idString);
	caption.LoadString(IDS_MODELEDIT_CAPTION);
	return MessageBox(str, caption, style);
}


int CModelEditDlg::DoMessageBox(LPCTSTR pStr, int style)
{
	CString caption;

	caption.LoadString(IDS_MODELEDIT_CAPTION);
	return MessageBox(pStr, caption, style);
}


BOOL CModelEditDlg::GetEditAnimInfo(DWORD *pAnimIndex, DWORD *pKeyframeIndex, bool bAllowChildAnims)
{
	DWORD iAnim, iKeyframe;

	if(!m_pModel)
		return FALSE;

	iAnim = m_AnimInfos[0].CurAnimIndex();
	if(iAnim >= m_pModel->NumAnims())
		return FALSE;

	if(!bAllowChildAnims && !m_pModel->GetAnimInfo(iAnim)->IsParentModel())
		return FALSE;

	iKeyframe = m_AnimInfos[0].CurKeyframe();
	if(iKeyframe >= m_pModel->GetAnim(iAnim)->NumKeyframes())
		return FALSE;

	if(pAnimIndex)
		*pAnimIndex = iAnim;

	if(pKeyframeIndex)
		*pKeyframeIndex = iKeyframe;

	return TRUE;
}

// ------------------------------------------------------------------------
// DoSave( filename )
// save out file 
// ------------------------------------------------------------------------
bool CModelEditDlg::DoSave(const char *pFilename)
{
	DWORD i, j, nParentAnims;
	ModelAnim *pAnim, *pTest;
//	DStream *pStream;
	CString str;
	int ret;

	if (!m_pModel)
		return false;

	//make sure that the artists have valid dimensions
	CheckValidAnimationDims();

	// Look for duplicate animation names. These can cause problems.
	m_pModel->ForceAnimOrder();
	
	nParentAnims = m_pModel->CalcNumParentAnims();
	for(i=0; i < nParentAnims; i++)
	{
		pAnim = m_pModel->GetAnimInfo(i)->m_pAnim;

		for(j=0; j < nParentAnims; j++)
		{
			if(i == j)
				continue;

			pTest = m_pModel->GetAnimInfo(j)->m_pAnim;

			if(strcmp(pAnim->GetName(), pTest->GetName()) == 0)
			{
				str.FormatMessage(IDS_DUPLICATE_ANIM_NAME, pAnim->GetName());
				ret = DoMessageBox(str, MB_YESNOCANCEL);
				if(ret == IDCANCEL)
				{
					return false;
				}
				else if(ret == IDYES)
				{
					DoRenameAnim(i);
				}
			}
		}
	}
 
	extern bool ltaSave( const char *, Model *, CLTATranslStatus &);

	// check if we are trying to save an lta type file...
	if( pFilename )
	{
		if( IsFilenameLTAExt( pFilename ) )
		{
			BeginWaitCursor();
			CLTATranslStatus status ;
			m_pModel->m_sProjectDir = m_szCurProjectPath;
			bool retVal = ltaSave( pFilename, m_pModel, status );

			//make sure that the file saved correctly. If not, odds are it was
			//a read only file, and we should inform the user of it -JohnO
			if(status != CLTATranslStatus::OK)
			{
				char pszError[128];
				sprintf(pszError, "Error opening %s, The file may be read only", pFilename);
				status.OnError(pszError);
			}

			EndWaitCursor();

			return retVal;
		}
		else
		{
			DoMessageBox( "Unrecognized file extension.  Save failed.", MB_OK );
			return false;
		}
	}

	return false;
}


void CModelEditDlg::OnSave() 
{
	if( DoSave(m_strFilename) )
		m_bChangesMade = false;
}

// ------------------------------------------------------------------------
// Event Handler: OnSaveAs
// ------------------------------------------------------------------------
void CModelEditDlg::OnSaveAs() 
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	// Look up the model directory in the registry
	CRegMgr regMgr;
	CString csOpenDir;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		regMgr.Get("Model", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		csOpenDir.ReleaseBuffer(-1);
	}
	// Strip the extension...
	char* pExt = _tcsrchr(csOpenDir,'.'); 
	if (pExt) { csOpenDir.TrimRight(pExt); }
	csOpenDir += "*.lta";

	// get a filename from the user
	CString sFilter;
	sFilter.LoadString(IDS_SAVEASFILTER);

	CFileDialog dlg (FALSE, "lta", (csOpenDir.GetLength()) ? (LPCTSTR)csOpenDir : NULL, OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, sFilter, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	
	// Store the model directory in the registry
	csOpenDir = dlg.GetPathName();
	csOpenDir = csOpenDir.Left(csOpenDir.GetLength() - dlg.GetFileName().GetLength());
	regMgr.Set("Model", (LPCTSTR)csOpenDir);
	
	if( DoSave( dlg.GetPathName() ) )
	{
		m_strFilename = dlg.GetPathName();
		CString titleText;
		titleText.FormatMessage( IDS_TITLE_TEXT, m_strFilename );
		SetWindowText( (LPCTSTR)titleText );
		m_pModel->SetFilename( m_strFilename );
		m_bChangesMade = false;
	}
}

// ------------------------------------------------------------------------
// Event Handler: OnCancel 
// ------------------------------------------------------------------------
void CModelEditDlg::OnCancel() 
{
	if (m_bChangesMade)
	{
		int nAnswer = MessageBox ("Save changes before exiting?", "Model Editor", MB_YESNOCANCEL | MB_ICONQUESTION);
		
		//determine if the user wanted to not exit
		if(nAnswer == IDCANCEL)
		{
			return;
		}

		//see if they wanted to save
		if (nAnswer == IDYES)
		{
			OnSave();
		}
	}

	m_bEndDialog = TRUE;
	CDialog::OnCancel();
}


void CModelEditDlg::OnStop() 
{
	uint32 i;
	MEAnimInfo *pInfo;
	DWORD dwActive[NUM_ANIM_INFOS];


	GetPlaybackActiveFlags(dwActive);

	// Apply the cm
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = GetAnimInfo(i);

		if(dwActive[i] & PA_ACTIVE)
		{
			GetAnimInfo(i)->StopPlayback();
		}
	}
}

void CModelEditDlg::OnPlayForward() 
{
	uint32 i;
	MEAnimInfo *pInfo;
	DWORD dwActive[NUM_ANIM_INFOS];


	GetPlaybackActiveFlags(dwActive);


	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = GetAnimInfo(i);

		if(dwActive[i])
		{
			GetAnimInfo(i)->StartPlayback();
		}
	}

	UpdateEditFrameString( );
	UpdateEditFrameTime( );
}

void CModelEditDlg::OnNextKeyframe() 
{
	IncKeyframe(TRUE);
}


void CModelEditDlg::OnPrevKeyframe() 
{
	IncKeyframe(FALSE);
}

int CModelEditDlg::GetSelectedParentAnims(int *pItems, int maxItems)
{
	int i, nRetItems;

	if (!m_pModel)
		return -1;

	nRetItems = 0;
	for (i=0; i < m_AnimList.GetItemCount(); i++)
	{
		if ((i > (int)m_pModel->m_Anims.GetSize()) || (nRetItems >= maxItems))
			break;
		if ((!m_pModel->GetAnimIfOwner(i)) || (m_AnimList.GetItemState(i, LVIS_SELECTED) == 0))
			continue;

		pItems[nRetItems] = i;
		nRetItems++;
	}
	
	return nRetItems;			
}


BOOL CModelEditDlg::HandleButtonDown(int iButton, CPoint point)
{
	m_RenderWnd.ClientToScreen(&point);
	ScreenToClient(&point);
	
	if (GetAsyncKeyState('C') & 0x8000)
	{
		StartTracker(iButton, point);
		return TRUE;
	}
	else if (GetAsyncKeyState('V') & 0x8000)
	{
		StartTracker(iButton+3, point);
		return TRUE;
	}

	return FALSE;
}


BOOL CModelEditDlg::HandleButtonUp(int iButton)
{
	if (m_bTracking)
	{
		SetWindowText(m_OldWindowText);
		ShowCursor(TRUE);
		ReleaseCapture();
		m_bTracking = FALSE;
		SetChangesMade();
		return TRUE;
	}

	return FALSE;
}


void CModelEditDlg::StartTracker(uint32 iTracker, CPoint centerPt)
{
	if (iTracker >= m_Trackers.GetSize())
		return;

	m_TotalDelta = 0;
	GetWindowText(m_OldWindowText);
	ShowCursor(FALSE);
	m_bTracking = TRUE;
	m_iCurTracker = iTracker;
	m_LastCursorPos = centerPt;
	m_TrackerCenterPt = centerPt;
	SetCapture();
}


// Moves selected animations up or down, depending on dir.
void CModelEditDlg::MoveAnims(int dir)
{
	int i, j, iTemp, nItems, items[100], nSelection, nDest;
	CString strSelection;
	AnimInfo temp;
	BOOL bChecked;
	
	if ((!m_pModel) || (!dir))
		return;
		
	nItems = GetSelectedParentAnims(items, 100);
	if (nItems == -1)
		return;

	// So we don't have animations pointing at bad data.
	ResetAnimInfos();

	// Process in reverse order for positive directions
	if (dir > 0)
	{
		for (i=0; i < (nItems / 2); i++)
		{
			iTemp = items[i];
			items[i] = items[nItems - (i + 1)];
			items[nItems - (i + 1)] = iTemp;
		}
	}

	for (i=0; i < nItems; i++)
	{
		nSelection = items[i];
		nDest = max(min(nSelection + dir, (int)m_pModel->m_Anims.GetSize() - 1), 0);

		if (nSelection == nDest)
			continue;

		// move the actual anim in the model
		temp = m_pModel->m_Anims[nSelection];
		if (nDest < nSelection)
		{
			for (j=nSelection; j > nDest; j--)
				m_pModel->m_Anims[j] = m_pModel->m_Anims[j - 1];
		}
		else
		{
			for (j=nSelection; j < nDest; j++)
				m_pModel->m_Anims[j] = m_pModel->m_Anims[j + 1];
		}
		m_pModel->m_Anims[nDest] = temp;

		// Move the item in the list
		strSelection = m_AnimList.GetItemText(nSelection, 0);
		bChecked = m_AnimList.GetCheck(nSelection);
		m_AnimList.DeleteItem(nSelection);
		m_AnimList.InsertItem(nDest, strSelection);
		m_AnimList.SetItemState(nDest, LVIS_SELECTED, LVIS_SELECTED);
		m_AnimList.SetCheck(nDest, bChecked);

		m_bChangesMade = TRUE;
	}
}

void CModelEditDlg::OnAnimMoveDown() 
{
	MoveAnims(1);
}

void CModelEditDlg::OnAnimMoveUp() 
{
	MoveAnims(-1);
}

void CModelEditDlg::OnAnimDelete() 
{
	int i, nItems, items[100], iTemp;

	if (!m_pModel) 
		return;

	if (!GetAnimList(items, sizeof(items), nItems))
		return;

	// Sort in descending order.
	for (i=0; i < (nItems-1); i++)
	{
		if (items[i] < items[i+1])
		{
			iTemp = items[i];
			items[i] = items[i+1];
			items[i+1] = iTemp;

			if (i == 0)
				i -= 1;
			else
				i -= 2;
		}
	}

	// Make sure we're not deleting them all.
	if (nItems == (int)m_pModel->m_Anims.GetSize())
	{
		MessageBox ("Cannot remove all animations from a model file.", "Delete Animation", MB_OK | MB_ICONINFORMATION);
		return;
	}

	// make sure they really want to delete it

	int nAnswer = MessageBox ("Are you sure you want to remove these animations from the model file?", "Delete Animation", MB_YESNO | MB_ICONQUESTION);
	if (nAnswer == IDNO)
	{
		return;
	}

	for (i=0; i < nItems; i++)
	{
		m_AnimList.DeleteItem(items[i]);
		// Delete the animation
		ModelAnim *pAnim = m_pModel->GetAnim(items[i]);
		if (pAnim)
			delete pAnim;
		// Remove it from the list
		m_pModel->m_Anims.Remove(items[i]);
		// Update the child models
		for (DWORD j=0; j < m_pModel->NumChildModels(); j++)
		{
			ChildInfo *pChild = m_pModel->GetChildModel(j);
			if ((DWORD)items[i] < pChild->m_AnimOffset)
				pChild->m_AnimOffset--;
		}
	}

	// Select the first item.
	m_AnimList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	for (i = 1; i < m_AnimList.GetItemCount(); i++)
		m_AnimList.SetItemState(i, 0, LVIS_SELECTED);

	ResetAnimInfos();
	
	SetChangesMade();
}



void CModelEditDlg::OnCreateSingleFrame() 
{
	DWORD nAnimation = m_AnimInfos[0].CurAnimIndex();
	DWORD nKeyframe = m_AnimInfos[0].CurKeyframe();
	DWORD i;
	char testName[100];
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	


	if (!m_pModel)
		return;


	StopAllPlayback();


	// get a name for the new model

	CRenameDlg dlg (this);

	// Get a default name.
	for (i=0; i < 1000; i++)
	{
		if (i == 0)
			sprintf(testName, "static_model");
		else
			sprintf(testName, "static_model%d", i);
		
		if (!m_pModel->FindAnim(testName))
		{
			dlg.m_strName = testName;
			break;
		}
	}

	if (dlg.DoModal() == IDCANCEL)
	{
		return;
	}


	// Make a copy of the current anim and strip out all keyframes but the one we want.
	ModelAnim *pAnim;
	ModelAnim *pCopyFrom = m_pModel->GetAnim(nAnimation);
	ChildInfo *pChildInfo = m_pModel->GetAnimInfo(nAnimation)->m_pChildInfo;

	pAnim = new ModelAnim(m_pModel);
	if (!pAnim)
		return;

	if(!pAnim->CopyAnim(pCopyFrom))
		return;
	
	AnimInfo* pNewInfo = m_pModel->AddAnim(pAnim, pChildInfo);

	if (!pNewInfo)
		return;
	
	// Set its name.
	pAnim->SetName((char*)(LPCTSTR)dlg.m_strName);

	//copy over the dimensions
	pNewInfo->m_vDims = m_pModel->GetAnimInfo(nAnimation)->m_vDims;

	// Remove the keyframes leading up.
	for (i=0; i < nKeyframe; i++)
	{
		pAnim->RemoveKeyFrame(0);
	}

	// Copy the first keyframe....

	ModelAnim *pFrame = pAnim->CopyKeyFrame(0);
	pAnim->PasteKeyFrame(pFrame, 1);

	// Remove the ones after.
	while(pAnim->NumKeyFrames() > 2)
	{
		pAnim->RemoveKeyFrame(pAnim->NumKeyFrames() - 1);
	}
	pAnim->m_KeyFrames[0].m_Time = 0;
	pAnim->m_KeyFrames[1].m_Time = 200;

	// add the new anim to the list box and select it
	InitTaggedArrays();

	FillAnimList();
	for (i = 0; i < (DWORD)m_AnimList.GetItemCount(); i++)
		m_AnimList.SetItemState((int)i, 0, LVIS_SELECTED);
	m_AnimList.SetItemState(m_pModel->m_Anims - 1, LVIS_SELECTED, LVIS_SELECTED);

	OnSelChangeAnimList();

	SetChangesMade();
}


void CModelEditDlg::DoRenameAnim(DWORD iAnim)
{
	ModelAnim *pAnim;
	char *pNewName;
	CString str;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	
	if (!m_pModel || 
		iAnim >= m_pModel->NumAnims() ||
		!m_pModel->GetAnimInfo(iAnim)->IsParentModel())
		return;										 

	pAnim = m_pModel->GetAnim(iAnim);

	// make sure the animation is stopped

	OnStop();

	// get the new name

	CRenameDlg dlg (this);
	
	while(1)
	{
		dlg.m_strName = pAnim->GetName();
		if (dlg.DoModal() == IDCANCEL)
		{
			return;
		}

		pNewName = (char*)(LPCTSTR)dlg.m_strName;
		if (m_pModel->FindAnim(pNewName))
		{
			str.FormatMessage(IDS_ENTER_UNIQUE_ANIM_NAME, pNewName);
			if (DoMessageBox(str, MB_YESNO) == IDNO)
			{
				return;
			}
		}
		else
		{
			break;
		}
	}

	// set the new name in the animation
	pAnim->SetName((char*)(LPCTSTR)dlg.m_strName);

	// set the new name in the listbox

	m_AnimList.SetItemText(iAnim, 0, pAnim->GetName());

	SetChangesMade();
}


void CModelEditDlg::OnRenameAnim() 
{
	DoRenameAnim(m_AnimInfos[0].CurAnimIndex());
}

void CModelEditDlg::OnContinuous() 
{
	CContinuousDlg dlg;
	ModelAnim *pCurAnim, *pFrame;
	DWORD nKeyframes, oldLastTime;
	DWORD iAnim;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	iAnim = m_AnimInfos[0].CurAnimIndex();
	if (!m_pModel || 
		iAnim >= m_pModel->NumAnims() ||
		!m_pModel->GetAnimInfo(iAnim)->IsParentModel())
		return;

	// make sure the animation is stopped
	StopAllPlayback();

	// get the delay
	if (dlg.DoModal() == IDCANCEL) 
		return;

	pCurAnim = m_pModel->GetAnim(iAnim);
	if (pCurAnim->m_KeyFrames == 0)
		return;

	nKeyframes = pCurAnim->NumKeyFrames();
	oldLastTime = pCurAnim->m_KeyFrames.Last().m_Time;

	// Copy and paste it..
	pFrame = pCurAnim->CopyKeyFrame(0);
	if (!pFrame)
		return;

	if (!pCurAnim->PasteKeyFrame(pFrame, pCurAnim->NumKeyFrames()))
	{
		delete pFrame;
		return;
	}

	delete pFrame;

	// Adjust its time.
	pCurAnim->m_KeyFrames.Last().m_Time = oldLastTime + dlg.m_nDelay;

	// now add an extra tagged array member
	InitTaggedArrays();

	// update the ui
	ResetAnimInfos();
	OnSelChangeAnimList();
	UpdateEditFrameTime();

	m_bChangesMade = TRUE;
}

void CModelEditDlg::OnDrawSkeleton() 
{
	m_RenderWnd.m_DrawStruct.m_bDrawSkeleton = !m_RenderWnd.m_DrawStruct.m_bDrawSkeleton;	
	UpdateMenuChecks();
}

void CModelEditDlg::OnWireframe() 
{
	 m_RenderWnd.m_DrawStruct.m_bWireframe = !m_RenderWnd.m_DrawStruct.m_bWireframe;
	UpdateMenuChecks();
}

void CModelEditDlg::OnCameraFollow() 
{
	  m_RenderWnd.m_bCameraFollow = !m_RenderWnd.m_bCameraFollow;
	UpdateMenuChecks();
}

void CModelEditDlg::OnDims() 
{
	 m_RenderWnd.m_DrawStruct.m_bDims = !m_RenderWnd.m_DrawStruct.m_bDims;
	UpdateMenuChecks();
}

void CModelEditDlg::OnAnimBox() 
{
	 m_RenderWnd.m_DrawStruct.m_bAnimBox = !m_RenderWnd.m_DrawStruct.m_bAnimBox;
	UpdateMenuChecks();
}


void CModelEditDlg::OnShowSockets() 
{
	 m_RenderWnd.m_DrawStruct.m_bDrawSockets = !m_RenderWnd.m_DrawStruct.m_bDrawSockets;
	UpdateMenuChecks();
}


void CModelEditDlg::OnShowTextures()
{
	 m_RenderWnd.m_DrawStruct.m_bDrawTextures = !m_RenderWnd.m_DrawStruct.m_bDrawTextures;
	UpdateMenuChecks();
}


void CModelEditDlg::OnShowAttachments() 
{
	 m_RenderWnd.m_DrawStruct.m_bDrawAttachments = !m_RenderWnd.m_DrawStruct.m_bDrawAttachments;
	UpdateMenuChecks();
}


void CModelEditDlg::OnSolidAttachments() 
{
	 m_RenderWnd.m_DrawStruct.m_bSolidAttachments = !m_RenderWnd.m_DrawStruct.m_bSolidAttachments;
	UpdateMenuChecks();
}


void CModelEditDlg::OnShowNormals()
{
	m_RenderWnd.m_DrawStruct.m_bNormals = !m_RenderWnd.m_DrawStruct.m_bNormals;
	UpdateMenuChecks();
}


void CModelEditDlg::OnShowNormalRef()
{
	 m_RenderWnd.m_DrawStruct.m_bShowNormalRef = !m_RenderWnd.m_DrawStruct.m_bShowNormalRef;
	UpdateMenuChecks();
}


void CModelEditDlg::OnProfile()
{
	 m_RenderWnd.m_DrawStruct.m_bProfile = !m_RenderWnd.m_DrawStruct.m_bProfile;
	UpdateMenuChecks();
}


void CModelEditDlg::OnMovementEncoding()
{
	CButton* checkBox = (CButton*)GetDlgItem( IDC_MOVEMENT_ENCODING );
	 m_RenderWnd.m_DrawStruct.m_bMovementEncoding = checkBox->GetCheck() != 0;
}


void CModelEditDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CScrollBar *pItem;
	int pos;
	DWORD newLOD;

	uint32 max_dist = 1000 ;

	if (!m_pModel)
		return;

	if (nSBCode != SB_ENDSCROLL)
	{
		pos = pScrollBar->GetScrollPos();
		
		pItem = (CScrollBar*)GetDlgItem(IDC_LOD_SCROLL);
		if (pItem && (pScrollBar==pItem) && m_pModel)
		{
			if (nSBCode == SB_PAGELEFT || nSBCode == SB_LINELEFT)
			{
				pos = CLAMP(pScrollBar->GetScrollPos() - 1, 0, (int)(max_dist - 1));
			}
			else if (nSBCode == SB_PAGERIGHT || nSBCode == SB_LINERIGHT)
			{
				pos = CLAMP(pScrollBar->GetScrollPos() + 1, 0, (int)(max_dist - 1));
			}
			else
			{
				pos = nPos;
			}

			pScrollBar->SetScrollPos(pos);

			newLOD = pos;
			//if (newLOD != m_nCurrentLOD)
			{
				m_LODSlider.SetCurrentValue((float)newLOD);
				SetCurrentLOD((float)newLOD);
				m_RenderWnd.Draw();
			}
		}
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CModelEditDlg::SetLODText(int nLOD)
{
	char str[256];
	CWnd *pWnd;

	sprintf(str, "LOD: %d", nLOD);
	if (pWnd = GetDlgItem(IDC_LOD_TEXT))
		pWnd->SetWindowText(str);
}


void SetGNModelText(char *pText)
{
}


void CModelEditDlg::OnPieceRemoveLODs()
{
	unsigned numNotRemoved = 0;

	vector<PieceLODInfo> selection;
	int numSelected = GetSelectedPieceLODs( &m_PieceList, m_pModel, selection );

	if( !numSelected || (DoMessageBox( "Are you sure you wish to delete the selected LODs?", MB_OKCANCEL ) != IDOK) )
		return;

	// go over list back to front to make sure we never remove the highest lod
	for( int i = selection.size() - 1; i >= 0; i-- )
	{
		// don't remove any LODs from a piece that has only one LOD left
		if( selection[i].m_ModelPiece->NumLODs() <= 1 )
		{
			numNotRemoved++;
			continue;
		}

		bool removeSuccess = selection[i].m_ModelPiece->RemoveLOD( selection[i].m_LODNum );
		ASSERT( removeSuccess );
	}

	if( numNotRemoved > 0 )
	{
		CString notRemovedStr;
		notRemovedStr.Format( "%d LODs not removed as they are the only LODs in a piece.", numNotRemoved );
		DoMessageBox( notRemovedStr, MB_OK );
	}

	// reload the piece tree
	FillPieceList( &m_PieceList, m_pModel);

	// adjust the LOD slider
	float maxLODDist = GetModelMaxLODDist( m_pModel );
	InitTheScrollBar( maxLODDist );
	SetCurrentLOD( 0.0f );

	SetChangesMade();
}


void CModelEditDlg::OnPieceBuildLODs() 
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	CPieceLODGenDlg dlg;
	CRegMgr regMgr;

	// default values
	CString distance = "100.0";
	CString maxEdgeLen = "1000.0";
	CString percent = "50.0";
	unsigned minNumTris = 8;

	// get the current piece lod selections
	vector<PieceLODInfo> selection;
	int numSelected = GetSelectedPieceLODs( &m_PieceList, m_pModel, selection, true );
	if( !numSelected )
	{
		MessageBox( "At least one piece LOD needs to be selected.", MB_OK );
		return;
	}

	// get the lod settings from the registry
	if( regMgr.Init( szRegKeyCompany, szRegKeyApp, szRegKeyVer, "LODGeneration", HKEY_CURRENT_USER ) )
	{
		unsigned bufSize = 256;
		regMgr.Get( "Distance", distance.GetBufferSetLength( bufSize ), bufSize );
		distance.ReleaseBuffer();
		dlg.m_Distance = (float)atof( distance );

		bufSize = 256;
		regMgr.Get( "MaxEdgeLen", maxEdgeLen.GetBufferSetLength( bufSize ), bufSize );
		maxEdgeLen.ReleaseBuffer();
		dlg.m_MaxEdgeLen = (float)atof( maxEdgeLen );

		bufSize = 256;
		regMgr.Get( "Percent", percent.GetBufferSetLength( bufSize ), bufSize );
		percent.ReleaseBuffer();
		dlg.m_Percent = (float)atof( percent );

		dlg.m_MinNumTris = regMgr.Get( "MinNumTris", minNumTris );
	}

	if( dlg.DoModal() != IDOK )
		return;

	// store the lod settings in the registry
	if( regMgr.Init( szRegKeyCompany, szRegKeyApp, szRegKeyVer, "LODGeneration", HKEY_CURRENT_USER ) )
	{
		distance.Format( "%f", dlg.m_Distance );
		regMgr.Set( "Distance", distance );

		maxEdgeLen.Format( "%f", dlg.m_MaxEdgeLen );
		regMgr.Set( "MaxEdgeLen", maxEdgeLen );

		percent.Format( "%f", dlg.m_Percent );
		regMgr.Set( "Percent", percent );

		regMgr.Set( "MinNumTris", dlg.m_MinNumTris );
	}

	// initialize the piece LOD builder
	int numBadLODs = 0;
	int numBadLODsVA = 0;

	BeginWaitCursor();

	// loop through each selected source piece LOD
	for( int i = 0; i < selection.size(); i++ )
	{
		// can't generate LODs for vertex animated pieces
		if( selection[i].m_ModelPiece->m_isVA )
		{
			numBadLODs++;
			numBadLODsVA++;
			continue;
		}

		CBuildPieceLOD* buildLOD = new CBuildPieceLOD;

		buildLOD->m_Distance = dlg.m_Distance;
		buildLOD->m_MaxEdgeLen = dlg.m_MaxEdgeLen;
		buildLOD->m_Percent = dlg.m_Percent;
		buildLOD->m_MinNumTris = dlg.m_MinNumTris;

		buildLOD->m_Model = m_pModel;
		buildLOD->m_ModelPiece = selection[i].m_ModelPiece;
		buildLOD->m_LODNum = selection[i].m_LODNum;
		// the lod pointer may be bogus after adding LODs to a piece, so get a fresh one
		buildLOD->m_PieceLOD = buildLOD->m_ModelPiece->GetLOD( (unsigned)selection[i].m_LODNum );

		// generate new piece LOD from currend piece LOD
		if( !buildLOD->BuildLOD() )
			numBadLODs++;

		delete buildLOD;
	}

	EndWaitCursor();

	// sort the piece lods
	for( i = 0; i < m_pModel->NumPieces(); i++ )
	{
		// get the current piece
		ModelPiece* curPiece = m_pModel->GetPiece( i );
		if( !curPiece )
		{
			ASSERT(0);
			continue;
		}
		curPiece->SortLODs();
	}

	// reload the piece tree
	FillPieceList( &m_PieceList, m_pModel );

	// adjust the LOD slider
	float maxLODDist = GetModelMaxLODDist( m_pModel );
	InitTheScrollBar( maxLODDist );
	SetCurrentLOD( 0.0f );

	SetChangesMade();

	// inform the user of errors
	if( numBadLODs || numBadLODsVA )
	{
		CString errMessage;
		errMessage.Format( "%d LODs didn't generate correctly.\n%d of these failed due to being vertex animated.", numBadLODs, numBadLODsVA );
		DoMessageBox( errMessage, MB_OK | MB_ICONERROR );
	}
}


void CModelEditDlg::OnPieceMerge( void )
{
	// get the selected pieces
	vector<ModelPiece*> selection;
	int numSelected = GetSelectedPieces( &m_PieceList, m_pModel, selection );
	int numNewPieces = 0;

	BeginWaitCursor();

	// merge the pieces if possible
	CPieceMerge mergePieces( m_pModel, selection );
	numNewPieces = mergePieces.Merge();

	EndWaitCursor();

	// set the dirty bit if anything actually happened
	ASSERT( numNewPieces <= numSelected );
	if( numNewPieces != numSelected )
		SetChangesMade();

	// reload the piece tree
	FillPieceList( &m_PieceList, m_pModel );
	m_RenderWnd.AllocSelections( m_pModel->NumPieces(), m_pModel->NumNodes());

	// tell the user what happened
	CString completedMessage;
	completedMessage.Format( "Merged %d pieces into %d pieces.", numSelected, numNewPieces );
	DoMessageBox( completedMessage, MB_OK );
}


void CModelEditDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CModelEditDlg::OnModelInfo()
{
	CModelInfoDlg dlg;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	if (m_pModel)
	{
		dlg.m_pModel = m_pModel;
		dlg.DoModal();
	}
}



void CModelEditDlg::OnCommandString() 
{
	CCommandStringDlg dlg;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	if (m_pModel)
	{
		dlg.m_pModel = m_pModel;
		dlg.DoModal();
		ProcessCommandString();
		SetChangesMade();
	}
}

void CModelEditDlg::OnDimensions() 
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	if (!m_pModel)
		return;

	if(m_pDimensionDlg == NULL)
	{
		m_pDimensionDlg = new CDimensionsDlg( this );
		
		//make sure the memory was allocated properly
		if(m_pDimensionDlg == NULL)
		{
			return;
		}

		//load the list of animations into the dialog
		if (!GetAnimList(	m_pDimensionDlg->m_Anims, 
							sizeof(m_pDimensionDlg->m_Anims), 
							m_pDimensionDlg->m_nAnims, FALSE))
		{
			return;		
		}

		m_pDimensionDlg->m_pModel = m_pModel;

		//create the dialog
		m_pDimensionDlg->Create(CDimensionsDlg::IDD,this);
		
		m_pDimensionDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		//load the list of animations into the dialog
		if (!GetAnimList(	m_pDimensionDlg->m_Anims, 
							sizeof(m_pDimensionDlg->m_Anims), 
							m_pDimensionDlg->m_nAnims, FALSE))
		{
			return;		
		}
		m_pDimensionDlg->ShowWindow(SW_SHOW);
	}
}

//this will run through each animation in the model, looking for animations that have the X and Z
//dimensions of the animation not equal. It will then prompt them for how to fix.
void CModelEditDlg::CheckValidAnimationDims()
{
	//we need a model
	if(!m_pModel)
		return;

	//now run through all animations and for each, make sure that they match
	for(uint32 nCurrAnim = 0; nCurrAnim < m_pModel->m_Anims.GetSize(); nCurrAnim++)
	{
		AnimInfo* pCurrAnim = &m_pModel->m_Anims[nCurrAnim];

		if(pCurrAnim->IsParentModel() && fabs(pCurrAnim->m_vDims.x - pCurrAnim->m_vDims.z) > 0.1f)
		{
			//we have a descrepincy here, have the user resolve it
			CInvalidAnimDimsDlg Dlg(this);
			Dlg.m_pAnimInfo = pCurrAnim;
			if(Dlg.DoModal() == IDOK)
			{
				//this should set a modified flag some how
			}

		}
	}
}


void CModelEditDlg::OnAnimationFramerate() 
{
	CAnimFramerateDlg dlg;
	char str[20];
	DWORD i, iAnim;
	float theRate;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	if (m_pModel)
	{
		sprintf(str, "%d", 10);
		dlg.m_FramerateString = str;
		dlg.m_bAllAnimations = FALSE;
		
		if (dlg.DoModal() == IDOK)
		{
			// Grab the currently playing animation index
			iAnim = m_AnimInfos[0].CurAnimIndex();

			// grab the tagged keyframes
			CMoArray<BOOL>& taggedKeysOrig = m_AnimInfos[0].m_Wnd.m_Tagged;
			vector<bool> taggedKeys( taggedKeysOrig );
			for( i = 0; i < taggedKeysOrig; i++ )
			{
				taggedKeys[i] = !!(taggedKeysOrig[i]);
			}

			// Stop playing
			ResetAnimInfos();

			theRate = (float)atof(dlg.m_FramerateString);
			
			if (dlg.m_bAllAnimations)
			{
				for (i=0; i < m_pModel->m_Anims; i++)
				{
					model_SetAnimFramerate(m_pModel, i, (float)(1.0f / theRate), NULL);
				}
			}
			else
			{
				if (iAnim < m_pModel->m_Anims && 
					m_pModel->GetAnimInfo(iAnim)->IsParentModel())
				{
					model_SetAnimFramerate(m_pModel, iAnim, (float)(1.0f / theRate), &taggedKeys);
				}
			}
		
			SetChangesMade();

			// Update the animation list
			OnSelChangeAnimList();
		}
	}
}

void CModelEditDlg::OnAnimationLength() 
{
	CKeyframeTimeDlg dlg;
	DWORD i, nKeyframes, dwNewKeyframeTime, iAnim;
	float fPosition;
	ModelAnim *pAnim;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	
	iAnim = m_AnimInfos[0].CurAnimIndex();	
	if(!m_pModel ||
		iAnim >= m_pModel->NumAnims() ||
		!m_pModel->GetAnimInfo(iAnim)->IsParentModel())
	{
		return;
	}

	pAnim = m_pModel->GetAnim(iAnim);
	nKeyframes = pAnim->m_KeyFrames.GetSize();
	dlg.m_nCurrentTime = pAnim->GetAnimTime();
	dlg.m_nNewTime = dlg.m_nCurrentTime;
	dlg.m_sCaption = "Animation Length";
	
	if (dlg.DoModal() == IDCANCEL || dlg.m_nNewTime == dlg.m_nCurrentTime)
	{
		return;
	}

	// Do all the frames except first and last...
	for ( i = 1; i < nKeyframes - 1; i++ )
	{
		fPosition = ( float )pAnim->m_KeyFrames[i].m_Time / ( float )pAnim->GetAnimTime();
		dwNewKeyframeTime = ( DWORD )((( float )dlg.m_nNewTime * fPosition ) + 0.5f );
		pAnim->m_KeyFrames[i].m_Time = dwNewKeyframeTime;
	}

	// Make sure the last frame is exactly the full time...
	pAnim->m_KeyFrames[nKeyframes - 1].m_Time = dlg.m_nNewTime;
	
	SetChangesMade();
}

void CModelEditDlg::OnAnimationInterpolation() 
{
	CKeyframeTimeDlg dlg;
	DWORD iAnim;
	ModelAnim *pAnim;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	
	iAnim = m_AnimInfos[0].CurAnimIndex();	
	if(!m_pModel ||
		iAnim >= m_pModel->NumAnims() ||
		!m_pModel->GetAnimInfo(iAnim)->IsParentModel())
	{
		return;
	}

	pAnim = m_pModel->GetAnim(iAnim);

	dlg.m_nCurrentTime = pAnim->m_InterpolationMS;
	dlg.m_nNewTime = pAnim->m_InterpolationMS;
	dlg.m_sCaption = "Animation Interpolation";
	
	if (dlg.DoModal() == IDCANCEL || dlg.m_nNewTime == dlg.m_nCurrentTime)
	{
		return;
	}

	pAnim->m_InterpolationMS = dlg.m_nNewTime;

	SetChangesMade();
}

void CModelEditDlg::UpdateEditFrameTime( )
{
	MEAnimInfo *pInfo;
	
	UpdateEditFrameTimeEnabled();

	if(!m_pModel)
	{
		return;
	}

	// The edit frame string can only be changed from anim 0.
	pInfo = GetAnimInfo(ANIMINFO_MAIN);
	
	TempVarSetter<BOOL> setter(&m_bInStringChange, TRUE);
	SetDlgItemInt( IDC_FRAMETIME, pInfo->CurAnimTime(), FALSE );
}

void CModelEditDlg::UpdateEditFrameTimeEnabled()
{
	bool bEnabled = true;

	if( !m_pModel ||
		m_AnimInfos[ANIMINFO_MAIN].IsAnimPlaying() ||
		(m_SelectedAnims.GetSize() != 1) ||
		(m_pModel->GetAnim(m_SelectedAnims[0])->GetModel() != m_pModel))
	{
		bEnabled = false;
	}

	m_EditFrameTime.EnableWindow( bEnabled );
}

void CModelEditDlg::UpdateEditFrameStringEnabled()
{
	bool bEnabled = true;

	if( !m_pModel ||
		m_AnimInfos[ANIMINFO_MAIN].IsAnimPlaying() ||
		(m_SelectedAnims.GetSize() != 1) ||
		(m_pModel->GetAnim(m_SelectedAnims[0])->GetModel() != m_pModel))
	{
		bEnabled = false;
	}

	m_EditFrameString.EnableWindow( bEnabled );
}

void CModelEditDlg::UpdateEditFrameString( )
{
	DWORD iAnim, iKeyframe;
	char *pStr;

	UpdateEditFrameStringEnabled();

	if(!GetEditAnimInfo(&iAnim, &iKeyframe, true))
		return;

	pStr = m_pModel->GetAnim(iAnim)->m_KeyFrames[iKeyframe].m_pString;
	
	TempVarSetter<BOOL> setter(&m_bInStringChange, TRUE);
	SetDlgItemText( IDC_FRAMESTRING, pStr );
}

void CModelEditDlg::SetEditFrameTime() 
{
	DWORD dwNewKeyframeTime = GetDlgItemInt( IDC_FRAMETIME, NULL, FALSE );
	DWORD nKeyframe = m_AnimInfos[ANIMINFO_MAIN].m_Wnd.ForceNearestKeyframe();
	m_AnimInfos[ANIMINFO_MAIN].m_Wnd.DoEditKeyframeTime( nKeyframe, dwNewKeyframeTime );
	UpdateEditFrameTime( );
}


void CModelEditDlg::SetEditFrameString()
{
	CString sFrameString;
	DWORD nKeyframe;

	GetDlgItemText( IDC_FRAMESTRING, sFrameString );
	nKeyframe = m_AnimInfos[ANIMINFO_MAIN].m_Wnd.ForceNearestKeyframe();
	m_AnimInfos[ANIMINFO_MAIN].m_Wnd.DoEditKeyframeString( nKeyframe, sFrameString );
}


void CModelEditDlg::SetCurrentAnimTime( uint32 iAnimInfo, DWORD dwAnimTime )
{
	Model *pModel;
	MEAnimInfo *pInfo;

	if(iAnimInfo >= NUM_ANIM_INFOS)
		return;

	pInfo = &m_AnimInfos[iAnimInfo];
	pModel = pInfo->GetModel();
	if(!pModel)
		return;

	trk_SetCurTime(&m_AnimInfos[iAnimInfo].m_Tracker, dwAnimTime, FALSE);
	UpdateEditFrameTime( );
}


BOOL CModelEditDlg::SetRenderTexture(const char *pFilename, DWORD nTexture)
{
	DStream *pStream;
	TextureData *pTexture;
	BOOL bRet;
	DRESULT dResult;


	bRet = FALSE;
	pStream = streamsim_Open(pFilename, "rb");
	if (pStream)
	{
		dResult = dtx_Create(pStream, &pTexture, TRUE);
		if (dResult == LT_OK)
		{
			bRet = m_RenderWnd.SetTexture( pTexture , nTexture );
			//bRet = SetGLMTexture(m_RenderWnd.GetContext(), pTexture, nTexture);
			if (bRet)
			{
				//m_RenderWnd.m_DrawStruct.m_bDrawBright = TRUE;
			}

			dtx_Destroy(pTexture);

			// if we've sucessfuly gotten the picture we keep its name/index values.
			m_pModel->AddTextureName( nTexture, pFilename );
		}

		pStream->Release();
	}




	return bRet;
}


BOOL CModelEditDlg::CheckModifyAnims(int *pAnims, int nAnims, BOOL bParentOnly, BOOL bMessage)
{
	int i, iAnim;
	CString str;
	AnimInfo *pAnimInfo;

	if (!m_pModel)
		return FALSE;

	for (i=0; i < nAnims; i++)
	{
		iAnim = pAnims[i];

		if (iAnim < 0 || iAnim >= (int)m_pModel->NumAnims())
		{
			if (bMessage)
			{
				str.FormatMessage(IDS_INVALIDANIMINDEX);
				DoMessageBox(str, MB_OK);
			}

			return FALSE;
		}

		if (bParentOnly)
		{
			pAnimInfo = m_pModel->GetAnimInfo(iAnim);
			if (pAnimInfo->GetAnimOwner() != m_pModel)
			{
				if (bMessage)
				{
					str.FormatMessage(IDS_CANTEDITBASEMODEL, pAnimInfo->m_pAnim->GetName());
					DoMessageBox(str, MB_OK);
				}

				return FALSE;
			}
		}
	}

	return TRUE;
}


BOOL CModelEditDlg::GetAnimList(int *pAnims, int listSizeBytes, int &nAnims, BOOL bParentOnly, BOOL bMessage)
{
	if ((!m_pModel) || (!pAnims))
		return FALSE;

	nAnims = 0;
	int listSize = listSizeBytes / sizeof(int);
	for (int i = 0; (i < m_AnimList.GetItemCount()) && (nAnims < listSize); i++)
	{
		if (m_AnimList.GetItemState(i, LVIS_SELECTED) != 0)
		{
			pAnims[nAnims] = i;
			nAnims++;
		}
	}
	if (nAnims == 0)
		return FALSE;

	return CheckModifyAnims(pAnims, nAnims, bParentOnly, bMessage);
}


void CModelEditDlg::ProcessCommandString()
{
	ConParse parse;
	ModelLoadRequest request;
	ModelSocket *pSocket;
	DWORD nTexture = 0;
	DDWORD nToken = 1;

	if (!m_pModel)
		return;
	
	// Set its skins.
	/*** DEPRECATED T.F 
	parse.Init(m_pModel->m_CommandString);
	while (parse.ParseFind("ModelEditTexture", FALSE, nToken))
	{
		if (parse.m_nArgs > 2)
			nTexture = atoi(parse.m_Args[2]);
		else
			nTexture = 0;
		SetRenderTexture(parse.m_Args[1], nTexture);
		nToken++;
	}
	***/ 

	// Setup socket attachment models.
	DeleteSocketAttachments();

	CRegMgr regMgr;
	regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Attachments", HKEY_CURRENT_USER);

	parse.Init(m_pModel->m_CommandString);
	while(parse.ParseFind("SocketModel", FALSE, 2))
	{
		if (pSocket = m_pModel->FindSocket(parse.m_Args[1]))
		{
			char *filename = parse.m_Args[2];
			
			if( IsFilenameLTAExt( filename ) )
			{

					Model *pModel = load_LTA_Using_FeedBackDialog( filename, false  );
					if( pModel != NULL )
					{
						pSocket->m_pAttachment = pModel ;
						// Save this attachment in the registry
						regMgr.Set(pSocket->GetName(), parse.m_Args[2]);
					}
			}
			else
			{
				MessageBox("ProcessCommandString error, trying to load bin abc file", "ERROR",MB_OK);
			
			}
		}
	}
}


void CModelEditDlg::OffsetTrans(DVector vec)
{
	int i;

	if (!m_pModel)
		return;

	for (i=0; (i < m_AnimList.GetItemCount()) && (i < (int)m_pModel->m_Anims.GetSize()); i++)
	{
		if (m_AnimList.GetItemState(i, LVIS_SELECTED) == 0)
			continue;
		m_pModel->GetAnimInfo(i)->m_vTranslation += vec;
	}

	SetChangesMade();
}


void CModelEditDlg::OffsetDims(DVector vec)
{
	int i;

	for (i=0; (i < m_AnimList.GetItemCount()) && (i < (int)m_pModel->m_Anims.GetSize()); i++)
	{
		if (m_AnimList.GetItemState(i, LVIS_SELECTED) == 0)
			continue;
		m_pModel->GetAnimInfo(i)->m_vDims += vec;
	}

	if( m_pDimensionDlg )
		m_pDimensionDlg->UpdateControls();

	SetChangesMade();
}


void CModelEditDlg::DoAnimEdit(DVector vec)
{
	if (IsButtonSelected(IDC_DIMS_EDIT))
		OffsetDims(vec);
	else
		OffsetTrans(vec);
}


void CModelEditDlg::OnTranslationButton() 
{
	TranslationDlg dlg;
	char tempStr[32];
	ModelAnim *pAnim;
	AnimInfo *pAnimInfo;
	NodeKeyFrame *pKeyframe;
	DVector newTrans, offset;
	int items[100], i, iAnim, nItems;
	DWORD iEditAnim;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	


	if(!GetEditAnimInfo(&iEditAnim, NULL, false))
		return;

	if (!GetAnimList(items, sizeof(items), nItems, FALSE))
		return;
		
	pAnim = m_pModel->GetAnim(iEditAnim);
	pAnimInfo = m_pModel->GetAnimInfo(iEditAnim);

	pKeyframe = &pAnim->GetRootNode()->m_KeyFrames[0];
	
	sprintf(tempStr, "%.3f", pAnimInfo->m_vTranslation.x);
	dlg.m_xTrans = tempStr;

	sprintf(tempStr, "%.3f", pAnimInfo->m_vTranslation.y);
	dlg.m_yTrans = tempStr;

	sprintf(tempStr, "%.3f", pAnimInfo->m_vTranslation.z);
	dlg.m_zTrans = tempStr;

	if (dlg.DoModal() == IDOK)
	{
		newTrans.x = (float)atof(dlg.m_xTrans);
		newTrans.y = (float)atof(dlg.m_yTrans);
		newTrans.z = (float)atof(dlg.m_zTrans);

		for (i=0; i < nItems; i++)
		{
			iAnim = items[i];

			pAnimInfo = m_pModel->GetAnimInfo(iAnim);
			pAnimInfo->m_vTranslation = newTrans;
		}
	}
}


void CModelEditDlg::OnRotationButton() 
{
	RotationDlg dlg;
	DVector vRot;
	int items[100], i, iAnim, nItems;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	
	if (!m_pModel)
		return;

	if (!GetAnimList(items, sizeof(items), nItems))
		return;

	dlg.m_sRotX = dlg.m_sRotY = dlg.m_sRotZ = "0.0";

	if (dlg.DoModal() == IDOK)
	{
		vRot.x = MATH_DEGREES_TO_RADIANS(( float )atof( dlg.m_sRotX ));
		vRot.y = MATH_DEGREES_TO_RADIANS(( float )atof( dlg.m_sRotY ));
		vRot.z = MATH_DEGREES_TO_RADIANS(( float )atof( dlg.m_sRotZ ));

		for (i=0; i < nItems; i++)
		{
			iAnim = items[i];

			model_RotateAnim(m_pModel, iAnim, &vRot);
		}

		m_bChangesMade = TRUE;
	}
}


void CModelEditDlg::OnXsub() 
{
	DoAnimEdit(DVector(-1.0f, 0.0f, 0.0f));
}

void CModelEditDlg::OnXadd() 
{
	DoAnimEdit(DVector(1.0f, 0.0f, 0.0f));
}

void CModelEditDlg::OnYsub() 
{
	DoAnimEdit(DVector(0.0f, -1.0f, 0.0f));
}

void CModelEditDlg::OnYadd() 
{
	DoAnimEdit(DVector(0.0f, 1.0f, 0.0f));
}

void CModelEditDlg::OnZsub() 
{
	DoAnimEdit(DVector(0.0f, 0.0f, -1.0f));
}

void CModelEditDlg::OnZadd() 
{
	DoAnimEdit(DVector(0.0f, 0.0f, 1.0f));
}

void CModelEditDlg::OnNumberAnim() 
{
	int i, nItems, items[100], iTemp, nSelection;
	DWORD insertAt, nTempAnims, nListItems;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	


	if (!m_pModel)
		return;

	if (!GetAnimList(items, sizeof(items), nItems))
		return;
	
	AnimNumberDlg dlg(this, m_pModel->m_Anims - nItems);

	if (dlg.DoModal() == IDOK)
	{	
		// Sort in descending order.
		for (i=0; i < (nItems-1); i++)
		{
			if (items[i] < items[i+1])
			{
				iTemp = items[i];
				items[i] = items[i+1];
				items[i+1] = iTemp;

				if (i == 0)
					i -= 1;
				else
					i -= 2;
			}
		}

		// Remove them all from the listbox.
		for (i=0; i < nItems; i++)
		{
			nSelection = items[i];

			if (nSelection < 0 || nSelection >= (int)m_pModel->m_Anims.GetSize())
				continue;

			m_AnimList.DeleteItem(nSelection);
		}
	
		for (i=0; i < m_AnimList.GetItemCount(); i++)
			m_AnimList.SetItemState(i, 0, LVIS_SELECTED);

		// Add them back in order.
		nListItems = m_pModel->m_Anims - nItems;
		insertAt = MIN(nListItems, (DWORD)dlg.m_MoveNumber);
		for (i=0; i < nItems; i++)
		{
			m_AnimList.InsertItem(insertAt, m_pModel->GetAnim(items[i])->GetName());
			m_AnimList.SetItemState(insertAt, LVIS_SELECTED, LVIS_SELECTED);
		}

		
		// Now rearrange the model's list.
		CMoArray<AnimInfo, NoCache> tempAnims;
		tempAnims.CopyArray(m_pModel->m_Anims);
		nTempAnims = m_pModel->m_Anims;

		for (i=0; i < nItems; i++)
		{
			m_pModel->m_Anims.Remove(items[i]);
		}

		insertAt = MIN(m_pModel->m_Anims.GetSize(), (DWORD)dlg.m_MoveNumber);
		for (i=0; i < nItems; i++)
		{
			m_pModel->m_Anims.Insert(insertAt, tempAnims[items[i]]);
		}
		

		// process the change
		StopAllPlayback();
		m_AnimInfos[ANIMINFO_MAIN].m_Tracker.m_TimeRef.Init(
			m_pModel,
			dlg.m_MoveNumber, 0,
			dlg.m_MoveNumber, 0);

		SetChangesMade();
	}
}


void CModelEditDlg::OnDuplicateAnim() 
{
	int nItems;
	DWORD i;
	ModelAnim *pNewAnim;
	char newName[300];
	AnimInfo *pSrcAnimInfo, *pAnimInfo;
	DVector srcDims;
	DVector srcTranslation;

	nItems = m_AnimList.GetItemCount();
	if (!nItems)
		return;

	// allocate a vector that will store the animation indices of duplicated animations
	std::vector<int> duplicatedIndices;
	duplicatedIndices.reserve( nItems );

	// the number of animations that originally existed
	int numOriginalAnimations = m_pModel->CalcNumParentAnims();

	// Add the duplicated ones.
	for (i=0; i < (DWORD)nItems; i++)
	{
		if (m_AnimList.GetItemState(i, LVIS_SELECTED) == 0)
			continue;

		pSrcAnimInfo = m_pModel->GetAnimInfo(i);
	
		// if the file version between the child and parent are different quit operation.
		// right now, if file versions are different ModelEdit will crash.
		if( m_pModel->GetFileVersion() != pSrcAnimInfo->m_pAnim->GetModel()->GetFileVersion() ) 
		{
			int iChildFileVersion       = pSrcAnimInfo->m_pAnim->GetModel()->GetFileVersion();
			const char *childFileName   = pSrcAnimInfo->m_pChildInfo->m_pFilename ;
			const char *parentName      = m_pModel->GetFilename() ;

			char msg[2048];
			sprintf(msg, "%s\nParent %s version : %d\nChild (%s) version : %d\n",
					"Cannot Copy Animations from Child Models of Different Version",
					parentName, m_pModel->GetFileVersion(),		
					childFileName,iChildFileVersion);		

			MessageBox(msg, "Anim Copy Error", MB_OK|MB_ICONEXCLAMATION);
			return ;
		}
		
		srcDims = pSrcAnimInfo->m_vDims;
		srcTranslation = pSrcAnimInfo->m_vTranslation;

		sprintf(newName, "%s_copy", pSrcAnimInfo->m_pAnim->GetName());

		pNewAnim = new ModelAnim(m_pModel);
		if (!pNewAnim)
			return;

		if (!pNewAnim->CopyAnim(m_pModel->GetAnim(i)) || 
			!(pAnimInfo = m_pModel->AddAnimToSelf(pNewAnim)))
		{
			delete pNewAnim;
			return;
		}

		pAnimInfo->m_vDims = srcDims;
		pAnimInfo->m_vTranslation = srcTranslation;

		pNewAnim->SetName(newName);

		duplicatedIndices.push_back( i );
	}

	// move the duplicated animations immediately below their original
	int curIndex = 0;
	for( std::vector<int>::iterator it = duplicatedIndices.begin(); it != duplicatedIndices.end(); it++ )
	{
		// stop moving animations if they are in child models
		if( *it >= numOriginalAnimations )
			break;

		AnimInfo curAnimInfo = m_pModel->m_Anims[numOriginalAnimations + curIndex];

		for( i = numOriginalAnimations + curIndex; i > (*it + curIndex); i-- )
		{
			m_pModel->m_Anims[i] = m_pModel->m_Anims[i-1];
		}

		m_pModel->m_Anims[*it+1+curIndex] = curAnimInfo;

		curIndex++;
	}

	InitTaggedArrays();
	FillAnimList();
	SetChangesMade();
}

// ------------------------------------------------------------------------
// FillAnimDataFromTransformNodeData()
// basically take the bind pose and put it in the animation hierarchy.
// ------------------------------------------------------------------------
static void FillAnimDataFromTransformNodeData( AnimNode *pAnimNode )
{
	ModelNode *pNode = pAnimNode->m_pNode ;
	LTMatrix mat ;
	LTVector pos;
	LTRotation rot ;
	
	mat = pNode->GetLocalTransform();

	// get components
	mat.GetTranslation(pos);
	quat_ConvertFromMatrix( rot.m_Quat, mat.m );
	
	// pass it to anim data.
	for( int kfcnt = 0 ; kfcnt < pAnimNode->m_KeyFrames.GetSize() ;kfcnt++) 
	{
		pAnimNode->m_KeyFrames[kfcnt].m_Translation = pos ;
		pAnimNode->m_KeyFrames[kfcnt].m_Quaternion  = rot ;	
	}

	// recurse through the heirarchy.
	for( int childcnt = 0 ; childcnt < pAnimNode->m_Children.GetSize() ; childcnt++)
	{
		FillAnimDataFromTransformNodeData( pAnimNode->m_Children[childcnt]);
	}
}


// ------------------------------------------------------------------------
// create an animation from the bind pose 
// ------------------------------------------------------------------------
void CModelEditDlg::OnCreateAnimFromBindPose()
{
	char *szAnimName = "bind_pose";

	m_pModel->AddDummyAnimation( szAnimName );

	AnimInfo* pInfo;
	uint32 nAnimIndex;
	ModelAnim *pAnim = m_pModel->FindAnim( szAnimName, &nAnimIndex, &pInfo );

	if( pAnim )
	{
		for( int kfcnt = 0 ; kfcnt < pAnim->m_KeyFrames.GetSize() ;kfcnt++) 
			pAnim->m_KeyFrames[kfcnt].m_pString = pAnim->GetModel()->AddString("");
		
		// fill anim. got down model hierarchy & anim 
		FillAnimDataFromTransformNodeData( pAnim->GetRootNode());

		// setup the dims for the binding pose
		if(pInfo)
		{
			LTVector vDims;
			if(CDimensionsDlg::FindAnimDims(m_pModel, nAnimIndex, 0, vDims))
				pInfo->m_vDims = vDims;
		}

		// tell model edit about it.
		FillAnimList();
	}
}

void CModelEditDlg::OnRenameNode() 
{
	RenameNodeDlg dlg;
	DWORD i;
	char msg[200];
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	if (!m_pModel)
		return;
	
	if (dlg.DoModal() == IDOK)
	{
		for (i=0; i < m_pModel->m_FlatNodeList; i++)
		{
			if (strcmp(m_pModel->m_FlatNodeList[i]->GetName(), dlg.m_OldName) == 0)
			{
				m_pModel->m_FlatNodeList[i]->SetName((char*)(LPCTSTR)dlg.m_NewName);

				sprintf(msg, "'%s' has been renamed to '%s'.", dlg.m_OldName, (char*)(LPCTSTR)dlg.m_NewName);
				MessageBox(msg, "ModelEdit", MB_OK);
				SetChangesMade();

				// Redo the node list.
				FillNodeList();
				return;
			}
		}
	
		sprintf(msg, "Couldn't find node: '%s'.", dlg.m_OldName);
		MessageBox(msg, "ModelEdit", MB_OK);
	}
}



void UnrotateNodes(Model *pModel, DWORD transformIndex, int *items, int nItems, int nLevels)
{
	int i;
	DWORD j, iAnim;
	ModelAnim *pAnim;
	ModelNode *pNode;


	pNode = pModel->m_FlatNodeList[transformIndex];
	for (i=0; i < nItems; i++)
	{
		iAnim = (DWORD)items[i];
		pAnim = pModel->GetAnim(iAnim);

		pModel->UnrotateAnimNode(pAnim, pAnim->m_AnimNodes[transformIndex]);
	}

	// Do the child nodes too?
	--nLevels;
	if (nLevels > 0)
	{
		for (j=0; j < pNode->m_Children; j++)
		{
			UnrotateNodes(pModel, pNode->m_Children[j]->GetNodeIndex(), items, nItems, nLevels);
		}
	}
}


void CModelEditDlg::OnUnrotateTopNode() 
{
	int status, nLevels, nItems, items[100];
	char msg[256];


	if (!m_pModel || !GetAnimList(items, sizeof(items), nItems))
		return;

	nLevels = 1;
	if (nLevels > 1)
	{
		sprintf(msg, "There are %d levels of top-level nulls.  Unrotate them all?", nLevels);
		status = MessageBox(msg, "ModelEdit", MB_YESNOCANCEL);
		if (status == IDCANCEL)
			return;
		else if (status == IDNO)
			nLevels = 1;
	}

	if (nLevels == 0)
		nLevels = 1;

	UnrotateNodes(m_pModel, m_pModel->GetRootNode()->GetNodeIndex(), items, nItems, nLevels);
	SetChangesMade();
}


BOOL CModelEditDlg::PreTranslateMessage(MSG* pMsg) 
{
	// check if the user is editing something in a tree control
	// see knowledge base Q167960 for an explanation (basically, tree controls and dialogs don't mix)
	if( pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE) )
	{
		// check the piece list
		CEdit* edit = m_PieceList.GetEditControl();
		if( edit )
		{
			edit->SendMessage( WM_KEYDOWN, pMsg->wParam, pMsg->lParam );
			return TRUE;
		}
	}

	TranslateAccelerator(m_hWnd, m_hAccel, pMsg);
	
	return CDialog::PreTranslateMessage(pMsg);
}


void CModelEditDlg::OnGenerateVertexNormals()
{
	if (!m_pModel)
		return;

	gn_BuildModelVertexNormals(m_pModel);
	SetChangesMade();
}


void CModelEditDlg::OnReverseAnimation()
{
	int items[100], nItems, i, j;
	ModelAnim *pAnim, *pFrame1, *pFrame2;
	DWORD nKeyFrames, iKeyframe1, iKeyframe2, oldAnimTime;


	if (!m_pModel || !GetAnimList(items, sizeof(items), nItems))
		return;

	for (i=0; i < nItems; i++)
	{
		if (items[i] >= (int)m_pModel->m_Anims.GetSize())
			continue;

		pAnim = m_pModel->GetAnim(items[i]);
		oldAnimTime = pAnim->GetAnimTime();

		nKeyFrames = pAnim->m_KeyFrames.GetSize();
		for (j=0; j < (int)(nKeyFrames/2); j++)
		{
			iKeyframe1 = j;
			iKeyframe2 = nKeyFrames - j - 1;

			pFrame1 = pAnim->CopyKeyFrame(iKeyframe1);
			pFrame2 = pAnim->CopyKeyFrame(iKeyframe2);

			if (!pFrame1 || !pFrame2)
			{
				if (pFrame1)
					delete pFrame1;

				if (pFrame2)
					delete pFrame2;

				MessageBox("Error copying frames.", "ModelEdit", MB_OK);
				return;
			}
			
			if (!pAnim->PasteKeyFrame(pFrame2, iKeyframe1))
			{
				MessageBox("Error pasting keyframe.");
				return;
			}
			pAnim->RemoveKeyFrame(iKeyframe1+1);

			if (!pAnim->PasteKeyFrame(pFrame1, iKeyframe2))
			{
				MessageBox("Error pasting keyframe.");
				return;
			}
			pAnim->RemoveKeyFrame(iKeyframe2+1);

			delete pFrame1;
			delete pFrame2;
		}

		// Reverse the keyframe times.
		for (i=0; i < (int)pAnim->m_KeyFrames; i++)
		{
			pAnim->m_KeyFrames[i].m_Time = oldAnimTime - pAnim->m_KeyFrames[i].m_Time;
		}
	}
}


void CModelEditDlg::OnSelectNullNodes()
{
}


void CModelEditDlg::OnRemoveNode()
{
	int iIndex;
							 
	if (!m_pModel)
		return;

	iIndex = m_NodeList.GetSelectionMark();
	if (iIndex < 0)
		return;

	if ((DWORD)iIndex < m_pModel->NumNodes())
	{
		m_pModel->RemoveNode(m_pModel->GetNode((DWORD)iIndex));
	}

	FillNodeList();
}


void CModelEditDlg::OnSetTexture()
{
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	DWORD nTexture = 0;

	// if we don't have a model loaded bomb out.
	if( m_pModel == NULL )
	{
		MessageBox("Must have a model to set textures.","WARNING", MB_ICONWARNING);
		return ;
	}

	// Look up the open directory in the registry
	CRegMgr regMgr;
	CString csOpenDir;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		regMgr.Get("Texture", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		csOpenDir.ReleaseBuffer(-1);
	}

	// Strip the extension...
	char* pExt = _tcsrchr(csOpenDir,'.'); 
	if (pExt) { csOpenDir.TrimRight(pExt); }
	csOpenDir += "*.dtx";

	char szFilter[] = "Texture Files (*.dtx)|*.dtx|All Files (*.*)|*.*||";
	CFileDialog dlg (TRUE, "dtx", (csOpenDir.GetLength()) ? (LPCTSTR)csOpenDir : NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	// Store the open directory in the registry
	csOpenDir = dlg.GetPathName();
	csOpenDir = csOpenDir.Left(csOpenDir.GetLength() - dlg.GetFileName().GetLength());
	regMgr.Set("Texture", (LPCTSTR)csOpenDir);

	//  checking for errors while setting the texture
	bool textureSetSuccessful = true;

	// Find the selected texture in the piece list
	if (m_pModel)
	{
		vector<PieceLODInfo> selection;
		int numSelected = GetSelectedPieceLODs( &m_PieceList, m_pModel, selection );

		// grab the indices we need to set
		set<int> indices;
		indices.clear();

		for( int i = 0; i < selection.size(); i++ )
		{
			indices.insert( selection[i].m_PieceLOD->m_iTextures[0] );
		}

		// set the texture indices
		set<int>::iterator it;

		for( it = indices.begin(); it != indices.end(); it++ )
		{
			if( !SetRenderTexture( dlg.GetPathName(), *it ) )
				textureSetSuccessful = false;
		}
	}

	if( !textureSetSuccessful )
	{
		MessageBox( "Unable to load or set texture", "ModelEdit", MB_OK );
	}
	/*
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	DWORD nTexture = 0;

	// if we don't have a model loaded bomb out.
	if( m_pModel == NULL )
	{
		MessageBox("Must have a model to set textures.","WARNING", MB_ICONWARNING);
		return ;
	}

	// if we don't have a model loaded bomb out.
	if( m_pModel == NULL )
	{
		MessageBox("Must have a model to set textures.","WARNING", MB_ICONWARNING);
		return ;
	}

	// Look up the open directory in the registry
	CRegMgr regMgr;
	CString csOpenDir;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "OpenDir", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		regMgr.Get("Texture", csOpenDir.GetBufferSetLength(dwSize), dwSize);
		csOpenDir.ReleaseBuffer(-1);
	}

	// Strip the extension...
	char* pExt = _tcsrchr(csOpenDir,'.'); 
	if (pExt) { csOpenDir.TrimRight(pExt); }
	csOpenDir += "*.dtx";

	char szFilter[] = "Texture Files (*.dtx)|*.dtx|All Files (*.*)|*.*||";
	CFileDialog dlg (TRUE, "dtx", (csOpenDir.GetLength()) ? (LPCTSTR)csOpenDir : NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	// Store the open directory in the registry
	csOpenDir = dlg.GetPathName();
	csOpenDir = csOpenDir.Left(csOpenDir.GetLength() - dlg.GetFileName().GetLength());
	regMgr.Set("Texture", (LPCTSTR)csOpenDir);

	//  checking for errors while setting the texture
	bool textureSetSuccessful = true;

	// Find the selected texture in the piece list
	
	for (i = 0; i < (DWORD)m_PieceList.GetItemCount(); i++)
	{
		vector<PieceLODInfo> selection;
		int numSelected = GetSelectedPieceLODs( &m_PieceList, m_pModel, selection );

		// grab the indices we need to set
		set<int> indices;
		indices.clear();

		for( int i = 0; i < selection.size(); i++ )
		{
			indices.insert( selection[i].m_PieceLOD->m_iTextures[0] );
		}

		// set the texture indices
		set<int>::iterator it;

		for( it = indices.begin(); it != indices.end(); it++ )
		{
			if( !SetRenderTexture( dlg.GetPathName(), *it ) )
				textureSetSuccessful = false;
		}
	}

	if( !textureSetSuccessful )
	{
		MessageBox( "Unable to load or set texture", "ModelEdit", MB_OK );
	}
	*/
}


void CModelEditDlg::OnAddChildModel()
{
	CString sDefExtension;
	sDefExtension.LoadString(IDS_ADDCHILDMODELEXT);
	CString sDefFilter;
	sDefFilter.LoadString(IDS_ADDCHILDMODELFILTER);

	CAddChildModelDlg dlg(TRUE, sDefExtension, NULL, OFN_FILEMUSTEXIST, sDefFilter);

	CString curFilename, str;
	ModelLoadRequest request;
	Model *pChild = NULL;
	ChildInfo *pInfo;
	char errStr[2048];
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	


	if (!m_pModel)
		return;

	if (dlg.DoModal() == IDOK)
	{
		if (m_pModel->FindChildModelByFilename((char*)(LPCTSTR)dlg.GetFileName()))
		{
			DoMessageBox(IDS_NODUPLICATECHILDMODELNAMES, MB_OK);
		}
		else
		{
			char dirName[1024];
			char newFilename[2048];


			request.m_bLoadChildModels = FALSE;
		//	pChild = LoadChildModel(&request, (char*)(LPCTSTR)m_strFilename, pChildFilename);

			// begin ----------------------------------------------------
			// I'm inlining the child model loading here.
			// we need the current alloc from the model and other data that's relevant.
			
			

			CString sFilepath = dlg.GetPathName();

			CHelpers::ExtractNames(m_strFilename, dirName, NULL, NULL, NULL);
			if (dirName[0] == 0)
			{
				sprintf(newFilename, "%s", (LPCTSTR)dlg.GetFileName());
			}
			else
			{
				sprintf(newFilename, "%s\\%s", dirName, (LPCTSTR)dlg.GetFileName());
			}

			sprintf(newFilename, "%s", sFilepath);
	
			int base_is_lta = IsFilenameLTAExt( m_strFilename ) ;
			int child_is_lta= IsFilenameLTAExt( (LPCTSTR)dlg.GetFileName() );

			if( base_is_lta && child_is_lta )
			{
				// Load up the shared anims.

				pChild = load_LTA_Using_FeedBackDialog( newFilename, true );
				//pChild = ltaLoadChildModel( newFilename , (LAllocCount*)m_pModel->GetAlloc() );			

			}
			else 
			{
				MessageBox("ABC LOAD ATTEMPT IN  ON_ADD_CHILD_MODEL", "ERROR", MB_OK);

			}

			// end -------------------------------------------------------

			if (!pChild)
			{
 				str.FormatMessage(IDS_CANTLOADCHILDMODEL, (LPCTSTR)dlg.GetFileName());
				DoMessageBox(str, MB_OK);
				return;
			}

			// Build the correct filename right here people....

			char sSrc1[1024];
			char sSrc2[1024];

			char *sSrc, *sDst;

			sprintf(sSrc1, "%s", m_strFilename);
			sprintf(sSrc2, "%s", (char *)(LPCSTR)dlg.GetPathName());

			// Compare strings until they differ....

			int szSrc1 = strlen(sSrc1);
			int szSrc2 = strlen(sSrc2);
			char sDstFilename[1024];
			memset(sDstFilename, 0, 1024);

			bool bUseNewFilename = false;
			
			if (szSrc1 && szSrc2)
			{			
				int nCount = 0;
				int nSize = szSrc1 > szSrc2 ? szSrc1 : szSrc2;

				while (nCount < nSize)
				{
					if ((sSrc1[nCount] != sSrc2[nCount]) && (nCount))
					{
						// This is where the change occurs, last character should be a 
						// backslash

						if (sSrc1[nCount - 1] == '\\')
						{
							sSrc = sSrc1 + nCount;
							sDst = sSrc2 + nCount;

							// Now, count how many backslashes there are....

							int nSave = nCount;
							int nBackSlashes = 0;
							
							while (nCount < szSrc1)
							{
								if (sSrc1[nCount] == '\\')
								{
									nBackSlashes ++;
								}

								nCount ++;
							}

							nCount = nSave;

							for (int i = 0; i < nBackSlashes; i ++)
							{
								strcat(sDstFilename, "..\\");
							}

							char *sOut = sSrc2 + nCount;

							strcat(sDstFilename, sOut);
							strcat(sDstFilename, "\0");

							nCount = nSize;
							bUseNewFilename = true;
						}
					}

					nCount ++;
				}
			}

			CString sFileName = bUseNewFilename ? sDstFilename : (char *)(LPCSTR)dlg.GetFileName();
//			if (pInfo = m_pModel->AddChildModel(pChild, (char*)((LPCTSTR)dlg.GetFileName()), errStr, dlg.m_bScaleSkeleton))
			if (pInfo = m_pModel->AddChildModel(pChild, (char *)(LPCSTR)sFileName, errStr, dlg.m_bScaleSkeleton ? true : false))
			{
				// Update bounding radii.
				pInfo->m_SaveIndex = pChild->GetSaveIndex();

				// Update other stuff.
				str.FormatMessage(IDS_ADDEDCHILDSUCCESSFULLY, (LPCTSTR)dlg.GetFileName());
				DoMessageBox(str, MB_OK);

				FillChildModelList();
				FillAnimList();
				InitTaggedArrays();

				SetChangesMade();
			}
			else
			{
				delete pChild;
				str.FormatMessage(IDS_CANTADDCHILDMODEL, errStr);
				DoMessageBox(str, MB_OK);
			}
		}
	}
}


void CModelEditDlg::OnRemoveChildModel()
{
	int iCurSel;
	uint32 itemIndex;
	CString curFilename, itemText;
							 
	if (!m_pModel)
		return;

	iCurSel = m_ChildModelList.GetCurSel();
	if (iCurSel != LB_ERR)
	{
		m_ChildModelList.GetText(iCurSel, itemText);
		if (m_pModel->FindChildModelByFilename((char*)(LPCTSTR)itemText, &itemIndex))
		{

			if (m_pModel->RemoveChildModel(itemIndex))
			{
				StopAllPlayback();
				m_AnimInfos[ANIMINFO_MAIN].m_Tracker.m_TimeRef.Init(m_pModel);

				FillChildModelList();
				FillAnimList();
				InitTaggedArrays();

				SetChangesMade();
			}
		}
	}
}


void CModelEditDlg::OnRebuildChildModelTree()
{
}


void CModelEditDlg::OnTimer(UINT nIDEvent) 
{
	static BOOL bFirst=TRUE;

	// Handle startup options (we do it in the timer so we can do MessageBox).
	if (bFirst)
	{
		bFirst = FALSE;

		if (__argc > 1)
		{
			DoLoadModel(__argv[1], FALSE);
		}

		if (__argc > 2)
		{
			SetRenderTexture(__argv[2]);
		}
	}

	CDialog::OnTimer(nIDEvent);
}


void CModelEditDlg::OnAddSocket()
{
	DWORD i;
    uint32 nodeIndex;
	int curSel;
	AddSocketDlg dlg;
	char testName[256];
	ModelSocket *pSocket;
	DVector nodePos, parentPos, forwardVec;
	DVector vecs[3];
	ModelNode *pNode, *pParent;
	DMatrix mat, fullTransform, *pNodeTransform, *pParentTransform;
	TransformMaker tMaker;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	


	if (!m_pModel)
		return;

	curSel = m_NodeList.GetSelectionMark();
	if ((DWORD)curSel < m_pModel->NumNodes())
	{
		dlg.m_NodeName = m_pModel->GetNode((DWORD)curSel)->GetName();
	}

	// Fill in a unique name.
	for (i=0; i < m_pModel->NumSockets()+1; i++)
	{
		sprintf(testName, "Socket%d", i);
		if (!m_pModel->FindSocket(testName))
		{
			dlg.m_SocketName = testName;
			break;
		}
	}

	dlg.m_pDlg = this;
	if (dlg.DoModal() == IDOK)
	{
		// No duplicate names..
		if (m_pModel->FindSocket((LPCTSTR)dlg.m_SocketName))
			return;

		// Node must exist.
		if (!m_pModel->FindNode((LPCTSTR)dlg.m_NodeName, &nodeIndex))
			return;

		if (pSocket = new ModelSocket)
		{
			pSocket->SetName((LPCTSTR)dlg.m_SocketName);
			pSocket->m_iNode = nodeIndex;
			
			// Set it up facing down the bone.
			pNode = m_pModel->GetNode(nodeIndex);
			if (pParent = m_pModel->FindParent(m_pModel->GetNode(nodeIndex), m_pModel->GetRootNode()))
			{
				if (m_pModel->NumAnims() > 0)
				{
					tMaker.m_Anims[0].Init(m_pModel, 0, 0, 0, 0, 0.0f);
					tMaker.m_nAnims = 1;
					if (tMaker.SetupTransforms())
					{
						pNodeTransform = m_pModel->GetTransform(pNode->GetNodeIndex());
						pParentTransform = m_pModel->GetTransform(pParent->GetNodeIndex());

						Mat_GetTranslation(*pNodeTransform, nodePos);
						Mat_GetTranslation(*pParentTransform, parentPos);
					
						forwardVec = nodePos - parentPos;
						forwardVec.Norm();

						gr_BuildFrameOfReference(&forwardVec, NULL, &vecs[0], &vecs[1], &vecs[2]);
						Mat_Identity(&mat);
						
						Mat_SetBasisVectors(&mat, &vecs[0], &vecs[1], &vecs[2]);

						fullTransform = pNodeTransform->MakeInverseTransform() * mat;
						pSocket->m_Rot.ConvertFromMatrix(fullTransform);
					}
				}
			}
			
			if (m_pModel->AddSocket(pSocket))
			{
				SetChangesMade();
				FillSocketList();
				m_SocketList.SetCurSel((int)(m_pModel->NumSockets() - 1));
			}
			else
			{
				delete pSocket;
			}
		}
	}
}


void CModelEditDlg::OnRemoveSocket()
{
	int curSel;

	if (!m_pModel)
		return;

	curSel = m_SocketList.GetCurSel();
	if (curSel != LB_ERR && curSel < (int)m_pModel->NumSockets())
	{
		m_pModel->RemoveSocket((DWORD)curSel);
		FillSocketList();
	}
}


void CModelEditDlg::OnRenameSocket()
{
	int curSel;
	CStringDlg dlg;
	LPCTSTR pName;
	ModelSocket *pSocket;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	if (!m_pModel)
		return;

	curSel = m_SocketList.GetCurSel();
	if (curSel != LB_ERR && curSel < (int)m_pModel->NumSockets())
	{
		pSocket = m_pModel->GetSocket((DWORD)curSel);

		dlg.m_bAllowLetters = TRUE;
		dlg.m_bAllowNumbers = TRUE;
		dlg.m_bAllowOthers = TRUE;
		dlg.m_EnteredText = pSocket->GetName();

		if (dlg.DoModal(IDS_MODELEDIT_CAPTION, IDS_ENTER_SOCKETNAME) == IDOK)
		{
			pName = (LPCTSTR)dlg.m_EnteredText;
			if (strlen(pName) == 0 || m_pModel->FindSocket(pName))
			{
				DoMessageBox(IDS_DUPLICATESOCKETNAME, MB_OK);
			}
			else
			{
				pSocket->SetName(pName);
				FillSocketList(TRUE);
			}
		}
	}
}


void CModelEditDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DWORD i;

	CDialog::OnLButtonDown(nFlags, point);

	// See if it's in a tracker.
	for (i=0; i < m_Trackers; i++)
	{
		if (m_Trackers[i]->m_Rect.PtInRect(point))
		{
			StartTracker(i, point);
		}
	}
}


void CModelEditDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	RectTracker *pTracker;
	CPoint delta, center;
	float rotAngle;
	ModelSocket *pSocket;
	LTMatrix mNewTransform, mOldTransform,mOffset, mFinal, mSocket, mBasis;
	LTVector vecs[3], vOffset, vTrans;
	char buf[512];
	int i;
	DWORD iNode;
	
	AnimInfo *pAnimInfo;
	ChildInfo *pChildInfo;
	CIRelation *pRelation;
	ModelNode *pNode, *pParent;
	TransformMaker tMaker;
	BOOL bGlobal;

	
	if (!m_pModel || !m_bTracking || m_iCurTracker >= m_Trackers.GetSize())
		return;

	pTracker = m_Trackers[m_iCurTracker];
	bGlobal = !!m_GlobalSpace.GetCheck();

	vecs[0].Init(1.0f, 0.0f, 0.0f);
	vecs[1].Init(0.0f, 1.0f, 0.0f);
	vecs[2].Init(0.0f, 0.0f, 1.0f);

	delta = point - m_LastCursorPos;
	if (delta.x == 0)
		return;

	m_TotalDelta += delta.x;

	// Generate some feedback to window's text field
	switch( pTracker->m_Type )
	{
	case RT_ROTATION:
		// rotate x degrees for every pixel
		rotAngle = GetRotAngle( delta.x );
		sprintf( buf, "Rotation: %.1f", (GetRotAngle(m_TotalDelta) * 360.0f) / MATH_CIRCLE );
		break;
	case RT_POSITION:
		sprintf( buf, "Translation: %.1f", GetPosAmount(m_TotalDelta) );
		break;
	case RT_SCALE:
		if( IsButtonSelected(IDC_SOCKET_EDIT) && GetSelectedSocket() )
			sprintf( buf, "Scale: %.2f", GetScaleAmount(m_TotalDelta) );
		else
			sprintf( buf, "Scale not supported on transforms." );
		break;
	}

	// Do socket/relation specific stuff.
	// note: this includes scale, but we treat scale separately.
	if (IsButtonSelected(IDC_SOCKET_EDIT) && (pSocket = GetSelectedSocket()))
	{
		pSocket->ConvertToMatrixNoScale(mSocket);

		// Setup the transforms..
		if(!m_RenderWnd.SetupTransformMaker(&tMaker))
			return;

		if (!tMaker.SetupTransforms())
			return;

		mOldTransform = m_pModel->m_Transforms[pSocket->m_iNode];
		mNewTransform = mOldTransform * mSocket;
			
		mOffset.Identity();

		switch( pTracker->m_Type )
		{
		case RT_ROTATION:
			Mat_SetupRot( &mOffset, &vecs[pTracker->m_Axis], rotAngle );
			break;
		case RT_POSITION:
			vOffset.Init();
			vOffset[pTracker->m_Axis] = GetPosAmount( delta.x );
			mOffset.SetTranslation( vOffset );
			break;
		}

		// Put the offset in local space.
		if (bGlobal)
		{
			mBasis.Identity();
			mNewTransform.GetTranslation(vTrans);
			mBasis.SetTranslation(vTrans);
		}
		else
		{
			mBasis = mNewTransform;
		}

		// Put the offset into the right basis.
		mOffset = mBasis * mOffset * ~mBasis;

		mFinal = ~mOldTransform * (mOffset * mNewTransform);

		pSocket->ConvertFromMatrixNoScale(mFinal);

		// scale is a special case: we don't want to include it in the other transforms...
		if( pTracker->m_Type == RT_SCALE )
		{
			pSocket->m_Scale[pTracker->m_Axis] = GetScaleAmount(m_TotalDelta );
		}
	}
	else if( pTracker->m_Type != RT_SCALE )// else change the transformation (if it isn't a scale)
	{
		// Find the first selected animation
		pAnimInfo = NULL;
		for (i=0; (i < m_AnimList.GetItemCount()) && (!pAnimInfo); i++)
		{
			if (m_AnimList.GetItemState(i, LVIS_SELECTED) != 0)
				pAnimInfo = m_pModel->GetAnimInfo(i);
		}
		if (pAnimInfo)
		{
			pChildInfo = pAnimInfo->m_pChildInfo;

			// Change the weights on the selected nodes.
			for (i=0; i < m_NodeList.GetItemCount(); i++)
			{
				// Skip over items that aren't selected
				if (!m_NodeList.GetItemState(i, LVIS_SELECTED))
					continue;

				iNode = (DWORD)i;
				if (iNode >= pChildInfo->m_Relation.GetSize())
					continue;

				pNode = m_pModel->GetNode(iNode);
				pParent = m_pModel->FindParent(pNode, m_pModel->GetRootNode());
				if (!pParent)
					continue;

				pRelation = &pChildInfo->m_Relation[iNode];
				
				// Get the old and new (using the relation) node transforms (so we can setup the relation).
				if(!m_RenderWnd.SetupTransformMaker(&tMaker))
					return;

				if (!tMaker.SetupTransforms())
					return;

				mNewTransform = m_pModel->m_Transforms[iNode];
				
				pRelation->m_Pos.Init();
				pRelation->m_Rot.Init();
				if (!tMaker.SetupTransforms())
					return;

				mOldTransform = m_pModel->m_Transforms[iNode];

				
				// Make an offset.
				mOffset.Identity();
				if (pTracker->m_Type == RT_ROTATION)
				{
					Mat_SetupRot(&mOffset, &vecs[pTracker->m_Axis], rotAngle);
				}
				else
				{
					vOffset.Init();
					vOffset[pTracker->m_Axis] = GetPosAmount(delta.x);
					mOffset.SetTranslation(vOffset);
				}

				
				// Put the offset into the node's local space.
				if (bGlobal)
				{
					mBasis.Identity();
					mNewTransform.GetTranslation(vTrans);
					mBasis.SetTranslation(vTrans);
				}
				else
				{
					mBasis = mNewTransform;
				}

				// Figure out the relation (~old * offset * new... ie: (offset*new) - old)
				mOffset = mBasis * (mOffset * ~mBasis);
				mFinal = ~mOldTransform * (mOffset * mNewTransform);
				pRelation->ConvertFromMatrix(mFinal);
			}
		}
	}

	SetWindowText(buf);

	// Center the cursor.
	center = m_TrackerCenterPt;
	m_LastCursorPos = center;
	ClientToScreen(&center);
	SetCursorPos(center.x, center.y);

	m_RenderWnd.Draw();
	
	CDialog::OnMouseMove(nFlags, point);
}


void CModelEditDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	HandleButtonUp(0);
	CDialog::OnLButtonUp(nFlags, point);
}



LRESULT CModelEditDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_MBUTTONUP)
	{
		HandleButtonUp(1);
	}
	else if (message == WM_RBUTTONUP)
	{
		HandleButtonUp(2);
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}


void CModelEditDlg::OnMood()
{

}


void CModelEditDlg::OnInternalRadius()
{
	CStringDlg dlg;
	char tempBuf[64];
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	if (!m_pModel)
		return;

	dlg.m_bAllowLetters = FALSE;
	dlg.m_bAllowNumbers = TRUE;
	dlg.m_bAllowOthers = FALSE;
	
	sprintf(tempBuf, "%.3f", m_pModel->m_GlobalRadius);
	dlg.m_EnteredText = tempBuf;

	if (dlg.DoModal(IDS_MODELEDIT_CAPTION, IDS_ENTER_GLOBAL_RADIUS) == IDOK)
	{
		m_pModel->m_GlobalRadius = (float)atof(dlg.m_EnteredText);
		SetChangesMade();
	}
}


void CModelEditDlg::OnCalcInternalRadius()
{
	if( !m_pModel )
	{
		MessageBox( "No model loaded.", "Error", MB_OK | MB_ICONERROR );
		return;
	}

	if( MessageBox( "This operation can take a while.  Do you wish to proceed?", "Do you really want to do this?", MB_OKCANCEL ) != IDOK )
		return;

	HCURSOR oldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

	// calculate the estimated internal radius

	// turn on render distance checking
	// t.f important
	m_RenderWnd.m_bCalcRadius = true;

	// get our main anim info
	MEAnimInfo* pAnimInfo = GetAnimInfo( 0 );

	// save our current animation state
	AnimTimeRef oldTracker = pAnimInfo->m_Tracker.m_TimeRef;
	ModelAnim* pOldAnim = pAnimInfo->CurAnim();

	// initialize the maximums
	float fNewRadius = 0.0f;
	char* pAnimName = NULL;

	// for each animation
	for( uint32 nAnimLoop = 0; nAnimLoop < m_pModel->NumAnims(); ++nAnimLoop )
	{
		// get the animation
		ModelAnim* pAnim = m_pModel->GetAnim( nAnimLoop );
		if( !pAnim )
			continue;
		// switch to that animation
		pAnimInfo->m_Wnd.SetAnim( pAnim );
		trk_Init( &pAnimInfo->m_Tracker, m_pModel, nAnimLoop );

		// resert the maximum radius
		float fAnimRadius = 0.0f;

		// for each keyframe
		for( uint32 nKeyframeLoop = 0; nKeyframeLoop < pAnim->NumKeyFrames(); ++nKeyframeLoop )
		{
			// move to this keyframe
			SetCurrentPosition( (DWORD)0, nKeyframeLoop, nKeyframeLoop, 0.0f );
			// only draw the first frame
			// t.f important
 			m_RenderWnd.m_bCalcAndDraw = (nKeyframeLoop == 0);
			// draw the model
			m_RenderWnd.Draw();
			// get the maximum of the animation and this frame's radii
			// t.f important			
			fAnimRadius = LTMAX( fAnimRadius, m_RenderWnd.m_fCurRadius );
		}

		// new maximum distance
		if( fAnimRadius > fNewRadius )
		{
			// save distance and animation name
			fNewRadius = fAnimRadius;
			pAnimName = m_pModel->GetAnim( nAnimLoop )->GetName();
		}
	}

	// turn off render distance checking
	// t.f important
	m_RenderWnd.m_bCalcRadius = false;

	// add a fudge factor to the distance
	fNewRadius *= 1.01f;

	// restore the old animation state
	pAnimInfo->m_Wnd.SetAnim( pOldAnim );
	pAnimInfo->m_Tracker.m_TimeRef = oldTracker;

	// restore non-wait cursor
	SetCursor( oldCursor );

	// display distance and largest animation and ask user to confirm
	char results[2048];
	sprintf( results, "Results:\n"
					  "\tOld Dimensions: %f\n"
					  "\tNew Dimensions: %f\n"
					  "\tBiggest Animation: %s\n"
					  "\nUse these results?", m_pModel->m_GlobalRadius, fNewRadius, pAnimName );
	if( MessageBox( results, "Is that better?", MB_YESNO ) != IDYES )
		return;

	// save the new distance
	m_pModel->m_GlobalRadius = fNewRadius;

	SetChangesMade();
}

void CModelEditDlg::OnReCalcOBBExtent()
{
	if(!m_pModel)
		return ;

	
	void model_CreateDefaultOBB(Model*);
	model_CreateDefaultOBB(	m_pModel );

}


void CModelEditDlg::OnEditWeights()
{
	WeightEditDlg dlg;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	
	
	if (!m_pModel)
		return;

	dlg.m_pDlg = this;
	dlg.m_pModel = m_pModel;
	dlg.DoModal();
	if (dlg.m_bChangesMade)
		SetChangesMade();
}


void CModelEditDlg::SetCurrentLOD(float fLOD)
{
	m_CurrentLODDist = fLOD;
	m_RenderWnd.SetLOD(fLOD);
}

//this will create a NULL piece LOD at the specified distance from the camera
void CModelEditDlg::OnCreateNullLOD()
{
	// grab the selected piece LODs
	vector<ModelPiece*> selection;
	int numSelected = GetSelectedPieces( &m_PieceList, m_pModel, selection );

	if( !numSelected )
		return;

	CStringDlg Dlg;
	Dlg.m_bAllowNumbers = TRUE;

	if(Dlg.DoModal(IDS_ENTER_NULL_LOD_DISTANCE_CAPTION, IDS_ENTER_NULL_LOD_DISTANCE) != IDOK)
	{
		return;
	}

	float fDist = (float)atof(Dlg.m_EnteredText);

	//alright, time to create the NULL lod at the specified distance
	for(uint32 nCurrSel = 0; nCurrSel < selection.size(); nCurrSel++)
	{
	
		//add it to the piece
		ModelPiece* pPiece = selection[nCurrSel];

		//sanity check
		assert(pPiece);

		//add this LOD to the list
		pPiece->AddNullLOD(fDist);
	}
	
	// reload the piece tree
	FillPieceList( &m_PieceList, m_pModel );

	// adjust the LOD slider
	float maxLODDist = GetModelMaxLODDist( m_pModel );
	InitTheScrollBar( maxLODDist );
	SetCurrentLOD( 0.0f );

	//flag changes for save
	SetChangesMade();
}


void CModelEditDlg::OnPieceInfo()
{
	int i;
	IdleChanger idle(&m_cWinIdle, IDLE_DIALOG_DELAY);  	

	// grab the selected piece LODs
	vector<PieceLODInfo> selection;

	int numSelected = GetSelectedPieceLODs( &m_PieceList, m_pModel, selection );

	if( !numSelected )
		return;

	// launch the piece info dialog
	CPieceMaterialDlg dlg( selection );
	if( dlg.DoModal() == IDOK )
	{
		// check if any lod dists changed
		if( dlg.m_LODDistChanged )
		{
			// sort the piece lods
			for( i = 0; i < m_pModel->NumPieces(); i++ )
			{
				// get the current piece
				ModelPiece* curPiece = m_pModel->GetPiece( i );
				if( !curPiece )
				{
					ASSERT(0);
					continue;
				}
				curPiece->SortLODs();
			}

			// reload the piece tree
			FillPieceList( &m_PieceList, m_pModel );

			// adjust the LOD slider
			float maxLODDist = GetModelMaxLODDist( m_pModel );
			InitTheScrollBar( maxLODDist );
			SetCurrentLOD( 0.0f );
		}

		SetChangesMade();
	}
}


void CModelEditDlg::OnPieceSelectNone( void )
{
	m_PieceList.ClearSelection();
}


void CModelEditDlg::OnPieceExpandAll( void )
{
	m_PieceList.ExpandAll();
}


void CModelEditDlg::OnPieceCollapseAll( void )
{
	m_PieceList.CollapseAll();
	m_PieceList.ClearSelection();
}


void CModelEditDlg::OnEndlabeleditNodeList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	// Update the node's name
	if ((pDispInfo->item.mask & LVIF_TEXT) != 0)
		m_pModel->GetNode(pDispInfo->item.iItem)->SetName(pDispInfo->item.pszText);
	
	SetChangesMade();

	*pResult = TRUE;
}

void CModelEditDlg::OnItemChangedNodeList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ModelNode *pNode;
	unsigned char newFlags;

	// Refresh the selection state array entry
	// t.f important	
	
	if (m_pModel && m_RenderWnd.GetSelectedNodesSize() ) //m_SelectedNodes)
	{
		DWORD iIndex = (DWORD)pNMListView->iItem;

		if (m_NodeList.GetItemState((int)iIndex, LVIS_SELECTED))
		{
			int bSelect = m_NodeList.GetCheck((int)iIndex) ? true : false;

			m_RenderWnd.m_SelectedNodes[iIndex] = TRUE;

			for (int nCount = 0; nCount < m_NodeList.GetItemCount(); nCount ++)
			{
				if (m_NodeList.GetItemState((int)nCount, LVIS_SELECTED))
				{
					m_NodeList.SetCheck(nCount, bSelect);

					pNode = m_pModel->GetNode(iIndex);

					if (g_bUpdateNodeFlags)
					{
						newFlags = pNode->m_Flags;

						if (m_NodeList.GetCheck(nCount))
							newFlags |= MNODE_ROTATIONONLY;
						else
							newFlags &= ~MNODE_ROTATIONONLY;

						if (newFlags != pNode->m_Flags)
						{
							pNode->m_Flags = newFlags;
						}			
					}

					SetChangesMade();
				}
			}
		}
		else
		{

			DWORD iIndex = (DWORD)pNMListView->iItem;
			if ((iIndex < (DWORD)m_NodeList.GetItemCount()) && (iIndex < m_RenderWnd.GetSelectedNodesSize()))
				m_RenderWnd.m_SelectedNodes[iIndex] = m_NodeList.GetItemState((int)iIndex, LVIS_SELECTED) != 0;

			pNode = m_pModel->GetNode(iIndex);

			if (g_bUpdateNodeFlags)
			{
				newFlags = pNode->m_Flags;

				if (m_NodeList.GetCheck(pNMListView->iItem))
					newFlags |= MNODE_ROTATIONONLY;
				else
					newFlags &= ~MNODE_ROTATIONONLY;

				if (newFlags != pNode->m_Flags)
				{
					pNode->m_Flags = newFlags;
				
				}
			}
			 SetChangesMade();
		}
	}
	
	*pResult = 0;
}

void CModelEditDlg::OnDestroy() 
{
	// Stop the idling thread
	m_cWinIdle.EndIdle();

	CDialog::OnDestroy();
}

// one of the tree controls has had its selection changed
LRESULT CModelEditDlg::OnTreeSelChanged( WPARAM wParam, LPARAM lParam )
{
	CLTWinTreeMgr* tree = (CLTWinTreeMgr*)wParam;

	switch( tree->GetDlgCtrlID() )
	{
	case IDC_PIECES:		// handle the piece tree control
		PieceSelChanged( tree );
		break;
	default:
		ASSERT(0);			// unhandled tree control
		break;
	}

	return 0;
}

// a new piece has been selected
void CModelEditDlg::PieceSelChanged( CLTWinTreeMgr* tree )
{
	// t.f important	
	
	if( !m_pModel || !m_RenderWnd.GetSelectedPiecesSize() )
		return;

	PieceListItem* item;

	// get the root iterator
	CLTWinTreeItemIter* iter = tree->CreateIter();
	if( !iter )
		return;

	// visit each piece
	for( ; iter->IsMore(); iter->Next() )
	{
		item = (PieceListItem*)iter->Current();
		int curPiece = item->m_PieceNum;

		bool selected = item->IsSelected() != 0;

		// check the children if the parent isn't selected
		if( !selected )
		{
			CLTWinTreeItemIter* childIter = item->CreateChildIter();

			for( ; !selected && childIter->IsMore(); childIter->Next() )
			{
				item = (PieceListItem*)childIter->Current();
				selected = item->IsSelected() != 0;
			}

			delete childIter;
		}

		if( curPiece < m_RenderWnd.GetSelectedPiecesSize() )
		{
			m_RenderWnd.m_SelectedPieces[curPiece] = selected;
		}
		else
			ASSERT(0);
	}

	delete iter;
	
}

// one of the tree controls has had its selection edited
LRESULT CModelEditDlg::OnTreeEditText( WPARAM wParam, LPARAM lParam )
{
	CLTWinTreeMgr* tree = (CLTWinTreeMgr*)wParam;
	PieceListItem* item = (PieceListItem*)lParam;

	switch( tree->GetDlgCtrlID() )
	{
	case IDC_PIECES:		// handle the piece tree control
		PieceEditText( tree, item );
		break;
	default:
		ASSERT(0);			// unhandled tree control
		break;
	}

	return 0;
}

// a piece has had its name changed
void CModelEditDlg::PieceEditText( CLTWinTreeMgr* tree, PieceListItem* item )
{
	m_pModel->GetPiece( item->m_PieceNum )->SetName( item->GetText() );

	SetChangesMade();
}


// retrieve a list of the selected pieces (returns number of selected pieces)
// this does not iterate through children, the piece itself must be explicitly selected in order to be returned
int CModelEditDlg::GetSelectedPieces( CLTWinTreeMgr* tree, Model* model, vector<ModelPiece*>& selection )
{
	int numSelected = 0;
	selection.clear();

	if( !model )
		return numSelected;

	// get the root iterator
	CLTWinTreeItemIter* iter = tree->CreateIter();
	if( !iter )
		return numSelected;

	// visit each piece
	for( ; iter->IsMore(); iter->Next() )
	{
		PieceListItem* item = (PieceListItem*)iter->Current();

		if( item->IsSelected() )
		{
			ModelPiece* curPiece = model->GetPiece( item->m_PieceNum );
			ASSERT( curPiece );
			selection.push_back( curPiece );
			numSelected++;
		}
	}

	delete iter;

	return numSelected;
}


// retrieve a list of the selected piece LODs (returns number of selected LODs)
// if firstIfParentOnly is true, then only the 0th lod will be returned if only the parent piece is selected
// otherwise all lods will be returned for the selected piece
int CModelEditDlg::GetSelectedPieceLODs( CLTWinTreeMgr* tree, Model* model, vector<PieceLODInfo>& selection, bool firstIfParentOnly )
{
	PieceLODInfo pieceLOD;
	int numSelected = 0;
	selection.clear();

	if( !model )
		return numSelected;

	// get the root iterator
	CLTWinTreeItemIter* iter = tree->CreateIter();
	if( !iter )
		return numSelected;

	// visit each piece
	for( ; iter->IsMore(); iter->Next() )
	{
		PieceListItem* item = (PieceListItem*)iter->Current();
		int curPiece = item->m_PieceNum;

		// check if any children are selected first
		bool childSelected = false;

		CLTWinTreeItemIter* childIter = item->CreateChildIter();

		for( ; childIter->IsMore(); childIter->Next() )
		{
			PieceListItem* childItem = (PieceListItem*)childIter->Current();
			if( childItem->IsSelected() )
			{
				// found a selected pieceLOD, add it to the selection list
				childSelected = true;
				pieceLOD.m_ModelPiece = model->GetPiece( curPiece );
				pieceLOD.m_LODNum = childItem->m_LOD;
				pieceLOD.m_PieceLOD = pieceLOD.m_ModelPiece->GetLOD( (uint32)pieceLOD.m_LODNum );
				ASSERT( pieceLOD.m_PieceLOD );
				selection.push_back( pieceLOD );
				numSelected++;
			}
		}

		// no children are selected, check if the parent is selected
		if( !childSelected )
		{
			if( item->IsSelected() )
			{
				ModelPiece* mainPiece = model->GetPiece( curPiece );

				// parent is selected
				if( firstIfParentOnly )
				{
					// add only the first child LOD to selection
					pieceLOD.m_ModelPiece = mainPiece;
					pieceLOD.m_LODNum = 0;
					pieceLOD.m_PieceLOD = mainPiece->GetLOD( (unsigned)0 );
					ASSERT( pieceLOD.m_PieceLOD );
					selection.push_back( pieceLOD );
					numSelected++;
				}
				else
				{
					// add all child LODs to selection
					for( uint32 i = 0; i < mainPiece->NumLODs(); i++ )
					{
						pieceLOD.m_ModelPiece = mainPiece;
						pieceLOD.m_LODNum = i;
						pieceLOD.m_PieceLOD = mainPiece->GetLOD( i );
						ASSERT( pieceLOD.m_PieceLOD );
						selection.push_back( pieceLOD );
						numSelected++;
					}
				}
			}
		}

		delete childIter;
	}

	delete iter;

	return numSelected;
}

BOOL CModelEditDlg::UpdateSelectedAnimList()
{
	DWORD i, index;

	
	// Get rid of them all if the model isn't there.
	if(!m_pModel)
	{
		m_SelectedAnims.Term();
		return TRUE;
	}

	// Get rid of invalid ones.
	for(i=0; i < m_SelectedAnims; i++)
	{
		if(m_SelectedAnims[i] >= m_pModel->NumAnims())
		{
			m_SelectedAnims.Remove(i);
			--i;
		}
	}

	// Add new selections and remove old ones.
	for(i=0; i < (DWORD)m_AnimList.GetItemCount(); i++)
	{
		if(m_AnimList.GetItemState((DWORD)i, LVIS_SELECTED) == 0)
		{
			index = m_SelectedAnims.FindElement(i);
			if(index != BAD_INDEX)
			{
				m_SelectedAnims.Remove(index);
			}
		}
		else
		{
			if(m_SelectedAnims.FindElement(i) == BAD_INDEX)
			{
				m_SelectedAnims.Append(i);
			}
		}
	}

	return TRUE;
}

void CModelEditDlg::OnSelChangeAnimList()
{
	DWORD i;
	MEAnimInfo *pInfo;
	
	if(!m_pModel)
		return;


	UpdateSelectedAnimList();

	
	// Activate/deactivate the animations.
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = &m_AnimInfos[i];

		if(i >= m_SelectedAnims.GetSize())
		{
			pInfo->Reset(NULL);
		}
	}

	
	// Activate any new ones and change the animation for any existing ones.
	for(i=0; i < m_SelectedAnims.GetSize(); i++)
	{
		if(i >= NUM_ANIM_INFOS)
			break;
	
		pInfo = &m_AnimInfos[i];
		if(!pInfo->m_Wnd.IsActive())
		{
			pInfo->m_Wnd.SetActive(TRUE);
			pInfo->m_Wnd.SetAnim(m_pModel->GetAnim(m_SelectedAnims[i]));
			trk_Init(&pInfo->m_Tracker, m_pModel, m_SelectedAnims[i]);
		}
		else if(m_SelectedAnims[i] != pInfo->m_Tracker.m_TimeRef.m_Cur.m_iAnim)
		{
			pInfo->m_Wnd.SetAnim(m_pModel->GetAnim(m_SelectedAnims[i]));
			trk_SetCurAnim(&pInfo->m_Tracker, m_SelectedAnims[i], pInfo->IsAnimPlaying() ? true : false);

			// Update the frame string and time....

			UpdateEditFrameString();
			UpdateEditFrameTime();
		}

		ModelAnim *pAnim = pInfo->CurAnim();

		pInfo->m_Tracker.m_TimeRef.m_iWeightSet = pAnim ? (pAnim->m_ModelEditWeightSet) : 0;
	}

	if(m_SelectedAnims.GetSize() == 0)
	{
		UpdateEditFrameString();
		UpdateEditFrameTime();
	}



	// Set button labels.
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = &m_AnimInfos[i];
		
		ModelAnim *pAnim = pInfo->CurAnim();

		if (pAnim)
		{
			if(pInfo->m_Wnd.IsActive())
			{
				::EnableWindow(::GetDlgItem(m_hWnd, pInfo->m_AnimButtonID), TRUE);
				SetDlgItemText(pInfo->m_AnimButtonID, pAnim->GetName());
			}
			else
			{
				::EnableWindow(::GetDlgItem(m_hWnd, pInfo->m_AnimButtonID), FALSE);
				SetDlgItemText(pInfo->m_AnimButtonID, "NONE");
			}
		}
	}

	UpdateEditFrameStringEnabled();
	UpdateEditFrameTimeEnabled();
}


void CModelEditDlg::OnBeginLabelEditAnimList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = ((m_pModel->GetAnimInfo(pDispInfo->item.iItem))->GetAnimOwner() != m_pModel);
}

void CModelEditDlg::OnEndLabelEditAnimList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	if ((pDispInfo->item.mask & LVIF_TEXT) != 0)
		(m_pModel->GetAnim(pDispInfo->item.iItem))->SetName(pDispInfo->item.pszText);
	
	SetChangesMade();

	*pResult = TRUE;
}


void CModelEditDlg::OnItemChangingAnimList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uOldState & 0x3000) == 0)
		*pResult = 0;
	// Make sure the checkbox state is correct.  (Note : I have no idea what constant value this mask associates with..)
	else if ((m_pModel->GetAnimInfo(pNMListView->iItem))->GetAnimOwner() == m_pModel)
		*pResult = (pNMListView->uNewState & 0x2000) == 0;
	else
		*pResult = (pNMListView->uNewState & 0x1000) == 0;
}

void CModelEditDlg::OnBeginDragAnimList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CModelEditDlg::OnDropAnimList(int iItem)
{
	// Handle rearrangement of the animation list
	if (iItem != m_AnimList.GetDragIndex())
	{
		// Adjust for post-insertion instead of pre-insertion
		iItem = min(iItem, m_AnimList.GetItemCount() - 1);
		MoveAnims(iItem - m_AnimList.GetDragIndex());
	}
}

// ------------------------------------------------------------------------
// when the right button is clicked on one of our controls, see 
// get the menu for it.
// ------------------------------------------------------------------------
void CModelEditDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu menu;
	
	if (pWnd->m_hWnd == m_AnimList.m_hWnd)
	{
		VERIFY(menu.LoadMenu(IDR_MENU_ANIMLIST));
	}
	else if( pWnd->m_hWnd == m_NodeList.m_hWnd )
	{
		VERIFY(menu.LoadMenu(IDR_NODE_EDIT));
	}
	else if( pWnd->m_hWnd == m_PieceList.m_hWnd )
	{
		VERIFY(menu.LoadMenu(IDR_PIECE_EDIT));
	}
	else if( pWnd->m_hWnd == m_SocketList.m_hWnd )
	{
		VERIFY(menu.LoadMenu(IDR_SOCKET_EDIT));
	}
	else return;

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);	
}


void CModelEditDlg::OnSortAnimName() 
{
	// Make sure this is valid...
	if ((!m_pModel) || (m_pModel->NumAnims() < 2))
		return;

	// Sort the animation list
	AnimInfo temp;
	DWORD i,j;

	for (i = 0; i < m_pModel->NumAnims() - 1; i++)
		for (j = i + 1; j < m_pModel->NumAnims(); j++)
		{
			if (stricmp(m_pModel->GetAnim(i)->GetName(), m_pModel->GetAnim(j)->GetName()) > 0)
			{
				temp = m_pModel->m_Anims[i];
				m_pModel->m_Anims[i] = m_pModel->m_Anims[j];
				m_pModel->m_Anims[j] = temp;
			}
		}

	SetChangesMade();

	// Re-display the animation list
	FillAnimList();
}

void CModelEditDlg::OnSortAnimRelation() 
{
	// Make sure this is valid...
	if ((!m_pModel) || (m_pModel->NumAnims() < 2))
		return;

	// Sort the animation list
	AnimInfo temp;
	DWORD i,j;

	for (i = 0; i < m_pModel->NumAnims() - 1; i++)
		for (j = i + 1; j < m_pModel->NumAnims(); j++)
		{
			int iDiffi = (int)(m_pModel->GetAnimInfo(i)->GetAnimOwner() - m_pModel);
			int iDiffj = (int)(m_pModel->GetAnimInfo(j)->GetAnimOwner() - m_pModel);
			if (iDiffi > iDiffj)
			{
				temp = m_pModel->m_Anims[i];
				m_pModel->m_Anims[i] = m_pModel->m_Anims[j];
				m_pModel->m_Anims[j] = temp;
			}
		}

	SetChangesMade();

	// Re-display the animation list
	FillAnimList();
}

void CModelEditDlg::OnDblClkSocketList() 
{
	ModelSocket *pSocket;
	int iSocket;
	EulerAngles angles;

	if (!m_pModel)
		return;

	// Get the selected socket
	iSocket = m_SocketList.GetCurSel();

	// This should never happen
	if ((iSocket < 0) || (iSocket >= (int)m_pModel->NumSockets()))
		return;

	pSocket = m_pModel->GetSocket(iSocket);
	
	// Get the attachment from the registry
	CString csAttachment("");

	CRegMgr regMgr;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Attachments", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		regMgr.Get(pSocket->GetName(), csAttachment.GetBufferSetLength(dwSize), dwSize);
		csAttachment.ReleaseBuffer(-1);
	}

	CSocketEdit dlg;

	int dlgResult = IDOK;
	do
	{
		// Convert the rotation to euler angles
		angles = Eul_FromQuat(pSocket->m_Rot, EulOrdXYZs);

		// Set up the dialog
		dlg.m_Name = pSocket->GetName();
		dlg.m_NodeName= m_pModel->GetNode( pSocket->m_iNode )->GetName();
		dlg.m_PosX = pSocket->m_Pos.x;
		dlg.m_PosY = pSocket->m_Pos.y;
		dlg.m_PosZ = pSocket->m_Pos.z;
		dlg.m_RotX = angles.x * 180.0f / MATH_PI;
		dlg.m_RotY = angles.y * 180.0f / MATH_PI;
		dlg.m_RotZ = angles.z * 180.0f / MATH_PI;
		dlg.m_SclX = pSocket->m_Scale.x ;
		dlg.m_SclY = pSocket->m_Scale.y ;
		dlg.m_SclZ = pSocket->m_Scale.z ;

		dlg.m_Attachment = csAttachment;

		// Edit the socket
		dlgResult = dlg.DoModal();
		if (dlgResult == IDCANCEL)
			return;

		// Change the position
		pSocket->m_Pos.x = dlg.m_PosX;
		pSocket->m_Pos.y = dlg.m_PosY;
		pSocket->m_Pos.z = dlg.m_PosZ;

		// Change the rotation
		angles.x = dlg.m_RotX * MATH_PI / 180.0f;
		angles.y = dlg.m_RotY * MATH_PI / 180.0f;
		angles.z = dlg.m_RotZ * MATH_PI / 180.0f;
		pSocket->m_Rot = Eul_ToQuat(angles);

		// Change the scale 
		pSocket->m_Scale.Init( dlg.m_SclX, dlg.m_SclY, dlg.m_SclZ);

		// Change the name
		if (dlg.m_Name.CompareNoCase(pSocket->GetName()) != 0)
		{
			pSocket->SetName(dlg.m_Name);
			FillSocketList(TRUE);
			// Make sure the attachment tries to change to the new value
			csAttachment = _T("");
		}

		// Change the attachment
		if (dlg.m_Attachment.CompareNoCase(csAttachment) != 0)
		{
			// Clear out the old attachment
			if (pSocket->m_pAttachment)
				delete pSocket->m_pAttachment;
			pSocket->m_pAttachment = NULL;

			if (dlg.m_Attachment.GetLength())
			{
				if( IsFilenameLTAExt( dlg.m_Attachment ) )
				{
					
					char *str = dlg.m_Attachment.LockBuffer();
					dlg.m_Attachment.UnlockBuffer();
					Model *pModel = load_LTA_Using_FeedBackDialog( str, false  );

					if( pModel == NULL )
					{
						MessageBox("Failed to load lta file","load error");
						dlg.m_Attachment = csAttachment ;
					
					}else // success !!
					{
						pSocket->m_pAttachment = pModel ;
					}
				}
				else // else we're loading abcs
				{
					MessageBox("OnDblClkSocketList error, trying to load bin abc file", "ERROR",MB_OK);

				}
			
			}

			// Store the attachment in the registry if it didn't fail loading
			if (dlg.m_Attachment.CompareNoCase(csAttachment) != 0)
				regMgr.Set(pSocket->GetName(), (LPCTSTR)dlg.m_Attachment);

			// Save the string for the next go-round
			csAttachment = dlg.m_Attachment;
		}

		SetChangesMade();

	} while (dlgResult != IDOK);
}

float CModelEditDlg::CalcFOV()
{
	CRegMgr regMgr;
	CString csFOV;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Render", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = 256;
		csFOV = "90";
		regMgr.Get("FOV", csFOV.GetBufferSetLength(dwSize), dwSize);
		csFOV.ReleaseBuffer(-1);
		return max((float)atof(csFOV), 10.0f);
	}
	else
		return 90.0f;
}

void CModelEditDlg::SetFOV(float FOV)
{
	CRegMgr regMgr;
	CString csFOV;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Render", HKEY_CURRENT_USER))
	{
		csFOV.Format("%f", FOV);
		regMgr.Set("FOV", csFOV);
	}
}


MEAnimInfo* CModelEditDlg::GetAnimInfo(uint32 iAnimInfo)
{
	ASSERT(iAnimInfo < NUM_ANIM_INFOS);
	return &m_AnimInfos[iAnimInfo];
}


void CModelEditDlg::StopAllPlayback()
{
	uint32 i;

	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		m_AnimInfos[i].StopPlayback();
	}
}


void CModelEditDlg::ResetAnimInfos()
{
	uint32 i;

	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		GetAnimInfo(i)->Reset();
	}
}


void CModelEditDlg::DrawActiveAnimRects(BOOL bForce)
{
	DWORD dwActive[NUM_ANIM_INFOS];
	DWORD i;
	CDC *pDC;

	
	// Get the key state.
	GetPlaybackActiveFlags(dwActive, FALSE);

	// Redraw if necessary.	
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		if((dwActive[i] & PA_ACTIVE) != (m_LastAnimActiveFlags[i] & PA_ACTIVE) || 
			bForce)
		{
			pDC = GetDC();
				
				if(dwActive[i] & PA_ACTIVE)
					m_AnimInfos[i].m_ActiveFlagsWnd.SetColor(RGB(255,0,0));
				else
					m_AnimInfos[i].m_ActiveFlagsWnd.SetColor(RGB(0,0,0));
			
			ReleaseDC(pDC);
		}
	}
	
	// Remember the last state..
	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		m_LastAnimActiveFlags[i] = dwActive[i];
	}
}

// ------------------------------------------------------------------------
// IncKeyframe( bForward )
// increment the frame forward (or backwards...).
// ------------------------------------------------------------------------
void CModelEditDlg::IncKeyframe(BOOL bForward)
{
	DWORD dwActive[NUM_ANIM_INFOS];
	DWORD i, iKeyframe;
	MEAnimInfo *pInfo;

	// determine which anims are active & valid
	GetPlaybackActiveFlags(dwActive);


	for(i=0; i < NUM_ANIM_INFOS; i++)
	{
		pInfo = GetAnimInfo(i);

		// if the animation slot is active AND valid 
		if( (dwActive[i] & (PA_ACTIVE | PA_VALID)) == (PA_ACTIVE | PA_VALID ))
		{
			if(bForward)
			{
				iKeyframe = (pInfo->m_Tracker.m_TimeRef.m_Cur.m_iFrame + 1) % pInfo->CurAnim()->NumKeyframes();
			}
			else
			{
				if(pInfo->m_Tracker.m_TimeRef.m_Cur.m_iFrame == 0)
				{
					if( pInfo->CurAnim()->NumKeyFrames() == 0 )
						iKeyframe = 0 ;
					else {
						iKeyframe = (int)(pInfo->CurAnim()->NumKeyframes() - 1);
					}
					//iKeyframe = pInfo->CurAnim()->NumKeyframes() == 0 ? 0 : (int)(pInfo->CurAnim()->NumKeyframes() - 1);
				}else
					iKeyframe = (int)(pInfo->m_Tracker.m_TimeRef.m_Cur.m_iFrame - 1);
			}

			SetCurrentPosition(i, (DWORD)iKeyframe, (DWORD)iKeyframe, 0.0f);
			pInfo->SetKeyframeWindowTime();
		}
	}
}


void CModelEditDlg::OnOK()
{
	// Forward the enter key to the two edit controls on the dialog 
	//	(Note: Gross hack because MFC uses the enter key for "OK" in dialog-based apps)
	if (GetFocus() == &m_EditFrameTime)
		SetEditFrameTime( );
	if (GetFocus() == &m_EditFrameString)
		SetEditFrameString( );
}


BOOL CModelEditDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	DWORD i;
	MEAnimInfo *pInfo;
	ModelAnim *pAnim;


	if(m_pModel)
	{
		for(i=0; i < NUM_ANIM_INFOS; i++)
		{
			pInfo = GetAnimInfo(i);

			if(LOWORD(wParam) == pInfo->m_AnimButtonID)
			{
				if(pInfo->m_Wnd.IsActive())
				{
					WeightSetSelectDlg dlg(m_pModel, pInfo->CurAnim()->m_ModelEditWeightSet);

					if(dlg.DoModal() == IDOK)
					{
						// Set it up..
						if(pAnim = pInfo->CurAnim())
						{
							if(pAnim->m_ModelEditWeightSet != dlg.m_SelectedSet)
							{
								pAnim->m_ModelEditWeightSet = dlg.m_SelectedSet;
								SetChangesMade();
							}
						}

						// Make it update the trackers..
						OnSelChangeAnimList();
					}
				}
				
				break;
			}
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}


void CModelEditDlg::OnFrameStringChange()
{
	if(m_bInStringChange)
		return;

	TempVarSetter<BOOL> setter(&m_bInStringChange, TRUE);

	SetEditFrameString();
}


void CModelEditDlg::OnFrameTimeChange()
{
	if(m_bInStringChange)
		return;

	TempVarSetter<BOOL> setter(&m_bInStringChange, TRUE);

	SetEditFrameTime();
}


void CModelEditDlg::OnNewBgColor() 
{
	extern void SetGLMBackgroundColor( GLM_CONTEXT hContext, COLORREF win_color );

	// TODO: Add your command handler code here
		// TODO: Add your command handler code here
	CColorDialog colorPicker ;
	colorPicker.DoModal();
	COLORREF ms_color = colorPicker.GetColor();

	CRegMgr regMgr;
	if (regMgr.Init(szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Render", HKEY_CURRENT_USER))
	{
		UINT32 dwSize = sizeof(float)*3;
		unsigned char *pColDecomp = (unsigned char*)&ms_color;
		float bgColor[3];
		// unitize
		bgColor[0] = pColDecomp[0]/256.0f;
		bgColor[1] = pColDecomp[1]/256.0f;
		bgColor[2] = pColDecomp[2]/256.0f;
		regMgr.Set("bgColor",bgColor , dwSize);

	}
	// set the bg color for m_RenderWnd->GLMContext.
	m_RenderWnd.SetBackgroundColor( ms_color );
}


void CModelEditDlg::OnLoadConfirm()
{
	m_bConfirmLoadDlg = !m_bConfirmLoadDlg;

	// store this setting in the registry
	CRegMgr regMgr;
	if( regMgr.Init( szRegKeyCompany, szRegKeyApp, szRegKeyVer, "Options", HKEY_CURRENT_USER ) )
	{
		regMgr.Set( "confirm_load", m_bConfirmLoadDlg ? 1 : 0 );
	}

	UpdateMenuChecks();
}


void CModelEditDlg::OnOptionsShoworiginalmodel() 
{
		m_RenderWnd.m_DrawStruct.m_bDrawOriginalModel = !m_RenderWnd.m_DrawStruct.m_bDrawOriginalModel ;
	UpdateMenuChecks();	
}


void CModelEditDlg::OnChangeFov() 
{
	// TODO: Add your command handler code here
	m_RenderWnd.OpenFOVDlg();	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnExportModelStringKeys()
//
//   PURPOSE  : Exports string keys from a given model
//
//------------------------------------------------------------------

void CModelEditDlg::OnExportModelStringKeys()
{
	if (!m_pModel)
	{
		AfxMessageBox("No model currently loaded !!");

		return;
	}

	char szFilter[] = "MSK Files (*.msk)|*.msk|All Files (*.*)|*.*||";
	CFileDialog dlg(FALSE, "msk", NULL, OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);

	if (dlg.DoModal() == IDOK)
	{
		CString sFile = dlg.GetPathName();

		FILE *fp = fopen(sFile, "wb");
		if (!fp)
		{
			CString sError;
			sError.Format("Error opening/creating %s", sFile);
			AfxMessageBox(sError, MB_ICONEXCLAMATION);

			return;
		}

		// Write out the user radius for this model

		float fRadius = m_pModel->m_GlobalRadius;
		fwrite(&fRadius, sizeof(float), 1, fp);

		// Write out the number of animations

		int nAnims = m_pModel->NumAnims();
		fwrite(&nAnims, sizeof(int), 1, fp);
		
		for (int i = 0; i < nAnims; i ++)
		{
			ModelAnim *pAnim = m_pModel->GetAnim(i);

			// Write out the name of the animation
			
			char *sName = pAnim->GetName();				
			int nLen = strlen(sName);

			// Write out the total length of the animation

			uint32 nAnimLen = pAnim->GetAnimTime();
			float fRatio = 1.0f;
			if (nAnimLen)
			{
				fRatio = 1.0f / (float)nAnimLen;
			}
			
			fwrite(&nLen, sizeof(int), 1, fp);
			fwrite(sName, nLen, 1, fp);

			// Write out the dims of the animation

			AnimInfo *pInfo = m_pModel->GetAnimInfo(i);
			float fDimsX = pInfo->m_vDims.x;
			float fDimsY = pInfo->m_vDims.y;
			float fDimsZ = pInfo->m_vDims.z;

			fwrite(&fDimsX, sizeof(float), 1, fp);
			fwrite(&fDimsY, sizeof(float), 1, fp);
			fwrite(&fDimsZ, sizeof(float), 1, fp);

			
			// Write out all the string keys

			for (int j = 0; j < (int)pAnim->NumKeyFrames(); j ++)
			{
				AnimKeyFrame *pKey = &pAnim->m_KeyFrames[j];

				if ((pKey->m_pString) && (strcmp(pKey->m_pString, "")))
				{
					// Write out marker
					
					char bMarker = 1;
					fwrite(&bMarker, sizeof(char), 1, fp);

					// Write out time at which this string occurs

					float fTime = (float)pKey->m_Time * fRatio;
					fwrite(&fTime, sizeof(uint32), 1, fp);

					// Write out the string itself

					nLen = strlen(pKey->m_pString);
					fwrite(&nLen, sizeof(int), 1, fp);
					fwrite(pKey->m_pString, nLen, 1, fp);
				}
			}

			// Write out the end marker

			char nDone = -1;
			fwrite(&nDone, sizeof(char), 1, fp);
		}

		fclose(fp);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnImportModelStringKeys()
//
//   PURPOSE  : Imports model string keys from a .msk file
//
//------------------------------------------------------------------
void CModelEditDlg::OnImportModelStringKeys()
{
	if (!m_pModel)
	{
		AfxMessageBox("No model currently loaded !!");

		return;
	}

	char szFilter[] = "MSK Files (*.msk)|*.msk|All Files (*.*)|*.*||";
	CFileDialog dlg(TRUE, "msk", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);

	if (dlg.DoModal() == IDOK)
	{
		CString sFile = dlg.GetPathName();

		char sTmp[1024];
		
		FILE *fp = fopen(sFile, "rb");
		if (!fp)
		{
			CString sError;
			sError.Format("Error opening/creating %s", sFile);
			AfxMessageBox(sError, MB_ICONEXCLAMATION);

			return;
		}

		CEdit *pEdit = (CEdit *)m_pImportStringKeyDlg->GetDlgItem(IDC_PROGRESS);
		pEdit->SetWindowText("");
		m_pImportStringKeyDlg->ShowWindow(SW_SHOW);

		// Read in the global radius

		float fRadius;
		fread(&fRadius, sizeof(float), 1, fp);

		sprintf(sTmp, "Setting Global Radius to %4.2f\n", fRadius);
		m_pImportStringKeyDlg->AddMsg(sTmp, 255, 0, 0);

		m_pModel->m_GlobalRadius = fRadius;

		// Read in the number of animations....

		int nAnims;
		fread(&nAnims, sizeof(int), 1, fp);

		for (int i = 0; i < nAnims; i ++)
		{
			// Read in the name of this animation....

			int nLen;
			char sAnimName[4096];

			fread(&nLen, sizeof(int), 1, fp);
			fread(sAnimName, nLen, 1, fp);
			sAnimName[nLen] = 0;

			// Read in the dims of the animation

			float fDimsX;
			float fDimsY;
			float fDimsZ;

			fread(&fDimsX, sizeof(float), 1, fp);
			fread(&fDimsY, sizeof(float), 1, fp);
			fread(&fDimsZ, sizeof(float), 1, fp);
			
			// Attempt to locate this animation....

			ModelAnim *pAnim = m_pModel->FindAnim(sAnimName);
			if (pAnim)
			{
				unsigned int dwIndex;
				AnimInfo *pInfo = m_pModel->FindAnimInfo(sAnimName, m_pModel, &dwIndex);

				if (pInfo)
				{
					sprintf(sTmp, "Setting Animation [%s] Dims to [%f4.2, %f4.2, %f4.2]\n", sAnimName, fDimsX, fDimsY, fDimsZ);
					m_pImportStringKeyDlg->AddMsg(sTmp, 255, 0, 0);

					// Set the dims...

					pInfo->m_vDims.x = fDimsX;
					pInfo->m_vDims.y = fDimsY;
					pInfo->m_vDims.z = fDimsZ;
				}
			}

			// Read the model keys....

			bool bReading = true;

			while (bReading)
			{
				char bMarker;
				fread(&bMarker, sizeof(char), 1, fp);

				if (bMarker == -1)
				{
					// End of animation marker
					
					bReading = false;
				}
				else
				{
					char sName[2048];

					// Read the keyframe time...

					float fTime;
					fread(&fTime, sizeof(uint32), 1, fp);
					
					// Read the string key...
					
					fread(&nLen, sizeof(int), 1, fp);
					fread(sName, nLen, 1, fp);
					sName[nLen] = 0;

					if (pAnim)
					{
						int nTime = (int)(fTime * (float)pAnim->GetAnimTime());

						// Attempt to locate the correct keyframe....

						for (int j = 0; j < (int)pAnim->NumKeyFrames(); j ++)
						{
							AnimKeyFrame *pKey = &pAnim->m_KeyFrames[j];

							if (pKey->m_Time >= (uint32)nTime)
							{
								char sTmp[2048];

								if (pKey->m_Time - nTime < 5)
								{
									sprintf(sTmp, "SUCCESS !! Anim [%s] - Added \"%s\" at exactly %d ms\r\n", pAnim->GetName(), sName, nTime);
									m_pImportStringKeyDlg->AddMsg(sTmp, 0, 0, 0);
								}
								else
								{
									sprintf(sTmp, "WARNING !! Anim [%s] - Added \"%s\" at time %d ms (out by %d ms, could not locate exact keyframe)\r\n", pAnim->GetName(), sName, pKey->m_Time, pKey->m_Time - nTime);
									m_pImportStringKeyDlg->AddMsg(sTmp, 0, 0, 0);
								}
								
								// This is the key, fill out the string....

								pKey->m_pString = m_pModel->AddString(sName);

								j = pAnim->NumKeyFrames();
							}
						}
					}
					else
					{
						sprintf(sTmp, "ERROR !! Anim %s missing for string key \"%s\"\r\n", sAnimName, sName);
						m_pImportStringKeyDlg->AddMsg(sTmp, 0, 0, 0);
					}
				}
			}
		}

		// Close file...

		fclose(fp);

		ModelAnim *pAnim = m_AnimInfos[0].CurAnim();
		if (pAnim)
		{
			int nKeyFrame = m_AnimInfos[0].m_Wnd.ForceNearestKeyframe();

			if (nKeyFrame < (int)pAnim->NumKeyFrames())
			{
				char *sTitle = pAnim->m_KeyFrames[nKeyFrame].m_pString;

				GetDlgItem(IDC_FRAMESTRING)->SetWindowText(sTitle);
			}
		}
	}
}

void CModelEditDlg::OnItemclickNodelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CModelEditDlg::OnOptionsShowVertexWeighting() 
{
		m_RenderWnd.m_DrawStruct.m_bDrawVertexWeights = !m_RenderWnd.m_DrawStruct.m_bDrawVertexWeights;
	UpdateMenuChecks();
}

void CModelEditDlg::OnNodeeditEditobb() 
{
	// find the first selected node 
	if (!m_pModel)
		return;

	int selection = m_NodeList.GetSelectionMark();

	if( selection > -1 )
	{
		ModelNode *pNode = m_pModel->m_FlatNodeList[selection];
		CEditOBBDlg dlg(pNode);

		dlg.DoModal();	
	}
}

void CModelEditDlg::OnNodeEditRename() 
{
	OnRenameNode();
}

void CModelEditDlg::OnNodeEditLookAt()
{
	if( !m_pModel )
		return;

	int selection = m_NodeList.GetSelectionMark();

	if( selection <= -1 )
		return;

	LTMatrix* mat = m_pModel->GetTransform( selection );
	ASSERT( mat );

	LTVector pos;
	mat->GetTranslation( pos );
	pos.z *= -1.0f;

	m_RenderWnd.m_Camera.LookAtPoint( pos );
}

void CModelEditDlg::OnSocketEditLookAt()
{
	if( !m_pModel )
		return;

	int selection = m_SocketList.GetCurSel();

	if( (selection < 0) || (selection >= (int)m_pModel->NumSockets()) )
		return;

	LTMatrix xform;
	ModelSocket* socket = m_pModel->GetSocket( selection );	
	if( !m_pModel->GetSocketTransform( socket, &xform ) )
	{
		ASSERT( 0 );
		return;
	}

	LTVector pos;
	xform.GetTranslation( pos );
	pos.z *= -1.0f;

	m_RenderWnd.m_Camera.LookAtPoint( pos );
}


//------------------------------------------------------------------
//
//   FUNCTION : OnExportWeightSets()
//
//   PURPOSE  : Exports weight sets for this model
//
//------------------------------------------------------------------

void CModelEditDlg::OnExportWeightSets() 
{
	if (!m_pModel) return;

	CFileDialog dlg(FALSE, "", "*.wst");

	if (dlg.DoModal() == IDOK)
	{
		CString sFile = dlg.GetPathName();

		// Create the file and export the weight sets

		CLTAWriter writer;
		CString sName = dlg.GetPathName();

		if (writer.Open(dlg.GetPathName(), false))
		{
			writer.BeginNode();

			for (int i = 0; i < m_pModel->NumWeightSets(); i ++)
			{
				WeightSet *pSet = m_pModel->GetWeightSet(i);

				// Write the weight set

				writer.BeginNode();

				writer.Write(pSet->GetName());

				writer.BeginNode();

				for (int j = 0; j < pSet->m_Weights.GetSize(); j ++)
				{
					CString sTxt;
					sTxt.Format("%4.2f", pSet->m_Weights[j]);

					writer.Write(sTxt);
					writer.BreakLine();
				}

				writer.EndNode();

				writer.EndNode();
			}

			writer.EndNode();
		
			writer.Close();
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnImportWeightSets()
//
//   PURPOSE  : Imports weight sets for this model
//
//------------------------------------------------------------------

void CModelEditDlg::OnImportWeightSets() 
{
	if (!m_pModel) return;

	CFileDialog dlg(TRUE, "*.wst");

	if (dlg.DoModal() == IDOK)
	{
		CLTADefaultAlloc *pAlloc = new CLTADefaultAlloc;

		CLTANode *pRoot = pAlloc->AllocateNode();

		CString sFile = dlg.GetPathName();

		if (CLTANodeReader::LoadEntireFile(sFile, false, pRoot, pAlloc))
		{
			// Read in the weight sets

			if (pRoot->IsList())
			{
				for (int i = 0; i < pRoot->GetNumElements(); i ++)
				{
					CLTANode *pWeightSet = pRoot->GetElement(i);

					for (int k = 0; k < pWeightSet->GetNumElements(); k ++)
					{
						WeightSet *pSet = new WeightSet(m_pModel);
						bool bRead = false;

						// Name should be the first child
						
						CLTANode *pChild = pWeightSet->GetElement(k);

						if (pChild->IsList())
						{
							int nChildren = pChild->GetNumElements();
							
							if (nChildren == 2)
							{
								CLTANode *pName = pChild->GetElement(0);
								if (pName->IsAtom())
								{
									CString sName = pName->GetValue();

									strcpy(pSet->m_Name, (char *)(LPCSTR)sName);

									CLTANode *pWeightList = pChild->GetElement(1);

									if (pWeightList->IsList())
									{
										int nElem = pWeightList->GetNumElements();

										if (nElem != m_pModel->NumNodes())
										{
											// Error....

											AfxMessageBox("Weight list doesn't correspond to node list, cannot import", MB_ICONEXCLAMATION);

											delete pSet;

											pAlloc->FreeNode(pRoot);

											delete pAlloc;

											return;
										}
										
										// Read the weights

										for (int j = 0; j < pWeightList->GetNumElements(); j ++)
										{
											CLTANode *pWeight = pWeightList->GetElement(j);

											if (pWeight->IsAtom())
											{
												float fWeight = (float)atof(pWeight->GetValue());

												// Add the weight to the list

												pSet->m_Weights.Add(fWeight);

												bRead = true;
											}
										}
									}
								}
							}
						}
					
						if (!bRead)
						{
							delete pSet;
						}
						else
						{
							m_pModel->AddWeightSet(pSet);
							SetChangesMade();
						}					
					}				
				}
			}
		}

		pAlloc->FreeNode(pRoot);

		delete pAlloc;
	}
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



