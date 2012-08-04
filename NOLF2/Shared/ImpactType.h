
#ifndef  _ImpactType_h_INCLUDED_
#define  _ImpactType_h_INCLUDED_


//
// When the server tells the client to play a weapon FX,
// there are different types of impacts we may want to 
// play.  This list will specify the different type of
// impact effects that we can play.
//
typedef enum _IMPACT_TYPE
{
	IMPACT_TYPE_IMPACT,
	IMPACT_TYPE_RICOCHET,
	IMPACT_TYPE_BLOCKED,

	IMPACT_TYPE_COUNT   // always last
} IMPACT_TYPE;

#endif //_ImpactType_h_INCLUDED_