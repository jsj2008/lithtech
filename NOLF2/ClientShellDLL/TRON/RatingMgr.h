//
//
// RatingMgr.h
//
//
//
#ifndef __RATINGMGR_H
#define __RATINGMGR_H

#include "Ratings.h"
#include "ScreenSpriteMgr.h"

#define ADDITIVE_DEFAULT_FILE	"Attributes\\Additives.txt"
#define ADDITIVE_MAX_NAME		32
#define ADDITIVE_MAX_FILE_PATH	64

// TODO
// IDS_ADDITIVE_DAMAGE
// IDS_ADDITIVE_DEFENSE
// IDS_ADDITIVE_ACCURACY
// IDS_ADDITIVE_STEALTH
// IDS_ADDITIVE_SUCCESS

struct ADDITIVE
{
	ADDITIVE();
	char				szName[ADDITIVE_MAX_NAME];		// internal string name
	uint16				nNameId;
	uint16				nDescriptionId;					// ID for description filler
	char				szSprite[ADDITIVE_MAX_FILE_PATH];
	CScreenSprite *		pSprite;
	int8				nTempSlot;
	int8				nSlot;							// -1 = nowhere, 0-2 = slot
	int8				Adjust[NUM_RATINGS];			// how much to change each rating
};

class CRatingMgr;
extern CRatingMgr* g_pRatingMgr;

class CRatingMgr : public CGameButeMgr
{
public:
	CRatingMgr();
	~CRatingMgr();

    LTBOOL      Init(const char* szAttributeFile=ADDITIVE_DEFAULT_FILE);
	void		Term();

	// Load? Save?

	void		SetPerformanceRating(PerformanceRating p, int iVal);
	uint8		GetRating(PerformanceRating p);
	uint8		GetBaseRating(PerformanceRating p);

	LTBOOL		IsSurgeRating(PerformanceRating p) {return (m_iAdjustedRating[p] >= 100); }

	// MSVC6 does NOT support templated member functions.  This function
	// belongs here but cannot due to this limitation.  This comment is
	// here to remind anyone looking for performance ratings that this
	// function exitst and is ready to go.  See the bottom of this file
	// for the implementation.
	//
	// template <typename T>
	// T InterpolateRating( PerformanceRating ePerformanceRating,
	//                      T tMinValue, 
	//                      T tMaxValue,
	//                      T tSurgeValue )

	void		RemovePlayerAdditives();

	void		GivePlayerAdditive(ADDITIVE * pAdditive);
	void		GivePlayerAdditive(char * pszName);

	bool		PlayerHasAdditive(ADDITIVE * pAdditive);
	bool		PlayerHasAdditive(char * pszName);

	void		PopulateRatings();	// function to fill the ratings screen

	ADDITIVE*	GetAdditive(const char *pszName);

	int			GetRatingGoo() { m_iRatingGoo; }
	void		SetRatingGoo(int iGoo) {m_iRatingGoo = iGoo; }
	void		AddRatingGoo(int iGoo) {m_iRatingGoo += iGoo; }
private:

	void ComputeAdjustedRatings();

	int		m_iRatingGoo;
	uint8 m_iBaseRating[NUM_RATINGS];			// base player stats
	uint8 m_iAdjustedRating[NUM_RATINGS];		// stats after additives

	typedef std::vector<ADDITIVE *> AdditiveArray;
	AdditiveArray m_AddArray;					// all additives in the game
	AdditiveArray m_PlayerAddArray;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InterpolateRating()
//
//	PURPOSE:	Using the specificed rating, map the value to a given range.
//				If the rating is in Surge mode, return the surge value.
//				Otherwise, interpolate between the min and max values.
//
//				NOTE: this belongs in the CRatingMgr class, but MSVC6
//				doesn't support member templates so this function must be
//				placed externally.  I'll place in here to keep it close to
//				where it is supposed to go.

// ----------------------------------------------------------------------- //

template <typename T>
T InterpolateRating( PerformanceRating ePerformanceRating,
                     T tMinValue,
                     T tMaxValue,
                     T tSurgeValue )
{
	if ( g_pRatingMgr->IsSurgeRating( ePerformanceRating ) )
	{
		//
		// rating is in surge mode
		//
		
		// return the surge value
		return tSurgeValue;
	}
	else
	{
		//
		// rating is NOT in surge mode
		//

		// get the current rating
		uint8 nCurrentRating = g_pRatingMgr->GetRating( ePerformanceRating );
		ASSERT( ( 0 <= nCurrentRating ) && ( 100 >= nCurrentRating ) );

		// return the interpolated value 
		return tMinValue + ( tMaxValue - tMinValue ) * nCurrentRating / 100;
	}
}



#endif // __RATINGMGR_H
// ScreenRatings reports back an amount of goo
// clears the additives
// sends the new values of the ratings