// this processes scatter objects and places scatter specific
// information into the blind object data

#include "bdefs.h"
#include "preworld.h"
#include "lightmapmaker.h"
#include "proputils.h"
#include "processing.h"
#include "noise.h"
#include "node_ops.h"
#include "geomroutines.h"
#include "convertscatter.h"


#define SCATTER_BLINDOBJECTID		0x73f53a84		// just a random, but constant 32-bit ID
#define SCATTER_LIGHTINGOFFSET		0.1f			// distance from the surface of the polygon to light
#define SCATTER_SUBVOLUMEDIMS		512.0f			// maximum length of a side of a scatter subvolume


struct ScatterParticle
{
	LTVector pos;		// particle position (no orientation at this time)
	uint32 color;		// color of the particle
	float scale;		// scale of the particle
	uint8 waveRot;		// 8-bit 0-1 rotation value for orientation of the wave motion
	uint8 waveStart;	// 8-bit 0-1 value for start time of the wave motion
	CPrePoly* poly;		// source polygon that spawned this particle
};


struct ScatterSubVolume
{
	LTVector pos;
	LTVector dims;
	std::vector<uint32> particles;
};


static inline double drand()
{
	return (double)rand() / (double)RAND_MAX;
}


static inline float Lerp( float t, float x0, float x1 )
{
	return x0 + t * (x1 - x0);
}


static void FillSubVolume( const std::vector<ScatterParticle>& particles, ScatterSubVolume& volume )
{
	volume.particles.clear();
	volume.particles.reserve( 256 );

	if( particles.size() <= 0 )
		return;

	LTVector min = volume.pos - volume.dims;
	LTVector max = volume.pos + volume.dims;

	uint32 curParticle = 0;
	std::vector<ScatterParticle>::const_iterator it;

	bool newBoundsInited = false;
	LTVector newMin, newMax;

	// grab particles that are within the original box
	for( it = particles.begin(); it != particles.end(); it++ )
	{
		LTVector pos = (*it).pos;

		if( (pos.x >= min.x) && (pos.z >= min.z) && (pos.x < max.x) && (pos.z < max.z) )
		{
			if( !newBoundsInited )
			{
				newMin = newMax = pos;
				newBoundsInited = true;
			}

			if( pos.x < newMin.x )
				newMin.x = pos.x;
			else if( pos.x > newMax.x )
				newMax.x = pos.x;
			if( pos.y < newMin.y )
				newMin.y = pos.y;
			else if( pos.y > newMax.y )
				newMax.y = pos.y;
			if( pos.z < newMin.z )
				newMin.z = pos.z;
			else if( pos.z > newMax.z )
				newMax.z = pos.z;

			volume.particles.push_back( curParticle );
		}

		curParticle++;
	}

	// update the dims with the newly calculated dimensions
	LTVector size = newMax - newMin;
	size *= 0.5f;

	volume.pos = newMin + size;
	volume.dims = size;
}


