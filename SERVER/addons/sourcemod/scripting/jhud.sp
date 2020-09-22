#include <sdktools>
#include <sdkhooks>
#include <clientprefs>
#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo =
{
	name = "jhud",
	author = "Blank",
	description = "Center of screen SSJ",
	version = "0.9.3",
	url = ""
};

#define BHOP_TIME 10

Handle g_hCookieJHUD;
Handle g_hCookieStrafeSpeed;
Handle g_hCookie60;
Handle g_hCookie6070;
Handle g_hCookie7080;
Handle g_hCookie80;
Handle g_hCookieDefaultsSet;

bool g_bJHUD[MAXPLAYERS+1];
bool g_bStrafeSpeed[MAXPLAYERS+1];
int g_i60[MAXPLAYERS+1];
int g_i6070[MAXPLAYERS+1];
int g_i7080[MAXPLAYERS+1];
int g_i80[MAXPLAYERS+1];

char lastChoice[MAXPLAYERS+1];

bool g_bTouchesWall[MAXPLAYERS+1];

int g_iTicksOnGround[MAXPLAYERS+1];
int g_iTouchTicks[MAXPLAYERS+1];
int g_strafeTick[MAXPLAYERS+1];
int g_iJump[MAXPLAYERS+1];

float g_flRawGain[MAXPLAYERS+1];

#define M_PI 3.14159265358979323846264338327950288

float g_vecLastAngle[MAXPLAYERS + 1][3];
float g_fTotalNormalDelta[MAXPLAYERS + 1];
float g_fTotalPerfectDelta[MAXPLAYERS + 1];

enum
{
	White,
	Red,
	Cyan,
	Purple,
	Green,
	Blue,
	Yellow,
	Orange,
	Gray
};

int colors[][3] = { 
	{255, 255, 255}, 	// White
	{255, 0, 0}, 		// Red
	{0, 255, 255}, 	// Cyan
	{128, 0, 128}, 	// Purple
	{0, 255, 0}, 		// Green
	{0, 0, 255}, 		// Blue
	{255, 255, 0}, 	// Yellow
	{255, 165, 0},	// Orange
	{128, 128, 128}	// Gray
};

int values[][3] = { 
	{}, 			// 0 / placeholder
	{280, 282, 287},	// 1st
	{366, 370, 375},	// 2nd
	{438, 442, 450},	// 3rd
	{500, 505, 515},	// 4th
	{555, 560, 570},	// 5th
	{605, 610, 620},	// 6th
	{},	{},	{},	{},	{},	{},	{}, {},	{},	// placeholders probably couldve come up with something better w/e
	{965, 980, 1000}	// 16th
};

public void OnAllPluginsLoaded()
{
	HookEvent("player_jump", OnPlayerJump);
}

public void OnPluginStart()
{
	RegConsoleCmd("sm_jhud", Command_JHUD, "JHUD");
   
	g_hCookieJHUD = RegClientCookie("jhud_enabled", "jhud_enabled", CookieAccess_Protected);
	g_hCookieStrafeSpeed = RegClientCookie("jhud_strafespeed", "jhud_strafespeed", CookieAccess_Protected);
	g_hCookie60 = RegClientCookie("jhud_60", "jhud_60", CookieAccess_Protected);
	g_hCookie6070 = RegClientCookie("jhud_6070", "jhud_6070", CookieAccess_Protected);
	g_hCookie7080 = RegClientCookie("jhud_7080", "jhud_7080", CookieAccess_Protected);
	g_hCookie80 = RegClientCookie("jhud_80", "jhud_80", CookieAccess_Protected);
	g_hCookieDefaultsSet = RegClientCookie("jhud_defaults", "jhud_defaults", CookieAccess_Protected);
   
	for(int i = 1; i <= MaxClients; i++)
	{
		if(AreClientCookiesCached(i))
		{
			OnClientPostAdminCheck(i);
			OnClientCookiesCached(i);
		}
	}
}

