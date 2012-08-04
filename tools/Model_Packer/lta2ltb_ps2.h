// ------------------------------------------------------------------------
// lithtech (c) 2000
// ------------------------------------------------------------------------
#include "ltb_file.h"
#include "stripify.h"



// ------------------------------------------------------------------------
// exporter for ps2 ltbs
// ------------------------------------------------------------------------
class C_LTB_PS2_File : public C_LTB_File {
public :
	C_LTB_PS2_File():texture_on(true),wireframe_on(false)					{ }
	~C_LTB_PS2_File()					{ }

	bool	ExportFile(Model* pModel, uint32 memorySize);

	// 
	void setPrimParams( bool do_texturemapping, bool do_wireframe = false)
	{
		texture_on = do_texturemapping ;
		wireframe_on = do_wireframe ;
	}
protected:

	// write material out
	void write_material( PieceLOD *,uint32 iTexture,
									float spec_map_pow,
									float spec_map_scl);
	
	void WritePiece( ModelPiece *pPiece, uint32 memorySize);

	uint8	GetFileType()				{ return LTB_PS2_MODEL_FILE; }
	uint16	GetFileVersion()		;//	{ return PS2_FILE_VERSION; }

	// calculate the size of a render patch including vif commands.
	uint32 calc_sizeof_patch_w_VIF(	const Stripification& stripification,
											uint32  patchNum );


	bool texture_on ;
	bool wireframe_on ;

};
