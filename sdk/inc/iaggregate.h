/*!  This header file defines the base aggregate structure.  Every
aggregate must have a link to its next one and a pointer to certain
functions that an aggregate must supply.  When implementing an
aggregate, you \em must derive from this!  (In C, just put it as the
first member in your structure.)  */

#ifndef __IAGGREGATE_H__
#define __IAGGREGATE_H__


#ifndef __LTSERVEROBJ_H__
#include "ltserverobj.h"
#endif


class IAggregate
{
public :

                    IAggregate() {}
    virtual         ~IAggregate() {}

    virtual uint32  EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT lData) { return 1; }
    virtual uint32  ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg) { return 1; }


public:



/*!  It is very important that these data members are \em exactly the same
as the C version of this class (Aggregate_t).  */

    IAggregate  *m_pNextAggregate;
};


#endif  //! __IAGGREGATE_H__







