// ------------------------------------------------------------------------
// lithtech (c) 2000
// Base type for handling ltb file io.
// ------------------------------------------------------------------------

#ifndef LT____LTB_FILE_H
#define LT____LTB_FILE_H


#include "ltb.h"
#include "sysstreamsim.h"

// ------------------------------------------------------------------------
// CLTBFile
// for tools 
// Derive the different types of LTB Files from this guy...
// ------------------------------------------------------------------------
class C_LTB_File {
public:
	C_LTB_File(){ 
		m_pFile = NULL; 
	}
	
	~C_LTB_File()							{ 
		CloseFile(); 
	}

	bool OpenFile(const char* szFileName)	
	{
		m_pFile = streamsim_Open(szFileName, "wb");
 		if (!m_pFile) 
		{ 
 			return false; 
		} 
 		return true;        
    }
        
	void CloseFile()						{
		if (m_pFile) { m_pFile->Release(); m_pFile = NULL; } }

protected:
	// All Export functions should call OutputHeader first thing...
	bool OutputHeader()						
	{
		LTB_Header TheHeader;

		if (!m_pFile) 
			return false;
		
        memset( &TheHeader,0,sizeof(TheHeader));
        
		TheHeader.m_iFileType = GetFileType();
		TheHeader.m_iVersion  = GetFileVersion(); 
		m_pFile->Write(&TheHeader,sizeof(TheHeader));
		return true;  
	}
	
	bool ReadHeader(LTB_Header& TheHeader)	
	{
		if (!m_pFile) return false;
		m_pFile->Read(&TheHeader,sizeof(TheHeader));
		return true; 
	}

	// Helper function (outputs a section header)...
	uint32 save_StartSection(char *pSectionName, uint32 prevSectionPos) 
	{
		if (prevSectionPos != (uint32)-1) {	// Go back and link the previous section to this one.
			uint32 curPos = m_pFile->GetPos();
			m_pFile->SeekTo(prevSectionPos);
			*m_pFile << curPos;
			m_pFile->SeekTo(curPos); }

		m_pFile->WriteString(pSectionName);	// Write the section header with no next section..
		uint32 retVal = m_pFile->GetPos();
		m_pFile->WriteVal((uint32)-1);
		return retVal; 
	}

	// ------------------------------------------------------------------------
	// WriteOBBInfo( model, file )
	// writes out obb information in the format 
	// [ num-obbs(u32) [  [ pos(f3) size(f3) orient(f4) parent-node-index(u32) ] ... ] ]
	// ------------------------------------------------------------------------
	void WriteOBBInfo( Model *pModel )
	{
		uint32 iNodeCnt ;
		uint32 iNumEnabledOBBs=0;
		
		// count the number of available obbs first
		for( iNodeCnt = 0 ; iNodeCnt < pModel->NumNodes() ; iNodeCnt++ )
		{
			if( pModel->m_FlatNodeList [ iNodeCnt ]->IsOBBEnabled())
				iNumEnabledOBBs++;
		}

		// write out number of obbs.
		*m_pFile << iNumEnabledOBBs;

		SOBB obb ;
		LTVector basis[3];
		LTMatrix mat;

		// write out the data.
		for( iNodeCnt = 0 ; iNodeCnt < pModel->NumNodes() ; iNodeCnt++ )
		{
			if( pModel->m_FlatNodeList [ iNodeCnt ]->IsOBBEnabled())
			{
				obb = pModel->m_FlatNodeList[ iNodeCnt ]->GetOBB();
				//m_pFile->Write( &obb, sizeof(SOBB));
				m_pFile->Write( &obb.m_Pos, sizeof(LTVector));
				m_pFile->Write( &obb.m_Size, sizeof(LTVector));
				// get the basis convert to matrix get basis from there
				obb.m_Orientation.ConvertToMatrix(mat);
				mat.GetBasisVectors( &basis[0], &basis[1], &basis[2] );
				m_pFile->Write( basis , sizeof(LTVector)*3);
				*m_pFile <<  iNodeCnt ;

				//Calculate the "best fit" radius
				// Set up our 3d points
				LTVector vStart(-obb.m_Size.x, -obb.m_Size.y, -obb.m_Size.z);
				LTVector vEnd(obb.m_Size.x, obb.m_Size.y, obb.m_Size.z);

				// Subtract the two
				LTVector vSub = vEnd - vStart;

				// Find the distance, then calulate the radius. 
				// We divide by 4 because the obb sizes are stored as full lengths
				// and not as radii.
				float fRadius = vSub.Mag()/4.0f;

				m_pFile->Write( &fRadius, sizeof(float));
			}
		}
	}



	virtual uint8  GetFileType() = 0;		// Implemented by the derived class
	virtual uint16 GetFileVersion() = 0;

	ILTStream* m_pFile;
};



#endif

