
#include "stdafx.h"
#include "TimedText.h"
#include "GameClientShell.h"

// uncomment the next line if you want to do some extra debugging
//#define TIMEDTEXT_DEBUG

namespace 
{
	float const ALPHA_OPAQUE = 255.0;  // this stays here
};


///
// CTimedText::~CTimedText()
//
CTimedText::~CTimedText()
{
}


///
// CTimedText::CTimedText()
//
CTimedText::CTimedText()
{
	m_hTextDisplaySound = LTNULL;							

	// initialize the internally used variables
	InitInternalInfo();	
}


///
// CTimedText::Init()
//
void CTimedText::Init( TIMED_TEXT_INIT_STRUCT const &initData )
{
	// initialize the internally needed info
	InitInternalInfo();
	
	//
	// extract the data from the initialization struct
	//
	
	// Visual Characteristics 
	ASSERT( initData.text );
	m_text = initData.text;
	m_color = ( initData.color & 0x00FFFFFF );
	
	// get the text's position, cause we may change this
	// as it scrolls
	m_text->GetPosition( &m_posX, &m_posY );
	
	// use dropped shadow?
	if ( initData.useDroppedShadow )
	{
		SETFLAG( m_flags, CTIMEDTEXTFLAG_USE_DROPPED_SHADOW );
	}
	else
	{
		CLEARFLAG( m_flags, CTIMEDTEXTFLAG_USE_DROPPED_SHADOW );
	}
	
	if ( initData.clipRect )
	{
		m_clipRect = *initData.clipRect;
		SETFLAG( m_flags, CTIMEDTEXTFLAG_USE_CLIP_RECT );
	}
	else
	{
		// clear the clip rect if we're not using it
		m_clipRect.x =
		m_clipRect.y = 
		m_clipRect.height = 
		m_clipRect.width = 0;
		CLEARFLAG( m_flags, CTIMEDTEXTFLAG_USE_CLIP_RECT );
	}
	
	m_textLen = strlen( m_text->GetText() );
	m_numberOfLinesBeforeScroll = initData.numberOfLinesBeforeScroll;
		
	// Timing Information
	m_initialDelay = initData.initialDelay;
	ASSERT( m_initialDelay >= 0 );
	
	m_characterDelay = initData.characterDelay;
	ASSERT( m_characterDelay >= 0 );
	
	m_lineDelay = initData.lineDelay;
	ASSERT( m_lineDelay >= 0 );
	
	m_scrollTime = initData.scrollTime;
	ASSERT( m_scrollTime >= 0 );
	
	m_completeDelay = initData.completeDelay;
	ASSERT( m_completeDelay >= 0 );
	
	m_fadeTime = initData.fadeTime;
	ASSERT( m_fadeTime >= 0 );
	
	
	//
	// Sound Information
	//
	
	// get character complete sound name (if any)
	if ( initData.textDisplaySound && 
	     initData.textDisplaySound[ 0 ] != '\0' )
	{
		strncpy( m_textDisplaySoundName, initData.textDisplaySound, TIMED_TEXT_SOUND_NAME_LENGTH );
	}
	
	// get scrolling sound name (if any)
	if ( initData.scrollSound &&
	     initData.scrollSound[ 0 ] != '\0' )
	{
		strncpy( m_scrollSoundName, initData.scrollSound, TIMED_TEXT_SOUND_NAME_LENGTH );
	}
	
	#ifdef TIMEDTEXT_DEBUG
	{
		// make sure strings terminate
		int i;
		LTBOOL charEndFound = LTFALSE;
		LTBOOL scrollEndFound = LTFALSE;
		
		for ( i = 0,
				charEndFound = LTFALSE,
				scrollEndFound = LTFALSE;
			  ( ( i < TIMED_TEXT_SOUND_NAME_LENGTH ) &&
			    ( ( !charEndFound ) ||
			      ( !scrollEndFound ) ) );
			  ++i )
		{
			if ( ( !charEndFound ) && ( m_textDisplaySoundName[ i ] == '\0' ) )
			{
				charEndFound = LTTRUE;
			}
			if ( ( !scrollEndFound ) && ( m_scrollSoundName[ i ] == '\0' ) )
			{
				scrollEndFound = LTTRUE;
			}
		}
		
		// assert if no end if found
		ASSERT( charEndFound );
		ASSERT( scrollEndFound );
	}
	#endif // TIMEDTEXT_DEBUG

	// now we're in the text writing phase
	m_state = CTIMEDTEXT_INITIAL_DELAY;
}


