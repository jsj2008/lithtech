#ifndef __ROTATINGMODEL_H__
#define __ROTATINGMODEL_H__


#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "DestructableModel.h"
#include "Rotating.h"

class CDestructableModel;

// RotatingModel class
class RotatingModel : public CDestructableModel
{
	public:

		RotatingModel();
		virtual ~RotatingModel();

	protected:

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, float lData);

	private:

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	protected:

		Rotating		m_Rotating;
};


#endif // __ROTATINGMODEL_H__