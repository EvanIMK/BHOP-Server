#if defined _levelsranks_included_
  #endinput
#endif
#define _levelsranks_included_

#define PLUGIN_VERSION "v2.3.6"

#define ST_VALUE 0
#define ST_RANK 1
#define ST_KILLS 2
#define ST_DEATHS 3
#define ST_SHOOTS 4
#define ST_HITS 5
#define ST_HEADSHOTS 6
#define ST_ASSISTS 7

int		g_iColorsOther[] = {0xFFFFFF, 0xFF0000, 0x00AD00, 0x00FF00, 0x99FF99, 0xFF4040, 0xCCCCCC, 0xFFBD6B, 0xFA8B00, 0x99CCFF, 0x3D46FF, 0xFA00FA};
char		g_sColors[][] = {"{WHITE}", "{RED}", "{GREEN}", "{LIME}", "{LIGHTGREEN}", "{LIGHTRED}", "{GRAY}", "{LIGHTOLIVE}", "{OLIVE}", "{LIGHTBLUE}", "{BLUE}", "{PURPLE}"};
char		g_sColorsCSGO[][] = {"\x01", "\x02", "\x04", "\x05", "\x06", "\x07", "\x08", "\x09", "\x10", "\x0B", "\x0C", "\x0E"};

/**
 * Checks if enough players are in this round to activate statistics
 * 
 * @return bool	-	if true - yes, otherwise - no
 */
native bool LR_CheckCountPlayers();

/**
 * Returns ID of statistics type
 * 
 * @return int		-	ID of stastics type
 */
native int LR_GetTypeStatistics();

/**
 * Gets the client's place in the TOP
 * 
 * @param iClient	-	Client index
 * @return int		-	Client position in TOP
 */
native int LR_GetClientPos(int iClient);

/**
 * Gets detailed statistics about the client
 * 
 * @param iClient	-	Client index
 * @param iStats	-	Number of selected data (Example: iStats = ST_VALUE).
 * @return int		-	Obtained data
 */
native int LR_GetClientInfo(int iClient, int iStats);

/**
 * Changes the amount of experience points (only for lr_type_stastics 0)
 * 
 * @param iClient	-	Client index
 * @param iAmount	Count Exp
 * @return int		-	New count client exp.
 */
native int LR_ChangeClientValue(int iClient, int iAmount);

/**
 * Assigning your values (only for lr_type_statistics 2 - synhc method)
 * 
 * @param iClient	-	Client index
 * @param iAmount	Your value
 * @return bool	-	If true, points are given, otherwise false.
 */
native bool LR_SetClientValue(int iClient, int iAmount);

/**
 * Exp Multiplier (only for lr_type_statistics 0)
 * 
 * @param iClient	-	Client index
 * @param fCoeff1	-	Multiplier Exp for cases when experience is increased
 * @param fCoeff2	-	Multiplier Exp for cases when experience is decreases
 * @return bool	-	If true, then the multiplier is set, otherwise false.
 */
native bool LR_SetMultiplierValue(int iClient, float fCoeff1, float fCoeff2);

/**
 * Calls up the inventory menu (necessary to go back)
 * 
 * @param iClient	-	Client index
 * @noreturn
 */
native void LR_MenuInventory(int iClient);

/**
 * Checks the VIP status of the client
 * 
 * @param iClient	-	Client index
 * @return bool	-	If true - client is VIP, otherwise - false.
 */
native bool LR_IsClientVIP(int iClient);

/**
 * Gives VIP status to players
 * 
 * @param iClient	-	Client index
 * @param iRank	-	The rank that will be assigned to the player forcibly
 * @return bool	-	If true, the rights are given out, otherwise - false
 */
native bool LR_SetClientVIP(int iClient, int iRank);

/**
 * Called when a list opens Inventory
 */
forward void LR_OnMenuCreated(int iClient, int iRank, Menu& hMenu);

/**
 * Called when a list opens Inventory
 */
#pragma deprecated
forward void LR_OnMenuItemSelected(int iClient, int iRank, const char[] sInfo);

/**
 * Called when the customer's rank changes
 *
 * @param iClient	-	Client index
 * @param iNewLevel	New level
 * @param bUp	-	If true, the level has risen, otherwise it has gone down.
 */
