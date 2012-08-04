/*!
Defines the assertion interface for Lithtech.
Assertions should use the ASSERT macro.
*/

#ifndef __LTASSERT_H__
#define __LTASSERT_H__

#ifndef __ASSERT_H__
#include <assert.h>
#define __ASSERT_H__
#endif

#ifndef ASSERT
    #define ASSERT assert
#endif


#endif  //! __LTASSERT_H__



