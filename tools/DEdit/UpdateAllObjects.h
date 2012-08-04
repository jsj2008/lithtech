#ifndef __UPDATEALLOBJECTS_H__
#define __UPDATEALLOBJECTS_H__

//calling this function will prompt the user for a directory to update prefabs
//under and will recursively traverse that directory structure looking for world
//LT* files and upon finding them, will open them, update the object properties,
//save them and close them. This will keep all prefabs in sync with the object
//LTO information
bool UpdateAllLevelObjects();

#endif

