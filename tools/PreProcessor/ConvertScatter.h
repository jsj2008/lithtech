#ifndef __CONVERTSCATTER_H__
#define __CONVERTSCATTER_H__


// given a world, this will go through finding ScatterVolume
// objects and store preprocessed information about them into
// the worlds blind object data
bool ConvertScatter( CPreMainWorld* world, std::vector<CLightingPoint>& lightingPoints );


#endif // __CONVERTSCATTER_H__
