/////////////////////////////////////////////////////////////////
// Copyright (C), RenEvo Software & Designs, 2008
// FGPlugin Source File
//
// PluginMain.cpp
//
// Purpose: Flowgraph Plugin DLL entry point
//
// History:
//	- 5/31/08 : File created - KAK
/////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <CryLibrary.h>
#include <platform_impl.h>
#include <windows.h>
#include "Nodes/G2FlowBaseNode.h"


// Plugin's name
#define PLUGIN_NAME "Move Bones"

// PLugin's author
#define PLUGIN_AUTHOR "Josh Harle"


void* g_hInst = 0;
IGame* g_pGame = 0;
SCVars *g_pGameCVars = 0;

// Needed for the Game02 specific flow node
CG2AutoRegFlowNodeBase *CG2AutoRegFlowNodeBase::m_pFirst=0;
CG2AutoRegFlowNodeBase *CG2AutoRegFlowNodeBase::m_pLast=0;

// DLL Entry point
BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		g_hInst = hInst;
		PLUGIN_VERSION.Set(CURRENT_PLUGIN_VERSION);
	}
	return TRUE;
}

// Setup global variables
void SetupSystem(ISystem *pSystem, SCVars *pCVars)
{
	gEnv = pSystem->GetGlobalEnvironment();
	g_pGame = pSystem->GetIGame();
	g_pGameCVars = pCVars;
}

extern "C"
{
	/////////////////////////////////////////////////////////////////
	// RegisterWithPluginSystem
	//
	// Purpose: Called by system when plugin is registered
	//
	// In:	pSystem - Environment system
	//		pCVars - CVar definitions
	//
	// Out: plugin - Plugin information returned to system
	//
	// Returns TRUE if registered properly, FALSE otherwise
	/////////////////////////////////////////////////////////////////
	GAME_API bool RegisterWithPluginSystem(ISystem *pSystem, SCVars *pCVars, SPluginRegister &plugin)
	{
		SetupSystem(pSystem, pCVars);

		// Fill in register
		plugin.version = PLUGIN_VERSION;
		plugin.nodesFirst = CG2AutoRegFlowNodeBase::m_pFirst;
		plugin.nodesLast = CG2AutoRegFlowNodeBase::m_pLast;

		return true;
	}

	/////////////////////////////////////////////////////////////////
	// GetName
	//
	// Purpose: Returns the name of the Plugin
	/////////////////////////////////////////////////////////////////
	GAME_API const char* GetName()
	{
		return PLUGIN_NAME;
	}

	/////////////////////////////////////////////////////////////////
	// GetAuthor
	//
	// Purpose: Returns the name of the Plugin's author
	/////////////////////////////////////////////////////////////////
	GAME_API const char* GetAuthor()
	{
		return PLUGIN_AUTHOR;
	}

	/////////////////////////////////////////////////////////////////
	// GetVersionStr
	//
	// Purpose: Returns the version of this Plugin
	/////////////////////////////////////////////////////////////////
	GAME_API const char* GetVersionStr()
	{
		return CURRENT_PLUGIN_VERSION;
	}

	/////////////////////////////////////////////////////////////////
	// GetNodeList
	//
	// Purpose: Returns the names of all nodes contained in this plugin
	/////////////////////////////////////////////////////////////////
	GAME_API const char* GetNodeList()
	{
		static char list[4098] = {0};
		if (!list[0])
		{
			CG2AutoRegFlowNodeBase *p = CG2AutoRegFlowNodeBase::m_pFirst;
			if (p)
			{
				strcpy(list,p->m_sClassName);
				p = p->m_pNext;
				while (p)
				{
					strcat(list,"\r\n");
					strcat(list,p->m_sClassName);
					p = p->m_pNext;
				}
			}
		}
		return list;
	}
}