forward void LR_OnLevelChanged(int iClient, int iNewLevel, bool bUp);

/**
 * Called when the plug-in is started (check for the presence of synchronization modules, if lr_type_statistics 2)
 *
 * @param iCount	-	Number of synchronization modules (if more than 1, the plug-in is disabled)
 */
forward void LR_OnCheckSync(int &iCount);

stock void LR_PrintToChat(int iClient, char[] szMessage, any ...)
{
	if(iClient && IsClientInGame(iClient) && !IsFakeClient(iClient))
	{
		char szBuffer[PLATFORM_MAX_PATH], szNewMessage[PLATFORM_MAX_PATH];

		switch(GetEngineVersion())
		{
			case Engine_CSGO:
			{
				Format(szBuffer, sizeof(szBuffer), " \x01%s", szMessage);
				VFormat(szNewMessage, sizeof(szNewMessage), szBuffer, 3);

				for(int i = 0; i < 12; i++)
				{
					ReplaceString(szNewMessage, sizeof(szNewMessage), g_sColors[i], g_sColorsCSGO[i]);
				}
				ReplaceString(szNewMessage, sizeof(szNewMessage), "{TEAM}", "\x03");
			}

			case Engine_CSS:
			{
				char sBuff[64];
				Format(szBuffer, sizeof(szBuffer), "\x01%s", szMessage);
				VFormat(szNewMessage, sizeof(szNewMessage), szBuffer, 3);

				switch(GetClientTeam(iClient))
				{
					case 1: Format(sBuff, sizeof(sBuff), "\x07%06X", g_iColorsOther[6]);
					case 2: Format(sBuff, sizeof(sBuff), "\x07%06X", g_iColorsOther[5]);
					case 3: Format(sBuff, sizeof(sBuff), "\x07%06X", g_iColorsOther[9]);
				}
				ReplaceString(szNewMessage, sizeof(szNewMessage), "{TEAM}", sBuff);

				for(int i = 0; i < 12; i++)
				{
					Format(sBuff, sizeof(sBuff), "\x07%06X", g_iColorsOther[i]);
					ReplaceString(szNewMessage, sizeof(szNewMessage), g_sColors[i], sBuff);
				}
			}
		}

		Handle hBf = StartMessageOne("SayText2", iClient, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);
		if(hBf != null)
		{
			if(GetUserMessageType() == UM_Protobuf)
			{
				Protobuf hProtoBuffer = UserMessageToProtobuf(hBf);
				hProtoBuffer.SetInt("ent_idx", iClient);
				hProtoBuffer.SetBool("chat", true);
				hProtoBuffer.SetString("msg_name", szNewMessage);
				hProtoBuffer.AddString("params", "");
				hProtoBuffer.AddString("params", "");
				hProtoBuffer.AddString("params", "");
				hProtoBuffer.AddString("params", "");
			}
			else
			{
				BfWrite hBfBuffer = UserMessageToBfWrite(hBf);
				hBfBuffer.WriteByte(iClient);
				hBfBuffer.WriteByte(true);
				hBfBuffer.WriteString(szNewMessage);
			}
		}
		EndMessage();
	}
}

public SharedPlugin __pl_levelsranks = 
{
	name = "levelsranks",
	file = "levelsranks.smx",

	#if defined REQUIRE_PLUGIN
		required = 1,
	#else
		required = 0,
	#endif
};

#if !defined REQUIRE_PLUGIN
public __pl_levelsranks_SetNTVOptional()
{
	MarkNativeAsOptional("LR_CheckCountPlayers");
	MarkNativeAsOptional("LR_GetTypeStatistics");
	MarkNativeAsOptional("LR_GetClientPos");
	MarkNativeAsOptional("LR_GetClientInfo");
	MarkNativeAsOptional("LR_ChangeClientValue");
	MarkNativeAsOptional("LR_IsClientVIP");
	MarkNativeAsOptional("LR_SetClientValue");
	MarkNativeAsOptional("LR_SetMultiplierValue");
	MarkNativeAsOptional("LR_MenuInventory");
	MarkNativeAsOptional("LR_SetClientVIP");
}
#endif