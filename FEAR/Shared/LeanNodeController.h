// ----------------------------------------------------------------------- //
//
// MODULE  : LeanNodeController.h
//
// PURPOSE : Definition of class used to manage the player lean
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LEAN_NODE_CONTROLLER_H__
#define __LEAN_NODE_CONTROLLER_H__

class CLeanNodeController : public ILTObjRefReceiver
{
	public: // Methods...

		CLeanNodeController();
		~CLeanNodeController();

		bool Init( HOBJECT hObject, HRECORD hLeanRecord );

		// Set the lean angle, in radians, to be used when modifying the nodes...
		void SetLeanAngle( float fLean );

		// Test to see if the object should be leaning...
		bool IsLeaning( ) const { return (m_fLeanAngle != 0.0f); }

		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Node controller for handling updating the rotation of the node...
		static void LeanNodeControler( const NodeControlData& iData, void *pUserData );


	private: // Methods...

		// Remove all node control functions and clear the list of lean nodes...
		void UnRegisterNodes( HOBJECT hObject );		

	private: // Members...

		// Object this lean controller is associated with...
		LTObjRefNotifier m_hObject;

		// Angle in radians of the lean...
		float		m_fLeanAngle;

		// Data used with the node controller call back to effect the node...
		class CLeanNode
		{
			public:

				CLeanNode( )
				:	m_pLeanNodeController	( NULL ),
					m_hNode					( INVALID_MODEL_NODE ),
					m_fWeight				( 0.0f )
				{ }

				// Pointer to the controller to access global data such as the amount of lean to apply...
				CLeanNodeController*	m_pLeanNodeController;

				//the node that this controlled node controls in the model
				HMODELNODE		m_hNode;

				// The blending weight used with this node
				float			m_fWeight;
		};

		typedef std::vector<CLeanNode, LTAllocator<CLeanNode, LT_MEM_TYPE_GAMECODE> > TLeanNodeList;
		TLeanNodeList			m_aLeanNodes;
};


#endif // __LEAN_NODE_CONTROLLER_H__

// EOF
