#include "s_dx8.h"

#define CHUNK_ID_RIFF			0x46464952	// 'RIFF'
#define CHUNK_ID_WAVE			0x45564157	// 'WAVE'
#define CHUNK_ID_FACT			0x74636166	// 'fact'
#define CHUNK_ID_DATA			0x61746164	// 'data'
#define CHUNK_ID_WAVH			0x68766177	// 'wavh'
#define CHUNK_ID_FMT			0x20746d66	// 'fmt'
#define CHUNK_ID_GUID			0x64697567	// 'guid'

bool ParseWaveFile( void* pWaveFileBlock, void*& rpWaveFormat, uint32& ruiWaveFormatSize,
	void*& rpSampleData, uint32& ruiSampleDataSize )
{
	
	ruiSampleDataSize = ruiWaveFormatSize = 0;
	rpSampleData = rpWaveFormat = NULL;

	uint8* pucFileBlock = ( uint8* )pWaveFileBlock;

	bool bFinished = false;
	while( !bFinished )
	{

		uint32 uiChunkId = *( ( uint32* )pucFileBlock );
		pucFileBlock += sizeof( uint32 );

		switch( uiChunkId )
		{
			case CHUNK_ID_RIFF:
			{
				uint32 uiRiffSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );
				break;
			}

			case CHUNK_ID_WAVE:
			{
				// skip this
				break;
//				uint32 uiFmtWord = *( ( uint32* )pucFileBlock );
//				pucFileBlock += sizeof( uint32 );

			}

			case CHUNK_ID_FACT:
			{
				uint32 uiFactSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				uint32* puiFactData = ( uint32* )pucFileBlock;
				pucFileBlock += uiFactSize;
				break;
			}

			case CHUNK_ID_DATA:
			{
				uint32 uiDataSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				rpSampleData = ( void* )pucFileBlock;
				ruiSampleDataSize = uiDataSize;
				pucFileBlock += uiDataSize;

				bFinished = true;
				break;
			}

			case CHUNK_ID_WAVH:
			case CHUNK_ID_GUID:
			{
				// just skip these
				uint32 uiSkipSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				uint32* pucSkipData = ( uint32* )pucFileBlock;
				pucFileBlock += uiSkipSize;
				break;
			}

			case CHUNK_ID_FMT:
			{
				uint32 uiFmtSize = *( ( uint32* )pucFileBlock );
				pucFileBlock += sizeof( uint32 );

				rpWaveFormat = ( void* )pucFileBlock;
				ruiWaveFormatSize = uiFmtSize;
				pucFileBlock += uiFmtSize;
				break;
			}

			default:
			{
				return false;
			}
		}
	}

	bool bSuccess = ( ( rpWaveFormat != NULL ) && ( rpSampleData != NULL ) );
	return bSuccess;

}
