// ------------------------------------------------------------------------
// ps2renderstyle.h
// ------------------------------------------------------------------------
#ifndef __PS2RENDERSTYLE_H__
#define __PS2RENDERSTYLE_H__


#ifndef __RENDERSTYLE_H__
#include "renderstyle.h"
#endif

#include <list>


class CPS2RenderPass {
public:
	RenderPassOp m_RenderPass ;
};


class CPS2RenderStyle : public CRenderStyle {

public :

	
	// Get/Set RenderState Settings...
	// RenderStates...
	virtual void	SetClipMode(ERenStyle_ClipMode eMode)	;
	virtual ERenStyle_ClipMode		GetClipMode()			;
	// Lighting Material...
	virtual bool	SetLightingMaterial(LightingMaterial& LightMaterial)	;
	virtual bool	GetLightingMaterial(LightingMaterial* pLightMaterial)	;
	// RenderPasses...
	virtual bool	AddRenderPass(RenderPassOp& RenderPass)					;
	virtual bool	RemoveRenderPass(uint32 iPass)							;
	virtual bool	SetRenderPass(uint32 iPass,RenderPassOp& RenderPass)	;
	virtual bool	GetRenderPass(uint32 iPass,RenderPassOp* pRenderPass)	;
	virtual uint32	GetRenderPassCount()									;

	// Platform Options: Direct3D...
	virtual	bool	SetDirect3D_Options(RSD3DOptions& Options)				{ return false ; }
	virtual bool	GetDirect3D_Options(RSD3DOptions* pOptions)				{ return false ; } 
	virtual bool	SetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass) { return false ; }
	virtual bool	GetRenderPass_D3DOptions(uint32 iPass,RSD3DRenderPass* pD3DRenderPass) { return false ; }


	// Helper Functions...
	virtual bool	Compile() {return true ; }
	virtual void	SetDefaults();

//protected:
	// Render States...
	ERenStyle_ClipMode				m_ClipMode;
	// Lighting Material...
	LightingMaterial				m_LightingMaterial;
	// Render Passes...
	std::list<CPS2RenderPass>		m_RenderPasses;



};


#if(0)
// ------------------------------------------------------------------------
// PS2 RenderStyle parameters & Scope
// ------------------------------------------------------------------------
class CPS2RenderStyle {

public : // ENUMS
	enum EPS2TestModes		{ NOTEST, LE, LEQ, GE, GEQ,  EQ, NEQ } ;
	enum EPS2ClipModes     { NO_CLIP, CLIP } ;
	enum EPS2BlendModes    { DISABLE,
							CSPCD,        // Cs + Cd
							CSTASPCDT1MAS,// Cs * As + Cd * ( 1 - As )
							CSTAS,        // Cs * As 
							CSTASPCD	  // Cs* As + Cd 
	} ;
	enum EPS2CullMode      { NONE, CCW, CW };
	enum ELIGHTPROPFIELD { DIFFUSE, AMBIENT, EMISSIVE, SPECULAR, SPECPOW, ALPHA_MODE, BLEND_MODE, TEX_MODE, U, V };

	// texture uv map modes.
	enum ETextureUVMapType { REPEAT, CLAMP, REGION_CLAMP, REGION_REPEAT };
	// indexing help
	enum {	MIN_U = 0 , MIN_V = 0 , 
				MAX_U = 1, MAX_V =1 , 
				MASK_U = 0 , MASK_V = 0 ,
				FIX_U = 1, FIX_V = 1 };
		
	enum ETextureFunction { NOTEXFUNC, MODULATE, DECAL , HIGHLIGHT , HIGHLIGHT2 };

	enum ETextureFilter { POINT, BILINEAR };
	// flags
	enum EUVSourceFlags { SET0= 1<<0, MATRIX=1<<1, CS_NORMAL = 1<<2 };

public : // MEMBER CLASSES

	// ------------------------------------------------------------------------
	// Lighting properties
	// ------------------------------------------------------------------------
	// class CLightProps : pbulic  LightingMaterial
	class  CLightProps {
	public :

