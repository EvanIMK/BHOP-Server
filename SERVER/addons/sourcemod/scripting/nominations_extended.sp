#include <sourcemod>
#include <mapchooser>
#include "include/mapchooser_extended"
#include <colors>
#tryinclude <shavit>
// put try include for surf 
#pragma semicolon 1
#pragma newdecls required

#define MCE_VERSION "1.10.0"

public Plugin myinfo =
{
	name = "Map Nominations Extended",
	author = "Powerlord and AlliedModders LLC", // mbhound version ( ͡° ͜ʖ ͡°)
	description = "Provides Map Nominations",
	version = MCE_VERSION,
	url = "https://forums.alliedmods.net/showthread.php?t=156974"
};

Handle g_Cvar_ExcludeOld = INVALID_HANDLE;
Handle g_Cvar_ExcludeCurrent = INVALID_HANDLE;
Handle g_Cvar_DisplayName = INVALID_HANDLE;
Handle g_Cvar_TierMenu = INVALID_HANDLE;

Handle g_MapList = INVALID_HANDLE;
Handle g_PopularMapList = INVALID_HANDLE;
Handle g_MapMenu = INVALID_HANDLE;
Handle g_PopularMapMenu = INVALID_HANDLE;
int g_mapFileSerial = -1;

Menu g_TiersMenu;
Menu g_Tier1Menu;
Menu g_Tier2Menu;
Menu g_Tier3Menu;
Menu g_Tier4Menu;
Menu g_Tier5Menu;
Menu g_Tier6Menu;

bool g_bShavit = false;

#define MAPSTATUS_ENABLED (1<<0)
#define MAPSTATUS_DISABLED (1<<1)
#define MAPSTATUS_EXCLUDE_CURRENT (1<<2)
#define MAPSTATUS_EXCLUDE_PREVIOUS (1<<3)
#define MAPSTATUS_EXCLUDE_NOMINATED (1<<4)

Handle g_mapTrie;

// Nominations Extended Convars
Handle g_Cvar_MarkCustomMaps = INVALID_HANDLE;

public void OnPluginStart()
{
	LoadTranslations("common.phrases");
	LoadTranslations("nominations.phrases");
	LoadTranslations("basetriggers.phrases"); // for Next Map phrase
	LoadTranslations("mapchooser_extended.phrases");
	
	int arraySize = ByteCountToCells(PLATFORM_MAX_PATH);	
	g_MapList = CreateArray(arraySize);
	g_PopularMapList = CreateArray(arraySize);
	
	g_Cvar_ExcludeOld = CreateConVar("sm_nominate_excludeold", "1", "Specifies if the current map should be excluded from the Nominations list", 0, true, 0.00, true, 1.0);
	g_Cvar_ExcludeCurrent = CreateConVar("sm_nominate_excludecurrent", "1", "Specifies if the MapChooser excluded maps should also be excluded from Nominations", 0, true, 0.00, true, 1.0);
	g_Cvar_DisplayName = CreateConVar("sm_nominate_displayname", "1", "Use custom Display Names instead of the raw map name", 0, true, 0.00, true, 1.0);
	g_Cvar_TierMenu = CreateConVar("smc_tier_menu", "1", "Nominate menu can show maps by alphabetic order and tiers", 0, true, 0.0, true, 1.0 );


	RegConsoleCmd("sm_nominate", Command_Nominate);
	
	RegAdminCmd("sm_nominate_addmap", Command_Addmap, ADMFLAG_CHANGEMAP, "sm_nominate_addmap <mapname> - Forces a map to be on the next mapvote.");
	
	// Nominations Extended cvars
	CreateConVar("ne_version", MCE_VERSION, "Nominations Extended Version", FCVAR_SPONLY|FCVAR_NOTIFY|FCVAR_DONTRECORD);

	g_mapTrie = CreateTrie();
}

public void OnAllPluginsLoaded()
{
	// This is an MCE cvar... this plugin requires MCE to be loaded.  Granted, this plugin SHOULD have an MCE dependency.
	g_Cvar_MarkCustomMaps = FindConVar("mce_markcustommaps");
}