static void CreateSubVolumes( const std::vector<ScatterParticle>& particles, std::vector<ScatterSubVolume>& subVolumes )
{
	std::vector<ScatterParticle>::const_iterator it;

	subVolumes.clear();

	if( particles.size() <= 0 )
		return;

	// get the dims of all the particles
	LTVector min = particles[0].pos;
	LTVector max = min;

	for( it = particles.begin(); it != particles.end(); it++ )
	{
		LTVector curPos = (*it).pos;

		if( curPos.x < min.x )
			min.x = curPos.x;
		else if( curPos.x > max.x )
			max.x = curPos.x;
		if( curPos.y < min.y )
			min.y = curPos.y;
		else if( curPos.y > max.y )
			max.y = curPos.y;
		if( curPos.z < min.z )
			min.z = curPos.z;
		else if( curPos.z > max.z )
			max.z = curPos.z;
	}

	// inflate slightly to get around float precision issues at volume boundaries
	min -= 0.1f;
	max += 0.1f;

	LTVector size = max - min;

	// determine how manu subvolumes it will divide into
	uint32 numXSubVolumes = (uint32)(size.x / SCATTER_SUBVOLUMEDIMS) + 1;
	uint32 numZSubVolumes = (uint32)(size.z / SCATTER_SUBVOLUMEDIMS) + 1;
	uint32 numSubVolumes = numXSubVolumes * numZSubVolumes;

	subVolumes.reserve( numSubVolumes );

	float remainingZSize = size.z;
	float curZMin = min.z;

	for( uint32 z = 0; z < numZSubVolumes; z++ )
	{
		float curZSize = remainingZSize;
		if( curZSize >= SCATTER_SUBVOLUMEDIMS )
		{
			curZSize = SCATTER_SUBVOLUMEDIMS;
			remainingZSize -= SCATTER_SUBVOLUMEDIMS;
		}
		curZSize *= 0.5f;

		float remainingXSize = size.x;
		float curXMin = min.x;

		for( uint32 x = 0; x < numXSubVolumes; x++ )
		{
			float curXSize = remainingXSize;
			if( curXSize >= SCATTER_SUBVOLUMEDIMS )
			{
				curXSize = SCATTER_SUBVOLUMEDIMS;
				remainingXSize -= SCATTER_SUBVOLUMEDIMS;
			}
			curXSize *= 0.5f;

			ScatterSubVolume curVolume;
			curVolume.pos.x = curXMin + curXSize;
			curVolume.pos.z = curZMin + curZSize;
			curVolume.dims.x = curXSize;
			curVolume.dims.z = curZSize;

			// check to see which particles, if any, lie within this volume
			FillSubVolume( particles, curVolume );
			if( curVolume.particles.size() )
				subVolumes.push_back( curVolume );

			curXMin += SCATTER_SUBVOLUMEDIMS;
		}

		curZMin += SCATTER_SUBVOLUMEDIMS;
	}
}


