#ifndef _LT_GRAPHICS_CAPS_H_
#define _LT_GRAPHICS_CAPS_H_


class LTGraphicsCaps
{
public:

	LTGraphicsCaps():
	  VertexShaderVersion(0),
	  PixelShaderVersion(0)
	  {
	  }

	uint32 VertexShaderVersion;
	uint32 PixelShaderVersion;

};

#endif