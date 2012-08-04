// RegionDoc.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "regiondoc.h"
#include "edithelpers.h"
#include "propertiesdlg.h"
#include "resource.h"
#include "projectbar.h"
#include "regionview.h"
#include "mainfrm.h"
#include "propertyhelpers.h"
#include "edit_actions.h"
#include "deditinternal.h"
#include "ExportObjFile.h"
#include "LoadLTADialog.h"
#include "LightMapDefs.h"
#include "ltamgr.h"
#include "CObjInterface.h"
#include "geomroutines.h"
#include "edithelpers.h"
#include "optionsmisc.h"
#include "levelerrordlg.h"
#include "optionsrun.h"
#include "draw_d3d.h"
#include "levelitemsdlg.h"
#include "objectsearchdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRegionDoc

IMPLEMENT_DYNCREATE(CRegionDoc, CDocument)

CRegionDoc::CRegionDoc() 
{
	m_fSavedSplitCenterX	= 0.5f;
	m_fSavedSplitCenterY	= 0.5f;

	m_TaggedPolies.SetCacheSize(4096);

	m_RegionStringHolder.SetAllocSize( 1000 );

	m_Region.m_pStringHolder = &m_RegionStringHolder;
	m_bCanSynchronize		= false;

	// Grab maximum undo buffer size out of registry
	int nUndos = (uint32)GetApp()->GetOptions().GetMiscOptions()->GetNumUndos();

	m_UndoMgr.Init(this, nUndos, nUndos);

	m_TaggedPolies.SetSize(1);

	m_TaggedVerts.Init(
		64,
		&m_TaggedVertsHelper);
}


CRegionDoc::~CRegionDoc()
{
	//see if we need to delete the files on close -JohnO
	if(GetApp()->GetOptions().DeleteOnClose())
	{
		DeleteFile(m_BackupName);
	}

	//let anything that may hold onto references into this level know about changes
	::GetLevelErrorDlg()->NotifyDocumentClosed(this);
	::GetLevelItemsDlg()->NotifyDocumentClosed(this);
	::GetObjectSearchDlg()->NotifyDocumentClosed(this);

	GetProjectBar()->RemoveRegionDoc(this);
	GetPropertiesDlg()->Term();
}



// ---------------------------------------------------------- //
// Initializes the properties dialog based on the current selections.
// ---------------------------------------------------------- //

void CRegionDoc::SetupPropertiesDlg( bool bShow )
{
	char		typeName[256];
	CEditRegion	*pRegion = GetRegion();
	CPropList	*pList;

	
	if(pRegion->m_Selections == 0)
	{
		GetPropertiesDlg()->Term();
		return;
	}
	else if(pRegion->m_Selections == 1)
	{
		strcpy(typeName, pRegion->m_Selections[0]->GetClassName());
	}
	else
	{
		strcpy(typeName, "N/A");
	}

	pList = CreateMainPropertyList(this);
	GetPropertiesDlg()->Init(pList, this, typeName);
	
	// TODO fix this code to show the properties dialog now that it
	// doesn't necessarily exist in the project bar.
	/*
	if(bShow)
		GetProjectBar()->SetTab( CProjectBar::TAB_PROPERTIESVIEW );
	*/
}			



void CRegionDoc::NotifyPropertiesChange( bool bUpdateView )
{
	// Setup undos.
	SetupUndoForSelections();
	
	ReadPropertiesIntoSelections(this);
	if( bUpdateView )
		RedrawAllViews( );
}


void CRegionDoc::NotifyPropertyChange( CBaseProp *pProp, bool bUpdateView )
{
	// Setup undos.
	SetupUndoForSelections();
	
	ReadPropertyIntoSelections( this, pProp );
	if( bUpdateView )
		RedrawAllViews( );
}


BOOL CRegionDoc::SaveModified()
{
	int			status;
	CString		prompt;

	if( IsModified() )
	{	
		AfxFormatString1( prompt, AFX_IDP_ASK_TO_SAVE, (LPCTSTR)m_WorldName );
		status = AfxMessageBox( prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE );

		if( status == IDYES )
		{
			//! if( !SaveRegion(TRUE))
			if( !SaveLTA(TRUE))
				return FALSE;

			SetModifiedFlag( FALSE );
			SetTitle(false);
		}
		else if( status == IDCANCEL )
		{
			return FALSE;
		}
	}

	return TRUE;
}


bool CRegionDoc::SaveLTA(bool bFullSave)
{
	BeginWaitCursor();

	// make sure the extension is .lta

	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( LPCTSTR(m_FileName), NULL, dir, fname, ext );

	bool bBinary = false;

	if(stricmp(ext, ".tbw") == 0)
		bBinary = true;

	//we want to see if the file exists and is read only. If so, we should prompt the user
	//to see if they want to make it read only (thus to combat source safe...)
	CFileStatus Status;
	if(CFile::GetStatus(m_FileName, Status))
	{
		if(Status.m_attribute & CFile::readOnly)
		{
			//we have a read only file, prompt the user
			CString sMsg;
			sMsg.Format(IDS_SAVETOREADONLYFILE, m_FileName);
			if(MessageBox(GetApp()->GetMainWnd()->m_hWnd, sMsg, "Make file writable?", MB_ICONQUESTION | MB_YESNO) == IDNO)
			{
				//they didn't want to make the file writable, bail
				return false;
			}

			//ok, make the file writable
			Status.m_attribute &= ~CFile::readOnly;
			CFile::SetStatus(m_FileName, Status);
		}
	}

	BOOL bBackupWorked	= FALSE;
	BOOL bFileSaved		= FALSE;

	clock_t	SaveTimeStart = clock();

	if(bBinary)
	{
		CMoFileIO OutFile;
		if(OutFile.Open(LPCTSTR(m_BackupName), "wb"))
		{
			//able to open up the backup name, so now save it
			m_Region.SaveTBW( OutFile );
			OutFile.Close();

			bFileSaved		= TRUE;
			bBackupWorked	= TRUE;
		}
	}
	else
	{
		CLTAFile OutFile(	LPCTSTR(m_BackupName), false, 
							CLTAUtil::IsFileCompressed(LPCTSTR(m_FileName)));

		if( OutFile.IsValid() )
		{
			//able to open up the backup name, so now save it
			m_Region.SaveLTA( &OutFile );
			if(OutFile.Close())
			{
				bFileSaved		= TRUE;
				bBackupWorked	= TRUE;
			}
		}
	}

	if(!bBackupWorked)
	{
		//failed to open the backup, so try saving directly to the
		//desired output file -JohnO
		if(bFullSave)
		{
			if(bBinary)
			{
				CMoFileIO OutFile;
				if(OutFile.Open(LPCTSTR(m_FileName), "wb"))
				{
					//able to open up the backup name, so now save it
					m_Region.SaveTBW( OutFile );
					OutFile.Close();
					
					bFileSaved = TRUE;
				}
			}
			else
			{
				CLTAFile OutFile(	LPCTSTR(m_FileName), false, 
									CLTAUtil::IsFileCompressed(LPCTSTR(m_FileName)));

				if(OutFile.IsValid())
				{
					m_Region.SaveLTA( &OutFile );
					
					if(OutFile.Close())
					{
						bFileSaved = TRUE;
					}
				}
			}
		}
	}

	//make sure that the file was loaded, or throw an error if it wasn't
	if(!bFileSaved)
	{
		CString error;
		error.FormatMessage( (bFullSave) ?	IDS_ERROR_SAVING_FILE : 
											IDS_ERROR_SAVING_BACKUP, 
							 m_BackupName );
		AfxMessageBox( error );
		return FALSE;
	}

	//Save the main file if the backup worked properly (if it didn't, we don't
	//need to copy it over)
	if(bFullSave && bBackupWorked)
	{
		FILE* pFile;
		BOOL bCopied = FALSE;

		if( pFile = fopen(LPCTSTR(m_FileName), "wt") )
		{
			fclose(pFile);
			pFile = NULL;
			bCopied = CopyFile(m_BackupName,m_FileName,FALSE);
		}

		if(!bCopied)
		{
			CString error;

			error.FormatMessage( IDS_ERROR_SAVING_FILE, m_FileName );
			AfxMessageBox( error );
			return FALSE;
		}
	}

	EndWaitCursor();

	return TRUE;
}


