//-------------------------------------------------------------------
// LTALimits.h
//
// Definitions for implementation limitations on the current LTA
// library. They are grouped to allow easy expanding of the library
// as the complexity of LTA files increases
//
// Created: 1/22/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------
#ifndef __LTALIMITS_H__
#define __LTALIMITS_H__

//the maximum length a single value or string (not counting quotes) can be
//plus one. Values that exceed this size will be truncated
#define MAX_VALUE_LENGTH			1024

//the maximum depth that an LTA can be nested.
#define MAX_LTA_DEPTH				1024

//this number represents the maximum span of the node tree. The span of the
//tree is the longest distance to a child from the root. For example, the
//tree (a (b c (d e (f g) h ) ) ) would have a span of 10 (remember to count
//the list items themselves, not just the values) going from a to g.
#define MAX_LTA_SPAN				2097152

//when printing out real numbers, this is the number of decimal places
//that will be printed
#define LTA_DECIMAL_ACCURACY		6

#endif
