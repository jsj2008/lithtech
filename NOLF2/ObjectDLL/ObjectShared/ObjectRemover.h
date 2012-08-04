// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectRemover.h
//
// PURPOSE : ObjectRemover - Definition
//
// CREATED : 04.23.1999
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECT_REMOVER_H__
#define __OBJECT_REMOVER_H__

LINKTO_MODULE( ObjectRemover );


class ObjectRemover : public BaseClass
{
	public : // Public constants

		enum Constants
		{
			kMaxGroups			= 12,
			kMaxObjectsPerGroup	= 11,
		};

	public : // Public methods

		ObjectRemover();
		~ObjectRemover();

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);

	protected :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);

        LTBOOL   Update();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

	public : // Public member variables

		int			m_cGroupsToKeep;
		HSTRING		m_ahstrObjects[kMaxGroups][kMaxObjectsPerGroup];
};

#endif // __ObjectRemover_H__