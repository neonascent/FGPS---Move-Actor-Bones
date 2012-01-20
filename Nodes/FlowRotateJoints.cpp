/////////////////////////////////////////////////////////////////
// Copyright (C), RenEvo Software & Designs, 2008
// FGPlugin Source File
//
// FlowXmlDataNodes.cpp
//
// Purpose: Flowgraph nodes to dealing with data in Xml elements
//
// History:
//	- 8/23/08 : File created - KAK
/////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Nodes/G2FlowBaseNode.h"
#include "IEntitySystem.h"
#include "IActorSystem.h"
#include "IMovementController.h"
#include "Actor.h"

////////////////////////////////////////////////////
class CFlowRotateJoints : public CFlowBaseNode
{

	enum EInputPorts
	{
		EIP_Move,
		EIP_Target,
		EIP_JointName,
		EIP_X,
		EIP_Y,
		EIP_Z
	};

	enum EOutputs
	{
		EOP_Success = 0,
		EOP_Fail,
		EOP_Error
	};
	
	// Actor stuff
	IEntity *m_pTarget;
	string jointName;
	Vec3 rotationVector;

	SActivationInfo m_actInfo;  // is this needed?  We already just use pActInfo

public:
	////////////////////////////////////////////////////
	CFlowRotateJoints(SActivationInfo *pActInfo)
	{
		m_pTarget = NULL;
		jointName = "";
		rotationVector = Vec3(ZERO);
	}

	
	////////////////////////////////////////////////////
	virtual ~CFlowRotateJoints(void)
	{

	}

	////////////////////////////////////////////////////
	virtual void Serialize(SActivationInfo *pActInfo, TSerialize ser)
	{
		
	}

	////////////////////////////////////////////////////
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		// Define input ports here, in same order as EInputPorts

		static const SInputPortConfig inputs[] =
		{
			InputPortConfig_Void("Move", _HELP("Move the bone")),
			InputPortConfig<EntityId>("Target", _HELP("Target entity whose bones to move")),
			InputPortConfig<string>("JointName", "Bip01 R Forearm", _HELP("Name of the joint to move"), 0, 0),
			InputPortConfig <float> ( "X Rotation", 0 , _HELP("X rotation in degrees" )),
			InputPortConfig <float> ( "Y Rotation", 0 , _HELP("Y rotation in degrees" )),
			InputPortConfig <float> ( "Z Rotation", 0 , _HELP("Z rotation in degrees" )),
			{0}
		};

		// Define output ports here, in same oreder as EOutputPorts
		static const SOutputPortConfig outputs[] =
		{
			OutputPortConfig<bool>("Success", _HELP("Moved")), 
			OutputPortConfig<bool>("Fail", _HELP("Didn't move")), 
			OutputPortConfig<string>("Error", _HELP("Any error messages")), 
			{0}
		};