		CLightProps() {
			SetDefault() ;
		}

		float diffuse[4];
		float ambient[4];
		float emissive[4];
		float specular[4];
		float specpow ;

		inline void SetDefault();
	};


	// ------------------------------------------------------------------------
	// Texture parameters
	// ------------------------------------------------------------------------
	class CTextureParams {

	public :
		
		// wrap params
		class  CWrap {
		public :
			// default ctor
			CWrap() :m_MapType(REPEAT){}

			// ctor ( type, param1 , param2 )
			CWrap( ETextureUVMapType mt, int p1, int p2 )
				:m_MapType(mt) 
			{
				switch( m_MapType ) {
				case REPEAT:
				case CLAMP:
					break;
				case REGION_CLAMP:
					m_RegionParams.m_Clamp[0] = p1;
					m_RegionParams.m_Clamp[1] = p2;
					break;
				case REGION_REPEAT:
					m_RegionParams.m_Repeat[0] = p1;
					m_RegionParams.m_Repeat[1] = p2;
					break;
				
				}		
			}


			ETextureUVMapType m_MapType ;

			union URegionParams {
				int m_Clamp[2]; // MIN/ MAX
				int m_Repeat[2]; // Mask / Fix 

			} m_RegionParams;
		};

	public :
		// CTOR
		CTextureParams():	m_TextureIndexRef(RENDERSTYLE_NOTEXTURE), 
							m_TextureFunction(MODULATE),
							m_TextureFilter(RENDERSTYLE_TEXFILTER_BILINEAR),
							m_UVSourceFlags( SET0 ){}
		// ACCESSOR
		void SetTextureIndex( ERenStyle_TextureParam index ) { m_TextureIndexRef  = index ; } 
		void SetTextureFilter( ERenStyle_TexFilter etf ) { m_TextureFilter = etf ; }
		void SetTextureFunc( ETextureFunction etf ) { m_TextureFunction=etf ; }
		void SetUVSourceFlag( EUVSourceFlags euvsf ) { m_UVSourceFlags = euvsf ; }
		void SetWrap( int uv, CWrap & wrap ) { if(uv) m_V_WrapParams = wrap ; else m_U_WrapParams = wrap ; }

		void GetTextureIndex( ERenStyle_TextureParam &index ) { index= m_TextureIndexRef   ; } 
		void GetTextureFilter( ERenStyle_TexFilter & etf ) { etf = m_TextureFilter  ; }
		void GetTextureFunc( ETextureFunction &etf ) { etf=m_TextureFunction ; }
		void GetUVSourceFlag( EUVSourceFlags &euvsf ) { euvsf = m_UVSourceFlags ; }
		void GetWrap( int uv, CWrap & wrap ) { if(uv) wrap = m_V_WrapParams ; else wrap=m_U_WrapParams  ; }

		// MEMBERS
		ERenStyle_TextureParam   m_TextureIndexRef ; // which texture map
		CWrap                    m_U_WrapParams, m_V_WrapParams ;
		ETextureFunction         m_TextureFunction ;
		ERenStyle_TexFilter      m_TextureFilter ;
		EUVSourceFlags           m_UVSourceFlags ;
		
	};

	// ------------------------------------------------------------------------
	// The render pass params
	// ------------------------------------------------------------------------
	class CRenderPass {

	public:
		CRenderPass() :
			m_AlphaTestMode( RENDERSTYLE_NOALPHATEST ),
			m_ZBufferTestMode( LEQ ),	
			m_BlendMode     ( DISABLE ),
			m_PassID(0)
		{
		}

			int             m_PassID ;
		
		ERenStyle_TestMode m_AlphaTestMode ;
		EPS2TestModes   m_ZBufferTestMode ;
		EPS2BlendModes  m_BlendMode;
		CTextureParams  m_TextureParams ;
		

