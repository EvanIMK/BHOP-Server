#include <sourcemod>
#include <multicolors>
#include <shavit>
#include <discord>

#pragma semicolon 1
#pragma newdecls required

#define VOTE_NO "###no###"
#define VOTE_YES "###yes###"

ConVar g_hCalladminDiscord = null;
bool g_bClientOwnReason[MAXPLAYERS + 1];
char g_sServerName[256];
char g_szSteamID[MAXPLAYERS + 1][32];
ConVar g_hHostName = null;
int g_iWaitingForResponse[MAXPLAYERS + 1];
char g_szMapName[128];
Handle mapTime;
char votetype[32];

public Plugin myinfo = 
{
    name = "IMK Utilities", 
    author = "Evan", 
    description = "Various utilities for IMK Servers", 
    version = "1.1.2"
};

public void OnPluginStart()
{
    g_hCalladminDiscord = CreateConVar("calladmin_discord", "", "Web hook link to allow players to call admin to discord, keep empty to disable");
    
    g_hHostName = FindConVar("hostname");
    GetConVarString(g_hHostName, g_sServerName, sizeof(g_sServerName));
    
    AddCommandListener(Say_Hook, "say");
    AddCommandListener(Say_Hook, "say_team");
    
    RegConsoleCmd("sm_colours", Command_Colours, "Shows colours");
    RegConsoleCmd("sm_bug", Command_Calladmin, "Report a bug to our discord");
    RegConsoleCmd("sm_calladmin", Command_Calladmin, "Sends a message to the staff");
    RegConsoleCmd("sm_report", Command_Calladmin, "Sends a message to the staff");
    RegConsoleCmd("sm_rm", Command_ReloadMap, "Reloads map");
	RegAdminCmd("sm_ve", Command_VoteExtend, ADMFLAG_CUSTOM1, "Vote to extend the map");
	RegAdminCmd("sm_voteextend", Command_VoteExtend, ADMFLAG_CUSTOM1, "Vote to extend the map");
	RegAdminCmd("sm_extend", Command_VoteExtend, ADMFLAG_CUSTOM1, "Vote to extend the map");
    
    AutoExecConfig(true, "imk");
	LoadTranslations("imk.phrases");
    
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i))
            OnClientPutInServer(i);
    }
}

public void OnClientPutInServer(int client)
{
    if (!IsValidClient(client))
    return;
    
    g_bClientOwnReason[client] = false;
    g_iWaitingForResponse[client] = -1;
    
    GetClientAuthId(client, AuthId_Steam2, g_szSteamID[client], MAX_NAME_LENGTH, true);
}

public void OnMapStart()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        g_bMessagesShown[i] = false;
    }
    
    GetCurrentMap(g_szMapName, 128);
}

public Action Command_ReloadMap(int client, int args)
{ 
    ServerCommand("changelevel %s", g_szMapName);
    return Plugin_Handled;
}

public Action Command_Colours(int client, int args)
{
    CPrintToChat(client, "{rand}, {blue}blue, {bluegrey}bluegrey, {darkblue}darkblue, {darkred}darkred, {orange}gold, {grey}grey, {grey2}grey2, {lightgreen}lightgreen, {lightred}lightred, {lime}lime, {orchid}orchid, {yellow}yellow, {green}green, {lightred}lightred", client);
}

public void Shavit_OnRankAssigned(int client, int rank, float points, bool first)
{
    if(first)
    {
        int eRanks = Shavit_GetRankedPlayers();
        
        if(rank == 0)
        {
            CPrintToChatAll("{default}[ {green}+ {default}] {bluegrey}%N{grey} {default}connected {grey}[{default}Unranked{grey}]", client);       
        }
    
        else
        {
            CPrintToChatAll("{default}[ {green}+ {default}] {bluegrey}%N {default}connected{grey} [{default}%d{grey}/{default}%d{grey}]", client, rank, eRanks);
        }   
    }
}

public Action Say_Hook(int client, const char[] command, int argc)
{
    if (g_bClientOwnReason[client])
    {
        g_bClientOwnReason[client] = false;
        return Plugin_Continue;
    }

    if (IsValidClient(client))
    {
        if (g_iWaitingForResponse[client] > -1)
        {
            char sText[1024];
            GetCmdArgString(sText, sizeof(sText));

            StripQuotes(sText);
            TrimString(sText);
            
            if (StrEqual(sText, "cancel"))
            {
                CPrintToChat(client, "Cancelled");
                g_iWaitingForResponse[client] = -1;
                return Plugin_Handled;
            }

            switch (g_iWaitingForResponse[client])
            {
                case 0:
                {
                    CallAdmin(client, sText);
                }
            }
            
            g_iWaitingForResponse[client] = -1;
            return Plugin_Handled;
        }
    }
    return Plugin_Continue;
}

public Action Command_Calladmin(int client, int args)
{
    g_iWaitingForResponse[client] = 0;
    CPrintToChat(client, "Type your message");
    return Plugin_Handled;
}