		// Fill in configuration
		config.pInputPorts = inputs;
		config.pOutputPorts = outputs;
		config.sDescription = _HELP("Moves bones");
		//config.SetCategory(EFLN_ADVANCED);
	}


	////////////////////////////////////////////////////
	virtual void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
	{
		m_actInfo = *pActInfo;
		switch (event)
		{
		case eFE_Initialize:
			{
				m_pTarget = NULL;
				jointName = "";
				rotationVector = Vec3(ZERO);
			}
			break;
		case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Move))
				{
					const EntityId targetId = GetPortEntityId(pActInfo, EIP_Target);
					m_pTarget = gEnv->pEntitySystem->GetEntity(targetId);
					jointName = GetPortString(pActInfo, EIP_JointName);
					float X = GetPortFloat(pActInfo, EIP_X) * 0.0174532925;
					float Y = GetPortFloat(pActInfo, EIP_Y) * 0.0174532925;
					float Z = GetPortFloat(pActInfo, EIP_Z) * 0.0174532925;
					rotationVector = Vec3(X, Y, Z);
					
					MoveBone(m_pTarget, jointName, rotationVector);
					


				}
			}
			break;
		}
	}


	void MoveBone (IEntity *pTarget, string joint, Vec3& rot) {


		IEntity *pObjectiveEntity = GetISystem()->GetIEntitySystem()->GetEntity(pTarget->GetId());
		if(!pObjectiveEntity) {
			ActivateOutput(&m_actInfo, EOP_Error, (string) "Entity not an pObjectiveEntity" );
			return;
		}

		Vec3 vWorldPos = Vec3(0,0,0);

		SEntitySlotInfo info;
		int slotCount = pObjectiveEntity->GetSlotCount();
		for(int i=0; i<slotCount; ++i)
		{
			if (pObjectiveEntity->GetSlotInfo(i, info))
			{
				if (info.pCharacter)
				{
					int16 id = info.pCharacter->GetISkeletonPose()->GetJointIDByName(joint);
					if (id >= 0)
					{
						//if it has a head, then lets move its bits!
						ISkeletonPose *pSkeletonPose = info.pCharacter->GetISkeletonPose();
						QuatT lQuat = pSkeletonPose->GetRelJointByID(id);
						//lQuat.q *= Quat::CreateRotationAA(180,Vec3(0.0f,1.0f,0.0f));
						lQuat.q = Quat::CreateRotationXYZ( Ang3(rot[0], rot[1], rot[2]) ); //  without *

						pSkeletonPose->SetPostProcessQuat(id, lQuat);
						
						
						
						ActivateOutput(&m_actInfo, EOP_Success, true); 
						ActivateOutput(&m_actInfo, EOP_Error, (string) joint + " found! tried to move" ); // 
						return;
						/*
						//vPos = pCharacter->GetISkeleton()->GetHelperPos(helper);
						vWorldPos = info.pCharacter->GetISkeletonPose()->GetAbsJointByID(id).t;
						if (!vWorldPos.IsZero())
						{
							vWorldPos = pObjectiveEntity->GetSlotWorldTM(i).TransformPoint(vWorldPos);
							break;
						}
						*/

					} else {
						ActivateOutput(&m_actInfo, EOP_Fail, true); 
						ActivateOutput(&m_actInfo, EOP_Error, (string) joint + " NOT found!" );
						return;
					}
				} else {
					ActivateOutput(&m_actInfo, EOP_Fail, true); 
					ActivateOutput(&m_actInfo, EOP_Error, (string) "Character NOT found!" );
					return;
				}
			} else {
				ActivateOutput(&m_actInfo, EOP_Fail, true); 
				ActivateOutput(&m_actInfo, EOP_Error, (string) "Couldn't find GetSlotInfo" );
				return;
			}
		}

		

//		// Target must be an actor
//		IActorSystem *pActSys = g_pGame->GetIGameFramework()->GetIActorSystem();
//		if (pActSys)
//		{
//			// need to compile against FGPS.dll, and had a problem building FGPS.dll that was solved by http://www.crydev.net/viewtopic.php?p=659601#p659601
//			
//			//CActor *pActor = static_cast<CActor *> (pActSys->GetActor(pTarget->GetId()));
//			IActor *pActor = (pActSys->GetActor(pTarget->GetId()));
//			if (pActor)
//			{
//				
//				
//				/*		
//				int limbID = pActor->GetIKLimbIndex(bone);
//				if (limbID > -1) {*/
//					pActor->SetIKPos(bone, pos, 1);
//					ActivateOutput(&m_actInfo, EOP_Success, true); 
//					return;
//				/*} else {
//					// not a valid bone
//					ActivateOutput(&m_actInfo, EOP_Error, (string) "Not a valid bone" );
//				}*/
//				
//			} else {
//				// not an actor
//				ActivateOutput(&m_actInfo, EOP_Error, (string) "Entity not an actor" );
//			}
//		}
//
//		// failed, for some reason shown in EOP_Error
//		ActivateOutput(&m_actInfo, EOP_Fail, true); 
		return;
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo)
	{
		return new CFlowRotateJoints(pActInfo);
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer *s)
	{
		s->Add(*this);
	}

};



////////////////////////////////////////////////////
////////////////////////////////////////////////////

REGISTER_FLOW_NODE("Actor:RotateJoints", CFlowRotateJoints);