public void OnLibraryAdded(const char[] szName)
{
	if (StrEqual(szName, "shavit"))
	{
		g_bShavit = true;
	}
}

public void OnLibraryRemoved(const char[] szName)
{
	if (StrEqual(szName, "shavit"))
	{
		g_bShavit = false;
	}
}

public void OnConfigsExecuted()
{
	if (ReadMapList(g_MapList, g_mapFileSerial, "nominations", MAPLIST_FLAG_CLEARARRAY|MAPLIST_FLAG_MAPSFOLDER) == INVALID_HANDLE)
	{
		if (g_mapFileSerial == -1)
		{
			SetFailState("Unable to create a valid map list.");
		}
	}	
	BuildMapMenu();
	
	if(ReadMapList(g_PopularMapList, g_mapFileSerial, "popular", MAPLIST_FLAG_CLEARARRAY|MAPLIST_FLAG_MAPSFOLDER) == INVALID_HANDLE)
	{
		if (g_mapFileSerial == -1)
		{
			SetFailState("Unable to create a valid popular maps list.");
		}
	}
	BuildPopularMenu();

	if (GetConVarBool(g_Cvar_TierMenu))
	{
		BuildTierMenus();
	}
}

public void OnNominationRemoved(const char[] map, int owner)
{
	int status;

	char resolvedMap[PLATFORM_MAX_PATH];
	FindMap(map, resolvedMap, sizeof(resolvedMap));

	/* Is the map in our list? */
	if (!GetTrieValue(g_mapTrie, map, status))
	{
		return;	
	}
	
	/* Was the map disabled due to being nominated */
	if ((status & MAPSTATUS_EXCLUDE_NOMINATED) != MAPSTATUS_EXCLUDE_NOMINATED)
	{
		return;
	}
	
	SetTrieValue(g_mapTrie, map, MAPSTATUS_ENABLED);	
}

stock void getMapName(const char[] map, char[] mapName, int size)
{
	if (GetConVarBool(g_Cvar_DisplayName))
	{
		GetMapName(map, mapName, size);
		return;
	}
	strcopy(mapName, size, map);
}

public Action Command_Addmap(int client, int args)
{
	if (args < 1)
	{
		CReplyToCommand(client, "[IMK] Usage: sm_nominate_addmap <mapname>");
		return Plugin_Handled;
	}
	
	char mapname[PLATFORM_MAX_PATH];
	char resolvedMap[PLATFORM_MAX_PATH];
	GetCmdArg(1, mapname, sizeof(mapname));

	if (FindMap(mapname, resolvedMap, sizeof(resolvedMap)) == FindMap_NotFound)
	{
		// We couldn't resolve the map entry to a filename, so...
		ReplyToCommand(client, "%t", "Map was not found", mapname);
		return Plugin_Handled;		
	}

	char mapName[PLATFORM_MAX_PATH];
	getMapName(mapname, mapName, sizeof(mapName));
	
	int status;
	if (!GetTrieValue(g_mapTrie, mapname, status))
	{
		CReplyToCommand(client, "%t", "Map was not found", mapName);
		return Plugin_Handled;		
	}
	
	NominateResult result = NominateMap(mapname, true, 0);
	
	if (result > Nominate_Replaced)
	{
		/* We assume already in vote is the casue because the maplist does a Map Validity check and we forced, so it can't be full */
		CReplyToCommand(client, "%t", "Map Already In Vote", mapName);
		
		return Plugin_Handled;	
	}

	SetTrieValue(g_mapTrie, mapname, MAPSTATUS_DISABLED|MAPSTATUS_EXCLUDE_NOMINATED);

	CReplyToCommand(client, "%t", "Map Inserted", mapName);
	LogAction(client, -1, "\"%L\" inserted map \"%s\".", client, mapname);

	return Plugin_Handled;		
}

