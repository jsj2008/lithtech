//------------------------------------------------------------------
//
//   MODULE    : GENDRAWPRIM.H
//
//   PURPOSE   : Implements derived class CGenDrawPrim
//
//   CREATED   : On 8/11/00 At 1:39:59 PM
//
//   COPYRIGHT : (C) 2000 LithTech Inc
//
//------------------------------------------------------------------

#ifndef __GENDRAWPRIM_H__
#define __GENDRAWPRIM_H__

// Includes....
#ifndef __ILTDRAWPRIM_H__
#include "iltdrawprim.h"
#endif


#ifdef _WIN32
#pragma warning (disable:4244)              // Disable the int to float conversion warning...
#endif

class CGenDrawPrim : public ILTDrawPrim
{
    public :
        // init the object
        CGenDrawPrim () 
		{
            m_pCamera			= NULL;
            m_pTexture			= NULL;
            m_eTransType		= DRAWPRIM_TRANSFORM_SCREEN;
            m_eClipType			= DRAWPRIM_FASTCLIP;

            m_ColorOp			= DRAWPRIM_NOCOLOROP;
            m_BlendMode			= DRAWPRIM_NOBLEND;
            m_eZBufferMode		= DRAWPRIM_ZRW;
            m_eTestMode			= DRAWPRIM_NOALPHATEST;
            m_eFillMode			= DRAWPRIM_FILL ;
            m_eCullMode			= DRAWPRIM_CULL_NONE;
			m_bFogEnable		= false;
			m_bReallyClose		= false;
			m_nEffectShaderID   = -1;
        }

        // Sets the current camera to use (viewport, field of view etc)
        virtual LTRESULT SetCamera(const HOBJECT hCamera) {
            m_pCamera = hCamera; return LT_OK;
        }

        // Sets current texture
        virtual LTRESULT SetTexture(const HTEXTURE hTexture) {
            m_pTexture = hTexture; return LT_OK;
        }

        // Sets transform type
        virtual LTRESULT SetTransformType(const ELTTransformType eType) {
            m_eTransType = eType; return LT_OK;
        }

        // Sets color operation
        virtual LTRESULT SetColorOp(const ELTColorOp eColorOp) {
            m_ColorOp = eColorOp; return LT_OK;
        }

        // Sets source/dest alpha blending operation
        virtual LTRESULT SetAlphaBlendMode(const ELTBlendMode eBlendMode) {
            m_BlendMode = eBlendMode; return LT_OK;
        }

        // Enables/disables z buffering
        virtual LTRESULT SetZBufferMode(const ELTZBufferMode eZBufferMode) {
            m_eZBufferMode = eZBufferMode; return LT_OK;
        }

        // Set AlphaTest Mode (on/off)
        virtual LTRESULT SetAlphaTestMode(const ELTTestMode eTestMode) {
            m_eTestMode = eTestMode; return LT_OK;
        }

        // set the type of clipping to be done
        virtual LTRESULT SetClipMode(const ELTClipMode eClipMode) {
            m_eClipType = eClipMode; return LT_OK;
        }

        // set the file mode
        virtual LTRESULT SetFillMode(ELTDPFillMode FillMode)
        {
            m_eFillMode = FillMode ;
            return LT_OK;
        }
    
        // set the cull mode
        virtual LTRESULT SetCullMode(ELTDPCullMode CullMode)
        {
            m_eCullMode = CullMode ;
            return LT_OK;
        }

		//set the fog enable status
		virtual LTRESULT SetFogEnable(bool bFogEnable)
		{
			m_bFogEnable = bFogEnable;
			return LT_OK;
		}

        //Specifiy whether or not to be in really close space for rendering
        virtual LTRESULT SetReallyClose(bool bReallyClose)
		{
			m_bReallyClose = bReallyClose;
			return LT_OK;
		}

		virtual LTRESULT SetEffectShaderID(uint32 nEffectShaderID)
		{
			m_nEffectShaderID = nEffectShaderID;
			return LT_OK;
		}