// add the particle information to the worlds blind object data and return the index to the data
static uint32 AddScatterBlindData( CPreMainWorld* world, float plantDepth, std::vector<ScatterParticle>& particles, std::vector<ScatterSubVolume>& subVolumes, std::vector<CLightingPoint>& lightingPoints, bool bOnlyUseAmbient, const LTVector& vAmbient )
{
	if( particles.empty() )
		return 0xffffffff;

	static uint32 fltSz = sizeof(float);

	uint32 numVolumes = subVolumes.size();
	uint32 numParticles = 0;
	std::vector<ScatterSubVolume>::iterator volIt;

	// calculate the total number of particles
	for( volIt = subVolumes.begin(); volIt != subVolumes.end(); volIt++ )
	{
		numParticles += (*volIt).particles.size();
	}

	ASSERT( numParticles == particles.size() );		// you've probably hit a floating point precision issue

	// add room to the lighting points vector for these particles
	uint32* colorLocation;
	CLightingPoint lightingPoint;
	lightingPoints.reserve( lightingPoints.size() + numParticles );

	// initialize the blind data
	uint32 blindDataSize = 4 + numVolumes * 28 + numParticles * 22;
	uint8* blindData = new uint8[blindDataSize];
	uint8* curBlindDataPtr = blindData;

	// add the number of particles to the blind data
	*((uint32*)curBlindDataPtr) = numVolumes;
	curBlindDataPtr += 4;

	// add each subvolume to the blind data
	for( volIt = subVolumes.begin(); volIt != subVolumes.end(); volIt++ )
	{
		// write the volume position
		*((float*)curBlindDataPtr) = (*volIt).pos.x;
		curBlindDataPtr += fltSz;
		*((float*)curBlindDataPtr) = (*volIt).pos.y - plantDepth;
		curBlindDataPtr += fltSz;
		*((float*)curBlindDataPtr) = (*volIt).pos.z;
		curBlindDataPtr += fltSz;

		// write the volume dims
		*((float*)curBlindDataPtr) = (*volIt).dims.x;
		curBlindDataPtr += fltSz;
		*((float*)curBlindDataPtr) = (*volIt).dims.y;
		curBlindDataPtr += fltSz;
		*((float*)curBlindDataPtr) = (*volIt).dims.z;
		curBlindDataPtr += fltSz;

		// write the number of particles
		*((uint32*)curBlindDataPtr) = (*volIt).particles.size();
		curBlindDataPtr += 4;

		// add the particles to the blind data
		std::vector<uint32>::iterator it = (*volIt).particles.begin();
		for( ; it != (*volIt).particles.end(); it++ )
		{
			ScatterParticle& particle = particles[*it];

			// write the position
			*((float*)curBlindDataPtr) = particle.pos.x;
			curBlindDataPtr += fltSz;
			*((float*)curBlindDataPtr) = particle.pos.y - plantDepth;
			curBlindDataPtr += fltSz;
			*((float*)curBlindDataPtr) = particle.pos.z;
			curBlindDataPtr += fltSz;

			// write the color
			colorLocation = (uint32*)curBlindDataPtr;
			*((uint32*)curBlindDataPtr) = particle.color;
			curBlindDataPtr += 4;

			// write the scale
			*((float*)curBlindDataPtr) = particle.scale;
			curBlindDataPtr += fltSz;

			// write the wave rotation
			*curBlindDataPtr = particle.waveRot;
			curBlindDataPtr++;

			// write the wave start time
			*curBlindDataPtr = particle.waveStart;
			curBlindDataPtr++;

			// add this particle to the lighting points
			lightingPoint.color = colorLocation;
			lightingPoint.pos = particle.pos + particle.poly->Normal() * SCATTER_LIGHTINGOFFSET;
			lightingPoint.normal = particle.poly->Normal();
			lightingPoint.lightingFlags = 0;

			if(bOnlyUseAmbient)
				lightingPoint.lightingFlags |= CLightingPoint::LIGHT_ONLYUSEAMBIENT;

			float noiseValue = Noise( lightingPoint.pos / 32.0f );
			lightingPoint.colorOffset = vAmbient + LTVector( noiseValue * 35.0f, noiseValue * 35.0f, noiseValue * 35.0f );

			lightingPoints.push_back( lightingPoint );
		}
	}

	ASSERT( (uint32(curBlindDataPtr - blindData) == blindDataSize) );

	// add the blind data to the world
	CPreBlindData* blindDataObject = new CPreBlindData( blindData, blindDataSize, SCATTER_BLINDOBJECTID );
	world->m_BlindObjectData.push_back( blindDataObject );

	return world->m_BlindObjectData.size() - 1;
}


static void RemoveOutsideParticles( const LTVector& min, const LTVector& max, std::vector<ScatterParticle>& particles )
{
	std::vector<ScatterParticle> constrainedParticles;
	constrainedParticles.reserve( particles.size() );

	std::vector<ScatterParticle>::iterator it = particles.begin();

	for( ; it != particles.end(); it++ )
	{
		LTVector pos = (*it).pos;

		// check to see if the particle is within the volume
		if( pos.x >= min.x && pos.y >= min.y && pos.z >= min.z && pos.x <= max.x && pos.y <= max.y && pos.z <= max.z )
		{
			constrainedParticles.push_back( *it );
		}
	}

	particles.swap( constrainedParticles );
}


// return true if this particle shouldn't exist based on alpha or clumping
static bool KillParticle( const LTVector& pos, float alpha, float clumpiness, float clumpSize, float clumpClamp, const LTVector& clumpOffset, float textureClamp )
{
	// kill particles below the alpha clamp cutoff
	if( alpha < textureClamp )
		return true;

	// build up the clumpiness factor
	float noiseValue = (Noise( (pos + clumpOffset) / clumpSize ) + 1.0f) * 0.5f;
	noiseValue = Lerp( (1.0f - clumpiness), noiseValue, 1.0f );

	// kill particles below the clumpiness clamp cutoff
	if( noiseValue < clumpClamp )
		return true;

	// combine the alpha and noise
	float testValue = alpha * noiseValue;

	// do a dice roll to see if this particle should live
	if( drand() <= testValue )
		return false;

	return true;
}


