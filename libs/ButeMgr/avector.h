#ifndef _AVECTOR_H_
#define _AVECTOR_H_



class CAVector
{

public:

	CAVector() { }
	CAVector(double i, double j, double k) { m_i = i; m_j = j; m_k = k; }

	void Set(double i, double j, double k) { m_i = i; m_j = j; m_k = k; }
	CAVector Get() { return CAVector(m_i, m_j, m_k); }

	double Geti() { return m_i; }
	double Getj() { return m_j; }
	double Getk() { return m_k; }


#if defined(_LITHTECH_)
    operator const LTVector() const
	{
        LTVector vec;
		vec.x = (float)m_i;
		vec.y = (float)m_j;
		vec.z = (float)m_k;
		return vec;
	}
#endif

protected:

	double m_i;
	double m_j;
	double m_k;

};



#endif