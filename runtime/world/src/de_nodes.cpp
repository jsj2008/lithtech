
// This module just defines the global nodes for the engine.

#include "bdefs.h"
#include "de_world.h"
#include "de_objects.h"


static Node _g_InNode(NF_IN);
static Node _g_OutNode(NF_OUT);

Node *NODE_IN = &_g_InNode;
Node *NODE_OUT = &_g_OutNode;