static void ScatterParticlesOnPoly( CPrePoly* poly, float density, float clumpiness, float clumpSize, float clumpClamp, const LTVector& clumpOffset, float waveSize, float waveDirSize, float scaleRange, const char* texture, float textureClamp, std::vector<ScatterParticle>& particles )
{
	bool useAlpha = false;
	bool invertAlpha = false;

	// check to see if the polygon has a texture that matches the scatter texture
	if( strlen( texture ) )
	{
		const char* t0 = poly->TextureName( 0 );
		const char* t1 = poly->TextureName( 1 );

		bool t0Match = (stricmp( t0, texture ) == 0);
		bool t1Match = (stricmp( t1, texture ) == 0);

		// neither poly texture matches, don't scatter
		if( !t0Match && !t1Match )
			return;

		// texture 0, scatter based on non-inverted alpha
		if( t0Match && !t1Match )
			useAlpha = true;

		// texture 1, scatter based on inverted alpha
		if( t1Match && !t0Match )
		{
			useAlpha = true;
			invertAlpha = true;
		}
	}

	int numTris = poly->NumVerts() - 2;

	// prime the vertex positions
	LTVector v0, v1, v2;
	v0 = poly->Pt( 0 );
	v2 = poly->Pt( 1 );

	// prime the vertex alpha values
	float a0, a1, a2;
	if( useAlpha )
	{
		a0 = poly->Alpha( 0 ) / 255.0f;
		a2 = poly->Alpha( 1 ) / 255.0f;

		if( invertAlpha )
		{
			a0 = 1.0f - a0;
			a2 = 1.0f - a2;
		}
	}
	else
	{
		a0 = a1 = a2 = 1.0f;
	}

	for( int i = 0; i < numTris; i++ )
	{
		// update the verts making up the triangle
		v1 = v2;
		v2 = poly->Pt( i + 2 );

		if( useAlpha )
		{
			a1 = a2;
			a2 = poly->Alpha( i + 2 ) / 255.0f;
			if( invertAlpha )
				a2 = 1.0f - a2;
		}

		// get the area of the triangle
		double area = ((v1 - v0).Cross(v2 - v0)).Mag() * 0.5;

		// determine how many particles to scatter on the triangle
		double fullNumParticles = (density * area) * (1.0 / (256.0 * 256.0));
		int numParticles = (int)fullNumParticles;

		// calculate the probability of the remainder resulting in an additional particle (for smaller triangles)
		if( drand() < (fullNumParticles - numParticles) )
			numParticles++;

		for( int j = 0; j < numParticles; j++ )
		{
			float a, b, c;

			// generate random barycentric coordinates within the tri
			b = (float)drand();
			c = (float)drand();

			// clamp the coordinates to within the tri (if in the other half of the parallelogram)
			if( b + c > 1.0f )
			{
				b = 1.0f - b;
				c = 1.0f - c;
			}

			a = 1.0f - b - c;

			// the position of a random point on the triangle
			LTVector particlePos = a*v0 + b*v1 + c*v2;

			// the polygon alpha at this point
			float particleAlpha = a*a0 + b*a1 + c*a2;

			// check the probability of a particle at this position based on alpha, clumping, etc.
			if( KillParticle( particlePos, particleAlpha, clumpiness, clumpSize, clumpClamp, clumpOffset, textureClamp ) )
				continue;

			ScatterParticle particle;
			particle.pos = particlePos;
			particle.color = 0xffffffff;
			particle.scale = 1.0f + ((float)drand() * 2.0f - 1.0f) * scaleRange;
			particle.poly = poly;

			// get noise, map it to 0-1, remap 0.3-0.7 to 0-1 (yeah, could be one step, but "math is hard")
			float waveNoise = (Noise(particlePos/waveDirSize) + 1.0f) * 0.5f;
			waveNoise -= 0.3f;
			waveNoise *= 2.5f;
			if( waveNoise < 0.0f )
				waveNoise = 0.0f;
			else if( waveNoise > 1.0f )
				waveNoise = 1.0f;

			// finally map it to 0-255 (255 is less equally represented than 0, but that shouldn't matter)
			particle.waveRot = (uint8)(waveNoise * 255.0f);

			// get noise, map it to 0-1, remap 0.3-0.7 to 0-1 (yeah, could be one step, but "math is hard")
			waveNoise = (Noise(particlePos/waveSize) + 1.0f) * 0.5f;
			waveNoise -= 0.3f;
			waveNoise *= 2.5f;
			if( waveNoise < 0.0f )
				waveNoise = 0.0f;
			else if( waveNoise > 1.0f )
				waveNoise = 1.0f;

			// finally map it to 0-255 (255 is less equally represented than 0, but that shouldn't matter)
			particle.waveStart = (uint8)(waveNoise * 255.0f);

			particles.push_back( particle );
		}
	}
}


