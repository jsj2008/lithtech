//------------------------------------------------------------------
//
//   MODULE  : CLIENTFX.H
//
//   PURPOSE : Defines class CClientFX
//
//   CREATED : On 1/25/99 At 3:57:10 PM
//
//------------------------------------------------------------------

#ifndef __CLIENTFX_H_
	#define __CLIENTFX_H_

	// Includes....

    #include "ltengineobjects.h"

	class CClientFX : public BaseClass
	{
		public :

			// Constructor

									CClientFX();

			// Destructor

									~CClientFX();

		protected :

			// Member Functions

            uint32                  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		private :

			void					SendFXMessage();
            void                    Save(HMESSAGEWRITE hWrite, uint32 dwFlags);
            void                    Load(HMESSAGEREAD hRead, uint32 dwFlags);


			HSTRING					m_hstrFxName;
			BOOL					m_bLoop;
	};

#endif