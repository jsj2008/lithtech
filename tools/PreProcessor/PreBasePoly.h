#ifndef __PREBASEPOLY_H__
#define __PREBASEPOLY_H__

 
	class CPolyVert

	{
		public:

			void operator=(const CPolyVert& other)
			{
				m_Vec		= other.m_Vec;
				m_Color		= other.m_Color;
				m_Alpha		= other.m_Alpha;
				m_Index		= other.m_Index;
				m_Normal	= other.m_Normal;
			}
		
			uint32		m_Index;		//the index into the worlds vertex list
			PVector		m_Vec;			//position of this vertex
			LTVector	m_Color;		//Color of this vertex RGB [0..255]
			PReal		m_Alpha;		//alpha of this vertex [0..255]
			PVector		m_Normal;		//the normal of this vertex
	};


	template<class T>
	class CPreBasePolyInitializer
	{
		public:

			static T* CreatePoly(uint32 nVerts, bool bSetNumVerts)
			{
				ASSERT(nVerts >= 3);

				uint32 size = sizeof(T) + sizeof(CPolyVert)*(nVerts-3);
				T *pRet = (T*)malloc(size);

				::new((DummyThingForConstructor*)NULL, pRet) T;
				
				if(pRet)
				{

				#ifdef _DEBUG
					pRet->m_BasePoly.m_nMaxVerts = nVerts;
				#endif

					if(bSetNumVerts)
						pRet->m_BasePoly.SetNumVerts((uint16)nVerts);
					else
						pRet->m_BasePoly.SetNumVerts(0);
				}
				
				return pRet;
			}

			static void FreePoly(T *pPoly)
			{
				pPoly->~T();
				free(pPoly);
			}

	};



	class CPreBasePoly
	{
		public:

			CPreBasePoly() : m_CenterFlags(0) {}
			~CPreBasePoly() {}

		public:

			// Accessors
			PVector&				PtNormal(uint32 i)		{ASSERT(i < m_nVerts); return m_Verts[i].m_Normal;}
			const PVector&			PtNormal(uint32 i) const{ASSERT(i < m_nVerts); return m_Verts[i].m_Normal;}
			LTVector&				Color(uint32 i)			{ASSERT(i < m_nVerts); return m_Verts[i].m_Color;}
			const LTVector&			Color(uint32 i)	const	{ASSERT(i < m_nVerts); return m_Verts[i].m_Color;}
			PVector&				Pt(uint32 i)			{ASSERT(i < m_nVerts); return m_Verts[i].m_Vec;}
			const PVector&			Pt(uint32 i) const		{ASSERT(i < m_nVerts); return m_Verts[i].m_Vec;}
			PReal&					Alpha(uint32 i)			{ASSERT(i < m_nVerts); return m_Verts[i].m_Alpha;}
			const PReal&			Alpha(uint32 i)	const	{ASSERT(i < m_nVerts); return m_Verts[i].m_Alpha;}

			PVector&				NextPt(uint32 i)			{return Pt((i+1) % m_nVerts);}
			const PVector&			NextPt(uint32 i) const		{return Pt((i+1) % m_nVerts);}
			PVector&				PrevPt(uint32 i)			{return Pt((i==0) ? (m_nVerts-1) : (i-1));}
			const PVector&			PrevPt(uint32 i) const		{return Pt((i==0) ? (m_nVerts-1) : (i-1));}
			PVector&				NextNextPt(uint32 i)		{return Pt((i+2) % m_nVerts);}
			const PVector&			NextNextPt(uint32 i) const	{return Pt((i+2) % m_nVerts);}

			uint32					NumVerts() const	{return m_nVerts;}
			void					SetVert(uint32 i, const PVector &val) {Pt(i) = val;}
			void					SetVert(uint32 i, const PVector &val, const PVector &vNormal) {Pt(i) = val; PtNormal(i) = vNormal;}

			virtual void			CopySplitAttributes(const CPreBasePoly *pInPoly) {}

			//directly accesses the center (assumes that it has already been calculated)
			const PVector &			GetCenterDirect() const
			{
				ASSERT(m_CenterFlags & KNOW_CENTER);
				return m_Center;
			}

			const PVector &			GetCenter() const
			{
				if ((m_CenterFlags & KNOW_CENTER) == 0)
					CalcCenter();

				return m_Center;
			}

			//directly accesses the center (assumes that it has already been calculated)
			PReal					GetRadiusDirect() const
			{
				ASSERT(m_CenterFlags & KNOW_RADIUS);
				return m_Radius;
			}

			PReal					GetRadius() const
			{
				if ((m_CenterFlags & KNOW_RADIUS) == 0)
					CalcRadius();

				return m_Radius;
			}

			void	ExtendBounds( PVector &min, PVector &max ) const
			{
				for(uint32 nCurrVert = 0; nCurrVert < NumVerts(); nCurrVert++)
				{
					if( Pt(nCurrVert).x < min.x	)	min.x = Pt(nCurrVert).x;
					if( Pt(nCurrVert).y < min.y	)	min.y = Pt(nCurrVert).y;
					if( Pt(nCurrVert).z < min.z	)	min.z = Pt(nCurrVert).z;

					if( Pt(nCurrVert).x > max.x )	max.x = Pt(nCurrVert).x;
					if( Pt(nCurrVert).y > max.y )	max.y = Pt(nCurrVert).y;
					if( Pt(nCurrVert).z > max.z )	max.z = Pt(nCurrVert).z;
				}
			}
			
			void	AddVert(const PVector &vec)
			{
				ASSERT(m_nVerts < m_nMaxVerts);
				// Clear the center knowledge flag
				m_CenterFlags = 0;

				m_Verts[m_nVerts].m_Vec = vec;
				m_nVerts++;
 
				

			}

			void	AddVert(const PVector &vec, const PVector &vNormal)
			{
				AddVert(vec);
				// Copy in the normal
				m_Verts[m_nVerts - 1].m_Normal = vNormal;
			}

			void	RemoveVert(uint32 index)
			{
				// Clear the center knowledge flag
				m_CenterFlags = 0;

				ASSERT(m_nVerts > 0);
				ASSERT(index < m_nVerts);
				memmove(&m_Verts[index], &m_Verts[index+1], sizeof(m_Verts[0])*((NumVerts()-index)-1));
				--m_nVerts;
			}

			uint32	FindIndex(const PVector &vec) const 
			{
				for(uint32 i = 0; i < NumVerts(); i++)
					if(Pt(i) == vec)
						return i;

				return BAD_INDEX;
			}
			
			// Force a re-calculation of the center and radius values
			void	FlushCenter() const
			{
				CalcCenter();
				CalcRadius();
			}

			//sets the number of vertices this polygon has
			void	SetNumVerts(uint16 nNumVerts)
			{
				ASSERT(nNumVerts <= m_nMaxVerts);
				m_nVerts = nNumVerts;
			}

		private:
			// Gets the approximate poly center (average of min/max dims)
			void CalcCenter() const
			{
				if (!NumVerts())
					return;

				PVector min, max;

				min = Pt(0);
				max = min;

				ExtendBounds(min, max);

				m_Center = (min + max) * 0.5f;

				m_CenterFlags |= KNOW_CENTER;
			}

			// Gets the radius of the polygon given the center
			void CalcRadius() const
			{
				if (!NumVerts())
					return;

				PVector vCenter = GetCenter();
				m_Radius = 0.0f;

				for(uint32 i=0; i < NumVerts(); i++)
				{
					float fDist = (Pt(i) - vCenter).MagSqr();
					if (fDist > m_Radius)
						m_Radius = fDist;
				}
				m_Radius = (float)sqrt(m_Radius);
				m_CenterFlags |= KNOW_RADIUS;
			}

			enum ECenterFlags {KNOW_NOTHING = 0, KNOW_CENTER = 1, KNOW_RADIUS = 2};
			mutable uint16		m_CenterFlags;

			mutable PVector		m_Center;
			mutable PReal		m_Radius;

			uint16		m_nVerts;
		public:

 
		#ifdef _DEBUG
			//used to make sure that no one exceeds the vertex size
			uint16		m_nMaxVerts;
		#endif

			CPolyVert	m_Verts[3];


	};

	#define BASEPOLY_MEMBER() \
		CPolyVert&			Vert(uint32 i) {return m_BasePoly.m_Verts[i];}\
		const CPolyVert&	Vert(uint32 i) const {return m_BasePoly.m_Verts[i];}\
		CPolyVert*			VertList() {return m_BasePoly.m_Verts;}\
		const CPolyVert*	VertList() const {return m_BasePoly.m_Verts;}\
		uint32				NumVerts() const {return m_BasePoly.NumVerts();}\
		PVector&			Pt(uint32 i) {return m_BasePoly.Pt(i);}\
		const PVector&		Pt(uint32 i) const {return m_BasePoly.Pt(i);}\
		PVector&			NextPt(uint32 i) {return m_BasePoly.NextPt(i);}\
		const PVector&		NextPt(uint32 i) const {return m_BasePoly.NextPt(i);}\
		PVector&			NextNextPt(uint32 i) {return m_BasePoly.NextNextPt(i);}\
		const PVector&		NextNextPt(uint32 i) const {return m_BasePoly.NextNextPt(i);}\
		PVector&			PrevPt(uint32 i) {return m_BasePoly.PrevPt(i);}\
		const PVector&		PrevPt(uint32 i) const {return m_BasePoly.PrevPt(i);}\
		LTVector&			Color(uint32 i) {return m_BasePoly.Color(i);}\
		const LTVector&		Color(uint32 i) const {return m_BasePoly.Color(i);}\
		PReal&				Alpha(uint32 i) {return m_BasePoly.Alpha(i);}\
		const PReal&		Alpha(uint32 i) const {return m_BasePoly.Alpha(i);}\
		PVector&			PtNormal(uint32 i) {return m_BasePoly.PtNormal(i);}\
		const PVector&		PtNormal(uint32 i) const {return m_BasePoly.PtNormal(i);}\
		void				AddVert(const PVector &vec) {m_BasePoly.AddVert(vec, Normal());}\
		void				AddVert(const PVector &vec, const PVector &vNormal) {m_BasePoly.AddVert(vec, vNormal);}\
		void				RemoveVert(uint32 i) {m_BasePoly.RemoveVert(i);}\
		void				SetVert(uint32 i, const PVector &val) {m_BasePoly.SetVert(i, val, Normal());}\
		void				SetVert(uint32 i, const PVector &val, const PVector &vNormal) {m_BasePoly.SetVert(i, val, vNormal);}\
		const PVector&		GetCenter() const {return m_BasePoly.GetCenter();}\
		PReal				GetRadius() const {return m_BasePoly.GetRadius();}\
		void				FlushCenter() const { m_BasePoly.FlushCenter();}\
		void				ExtendBounds(PVector &min, PVector &max) const {m_BasePoly.ExtendBounds(min, max);}\
		CPreBasePoly		m_BasePoly;


	// CPreBasePoly-derived classes must be created with these routines.
	// Use CreatePoly and FreePoly...
	template<class T>
	inline void InternalFreePoly(T *pPoly)
	{
		CPreBasePolyInitializer<T>::FreePoly(pPoly);
	}
	
	#define CreatePoly(type, nVerts, bSetNumVerts) \
		CPreBasePolyInitializer<type>::CreatePoly(nVerts, bSetNumVerts)
	
	#define DeletePoly(poly) \
		InternalFreePoly(poly)

#endif // __PREBASEPOLY_H__
			

