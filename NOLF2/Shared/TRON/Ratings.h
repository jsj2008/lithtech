// ratings.h
// definitions useful to other programs for ratings.

#ifndef __RATINGS_H
#define __RATINGS_H

typedef enum 
{
	Rating_Health = 0,
	Rating_Energy,
	Rating_Accuracy,
	Rating_Stealth,
	Rating_Efficiency,
	NUM_RATINGS,
} PerformanceRating;

#endif // __RATINGS_H