///
// CTimedText::Update()
//
void CTimedText::Update()
{
	// bail if we are not set up (current state == none),
	//    or if we are finished
	//    or if we are not running
	if ( ( m_state == CTIMEDTEXT_NONE ) ||
	     ( m_state == CTIMEDTEXT_FINISHED ) ||
	       !TESTFLAG( m_flags, CTIMEDTEXTFLAG_RUNNING ) )
	{
		return;
	}

	// update the elapsed time		
	float currentUpdateTime;
	currentUpdateTime = g_pLTClient->GetTime();
	m_timeElapsed += currentUpdateTime - m_prevUpdateTime;
	m_prevUpdateTime = currentUpdateTime;
		
	// check if we're just waiting to start
	if ( m_state == CTIMEDTEXT_INITIAL_DELAY )
	{
		if ( m_timeElapsed < m_initialDelay )
		{
			// still waiting, don't do anything
			return;
		}

		// no longer waiting, change to next state
		m_state = CTIMEDTEXT_TIMING;
		m_timeElapsed = m_timeElapsed - m_initialDelay;
	}
	
	// determine how much text we've done so far
	float processedTime;
	if ( m_state == CTIMEDTEXT_TIMING )
	{
		// the processed time is important for timing text
		// because it determines what position we have reached
		// in the text (how much text is revealed)
		processedTime = ( m_charactersProcessed - m_nNewlinesProcessed ) * m_characterDelay;
		
		// Get the minimum of number of lines before scrolling
		// or the number of lines we've processed.  This will be
		// the number of line delays to use.
		float numberOfNonScrolls;
		if ( m_linesProcessed > ( m_numberOfLinesBeforeScroll - 1 ) )
		{
			numberOfNonScrolls = (float)( m_numberOfLinesBeforeScroll - 1 );
		}
		else
		{
			numberOfNonScrolls = m_linesProcessed;
		}
		
		// add the proper number of line delays to the processed time
		processedTime += numberOfNonScrolls * m_lineDelay;

		// determine how many times we've scrolled so far
		float numberOfScrolls;
		numberOfScrolls = m_linesProcessed - (float)( m_numberOfLinesBeforeScroll - 1 );
		if ( numberOfScrolls > 0 )
		{
			processedTime += numberOfScrolls * m_scrollTime;
		}
	}
	else
	{
		// the other states are easier cause they are 
		// just straight timers and we don't have to know
		// where we are in the text
		processedTime = m_timeElapsed;
	}

	// determine where we currently are in the text
	int textPos;
	textPos = m_charactersProcessed;
	ASSERT( textPos <= m_textLen );
	
	//
	// bring our text up to date
	//
	
	// while processed time is less than true elapsed time
	while (processedTime <= m_timeElapsed )
	{
		// *check state
		if ( m_state == CTIMEDTEXT_TIMING )
		{
			if ( TESTFLAG( m_flags, CTIMEDTEXTFLAG_SCROLLING ) )
			{
				//
				// the text is currently scrolling
				//

				float timeLeft;
				timeLeft = m_timeElapsed - m_scrollRefTime;
				uint8 textHeight;
				textHeight = m_text->GetCharScreenHeight();
				if ( timeLeft < m_scrollTime )
				{
					// move text to proper position
					m_posY = m_scrollRefPosY - ( textHeight * timeLeft / m_scrollTime );

#ifdef TIMED_TEXT_FADE_TOP_LINE
					// fade top line by appropiate amount
					for ( int ii = m_nLineFadeCharStart; ii < m_nLineFadeCharEnd; ++ii )
					{
						 g_pLTClient->GetDrawPrim()->SetALPHA( &m_text->GetPolys()[ ii ], ( uint8 ) ( ALPHA_OPAQUE - ALPHA_OPAQUE * timeLeft / m_scrollTime ) );
					}
#endif // TIMED_TEXT_FADE_TOP_LINE
					
					break;
				}
				else
				{
					// move text to the final position
					m_posY = m_scrollRefPosY - textHeight;

					// increment the current time
					processedTime = m_scrollRefTime + m_scrollTime;
					
					// update our line count
					++m_linesProcessed;
				
#ifdef TIMED_TEXT_FADE_TOP_LINE
					// fade line completely
					for ( int ii = m_nLineFadeCharStart; ii < m_nLineFadeCharEnd; ++ii )
					{
						g_pLTClient->GetDrawPrim()->SetALPHA( &m_text->GetPolys()[ ii ], 0 );
					}
					
					// the next line to fade starts where this one ended
					m_nLineFadeCharStart = m_nLineFadeCharEnd;
#endif // TIMED_TEXT_FADE_TOP_LINE					
					
					// reset our references
					m_scrollRefPosY = 0;
					m_scrollRefTime = 0;
					
					// remember the new line height
					m_fLineYPos = m_text->GetPolys()[ textPos ].verts[ 0 ].y;
					
					// done scrolling
					CLEARFLAG( m_flags, CTIMEDTEXTFLAG_SCROLLING );
				}
			}
			// if we are done displaying the text
			else if ( textPos >= m_textLen )
			{
				// reset time elapsed
				m_timeElapsed -= processedTime;
				processedTime = 0;
				
				if ( m_hTextDisplaySound != LTNULL )
				{
					// kill the looping typing sound
					g_pLTClient->SoundMgr()->KillSound( m_hTextDisplaySound );
					m_hTextDisplaySound = LTNULL;							
				}

				// switch current state to complete
				m_state = CTIMEDTEXT_COMPLETE;
				
				ASSERT( m_timeElapsed >= 0 );
			}
			// else if the text at the current position is a linefeed
			else if ( ( m_text->GetPolys()[ textPos ].verts[ 0 ].y - m_fLineYPos ) > 0.5f ) 
			{
				// The only way we know we have a new line is if the polygon
				// has a different y value than the one before it.
				
				if ( m_linesProcessed >= ( m_numberOfLinesBeforeScroll - 1 ) )
				{				
					if ( !TESTFLAG( m_flags, CTIMEDTEXTFLAG_SCROLLING ) )
					{
						// set up to scroll
						SETFLAG( m_flags, CTIMEDTEXTFLAG_SCROLLING );
						m_scrollRefTime = processedTime;
						m_scrollRefPosY = m_posY;

#ifdef TIMED_TEXT_FADE_TOP_LINE
						// to fade the top line of text, start from where
						// the line starts and scan for end of line
						for ( m_nLineFadeCharEnd = m_nLineFadeCharStart + 1;
								( ( m_text->GetPolys()[ m_nLineFadeCharEnd ].verts[ 0 ].y - m_text->GetPolys()[ m_nLineFadeCharStart ].verts[ 0 ].y ) < 0.5f );
								++m_nLineFadeCharEnd )
						{
						}
#endif // TIMED_TEXT_FADE_TOP_LINE

						if ( m_hTextDisplaySound != LTNULL )
						{
							// kill the looping typing sound
		    	            g_pLTClient->SoundMgr()->KillSound( m_hTextDisplaySound );
	    	    	        m_hTextDisplaySound = LTNULL;							
						}
						
						// play the scroll sound
						if ( m_scrollSoundName[ 0 ] != '\0' )
						{
							g_pClientSoundMgr->PlayInterfaceSound( m_scrollSoundName );
						}
					}
				}
				else
				{
					// increment the current time
					processedTime += m_lineDelay;
					
					// update our line count
					++m_linesProcessed;

					m_fLineYPos = m_text->GetPolys()[ textPos ].verts[ 0 ].y;
					
					if ( m_hTextDisplaySound != LTNULL )
					{
						// kill the looping typing sound
	    	            g_pLTClient->SoundMgr()->KillSound( m_hTextDisplaySound );
    	    	        m_hTextDisplaySound = LTNULL;							
					}
				}
				
			}
			else if ( m_text->GetText()[ textPos ] == '\n' )
			{
				// Ignore newlines completely.  We want to keep track of them so
				// we know where we are in the text, but we won't do any
				// of the timing delay calculations with them.
				++m_charactersProcessed;
				++m_nNewlinesProcessed;
				++textPos;
			}
			// else if the text at the current position is printable
			else
			{
				// play the sound
				if ( ( m_hTextDisplaySound == LTNULL ) &&
				     ( m_textDisplaySoundName[ 0 ] != '\0' ) )
				{
					m_hTextDisplaySound = g_pClientSoundMgr->PlayInterfaceSound( m_textDisplaySoundName, ( PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT ) );
				}
				
				// update our character count
				++m_charactersProcessed;
				++textPos;
								
				// increment the current time
				processedTime += m_characterDelay;
			}
			
		}
		// else if complete
		else if ( m_state == CTIMEDTEXT_COMPLETE )
		{
			// *check to switch to next state
			// if current time greater than delay
			if ( processedTime >= m_completeDelay )
			{
				// reset time elapsed
				m_timeElapsed -= processedTime;
				processedTime = 0;
				
				// switch current state to fade
				m_state = CTIMEDTEXT_FADING;
			}
			else
			{
				break;
			}
		}
		// else if fading
		else if ( m_state == CTIMEDTEXT_FADING )
		{
			// *check to switch to next state
			// if current time is greater than fade time
			if ( processedTime >= m_fadeTime )
			{
				// switch current state to finished
				m_state = CTIMEDTEXT_FINISHED;
			}
			else
			{
				//TEMP: turn the text to black
				break;
			}
		}
		// else if finished
		else
		{
			CLEARFLAG( m_flags, ( CTIMEDTEXTFLAG_RUNNING | CTIMEDTEXTFLAG_DISPLAY ) );
			
			// DONE
			break;
		}
	}
	
	#ifdef TIMEDTEXT_DEBUG
		// put this here to make debugging easier
		// if a lot of time is spent in the debugger
		// in the update loop, we won't count that
		// time against the timed text
		m_prevUpdateTime = g_pLTClient->GetTime();
	#endif // TIMEDTEXT_DEBUG
}

