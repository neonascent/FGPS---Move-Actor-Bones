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
class CFlowMoveBones : public CFlowBaseNode
{

	enum EInputPorts
	{
		EIP_Move,
		EIP_Target,
		EIP_JointName,
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
	string boneName;
	Vec3 targetPos;

	SActivationInfo m_actInfo;  // is this needed?  We already just use pActInfo

public:
	////////////////////////////////////////////////////
	CFlowMoveBones(SActivationInfo *pActInfo)
	{
		m_pTarget = NULL;
		boneName = "";
		targetPos = Vec3(ZERO);
	}

	
	////////////////////////////////////////////////////
	virtual ~CFlowMoveBones(void)
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
			InputPortConfig<string>("JointName", "Pelvis", _HELP("Name of the bone to move"), 0, 0),
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
				boneName = "";
				targetPos = Vec3(ZERO);
			}
			break;
		case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Move))
				{
					const EntityId targetId = GetPortEntityId(pActInfo, EIP_Target);
					m_pTarget = gEnv->pEntitySystem->GetEntity(targetId);
					boneName = GetPortString(pActInfo, EIP_JointName);
					targetPos = GetPortVec3(pActInfo, EIP_Coordinate);
					
					MoveBone(m_pTarget, boneName, targetPos);
					


				}
			}
			break;
		}
	}


	void MoveBone (IEntity *pTarget, string bone, Vec3& pos) {

		// Target must be an actor
		IActorSystem *pActSys = g_pGame->GetIGameFramework()->GetIActorSystem();
		if (pActSys)
		{
			//CActor *pActor = static_cast<CActor *> (pActSys->GetActor(pTarget->GetId()));
			IActor *pActor = (pActSys->GetActor(pTarget->GetId()));
			if (pActor)
			{
				/*		
				int limbID = pActor->GetIKLimbIndex(bone);
				if (limbID > -1) {*/
					pActor->SetIKPos(bone, pos, 1);
					ActivateOutput(&m_actInfo, EOP_Success, true); 
					return;
				/*} else {
					// not a valid bone
					ActivateOutput(&m_actInfo, EOP_Error, (string) "Not a valid bone" );
				}*/
				
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
		return new CFlowMoveBones(pActInfo);
	}

	////////////////////////////////////////////////////
	virtual void GetMemoryStatistics(ICrySizer *s)
	{
		s->Add(*this);
	}

};



////////////////////////////////////////////////////
////////////////////////////////////////////////////

REGISTER_FLOW_NODE("Actor:MoveBones", CFlowMoveBones);