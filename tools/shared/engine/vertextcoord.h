// This module defines the vertex TCoord accessors.

#ifdef LGLIDE_COMPILE
	void Setup(float inRHW, float inTU, float inTV)
	{
		rhw = inRHW;
		tu = inTU * rhw;
		tv = inTV * rhw;
	}

	void SetTCoords(float inTU, float inTV)
	{
		tu = inTU * rhw;
		tv = inTV * rhw;
	}

	void SetTCoordsRaw(float inTU, float inTV)
	{
		tu = inTU;
		tv = inTV;
	}

	void TranslateTCoords() {tu *= rhw; tv *= rhw;}

	static int OffsetOfTCoords() {return offsetof(TLVertex, tu);}

#else

	void Setup(float inRHW, float inTU, float inTV)
	{
		rhw = inRHW;
		tu = inTU;
		tv = inTV;
	}
	void SetTCoords(float inTU, float inTV)
	{
		tu = inTU;
		tv = inTV;
	}
	void SetTCoordsRaw(float inTU, float inTV)
	{
		tu = inTU;
		tv = inTV;
	}
	void TranslateTCoords() {}

#endif

	float TU() {return tu;}
	float TV() {return tv;}