bool CRegionDoc::SaveOBJ(bool bFullSave)
{
	//! SaveRegion(bool bFullSave);
	// load the saved ed file into scene
	// save scene as obj

	bool retVal = FALSE;

	SaveLTA(bFullSave);
	CObjInterface objInterface;
	// fill in objInterface with data from the saved ed file

	CEditRegion		region;
	CMoFileIO		file;

	CExportObjFile  exportObjFileDlg;
	exportObjFileDlg.DoModal();

	if( exportObjFileDlg.m_isOK )
	{
		BeginWaitCursor( ); // Put the wait cursor on the screen

		if(file.Open(m_FileName, "rb"))
		{
			uint32 nFileVersion;
			bool bBinary = false;

			if( REGIONLOAD_OK == region.LoadFile(m_FileName, NULL, nFileVersion, bBinary) )
			{
				// printf("\n( p \"loaded ed file: %s\");", edfilename);
				// printf("\n( p \"num polies: %d\");", region.GetTotalNumPolies());
				// printf("\n( p \"num points: %d\");", region.GetTotalNumPoints());
				
				CWorldNode *pRoot = region.GetRootNode();
				CMoArray<CWorldNode*> nodeArray;

				if( pRoot )
				{
					region.FlattenTreeToArray(pRoot, nodeArray);
					int brushCount = 0;
					int i = 0;

					for( i = 0; i < nodeArray.GetSize(); i++ )
					{
						if( Node_Brush == nodeArray[i]->GetType() )
						{
							if( (exportObjFileDlg.m_exportHiddenBrushes) || 
								!(NODEFLAG_HIDDEN &  nodeArray[i]->GetFlags()) )
							{
								CEditBrush*	pBrush = nodeArray[i]->AsBrush();
								int j = 0;
								int k = 0;
								for( j = 0; j < pBrush->m_Points.GetSize(); j++ )
								{
									// printf("\nv  %f %f %f", pBrush->m_Points[j][0],pBrush->m_Points[j][1],pBrush->m_Points[j][2]);
									if(exportObjFileDlg.m_exportFor3DSMax)
									{
										LTVector v(pBrush->m_Points[j][0],pBrush->m_Points[j][2],pBrush->m_Points[j][1]);
										objInterface.m_vertexList.Append(v);
									}
									else
									{
										LTVector v(pBrush->m_Points[j][0],pBrush->m_Points[j][2],pBrush->m_Points[j][1]);
										objInterface.m_vertexList.Append(v);
									}
								}
							}
						}
				
					}

					// printf("\ng object");
					int vertFound = 1;
					for( i = 0, vertFound = 1; i < nodeArray.GetSize(); i++ )
					{
						if( Node_Brush == nodeArray[i]->GetType() )
						{
							if( (exportObjFileDlg.m_exportHiddenBrushes) || 
								!(NODEFLAG_HIDDEN &  nodeArray[i]->GetFlags()) )
							{
								CEditBrush*			pBrush = nodeArray[i]->AsBrush();
							
								int j = 0;
								int k = 0;

								for( j = 0; j < pBrush->m_Polies.GetSize(); j++ )
								{
									
									// printf("\nf  ");
									CMoArray< int > iv;

									for( k = 0; k < pBrush->m_Polies[j]->m_Indices.GetSize(); k++ )
									{
										// printf("%d ", vertFound+(pBrush->m_Polies[j]->m_Indices[k]) );
										iv.Append(vertFound+(pBrush->m_Polies[j]->m_Indices[k]));
									}

									objInterface.m_faceSetList.Append(iv);
								}
								for( j = 0; j < pBrush->m_Points.GetSize(); j++ )
								{
									vertFound++;
								}
							}
						}
					}
				}
			}
			else
			{
				// printf("\n( p \"error: can't load ed file: %s\");", edfilename);
				retVal = FALSE;
			}
		}


		char	pathName[256], fileName[256];
		char	worldName[256], extension[256], backupName[256];

		CHelpers::ExtractPathAndFileName( m_FileName, pathName, fileName );
		CHelpers::ExtractFileNameAndExtension( fileName, worldName, extension );

		CString objFilename;

		if( stricmp(extension, "obj" ) == 0 )
		{
			objFilename = m_FileName;
		}
		else
		{
			objFilename = m_FileName + ".obj";
		}

		FILE* objFile = fopen(LPCTSTR(objFilename), "wb");
		if(objFile)
		{
			objInterface.ExportObj(objFile, exportObjFileDlg.m_reverseNormals);
			fclose(objFile);
		}

		if( stricmp(extension, "obj" ) == 0 )
		{
			m_FileName = CString(worldName) + ".ed";
		}
		else
		{
			m_FileName;
		}
		SaveLTA(bFullSave);

		EndWaitCursor( ); // End the wait cursor
	}
	return retVal;
}