		void SetBlendMode( EPS2BlendModes bm ) { m_BlendMode = bm ;}
		void SetAlphaTestMode( ERenStyle_TestMode tm ) { m_AlphaTestMode = tm ; }
		void SetZTestMode( EPS2TestModes tm ) { m_ZBufferTestMode = tm ; }
		void SetPassID( int pass_id ) { m_PassID = pass_id ;}

		void GetBlendMode( EPS2BlendModes &bm ) { bm = m_BlendMode ;}
		void GetAlphaTestMode( ERenStyle_TestMode &tm ) { tm = m_AlphaTestMode ; }
		void GetZTestMode( EPS2TestModes &tm ) { tm = m_ZBufferTestMode ; }
		void GetPassID( int &pass_id ) { pass_id = m_PassID ;}
	};


	// number and array of renderpasses
	
	CRenderPass*	m_vRenderPasses;
	int				m_RenderPassesArraySize ;
	char*			m_name ;
	CLightProps     m_LightingProps ;
	EPS2ClipModes   m_ClipMode ;


public : // CLASS METHODS 

	CPS2RenderStyle():m_vRenderPasses(NULL), m_RenderPassesArraySize(0),m_name(NULL),	m_ClipMode( CLIP )
	{
	}

	// return false if alloc failed.
	bool AllocRenderPasses( int size ) ;
	int GetNumPasses();

	// light props
	void SetLightProps(  ELIGHTPROPFIELD prpfield, float, float, float, float);
	void SetLightProps( ELIGHTPROPFIELD prpfield, const float *);
	void GetLightProps( ELIGHTPROPFIELD prpfield, float *vec);

	//name
	inline void SetName( const char *name );
	inline const char*GetName();


	// PER BLEND PASS SETS
	// texture props (per pass )
	// set/get texture index 
	void SetTextureIndex( int pass_num, ERenStyle_TextureParam index );
	void GetTextureIndex( int pass_num, ERenStyle_TextureParam &idx );

	// set texture function
	void SetTextureFunction( int pass_num, ETextureFunction );
	void GetTextureFunction( int pass_num ,ETextureFunction &);
	// set texture filter
	void SetTextureFilter( int pass_num, ERenStyle_TexFilter etf);
	void GetTextureFilter( int pass_num, ERenStyle_TexFilter &etf);
	// set uv source
	void SetTextureUVSource( int pass_num, EUVSourceFlags uv_source_flag );
	void GetTextureUVSource( int pass_num,EUVSourceFlags &uv_source_flag );
	// set uv wrap 
	void SetTextureUVWrap( int pass_num, int uv, ETextureUVMapType style, int style_params1, int style_params2);
	void GetTextureUVWrap( int pass_num, int uv, ETextureUVMapType &style, int &style_params1, int &style_params2);

	// blend mode 
	void SetBlendMode( int pass_num, EPS2BlendModes);
	void GetBlendMode( int pass_num, EPS2BlendModes &);

	// zbuf & alpha test modes
	void SetZBufferTestMode( int pass_num, EPS2TestModes );
	void GetZBufferTestMode( int pass_num, EPS2TestModes & );
	void SetAlphaTestMode( int pass_num,  ERenStyle_TestMode );
	void GetAlphaTestMode( int pass_num,  ERenStyle_TestMode &);

};


// ------------------------------------------------------------------------
// ************************************************************************
// ------------------------------------------------------------------------
// I N L I N E S 
// ------------------------------------------------------------------------
// ************************************************************************
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// bool AllocRenderPasses( size )
// creates memory space for renderpasses. 
// return true =ok, false= fail
// ------------------------------------------------------------------------
inline bool CPS2RenderStyle::AllocRenderPasses( int num )
{
	if(m_vRenderPasses == NULL )
	{
		m_vRenderPasses = new CRenderPass [ num ];
		m_RenderPassesArraySize = num;

		if (m_vRenderPasses )
		{
			for( int i = 0 ;i < m_RenderPassesArraySize ; i++ )
				m_vRenderPasses[i].SetPassID( i );
			return true ;
		}
	}else
	{
		delete [] m_vRenderPasses ;
		m_vRenderPasses = NULL ;
		m_RenderPassesArraySize = 0;
		// ok try again
		return AllocRenderPasses(num);
	}
	return false;
}

