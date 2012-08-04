
#ifndef  _DiscDeflector_h_INCLUDED_
#define  _DiscDeflector_h_INCLUDED_

class CDiscDeflector : public GameBase
{
public:
	CDiscDeflector();
	~CDiscDeflector();

	// engine messages directed to this object go through here
	uint32 EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

	// object messages directed to this object go through here
	uint32 ObjectMessageFn( HOBJECT hSender, ILTMessage_Read *pMsg );

private:
};


#endif //_DiscDeflector_h_INCLUDED_