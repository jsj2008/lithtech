// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATIONSTD_H__
#define __ANIMATIONSTD_H__

// Strings

namespace Animation
{
	class CStr
	{
		public :

			// Ctors/Dtors/etc

			CStr()
			{
				m_szString[0] = 0;
			}

			CStr(const char* szString)
			{
				strcpy(m_szString, szString);
			}

			// Casting
			//
			// Note: casting into varargs must be made explicit (ie, printf, etc)
			// Not doing so will crash the c runtime.

			operator const char*() const
			{
				return m_szString;
			}

			operator char*()
			{
				return m_szString;
			}

			// Concatenation

            void operator+=(const CStr& sStr)
			{
				strcat(m_szString, sStr);
			}

			CStr operator+(const CStr& sStr) const
			{
				CStr sStrNew;
				sprintf(sStrNew, "%s%s", m_szString, (const char*)sStr);
				return sStrNew;
			}

			CStr operator+(const char* szStr) const
			{
				CStr sStrNew;
				sprintf(sStrNew, "%s%s", m_szString, szStr);
				return sStrNew;
			}

			// Case sensitive compare

			operator==(const CStr& sStr) const
			{
				return !strcmp(m_szString, sStr.m_szString);
			}

			operator==(const char* szStr) const
			{
				return !strcmp(m_szString, szStr);
			}

			// Case insensitive compare

			operator^=(const CStr& sStr) const
			{
				return !stricmp(m_szString, sStr.m_szString);
			}

			operator^=(const char* szStr) const
			{
				return !stricmp(m_szString, szStr);
			}

		private :

			friend class CStr;

			char m_szString[64];
	};

	// Stack

	template <class TYPE, int32 SIZE>
	class CStack
	{
		public :

			inline CStack();

			inline void Push(TYPE* pTYPE);
			inline TYPE* Pop();

			inline TYPE* GetTop();

			inline void Clear() { m_iTYPE = 0; }

			inline LTBOOL IsEmpty() { return m_iTYPE == 0; }

			inline int32 GetSize() { return m_iTYPE; }

		protected :

			int32	m_iTYPE;		// The next free TYPE
			TYPE*	m_aTYPEs[SIZE];	// The array of types
	};

	// Methods

	template <class TYPE, int32 SIZE>
	inline CStack<TYPE, SIZE>::CStack()
	{
		m_iTYPE = 0;
	}

	template <class TYPE, int32 SIZE>
	inline void CStack<TYPE, SIZE>::Push(TYPE* pTYPE)
	{
		m_aTYPEs[m_iTYPE++] = pTYPE;
	}

	template <class TYPE, int32 SIZE>
	inline TYPE* CStack<TYPE, SIZE>::Pop()
	{
		return m_aTYPEs[--m_iTYPE];
	}

	// Map

	template <class t_ClsKey, class t_ClsValue>
	struct FnForEach
	{
        virtual void operator()(const t_ClsKey& Key, t_ClsValue Value) = 0;
	};

	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	class CMap
	{
		public :

			// Functors

			struct FnHash
			{
				virtual uint32 Hash(const t_ClsKey& Key) const = 0;
			};

			struct FnCompare
			{
				virtual LTBOOL Compare(const t_ClsKey& Key1, const t_ClsKey& Key2) const = 0;
			};

		public :

			// Ctors/Dtors

			CMap();
			~CMap();

			// Methods

           virtual const FnHash& GetFnHash() const = 0;
           virtual const FnCompare& GetFnCompare() const = 0;
			t_ClsValue* Add(const t_ClsKey& clsKey, const t_ClsValue& clsValue);
			void Remove(const t_ClsKey& clsKey);
			LTBOOL Find(const t_ClsKey& clsKey, t_ClsValue** ppclsValue = LTNULL) const;
			void Clear();
			void ForEach(FnForEach<t_ClsKey, t_ClsValue>& fnForEach);

		protected :

			class CMapEntry
			{
				public :

					CMapEntry*	m_pNext;
					t_ClsValue	m_Value;
					t_ClsKey	m_Key;

					CMapEntry()
					{
						m_pNext = LTNULL;
					}
			};

		protected :

			CMapEntry*	m_apEntries[t_nSize];
	};

	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	CMap<t_ClsKey, t_ClsValue, t_nSize>::CMap()
	{
		memset(m_apEntries, LTNULL, sizeof(CMapEntry*)*t_nSize);
	}


	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	CMap<t_ClsKey, t_ClsValue, t_nSize>::~CMap()
	{
		for ( uint32 iEntry = 0 ; iEntry < t_nSize ; iEntry++ )
		{
			CMapEntry* pEntry = m_apEntries[iEntry];
			while ( pEntry )
			{
				CMapEntry* pEntryNext = pEntry->m_pNext;
				debug_delete(pEntry);
				pEntry = pEntryNext;
			}
		}
	}

	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	t_ClsValue* CMap<t_ClsKey, t_ClsValue, t_nSize>::Add(const t_ClsKey& clsKey, const t_ClsValue& clsValue)
	{
		uint32 iEntry = GetFnHash().Hash(clsKey)%t_nSize;
		CMapEntry* pEntry = debug_new(CMapEntry);
		pEntry->m_pNext = m_apEntries[iEntry];
		pEntry->m_Key = clsKey;
		pEntry->m_Value = clsValue;
		m_apEntries[iEntry] = pEntry;
		return &pEntry->m_Value;
	}

	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	void CMap<t_ClsKey, t_ClsValue, t_nSize>::Remove(const t_ClsKey& clsKey)
	{
		uint32 iEntry = GetFnHash().Hash(clsKey)%t_nSize;
		CMapEntry* pEntry = m_apEntries[iEntry];
		CMapEntry* pEntryPrev = LTNULL;
		while ( pEntry )
		{
			if ( GetFnCompare().Compare(pEntry->m_Key, clsKey) )
			{
				if ( pEntryPrev )
				{
					pEntryPrev->m_pNext = pEntry->m_pNext;
				}
				else
				{
					m_apEntries[iEntry] = pEntry->m_pNext;
				}

				debug_delete(pEntry);

				return;
			}

			pEntryPrev = pEntry;
			pEntry = pEntry->m_pNext;
		}
	}

	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	void CMap<t_ClsKey, t_ClsValue, t_nSize>::ForEach(FnForEach<t_ClsKey, t_ClsValue>& fnForEach)
	{
		for ( uint32 iEntry = 0 ; iEntry < t_nSize ; iEntry++ )
		{
			CMapEntry* pEntry = m_apEntries[iEntry];
			while ( pEntry )
			{
				fnForEach(pEntry->m_Key, pEntry->m_Value);
				pEntry = pEntry->m_pNext;
			}

		}
	}

	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	LTBOOL CMap<t_ClsKey, t_ClsValue, t_nSize>::Find(const t_ClsKey& clsKey, t_ClsValue** ppclsValue /* = LTNULL */) const
	{
		uint32 iEntry = GetFnHash().Hash(clsKey)%t_nSize;
		CMapEntry* pEntry = m_apEntries[iEntry];
		while ( pEntry )
		{
			if ( GetFnCompare().Compare(pEntry->m_Key, clsKey) )
			{
				if ( ppclsValue )
				{
					*ppclsValue = &pEntry->m_Value;
				}

				return LTTRUE;
			}
			pEntry = pEntry->m_pNext;
		}

		return LTFALSE;
	}