static void GetPoliesInAABB( CPreMainWorld* mainWorld, const LTVector& min, const LTVector& max, std::vector<CPrePoly*>& polies )
{
	if( !mainWorld->m_WorldModels.GetSize() )
		return;

	CPreWorld* world = mainWorld->GetPhysicsBSP();
	ASSERT( world );

	for( GPOS pos = world->m_Polies; pos; )
	{
		CPrePoly* curPoly = world->m_Polies.GetNext( pos );

		curPoly->CalcExtents();

		LTVector polyMin = curPoly->m_vExtentsMin;
		LTVector polyMax = curPoly->m_vExtentsMax;

		// check if the AABBs are disjoint (don't really need a full tri to AABB test)
		if( ((min.x > polyMax.x) || (polyMin.x > max.x)) ||
			((min.y > polyMax.y) || (polyMin.y > max.y)) ||
			((min.z > polyMax.z) || (polyMin.z > max.z)) )
				continue;

		// they overlap, so add the polygon to the vector
		polies.push_back( curPoly );
	}
}


static bool ConvertScatterVolume( CPreMainWorld* world, CWorldNode* node, std::vector<CLightingPoint>& lightingPoints )
{
	CVectorProp* dimsProp = (CVectorProp*)node->GetPropertyList()->GetMatchingProp( "Dims", PT_VECTOR );
	CRealProp* indexProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "BlindDataIndex", PT_REAL );
	CRealProp* densityProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "Density", PT_REAL );
	CRealProp* plantDepthProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "PlantDepth", PT_REAL );
	CRealProp* clumpinessProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "Clumpiness", PT_REAL );
	CRealProp* clumpSizeProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "ClumpSize", PT_REAL );
	CRealProp* clumpClampProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "ClumpCutoff", PT_REAL );
	CVectorProp* clumpOffsetProp = (CVectorProp*)node->GetPropertyList()->GetMatchingProp( "ClumpOffset", PT_VECTOR );
	CRealProp* waveSizeProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "WaveSize", PT_REAL );
	CRealProp* waveDirSizeProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "WaveDirSize", PT_REAL );
	CRealProp* scaleRangeProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "ScaleRange", PT_REAL );
	CStringProp* textureProp = (CStringProp*)node->GetPropertyList()->GetMatchingProp( "PlacementTexture", PT_STRING );
	CRealProp* textureClampProp = (CRealProp*)node->GetPropertyList()->GetMatchingProp( "PlacementCutoff", PT_REAL );
	CColorProp* ambientLightProp = (CColorProp*)node->GetPropertyList()->GetMatchingProp( "AmbientLight", PT_COLOR );
	CBoolProp* onlyUseAmbientProp = (CBoolProp*)node->GetPropertyList()->GetMatchingProp( "OnlyUseAmbient", PT_BOOL );

	if( !dimsProp || !indexProp || !densityProp || !clumpinessProp || !clumpSizeProp || 
		!waveSizeProp || !scaleRangeProp || !textureProp || !ambientLightProp || !onlyUseAmbientProp)
	{
		DrawStatusText(eST_Error, "Found bad ScatterVolume object.  Please update objects before reprocessing." );
		return false;
	}

	indexProp->SetValue( -1.0f );

	LTVector pos = node->GetPos();
	LTVector dims = dimsProp->m_Vector;

	// grab the polies that are in this AABB
	std::vector<CPrePoly*> polies;
	GetPoliesInAABB( world, pos - dims, pos + dims, polies );

	// no touching polies were found, do nothing
	if( polies.empty() )
		return true;

	// get the desired particle density
	float density = densityProp->m_Value;
	if( density <= 0.0f )
		return true;

	// get the desired plant depth
	float plantDepth = (plantDepthProp ? plantDepthProp->m_Value : 0.0f);

	// get the desired clumpiness
	float clumpiness = clumpinessProp->m_Value;
	float clumpSize = clumpSizeProp->m_Value;

	// get the clump clamp value
	float clumpClampValue = (clumpClampProp ? clumpClampProp->m_Value : 0.0f);

	// get the clump offset
	LTVector clumpOffset = (clumpOffsetProp ? clumpOffsetProp->m_Vector : LTVector(0.0f,0.0f,0.0f));

	// get the placement clamp value
	float textureClampValue = (textureClampProp ? textureClampProp->m_Value : 0.0f );

	// get the desired wave size
	float waveSize = waveSizeProp->m_Value;

	// get the desired wave direction size
	float waveDirSize = (waveDirSizeProp ? waveDirSizeProp->m_Value : waveSize );

	// get the desired scale range (1.0 +/- scalerange)
	float scaleRange = scaleRangeProp->m_Value;

	// get the name of the texture used to choose which polygons to place particles on
	char* texture = textureProp->GetString();
	if( !texture )
		return true;

	std::vector<ScatterParticle> particles;
	particles.reserve( 4096 );

	// loop over the polygons, scattering particles on each
	std::vector<CPrePoly*>::iterator it = polies.begin();
	for( ; it != polies.end(); it++ )
	{
		ScatterParticlesOnPoly( *it, density, clumpiness, clumpSize, clumpClampValue, clumpOffset, waveSize, waveDirSize, scaleRange, texture, textureClampValue, particles );
	}

	// remove any particles outside the AABB
	RemoveOutsideParticles( pos - dims, pos + dims, particles );

	// sort the particles into subvolumes
	std::vector<ScatterSubVolume> subVolumes;
	CreateSubVolumes( particles, subVolumes );

	//get the lighting properties
	LTVector vAmbient = ambientLightProp->m_Vector;
	bool bOnlyUseAmbient = !!onlyUseAmbientProp->m_Value;

	uint32 blindDataIndex = AddScatterBlindData( world, plantDepth, particles, subVolumes, lightingPoints, bOnlyUseAmbient, vAmbient );

	if( blindDataIndex != 0xffffffff )
		indexProp->SetValue( (float)blindDataIndex + 0.1f );

	return true;
}