// -------------------------------------------------------------
//! this is where we will change the call to the processor
//  so that it will handle lta files 
void CRegionDoc::StartPreProcessor()
{
	CString sCommand;

	// Get parameters.
	try
	{
		//! change this call to save an LTA file -> SaveLTA(TRUE)
		//! SaveRegion(TRUE);
		//SaveLTA(TRUE);

		if(SaveModifiedLTA()) // Ask user if he/she wishes to save level, if changed
		{

			// Save and then set the working directory
			char szWorkingDirectory[MAX_PATH];
			GetCurrentDirectory(sizeof(szWorkingDirectory), (char *)&szWorkingDirectory);
			SetCurrentDirectory(GetApp()->m_ExeDirectory);

			sCommand.Format("\"%swinpacker\" -file \"%s\" -ProjectPath \"%s\"", GetApp()->m_ExeDirectory, GetPathName(), GetProject()->GetBaseProjectDir());

			if( WinExec(sCommand, SW_SHOW) <= 31 )
				AppMessageBox( IDS_ERRORRUNNINGPREPROCESSOR, MB_OK );  

			// Reset the current working directory
			SetCurrentDirectory(szWorkingDirectory);
		}
	}
	catch( CMemoryException * x )
	{
		AppMessageBox( IDS_OUTOFMEMORY_SAVING, MB_OK );
		x->Delete();
	}
}


CRegionView* CRegionDoc::CreateNewRegionView()
{
	CDocTemplate	*pDocTemp = GetApp()->m_pWorldTemplate;
	CFrameWnd		*pFrameWnd;
	
	pFrameWnd = pDocTemp->CreateNewFrame( this, NULL );
	pDocTemp->InitialUpdateFrame( pFrameWnd, this, TRUE );
	return (CRegionView*)pFrameWnd->GetActiveView();
}

bool CRegionDoc::LoadLTAIntoRegion( const char* filename, bool bForceUpdateObjects, bool bAllowPropSync)
{
	// RegionLoadStatus		LoadLTA( char* filename, CEditProjectMgr* pProject=NULL );
	// SetModifiedFlag( FALSE );
	// NotifySelectionChange();

	CLoadLTADialog LoadDlg;
	LoadDlg.m_sFileName = filename;

	POSITION FirstPos = GetFirstViewPosition();
	CView* pView = GetNextView(FirstPos);

	LoadDlg.Create(MAKEINTRESOURCE(IDD_LTALOADING), (CWnd*)pView);
	LoadDlg.RedrawWindow();

	uint32 nFileVersion;
	bool bBinary = false;
	if( m_Region.LoadFile((char*)filename, GetProject(), nFileVersion, bBinary) != REGIONLOAD_OK )
		return false;

	m_Region.GetPrefabMgr()->SetRootPath(dfm_GetBaseDir(GetFileMgr()));
	m_Region.GetPrefabMgr()->BindAllRefs(m_Region.GetRootNode());

	bool bModified = false;
	if(!bBinary)
		m_Region.PostLoadUpdateVersion(nFileVersion, bModified);

	if(bModified)
		SetModifiedFlag(TRUE);


	ASSERT(!m_Region.CheckNodes());
	
	// Update all the texture IDs and polygon pieces.
	m_Region.UpdateTextureIDs( GetProject() );

	//if we are allowed to sync up properties, go ahead and do so
	if(bAllowPropSync)
		SynchronizePropertyLists(&m_Region, bForceUpdateObjects);

	//initialize all of our selection data
	NotifySelectionChange();

	LoadDlg.DestroyWindow();

	return true;
}


void CRegionDoc::Modify(CPreAction *pAction, bool bDeleteAction)
{
	if(!IsModified())
	{
		SetModifiedFlag( TRUE );
		SetTitle(true);
	}

	if(pAction)
	{
		m_UndoMgr.PreAction(pAction);

		if(bDeleteAction)
			delete pAction;
	}
}


void CRegionDoc::Modify(PreActionList *pActionList, bool bDeleteList)
{
	if(!pActionList || pActionList->GetSize() == 0)
		return;

	if(!IsModified())
	{
		SetModifiedFlag( TRUE );
		SetTitle(true);
	}

	m_UndoMgr.PreActions(pActionList, bDeleteList);
}


void CRegionDoc::SetupUndoForSelections(bool bOnlyOfType, int type)
{
	PreActionList actionList;
	DWORD i;

	for(i=0; i < m_Region.m_Selections; i++)
	{
		if(bOnlyOfType && m_Region.m_Selections[i]->GetType() == type)
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, m_Region.m_Selections[i]));
		}
		else
		{
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, m_Region.m_Selections[i]));
		}
	}

	Modify(&actionList, TRUE);
}


void CRegionDoc::RedrawAllViews( DWORD extraHint )
{
	if(extraHint)
	{
		UpdateAllViews( NULL, extraHint );
	}
	else
	{
		UpdateAllViews( NULL, REGIONVIEWUPDATE_REDRAW );
	}
}

//invalidates all perspective views of this document causing them to
//be redrawn. This should be done for texture operations, and other
//things which will not be reflected in wireframe modes
void CRegionDoc::RedrawPerspectiveViews(DWORD extraHint)
{
	POSITION Pos = GetFirstViewPosition();

	//run through all views of this document
	CView* pView;
	while(pView = GetNextView(Pos))
	{
		CRegionView* pRegionView = (CRegionView*)pView;

		//see if this is a perspective view
		if(pRegionView->GetViewMode() == VM_PERSPECTIVE)
		{
			//it is, so trigger the update
			pRegionView->OnUpdate(NULL, (extraHint) ? extraHint : REGIONVIEWUPDATE_REDRAW, NULL);
		}
	}
}