public void OnClientSayCommand_Post(int client, const char[] command, const char[] sArgs)
{
	if (!client)
	{
		return;
	}
	
	if (strcmp(sArgs, "nominate", false) == 0)
	{
		ReplySource old = SetCmdReplySource(SM_REPLY_TO_CHAT);
		
		AttemptNominate(client);
		
		SetCmdReplySource(old);
	}
}

public Action Command_Nominate(int client, int args)
{
	if (!client || !IsNominateAllowed(client))
	{
		return Plugin_Handled;
	}
	
	if (args == 0)
	{
		if (GetConVarBool(g_Cvar_TierMenu)) 
		{
			OpenTiersMenu(client);
		}
		else
		{
			AttemptNominate(client);
		}
		return Plugin_Handled;
	}
	
	char mapname[PLATFORM_MAX_PATH];
	GetCmdArg(1, mapname, sizeof(mapname));

	ShowMatches(client, mapname);

	return Plugin_Continue;
}

void ShowMatches(int client, char[] mapname) 
{
	Menu SubMapMenu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	SetMenuTitle(SubMapMenu, "Nominate Menu\nMaps matching \"%s\"\n ", mapname);
	SetMenuExitButton(SubMapMenu, true);

	bool isCurrent = false;
	bool isExclude = false;

	char map[PLATFORM_MAX_PATH];
	char lastMap[PLATFORM_MAX_PATH];

	Handle excludeMaps = INVALID_HANDLE;
	char currentMap[32];
	
	excludeMaps = CreateArray(ByteCountToCells(PLATFORM_MAX_PATH));
	GetExcludeMapList(excludeMaps);

	GetCurrentMap(currentMap, sizeof(currentMap));	

	for (int i = 0; i < GetArraySize(g_MapList); i++)
	{	
		GetArrayString(g_MapList, i, map, sizeof(map));

		if(StrContains(map, mapname, false) != -1)
		{
			if (GetConVarBool(g_Cvar_ExcludeCurrent) && StrEqual(map, currentMap))
			{
				isCurrent = true;
				continue;
			}

			if (GetConVarBool(g_Cvar_ExcludeOld) && FindStringInArray(excludeMaps, map) != -1)
			{
				isExclude = true;
				continue;
			}

			if (GetConVarBool(g_Cvar_DisplayName))
			{
				char mapName[PLATFORM_MAX_PATH];
				GetMapName(map, mapName, sizeof(mapName));
				AddMenuItem(SubMapMenu, map, mapName);
			}
			else
			{
				AddMenuItem(SubMapMenu, map, map);
			}
			strcopy(lastMap, sizeof(map), map);
		}
	}

	delete excludeMaps;

	switch (GetMenuItemCount(SubMapMenu)) 
	{
    	case 0:
    	{
			if (isCurrent) 
			{
				CReplyToCommand(client, "[NE] %t", "Can't Nominate Current Map");
			}
			else if (isExclude)
			{
				CReplyToCommand(client, "[NE] %t", "Map in Exclude List");
			}
			else 
			{
				CReplyToCommand(client, "%t", "Map was not found", mapname);
			}

			delete SubMapMenu;
    	}
   		case 1:
   		{
			NominateResult result = NominateMap(lastMap, false, client);
	
			if (result > Nominate_Replaced)
			{
				if (result == Nominate_AlreadyInVote)
				{
					CReplyToCommand(client, "%t", "Map Already In Vote", lastMap);
				}
				else
				{
					CReplyToCommand(client, "[NE] %t", "Map Already Nominated");
				}
			}
			else 
			{
				SetTrieValue(g_mapTrie, lastMap, MAPSTATUS_DISABLED|MAPSTATUS_EXCLUDE_NOMINATED);

				char name[MAX_NAME_LENGTH];
				GetClientName(client, name, sizeof(name));
				PrintToChatAll("[NE] %t", "Map Nominated", name, lastMap);
				LogMessage("%s nominated %s", name, lastMap);
			}	


			delete SubMapMenu;
   		}
   		default: 
   		{
			DisplayMenu(SubMapMenu, client, MENU_TIME_FOREVER);   		
		}
  	}
}