        // POLYGON COORDS.
        
        
        // set polygon coords for a GT4
        inline void SetXY4 (LT_POLYGT4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x0;
            pPrim->verts[0].y = y0;
            pPrim->verts[1].x = x1;
            pPrim->verts[1].y = y1;
            pPrim->verts[2].x = x2;
            pPrim->verts[2].y = y2;
            pPrim->verts[3].x = x3;
            pPrim->verts[3].y = y3;
        }
        
        // set polygon coords for a FT4
        inline void SetXY4 (LT_POLYFT4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x0;
            pPrim->verts[0].y = y0;
            pPrim->verts[1].x = x1;
            pPrim->verts[1].y = y1;
            pPrim->verts[2].x = x2;
            pPrim->verts[2].y = y2;
            pPrim->verts[3].x = x3;
            pPrim->verts[3].y = y3;
        }

        // set polygon coords for a G4
        inline void SetXY4 (LT_POLYG4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x0; 
            pPrim->verts[0].y = y0;
            pPrim->verts[1].x = x1;
            pPrim->verts[1].y = y1;
            pPrim->verts[2].x = x2;
            pPrim->verts[2].y = y2;
            pPrim->verts[3].x = x3;
            pPrim->verts[3].y = y3;
        }

        // set polygon coords for a F4
        inline void SetXY4 (LT_POLYF4 *pPrim,
                             float x0, float y0,
                             float x1, float y1,
                             float x2, float y2,
                             float x3, float y3)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x0;
            pPrim->verts[0].y = y0;
            pPrim->verts[1].x = x1;
            pPrim->verts[1].y = y1;
            pPrim->verts[2].x = x2;
            pPrim->verts[2].y = y2;
            pPrim->verts[3].x = x3;
            pPrim->verts[3].y = y3;
        }


        // -------------------------------------------------------
        // SetXYWH
        
        // set polygon coords for a GT4
        inline void SetXYWH (LT_POLYGT4 *pPrim,
                             float x, float y,
                             float w, float h)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x;
            pPrim->verts[0].y = y;
            pPrim->verts[1].x = x + w;// - 1;
            pPrim->verts[1].y = y;
            pPrim->verts[2].x = x + w;// - 1;
            pPrim->verts[2].y = y + h;// - 1;
            pPrim->verts[3].x = x;
            pPrim->verts[3].y = y + h;// - 1;
        }

        // set polygon coords for a FT4
        inline void SetXYWH (LT_POLYFT4 *pPrim,
                             float x, float y,
                             float w, float h)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x;
            pPrim->verts[0].y = y;
            pPrim->verts[1].x = x + w;// - 1;
            pPrim->verts[1].y = y;
            pPrim->verts[2].x = x + w;// - 1;
            pPrim->verts[2].y = y + h;// - 1;
            pPrim->verts[3].x = x;
            pPrim->verts[3].y = y + h;// - 1;
        }

        // set polygon coords for a G4
        inline void SetXYWH (LT_POLYG4 *pPrim,
                             float x, float y,
                             float w, float h)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x;
            pPrim->verts[0].y = y;
            pPrim->verts[1].x = x + w;// - 1;
            pPrim->verts[1].y = y;
            pPrim->verts[2].x = x + w;// - 1;
            pPrim->verts[2].y = y + h;// - 1;
            pPrim->verts[3].x = x;
            pPrim->verts[3].y = y + h;// - 1;
        }

        // set polygon coords for a F4
        inline void SetXYWH (LT_POLYF4 *pPrim,
                             float x, float y,
                             float w, float h)
        {
            if (!pPrim) return;
            pPrim->verts[0].x = x;
            pPrim->verts[0].y = y;
            pPrim->verts[1].x = x + w;// - 1;
            pPrim->verts[1].y = y;
            pPrim->verts[2].x = x + w;// - 1;
            pPrim->verts[2].y = y + h;// - 1;
            pPrim->verts[3].x = x;
            pPrim->verts[3].y = y + h;// - 1;
        }
        
        
        // TEXTURE COORDS.
        
        
        // set texture coords for a GT4
        inline void SetUV4 (LT_POLYGT4 *pPrim,
                             float u0, float v0,
                             float u1, float v1,
                             float u2, float v2,
                             float u3, float v3)
        {
            if (!pPrim) return;
            pPrim->verts[0].u = u0;
            pPrim->verts[0].v = v0;
            pPrim->verts[1].u = u1;
            pPrim->verts[1].v = v1;
            pPrim->verts[2].u = u2;
            pPrim->verts[2].v = v2;
            pPrim->verts[3].u = u3;
            pPrim->verts[3].v = v3;
        }
        
        // set texture coords for a FT4
        inline void SetUV4 (LT_POLYFT4 *pPrim,
                             float u0, float v0,
                             float u1, float v1,
                             float u2, float v2,
                             float u3, float v3)
        {
            if (!pPrim) return;
            pPrim->verts[0].u = u0;
            pPrim->verts[0].v = v0;
            pPrim->verts[1].u = u1;
            pPrim->verts[1].v = v1;
            pPrim->verts[2].u = u2;
            pPrim->verts[2].v = v2;
            pPrim->verts[3].u = u3;
            pPrim->verts[3].v = v3;
        }

        // -------------------------------------------------------
        // SetXYWH
        
        // set texture coords for a GT4
        inline void SetUVWH (LT_POLYGT4 *pPrim,
                             float u, float v,
                             float w, float h)
        {
            if (!pPrim) return;
            pPrim->verts[0].u = u;
            pPrim->verts[0].v = v;
            pPrim->verts[1].u = u + w;// - 1;
            pPrim->verts[1].v = v;
            pPrim->verts[2].u = u + w;// - 1;
            pPrim->verts[2].v = v + h;// - 1;
            pPrim->verts[3].u = u;
            pPrim->verts[3].v = v + h;// - 1;
        }

		// set texture coords for a GT4
		/*
		void SetUVWH (LT_POLYGT4 *pPrim, HTEXTURE pTex,
							 float u, float v,
							 float w, float h);
							 */
		/*
		{
			if (!pPrim) return;

			uint32 ttw,tth;
			float tw, th;
			
			if (pTex) {
				pTexInterface->GetTextureDims(pTex, ttw, tth);
				tw = (float)ttw;
				th = (float)tth;

				float factor = 1.0f;

				float pixelcenterh = 0.05/tw;
				float pixelcenterv = 0.05/th;

				pPrim->verts[0].u = u / tw					+ pixelcenterh;
				pPrim->verts[0].v = v / th					+ pixelcenterv;
				pPrim->verts[1].u = (u + w + factor) / tw	+ pixelcenterh;
				pPrim->verts[1].v = v / th					+ pixelcenterv;
				pPrim->verts[2].u = (u + w + factor) / tw	+ pixelcenterh;
				pPrim->verts[2].v = (v + h + factor) / th	+ pixelcenterv;
				pPrim->verts[3].u = u / tw					+ pixelcenterh;
				pPrim->verts[3].v = (v + h + factor) / th	+ pixelcenterv;

			}
			else {
				pPrim->verts[0].u = pPrim->verts[1].u = pPrim->verts[2].u = pPrim->verts[3].u = 0.0f;
				pPrim->verts[0].v = pPrim->verts[1].v = pPrim->verts[2].v = pPrim->verts[3].v = 0.0f;
			}			
		}*/

        // set texture coords for a FT4
        inline void SetUVWH (LT_POLYFT4 *pPrim,
                             float u, float v,
                             float w, float h)
        {
            if (!pPrim) return;
            pPrim->verts[0].u = u;
            pPrim->verts[0].v = v;
            pPrim->verts[1].u = u + w;// - 1;
            pPrim->verts[1].v = v;
            pPrim->verts[2].u = u + w;// - 1;
            pPrim->verts[2].v = v + h;// - 1;
            pPrim->verts[3].u = u;
            pPrim->verts[3].v = v + h;// - 1;
        }
        
        
        // COLOR
        
                
        // some color macros
        #define dALPHA(c) ((c >> 24) & 0xFF)
        #define dRED(c)   ((c >> 16) & 0xFF)
        #define dGREEN(c) ((c >> 8)  & 0xFF)
        #define dBLUE(c)  ((c)       & 0xFF)
        
        inline void SetRGB(LT_POLYGT4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = 
                    pPrim->verts[1].rgba.r = 
                        pPrim->verts[2].rgba.r =
                            pPrim->verts[3].rgba.r = dRED(color);
            pPrim->verts[0].rgba.g = 
                    pPrim->verts[1].rgba.g = 
                        pPrim->verts[2].rgba.g =
                            pPrim->verts[3].rgba.g = dGREEN(color);
            pPrim->verts[0].rgba.b = 
                    pPrim->verts[1].rgba.b = 
                        pPrim->verts[2].rgba.b =
                            pPrim->verts[3].rgba.b = dBLUE(color);
            /*
            pPrim->verts[0].rgba.a = 
                    pPrim->verts[1].a = 
                        pPrim->verts[2].a =
                            pPrim->verts[3].a = 0;
            */
        }
        
        inline void SetRGB(LT_POLYG4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = 
                    pPrim->verts[1].rgba.r = 
                        pPrim->verts[2].rgba.r =
                            pPrim->verts[3].rgba.r = dRED(color);
            pPrim->verts[0].rgba.g = 
                    pPrim->verts[1].rgba.g = 
                        pPrim->verts[2].rgba.g =
                            pPrim->verts[3].rgba.g = dGREEN(color);
            pPrim->verts[0].rgba.b = 
                    pPrim->verts[1].rgba.b = 
                        pPrim->verts[2].rgba.b =
                            pPrim->verts[3].rgba.b = dBLUE(color);
            /*
            pPrim->verts[0].rgba.a = 
                    pPrim->verts[1].a = 
                        pPrim->verts[2].a =
                            pPrim->verts[3].a = 0;
            */
        }

        inline void SetRGB(LT_POLYFT4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->rgba.r = dRED(color);
            pPrim->rgba.g = dGREEN(color);
            pPrim->rgba.b = dBLUE(color);
            //pPrim.rgba.a = 0;                 
        }

        inline void SetRGB(LT_POLYF4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->rgba.r = dRED(color);
            pPrim->rgba.g = dGREEN(color);
            pPrim->rgba.b = dBLUE(color);
            //pPrim.rgba.a = 0;                 
        }

        // ------------------------------------------
        
        inline void SetRGBA(LT_POLYGT4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = 
                    pPrim->verts[1].rgba.r = 
                        pPrim->verts[2].rgba.r =
                            pPrim->verts[3].rgba.r = dRED(color);
            pPrim->verts[0].rgba.g = 
                    pPrim->verts[1].rgba.g = 
                        pPrim->verts[2].rgba.g =
                            pPrim->verts[3].rgba.g = dGREEN(color);
            pPrim->verts[0].rgba.b = 
                    pPrim->verts[1].rgba.b = 
                        pPrim->verts[2].rgba.b =
                            pPrim->verts[3].rgba.b = dBLUE(color);
            pPrim->verts[0].rgba.a = 
                    pPrim->verts[1].rgba.a = 
                        pPrim->verts[2].rgba.a =
                            pPrim->verts[3].rgba.a = dALPHA(color);
            
        }
        
        inline void SetRGBA(LT_POLYG4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = 
                    pPrim->verts[1].rgba.r = 
                        pPrim->verts[2].rgba.r =
                            pPrim->verts[3].rgba.r = dRED(color);
            pPrim->verts[0].rgba.g = 
                    pPrim->verts[1].rgba.g = 
                        pPrim->verts[2].rgba.g =
                            pPrim->verts[3].rgba.g = dGREEN(color);
            pPrim->verts[0].rgba.b = 
                    pPrim->verts[1].rgba.b = 
                        pPrim->verts[2].rgba.b =
                            pPrim->verts[3].rgba.b = dBLUE(color);
            pPrim->verts[0].rgba.a = 
                    pPrim->verts[1].rgba.a = 
                        pPrim->verts[2].rgba.a =
                            pPrim->verts[3].rgba.a = dALPHA(color);
            
        }

        inline void SetRGBA(LT_POLYFT4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->rgba.r = dRED(color);
            pPrim->rgba.g = dGREEN(color);
            pPrim->rgba.b = dBLUE(color);
            pPrim->rgba.a = dALPHA(color);                  
        }

        inline void SetRGBA(LT_POLYF4 *pPrim, uint32 color) {
            if (!pPrim) return;
            pPrim->rgba.r = dRED(color);
            pPrim->rgba.g = dGREEN(color);
            pPrim->rgba.b = dBLUE(color);
            pPrim->rgba.a = dALPHA(color);                  
        }
        
        // ------------------------------------------
        
        // note: it makes no sense to have a SetRGB4 on flat-shaded polys       

        inline void SetRGB4(LT_POLYGT4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3)
        {       
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = dRED(color0);
            pPrim->verts[1].rgba.r = dRED(color1);
            pPrim->verts[2].rgba.r = dRED(color2);
            pPrim->verts[3].rgba.r = dRED(color3);
    
            pPrim->verts[0].rgba.g = dGREEN(color0);
            pPrim->verts[1].rgba.g = dGREEN(color1);
            pPrim->verts[2].rgba.g = dGREEN(color2);
            pPrim->verts[3].rgba.g = dGREEN(color3);
            
            pPrim->verts[0].rgba.b = dBLUE(color0); 
            pPrim->verts[1].rgba.b = dBLUE(color1); 
            pPrim->verts[2].rgba.b = dBLUE(color2);
            pPrim->verts[3].rgba.b = dBLUE(color3);
            /*
            pPrim->verts[0].rgba.a = dALPHA(color0); 
            pPrim->verts[1].rgba.a = dALPHA(color1); 
            pPrim->verts[2].rgba.a = dALPHA(color2);
            pPrim->verts[3].rgba.a = dALPHA(color3);
            */
        }
    

        inline void SetRGB4(LT_POLYG4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3)
        {       
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = dRED(color0);
            pPrim->verts[1].rgba.r = dRED(color1);
            pPrim->verts[2].rgba.r = dRED(color2);
            pPrim->verts[3].rgba.r = dRED(color3);
    
            pPrim->verts[0].rgba.g = dGREEN(color0);
            pPrim->verts[1].rgba.g = dGREEN(color1);
            pPrim->verts[2].rgba.g = dGREEN(color2);
            pPrim->verts[3].rgba.g = dGREEN(color3);
            
            pPrim->verts[0].rgba.b = dBLUE(color0); 
            pPrim->verts[1].rgba.b = dBLUE(color1); 
            pPrim->verts[2].rgba.b = dBLUE(color2);
            pPrim->verts[3].rgba.b = dBLUE(color3);
            /*
            pPrim->verts[0].rgba.a = dALPHA(color0); 
            pPrim->verts[1].rgba.a = dALPHA(color1); 
            pPrim->verts[2].rgba.a = dALPHA(color2);
            pPrim->verts[3].rgba.a = dALPHA(color3);
            */
        }
        

        inline void SetRGBA4(LT_POLYGT4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3)
        {       
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = dRED(color0);
            pPrim->verts[1].rgba.r = dRED(color1);
            pPrim->verts[2].rgba.r = dRED(color2);
            pPrim->verts[3].rgba.r = dRED(color3);
    
            pPrim->verts[0].rgba.g = dGREEN(color0);
            pPrim->verts[1].rgba.g = dGREEN(color1);
            pPrim->verts[2].rgba.g = dGREEN(color2);
            pPrim->verts[3].rgba.g = dGREEN(color3);
            
            pPrim->verts[0].rgba.b = dBLUE(color0); 
            pPrim->verts[1].rgba.b = dBLUE(color1); 
            pPrim->verts[2].rgba.b = dBLUE(color2);
            pPrim->verts[3].rgba.b = dBLUE(color3);
            
            pPrim->verts[0].rgba.a = dALPHA(color0); 
            pPrim->verts[1].rgba.a = dALPHA(color1); 
            pPrim->verts[2].rgba.a = dALPHA(color2);
            pPrim->verts[3].rgba.a = dALPHA(color3);
        }

        inline void SetRGBA4(LT_POLYG4 *pPrim, 
                             uint32 color0, uint32 color1,
                             uint32 color2, uint32 color3)
        {       
            if (!pPrim) return;
            pPrim->verts[0].rgba.r = dRED(color0);
            pPrim->verts[1].rgba.r = dRED(color1);
            pPrim->verts[2].rgba.r = dRED(color2);
            pPrim->verts[3].rgba.r = dRED(color3);
    
            pPrim->verts[0].rgba.g = dGREEN(color0);
            pPrim->verts[1].rgba.g = dGREEN(color1);
            pPrim->verts[2].rgba.g = dGREEN(color2);
            pPrim->verts[3].rgba.g = dGREEN(color3);
            
            pPrim->verts[0].rgba.b = dBLUE(color0); 
            pPrim->verts[1].rgba.b = dBLUE(color1); 
            pPrim->verts[2].rgba.b = dBLUE(color2);
            pPrim->verts[3].rgba.b = dBLUE(color3);
            
            pPrim->verts[0].rgba.a = dALPHA(color0); 
            pPrim->verts[1].rgba.a = dALPHA(color1); 
            pPrim->verts[2].rgba.a = dALPHA(color2);
            pPrim->verts[3].rgba.a = dALPHA(color3);
            
        }
        
        // ------------------------------------------
        
        inline void SetALPHA(LT_POLYGT4 *pPrim, uint8 alpha) {
            if (!pPrim) return;

            pPrim->verts[0].rgba.a = 
                    pPrim->verts[1].rgba.a = 
                        pPrim->verts[2].rgba.a =
                            pPrim->verts[3].rgba.a = alpha;     
        }
            
        inline void SetALPHA(LT_POLYG4 *pPrim, uint8 alpha) {
            if (!pPrim) return;

            pPrim->verts[0].rgba.a = 
                    pPrim->verts[1].rgba.a = 
                        pPrim->verts[2].rgba.a =
                            pPrim->verts[3].rgba.a = alpha;         
        }

        inline void SetALPHA(LT_POLYFT4 *pPrim, uint8 alpha) {
            if (!pPrim) return;

            pPrim->rgba.a = alpha;          
        }

        inline void SetALPHA(LT_POLYF4 *pPrim, uint8 alpha) {
            if (!pPrim) return;

            pPrim->rgba.a = alpha;      
        }
        
        // ------------------------------------------
        
        inline void SetALPHA4(LT_POLYGT4 *pPrim, 
                              uint8 alpha0, uint8 alpha1,
                              uint8 alpha2, uint8 alpha3) {
            if (!pPrim) return;

            pPrim->verts[0].rgba.a = alpha0;
            pPrim->verts[1].rgba.a = alpha1;
            pPrim->verts[2].rgba.a = alpha2;
            pPrim->verts[3].rgba.a = alpha3;            
        }
            
        inline void SetALPHA4(LT_POLYG4 *pPrim, 
                              uint8 alpha0, uint8 alpha1,
                              uint8 alpha2, uint8 alpha3) {
            if (!pPrim) return;

            pPrim->verts[0].rgba.a = alpha0;
            pPrim->verts[1].rgba.a = alpha1;
            pPrim->verts[2].rgba.a = alpha2;
            pPrim->verts[3].rgba.a = alpha3;            
        }
        
        
    protected:
        // the camera
        HOBJECT m_pCamera;
        // the texture
        HTEXTURE m_pTexture;
        // transformation that needs to occur
        ELTTransformType m_eTransType;
        // color operation
        ELTColorOp m_ColorOp;
        // blend mode
        ELTBlendMode m_BlendMode;
        // Z Buffer control
        ELTZBufferMode m_eZBufferMode;
        // Alpha Buffer control
        ELTTestMode m_eTestMode;
        // type of clipping to do
        ELTClipMode m_eClipType;
        // fillmode
        ELTDPFillMode m_eFillMode;
        // CullMode
        ELTDPCullMode m_eCullMode;
		// Really close
		bool		 m_bReallyClose;
		// Fog enable
		bool		m_bFogEnable;
		// Effect ID
		uint32		m_nEffectShaderID;
};

#endif