void CRegionDoc::UpdateSelectionBox( LPARAM extraHint )
{
	DWORD			i, k;
	CWorldNode		*pNode;
	CEditBrush		*pBrush;
	CBaseEditObj    *pObject;

	//used to count up the type of each item selected
	uint32 nNumSelObjects	= 0;
	uint32 nNumSelBrushes	= 0;
	uint32 nNumSelOther		= 0;
	uint32 nNumSelPrefabs	= 0;

	uint32 nNumSelVerts		= 0;
	uint32 nNumSelPolys		= 0;

	m_SelectionMin.Init( (CReal)MAX_CREAL, (CReal)MAX_CREAL, (CReal)MAX_CREAL );
	m_SelectionMax.Init( (CReal)-MAX_CREAL, (CReal)-MAX_CREAL, (CReal)-MAX_CREAL );

	for( i=0; i < m_Region.m_Selections; i++ )
	{
		pNode = m_Region.m_Selections[i];
		if(pNode->GetType() == Node_Brush)
		{
			nNumSelBrushes++;

			pBrush = pNode->AsBrush();

			for( k=0; k < pBrush->m_Points; k++ )
			{
				LTVector vPt = pBrush->m_Points[k];

				if( vPt.x < m_SelectionMin.x )	m_SelectionMin.x = vPt.x;
				if( vPt.y < m_SelectionMin.y )	m_SelectionMin.y = vPt.y;
				if( vPt.z < m_SelectionMin.z )	m_SelectionMin.z = vPt.z;

				if( vPt.x > m_SelectionMax.x )	m_SelectionMax.x = vPt.x;
				if( vPt.y > m_SelectionMax.y )	m_SelectionMax.y = vPt.y;
				if( vPt.z > m_SelectionMax.z )	m_SelectionMax.z = vPt.z;
			}

			nNumSelVerts += pBrush->m_Points.GetSize();
			nNumSelPolys += pBrush->m_Polies.GetSize();
		}
		else if(pNode->GetType() == Node_Object)
		{
			nNumSelObjects++;

			// Don't contribute to the selection box if not in Object Mode
			//if (m_pView->GetEditMode() == OBJECT_EDITMODE) {

			pObject = pNode->AsObject();

			//Sometimes the object can get here without having properly initialized dimensions
			//so if that is the case, it should update its dimensions
			if(pObject->ShouldSearchForDims())
			{
				pObject->InitDims();
			}

			char debugout[512];

			if (pObject->GetNumDims() != 0)  // check if object has dims
			{
				LTVector* dimV = pObject->GetDim(0);

				ASSERT(dimV != NULL); // bad!

				LTVector vPt = pObject->GetPos();

				if( vPt.x - fabs(dimV->x) < m_SelectionMin.x )	m_SelectionMin.x = vPt.x - fabs(dimV->x);
				if( vPt.y - fabs(dimV->y) < m_SelectionMin.y )	m_SelectionMin.y = vPt.y - fabs(dimV->y);
				if( vPt.z - fabs(dimV->z) < m_SelectionMin.z )	m_SelectionMin.z = vPt.z - fabs(dimV->z);

				if( vPt.x + fabs(dimV->x) > m_SelectionMax.x )	m_SelectionMax.x = vPt.x + fabs(dimV->x);
				if( vPt.y + fabs(dimV->y) > m_SelectionMax.y )	m_SelectionMax.y = vPt.y + fabs(dimV->y);
				if( vPt.z + fabs(dimV->z) > m_SelectionMax.z )	m_SelectionMax.z = vPt.z + fabs(dimV->z);
			} 
			else // non-dimmed object
			{
				LTVector vPt = pObject->GetPos();

				if( vPt.x < m_SelectionMin.x )	m_SelectionMin.x = vPt.x;
				if( vPt.y < m_SelectionMin.y )	m_SelectionMin.y = vPt.y;
				if( vPt.z < m_SelectionMin.z )	m_SelectionMin.z = vPt.z;

				if( vPt.x > m_SelectionMax.x )	m_SelectionMax.x = vPt.x;
				if( vPt.y > m_SelectionMax.y )	m_SelectionMax.y = vPt.y;
				if( vPt.z > m_SelectionMax.z )	m_SelectionMax.z = vPt.z;
			}
		}
		else if(pNode->GetType() == Node_PrefabRef)
		{
			++nNumSelPrefabs;
			CPrefabRef *pPrefab = (CPrefabRef*)pNode;

			// Get the raw dimensions of the object
			LTMatrix mPrefabTrans;
			gr_SetupMatrixEuler(pPrefab->GetOr(), mPrefabTrans.m);

			LTVector vUp, vRight, vForward;
			mPrefabTrans.GetBasisVectors(&vRight, &vUp, &vForward);

			// Transform the dimensions into world-space
			LTVector vMinDims;
			LTVector vPrefabMin = pPrefab->GetPrefabMin();
			vMinDims.x = vPrefabMin.x * vRight.x + vPrefabMin.y * vUp.x + vPrefabMin.z * vForward.x;
			vMinDims.y = vPrefabMin.x * vRight.y + vPrefabMin.y * vUp.y + vPrefabMin.z * vForward.y;
			vMinDims.z = vPrefabMin.x * vRight.z + vPrefabMin.y * vUp.z + vPrefabMin.z * vForward.z;

			LTVector vMaxDims;
			LTVector vPrefabMax = pPrefab->GetPrefabMax();
			vMaxDims.x = vPrefabMax.x * vRight.x + vPrefabMax.y * vUp.x + vPrefabMax.z * vForward.x;
			vMaxDims.y = vPrefabMax.x * vRight.y + vPrefabMax.y * vUp.y + vPrefabMax.z * vForward.y;
			vMaxDims.z = vPrefabMax.x * vRight.z + vPrefabMax.y * vUp.z + vPrefabMax.z * vForward.z;

			// Get the min/max of the bounding box
			LTVector vOffset = pPrefab->GetPos();

			vMinDims += vOffset;
			vMaxDims += vOffset;

			LTVector vMin;
			LTVector vMax;

			VEC_MIN(vMin, vMinDims, vMaxDims);
			VEC_MAX(vMax, vMinDims, vMaxDims);

			VEC_MIN(m_SelectionMin, m_SelectionMin, vMin);
			VEC_MAX(m_SelectionMax, m_SelectionMax, vMax);
		}
		else
		{
			nNumSelOther++;
		}
	}	

	// Create the string to be put on the status bar
	CString sBoundingText;
	if (m_Region.m_Selections.GetSize() != 0)
	{		
		sBoundingText.Format("(%1.0f, %1.0f, %1.0f)->(%1.0f, %1.0f, %1.0f) = (%1.0f, %1.0f, %1.0f)",
							 m_SelectionMin.x, m_SelectionMin.y, m_SelectionMin.z,
							 m_SelectionMax.x, m_SelectionMax.y, m_SelectionMax.z,
							 m_SelectionMax.x-m_SelectionMin.x,
							 m_SelectionMax.y-m_SelectionMin.y,
							 m_SelectionMax.z-m_SelectionMin.z);
	}

	// Update the status bar text
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	pFrame->UpdateStatusText(ID_INDICATOR_BOUNDING_BOX, sBoundingText);

	//update the selected items pane if anything is selected -JohnO
	if(nNumSelObjects || nNumSelBrushes || nNumSelPrefabs || nNumSelOther)
	{
		//used for building up the string to be displayed
		CString sSelectionText;
		CString sFormatText;

		//start out with the indicator of the pane
		sSelectionText.Format("Sel: ");

		//add any selected objects to the display
		if(nNumSelObjects)
		{
			sFormatText.Format("Obj: %d ", nNumSelObjects);
			sSelectionText += sFormatText;
		}

		//add any brushes to the display
		if(nNumSelBrushes)
		{
			sFormatText.Format("Brsh: %d (V:%d P:%d)", nNumSelBrushes, nNumSelVerts, nNumSelPolys);
			sSelectionText += sFormatText;
		}

		//add any prefabs to the display
		if(nNumSelPrefabs)
		{
			sFormatText.Format("Pfb: %d ", nNumSelPrefabs);
			sSelectionText += sFormatText;
		}

		//add any miscelanious objects to the display
		if(nNumSelOther)
		{
			sFormatText.Format("Misc: %d", nNumSelOther);
			//don't need misc items yet, but in the future, they 
			//may be needed. Currently misc items include only containers
			//but may be needed later -JohnO
			//sSelectionText += sFormatText;
		}
			
		//print the actual pane
		pFrame->UpdateStatusText(ID_INDICATOR_SELECTION, sSelectionText);
	}
	else
	{
		//no items were selected, so clear out all text from that pane
		pFrame->UpdateStatusText(ID_INDICATOR_SELECTION, "Sel: None");
	}


	RedrawAllViews( extraHint );
}