void AttemptNominate(int client)
{
	SetMenuTitle(g_MapMenu, "%T", "Nominate Title", client);
	DisplayMenu(g_MapMenu, client, MENU_TIME_FOREVER);
	
	return;
}

void AttemptNominate2(int client)
{
	SetMenuTitle(g_PopularMapMenu, "%T", "Nominate Title", client);
	DisplayMenu(g_PopularMapMenu, client, MENU_TIME_FOREVER);
	
	return;
}

void OpenTiersMenu(int client)
{
	if (GetConVarBool(g_Cvar_TierMenu))
	{
		DisplayMenu(g_TiersMenu, client, MENU_TIME_FOREVER);
	}

	return;
}

void BuildMapMenu()
{
	delete g_MapMenu;
	
	//ClearTrie(g_mapTrie);
	
	g_MapMenu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);

	char map[PLATFORM_MAX_PATH];
	
	Handle excludeMaps = INVALID_HANDLE;
	char currentMap[PLATFORM_MAX_PATH];
	
	if (GetConVarBool(g_Cvar_ExcludeOld))
	{	
		excludeMaps = CreateArray(ByteCountToCells(PLATFORM_MAX_PATH));
		GetExcludeMapList(excludeMaps);
	}
	
	if (GetConVarBool(g_Cvar_ExcludeCurrent))
	{
		GetCurrentMap(currentMap, sizeof(currentMap));
	}
	
	bool DisplayName = GetConVarBool(g_Cvar_DisplayName);
	for (int i = 0; i < GetArraySize(g_MapList); i++)
	{
		int status = MAPSTATUS_ENABLED;
		
		GetArrayString(g_MapList, i, map, sizeof(map));

		FindMap(map, map, sizeof(map));

		char displayName[PLATFORM_MAX_PATH];
		GetMapDisplayName(map, displayName, sizeof(displayName));

		if (GetConVarBool(g_Cvar_ExcludeCurrent))
		{
			if (StrEqual(map, currentMap))
			{
				status = MAPSTATUS_DISABLED|MAPSTATUS_EXCLUDE_CURRENT;
			}
		}
		
		/* Dont bother with this check if the current map check passed */
		if (GetConVarBool(g_Cvar_ExcludeOld) && status == MAPSTATUS_ENABLED)
		{
			if (FindStringInArray(excludeMaps, map) != -1)
			{
				status = MAPSTATUS_DISABLED|MAPSTATUS_EXCLUDE_PREVIOUS;
			}
		}
		
		if (DisplayName)
		{
			char mapName[PLATFORM_MAX_PATH];
			GetMapName(map, mapName, sizeof(mapName));
			AddMenuItem(g_MapMenu, map, mapName);
		}
		else
		{
			AddMenuItem(g_MapMenu, map, map);
		}
		SetTrieValue(g_mapTrie, map, status);
	}
	
	SetMenuExitButton(g_MapMenu, true);

	if(GetConVarBool(g_Cvar_TierMenu)) 
	{
		SetMenuExitBackButton(g_MapMenu, true);
	}

	delete excludeMaps;
}

