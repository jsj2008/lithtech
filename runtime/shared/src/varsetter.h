
// A helper thing to set a state variable within a scope.

#ifndef __VARSETTER_H__
#define __VARSETTER_H__

template<class T>
class VarSetter {
public:
    VarSetter(T *pVal, T inVal, T outVal) {
        *pVal = inVal;
        m_pVal = pVal;
        m_OutVal = outVal;
    }

    ~VarSetter() {
        *m_pVal = m_OutVal;
    }

    T *m_pVal;
    T m_OutVal;
};                      


#endif  // __VARSETTER_H__



