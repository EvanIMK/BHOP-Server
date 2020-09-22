#include <sourcemod>
#include <sdkhooks>
#include <sdktools>
#undef REQUIRE_EXTENSIONS 
#include <outputinfo>
#include <improved-st>

#define PLUGIN_VERSION "1.1.0"

// Enforce new syntax
#pragma newdecls required

// Notify me if I miss a semicolon
#pragma semicolon 1

// Entity is completely ignored by the client.
// Can cause prediction errors if a player proceeds to collide with it on the server.
// https://developer.valvesoftware.com/wiki/Effects_enum
#define EF_NODRAW 32

int g_iEffects = -1;
bool g_bShowTriggers[MAXPLAYERS + 1];
int g_iTransmit;
public Plugin myinfo =
{
    name = "Show Triggers",
    author = "Ici, Eric, Blank",
    description = "Make trigger brushes visible.",
    version = PLUGIN_VERSION,
	url = "http://steamcommunity.com/id/1ci & https://steamcommunity.com/id/-eric"
};

public void OnPluginStart()
{
	g_iEffects = FindSendPropInfo("CBaseEntity", "m_fEffects");

	if (g_iEffects == -1)
	{
		SetFailState("Couldn't find offset for m_fEffects");
	}

	// Event hooks
	HookEvent("round_start", OnRoundStart);

	// ConVars
	CreateConVar("showtriggers_version", PLUGIN_VERSION, "Show triggers version", FCVAR_NOTIFY | FCVAR_REPLICATED);

	// Register commands
	RegConsoleCmd("sm_showtriggers", CommandShowTriggers, "");
	RegConsoleCmd("sm_st", CommandShowTriggers, "");
	RegConsoleCmd("sm_stmenu", CommandShowTriggersMenu, "");
}

public void OnRoundStart(Event event, const char[] name, bool dontBroadcast)
{
	int entity = -1;
	char buffer[32];

	// Loop through all triggers
	while ((entity = FindEntityByClassname(entity, "trigger_*")) != -1)
	{
		GetEntityClassname(entity, buffer, sizeof(buffer));
		
		if (StrEqual(buffer, "trigger_push"))
		{
			SetEntityRenderColor(entity, 0, 255, 0, 255);
		}
		else if (StrEqual(buffer, "trigger_teleport"))
		{
			SetEntityRenderColor(entity, 255, 0, 0, 255);
		}
		else
		{
			SetEntityRenderColor(entity, 0, 0, 0, 0);
		}
		int count = GetOutputCount(entity, "m_OnStartTouch");
		for (int i = 0; i < count; i++)
		{
			GetOutputParameter(entity, "m_OnStartTouch", i, buffer);
			// Gravity anti-pre
			// https://gamebanana.com/prefabs/6760
			if (StrEqual(buffer, "gravity 40"))
				SetEntityRenderColor(entity, 255, 100, 0, 255);
		}

		count = GetOutputCount(entity, "m_OnEndTouch");
		for (int i = 0; i < count; i++)
		{
			GetOutputParameter(entity, "m_OnEndTouch", i, buffer);

			// Gravity booster
			// https://gamebanana.com/prefabs/6677
			if (StrContains(buffer, "gravity -") != -1)
				SetEntityRenderColor(entity, 0, 255, 185, 255);

			// Basevelocity booster
			// https://gamebanana.com/prefabs/7118
			if (StrContains(buffer, "basevelocity") != -1)
				SetEntityRenderColor(entity, 0, 255, 0, 255);
		}
	}
}

public void OnClientPutInServer(int client)
{
	g_bShowTriggers[client] = false;
}

public int MenuHandle(Menu menu, MenuAction action, int param1, int param2)
{
	if (action == MenuAction_End)
	{
		delete menu;
	}
}

public Action CommandShowTriggersMenu(int client, int args)
{
	CPrintToChat(client, "Displaying trigger menu.");
	Menu menu = new Menu(MenuHandle);
	menu.SetTitle("Trigger Menu");
	menu.AddItem("l1", "Red = Teleport / Bhop Block");
	menu.AddItem("l2", "Green = Booster / Gravity");
	menu.AddItem("l3", "Orange = Anti Prespeed");
	menu.ExitButton = true;
	menu.Display(client, 15);
	
	return Plugin_Handled;
}

public Action CommandShowTriggers(int client, int args)
{
	if (!IsValidClient(client))
	{
		return Plugin_Handled;
	}

	g_bShowTriggers[client] = !g_bShowTriggers[client];

	if (g_bShowTriggers[client])
	{
		g_iTransmit++;
		CPrintToChat(client, "Displaying triggers.");
		CPrintToChat(client, "!stmenu to view the color descriptions.");
	}
	else
	{
		g_iTransmit--;
		CPrintToChat(client, "Stopped displaying triggers.");
	}
    
	TransmitTriggers(g_iTransmit > 0);
    
	return Plugin_Handled;
}

void TransmitTriggers(bool transmit)
{
	static bool s_bHook = false;
	if (s_bHook == transmit)
	{
		return;
	}

	char buffer[8];
	int entityCount = GetEntityCount();

	// Loop through entities
	for (int i = MaxClients + 1; i <= entityCount; i++)
	{
		if (!IsValidEdict(i))
		{
			continue;
		}

		// Is this entity a trigger?
		GetEdictClassname(i, buffer, sizeof(buffer));
		if (strcmp(buffer, "trigger") != 0)
		{
			continue;
		}
	
		// Is this entity's model a VBSP model?
		GetEntPropString(i, Prop_Data, "m_ModelName", buffer, sizeof(buffer));
		if (buffer[0] != '*')
		{
			// The entity must have been created by a plugin and assigned some random model.
			// Skipping in order to avoid console spam.
			continue;
		}
		
		if(!GetEntProp(i, Prop_Data, "m_spawnflags"))
		{
			continue;
		}

		// Get flags
		int effectFlags = GetEntData(i, g_iEffects);
		int edictFlags = GetEdictFlags(i);

		// Determine whether to show triggers or not
		if (transmit)
		{
			effectFlags &= ~EF_NODRAW;
			edictFlags &= ~FL_EDICT_DONTSEND;
		}
		else
		{
			effectFlags |= EF_NODRAW;
			edictFlags |= FL_EDICT_DONTSEND;
		}
		// Apply state changes
		SetEntData(i, g_iEffects, effectFlags);
		ChangeEdictState(i, g_iEffects);
		SetEdictFlags(i, edictFlags);

		// Should we hook?
		if (transmit)
		{
			SDKHook(i, SDKHook_SetTransmit, OnSetTransmit);
		}
		else
		{
			SDKUnhook(i, SDKHook_SetTransmit, OnSetTransmit);
		}
	}
}

public Action OnSetTransmit(int entity, int client)
{
	if (!g_bShowTriggers[client])
	{
		return Plugin_Handled;
	}

	return Plugin_Continue;
}

stock bool IsValidClient(int client)
{
	return (0 < client && client <= MaxClients && IsClientInGame(client) && !IsFakeClient(client));
}