void CRegionDoc::NotifySelectionChange()
{
	SetupPropertiesDlg(FALSE);
	UpdateSelectionBox();
}

BEGIN_MESSAGE_MAP(CRegionDoc, CDocument)
	//{{AFX_MSG_MAP(CRegionDoc)
		ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
		ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
		ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
		ON_COMMAND(ID_EDIT_CUT, OnEditCut)
		ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
		ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
		ON_UPDATE_COMMAND_UI(ID_EDIT_STAMP, OnUpdateEditStamp)
		ON_COMMAND(ID_EDIT_STAMP, OnEditStamp)
	ON_COMMAND(ID_EDIT_PASTE_ALTERNATE, OnEditPasteAlternate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_ALTERNATE, OnUpdateEditPasteAlternate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegionDoc diagnostics

#ifdef _DEBUG
void CRegionDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRegionDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRegionDoc serialization

void CRegionDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRegionDoc commands

BOOL CRegionDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	
	return TRUE;
}


BOOL CRegionDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	//we want to make sure that the focus is moved out from any of the controls
	//in the properties page so that they can be updated and appropriately saved
	bool bSetViewFocus = true;

	//run through all the views and see if any have the focus
	POSITION posView = GetFirstViewPosition();
	CRegionView *pFirst = (CRegionView*)GetNextView(posView);
	CRegionView *pView = pFirst;
	while(pView)
	{
		if(GetFocus() == pView->m_hWnd)
		{
			bSetViewFocus = false;
			break;
		}
		
		pView = (CRegionView*)GetNextView(posView);
	}
	
	//if there are any views, and none of the views had focus, we want to set the focus
	//to the first view
	if(bSetViewFocus && pFirst)
	{
		pFirst->SetFocus();
	}

	//! if( !SaveRegion(TRUE))
	if( !SaveLTA(TRUE))
		return FALSE;

	SetModifiedFlag( FALSE );

	SetTitle(false);
	
	return 1;
	
	// This was overwriting the file SaveRegion() saved!
	//return CDocument::OnSaveDocument(lpszPathName);
}


//this will setup the documents backup file name information as well as
//open up the specified LTA and load it into the region. This is essentially
//the internals to OnOpenDocument, but are broken out to allow forcing 
//synchronization without breaking the overriding of OnOpenDocument
//from CDocument
bool CRegionDoc::InitDocument(LPCTSTR lpszPathName, bool bForceSync, bool bAllowPropSync)
{
	char	pathName[_MAX_PATH], fileName[_MAX_PATH];
	char	worldName[_MAX_PATH], extension[_MAX_PATH];
	char	ext[_MAX_PATH];

	CHelpers::ExtractPathAndFileName( lpszPathName, pathName, fileName );
	CHelpers::ExtractFileNameAndExtension( fileName, worldName, extension );

	m_WorldName = worldName;
	m_FileName = lpszPathName;
	
	UpdateBackupName(lpszPathName);

	_splitpath( lpszPathName,NULL, NULL, NULL, ext );

	//set the modified flag to be false at first
	SetModifiedFlag( FALSE );

	// Set the new region doc so GetActiveRegion() won't return the old one..

	GetProjectBar()->m_pNewRegionDoc = this;

	if( !LoadLTAIntoRegion(lpszPathName, bForceSync, bAllowPropSync) )
	{
		CString str;
		str.FormatMessage( IDS_ERROR_OPENING_WORLD, worldName );
		AppMessageBox( str, MB_OK );
		
		GetProjectBar()->m_pNewRegionDoc = NULL;

		return false;
	}		

	// Update all the properties in the new LTA...

	CEditRegion	*pRegion = GetRegion();
	if( !pRegion ) return false;

	// First update only the variable declerations...
	
	pRegion->UpdateAllObjectProperties( "VVarDecsOnly; ClearVars;" );

	// Now update our commands to add any dynamic objects...

	pRegion->UpdateAllObjectProperties( "ClearDynaObjs; IMsgErs; IVarCmds;" );

	// Then update everything but the variable declerations...

	pRegion->UpdateAllObjectProperties( "IVarDecs; IAddDynaObj;" );

	GetProjectBar()->m_pNewRegionDoc = NULL;

	return true;
}


BOOL CRegionDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if(!InitDocument(lpszPathName, false, true))
	{
		return FALSE;
	}

	return TRUE;
}



void CRegionDoc::OnCloseDocument() 
{
	// Make sure the properties dialog isn't sitting on an invalid object.
	for( DWORD i=0; i < m_Region.m_Objects; i++ )
		GetPropertiesDlg()->TermIf( &m_Region.m_Objects[i]->m_PropList );

	if( this == GetNodeView( )->GetDoc( ))
		GetNodeView( )->Term( );

	CDocument::OnCloseDocument();
}



void CRegionDoc::OnUpdateEditCopy( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_Region.m_Selections.GetSize( ) != 0 );
}


