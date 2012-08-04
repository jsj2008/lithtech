
// This class manages the tracking of a quantity's increment per second (like
// measuring framerate, bytes per second, packets per second, etc).

#ifndef __RATETRACKER_H__
#define __RATETRACKER_H__

class RateTracker
{
public:

				RateTracker();

	// Set the cycle time.  0.0f means to never cycle automatically.
	void		Init(float cycleTime);
	
	// Get the goods..			
	float		GetRate()			{return m_Total / m_Seconds;}
	
	// Add to the total amount (like increment the frame count).
	void		Add(float amount)	{m_Total += amount;}
	
	// Call this as often as you want to update the 
	void		Update(float timeDelta);
	
	// This is usually called internally automatically as you update it but
	// this resets its seconds counter.
	void		Cycle();


protected:
	
	float		m_Total;
	float		m_Seconds; 
	float		m_CycleTime;  // Controls how often it cycles.
};


#endif 




