
#ifndef __COMMON_DE_H__
#define __COMMON_DE_H__


	#include "basedefs_de.h"


	class LMessage;


	class CommonLT
	{
	public:

		virtual		~CommonLT() {}

	// Helpers.
	public:

		// Initialize the ConParse and call.  Fills in m_Args and m_nArgs with each set of tokens.  
		// Returns DE_FINISHED when done.  Sample loop:
		// parse.Init(pStr);
		// while(pEngine->Parse2(&parse) == LT_OK)
		// {
		//     .. process args ..
		// }
		virtual DRESULT	Parse(ConParse *pParse)=0;

		// Tells you what type of object this is.
		virtual DRESULT GetObjectType(HOBJECT hObj, DDWORD *type)=0;

		// Gets a model animation's user dimensions (what they set for dimensions in ModelEdit).
		virtual DRESULT	GetModelAnimUserDims(HOBJECT hObject, DVector *pDims, HMODELANIM hAnim)=0;

		// Get the vectors from a rotation.
		virtual DRESULT GetRotationVectors(DRotation &rot, DVector &up, DVector &right, DVector &forward)=0;

		// Setup the rotation based on euler angles 
		// pitch = rotation around X
		// yaw = rotation around Y
		// roll = rotation around Z
		virtual DRESULT SetupEuler(DRotation &rot, float pitch, float yaw, float roll)=0;

		// Start a message.  Send it with a SendTo function and free it by calling LMessage::Release.
		virtual DRESULT	CreateMessage(LMessage* &pMsg)=0;


	// Geometry stuff.
	public:
		// Returns DE_NOTINWORLD if no world is loaded, or DE_INSIDE if the point
		// is in the world or DE_OUTSIDE if the point is not in the world.
		virtual DRESULT GetPointStatus(DVector *pPoint)=0;

		// Get the shade (RGB, 0-255) at the point you specify.
		// Returns DE_NOTINWORLD if the point is outside the world.
		virtual DRESULT GetPointShade(DVector *pPoint, DVector *pColor)=0;
			
		// Get the texture flags from a poly.  Returns DE_OK
		// or DE_ERROR if no world is loaded or hPoly is invalid.
		virtual DRESULT	GetPolyTextureFlags(HPOLY hPoly, DDWORD *pFlags)=0;
	};


#endif 