void BuildPopularMenu()
{
	delete g_PopularMapMenu;
	
	ClearTrie(g_mapTrie);
	
	g_PopularMapMenu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);

	char map[PLATFORM_MAX_PATH];
	
	Handle excludeMaps = INVALID_HANDLE;
	char currentMap[PLATFORM_MAX_PATH];
	
	if (GetConVarBool(g_Cvar_ExcludeOld))
	{	
		excludeMaps = CreateArray(ByteCountToCells(PLATFORM_MAX_PATH));
		GetExcludeMapList(excludeMaps);
	}
	
	if (GetConVarBool(g_Cvar_ExcludeCurrent))
	{
		GetCurrentMap(currentMap, sizeof(currentMap));
	}
	
	bool DisplayName = GetConVarBool(g_Cvar_DisplayName);
	for (int i = 0; i < GetArraySize(g_PopularMapList); i++)
	{
		int status = MAPSTATUS_ENABLED;
		
		GetArrayString(g_PopularMapList, i, map, sizeof(map));

		FindMap(map, map, sizeof(map));

		char displayName[PLATFORM_MAX_PATH];
		GetMapDisplayName(map, displayName, sizeof(displayName));

		if (GetConVarBool(g_Cvar_ExcludeCurrent))
		{
			if (StrEqual(map, currentMap))
			{
				status = MAPSTATUS_DISABLED|MAPSTATUS_EXCLUDE_CURRENT;
			}
		}
		
		/* Dont bother with this check if the current map check passed */
		if (GetConVarBool(g_Cvar_ExcludeOld) && status == MAPSTATUS_ENABLED)
		{
			if (FindStringInArray(excludeMaps, map) != -1)
			{
				status = MAPSTATUS_DISABLED|MAPSTATUS_EXCLUDE_PREVIOUS;
			}
		}
		
		if (DisplayName)
		{
			char mapName[PLATFORM_MAX_PATH];
			GetMapName(map, mapName, sizeof(mapName));
			AddMenuItem(g_PopularMapMenu, map, mapName);
		}
		else
		{
			AddMenuItem(g_PopularMapMenu, map, map);
		}
		SetTrieValue(g_mapTrie, map, status);
	}
	
	SetMenuExitButton(g_PopularMapMenu, true);

	if(GetConVarBool(g_Cvar_TierMenu)) 
	{
		SetMenuExitBackButton(g_PopularMapMenu, true);
	}

	delete excludeMaps;
}

void BuildTiersMenu()
{
	delete g_TiersMenu;

	g_TiersMenu = new Menu(TiersMenuHandler);
	g_TiersMenu.ExitButton = true;
	
	g_TiersMenu.SetTitle("Nominate Menu");	
	g_TiersMenu.AddItem("Popular Maps", "Popular Maps");
	g_TiersMenu.AddItem("Alphabetical", "Alphabetical");

	for( int i = 1; i <= 2; ++i )
	{
		char tierDisplay[PLATFORM_MAX_PATH + 32];
		Format(tierDisplay, sizeof(tierDisplay), "Tier %i", i);

		char tierString[PLATFORM_MAX_PATH + 32];
		Format(tierString, sizeof(tierString), "%i", i);
		g_TiersMenu.AddItem(tierString, tierDisplay);
	}
}

