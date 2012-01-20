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
class CFlowMoveJoints : public CFlowBaseNode
{

	enum EInputPorts
	{
		EIP_Move,
		EIP_Target,
		EIP_StartJointName,
		EIP_MidJointName,
		EIP_EndJointName,
		EIP_Coordinate,
	};

	enum EOutputs
	{
		EOP_Success = 0,
		EOP_Fail,
		EOP_Error
	};
	
	// Actor stuff
	IEntity *m_pTarget;
	string StartJointName;
	string MidJointName;
	string EndJointName;
	Vec3 targetPos;

	SActivationInfo m_actInfo;  // is this needed?  We already just use pActInfo

public:
	////////////////////////////////////////////////////
	CFlowMoveJoints(SActivationInfo *pActInfo)
	{
		m_pTarget = NULL;
		StartJointName = "";
		MidJointName = "";
		EndJointName = "";
		targetPos = Vec3(ZERO);
	}

	
	////////////////////////////////////////////////////
	virtual ~CFlowMoveJoints(void)
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
			InputPortConfig<string>("StartJointName", "Bip01 L Thigh", _HELP("Name of the bone to move"), 0, 0),
			InputPortConfig<string>("MidJointName", "Bip01 L Calf", _HELP("optional - can leave blank"), 0, 0),
			InputPortConfig<string>("EndJointName", "Bip01 L Foot", _HELP("Name of the bone to move"), 0, 0),
			InputPortConfig<Vec3> ( "Pos", Vec3(ZERO), _HELP("Where to move to" )),
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
				StartJointName = "";
				MidJointName = "";
				EndJointName = "";
				targetPos = Vec3(ZERO);
			}
			break;
		case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Move))
				{
					const EntityId targetId = GetPortEntityId(pActInfo, EIP_Target);
					m_pTarget = gEnv->pEntitySystem->GetEntity(targetId);
					StartJointName = GetPortString(pActInfo, EIP_StartJointName);
					MidJointName = GetPortString(pActInfo, EIP_MidJointName);
					EndJointName = GetPortString(pActInfo, EIP_EndJointName);
					targetPos = GetPortVec3(pActInfo, EIP_Coordinate);
					pActInfo->pGraph->SetRegularlyUpdated(pActInfo->myID, true);
				}
			}
			break;
		case eFE_Update:
			{
				gEnv->pRenderer->GetIRenderAuxGeom()->DrawSphere(targetPos,0.2f,ColorB(0,255,255,255) );
				MoveBone();
			}
			break;
		}
	}


	void MoveBone() {

		// Target must be an actor
		IActorSystem *pActSys = g_pGame->GetIGameFramework()->GetIActorSystem();
		if (pActSys)
		{
			// need to compile against FGPS.dll, and had a problem building FGPS.dll that was solved by http://www.crydev.net/viewtopic.php?p=659601#p659601

			IActor *pActor = (pActSys->GetActor(m_pTarget->GetId()));
			if (pActor)
			{	

				
				ICharacterInstance *pCharacter = m_pTarget->GetCharacter(0);
				int rootBoneID = pCharacter->GetISkeletonPose()->GetJointIDByName(StartJointName);
				int middleBoneID = pCharacter->GetISkeletonPose()->GetJointIDByName(MidJointName);
				int endBoneID = pCharacter->GetISkeletonPose()->GetJointIDByName(EndJointName);



				if ((rootBoneID > -1) && (endBoneID > -1)) {

					//Vec3 vRootBone = pCharacter->GetISkeletonPose()->GetAbsJointByID(0).t; // - pOwner->GetSlotLocalTM (characterSlot, false).GetTranslation ();


					// from void SIKLimb::Update(IEntity *pOwner,float frameTime) in Actor.cpp

					Vec3 vRootBone = pCharacter->GetISkeletonPose()->GetAbsJointByID(0).t; // - pOwner->GetSlotLocalTM (characterSlot, false).GetTranslation ();
					vRootBone.z=0;
					Vec3 lAnimPos = pCharacter->GetISkeletonPose()->GetAbsJointByID(endBoneID).t-vRootBone;// - pOwner->GetSlotLocalTM (characterSlot, false).GetTranslation ();



					if (middleBoneID > -1)
					{
						uint32 test = pCharacter->GetISkeletonPose()->SetCustomArmIK(targetPos,rootBoneID,middleBoneID,endBoneID);
					}
					else
					{
						ISkeletonPose* pISkeletonPose = pCharacter->GetISkeletonPose();
						uint32 numJoints	= pISkeletonPose->GetJointCount();
						QuatT* pRelativeQuatIK = (QuatT*)alloca( numJoints*sizeof(QuatT) );
						QuatT* pAbsoluteQuatIK = (QuatT*)alloca( numJoints*sizeof(QuatT) );

						pISkeletonPose->CCDInitIKBuffer(pRelativeQuatIK,pAbsoluteQuatIK);
						pISkeletonPose->CCDInitIKChain(rootBoneID,endBoneID);

						//Vec3 limbEndNormal(0,0,0);
						//limbEndNormal = Matrix33(pISkeleton->GetAbsJMatrixByID(endBoneID)).GetColumn(0);
						//gEnv->pRenderer->GetIRenderAuxGeom()->DrawLine(targetPos, ColorB(0,255,0,100), targetPos + limbEndNormal * 3.0f, ColorB(255,0,0,100));
						pISkeletonPose->CCDRotationSolver(targetPos,0.02f,0.25f,100,ZERO,pRelativeQuatIK,pAbsoluteQuatIK);
						pISkeletonPose->CCDTranslationSolver(targetPos,pRelativeQuatIK,pAbsoluteQuatIK);
						pISkeletonPose->CCDUpdateSkeleton(pRelativeQuatIK,pAbsoluteQuatIK);

					}

					ActivateOutput(&m_actInfo, EOP_Success, true );
					return;

				} else {
					// not a bone name
					ActivateOutput(&m_actInfo, EOP_Error, (string) "A start or end joint name is not found" );
				}

			} else {
				// not an actor
				ActivateOutput(&m_actInfo, EOP_Error, (string) "Entity not an actor" );
			}
		}

		// failed, for some reason shown in EOP_Error
		ActivateOutput(&m_actInfo, EOP_Fail, true); 
		return;
	}

	////////////////////////////////////////////////////
	virtual IFlowNodePtr Clone(SActivationInfo *pActInfo)
	{
		return new CFlowMoveJoints(pActInfo);
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer *s)
	{
		s->Add(*this);
	}

};



////////////////////////////////////////////////////
////////////////////////////////////////////////////

REGISTER_FLOW_NODE("Actor:MoveJoints", CFlowMoveJoints);	