	template <class t_ClsKey, class t_ClsValue, uint32 t_nSize>
	void CMap<t_ClsKey, t_ClsValue, t_nSize>::Clear()
	{
		for ( uint32 iEntry = 0 ; iEntry < t_nSize ; iEntry++ )
		{
			CMapEntry* pEntry = m_apEntries[iEntry];
			while ( pEntry )
			{
				CMapEntry* pEntryNext = pEntry->m_pNext;
				debug_delete(pEntry);

				pEntry = pEntryNext;
			}
			m_apEntries[iEntry] = LTNULL;
		}
	}

	template <uint32 t_nSize, LTBOOL t_bCaseInsensitive = LTTRUE>
	class CMapStrStr : public CMap<CStr, CStr, t_nSize>
	{
		private :

			// Functors

			struct FnHashCStr : public CMap<CStr, CStr, t_nSize>::FnHash
			{
				uint32 Hash(CStr const& sString) const
				{
					return t_bCaseInsensitive ? toupper(sString[0]) : sString[0];
				}
			};

			struct FnCompareCStr : public CMap<CStr, CStr, t_nSize>::FnCompare
			{
				LTBOOL Compare(CStr const& sString1, CStr const& sString2) const
				{
					return !t_bCaseInsensitive ? !strcmp(sString1, sString2) : !stricmp(sString1, sString2);
				}
			};

		public :

			// Methods

            const CMap<CStr, CStr, t_nSize>::FnHash& GetFnHash() const
			{
				static FnHashCStr fnHashCStr;
				return fnHashCStr;
			}

            const CMap<CStr, CStr, t_nSize>::FnCompare& GetFnCompare() const
			{
				static FnCompareCStr fnCompareCStr;
				return fnCompareCStr;
			}
	};

	template <class t_Cls, uint32 t_nSize, LTBOOL t_bCaseInsensitive = LTTRUE>
	class CMapStrCls : public CMap<CStr, t_Cls, t_nSize>
	{
		private :

			// Functors

			struct FnHashCStr : public CMap<CStr, t_Cls, t_nSize>::FnHash
			{
                uint32 Hash(CStr const& sString) const
				{
                    LPCTSTR temp = sString;
                    return t_bCaseInsensitive ? toupper(temp[0]) : temp[0];
				}
			};

			struct FnCompareCStr : public CMap<CStr, t_Cls, t_nSize>::FnCompare
			{
				LTBOOL Compare(CStr const& sString1, CStr const& sString2) const
				{
					return !t_bCaseInsensitive ? !strcmp(sString1, sString2) : !stricmp(sString1, sString2);
				}
			};

		public :

			// Methods

            const CMap<CStr, t_Cls, t_nSize>::FnHash& GetFnHash() const
			{
				static FnHashCStr fnHashCStr;
				return fnHashCStr;
			}

            const CMap<CStr, t_Cls, t_nSize>::FnCompare& GetFnCompare() const
			{
				static FnCompareCStr fnCompareCStr;
				return fnCompareCStr;
			}
	};
};

#endif