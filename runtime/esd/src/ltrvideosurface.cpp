/****************************************************************************
;
;	MODULE:		LTRealVideoSurface (.CPP)
;
;	PURPOSE:	Support class for RealVideoMgr
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrvideosurface.h"
#include "ltrconout.h"
#include "ltrealvideo_impl.h"
#include "render.h"

#include "interface_helpers.h"

//-----------------------------------------------------------------------------
// CLTRealVideoSurface member functions
//-----------------------------------------------------------------------------
CLTRealVideoSurface::CLTRealVideoSurface()
{
	m_lRefCount = 0;
	m_pUnknown = LTNULL;

	m_pPlayer = LTNULL;

	memset(&m_BitmapInfo, 0, sizeof(RMABitmapInfoHeader));
	m_pImageBits = LTNULL;
	m_lImageBufferSize = 0;

	m_pTextureData = LTNULL;
	m_uTextureWidth = 0;
	m_uTextureHeight = 0;
}

//-----------------------------------------------------------------------------
CLTRealVideoSurface::~CLTRealVideoSurface()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoSurface::Init(IUnknown* pUnknown, CLTWindowlessSite* pWindowlessSite, CLTRealVideoPlayer* pPlayer)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::Init()");

	if (pUnknown)
	{
		m_pUnknown = pUnknown;
		m_pUnknown->AddRef();
	}

	if (pPlayer)
	{
		m_pPlayer = pPlayer;

		RealVideoSurfaceNodeList* pList = m_pPlayer->GetRealVideoSurfaceNodeList();
		if (pList)
		{
			CLTRealVideoSurfaceNode* pNode;
			LT_MEM_TRACK_ALLOC(pNode = new CLTRealVideoSurfaceNode,LT_MEM_TYPE_MISC);
			pNode->m_pVideoSurface = this;
			pList->Insert(pNode);
		}
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoSurface::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::Term()");

	DestroyTexture();

	if (m_pUnknown)
	{
		m_pUnknown->Release();
		m_pUnknown = LTNULL;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTRealVideoSurface::QueryInterface(REFIID riid, void** ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::QueryInterface()");

    if (IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppvObj = this;
        return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMAVideoSurface))
    {
        AddRef();
        *ppvObj = (IRMAVideoSurface*)this;
        return PNR_OK;
    }
    
    *ppvObj = LTNULL;
    return PNR_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTRealVideoSurface::AddRef()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::AddRef()");

	return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTRealVideoSurface::Release()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::Release()");

    if (InterlockedDecrement(&m_lRefCount) > 0)
    {
        return m_lRefCount;
    }

    delete this;
    return 0;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTRealVideoSurface::Blt(	UCHAR*					pImageData,
										RMABitmapInfoHeader*	pBitmapInfo,
										REF(PNxRect)			inDestRect,
										REF(PNxRect)			inSrcRect)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::Blt()");

    BeginOptimizedBlt(pBitmapInfo);

    return OptimizedBlt(pImageData, inDestRect, inSrcRect);
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTRealVideoSurface::BeginOptimizedBlt(RMABitmapInfoHeader* pBitmapInfo)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::BeginOptimizedBlt()");

	if (!m_pPlayer)
		return PNR_FAIL;

	if (!pBitmapInfo)
		return PNR_FAIL;

    switch (pBitmapInfo->biCompression)
    {
		/*
		 * RMA image formats:
		 */
		case RMA_RGB://			BI_RGB  /* Windows-compatible RGB formats:  */
		case RMA_RLE8://		BI_RLE8
		case RMA_RLE4://		BI_RLE4
		case RMA_BITFIELDS://	BI_BITFIELDS
		case RMA_I420://		MKFOURCC('I','4','2','0') /* planar YCrCb   */
		case RMA_YV12://		MKFOURCC('Y','V','1','2') /* planar YVU420  */
		case RMA_YUY2://		MKFOURCC('Y','U','Y','2') /* packed YUV422  */
		case RMA_UYVY://		MKFOURCC('U','Y','V','Y') /* packed YUV422  */
		case RMA_YVU9://		MKFOURCC('Y','V','U','9') /* Intel YVU9     */

		/*
		 * Non-standard FOURCC formats (these are just few aliases to what can be
		 * represented by the standard formats, and they are left for backward
		 * compatibility only).
		 */
		case RMA_RGB3_ID://		MKFOURCC('3','B','G','R') /* RGB-32 ??      */
		case RMA_RGB24_ID://	MKFOURCC('B','G','R',' ') /* top-down RGB-24*/
		case RMA_RGB565_ID://	MKFOURCC('6','B','G','R') /* RGB-16 565     */
		case RMA_RGB555_ID://	MKFOURCC('5','B','G','R') /* RGB-16 555     */
		case RMA_8BIT_ID://		MKFOURCC('T','I','B','8') /* RGB-8 w. pal-e */
		case RMA_YUV420_ID://	MKFOURCC('2','V','U','Y') /* planar YCrCb   */
		case RMA_YUV411_ID://	MKFOURCC('1','V','U','Y') /* ???            */
		case RMA_YUVRAW_ID://	MKFOURCC('R','V','U','Y') /* ???            */
		{
			// NOTE: In talking with the Real team, this routine is always called
			//  first with YUV first.  If we fail this call, a second call will
			//  be made using RGB format, the one we want.  Note also that the
			//  GetPreferredFormat() routine isn't called.
			if (RMA_YUV420_ID == pBitmapInfo->biCompression)
				return PNR_FAIL;

			// This is the format we really want...
			if (RMA_RGB == pBitmapInfo->biCompression)
			{
				memcpy(&m_BitmapInfo, pBitmapInfo, sizeof(RMABitmapInfoHeader));
				ASSERT(m_BitmapInfo.biWidth > 0);
				ASSERT(m_BitmapInfo.biHeight > 0);

				// 2D Initialization
				//  Update the overlay objects with the new movie size
				if (m_pPlayer->m_b2DRenderingEnabled)
				{
					RealVideoOverlayList* pList = m_pPlayer->GetRealVideoOverlayList();
					ASSERT(pList);
					CLTRealVideoOverlay* pOverlay = pList->GetFirst();
					while (pOverlay)
					{
						pOverlay->CreateSurface(m_BitmapInfo.biWidth, m_BitmapInfo.biHeight);
						pOverlay = pOverlay->Next();
					}
				}

				// 3D Initialization
				//  Assign shared texture to surface textures
				if (m_pPlayer->m_b3DRenderingEnabled)
				{
					SurfaceNodeList* pSurfaceNodeList = m_pPlayer->GetSurfaceNodeList();
					if (pSurfaceNodeList)
					{
						// Create/init our texture (used by the shared texture)
						if (LTNULL == m_pTextureData)
						{
							CreateTexture(m_BitmapInfo.biWidth, m_BitmapInfo.biHeight);

							CLTSurfaceNode* pSurfaceNode = pSurfaceNodeList->GetFirst();
							while (pSurfaceNode)
							{
								Surface* pSurface = pSurfaceNode->m_pSurface;
								if (pSurface)
								{
									pSurfaceNode->m_pOriginalTexture = pSurface->m_pTexture;
									pSurface->m_pTexture = &m_SharedTexture;
								}
								pSurfaceNode = pSurfaceNode->m_pNext;
							}
						}
					}
				}

				// Accepted format
				return PNR_OK;
			}
		}
	}

	return PNR_FAIL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTRealVideoSurface::OptimizedBlt(	UCHAR*			pImageBits,
												REF(PNxRect)	rDestRect,
												REF(PNxRect)	rSrcRect)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::OptimizedBlt()");
    
	if (!pImageBits)
		return PNR_UNEXPECTED;

	// Convert the incoming image bits based on screen format
	// NOTE: According to the Real team, the incoming format is always 24 bit
	//  so we'll need to convert to 32 or 16 bit
	switch (g_ScreenFormat.m_BPP)
	{
		case BPP_32:
		{
			// Allocate the target buffer if needed
			if (!m_pImageBits)
			{
				// Going from 3 bytes per pixel to 4
				m_lImageBufferSize = m_BitmapInfo.biWidth * m_BitmapInfo.biHeight * 4;
				LT_MEM_TRACK_ALLOC(m_pImageBits = new UCHAR[m_lImageBufferSize],LT_MEM_TYPE_MISC);
			}

			// Copy
			UINT32* pDest = (UINT32*)m_pImageBits;
			UCHAR* pSrc = pImageBits + (((m_BitmapInfo.biWidth * m_BitmapInfo.biHeight) - m_BitmapInfo.biWidth) * 3);
			UCHAR* pRow = pSrc;
			for (INT32 height = 0; height < m_BitmapInfo.biHeight; height++)
			{
				for (INT32 width = 0; width < m_BitmapInfo.biWidth; width++)
				{
					UCHAR* pAlpha = pSrc;
					UCHAR uRed, uGreen, uBlue;
					
					uRed = *pAlpha++;
					uGreen = *pAlpha++;
					uBlue =  *pAlpha++;

					*pDest = *(UINT32*)pSrc & 0xFFFFFF00 | (UCHAR)((uRed + uGreen + uBlue) / (UCHAR)3);

					pSrc += 3;
					pDest++;
				}
				pRow -= m_BitmapInfo.biWidth * 3;
				pSrc = pRow;
			}
		}
		break;

		case BPP_16:
		{
			// Allocate the target buffer if needed
			if (!m_pImageBits)
			{
				// Going from 3 bytes per pixel to 2
				m_lImageBufferSize = m_BitmapInfo.biWidth * m_BitmapInfo.biHeight * 2;
				LT_MEM_TRACK_ALLOC(m_pImageBits = new UCHAR[m_lImageBufferSize],LT_MEM_TYPE_MISC);
			}
			
			// Copy
			UINT16* pDest = (UINT16*)m_pImageBits;
			UCHAR* pSrc = pImageBits + (((m_BitmapInfo.biWidth * m_BitmapInfo.biHeight) - m_BitmapInfo.biWidth) * 3);
			UCHAR* pRow = pSrc;
			for (INT32 height = 0; height < m_BitmapInfo.biHeight; height++)
			{
				for (INT32 width = 0; width < m_BitmapInfo.biWidth; width++)
				{
					//-----------------------------------------------------------------------------
					// Assume 5-6-5 BGR for now...
					UCHAR* pColor = pSrc;
					UCHAR uBlue		= *pColor++;
					UCHAR uGreen	= *pColor++;
					UCHAR uRed		= *pColor++;

					UINT16 uNewRed = uRed >> 3;
					UINT16 uNewGreen = uGreen >> 2;
					UINT16 uNewBlue = uBlue >> 3;

					*pDest = (UINT16)((uNewRed << 11) | (uNewGreen << 5) | (uNewBlue << 0));
					//-----------------------------------------------------------------------------
					pSrc += 3;
					pDest++;
				}
				pRow -= m_BitmapInfo.biWidth * 3;
				pSrc = pRow;
			}
		}
		break;
	}

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTRealVideoSurface::EndOptimizedBlt()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::EndOptimizedBlt()");

	if (!m_pPlayer)
		return PNR_FAIL;

	// 2D Clean up
	//  Clean up overlays
	if (m_pPlayer->m_b2DRenderingEnabled)
	{
		RealVideoOverlayList* pList = m_pPlayer->GetRealVideoOverlayList();
		ASSERT(pList);
		CLTRealVideoOverlay* pOverlay = pList->GetFirst();
		while (pOverlay)
		{
			pOverlay->DeleteSurface();
			pOverlay = pOverlay->Next();
		}
	}

	// 3D Clean up
	//  Restore textures
	if (m_pPlayer->m_b3DRenderingEnabled)
	{
		SurfaceNodeList* pSurfaceNodeList = m_pPlayer->GetSurfaceNodeList();
		if (pSurfaceNodeList)
		{
			CLTSurfaceNode* pSurfaceNode = pSurfaceNodeList->GetFirst();
			while (pSurfaceNode)
			{
				Surface* pSurface = pSurfaceNode->m_pSurface;
				if (pSurface)
					pSurface->m_pTexture = pSurfaceNode->m_pOriginalTexture;
		
				pSurfaceNode = pSurfaceNode->m_pNext;
			}
		}
		//  Destroy allocated texture
		DestroyTexture();
	}

	memset(&m_BitmapInfo, 0, sizeof(RMABitmapInfoHeader));

	if (m_pImageBits)
	{
		delete []m_pImageBits;
		m_pImageBits = LTNULL;
	}

	m_lImageBufferSize = 0;

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTRealVideoSurface::GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
	ulType = m_BitmapInfo.biCompression;

    return PNR_NOTIMPL;
}

