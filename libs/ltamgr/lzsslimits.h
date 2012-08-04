#ifndef __LZSSLIMITS_H__
#define __LZSSLIMITS_H__

//number of bits used to store the offset into the window
#define NUM_OFFSET_BITS			12

//the number of bits used to incode the length of the run
#define NUM_LENGTH_BITS			4

//the size of the window in bytes
#define WINDOW_SIZE				(1 << NUM_OFFSET_BITS)

//the length of a run needed to get at least the same comrpessed size
#define BREAK_EVEN_POINT		((1 + NUM_OFFSET_BITS + NUM_LENGTH_BITS) / 9)

//the amount of characters that need to be read into the buffer for
//looking ahead for runs
#define LOOK_AHEAD_SIZE			((1 << NUM_LENGTH_BITS) + BREAK_EVEN_POINT)

//the index in the node list that represents the root of the tree
#define ROOT_NODE				WINDOW_SIZE

//a specieal end of stream node
#define END_OF_STREAM			0

//the node that essentially acts as the null tie off node
#define NULL_NODE				0

//given an index, it will wrap it around to ensure that it is a valid
//window position
#define WINDOW_POS(a)			((a) & (WINDOW_SIZE - 1))

#endif

