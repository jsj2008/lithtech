#ifndef __D3D_RENDERSHADER_H__
#define __D3D_RENDERSHADER_H__

// External declarations
class ViewParams;
class CRBSection;
struct SRBVertex;
class CMemStats_World;
class DynamicLight;
class IAggregateShader;
class CD3D_RenderBlock;

#include "erendershader.h"

// Baseclass for shaders
class CRenderShader
{
public:
	CRenderShader() {}
	virtual ~CRenderShader() {}

	// Validate this shader for use with this section
	virtual bool ValidateShader(const CRBSection &cSection) { return true; }

	// Preview a renderblock
	// Returns the renderblock's index
	virtual uint32 PreviewRenderBlock(const CD3D_RenderBlock &cRenderBlock) { return 0; }
	// Preview a section (for growing internal buffers and etc)
	// Note : This must be called once for each call to AddSection, in the same order
	virtual void PreviewSection(const CRBSection &cSection) {}
	// Lock (AddSection is going to be called for each of the sections)
	virtual void Lock(uint32 nRenderBlock) {}
	// Add a section to the rendering list
	virtual void AddSection(const CRBSection &cSection, const uint16 *aIndices, const SRBVertex *aVertices, const SRBVertex *aSrcVertices) {}
	// Unlock (AddSection is done being called)
	virtual void Unlock() {}
	// Clear the rendering data
	virtual void Clear() {}
	// Flush any pending rendering
	virtual void Flush() {}
	// Mark the section breaks
	virtual void MarkSectionBreaks() {}

	// Gets the shader ID of this shader
	virtual ERenderShader GetShaderID() const = 0;//{ return eShader_Invalid; }

	// Draw the sections of a renderblock
	struct DrawState
	{
		const ViewParams *m_pParams;
		const DynamicLight * const *m_pLightList;
		uint32 m_nNumLights;
		// Convenience type for iterating through the light list
		typedef const DynamicLight * const * TLightListIterator;
	};
	virtual void Draw(const DrawState &cState, uint32 nRenderBlock) {}

	// Renders an aggreagate shader on all of its child shader blocks
	virtual bool DrawAggregateShader(IAggregateShader *pShader, uint32 nRenderBlock) { return true; }

	//////////////////////////////////////////////////////////////////////////////
	// Debugging functions

	// Display the texture mapping for a triangle in the given screen area
	virtual void DebugTri(
		uint32 nRenderBlock,
		const CRBSection &cSection,
		uint32 nTriIndex,
		const uint16 *aIndices,
		const SRBVertex *aVertices,
		float fX, float fY,
		float fSizeX, float fSizeY) {}

	// Count the memory used by this shader
	virtual void GetMemStats(CMemStats_World &cMemStats) const {}
};

#endif //__D3D_RENDERSHADER_H__