///
// CTimedText::Render()
//
void CTimedText::Render()
{
	// bail if we are not set up (current state == none),
	//    or if we are finished
	//    or if we are not running
	if ( ( m_state == CTIMEDTEXT_NONE ) ||
		 ( m_state == CTIMEDTEXT_INITIAL_DELAY ) ||
	     ( m_state == CTIMEDTEXT_FINISHED ) ||
	       !TESTFLAG( m_flags, CTIMEDTEXTFLAG_DISPLAY ) )
	{
		return;
	}

	// if we are fading, setup the alpha
	uint8 alpha;
	alpha = ( uint8 ) ALPHA_OPAQUE;
	
	int startChar;
	int endChar;
	
	startChar = 0;
	endChar = 0;

	if ( m_state == CTIMEDTEXT_TIMING )
	{
		endChar = m_charactersProcessed;
	}
	else if ( m_state == CTIMEDTEXT_COMPLETE )
	{
		// just display everything like normal
		startChar = 0;
		endChar = m_textLen;
	}
	else if ( m_state == CTIMEDTEXT_FADING )
	{
		// just display everything like normal
		startChar = 0;
		endChar = m_textLen;
		
		alpha = ( uint8 ) (ALPHA_OPAQUE - ALPHA_OPAQUE * ( m_timeElapsed / m_fadeTime ));
	}
	else
	{
		// we are in the wrong state and shouldn't be here
		ASSERT( 0 );
	}
			
	// drop shadow if necessary
	if ( TESTFLAG( m_flags, CTIMEDTEXTFLAG_USE_DROPPED_SHADOW ) )
	{
		// change the text appearance to a drop shadow		
		m_text->SetColor( ( alpha << 24 ) | SETRGB( 0.0f, 0.0f, 0.0f ) );
		m_text->SetPosition( ( m_posX + 2 ), ( m_posY + 2 ) );
		
		// show the amount of text we should reveal so far
		if ( TESTFLAG( m_flags, CTIMEDTEXTFLAG_USE_CLIP_RECT ) )
		{
			m_text->RenderClipped( &m_clipRect, startChar, endChar );
		}
		else
		{
			m_text->Render( startChar, endChar );
		}

		// set the text back
		m_text->SetColor( ( alpha << 24 ) | m_color );
		m_text->SetPosition( m_posX, m_posY );
	}

	// show the amount of text we should reveal so far
	m_text->SetColor( ( alpha << 24 ) | m_color );
	if ( TESTFLAG( m_flags, CTIMEDTEXTFLAG_USE_CLIP_RECT ) )
	{
		m_text->RenderClipped( &m_clipRect, startChar, endChar );
	}
	else
	{
		m_text->Render( startChar, endChar );
	}
}