public void CallAdmin(int client, char[] sText)
{   
    char webhook[1024];
    GetConVarString(g_hCalladminDiscord, webhook, 1024);
    if (StrEqual(webhook, ""))
        return;
		
    DiscordWebHook hook = new DiscordWebHook(webhook);
    hook.SlackMode = true;
	
	hook.SetUsername("Calladmin");
	hook.SetAvatar("https://i.imgur.com/BisyYvV.png");
	
	MessageEmbed Embed = new MessageEmbed();
	
	char sTitle[256];
	Format(sTitle, sizeof(sTitle), "Server: %s || Map: %s", g_sServerName, g_szMapName);
	Embed.SetTitle(sTitle);

	char sName[MAX_NAME_LENGTH];
	GetClientName(client, sName, sizeof(sName));

    char sMessage[512];
    Format(sMessage, sizeof(sMessage), "%s (%s): %s", sName, g_szSteamID[client], sText);
    Embed.AddField("", sMessage, true);

    hook.Embed(Embed);
    hook.Send();
    delete hook;

    CPrintToChat(client, "Message successfully sent to admins");
}


public Action Command_VoteExtend(int client, int args)
{
	VoteExtend(client);
	return Plugin_Handled;
}

public void VoteExtend(int client)
{
	int timeleft;
	GetMapTimeLeft(timeleft);

	if (timeleft > 300)
	{
		Shavit_PrintToChat(client, "%t", "Commands4", client);
		return;
	}

	if (IsVoteInProgress())
	{
		Shavit_PrintToChat(client, "%T", "Commands5", client);
		return;
	}

	char szPlayerName[MAX_NAME_LENGTH];
	GetClientName(client, szPlayerName, MAX_NAME_LENGTH);

	Menu menu = CreateMenu(Handle_VoteMenuExtend);
	SetMenuTitle(menu, "Extend the map by 15 minutes?");
	AddMenuItem(menu, "###yes###", "Yes");
	AddMenuItem(menu, "###no###", "No");
	SetMenuExitButton(menu, false);
	VoteMenuToAll(menu, 20);
	Shavit_PrintToChatAll("%t", "VoteStartedBy", szPlayerName);

	return;
}

public int Handle_VoteMenuExtend(Menu menu, MenuAction action, int param1, int param2)
{
	if (action == MenuAction_End)
	{
		CloseHandle(menu);
	}
	else if (action == MenuAction_VoteEnd)
	{
		char item[64], display[64];
		float percent, limit;
		int votes, totalVotes;

		menu.GetItem(param1, item, sizeof(item), _, display, sizeof(display));
		GetMenuVoteInfo(param2, votes, totalVotes);

		if (strcmp(item, VOTE_NO) == 0 && param1 == 1)
		votes = totalVotes - votes;

		percent = (float(votes) / float(totalVotes));

		GetCurrentMaptime();
		int iTimeLimit = GetConVarInt(mapTime);

		if (iTimeLimit >= 90)
			limit = 0.75;
		else
			limit = 0.50;

		if ((strcmp(item, VOTE_YES) == 0 && FloatCompare(percent,limit) < 0 && param1 == 0) || (strcmp(item, VOTE_NO) == 0 && param1 == 1))
		{
			Shavit_PrintToChatAll("%t", "CVote8", RoundToNearest(100.0*limit), RoundToNearest(100.0*percent), totalVotes);
		}
		else
		{
			Shavit_PrintToChatAll("%t", "CVote9", RoundToNearest(100.0*percent), totalVotes);
			Shavit_PrintToChatAll("%t", "CVote10");
			extendMap(900);
		}
	}
}

public void extendMap(int seconds)
{
	ExtendMapTimeLimit(seconds);
	GetCurrentMaptime();
}

public void GetCurrentMaptime()
{
	mapTime = FindConVar("mp_timelimit");
}

public Action start_vote(int client, int args)
{
	if (!IsValidClient(client))
	return Plugin_Handled;

	if (IsVoteInProgress())
	{
		Shavit_PrintToChat(client, "%t", "VoteInProgress", client);
		return Plugin_Handled;
	}
	else if (args < 1)
	{
		Shavit_PrintToChat(client, "%t", "CVote2", client);
	}

	GetCmdArg(1, votetype, sizeof(votetype));

	char szPlayerName[MAX_NAME_LENGTH];
	GetClientName(client, szPlayerName, MAX_NAME_LENGTH);

	if (strcmp(votetype, "extend", false) == 0)
	{
		Menu menu = CreateMenu(Handle_VoteMenuExtend);
		SetMenuTitle(menu, "Extend the map by 15 minutes?");
		AddMenuItem(menu, "yes", "Yes");
		AddMenuItem(menu, "no", "No");
		SetMenuExitButton(menu, false);
		VoteMenuToAll(menu, 20);
		Shavit_PrintToChatAll("%t", "VoteStartedBy", szPlayerName);
	}
    
	return Plugin_Handled;
}