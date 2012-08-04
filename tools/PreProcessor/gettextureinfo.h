
// This module sets up a list of texture info structures for each texture in a level,
// then you can search the list for info on each texture.

#ifndef __GETTEXTUREINFO_H__
#define __GETTEXTUREINFO_H__


	class TLightDef
	{
		public:

					TLightDef()
					{
						m_bCentered			= false;
						m_bDirectional		= false;
						m_bLightModels		= false;
						m_bClipLight		= true;
						m_bApproxLinear		= true;
						m_UOffset			= 0.0f;
						m_VOffset			= 0.0f;
						m_ZOffset			= 1.0f;
						m_Radius			= 0.0f;
						m_BrightScale		= 1.0f;
						m_FOVDegrees		= 90.0f;

						m_Coefficients.Init(1, 0, 19);
						m_InnerColor.Init(255, 255, 255);
					}

			//determines if this light should only be created at the center of
			//the polygon
			bool	m_bCentered;

			//determines if this texture light is an omni or directional light
			bool	m_bDirectional;

			//specifies the brightscale of this light
			PReal	m_BrightScale;

			//determine if the light should be clipped to geometry
			bool	m_bClipLight;

			//determines if the light should be added to the list of objects
			//to light models
			bool	m_bLightModels;

			//determines if the coefficients should be used, or the default
			//ones should be used to approximate linear lighting based upon
			//the radius
			bool	m_bApproxLinear;
			
			//the field of view if it is a directional light, in degrees
			PReal	m_FOVDegrees;

			//UV coordinates of lights.
			PReal	m_UOffset;
			PReal	m_VOffset;

			//the amount to extrude texture light along normal of polygon
			PReal	m_ZOffset;

			//the radius of the light. If the radius is 0, the coefficients should
			//be used insteal
			PReal	m_Radius;

			//the ABC coefficients of the texture light
			PVector	m_Coefficients;

			//the inner color of the light
			PVector	m_InnerColor;
	};


	class TInfo : public CGLLNode
	{
		public:

					TInfo()
					{
						m_pTextureName = "";
					}

					~TInfo()
					{
						DeleteAndClearArray(m_LightDefs);
					}

			uint32					m_Width, m_Height;
			uint16					m_Flags;
			char					*m_pTextureName;
			CMoArray<TLightDef*>	m_LightDefs;
	};


	// Builds the list of TInfos for the textures.
	uint32 GetWorldTextureInfo(CPreWorld *pWorld, CGLinkedList<TInfo*> &theList);
	uint32 GetTextureInfo(CPreMainWorld *pWorld, CGLinkedList<TInfo*> &theList);

	// Find a TInfo in the list.
	TInfo* FindTInfo(CGLinkedList<TInfo*> &theList, const char *pTextureName);


#endif  // __GETTEXTUREINFO_H__