void CRegionDoc::OnEditCopy()
{
	(( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->Copy( GetRegion() );
}


void CRegionDoc::OnUpdateEditCut( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_Region.m_Selections.GetSize( ) != 0 );
}

void CRegionDoc::OnEditCut()
{
	// copy the brushes
	(( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->Copy( GetRegion() );

	// then delete them
	POSITION pos = GetFirstViewPosition();
	CRegionView* Ctest = (CRegionView*)GetNextView(pos);
	Ctest->OnDelete(NULL);
}


void CRegionDoc::OnUpdateEditPaste( CCmdUI *pCmdUI )
{
	pCmdUI->Enable((( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->IsFull( ));
}


void CRegionDoc::OnEditPaste()
{
	PreActionList actionList;

	(( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->Paste( GetRegion(), actionList, TRUE );

	// Setup the undo for the new nodes it added.
	Modify(&actionList, TRUE);
	NotifySelectionChange();
}



void CRegionDoc::OnUpdateEditStamp( CCmdUI *pCmdUI )
{
	pCmdUI->Enable( m_Region.m_Selections.GetSize( ) != 0 );
}


void CRegionDoc::OnEditStamp()
{
	PreActionList actionList;

	(( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->Copy( GetRegion() );
	(( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->Paste(GetRegion(), actionList, TRUE);
	Modify(&actionList, TRUE);

	NotifySelectionChange();
}




// --------------------------------------------------------------- //
// Goes thru all the nodes and synchronizes their property lists
// with their classes.  Returns FALSE if the user says not to 
// synchronize the lists.
// --------------------------------------------------------------- //

static bool RecurseAndSynchronize(CRegionDoc *pDoc, CEditRegion *pRegion, CWorldNode *pNode)
{
	DWORD			i, j, k, nProps, propIndex;
	CPropList		*pList;
	ClassDef		*pClass;
	CBaseProp		*pProp, *pNewProp;
	PropDef			*pVar;
	char			szDebug[64];

	CWorldNode *pChild;
	GPOS pos;
	CMoArray<PropDef*> *pPropList;


	for(pos=pNode->m_Children; pos; )
	{
		pChild = pNode->m_Children.GetNext(pos);

		if(!RecurseAndSynchronize(pDoc, pRegion, pChild))
			return FALSE;
	}


	pList = &pNode->m_PropList;
	pClass = GetProject()->FindClassDef(pNode->GetClassName());
	
	if(pClass && (pPropList = GetProject()->GetClassDefProps(pClass)))	// If the class exists, sync stuff up.
	{
		// Remove variables that don't exist anymore.
		nProps = pList->m_Props;
		for( j=0; j < nProps; j++ )
		{
			pProp = pList->m_Props[j];
			
			if(!GetProject()->FindPropInList(*pPropList, pProp->m_Name, NULL))
			{
				sprintf(szDebug,"Unknown prop name: %s",pProp->m_Name);
				AddDebugMessage(szDebug);

				// Make sure they want to synchronize things.
				if( !pDoc->m_bCanSynchronize && !pDoc->CheckSynchronize() )
					return FALSE;

				pDoc->SetModifiedFlag();
				delete pList->m_Props[j];
				pList->m_Props.Remove(j);
				--j;
				nProps--;
			}
		}

		// Add variables that are new.
		for(k=0; k < pPropList->GetSize(); k++)
		{
			pVar = pPropList->Get(k);

			pProp = pList->GetProp(pVar->m_PropName, FALSE, &propIndex);
			if(pProp)
			{
				if(pProp->m_Type != pVar->m_PropType)
				{
					sprintf(szDebug,"Unknown prop type: %s",pProp->m_Name);
					AddDebugMessage(szDebug);

					// Make sure they want to synchronize things.
					if( !pDoc->m_bCanSynchronize && !pDoc->CheckSynchronize() )
						return FALSE;
				
					delete pProp;

					pProp = CreatePropFromCode(pVar->m_PropType, pVar->m_PropName);
					if(pProp)
					{
						SetupNewProp(pProp, pVar);
						pDoc->SetModifiedFlag();
						pList->m_Props[propIndex] = pProp;
					}
					else
						pList->m_Props.Remove(propIndex);
				}
				// {BP 1/19/98}
				// Make sure the propflags jive...
				if(pProp->m_PropFlags != pVar->m_PropFlags)
				{
					sprintf(szDebug,"Unknown prop flags: %s",pProp->m_Name);
					AddDebugMessage(szDebug);

					// Make sure they want to synchronize things.
					if( !pDoc->m_bCanSynchronize && !pDoc->CheckSynchronize() )
						return FALSE;
				
					pProp->m_PropFlags = pVar->m_PropFlags;
				}

				// If it's a static list, make sure the value matches something in the list
				DEditInternal* pDEditInternal = pVar->m_pDEditInternal;
				if(pDEditInternal && (pVar->m_PropFlags & PF_STATICLIST))
				{
					_ASSERT(pProp->m_Type == LT_PT_STRING);

					const char* szString = ((CStringProp*)pProp)->m_String;
					bool bMatch = false;
					for ( int iString = 0 ; iString < pDEditInternal->m_cStrings ; iString++ )
					{
						if ( !strcmp(szString, pDEditInternal->m_aszStrings[iString]) )
						{
							bMatch = TRUE;
							break;
						}
					}
					if ( !bMatch )
					{
						sprintf(szDebug,"Unmatchable string for static list: %s",pProp->m_Name);
						AddDebugMessage(szDebug);

						// Make sure they want to synchronize things.
						if( !pDoc->m_bCanSynchronize && !pDoc->CheckSynchronize() )
							return FALSE;

						if( pDEditInternal->m_aszStrings )
 							((CStringProp*)pProp)->SetString( pDEditInternal->m_aszStrings[0] );
 						else
 							((CStringProp*)pProp)->SetString( "" );						
						
						pDoc->SetModifiedFlag();
					}
				}
			}
			else
			{
				// Make sure they want to synchronize things.
				if( !pDoc->m_bCanSynchronize && !pDoc->CheckSynchronize() )
					return FALSE;

				pProp = CreatePropFromCode(pVar->m_PropType, pVar->m_PropName);
				if(pProp)
				{
					SetupNewProp(pProp, pVar);
					pDoc->SetModifiedFlag();
					pList->m_Props.Append(pProp);
				}
			}
		}
	}

	return TRUE;
}


void CRegionDoc::SynchronizePropertyLists( CEditRegion *pRegion, bool bForceSync )
{
	//see if we need to force the updating of objects
	if(bForceSync)
	{
		//by setting this, it makes the function SyncPropLists essentially
		//think that it has already asked the user if they want to update
		//objects and they said yes, which will in turn force synchronization
		//without user input
		m_bCanSynchronize = TRUE;
	}

	// MIKE 12/29/97 - Instead of doing a full Modify, it queues up the changes
	// and sets up the PreActionList.
	//Modify((( CMainFrame * )AfxGetMainWnd( ))->CountUndoProperties( ));
	
	//PreActionList actionList;
	RecurseAndSynchronize(this, pRegion, pRegion->GetRootNode());
	//Modify(&actionList, TRUE);
	
	m_bCanSynchronize = FALSE;
}


bool CRegionDoc::CheckSynchronize()
{
	if( AppMessageBox(IDS_OUTOFDATEOBJECTS, MB_YESNO) == IDYES )
	{
		m_bCanSynchronize = TRUE;
	}
	else
	{
		m_bCanSynchronize = FALSE;
	}

	return m_bCanSynchronize;
}


// Adds an object to the document
CBaseEditObj *CRegionDoc::AddObject(const char *pClassName, CVector vPos)
{
	CBaseEditObj *pObj = new CBaseEditObj;
	if(SetupWorldNode(pClassName, pObj, &m_Region))
	{
		// Get the region
		CEditRegion *pRegion=GetRegion();
		if (!pRegion)
		{
			return NULL;
		}

		// Add the object to the region
		pRegion->AddObject(pObj);
		pRegion->AddNodeToRoot( pObj->AsNode() );

		Modify(new CPreAction(ACTION_ADDEDNODE, pObj), TRUE);

		// Set the objects position
		pObj->SetPos(vPos);

		// Add the object to the node view		
		GetNodeView( )->AddNode( pObj->AsNode() );

		// Return the object
		return pObj;
	}
	else
	{
		delete pObj;
		return NULL;
	}
}

// Binds an object
void CRegionDoc::BindNode(CWorldNode *pChild, CWorldNode *pParent, PreActionList *pActionList)
{
	CEditRegion *pRegion = GetRegion();
	
	if (pActionList)
		pActionList->AddTail(new CPreAction(ACTION_MODIFYNODE, pChild));
	else
		GetNodeView( )->GetDoc( )->Modify(new CPreAction(ACTION_MODIFYNODE, pChild), TRUE );
	pRegion->AttachNode( pChild, pParent );
}

// Binds an object to the selected nodes
void CRegionDoc::BindNodeToSelected(CWorldNode *pNode, PreActionList *pActionList)
{
	CEditRegion *pRegion = GetRegion();

	unsigned int i;
	for(i = 0; i < pRegion->m_Selections.GetSize(); i++)
	{
		CWorldNode *pChild = pRegion->m_Selections[i];

		// Attach node to new parent if it is not a container
		if (pChild->GetType() != Node_Null)
		{
			BindNode(pChild, pNode, pActionList);
		}
	}
}

// Selects a node and updates the views
void CRegionDoc::SelectNode(CWorldNode *pNode, bool bClearSelections)
{
	CEditRegion *pRegion = GetRegion();
	if (!pRegion)
	{
		return;
	}

	if (bClearSelections)
	{
		pRegion->ClearSelections( );
	}

	pRegion->SelectNode(pNode);
	NotifySelectionChange( );
	RedrawAllViews( );
}

/************************************************************************/
// Moves the selected nodes to the indicated parent node.
// This sets up the undo information as well.
void CRegionDoc::MoveSelectedNodes(CWorldNode *pParentNode)
{	
	// Get the region
	CEditRegion *pRegion=GetRegion();
	if (!pRegion)
	{
		return;
	}	

	// Get the selected nodes
	CMoArray<CWorldNode*> selectedNodes;
	selectedNodes.CopyArray(pRegion->m_Selections);
		
	CWorldNode::FindParentNodes(selectedNodes);

	// Setup the undo information
	PreActionList actionList;	
	int i;
	for(i=0; i < selectedNodes.GetSize(); i++)
	{
		// Ignore a selected node if it is also the parent node or is a parent to the parent node
		if ((selectedNodes[i] == pParentNode) || (selectedNodes[i]->FindNodeInChildren(pParentNode)))
		{
			continue;
		}

		actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, selectedNodes[i]));
	}
	Modify(&actionList, TRUE);

	// Attach the topmost nodes to the parent, children will follow...
	for( i = 0; i < selectedNodes.GetSize( ); i++ )
	{
		// Ignore a selected node if it is also the parent node or is a parent to the parent node
		if ((selectedNodes[i] == pParentNode) || (selectedNodes[i]->FindNodeInChildren(pParentNode)))
		{
			if (selectedNodes[i]->FindNodeInChildren(pParentNode))  
			{
				AppMessageBox("Cannot move a parent node under its own child.\nPlease move the child node and try again.", MB_OK | MB_ICONEXCLAMATION );
			}

			continue;
		}

		CWorldNode *pNode = selectedNodes[i];
		GetRegion()->AttachNode( pNode, pParentNode);
	}

	ASSERT(GetRegion()->CheckNodes() == NULL);
}

/************************************************************************/
// Converts an object from one class to another.  A pointer to the object is then returned.
CBaseEditObj *CRegionDoc::ConvertObjectClass(CBaseEditObj *pOldObject, const char *lpszClassName)
{
	// Create the new object at the coordinates of the one being converted
	CBaseEditObj *pNewNode=AddObject(lpszClassName, pOldObject->GetPos());

	// Attach the node to the parent
	BindNode(pNewNode, pOldObject->GetParent());

	// Move the child attachments over to the new node
	GPOS gPos;
	for (gPos=pOldObject->m_Children; gPos; )
	{
		BindNode(pOldObject->m_Children.GetNext(gPos), pNewNode);
	}

	// Get the property lists for both the new and the old object
	CPropList *pNewPropList=pNewNode->GetPropertyList();
	CPropList *pOldPropList=pOldObject->GetPropertyList();

	ASSERT(pNewPropList);
	ASSERT(pOldPropList);

	// Copy the property values from the old list to the new list
	pNewPropList->CopyMatchingValues(pOldPropList);

	// Copy the flags
	pNewNode->SetFlags(pOldObject->GetFlags());

	// Clear the selection flag if it is set
	if (pNewNode->IsFlagSet(NODEFLAG_SELECTED))
	{
		GetRegion()->UnselectNode(pNewNode);
	}

	// Clear the frozen flag if it is set
	if( pNewNode->IsFlagSet( NODEFLAG_FROZEN ) )
	{
		pNewNode->ClearFlag( NODEFLAG_FROZEN );
	}
		
	// Update the label in the node view
	GetNodeView()->UpdateTreeItemLabel(pNewNode);

	return pNewNode;
}

void CRegionDoc::OnEditPasteAlternate() 
{
	PreActionList actionList;

	(( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->Paste( GetRegion(), actionList, FALSE);

	// Setup the undo for the new nodes it added.
	Modify(&actionList, TRUE);
	NotifySelectionChange();		
}

void CRegionDoc::OnUpdateEditPasteAlternate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable((( CMainFrame * )AfxGetMainWnd( ))->GetClipboard( )->IsFull( ));
}

// Clone the selected objects and select the new version
void CRegionDoc::Clone()
{
	// Note : This is lame that it goes through the clipboard, but that was the easy way to implement it..
	OnEditCopy();
	OnEditPasteAlternate();
}

// Select the brushes which are using a given texture
void CRegionDoc::SelectBrushesByTexture(const char *pFilename)
{
	// Get our region
	CEditRegion *pRegion = GetRegion();
	if (!pRegion)
		return;

	// Get the geometry mode state
	POSITION firstPos = GetFirstViewPosition();
	CRegionView *pFirstView = (CRegionView*)GetNextView(firstPos);
	bool bInGeometryMode = (pFirstView && (pFirstView->GetEditMode() == GEOMETRY_EDITMODE));

	// This might take a sec..
	BeginWaitCursor();

	// For each brush..
	for (LPOS iCurBrush = pRegion->m_Brushes.GetHeadPosition(); iCurBrush; )
	{
		CEditBrush *pBrush = pRegion->m_Brushes.GetNext(iCurBrush);

		// For each polygon...
		for (uint32 nPolyLoop = 0; nPolyLoop < pBrush->m_Polies.GetSize(); ++nPolyLoop)
		{
			bool bMatched = false;

			//for each texture
			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				// Select this brush if it's got the right texture on it
				if (stricmp(pFilename, pBrush->m_Polies[nPolyLoop]->GetTexture(nCurrTex).m_pTextureName) == 0)
				{
					// Select the brush
					pRegion->SelectNode(pBrush);
					bMatched = true;

					if (bInGeometryMode)
					{
						// Tag the poly if we're in geometry mode
						TagPoly(CPolyRef(pBrush, nPolyLoop));
					}
					
					// Don't bother searching any more texture channels
					break;
				}
			}

			//we don't need to look further if we already matched a poly in this brush and aren't in geo mode
			if(bMatched && !bInGeometryMode)
				break;
		}
	}

	// Redraw stuff..
	NotifySelectionChange( );
	RedrawAllViews( );

	EndWaitCursor();
}
 
//updates the backup name to reflect any changes in the filename or 
//to the autosave options
void CRegionDoc::UpdateBackupName(const char* pszFilename)
{
	char	pathName[_MAX_PATH], fileName[_MAX_PATH];
	char	worldName[_MAX_PATH], extension[_MAX_PATH], backupName[_MAX_PATH];


	CHelpers::ExtractPathAndFileName( pszFilename, pathName, fileName );
	CHelpers::ExtractFileNameAndExtension( fileName, worldName, extension );

	
	//see if autosaving is disabled, if so, set the path to the default temp one
	if(GetApp()->GetOptions().IsAutoSave() == FALSE)
	{
		//SCHLEGZ: get the temporary path
		GetTempPath(sizeof(pathName),pathName);

		//create the temporary filename
		fileName[3] = '\0';
		GetTempFileName(pathName, fileName,0, backupName);
		m_BackupName = backupName;
	}
	else
	{
		//get the path for the backup directory
		m_BackupName = GetApp()->GetOptions().GetAutoSavePath();

		//ensure that it ends with a trailing slash
		if(	(!m_BackupName.IsEmpty()) && 
			(m_BackupName[m_BackupName.GetLength() - 1] != '\\'))
		{
			m_BackupName += '\\';
		}

		m_BackupName += worldName;
		m_BackupName += "-Backup.";
		m_BackupName += extension;
	}

}
 

// adjust OPQs of polygons based on scaled texture sizes
void CRegionDoc::TextureScale( const CMoArray<BatchTextureScaleInfo*>& textureInfo, CWorldNode* node )
{
	// prime the pump
	if( !node )
		node = m_Region.GetRootNode();

	ASSERT( node );

	// node is a brush, process it
	if( node->GetType() == Node_Brush )
	{
		CEditBrush* brush = node->AsBrush();
		
		// loop through the polygons in the brush
		for( int i = 0; i < brush->m_Polies; i++ )
		{
			CEditPoly* poly = brush->m_Polies[i];

			for(uint32 nCurrTex = 0; nCurrTex < CEditPoly::NUM_TEXTURES; nCurrTex++)
			{
				CTexturedPlane& Texture = poly->GetTexture(nCurrTex);

				// look to see if this poly has a rescaled texture
				// (a hash table would be nice here, just seeing if this might be fast enough)
				for( int j = 0; j < textureInfo.GetSize(); j++ )
				{
					if( !(textureInfo[j]->texName.CompareNoCase( Texture.m_pTextureName )) )
					{
						// found a rescaled polygon
						float xScale = (float)textureInfo[j]->newX / (float)textureInfo[j]->oldX;
						float yScale = (float)textureInfo[j]->newY / (float)textureInfo[j]->oldY;

						LTVector newO = Texture.GetO();
						LTVector newP = Texture.GetP() * xScale;
						LTVector newQ = Texture.GetQ() * yScale;

						poly->SetTextureSpace(nCurrTex, newO, newP, newQ );
					}
				}
			}
		}
	}

	// recurse the children
	GPOS pos = node->m_Children;
	while( pos )
	{
		CWorldNode* child = node->m_Children.GetNext( pos );
		ASSERT( child );
		TextureScale( textureInfo, child );
	}
}

void CRegionDoc::SetTitle(bool bModified)
{
	//this has the filename passed in, so what we need to do is possibly add the
	//path beforehand if the user has that setting enabled, then pass it on down
	CString sTitle;

	COptionsMisc *pOptions = (GetApp()) ? GetApp()->GetOptions().GetMiscOptions() : NULL;

	if(pOptions && pOptions->IsShowFullPathTitle())
	{
		sTitle = m_FileName;
	}
	else
	{
		sTitle = m_WorldName;
	}

	//add the asterisk if needed
	if(bModified)
	{
		sTitle += " *";
	}

#ifdef _DEBUG
	//add an indicator that we are running a debug version
	sTitle += "- DEBUG";
#endif

	CDocument::SetTitle(sTitle);
}
		
void CRegionDoc::SetTitleKeepModified()
{
	//get the title of the document
	CString sTitle = CDocument::GetTitle();

	//see if it has the asterisk on the end
	bool bModified = !(sTitle.IsEmpty()) && (sTitle[sTitle.GetLength() - 1] == '*');

	SetTitle(bModified);
}

void CRegionDoc::TagPoly(const CPolyRef &cPoly)
{
	for (uint32 nCurPoly = 1; nCurPoly < m_TaggedPolies.GetSize(); ++nCurPoly)
	{
		if (m_TaggedPolies[nCurPoly] == cPoly)
			return;
	}

	m_TaggedPolies.Add(cPoly);
}

//called to have all the properties in objects checked in the proper order for reporting
//errors back to the debug window
void CRegionDoc::UpdateAllObjectProperties()
{
	CEditRegion* pRegion = GetRegion();

	if(!pRegion)
		return;

	// First update only the variable declerations...
	
	pRegion->UpdateAllObjectProperties( "VVarDecsOnly; ClearVars;" );

	// Now update our commands to add any dynamic objects...

	pRegion->UpdateAllObjectProperties( "ClearDynaObjs; IMsgErs; IVarCmds;" );

	// Then update everything but the variable declerations and dynamic objects...

	pRegion->UpdateAllObjectProperties( "IVarDecs; IAddDynaObj;" );
}
