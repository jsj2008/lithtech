
// This clips a line in pVerts.
// CLIPTEST and DOCLIP should be defined.

if(CLIPTEST(pVerts[0].m_Vec))
{
	if(!CLIPTEST(pVerts[1].m_Vec))
	{
		// 0 inside, 1 outside.
		pOut = &pVerts[1];
		
		DOCLIP(pVerts[0].m_Vec, pVerts[1].m_Vec);
		TLVertex::ClipExtra(&pVerts[0], &pVerts[1], pOut, t);
	}
}
else
{
	if(CLIPTEST(pVerts[1].m_Vec))
	{
		// 0 outside, 1 inside.
		pOut = &pVerts[0];
		
		DOCLIP(pVerts[0].m_Vec, pVerts[1].m_Vec);
		TLVertex::ClipExtra(&pVerts[0], &pVerts[1], pOut, t);
	}
	else
	{
		// Both outside.
		return false; 
	}
}

