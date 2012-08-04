#ifndef __ILTMATH_H__
#define __ILTMATH_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


//---------------------------------------------------------------------------//

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/*!
The ILTMath interface exposes various math routines for dealing with
Euler angles, quaternions and matrices.

Define a holder to get this interface like this:
\code
define_holder(ILTMath, your_var);
\endcode

Used for: Obsolete.
*/

class ILTMath : public IBase {
public:
    interface_version(ILTMath, 0);

    virtual LTRESULT AlignRotation(LTRotation &q, LTVector &vf, LTVector &vu) = 0;
    virtual LTRESULT EulerRotateX(LTRotation &q, float angle) = 0;
    virtual LTRESULT EulerRotateY(LTRotation &q, float angle) = 0;
    virtual LTRESULT EulerRotateZ(LTRotation &q, float angle) = 0;
    virtual LTRESULT GetRotationVectors(LTRotation &q, LTVector &R0, LTVector &R1, LTVector &R2) = 0;
    virtual LTRESULT InterpolateRotation(LTRotation &qu, LTRotation &q0, LTRotation &q1, float u) = 0;
    virtual LTRESULT SetupEuler(LTRotation &q, float tx, float ty, float tz) = 0;
    virtual LTRESULT SetupRotationMatrix(LTMatrix &M, LTRotation &q) = 0;
    virtual LTRESULT SetupTransformationMatrix(LTMatrix &M, LTVector &O, LTRotation &q) = 0;
    virtual LTRESULT SetupTranslationFromMatrix(LTVector &t, LTMatrix &M) = 0;
    virtual LTRESULT SetupTranslationMatrix(LTMatrix &M, LTVector &t) = 0;
    virtual LTRESULT SetupRotationFromMatrix(LTRotation &q, LTMatrix &M) = 0;
    virtual LTRESULT SetupRotationAroundPoint(LTMatrix &M, LTRotation &q, LTVector &pl) = 0;
    virtual LTRESULT RotateAroundAxis(LTRotation &q, LTVector &u, float angle) = 0;
    virtual LTRESULT GetRotationVectorsFromMatrix(LTMatrix &M, LTVector &R0, LTVector &R1, LTVector &R2) = 0;

};
#endif //DOXYGEN_SHOULD_SKIP_THIS


#endif
