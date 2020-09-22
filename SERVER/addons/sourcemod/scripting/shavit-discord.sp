#include <sourcemod>
#include <shavit>
#include <discord>

#define PLUGIN_VERSION "1.1"

#define WEBHOOK ""

#define MIN_RECORDS 50

#define MAIN_MSG_COLOUR "#00ffff"

char g_cCurrentMap[PLATFORM_MAX_PATH];

ConVar g_cvHostname;
char g_cHostname[128];

public Plugin myinfo =
{
	name = "[SHAVIT] Discord WR Bot",
	author = "SlidyBat",
	description = "Makes discord bot post message when server WR is beaten",
	version = PLUGIN_VERSION
}

public void OnPluginStart()
{  
	g_cvHostname = FindConVar("hostname");
	g_cvHostname.GetString( g_cHostname, sizeof( g_cHostname ) );
	g_cvHostname.AddChangeHook( OnConVarChanged );
}

public void OnConVarChanged( ConVar convar, const char[] oldValue, const char[] newValue )
{
	g_cvHostname.GetString( g_cHostname, sizeof( g_cHostname ) );
}

public void OnMapStart()
{
	GetCurrentMap( g_cCurrentMap, sizeof( g_cCurrentMap ) );
}

public void Shavit_OnWorldRecord(int client, int style, float time, int jumps, int strafes, float sync, int track, float oldwr, float oldtime, float perfs)
{
	char styleName[128];
	Shavit_GetStyleStrings( style, sStyleName, styleName, sizeof( styleName ));
	
	if(MIN_RECORDS > 0 && (Shavit_GetRecordAmount( style, track ) < MIN_RECORDS))
	{
		return;
	}
	
	if(!StrEqual(styleName, "Normal"))
	{
		return;
	}
	
	if(track == Track_Bonus)
	{
		return;
	}
	
	int sMaprank = Shavit_GetRecordAmount(style, track);
	
	DiscordWebHook hook = new DiscordWebHook( WEBHOOK );
	hook.SlackMode = true;
	hook.SetUsername( "Record Bot" );
	hook.SetAvatar("https://i.imgur.com/BisyYvV.png");
	
	MessageEmbed embed = new MessageEmbed();
	
	embed.SetColor(MAIN_MSG_COLOUR);
	
	char buffer[512];
	Format( buffer, sizeof( buffer ), "__**NEW SERVER RECORD**__ | **%s**", g_cCurrentMap );
	embed.SetTitle( buffer );
	
	char steamid[65];
	GetClientAuthId( client, AuthId_SteamID64, steamid, sizeof( steamid ) );
	Format( buffer, sizeof( buffer ), "[%N](http://www.steamcommunity.com/profiles/%s)", client, steamid );
	embed.AddField( "Player:", buffer, true	);
	
	FormatSeconds( time, buffer, sizeof( buffer ) );
	Format( buffer, sizeof( buffer ), "󠇰    󠇰    󠇰 󠇰 󠇰 󠇰 󠇰 󠇰  󠇰 󠇰 󠇰%ss", buffer );
	embed.AddField( "󠇰    󠇰    󠇰  Time:", buffer, true );
	
	Format( buffer, sizeof( buffer ), "**Strafes**: %i\t\t\t\t\t\t**Sync**: %.2f%%\t\t\t\t\t\t**Jumps**: %i", strafes, sync, jumps );
	embed.AddField( "Stats:", buffer, true );
    
	Format( buffer, sizeof( buffer ), "1/%d", sMaprank);
	embed.AddField("Rank:", buffer, true);
	
	hook.Embed( embed );
	hook.Send();
}