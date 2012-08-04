#ifndef __ROTATINGBRUSH_H__
#define __ROTATINGBRUSH_H__


#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "DestructableBrush.h"
#include "Rotating.h"

class CDestructableBrush;

// RotatingBrush class
class RotatingBrush : public CDestructableBrush
{
	public:

		RotatingBrush();
		virtual ~RotatingBrush();

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);

	private:

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	protected:

		Rotating		m_Rotating;
};


#endif // __ROTATINGBRUSH_H__