public void OnClientCookiesCached(int client)
{
	char strCookie[8];
	
	GetClientCookie(client, g_hCookieDefaultsSet, strCookie, sizeof(strCookie));
	
	if(StringToInt(strCookie) == 0)
	{
		SetCookie(client, g_hCookieJHUD, false);
		SetCookie(client, g_hCookieStrafeSpeed, false);
		SetCookie(client, g_hCookie60, Red);
		SetCookie(client, g_hCookie6070, Orange);
		SetCookie(client, g_hCookie7080, Green);
		SetCookie(client, g_hCookie80, Cyan);
		SetCookie(client, g_hCookieDefaultsSet, true);
	}
	
	GetClientCookie(client, g_hCookieJHUD, strCookie, sizeof(strCookie));
	g_bJHUD[client] = view_as<bool>(StringToInt(strCookie));

	GetClientCookie(client, g_hCookieStrafeSpeed, strCookie, sizeof(strCookie));
	g_bStrafeSpeed[client] = view_as<bool>(StringToInt(strCookie));
	
	GetClientCookie(client, g_hCookie60, strCookie, sizeof(strCookie));
	g_i60[client] = StringToInt(strCookie);

	GetClientCookie(client, g_hCookie6070, strCookie, sizeof(strCookie));
	g_i6070[client] = StringToInt(strCookie);

	GetClientCookie(client, g_hCookie7080, strCookie, sizeof(strCookie));
	g_i7080[client] = StringToInt(strCookie);

	GetClientCookie(client, g_hCookie80, strCookie, sizeof(strCookie));
	g_i80[client] = StringToInt(strCookie);
}

public void OnClientPostAdminCheck(int client)
{
	g_iJump[client] = 0;
	g_strafeTick[client] = 0;
	g_flRawGain[client] = 0.0;
	g_iTicksOnGround[client] = 0;
	SDKHook(client, SDKHook_Touch, onTouch);
}

public Action onTouch(int client, int entity)
{
	if(!(GetEntProp(entity, Prop_Data, "m_usSolidFlags") & 12))
	{
		g_bTouchesWall[client] = true;
	}
}

