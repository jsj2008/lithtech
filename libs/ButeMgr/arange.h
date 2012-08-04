#ifndef _ARANGE_H_
#define _ARANGE_H_


class CARange
{

public:

	CARange() { }
	CARange(double min, double max) { m_min = min; m_max = max; }

	void Set(double min, double max) { m_min = min; m_max = max; }
	CARange Get() { return CARange(m_min, m_max); }

	double GetMin() { return m_min; }
	double GetMax() { return m_max; }

protected:

	double m_min;
	double m_max;

};



#endif 