

#ifndef __D3D_EFFECTMGR_H__
#define __D3D_EFFECTMGR_H__

#include "d3d_device.h"
#include <d3dx9.h>

class CD3DEffectMgr {
public:
	CD3DEffectMgr();
	~CD3DEffectMgr();

	bool Init();
	void Term();
	bool Load(const char* szFilename, bool bSoftware);
	ID3DXEffect* GetEffect(){ return m_pEffect; }
	void UploadVertexDecl();
	void SetEnabled(bool bEnabled){ m_bEnabled = bEnabled; }
	bool GetEnabled(){ return m_bEnabled; }
	const char* GetLastError(){return m_pszLastError;}
	void SetPosition(float x, float y, float z)
	{
		m_vPosition.x = x;
		m_vPosition.y = y;
		m_vPosition.z = z;
	}
	D3DXVECTOR3 *GetPosition() { return &m_vPosition; }
	void SetTechnique(const char* szTechnique){strncpy(m_szTechnique, szTechnique, 63);}
	const char* GetTechnique(){ return m_szTechnique; }

protected:
	ID3DXEffect* m_pEffect;
	IDirect3DVertexDeclaration9* m_pVertexDeclaration;
	bool m_bEnabled;
	char m_pszLastError[1024];
	D3DXVECTOR3 m_vPosition;
	char m_szTechnique[64];

};



extern CD3DEffectMgr g_EffectMgr;

#endif // __D3D_EFFECTMGR_H__