static uint32 FindObjectsOfType( CPreMainWorld* world, const char* objectType, std::vector<CWorldNode*>& objects )
{
	objects.clear();

	for( uint32 curObj = 0; curObj < world->m_Objects; curObj++ )
	{
		CBaseEditObj* pObj = world->m_Objects[curObj];

		if( pObj->GetType() != Node_Object )
			continue;

		if( const char* className = pObj->GetClassName() )
		{
			if( strcmp( className, objectType ) == 0 )
				objects.push_back( pObj );
		}
	}

	return objects.size();
}


// given a world, this will go through finding ScatterVolume
// objects and store preprocessed information about them into
// the worlds blind object data
bool ConvertScatter( CPreMainWorld* world, std::vector<CLightingPoint>& lightingPoints )
{
	std::vector<CWorldNode*> scatterVolumes;

	// force noise to initialize so it doesn't trample our srand :)
	Noise( LTVector(0.0f,0.0f,0.0f) );

	srand( 1337 );

	FindObjectsOfType( world, "ScatterVolume", scatterVolumes );

	std::vector<CWorldNode*>::iterator it = scatterVolumes.begin();

	for( ; it != scatterVolumes.end(); it++ )
	{
		if( !ConvertScatterVolume( world, *it, lightingPoints ) )
			return false;
	}

	return true;
}