///
// CTimedText::InitInternalInfo()
//
void CTimedText::InitInternalInfo()
{
	// current state
	m_state = CTIMEDTEXT_NONE;
	m_text = 0;
	m_textLen = 0;
	
	// time tracking
	m_timeElapsed = 0.0f;
	m_uTime.m_timeElapseda = 0.0f;
	m_prevUpdateTime = 0.0f;
	
	//  character tracking (what we have seen to compute where we should be)
	m_charactersProcessed = 0;
	m_linesProcessed = 0;
	m_nNewlinesProcessed = 0;
	
#ifdef TIMED_TEXT_FADE_TOP_LINE
	m_nLineFadeCharStart = 0;
	m_nLineFadeCharEnd = 0;  // noninclusive
#endif // TIMED_TEXT_FADE_TOP_LINE
	
	// clear all the flags
	m_flags = 0;

	m_posX = 0;
	m_posY = 0;
	
	// references needed for scrolling		
	m_scrollRefPosY = 0;
	m_scrollRefTime = 0;

	// stub the sound names
	m_textDisplaySoundName[ 0 ] = '\0';
	m_scrollSoundName[ 0 ] = '\0';

	// sound handles
	if ( m_hTextDisplaySound != LTNULL )
	{
		// kill the looping typing sound
		g_pLTClient->SoundMgr()->KillSound( m_hTextDisplaySound );
		m_hTextDisplaySound = LTNULL;							
	}
}