public Action OnPlayerJump(Event event, char[] name, bool dontBroadcast)
{
	int userid = GetEventInt(event, "userid");
	int client = GetClientOfUserId(userid);

	if(IsFakeClient(client))
	{
		return;   
	}

	if(g_iJump[client] && g_strafeTick[client] <= 0)
	{
		return;
	}

	g_iJump[client]++;

	for(int i=1; i<MaxClients;i++)
	{
		if(IsClientInGame(i) && ((!IsPlayerAlive(i) && GetEntPropEnt(i, Prop_Data, "m_hObserverTarget") == client && GetEntProp(i, Prop_Data, "m_iObserverMode") != 7 && g_bJHUD[i]) || ((i == client && g_bJHUD[i]))))
		{
			JHUD_DrawStats(i, client);
		}
	}

	g_flRawGain[client] = 0.0;
	g_strafeTick[client] = 0;
	g_fTotalNormalDelta[client] = 0.0;
	g_fTotalPerfectDelta[client] = 0.0;
}

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon, int &subtype, int &cmdnum, int &tickcount, int &seed, int mouse[2])
{
	if(IsFakeClient(client))
	{
		return Plugin_Continue;
	}

	float yaw = NormalizeAngle(angles[1] - g_vecLastAngle[client][1]);
		
	float g_vecAbsVelocity[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", g_vecAbsVelocity);
	float velocity = GetVectorLength(g_vecAbsVelocity);
		
	float wish_angle = FloatAbs(ArcSine(30.0 / velocity)) * 180 / M_PI;
	
	if(GetEntityFlags(client) & FL_ONGROUND)
	{
		if(g_iTicksOnGround[client] > BHOP_TIME)
		{
			g_iJump[client] = 0;
			g_strafeTick[client] = 0;
			g_flRawGain[client] = 0.0;
			g_fTotalNormalDelta[client] = 0.0;
			g_fTotalPerfectDelta[client] = 0.0;
		}

		g_iTicksOnGround[client]++;

		if(buttons & IN_JUMP && g_iTicksOnGround[client] == 1)
		{
			JHUD_GetStats(client, vel, angles);
			g_iTicksOnGround[client] = 0;
		}
	}
	else
	{
		g_fTotalNormalDelta[client] += FloatAbs(yaw);
		g_fTotalPerfectDelta[client] += wish_angle;

		if(GetEntityMoveType(client) != MOVETYPE_NONE && GetEntityMoveType(client) != MOVETYPE_NOCLIP && GetEntityMoveType(client) != MOVETYPE_LADDER && GetEntProp(client, Prop_Data, "m_nWaterLevel") < 2)
		{
			JHUD_GetStats(client, vel, angles);
		}
		g_iTicksOnGround[client] = 0;
	}

	if(g_bTouchesWall[client])
	{
		g_iTouchTicks[client]++;
		g_bTouchesWall[client] = false;
	}
	else 
	{
		g_iTouchTicks[client] = 0;
	}

	g_vecLastAngle[client] = angles;

	return Plugin_Continue;
}

stock float NormalizeAngle(float ang)
{
	if (ang > 180.0)
	{
		ang -= 360.0;
	}
	else if (ang < -180.0)
	{
		ang += 360.0;
	}
	
	return ang;
}

stock bool IsNaN(float x)
{
	return x != x;
}

public Action Command_JHUD(int client, any args)
{
	if(client != 0)
	{
		ShowJHUDMenu(client);
	}
	return Plugin_Handled;
}

void ShowJHUDMenu(int client, int position = 0)
{
	Menu menu = CreateMenu(JHUD_Select);
	SetMenuTitle(menu, "JHUD Menu\n \n");

	if(g_bJHUD[client])
	{
		AddMenuItem(menu, "usage", "JHUD: [ON]");
	}
	else 
	{
		AddMenuItem(menu, "usage", "JHUD: [OFF]");
	}
	
	if(g_bStrafeSpeed[client])
	{
		AddMenuItem(menu, "strafespeed", "JSS: [ON]\n \n");
	}
	else 
	{
		AddMenuItem(menu, "strafespeed", "JSS: [OFF]\n \n");
	}

	AddMenuItem(menu, "< 60 Gain", "< 60 Gain");
	AddMenuItem(menu, "60-70 Gain", "60-70 Gain");
	AddMenuItem(menu, "70-80 Gain", "70-80 Gain");
	AddMenuItem(menu, "> 80 Gain", "> 80 Gain");
	
	AddMenuItem(menu, "reset", "Reset to default values");
	
	menu.DisplayAt(client, position, MENU_TIME_FOREVER);
}

public int JHUD_Select(Menu menu, MenuAction action, int client, int option)
{
	if(action == MenuAction_Select)
	{
		char info[32];
		GetMenuItem(menu, option, info, sizeof(info));
		lastChoice = info;

		if(StrEqual(info, "usage"))
		{
			g_bJHUD[client] = !g_bJHUD[client];
			SetCookie(client, g_hCookieJHUD, g_bJHUD[client]);
			ShowJHUDMenu(client, GetMenuSelectionPosition());
		}
		else if(StrEqual(info, "strafespeed"))
		{
			g_bStrafeSpeed[client] = !g_bStrafeSpeed[client];
			SetCookie(client, g_hCookieStrafeSpeed, g_bStrafeSpeed[client]);
			ShowJHUDMenu(client, GetMenuSelectionPosition());
		}
		else if(StrEqual(info, "reset"))
		{
			JHUD_ResetValues(client);
			ShowJHUDMenu(client, GetMenuSelectionPosition());
		}
		else
		{
			ShowJHUDSettingsMenu(client, GetMenuSelectionPosition());
		}

	}
	else if(action == MenuAction_End)
	{
		delete menu;
	}
}

void ShowJHUDSettingsMenu(int client, int positon = 0)
{
	Menu menu = CreateMenu(JHUDSettingsMenu_Handler);
	SetMenuTitle(menu, "JHUD Menu\nChoice: %s\n \n", lastChoice);

	int selectedColor;

	if(StrEqual(lastChoice, "< 60 Gain"))
	{
		selectedColor = g_i60[client];
	}
	else if(StrEqual(lastChoice, "60-70 Gain"))
	{
		selectedColor = g_i6070[client];
	}
	else if(StrEqual(lastChoice, "70-80 Gain"))
	{
		selectedColor = g_i7080[client];
	}
	else if(StrEqual(lastChoice, "> 80 Gain"))
	{
		selectedColor = g_i80[client];
	}

	AddMenuItem(menu, "0", "White", selectedColor == White ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "1", "Red", selectedColor == Red ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "2", "Cyan", selectedColor == Cyan ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "3", "Purple", selectedColor == Purple ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "4", "Green", selectedColor == Green ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "5", "Blue", selectedColor == Blue ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "6", "Yellow", selectedColor == Yellow ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "7", "Orange", selectedColor == Orange ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(menu, "8", "Gray", selectedColor == Gray ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);

	menu.ExitBackButton = true;
	menu.DisplayAt(client, positon, MENU_TIME_FOREVER);
}

public int JHUDSettingsMenu_Handler(Menu menu, MenuAction action, int client, int option)
{
	if(action == MenuAction_Cancel && option == MenuCancel_ExitBack)
	{
		ShowJHUDMenu(client);
	}
	else if(action == MenuAction_Select)
	{
		char info[32];
		GetMenuItem(menu, option, info, sizeof(info));
		int i = StringToInt(info, sizeof(info));

		if(StrEqual(lastChoice, "< 60 Gain"))
		{
			g_i60[client] = i;
			SetCookie(client, g_hCookie60, g_i60[client]);
		}
		else if(StrEqual(lastChoice, "60-70 Gain"))
		{
			g_i6070[client] = i;
			SetCookie(client, g_hCookie6070, g_i6070[client]);
		}
		else if(StrEqual(lastChoice, "70-80 Gain"))
		{
			g_i7080[client] = i;
			SetCookie(client, g_hCookie7080, g_i7080[client]);
		}
		else if(StrEqual(lastChoice, "> 80 Gain"))
		{
			g_i80[client] = i;
			SetCookie(client, g_hCookie80, g_i80[client]);
		}
		ShowJHUDSettingsMenu(client, GetMenuSelectionPosition()); 
	}
	else if(action == MenuAction_End)
	{
		delete menu;
	}
}

void JHUD_ResetValues(int client)
{
	g_bStrafeSpeed[client] = false;
	g_i60[client] = Red;
	g_i6070[client] = Orange;
	g_i7080[client] = Green;
	g_i80[client] = Cyan;
	
	SetCookie(client, g_hCookieStrafeSpeed, g_bStrafeSpeed[client]);
	SetCookie(client, g_hCookie60, g_i60[client]);
	SetCookie(client, g_hCookie6070, g_i6070[client]);
	SetCookie(client, g_hCookie7080, g_i7080[client]);
	SetCookie(client, g_hCookie80, g_i80[client]);
}

void JHUD_GetStats(int client, float vel[3], float angles[3])
{
	float velocity[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", velocity);
	
	float gaincoeff;
	g_strafeTick[client]++;
	
	float fore[3], side[3], wishvel[3], wishdir[3];
	float wishspeed, wishspd, currentgain;
	
	GetAngleVectors(angles, fore, side, NULL_VECTOR);
	
	fore[2] = 0.0;
	side[2] = 0.0;
	NormalizeVector(fore, fore);
	NormalizeVector(side, side);
	
	for(int i = 0; i < 2; i++)
	{
		wishvel[i] = fore[i] * vel[0] + side[i] * vel[1];
	}
	
	wishspeed = NormalizeVector(wishvel, wishdir);
	if(wishspeed > GetEntPropFloat(client, Prop_Send, "m_flMaxspeed") && GetEntPropFloat(client, Prop_Send, "m_flMaxspeed") != 0.0)
	{
		wishspeed = GetEntPropFloat(client, Prop_Send, "m_flMaxspeed");
	}
	
	if(wishspeed)
	{
		wishspd = (wishspeed > 30.0) ? 30.0 : wishspeed;
		
		currentgain = GetVectorDotProduct(velocity, wishdir);
		if(currentgain < 30.0)
		{
			gaincoeff = (wishspd - FloatAbs(currentgain)) / wishspd;
		}

		if(g_bTouchesWall[client] && g_iTouchTicks[client] && gaincoeff > 0.5)
		{
			gaincoeff -= 1;
			gaincoeff = FloatAbs(gaincoeff);
		}

		g_flRawGain[client] += gaincoeff;
	}
}



void JHUD_DrawStats(int client, int target)
{
	float totalPercent = ((g_fTotalNormalDelta[target] / g_fTotalPerfectDelta[target]) * 100.0);

	float velocity[3];
	GetEntPropVector(target, Prop_Data, "m_vecAbsVelocity", velocity);

	float coeffsum = g_flRawGain[target];
	coeffsum /= g_strafeTick[target];
	coeffsum *= 100.0;
	
	coeffsum = RoundToFloor(coeffsum * 100.0 + 0.5) / 100.0;

	int rgb[3];

	char sMessage[256];
	if(g_iJump[target] <= 6 || g_iJump[target] == 16)
	{
		if(RoundToFloor(GetVectorLength(velocity)) < values[g_iJump[target]][0])
		{
			rgb = colors[g_i60[client]];
		}
		else if(RoundToFloor(GetVectorLength(velocity)) >= values[g_iJump[target]][0] && RoundToFloor(GetVectorLength(velocity)) < values[g_iJump[target]][1])
		{
			rgb = colors[g_i6070[client]];
		}
		else if(RoundToFloor(GetVectorLength(velocity)) >= values[g_iJump[target]][1] && RoundToFloor(GetVectorLength(velocity)) < values[g_iJump[target]][2])
		{
			rgb = colors[g_i7080[client]];
		}
		else
		{
			rgb = colors[g_i80[client]];
		}

		if(!IsNaN(totalPercent) && g_bStrafeSpeed[client])
		{
			Format(sMessage, sizeof(sMessage), "%i: %i (%.0f%%%%)", g_iJump[target], RoundToFloor(GetVectorLength(velocity)), totalPercent);
		}
		else
		{
			Format(sMessage, sizeof(sMessage), "%i: %i", g_iJump[target], RoundToFloor(GetVectorLength(velocity)));
		}
	}
	else
	{
		if(coeffsum < 60)
		{
			rgb = colors[g_i60[client]];
		}
		else if(coeffsum >= 60 && coeffsum < 70)
		{
			rgb = colors[g_i6070[client]];
		}
		else if(coeffsum >= 70 && coeffsum < 80)
		{
			rgb = colors[g_i7080[client]];
		}
		else
		{
			rgb = colors[g_i80[client]];
		}

		if(!IsNaN(totalPercent) && g_bStrafeSpeed[client])
		{
			Format(sMessage, sizeof(sMessage), "%.2f%%%% (%.0f%%%%)", coeffsum, totalPercent);
		}
		else
		{
			Format(sMessage, sizeof(sMessage), "%.2f%%", coeffsum);
		}
	}
 	
	SetHudTextParams(-1.0, 0.4, 1.0, rgb[0], rgb[1], rgb[2], 255, 0, 0.0, 0.0, 0.2);
	ShowHudText(client, 2, sMessage);
}

stock bool GetClientCookieBool(int client, Handle cookie)
{
	char sValue[8];
	GetClientCookie(client, g_hCookieJHUD, sValue, sizeof(sValue));
	return (sValue[0] != '\0' && StringToInt(sValue));
}



stock void SetCookie(int client, Handle hCookie, int n)
{
	char strCookie[64];
	IntToString(n, strCookie, sizeof(strCookie));
	SetClientCookie(client, hCookie, strCookie);
}