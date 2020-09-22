#pragma semicolon 1

#include <sourcemod>
#include <sdktools>
#include <multicolors>
#include <store>

ConVar gB_ShowGamble;
ConVar gB_MinimumBet;
ConVar gB_MaximumBet;
ConVar gB_WinPrecent;
bool gB_Gamble[MAXPLAYERS + 1] = true;

public Plugin myinfo = 
{
	name = "[Zeph Store] Gamble Module",
	author = "nhnkl159, Evan",
	description = "Simple gamble module for zeph store.",
	version = "1.1"
};

public void OnPluginStart()
{
	RegConsoleCmd("sm_gamble", Cmd_Gamble, "Command for player to gamble his credits.");
	RegConsoleCmd("sm_coinflip", Cmd_Gamble, "Command for player to gamble his credits.");
	RegConsoleCmd("sm_flip", Cmd_Gamble, "Command for player to gamble his credits.");
	
	gB_ShowGamble = CreateConVar("sm_gamble_showgamble", "1", "Sets whether or not to show everyone gambles");
	gB_MinimumBet = CreateConVar("sm_gamble_minbet", "25", "Sets the minimum amount of credits to gamble");
	gB_MaximumBet = CreateConVar("sm_gamble_maxbet", "7000", "Sets the maximum amount of credits to gamble");
	gB_WinPrecent = CreateConVar("sm_gamble_winprecent", "90", "Sets the precent of winning in the gamble system");
	
	AutoExecConfig(true, "sm_storegamble");
}

public void OnMapStart()
{
	for (int i = 1; i <= MaxClients; i++)
	{
		gB_Gamble[i] = true;
	}	
}

public Action Cmd_Gamble(int client, int args)
{
	if(!IsValidClient(client))
	{
		return Plugin_Handled;
	}
	
	char arg1[32];
	GetCmdArg(1, arg1, 32);
	int gB_OurNumber = StringToInt(arg1);
	int gB_PlayerCredits = Store_GetClientCredits(client);
	
	if(args < 1)
	{
		CPrintToChat(client, "[{lightred}Gamble{default}] Usage : sm_gamble <credits>");
		return Plugin_Handled; 
	}
	
	if(gB_OurNumber > gB_PlayerCredits)
	{
		CPrintToChat(client, "[{lightred}Gamble{default}] You don't have this amount of credits.");
		return Plugin_Handled;
	}
    
	if(gB_Gamble[client] == true)
	{
		if(StrEqual(arg1, "all"))
		{
			int gB_RandomNum = GetRandomInt(1, 100);
			
			if(gB_RandomNum <= gB_WinPrecent.IntValue)
			{
				Store_SetClientCredits(client, gB_PlayerCredits + gB_PlayerCredits);
				Gamble_PrintToChat(client, gB_PlayerCredits, true);
				gB_Gamble[client] = false;
				CreateTimer(20.0, Timer_Gamble, GetClientUserId(client));
			}
			else
			{
				Store_SetClientCredits(client, gB_PlayerCredits - gB_PlayerCredits);
				Gamble_PrintToChat(client, gB_PlayerCredits, false);
				gB_Gamble[client] = false;
				CreateTimer(20.0, Timer_Gamble, GetClientUserId(client));
			}
			
			return Plugin_Handled;
		}
		
		int gB_RandomNum = GetRandomInt(1, 100); 
		if(!isNumeric(arg1)) 
		{
			CPrintToChat(client, "[{lightred}Gamble{default}] Please enter only numbers if you wish to gamble.");
			return Plugin_Handled;
		}
		
		if(!IsNumberValid(gB_OurNumber)) 
		{
			CPrintToChat(client, "[{lightred}Gamble{default}] Please enter a number between {red}%d {default}- {red}%d {default}if you wish to gamble.", gB_MinimumBet.IntValue, gB_MaximumBet.IntValue);
			return Plugin_Handled;
		}
			
		if(gB_RandomNum <= gB_WinPrecent.IntValue)
		{
			Store_SetClientCredits(client, gB_PlayerCredits + gB_OurNumber);
			Gamble_PrintToChat(client, gB_OurNumber, true);
			gB_Gamble[client] = false;
			CreateTimer(20.0, Timer_Gamble, GetClientUserId(client));
		}
		else
		{
			Store_SetClientCredits(client, gB_PlayerCredits - gB_OurNumber);
			Gamble_PrintToChat(client, gB_OurNumber, false);
			gB_Gamble[client] = false;
			CreateTimer(20.0, Timer_Gamble, GetClientUserId(client));
		}
	}
	
	else
	{
		CPrintToChat(client, "[{lightred}Gamble{default}] You can only gamble every 20 seconds");
	}
	
	return Plugin_Handled;
}

stock void Gamble_PrintToChat(int client, int gB_Number, bool winner)
{
	if(gB_ShowGamble.BoolValue)
	{
		CPrintToChatAll("[{lightred}Gamble{default}] {lime}%N {bluegrey}just gambled {yellow}%d{bluegrey} credits and %s", client, gB_Number, winner ? "{green}WON" : "{lightred}LOST");
	}
	else
	{
		PrintToChat(client, "%s You just gambled %d amount of credits and %s", gB_Number, winner ? "won" : "lost");
	}
}

stock bool IsNumberValid(int number)
{
	if(number < gB_MinimumBet.IntValue)
	{
		return false;
	}
	else if (number > gB_MaximumBet.IntValue)
	{
	   return false;
	}
	else if (number == 0)
	{
	   return false;
	}
	return true;
}

stock bool IsValidClient(int client, bool alive = false, bool bots = false)
{
	if (client > 0 && client <= MaxClients && IsClientInGame(client) && (alive == false || IsPlayerAlive(client)) && (bots == false && !IsFakeClient(client)))
	{
		return true;
	}
	return false;
}

stock bool isNumeric(char[] arg)
{
	int argl = strlen(arg);
	for (int i = 0; i < argl; i++)
	{
		if (!IsCharNumeric(arg[i]))
		{
			return false;
		}
	}
	return true;
}

public Action Timer_Gamble(Handle timer, any data)
{
	int client = GetClientOfUserId(data);
	gB_Gamble[client] = true;
}