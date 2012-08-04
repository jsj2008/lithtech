//----------------------------------------------------------
//
// MODULE  : FireFX.CPP
//
// PURPOSE : defines classes for Fire on Objects
//
// CREATED : 2/23/98
//
//----------------------------------------------------------

// Includes....
#include <stdio.h>
#include <string.h>
#include "serverobj_de.h"
#include "FireFX.h"
#include "cpp_server_de.h"
#include "BaseCharacter.h"
#include "ClientSmokeTrail.h"


BEGIN_CLASS(FireFX)
	ADD_STRINGPROP(PlaceFireOn, "MyName")	    //  Object to place the fire on
//	ADD_REALPROP(BurnTime, 0.0f)	            //  Burn time
//	ADD_STRINGPROP(FireType, "small")	        //  Small/Medium/Large
END_CLASS_DEFAULT(FireFX, B2BaseClass, NULL, NULL)

void BPrint(char*);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::FireFX
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

FireFX::FireFX() : B2BaseClass(OT_NORMAL)
{
	m_bPlaySound	= DTRUE;
	m_bFirstUpdate	= DTRUE;

	VEC_SET(m_vScale, 0.2f, 0.2f, 0.0f);

	m_fSoundVol		= 200;

	m_hstrAttachTo	= DNULL;
    
    m_hLinkObject   = DNULL;
    m_hLight        = DNULL;
    
    m_fLastTime     = 0.0f;
    m_bDead         = DFALSE;

	m_fBurnTime		= 0.0f;
    
    // Setup default values
    for (int x=0; x<MAXFIRES; x++)
    {
        m_hSmokeTrail[x] = DNULL;
		m_hSprite[x] = DNULL;
		m_fDuration[x] = 0.0f;
    	m_fStartTime[x]	= 0.0f;
    }            

    m_nFireIndex = 0;    
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::~FireFX
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

FireFX::~FireFX()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrAttachTo)
	{
		pServerDE->FreeString(m_hstrAttachTo);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD FireFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update((DVector *)pData))
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) return 0;

				pServerDE->RemoveObject(m_hObject);
			}
			break;
		}

		case MID_PRECREATE:
		{
			if (fData == 1.0f)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		// If we created a link to the target, this will tell us that it no longer exists
		case MID_LINKBROKEN:
		{
			HOBJECT hObj = (HOBJECT)pData;
            if (m_hLinkObject == hObj)
            {
	    		m_hLinkObject = DNULL;
		    }
		}
		break;
		
		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::Setup
//
//	PURPOSE:	Set up a impact with the information needed
//
// ----------------------------------------------------------------------- //

void FireFX::Setup(HOBJECT hLinkObject, DFLOAT fBurnTime)
{
	m_hLinkObject = hLinkObject;
	m_fBurnTime = fBurnTime;
    
	FirstUpdate();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

DBOOL FireFX::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;
	char buf[MAX_CS_FILENAME_LEN];


	buf[0] = '\0';
	pServerDE->GetPropString("PlaceFireOn", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrAttachTo = pServerDE->CreateString(buf);

	pServerDE->GetPropReal("BurnTime", &m_fBurnTime);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::InitialUpdate
//
//	PURPOSE:	Do initial updating
//
// ----------------------------------------------------------------------- //

DBOOL FireFX::InitialUpdate(DVector*)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.1);

    return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::FirstUpdate
//
//	PURPOSE:	Do First updating
//
// ----------------------------------------------------------------------- //

void FireFX::FirstUpdate()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// Can Create Up to 5 Fire Object at a Time....
    // Create Random durations for each Flame
    for (int x=0; x<MAXFIRES; x++)
    {
        m_fDuration[x] = pServerDE->Random(0.25f, 0.50f);
        m_fStartTime[x] = pServerDE->GetTime();
    }            

    // If FireFx was created from something else then They would set the LinkObject    
    
    if (m_hLinkObject == DNULL)
    {
    	DVector vPos;
    	pServerDE->GetObjectPos(m_hObject, &vPos);

        //
        // Need to get this name from the Props...
        //
        
    	ObjectList* pTargets = pServerDE->FindNamedObjects(pServerDE->GetStringData(m_hstrAttachTo));
        
        
	    if (!pTargets || pTargets->m_nInList <= 0) return;

        // Found all the Objects
    	ObjectLink *pLink = pTargets->m_pFirstLink;
        
    	while(pLink && pLink->m_hObject)
	    {
		    HCLASS hType = pServerDE->GetObjectClass(pLink->m_hObject);

            HCLASS hObjectTest = pServerDE->GetClass("CBaseCharacter");

		    if( (pServerDE->IsKindOf(hType, hObjectTest)) )
    		{
                m_hLinkObject = pLink->m_hObject;
				break;
    		}

            // Next object
		    pLink = pLink->m_pNext;
    	}

        // clean up	
	    pServerDE->RelinquishList(pTargets);

    }

    if (m_hLinkObject)
    {
        m_bFirstUpdate = DFALSE;

        // Create a Link to the Object on Fire
	   	pServerDE->CreateInterObjectLink(m_hObject, m_hLinkObject);

	    if (m_hLight == DNULL)
        {
            // Add Light
            ObjectCreateStruct theStruct;
        	INIT_OBJECTCREATESTRUCT(theStruct);

        	theStruct.m_Flags = FLAG_VISIBLE;
        	theStruct.m_ObjectType = OT_LIGHT; 
           //	VEC_COPY(theStruct.m_Pos, *pvPos);
            pServerDE->GetObjectPos(m_hObject, &theStruct.m_Pos);

        	HCLASS hClass = pServerDE->GetClass("BaseClass");
        
            if (hClass)
            {
                LPBASECLASS	pLight = pServerDE->CreateObject(hClass, &theStruct);
                if (pLight)
                {
            	    m_hLight = pLight->m_hObject;
                	pServerDE->SetLightRadius(m_hLight, 250.0f);
                	pServerDE->SetLightColor(m_hLight, 0.7f, 0.5f, 0.5f);
                }
            }        
        }
    } 
    
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FireFX::Update
//
//	PURPOSE:	Update the impact
//
// ----------------------------------------------------------------------- //

DBOOL FireFX::Update(DVector* pMovement)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, (DFLOAT)0.01);

    // Increase the Fire Index
    m_nFireIndex++;
    if (m_nFireIndex >= MAXFIRES)        m_nFireIndex = 0;
    
    // If First Update then look for the object to attach to...
    // and Add Light...
    //
	if (m_bFirstUpdate == DTRUE)
	{
		FirstUpdate();
	}
    else
    {
        if (m_hLinkObject)
        {
        
        	DFLOAT fTime = pServerDE->GetTime();
            
            //
            // Send Damage to LinkObject!!!
            //                  
            
            // Check for Object Dead
            if (m_bDead == DTRUE)
            {
                // Move the Smoke Up
            	if (m_hSmokeTrail[m_nFireIndex])
            	{
		            DVector vPos;
                    pServerDE->GetObjectPos(m_hSmokeTrail[m_nFireIndex], &vPos);
                    
                	vPos.y = vPos.y + 2;
		            pServerDE->SetObjectPos(m_hSmokeTrail[m_nFireIndex], &vPos);
            	}
                
                // Keep Smoke around longer 
            	if (fTime > m_fLastTime + 5.0f)
                {
                    for (int x=0; x<MAXFIRES; x++)
                    {
                       	if (m_hSmokeTrail[x])
						{
							pServerDE->RemoveObject(m_hSmokeTrail[x]);
							m_hSmokeTrail[x] = DNULL;
						}
                    }
                                
                    if (m_hLight)   pServerDE->RemoveObject( m_hLight );
                    pServerDE->RemoveObject( m_hObject );
                }
                return DTRUE;
                
            }
            else
            {
                if (m_hLinkObject)
                {
    	            if(pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hLinkObject), pServerDE->GetClass("CBaseCharacter"))) 
                    {
                        CBaseCharacter *pLinkObject = (CBaseCharacter*)pServerDE->HandleToObject(m_hLinkObject);
                        // Check to see if the Linked object is dead
                	    if(pLinkObject->IsDead() )
    	                {
                            for (int x=0; x<MAXFIRES; x++)
                            {
                            	if (m_hSprite[x])
								{
									pServerDE->RemoveObject(m_hSprite[x]);
									m_hSprite[x] = DNULL;
								}
                            }           
                            
                            m_bDead = DTRUE;
        	                m_fLastTime = pServerDE->GetTime();
                        
                            return DTRUE;
                        }
                    }        
                }
            }
            
            
            DRotation rRot;
            DVector m_vUp, m_vRight, m_vForward;
            DVector vPos, vTemp, vDir, vDims;

// Need to build a table of Sprites and There Positions...
// Then Update Each position???
//
            int x = m_nFireIndex;
            
            {
            	// Remove everything if we're done.
            	if (fTime > m_fStartTime[x] + m_fDuration[x])
        	    {
            
        	        m_fStartTime[x] = pServerDE->GetTime();
            
    		        if (m_hSprite[x]) 
					{
						g_pServerDE->RemoveObject(m_hSprite[x]);
						m_hSprite[x] = DNULL;
					}
    		        if (m_hSmokeTrail[x])
					{
						g_pServerDE->RemoveObject(m_hSmokeTrail[x]);
						m_hSmokeTrail[x] = DNULL;
					}
                
                    // Add Flame
                    VEC_INIT(m_vUp);
            	    VEC_INIT(m_vRight);
                	VEC_INIT(m_vForward);

                    if (!m_hLinkObject) return DFALSE;
            	    pServerDE->GetObjectRotation(m_hLinkObject, &rRot);
                	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);    

                    if (!m_hLinkObject) return DFALSE;
             	    pServerDE->GetObjectDims(m_hLinkObject, &vDims);
                	pServerDE->GetObjectPos(m_hLinkObject, &vPos);
                    
                    // Need to use the Dims of the Object to set these!!!
					VEC_MULSCALAR(vDims, vDims, 0.8f);
                    DFLOAT m_fForwardOffset = pServerDE->Random(-vDims.x, vDims.x);
                    DFLOAT m_fUpOffset = pServerDE->Random(-vDims.y, vDims.y/2.0f);
                    DFLOAT m_fRightOffset = pServerDE->Random(-vDims.z, vDims.z);
    
    	            VEC_MULSCALAR(vTemp, m_vForward, m_fForwardOffset);
        	        VEC_ADD(vPos, vPos, vTemp);
    
                    // vPos is a point in front of you.
                    // vDir is a point in front of you, but down
        	        VEC_COPY(vDir, vPos);
                	vDir.y = vDir.y - m_fUpOffset;
                	vDir.x = vDir.x - m_fRightOffset;
					vDir.z = vDir.z - m_fForwardOffset;
                
            	    VEC_COPY(vPos, vDir);

                    ObjectCreateStruct theStruct;
                    INIT_OBJECTCREATESTRUCT(theStruct);

                    theStruct.m_Flags = FLAG_VISIBLE;
                    theStruct.m_ObjectType = OT_SPRITE; 
					VEC_COPY(theStruct.m_Pos, vPos);

        			theStruct.m_SkinName[0] = '\0';
	        		theStruct.m_NextUpdate = 0.0f;
		        	_mbscpy((unsigned char*)theStruct.m_Filename, (const unsigned char*)"Sprites\\gibflame.spr");
				    VEC_COPY(theStruct.m_Scale, m_vScale)

                    HCLASS hClass = pServerDE->GetClass("BaseClass");
        
                    if (hClass)
                    {
                	    LPBASECLASS	pSprite = pServerDE->CreateObject(hClass, &theStruct);
                        m_hSprite[x] = pSprite->m_hObject;

                        if (!m_hLinkObject) return DFALSE;

						DVector vVel;
						pServerDE->GetVelocity(m_hLinkObject, &vVel);
						pServerDE->SetVelocity(pSprite->m_hObject, &vVel);

                    }        
                    

                    // Add Smoke
/*                    {
                    	ObjectCreateStruct theStruct;
                    	INIT_OBJECTCREATESTRUCT(theStruct);

                        pServerDE->GetObjectPos(m_hSprite[x], &vPos);
                       	vPos.y = vPos.y + 5;
                        
                        VEC_COPY(theStruct.m_Pos, vPos);
                    	HCLASS hClass = pServerDE->GetClass("CClientSmokeTrail");

                    	CClientSmokeTrail* pTrail = DNULL;

                    	if (hClass)
                    	{
		                    pTrail = (CClientSmokeTrail*)pServerDE->CreateObject(hClass, &theStruct);
                    	}
    
    	                if (pTrail)
                    	{
		                    DVector vVel;
                            if (!m_hLinkObject) return DFALSE;
        	    	        pServerDE->GetVelocity(m_hLinkObject, &vVel);
        		            pTrail->Setup(vVel, DFALSE);
                	    	m_hSmokeTrail[x] = pTrail->m_hObject;
                    	}
                    }            */

               	}
                
            	if (m_hSmokeTrail[x])
            	{
		            DVector vPos;
                
                    // Move the Smoke Up
                    if (m_hSprite[x])   pServerDE->GetObjectPos(m_hSprite[x], &vPos);
                    else                pServerDE->GetObjectPos(m_hSmokeTrail[x], &vPos);
                    
                	vPos.y = vPos.y + 5;
                
		            pServerDE->SetObjectPos(m_hSmokeTrail[x], &vPos);
	
            	}
            }
            
            
            if (m_hLight)
            {
                    DFLOAT fRed, fGreen, fBlue, fAlpha;
		            DVector vPos;
                    
                    
                    if (!m_hLinkObject) return DFALSE;
                    pServerDE->GetObjectPos(m_hLinkObject, &vPos);
    		        pServerDE->SetObjectPos(m_hLight, &vPos);
                    
                    if (!m_hLinkObject) return DFALSE;
                    pServerDE->GetObjectColor(m_hLinkObject, &fRed, &fGreen, &fBlue, &fAlpha);
                    pServerDE->SetObjectColor(m_hLinkObject, (fRed+0.3f), fGreen, fBlue, fAlpha);
            }    
        }            
        else
        {
            // Lost the Link, Remove Fire.
//            BPrint("Lost Link, Remove Fire...");
            
            for (int x=0; x<MAXFIRES; x++)
            {
               	if (m_hSprite[x])
				{
					pServerDE->RemoveObject(m_hSprite[x]);
					m_hSprite[x] = DNULL;
				}
               	if (m_hSmokeTrail[x])
				{
					pServerDE->RemoveObject(m_hSmokeTrail[x]);
					m_hSmokeTrail[x] = DNULL;
				}
            }
                        
            if (m_hLight)   pServerDE->RemoveObject( m_hLight );
            pServerDE->RemoveObject( m_hObject );
            
            return DFALSE;
        }
     }


	return DTRUE;
}