void BuildTierMenus() 
{
	BuildTiersMenu();

	g_Tier1Menu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	g_Tier1Menu.SetTitle("Nominate Menu\nTier 1 Maps\n ");
	g_Tier1Menu.ExitBackButton = true;
	g_Tier2Menu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	g_Tier2Menu.SetTitle("Nominate Menu\nTier 2 Maps\n ");
	g_Tier2Menu.ExitBackButton = true;
	g_Tier3Menu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	g_Tier3Menu.SetTitle("Nominate Menu\nTier 2 Maps\n ");
	g_Tier3Menu.ExitBackButton = true;
	g_Tier4Menu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	g_Tier4Menu.SetTitle("Nominate Menu\nTier 2 Maps\n ");
	g_Tier4Menu.ExitBackButton = true;
	g_Tier5Menu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	g_Tier5Menu.SetTitle("Nominate Menu\nTier 2 Maps\n ");
	g_Tier5Menu.ExitBackButton = true;
	g_Tier6Menu = new Menu(Handler_MapSelectMenu, MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	g_Tier6Menu.SetTitle("Nominate Menu\nTier 2 Maps\n ");
	g_Tier6Menu.ExitBackButton = true;

	char map[PLATFORM_MAX_PATH];
	
	bool DisplayName = GetConVarBool(g_Cvar_DisplayName);
	for (int i = 0; i < GetArraySize(g_MapList); i++)
	{		
		GetArrayString(g_MapList, i, map, sizeof(map));

		FindMap(map, map, sizeof(map));

		char displayName[PLATFORM_MAX_PATH];
		GetMapDisplayName(map, displayName, sizeof(displayName));

		int tier = GetTier(map);
		
		if (DisplayName)
		{
			char mapName[PLATFORM_MAX_PATH];
			GetMapName(map, mapName, sizeof(mapName));
			AddMapToTierMenu(tier, map, mapName);
		}
		else
		{
			AddMapToTierMenu(tier, map, map);
		}
	}

}

void AddMapToTierMenu(int tier, char[] map, char[] mapName)
{
	if (tier == 1)
	{
		g_Tier1Menu.AddItem(map, mapName);
	}
	if (tier == 2)
	{
		g_Tier2Menu.AddItem(map, mapName);	
	}
	if (tier == 3)
	{
		g_Tier3Menu.AddItem(map, mapName);	
	}
	if (tier == 4)
	{
		g_Tier2Menu.AddItem(map, mapName);	
	}
	if (tier == 5)
	{
		g_Tier5Menu.AddItem(map, mapName);	
	}
	if (tier == 6)
	{
		g_Tier6Menu.AddItem(map, mapName);	
	}
}

public int Handler_MapSelectMenu(Menu menu, MenuAction action, int param1, int param2)
{
	switch (action)
	{
		case MenuAction_Select:
		{
			char map[PLATFORM_MAX_PATH], name[MAX_NAME_LENGTH];
			GetMenuItem(menu, param2, map, sizeof(map));		
			
			char mapName[PLATFORM_MAX_PATH];
			getMapName(map, mapName, sizeof(mapName));
			
			GetClientName(param1, name, MAX_NAME_LENGTH);
	
			NominateResult result = NominateMap(map, false, param1);
			
			/* Don't need to check for InvalidMap because the menu did that already */
			if (result == Nominate_AlreadyInVote)
			{
				PrintToChat(param1, "[IMK] %t", "Map Already Nominated");
				return 0;
			}
			else if (result == Nominate_VoteFull)
			{
				PrintToChat(param1, "[IMK] %t", "Max Nominations");
				return 0;
			}
			
			SetTrieValue(g_mapTrie, map, MAPSTATUS_DISABLED|MAPSTATUS_EXCLUDE_NOMINATED);

			if (result == Nominate_Replaced)
			{
				PrintToChatAll("[IMK] %t", "Map Nomination Changed", name, mapName);
				return 0;	
			}
			
			PrintToChatAll("[IMK] %t", "Map Nominated", name, mapName);
			LogMessage("%s nominated %s", name, map);
		}
		
		case MenuAction_DrawItem:
		{
			char map[PLATFORM_MAX_PATH];
			GetMenuItem(menu, param2, map, sizeof(map));
			
			int status;
			
			if (!GetTrieValue(g_mapTrie, map, status))
			{
				return ITEMDRAW_DEFAULT;
			}
			
			if ((status & MAPSTATUS_DISABLED) == MAPSTATUS_DISABLED)
			{
				return ITEMDRAW_DISABLED;	
			}
			
			return ITEMDRAW_DEFAULT;
						
		}
		
		case MenuAction_DisplayItem:
		{
			char map[PLATFORM_MAX_PATH];
			GetMenuItem(menu, param2, map, sizeof(map));
			
			int mark = GetConVarInt(g_Cvar_MarkCustomMaps);
			bool official;

			int status;
			
			if (!GetTrieValue(g_mapTrie, map, status))
			{
				return 0;
			}
			
			char buffer[100];
			char display[150];
			
			if (mark)
			{
				official = IsMapOfficial(map);
			}
			
			if (mark && !official)
			{
				switch (mark)
				{
					case 1:
					{
						Format(buffer, sizeof(buffer), "%T", "Custom Marked", param1, map);
					}
					
					case 2:
					{
						Format(buffer, sizeof(buffer), "%T", "Custom", param1, map);
					}
				}
			}
			else
			{
				getMapName(map, buffer, sizeof(buffer));
			}
			
			if ((status & MAPSTATUS_DISABLED) == MAPSTATUS_DISABLED)
			{
				if ((status & MAPSTATUS_EXCLUDE_CURRENT) == MAPSTATUS_EXCLUDE_CURRENT)
				{
					Format(display, sizeof(display), "%s (%T)", buffer, "Current Map", param1);
					return RedrawMenuItem(display);
				}
				
				if ((status & MAPSTATUS_EXCLUDE_PREVIOUS) == MAPSTATUS_EXCLUDE_PREVIOUS)
				{
					Format(display, sizeof(display), "%s (%T)", buffer, "Recently Played", param1);
					return RedrawMenuItem(display);
				}
				
				if ((status & MAPSTATUS_EXCLUDE_NOMINATED) == MAPSTATUS_EXCLUDE_NOMINATED)
				{
					Format(display, sizeof(display), "%s (%T)", buffer, "Nominated", param1);
					return RedrawMenuItem(display);
				}
			}
			
			if (mark && !official)
				return RedrawMenuItem(buffer);
			
			return 0;
		}

		case MenuAction_Cancel:
		{
			if (param2 == MenuCancel_ExitBack)
			{
				if (GetConVarBool(g_Cvar_TierMenu))
				{
					OpenTiersMenu(param1);
				}
			}
		}
	}

	if (action == MenuAction_End) 
	{
		if (menu != g_MapMenu && menu != g_Tier1Menu && menu != g_Tier2Menu && menu != g_PopularMapMenu)
		{
			delete menu;
		}
	}
	
	return 0;
}

public int TiersMenuHandler(Menu menu, MenuAction action, int client, int param2) 
{
	if (action == MenuAction_Select) 
	{
		char option[PLATFORM_MAX_PATH];
		menu.GetItem(param2, option, sizeof(option));

		if (StrEqual(option , "Alphabetical")) 
		{
			AttemptNominate(client);
		}
		else if (StrEqual(option , "Popular Maps")) 
		{
			AttemptNominate2(client);
		}
		else 
		{
			int tier = StringToInt(option);
			if (tier == 1 && GetMenuItemCount(g_Tier1Menu) > 0)
			{
				DisplayMenu(g_Tier1Menu, client, MENU_TIME_FOREVER);
			}
			if (tier == 2 && GetMenuItemCount(g_Tier2Menu) > 0)
			{
				DisplayMenu(g_Tier2Menu, client, MENU_TIME_FOREVER);
			}
			if (tier == 3 && GetMenuItemCount(g_Tier3Menu) > 0)
			{
				DisplayMenu(g_Tier3Menu, client, MENU_TIME_FOREVER);
			}
			if (tier == 4 && GetMenuItemCount(g_Tier4Menu) > 0)
			{
				DisplayMenu(g_Tier4Menu, client, MENU_TIME_FOREVER);
			}
			if (tier == 5 && GetMenuItemCount(g_Tier5Menu) > 0)
			{
				DisplayMenu(g_Tier5Menu, client, MENU_TIME_FOREVER);
			}
			if (tier == 6 && GetMenuItemCount(g_Tier6Menu) > 0)
			{
				DisplayMenu(g_Tier6Menu, client, MENU_TIME_FOREVER);
			}
		}
	}
}

stock bool IsNominateAllowed(int client)
{
	CanNominateResult result = CanNominate();
	
	switch(result)
	{
		case CanNominate_No_VoteInProgress:
		{
			CReplyToCommand(client, "[IMK] %t", "Nextmap Voting Started");
			return false;
		}
		
		case CanNominate_No_VoteComplete:
		{
			char map[PLATFORM_MAX_PATH];
			GetNextMap(map, sizeof(map));
			CReplyToCommand(client, "[IMK] %t", "Next Map", map);
			return false;
		}
		
		case CanNominate_No_VoteFull:
		{
			CReplyToCommand(client, "[IMK] %t", "Max Nominations");
			return false;
		}
	}
	
	return true;
}

int GetTier(char[] mapname)
{
	int tier = 0;
	if (g_bShavit) 
	{
		char mapdisplay[PLATFORM_MAX_PATH + 32];
		GetMapDisplayName(mapname, mapdisplay, sizeof(mapdisplay));
		tier = Shavit_GetMapTier(mapdisplay);
	}

	if (tier < 1) 
	{
		tier = 1;
	}

	return tier;
}