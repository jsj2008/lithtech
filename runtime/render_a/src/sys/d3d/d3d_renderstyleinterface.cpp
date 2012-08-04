
#include "precompile.h"
#include "d3d_device.h"
#include "d3d_renderstyleinterface.h"
#include "d3d_renderstyle.h"
#include "client_filemgr.h"					// For class FileRef...

#include <string>

// INTERFACES...
static IClientFileMgr* client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

define_interface(D3DRenderStyles, ILTRenderStyles);

// Duplicate a render style (maybe you want to change it just for a certain object - you better dup it first, then change it, and set it)...
CRenderStyle* D3DRenderStyles::DuplicateRenderStyle(CRenderStyle* pRendStyle)
{
	CD3DRenderStyle* pNewRenderStyle			= g_Device.CreateRenderStyle();
	if (!pNewRenderStyle) return NULL;
	if (!pNewRenderStyle->CopyRenderStyle(pRendStyle)) return NULL;
	return pNewRenderStyle;
}

// Create a render style. You can then set all it's internals yourself...
CRenderStyle* D3DRenderStyles::CreateRenderStyle(bool bSetToDefault)
{
	CD3DRenderStyle* pRenderStyle				= g_Device.CreateRenderStyle();
	return pRenderStyle;
}

// Load a render style (ltb file) and create a render object for it...
CRenderStyle* D3DRenderStyles::LoadRenderStyle(const char* szFilename)
{
	FileRef ref; 
	CD3DRenderStyle* pRenderStyle	= NULL;
	
	ref.m_FileType		= TYPECODE_RSTYLE;
	ref.m_pFilename		= szFilename;

	FileIdentifier* pIdent	= client_file_mgr->GetFileIdentifier(&ref, TYPECODE_RSTYLE);

	if (pIdent) 
	{
		if (pIdent->m_pData) {
			// Is it already loaded?
			pRenderStyle	= (CD3DRenderStyle*)pIdent->m_pData; 
			pRenderStyle->IncRefCount(); 
		}
		else 
		{
			ILTStream* pFileStream = client_file_mgr->OpenFile(&ref);

			if (pFileStream) 
			{					// Create and Load it...
				pRenderStyle = g_Device.CreateRenderStyle();
				if (!pRenderStyle) return NULL;

				if (!pRenderStyle->Load_LTBData(pFileStream)) 
				{ 
					g_Device.DestroyRenderStyle(pRenderStyle); 
					pFileStream->Release(); 
					string szTmp = "Couldn't Load Renderstyle "; 
					szTmp += szFilename; szTmp += "\n"; 
					OutputDebugString(szTmp.c_str()); 
					return false; 
				} 

				// We're keeping a pointer to it in the m_pData member, so inc it's ref count. 
				// This means that all loaded render styles wont be freed until all files are flushed.
				pIdent->m_pData	= pRenderStyle; 
				pRenderStyle->IncRefCount(); 
				pRenderStyle->SetFilename( szFilename ) ; 
				pFileStream->Release(); 
			} 
		} 
	}	

	return pRenderStyle;
}

// Free render style - you no longer need it (may or may not be internally freed - depending on the ref count)...
void D3DRenderStyles::FreeRenderStyle(CRenderStyle* pRendStyle)
{
	pRendStyle->DecRefCount(); assert(pRendStyle->GetRefCount() < 1000000); // Sanity check it...
	if (pRendStyle->GetRefCount() == 0) {
		g_Device.DestroyRenderStyle((CD3DRenderStyle*)pRendStyle); }
}

void D3DRenderStyles::OnDelete(CD3DRenderStyle* pRendStyle)
{
	FileRef ref; 
	
	ref.m_FileType		= TYPECODE_RSTYLE;
	ref.m_pFilename		= pRendStyle->GetFilename();

	FileIdentifier* pIdent	= client_file_mgr->GetFileIdentifier(&ref, TYPECODE_RSTYLE);

	if (pIdent) 
	{
		pIdent->m_pData = 0;
	}
}