inline int CPS2RenderStyle::GetNumPasses() { return m_RenderPassesArraySize ; } 


// texture props (per pass )
// set texture index 
inline void CPS2RenderStyle::SetTextureIndex( int pass_num, ERenStyle_TextureParam index )
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
	{
		m_vRenderPasses[pass_num].m_TextureParams.SetTextureIndex( index  );
	}
}
inline void CPS2RenderStyle::GetTextureIndex( int pass_num, ERenStyle_TextureParam &index )
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
	{
		m_vRenderPasses[pass_num].m_TextureParams.GetTextureIndex(index);
	}
}


// set texture function
inline void CPS2RenderStyle::SetTextureFunction( int pass_num, ETextureFunction etf)
{
		if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].m_TextureParams.SetTextureFunc(etf) ;
		}
}
// set texture function
inline void CPS2RenderStyle::GetTextureFunction( int pass_num, ETextureFunction &etf)
{
		if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].m_TextureParams.GetTextureFunc(etf) ;
		}
}

// set texture filter
inline void CPS2RenderStyle::SetTextureFilter( int pass_num, ERenStyle_TexFilter etf)
{
		if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].m_TextureParams.SetTextureFilter(etf) ;
		}
}
// set texture filter
inline void CPS2RenderStyle::GetTextureFilter( int pass_num, ERenStyle_TexFilter &etf)
{
		if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].m_TextureParams.GetTextureFilter(etf) ;
		}
}

// set uv source
inline void CPS2RenderStyle::SetTextureUVSource( int pass_num, EUVSourceFlags uv_source_flag )
{
		if( pass_num < m_RenderPassesArraySize && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].m_TextureParams.SetUVSourceFlag(uv_source_flag) ;
		}
}
// get uv source
inline void CPS2RenderStyle::GetTextureUVSource( int pass_num, EUVSourceFlags &uv_source_flag )
{
		if( pass_num < m_RenderPassesArraySize && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].m_TextureParams.GetUVSourceFlag(uv_source_flag) ;
		}
}

// set uv wrap 
inline void CPS2RenderStyle::SetTextureUVWrap( int pass_num, int uv, ETextureUVMapType style, int style_param1, int style_param2)
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			CTextureParams::CWrap wrap(style, style_param1,style_param2);
			m_vRenderPasses[pass_num].m_TextureParams.SetWrap(uv, wrap) ;
		}
}
// set uv wrap 
inline void CPS2RenderStyle::GetTextureUVWrap( int pass_num, int uv, ETextureUVMapType &style, int &style_param1, int &style_param2)
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			CTextureParams::CWrap wrap;
			m_vRenderPasses[pass_num].m_TextureParams.GetWrap(uv, wrap) ;
			style= wrap.m_MapType;
			style_param1= wrap.m_RegionParams.m_Clamp[0];
			style_param2 = wrap.m_RegionParams.m_Clamp[1] ;
		}
}

// blend mode 
inline void CPS2RenderStyle::SetBlendMode( int pass_num, EPS2BlendModes bm)
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].SetBlendMode( bm ) ;
		}
}

// blend mode 
inline void CPS2RenderStyle::GetBlendMode( int pass_num, EPS2BlendModes &bm)
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].GetBlendMode( bm ) ;
		}
}

// zbuf & alpha test modes
inline void CPS2RenderStyle::SetZBufferTestMode( int pass_num, EPS2TestModes tm)
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].SetZTestMode( tm ) ;
		}
}
// zbuf & alpha test modes
inline void CPS2RenderStyle::GetZBufferTestMode( int pass_num, EPS2TestModes &tm)
{
	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].GetZTestMode( tm ) ;
		}
}

inline void CPS2RenderStyle::SetAlphaTestMode( int pass_num,  ERenStyle_TestMode tm)
{	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].SetAlphaTestMode( tm ) ;
		}
}