//-----------------------------------------------------------------------------
// NOTE: Never gets called...
STDMETHODIMP CLTRealVideoSurface::GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) ulType)
{
    ulType = RMA_RGB;

    return PNR_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoSurface::Update(LTRV_UPDATE_MODE updateMode)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::Update()");

	// Check the update mode
	if (!(LTRV_UPDATE_2D == updateMode || LTRV_UPDATE_3D == updateMode))
		return LT_ERROR;

	// We need to have a player (sanity check)
	if (!m_pPlayer)
		return LT_ERROR;

	// Verify that we have data to render, otherwise return
	if (!m_pImageBits)
		return LT_OK;

	// Did we just render our final screen?
	LTBOOL bDone = LTFALSE;
	m_pPlayer->IsDone(&bDone);
	if (bDone)
	{
		EndOptimizedBlt();
		return LT_OK;
	}

	// Copy next frame
	if (LTRV_UPDATE_2D == updateMode)
	{
		// Update 2D
		RealVideoOverlayList* pList = m_pPlayer->GetRealVideoOverlayList();
		ASSERT(pList);
		CLTRealVideoOverlay* pOverlay = pList->GetFirst();
		while (pOverlay)
		{
			CisSurface* pSurface = (CisSurface*)pOverlay->GetSurface();
			if(pSurface)
			{
				uint32 iPitch = 0;
				uint8* pBuffer = (uint8*)g_pCisRenderStruct->LockSurface(pSurface->m_hBuffer, iPitch);
				if (pBuffer)
				{
					memcpy(pBuffer, m_pImageBits, m_lImageBufferSize);
					g_pCisRenderStruct->UnlockSurface(pSurface->m_hBuffer);
					cis_SetDirty(pSurface);
				}
			}
			
			pOverlay = pOverlay->Next();
		}
	}
	else
	{
		// Update 3D
		SurfaceNodeList* pSurfaceNodeList = m_pPlayer->GetSurfaceNodeList();
		if (pSurfaceNodeList)
		{
			CLTSurfaceNode* pSurfaceNode = pSurfaceNodeList->GetFirst();

			// Copy the data to the shared surface?  If no surface node, don't bother...
			if (pSurfaceNode)
			{
				// Paranoid assertions
				ASSERT(m_pTextureData);
				ASSERT(m_uTextureWidth >= (UINT32)m_BitmapInfo.biWidth);
				ASSERT(m_uTextureHeight >= (UINT32)m_BitmapInfo.biHeight);

				UINT32 uWidthOffset = (m_uTextureWidth - m_BitmapInfo.biWidth) / 2;
				UINT32 uHeightOffset = (m_uTextureHeight - m_BitmapInfo.biHeight) / 2;

				// Copy our image to the texture mip level zero data
				switch (g_ScreenFormat.m_BPP)
				{
					case BPP_32:
					{
						UINT32* pSrc = (UINT32*)m_pImageBits;
						UINT32* pDest = (UINT32*)m_pTextureData->m_Mips[0].m_Data;

						// Offset the dest position to get us started
						pDest += ((uHeightOffset * m_uTextureWidth) + uWidthOffset);

						for (INT32 height = 0; height < m_BitmapInfo.biHeight; height++)
						{
							for (INT32 width = 0; width < m_BitmapInfo.biWidth; width++)
							{
								*pDest = *pSrc;

								pSrc++;
								pDest++;
							}

							// Skip down to the next row
							pDest += (uWidthOffset * 2);
						}
					}
					break;

					case BPP_16:
					{
						UINT16* pSrc = (UINT16*)m_pImageBits;
						UINT32* pDest = (UINT32*)m_pTextureData->m_Mips[0].m_Data;

						// Offset the dest position to get us started
						pDest += ((uHeightOffset * m_uTextureWidth) + uWidthOffset);

						for (INT32 height = 0; height < m_BitmapInfo.biHeight; height++)
						{
							for (INT32 width = 0; width < m_BitmapInfo.biWidth; width++)
							{
								//-----------------------------------------------------------------------------
								// Assume 5-6-5 BGR for now...
								UINT16 uColor = *pSrc;

								UINT32 uRed		= uColor;
								uRed = uRed << 16;
								uRed = uRed >> 24;
								
								UINT32 uGreen	= uColor;
								uGreen = uGreen << 21;
								uGreen = uGreen >> 24;
								
								UINT32 uBlue	= uColor;
								uBlue = uBlue << 27;
								uBlue = uBlue >> 24;

								UINT32 uNewRed = uRed << 16;
								UINT32 uNewGreen = uGreen << 8;
								UINT32 uNewBlue = uBlue << 0;

								*pDest = (UINT32)(uNewRed | uNewGreen | uNewBlue);

								pSrc++;
								pDest++;
							}

							// Skip down to the next row
							pDest += (uWidthOffset * 2);
						}
					}
					break;
				}

				// Tell the renderer to reload the texture.
				r_BindTexture(&m_SharedTexture, LTTRUE);
			}
		}
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoSurface::CreateTexture(UINT32 uWidth, UINT32 uHeight)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::CreateTexture()");

	// Clean up first
	DestroyTexture();

	// Textures must be a power of two, so find the next higher size, if needed
	m_uTextureWidth = 1;
	m_uTextureHeight = 1;
	while (m_uTextureWidth < uWidth)
		m_uTextureWidth = m_uTextureWidth << 1;
	while (m_uTextureHeight < uHeight)
		m_uTextureHeight = m_uTextureHeight << 1;

// [mds] Remove
m_uTextureWidth = 256;
m_uTextureHeight = 256;
// [mds] Remove

	// Allocate texture
	m_pTextureData = dtx_Alloc(BPP_32, m_uTextureWidth, m_uTextureHeight, 1, LTNULL, LTNULL);
	if (!m_pTextureData)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealVideoSurface::CreateTexture() texture allocation failed.");
		return PNR_FAIL;
	}

	UINT32 uSize = 0;
	//if (BPP_32 == g_ScreenFormat.m_BPP)
		uSize = m_uTextureWidth * m_uTextureHeight * (32 / 8);	// 4 bytes per pixel [mds!] 32 bit only!!
	//else
	//	uSize = m_uTextureWidth * m_uTextureHeight * (16 / 8);	// 2 bytes per pixel [mds!] 32 bit only!!
	ASSERT(0 < uSize);
	memset(m_pTextureData->m_Mips[0].m_Data, 0, uSize);
//	m_pTextureData->m_Header.m_IFlags |= DTX_NOSYSCACHE; [mds] // Check with Jeff E on this
	m_SharedTexture.m_pEngineData = m_pTextureData;
	r_BindTexture(&m_SharedTexture, LTTRUE);

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoSurface::DestroyTexture()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoSurface::DestroyTexture()");

	if(m_pTextureData)
	{
//		r_UnbindTexture(&m_SharedTexture);	[mds] // Check with Jeff E on this
		m_SharedTexture.m_pEngineData = LTNULL;

		dtx_Destroy(m_pTextureData);
		m_pTextureData = LTNULL;
	}

	return LT_OK;
}
#endif // LITHTECH_ESD