inline void CPS2RenderStyle::GetAlphaTestMode( int pass_num,  ERenStyle_TestMode &tm)
{	if( pass_num < m_RenderPassesArraySize  && pass_num >= 0 )
		{
			m_vRenderPasses[pass_num].GetAlphaTestMode( tm ) ;
		}
}
// ------------------------------------------------------------------------
// 
// ------------------------------------------------------------------------
inline void CPS2RenderStyle::CLightProps::SetDefault()
{
	for(int i = 0 ; i < 3; i++ )
	{
		diffuse[i] = .8f;
		ambient[i] = .2f;
		emissive[i] = 0.0f ;
		specular[i] = 0.0f ;
	}

	diffuse[3] = 1.0f ;
	ambient[3] = 1.0f ;
	emissive[3]= 1.0f;
	specular[3]= 1.0f ;
	specpow = 0.0f ;
}

inline void CPS2RenderStyle::SetName( const char *name )
{
	if(m_name == NULL )
	{
		m_name = new char [ strlen(name) +1 ];
		strcpy( m_name , name );
	}else
	{
		delete [] m_name ;
		m_name = NULL ;
		SetName(name);
	}
}

inline const char *CPS2RenderStyle::GetName()
{
	return m_name;
}

inline void CPS2RenderStyle::SetLightProps( ELIGHTPROPFIELD prpfield, float a=0.0f, float b=0.0f, float c=0.0f, float d=1.0f)
{
	switch( prpfield )
	{
	case DIFFUSE : 
		m_LightingProps.diffuse[0] = a ; m_LightingProps.diffuse[1] = b; 
		m_LightingProps.diffuse[2] = c ; m_LightingProps.diffuse[3] = d ; 
		break;
	case AMBIENT :
		m_LightingProps.ambient[0] = a ; m_LightingProps.ambient[1] = b; 
		m_LightingProps.ambient[2] = c ; m_LightingProps.ambient[3] = d ; 
		break;
	case EMISSIVE:
		m_LightingProps.emissive[0] = a ; m_LightingProps.emissive[1] = b; 
		m_LightingProps.emissive[2] = c ; m_LightingProps.emissive[3] = d ; 
		break;
	case SPECULAR :
		m_LightingProps.specular[0] = a ; m_LightingProps.specular[1] = b; 
		m_LightingProps.specular[2] = c ; m_LightingProps.specular[3] = d ; 
		break;
	case SPECPOW :
		m_LightingProps.specpow = a ;
	default :
		// error 
		break ;
	}
}


inline void CPS2RenderStyle::SetLightProps( ELIGHTPROPFIELD prpfield, const float *vec)
{
	switch( prpfield )
	{
	case DIFFUSE : 
		memcpy( m_LightingProps.diffuse, vec, sizeof(float)*4);
		break;
	case AMBIENT :
		memcpy( m_LightingProps.ambient, vec, sizeof(float)*4);
		break;
	case EMISSIVE:
		memcpy( m_LightingProps.emissive, vec, sizeof(float)*4);
		break;
	case SPECULAR :
		memcpy( m_LightingProps.specular, vec, sizeof(float)*4);
		break;
	case SPECPOW :
		m_LightingProps.specpow = vec[0] ;
	default :
		// error 
		break ;
	}
}


inline void CPS2RenderStyle::GetLightProps( ELIGHTPROPFIELD prpfield, float *vec)
{
	switch( prpfield )
	{
	case DIFFUSE : 
		memcpy(vec, m_LightingProps.diffuse,  sizeof(float)*4);
		break;
	case AMBIENT :
		memcpy( vec,  m_LightingProps.ambient,sizeof(float)*4);
		break;
	case EMISSIVE:
		memcpy(vec, m_LightingProps.emissive,  sizeof(float)*4);
		break;
	case SPECULAR :
		memcpy(vec, m_LightingProps.specular,  sizeof(float)*4);
		break;
	case SPECPOW :
		vec[0] = m_LightingProps.specpow  ;
	default :
		// error 
		break ;
	}
}

#endif
#endif