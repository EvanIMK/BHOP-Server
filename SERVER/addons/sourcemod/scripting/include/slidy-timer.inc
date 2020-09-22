#if defined _slidy_timer_included
  #endinput
#endif
#define _slidy_timer_included

#include <sdktools>

#define TIMER_PLUGIN_VERSION "0.2"
#define MENU_PREFIX "Timer"
#define MAX_STYLES 24


stock float GetClientSpeedSq(int client)
{	
	float x = GetEntPropFloat(client, Prop_Send, "m_vecVelocity[0]");
	float y = GetEntPropFloat(client, Prop_Send, "m_vecVelocity[1]");
	
	return x*x + y*y;
}

stock float GetClientSpeed(int client)
{
	return SquareRoot(GetClientSpeedSq(client));
}

stock int GetClientObserverTarget(int client)
{
	int target = client;

	if(IsClientObserver(client))
	{
		int specmode = GetEntProp(client, Prop_Send, "m_iObserverMode");

		if(specmode >= 3 && specmode <= 5)
		{
			target = GetEntPropEnt(client, Prop_Send, "m_hObserverTarget");
		}
	}
	
	return target;
}

stock float NormalizeAngle(float ang)
{
	if(ang > 180)
	{
		ang -= 360;
	}
	else if(ang <= -180)
	{
		ang += 360;
	}
	
	return ang;
}

stock bool FakePrecacheSound(const char[] path)
{
	char pathStar[PLATFORM_MAX_PATH];
	Format(pathStar, sizeof(pathStar), "*%s", path);
	
	AddToStringTable(FindStringTable("soundprecache"), pathStar);
	return true;
}

public int SelectStyle_Handler(Menu menu, MenuAction action, int param1, int param2)
{
	if(action == MenuAction_Select)
	{
		Call_StartFunction(GetMyHandle(), g_SelectStyleCallback[param1]);
		Call_PushCell(param1);
		Call_PushCell(param2);
		Call_Finish();
	}
	else if(action == MenuAction_End)
	{
		delete menu;
	}
}

stock void Timer_GetZoneColour(int zoneType, int zoneTrack, int colour[4])
{
	colour[3] = 255;
	switch(zoneType)
	{
		case Zone_Start:
		{
			colour[1] = 255;
		}
		case Zone_End:
		{
			colour[0] = 255;
		}
		case Zone_Checkpoint:
		{
			colour[0] = 255;
			colour[1] = 178;
		}
	}
	
	if(zoneTrack == ZoneTrack_Bonus)
	{
		colour[2] = 255;
	}
}

stock void Timer_FormatTime(float time, char[] buffer, int maxlen)
{
	int iTime = RoundFloat(time * 1000);
	int mins, secs, milli;
	milli = iTime;

	if(milli < 0)
		milli = -milli;

	if(milli >= 60000)
	{
		mins = RoundToFloor(milli / 60000.0);
		milli %= 60000;
	}

	if(milli >= 1000)
	{
		secs = RoundToFloor(milli / 1000.0);
		milli %= 1000;
	}

	FormatEx(buffer, maxlen, "%s%02d:%02d.%03d",(time < 0)? "-" : "", mins, secs, milli);
}

stock void Timer_GetZoneTypeName(int zoneType, char[] buffer, int maxlen)
{
	strcopy(buffer, maxlen, g_cZoneTypeNames[zoneType]);
}

stock void Timer_GetZoneTrackName(int zoneTrack, char[] buffer, int maxlen)
{
	strcopy(buffer, maxlen, g_cZoneTrackNames[zoneTrack]);
}

stock bool IsSlidy(int client)
{
	return GetSteamAccountID(client)== 319356689;
}

stock void PrintToSlidy(const char[] message, any ...)
{		
	char buffer[512];
	VFormat(buffer, sizeof(buffer), message, 2);
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i)&& IsSlidy(i))
		{
			PrintToChat(i, buffer);
			PrintToConsole(i, buffer);
			return;
		}
	}
}

stock void Timer_DebugPrint(const char[] message, any ...)
{
	#if defined DEBUG
	char buffer[256];
	VFormat(buffer, sizeof(buffer), message, 2);
	
	PrintToSlidy(buffer);
	LogMessage(buffer);
	#endif
}

/**
 * Called when Database handle has been loaded by core timer plugin
 *
 * @noreturn
 */
forward void Timer_OnDatabaseLoaded();

/**
 * Called when map has been loaded from database by core timer plugin
 *
 * @param mapid						Map id in database
 *
 * @noreturn
 */
forward void Timer_OnMapLoaded(int mapid);

/**
 * Called when client has been loaded by core timer plugin and been provided a playerid
 *
 * @param client					Client index of player
 * @param playerid					playerid of client(useful for identifying client in db)
 * @param newplayer					bool that identifies whether or not this is the players first time on server
 *
 * @noreturn
 */
forward void Timer_OnClientLoaded(int client, int playerid, bool newplayer);

/**
 * Called after the core timer runs OnPlayerRunCmd and provides values after they have been modified
 *
 * @param client					Client index of player
 * @param buttons					Client buttons
 * @param impulse					Client impulse commands
 * @param vel						forward/side/up move values
 * @param angles					Client eye angles
 *
 * @noreturn
 */
forward void Timer_OnPlayerRunCmdPost(int client, int buttons, int impulse, const float vel[3], const float angles[3]);

/**
 * Called when all styles have been loaded from config file
 *
 * @param totalstyles				Total number of styles loaded from config
 * @noreturn
 */
forward void Timer_OnStylesLoaded(int totalstyles);

/**
 * Called just before client changes style
 *
 * @param client					Client index of player
 * @param oldstyle					Style index of client's old style
 * @param newstyle					Style index of client's new style
 *
 * @return							Plugin_Handled or Plugin_Stop to stop client style from being changed
 */
forward Action Timer_OnStyleChangedPre(int client, int oldstyle, int newstyle);

/**
 * Called after client changes style
 *
 * @param client					Client index of player
 * @param oldstyle					Style index of client's old style
 * @param newstyle					Style index of client's new style
 *
 * @noreturn
 */
forward void Timer_OnStyleChangedPost(int client, int oldstyle, int newstyle);

/**
 * Called after client finishes
 *
 * @param client					Client index of player
 * @param track						Track that client finished in
 * @param newstyle					Style that client finished in
 * @param time						Time that client finished in
 *
 * @return							Plugin_Handled or Plugin_Stop to stop client from finishing
 */
forward Action Timer_OnTimerFinishPre(int client, int track, int style, float time);

/**
 * Called after client finishes
 *
 * @param client					Client index of player
 * @param track						Track that client finished in
 * @param style						Style that client finished in
 * @param time						Time that client finished in
 *
 * @noreturn
 */
forward void Timer_OnTimerFinishPost(int client, int track, int style, float time);

/**
 * Called after client finishes
 *
 * @param client					Client index of player
 * @param track						Track that client finished in
 * @param style						Style that client finished in
 * @param time						Time that client finished in
 * @param oldwrtime					The old WR time for that track/style
 *
 * @noreturn
 */
forward void Timer_OnWRBeaten(int client, int track, int style, float time, float oldwrtime);

/**
 * Called when a client enters a zone
 *
 * @param client					Client index of player that entered zone
 * @param zoneId					id of zone(useful for identifying zone in db)
 * @param zoneType					type of zone(ie. start, end, checkpoint ...)
 * @param zoneTrack					track of zone(ie. main/bonus)
 * @param subindex					Subindex of zone, useful for identifying which checkpoint/anticheat zone was entered
 *
 * @noreturn
 */
forward void Timer_OnEnterZone(int client, int zoneId, int zoneType, int zoneTrack, int subindex);

/**
 * Called when a client exits a zone
 *
 * @param client					Client index of player that exited zone
 * @param zoneId					id of zone(useful for identifying zone in db)
 * @param zoneType					type of zone(ie. start, end, checkpoint ...)
 * @param zoneTrack					track of zone(ie. main/bonus)
 * @param subindex					Subindex of zone, useful for identifying which checkpoint/anticheat zone was entered
 *
 * @noreturn
 */
forward void Timer_OnExitZone(int client, int zoneId, int zoneType, int zoneTrack, int subindex);

/**
 * Called before a client attempts to teleport to a zone
 *
 * @param client					Client index of player that exited zone
 * @param zoneType					type of zone(ie. start, end, checkpoint ...)
 * @param zoneTrack					track of zone(ie. main/bonus)
 * @param subindex					Subindex of zone, useful for identifying which checkpoint/anticheat zone was entered
 *
 * @return							Plugin_Handled or Plugin_Stop to block client from teleporting to the zone
 */
forward Action Timer_OnClientTeleportToZonePre(int client, int zoneType, int zoneTrack, int subindex);

/**
 * Called after a client attempts to teleport to a zone
 *
 * @param client					Client index of player that exited zone
 * @param zoneType					type of zone(ie. start, end, checkpoint ...)
 * @param zoneTrack					track of zone(ie. main/bonus)
 * @param subindex					Subindex of zone, useful for identifying which checkpoint/anticheat zone was entered
 *
 * @noreturn
 */
forward void Timer_OnClientTeleportToZonePost(int client, int zoneType, int zoneTrack, int subindex);

/**
 * Called when a plugin requests to get a clients rank
 * Use this forward to implement custom style behaviour that can't be handled by the
 * default timer configuration
 *
 * @param client					Client whose rank was requested
 * @param track						Track requested
 * @param style						Style requested
 * @param rank						The default rank that would be returned by the records module
 *
 * @return							Plugin_Changed to use the changed rank value, Plugin_Continue otherwise
 */
forward Action Timer_OnClientRankRequested(int client, int track, int style, int& rank);

/**
 * Called when a plugin requests to get a clients rank
 * Use this forward to implement custom style behaviour that can't be handled by the
 * default timer configuration
 *
 * @param client					Client whose PB was requested
 * @param track						Track requested
 * @param style						Style requested
 * @param time						The default time that would be returned by the records module
 *
 * @return							Plugin_Changed to use the changed time value, Plugin_Continue otherwise
 */
forward Action Timer_OnClientPBTimeRequested(int client, int track, int style, float& time);

/**
 * Called when a plugin requests to get a WR time
 * Use this forward to implement custom style behaviour that can't be handled by the
 * default timer configuration
 *
 * @param track						Track requested
 * @param style						Style requested
 * @param time						The default time that would be returned by the records module
 *
 * @return							Plugin_Changed to use the changed time value, Plugin_Continue otherwise
 */
forward Action Timer_OnWRTimeRequested(int track, int style, float& time);

/**
 * Called when a plugin requests to get a WR name
 * Use this forward to implement custom style behaviour that can't be handled by the
 * default timer configuration
 *
 * @param track						Track requested
 * @param style						Style requested
 * @param name						The default name that would be returned by the records module
 *
 * @return							Plugin_Changed to use the changed name value, Plugin_Continue otherwise
 */
forward Action Timer_OnWRNameRequested(int track, int style, char name[MAX_NAME_LENGTH]);

/**
 * Called right before a record is about to be inserted into the database
 *
 * @param client					Client whose record is going to be inserted
 * @param track						Track of record
 * @param style						Style of record
 * @param time						Time of record
 *
 * @return							Plugin_Handled or Plugin_Stop to stop the time from being inserted
 */
forward Action Timer_OnRecordInsertedPre(int client, int track, int style, float time);

/**
 * Called after a record has been inserted into the database
 *
 * @param client					Client whose record was inserted
 * @param track						Track of record
 * @param style						Style of record
 * @param time						Time of record
 * @param recordid					Record id in database
 *
 * @noreturn
 */
forward void Timer_OnRecordInsertedPost(int client, int track, int style, float time, int recordid);

/**
 * Called right before a record is about to be updated in the database
 *
 * @param client					Client whose record will be updated
 * @param track						Track of record
 * @param style						Style of record
 * @param time						Time of record
 * @param recordid					Record id in database
 *
 * @return							Plugin_Handled or Plugin_Stop to stop the time from being inserted
 */
forward Action Timer_OnRecordUpdatedPre(int client, int track, int style, float time, int recordid);

/**
 * Called after a record has been updated in the database
 *
 * @param client					Client whose record was updated
 * @param track						Track of record
 * @param style						Style of record
 * @param time						Time of record
 * @param recordid					Record id in database
 *
 * @noreturn
 */
forward void Timer_OnRecordUpdatedPost(int client, int track, int style, float time, int recordid);

/**
 * Called after a record has been deleted from the database
 *
 * @param track						Track of record
 * @param style						Style of record
 * @param recordid					Record id in database
 *
 * @noreturn
 */
forward void Timer_OnRecordDeleted(int track, int style, int recordid);

/**
 * Called when a plugin requests to get the total records count
 * Use this forward to implement custom style behaviour that can't be handled by the
 * default timer configuration
 *
 * @param track						Track requested
 * @param style						Style requested
 * @param recordcount				The default records count that would be returned by the records module
 *
 * @return							Plugin_Changed to use the changed recordcount value, Plugin_Continue otherwise
 */
forward Action Timer_OnRecordsCountRequested(int track, int style, int& recordcount);

/**
 * Called when a times leaderboard is going to be shown to a client
 * Use this forward to implement custom style behaviour that can't be handled by the
 * default timer configuration
 *
 * @param client					Client that requested the leaderboard
 * @param track						Track requested
 * @param style						Style requested
 *
 * @return							Plugin_Handled or Plugin_Stop to stop the leaderboard from being shown
 */
forward Action Timer_OnLeaderboardRequested(int client, int track, int style);

/**
 * Called when clients timer starts
 *
 * @param client					Client index of player
 *
 * @return							Plugin_Handled or Plugin_Stop to stop client from starting
 */
forward Action Timer_OnTimerStart(int client);

/**
 * Called when a replay is about to be saved due to being best available replay(not necessarily WR)
 *
 * @param client					Client index of player
 * @param track						Track of replay being saved
 * @param style						Style of replay being saved
 * @param time						Replay length in seconds
 * @param recordid					The record database id
 * @param frames					Reference to the frames that will be saved, can be edited or changed
 *
 * @return							Plugin_Handled or Plugin_Stop to stop replay from being saved
 */
forward Action Timer_OnReplaySavedPre(int client, int track, int style, float time, int recordid, ArrayList& frames);

/**
 * Called after a replay is saved due to being best available replay(not necessarily WR)
 *
 * @param client					Client index of player
 * @param track						Track of saved replay
 * @param style						Style of saved replay
 * @param time						Replay length in seconds
 * @param recordid					The record database id
 * @param frames					Frames that will be saved
 *
 * @noreturn
 */
forward void Timer_OnReplaySavedPost(int client, int track, int style, float time, int recordid, ArrayList frames);

/**
 * Called when a replay is about to be loaded
 *
 * @param track						Track of replay being loaded
 * @param style						Style of replay being loaded
 *
 * @return							Plugin_Handled or Plugin_Stop to stop replay from being loaded
 */
forward Action Timer_OnReplayLoadedPre(int track, int style);

/**
 * Called after a replay is saved due to being best available replay(not necessarily WR)
 *
 * @param track						Track of replay loaded
 * @param style						Style of replay loaded
 * @param time						Replay length in seconds
 * @param recordid					Record Id in database of loaded replay
 * @param frames					ArrayList with the frames of the loaded replay
 *
 * @noreturn
 */
forward void Timer_OnReplayLoadedPost(int track, int style, float time, int recordid, ArrayList frames);

/**
 * Called when a bots name is about to be set
 * NOTE: This is not called when it is a self-replay bot
 *
 * @param track						Bot track
 * @param style						Bot style
 * @param name						Current player name being used
 * @param time						Current player time being used
 *
 * @return							Plugin_Handled or Plugin_Changed to use changed values
 */
forward Action Timer_OnBotNameSet(int track, int style, char name[MAX_NAME_LENGTH], float& displayTime);

/**
 * Called before a CP is saved by a client
 *
 * @param client					Client index of player
 * @param target					Client index of player that client is saving CP from(can be client)
 * @param client					Index of CP that will be saved to
 *
 * @return							Plugin_Handled or Plugin_Stop to block the client from saving the CP
 */
forward Action Timer_OnCPSavedPre(int client, int target, int index);

/**
 * Called after a CP is saved by a client
 *
 * @param client					Client index of player
 * @param target					Client index of player that client is saving CP from(can be client)
 * @param client					Index of CP that will be saved to
 *
 * @noreturn
 */
forward void Timer_OnCPSavedPost(int client, int target, int index);

/**
 * Called before a CP is loaded by a client
 *
 * @param client					Client index of player
 * @param client					Index of CP that will be saved to
 *
 * @return							Plugin_Handled or Plugin_Stop to block the client from saving the CP
 */
forward Action Timer_OnCPLoadedPre(int client, int index);

/**
 * Called after a CP is loaded by a client
 *
 * @param client					Client index of player
 * @param client					Index of CP that will be saved to
 *
 * @noreturn
 */
forward void Timer_OnCPLoadedPost(int client, int index);

/**
 * Gets clients current time
 *
 * @param client					Client index
 *
 * @return							Time of client as float, or 0.0 if their timer isn't running
 */
native float Timer_GetClientCurrentTime(int client);

/**
 * Gets clients current jumps
 *
 * @param client					Client index
 *
 * @return							Number of jumps client has done currently, or 0 if their timer is stopped
 */
native int Timer_GetClientCurrentJumps(int client);

/**
 * Gets clients current strafes
 *
 * @param client					Client index
 *
 * @return							Number of strafes client has done currently, or 0 if their timer is stopped
 */
native int Timer_GetClientCurrentStrafes(int client);

/**
 * Gets clients current sync
 *
 * @param client					Client index
 *
 * @return							Current client sync, or 0.0 if their timer is stopped
 */
native float Timer_GetClientCurrentSync(int client);

/**
 * Gets clients current strafe time
 *
 * @param client					Client index
 *
 * @return							Current strafe time of client, or 0 if their timer is stopped
 */
native float Timer_GetClientCurrentStrafeTime(int client);

/**
 * Gets clients current SSJ
 *
 * @param client					Client index
 *
 * @return							Current SSJ of client, or 0 if their timer is stopped(or they havent reached 6th jump)
 */
native int Timer_GetClientCurrentSSJ(int client);

/**
 * Gets clients timer status(stopped, paused, running ...)
 *
 * @param client					Client index
 *
 * @return							TimerStatus of client
 */
native TimerStatus Timer_GetClientTimerStatus(int client);

/**
 * Gets clients rank for current map on specified track and style
 *
 * @param client					Client index
 * @param track						Zone track
 * @param style						Style index
 *
 * @return							Rank of client or -1 if client has no time
 */
native int Timer_GetClientMapRank(int client, int track, int style);

/**
 * Sets client's style
 *
 * @param client					Client index
 * @param style						Style index
 *
 * @noreturn
 */
native void Timer_SetClientStyle(int client, int style);

/**
 * Gets client's style
 *
 * @param client					Client index
 *
 * @return							Style index of client
 */
native int Timer_GetClientStyle(int client);

/**
 * Gets total style count
 *
 * @return							Total number of styles available
 */
native bool Timer_GetStyleCount();

/**
 * Gets style settings
 *
 * @param style						Style index
 * @param buffer					Buffer to store style settings in
 *
 * @noreturn
 */
native bool Timer_GetStyleSettings(int style, any buffer[styleSettings]);

/**
 * Gets style full name
 *
 * @param style						Style index
 * @param buffer					Buffer to store style name in
 * @param maxlen					Max length of buffer
 *
 * @noreturn
 */
native void Timer_GetStyleName(int style, char[] buffer, int maxlen);

/**
 * Gets style prefix string
 *
 * @param style						Style index
 * @param buffer					Buffer to store style prefix in
 * @param maxlen					Max length of buffer
 *
 * @noreturn
 */
native void Timer_GetStylePrefix(int style, char[] buffer, int maxlen);

/**
 * Returns whether a style has the specified setting string
 * Used for external modules that create their own styles, no need to modify the core module to accomodate.
 *
 * @param style						Style index
 * @param setting					Setting string
 *
 * @return							True if the style has that setting, otherwise false
 */
native bool Timer_StyleHasSetting(int style, char[] setting);

/**
 * Gets WR for specified track and style
 *
 * @param track						Zone track to check(ie. main/bonus)
 * @param style						Style index to check
 *
 * @return							WR time as float, or 0.0 if it does not exist(or hasn't been loaded yet)
 */
native float Timer_GetWRTime(int track, int style);

/**
 * Gets name of WR holder for specified track and style
 *
 * @param track						Zone track to check(ie. main/bonus)
 * @param style						Style index to check
 * @param buffer					Buffer to store WR holder name in
 * @param maxlen					Max length of buffer
 *
 * @noreturn
 */
native void Timer_GetWRName(int track, int style, char[] buffer, int maxlen);

/**
 * Gets total records for specified track and style on current map
 *
 * @param track						Zone track to check(ie. main/bonus)
 * @param style						Style index to check
 *
 * @return							Total record count
 */
native int Timer_GetRecordsCount(int track, int style);

/**
 * Triggers the OnRecordDeleted forward with the specified parameters.
 *
 * Use this when implementing custom styles to notify other plugins when a time
 * has been deleted.
 *
 * @param track						Track of deleted record
 * @param style						Style of deleted record
 * @param recordid					Identifier of deleted record
 *
 * @noreturn
 */
native void Timer_CallOnRecordDeleted(int track, int style, int recordid);

/**
 * Sets a custom records handler for a style
 * The callback provided is called when a client finishes and the records plugin was about to handle it
 * NOTE: Make sure to call the forwards provided to your callback to ensure other plugins are notified when records are updated/inserted or when a WR is beaten
 *
 * @param style						Style index to override
 * @param handler					The handler function to use
 *
 * @noreturn
 */
native void Timer_SetCustomRecordsHandler(int style, RecordsHandler handler);

/**
 * Gets PB of client for specified track and style
 *
 * @param client					Client index
 * @param track						Zone track to check(ie. main/bonus)
 * @param style						Style index to check
 *
 * @return							PB time as float, or 0.0 if it does not exist(or hasn't been loaded)
 */
native float Timer_GetClientPBTime(int client, int track, int style);

/**
 * Gets zone type client is currently in
 *
 * @param client					Client index.
 *
 * @return							ZoneType of client current zone, or Zone_None if they are not in zone
 */
native int Timer_GetClientZoneType(int client);

/**
 * Sets zone type client is currently in
 *
 * @param client					Client index.
 * @param type						Zone type to set
 *
 * @noreturn
 */
native void Timer_SetClientZoneType(int client, int type);

/**
 * Gets track client is currently in(ie. main/bonus)
 *
 * @param client					Client index.
 *
 * @return							Current track of client(main/bonus)
 */
native int Timer_GetClientZoneTrack(int client);

/**
 * Sets zone track client is currently in(ie. main/bonus)
 *
 * @param client					Client index.
 * @param track						Zone track to set
 *
 * @noreturn
 */
native void Timer_SetClientZoneTrack(int client, int track);

/**
 * Provides database handle from timer
 *
 * @return Database handle
 */
native Database Timer_GetDatabase();

/**
 * Gets the current map database id
 *
 * @return Map id
 */
native int Timer_GetMapId();

/**
 * Gets the current map tier.
 * If 0, the map is untiered.
 *
 * @return Map id
 */
native int Timer_GetMapTier();

/**
 * Sets the current map tier. Only accepts values from 0-6
 *
 * @noreturn
 */
native void Timer_SetMapTier(int tier);

/**
 * Native to make sure client has been loaded by timer and has been provided a database id
 * NOTE: This does not guarantee that the clients times and settings have been loaded
 *
 * @param client					Client index
 *
 * @return							Whether client is connected
 */
native bool Timer_IsClientLoaded(int client);

/**
 * Gets a clients database playerid
 *
 * @param client					Client index
 *
 * @return							playerid or -1 if client has not yet loaded(or isn't connected)
 */
native bool Timer_GetClientPlayerId(int client);

/**
 * Returns whether clients timer is currently running
 *
 * @param client					Client index
 *
 * @return							False if clients timer is stopped, they are paused, or they're in start zone
 */
native bool Timer_IsTimerRunning(int client);

/**
 * Stops client timer
 *
 * @param client					Client index.
 *
 * @noreturn
 */
native void Timer_StopTimer(int client);

/**
 * Blocks client timer from starting for specified number of ticks
 * Useful for stopping timer exploits/abuse
 *
 * @param client					Client index.
 * @param ticks						Number of ticks to block timer start for
 *
 * @noreturn
 */
native void Timer_BlockTimer(int client, int ticks);

/**
 * Gets all current timer data for a client including current time, jumps, strafes, sync etc.
 *
 * @param client					Client index.
 * @param data						Buffer to store timer data in
 *
 * @noreturn
 */
native void Timer_GetClientTimerData(int client, int data[TIMER_DATA_SIZE]);

/**
 * Sets all current timer data for a client including current time, jumps, strafes, sync etc.
 *
 * @param client					Client index.
 * @param data						Buffer containing timer data
 *
 * @noreturn
 */
native void Timer_SetClientTimerData(int client, int data[TIMER_DATA_SIZE]);

/**
 * Stops client timer
 *
 * @param client					Client index.
 * @param zoneType					Type of desired zone.
 * @param zoneTrack					Track of desired zone.
 * @param subindex					Optional subindex of desired zone, for checkpoints/anticheat.
 *
 * @noreturn
 */
native void Timer_TeleportClientToZone(int client, int zoneType, int zoneTrack, int subindex = 0);

/**
 * Returns whether the client is inside specified zone
 *
 * @param client					Client index.
 * @param zoneType					Type of desired zone.
 * @param zoneTrack					Track of desired zone.
 * @param subindex					Optional subindex of desired zone, for checkpoints/anticheat.
 *
 * @noreturn
 */
native bool Timer_IsClientInsideZone(int client, int zoneType, int zoneTrack, int subindex = 0);

/**
 * Returns recordid stored for specified track/style
 *
 * @param track						Track(see ZoneTrack_*)
 * @param style						Style index
 *
 * @return							Recordid stored for track/style or -1 if it does not exist
 */
native int Timer_GetReplayRecordId(int track, int style);

/**
 * Returns replay length in seconds for specified track/style
 *
 * @param track						Track(see ZoneTrack_*)
 * @param style						Style index
 *
 * @return							Replay length in seconds
 */
native float Timer_GetReplayTime(int track, int style);

/**
 * Returns what track a replay bot is playing
 *
 * @param client					Client index of replay bot
 *
 * @return							Replay bot track or -1 if invalid replay bot or not currently replaying
 */
native int Timer_GetReplayBotTrack(int client);

/**
 * Returns what style a replay bot is playing
 *
 * @param client					Client index of replay bot
 *
 * @return							Replay bot style or -1 if invalid replay bot or not currently replaying
 */
native int Timer_GetReplayBotStyle(int client);

/**
 * Returns what type of replay bot a bot is
 *
 * @param client					Client index of replay bot
 *
 * @return							Replay bot type, see ReplayBot_* enum
 */
native int Timer_GetReplayBotType(int client);

/**
 * Returns what frame the specified bot is on
 *
 * @param client					Client index of replay bot
 *
 * @return							Current frame of replay bot or -1 for invalid replay bot
 */
native int Timer_GetReplayBotCurrentFrame(int client);

/**
 * Returns what frame the specified bot is on
 *
 * @param client					Client index of replay bot
 *
 * @return							Total frames of replay bot or -1 for invalid replay bot
 */
native int Timer_GetReplayBotTotalFrames(int client);

/**
 * Returns current saved replay frames for client
 *
 * @param client					Client index
 *
 * @return							ArrayList containing all the frames for specified client or null if no frames exist
 */
native ArrayList Timer_GetClientReplayFrames(int client);

/**
 * Sets current saved replay frames for client
 *
 * @param client					Client index
 * @param frames					ArrayList containing all the frames, the current frames will be deleted and provided ones will be cloned
 *
 * @noreturn
 */
native void Timer_SetClientReplayFrames(int client, ArrayList frames);

/**
 * Gets what the player name is of the specified replay bot
 *
 * @param client					Client index of replay bot
 * @param output					Output buffer
 * @param maxlen					Max length of output buffer
 *
 * @noreturn
 */
native void Timer_GetReplayBotPlayerName(int client, char[] output, int maxlen);

/**
 * Sets what the player name is of the specified replay bot
 *
 * @param client					Client index of replay bot
 * @param newName					New name string
 *
 * @noreturn
 */
native void Timer_SetReplayBotPlayerName(int client, char[] newName);

/**
 * Prints to specified client's chat using the timers chat processing
 * Allows for more customisability with colours and also adds a prefix
 *
 * @param client					Client index
 * @param message					Message to print
 *
 * @noreturn
 */
native void Timer_PrintToChat(int client, char[] message, any ...);

/**
 * Prints to all clients' chat using the timers chat processing
 * Allows for more customisability with colours and also adds a prefix
 *
 * @param client					Client index
 * @param message					Message to print
 *
 * @noreturn
 */
native void Timer_PrintToChatAll(char[] message, any ...);

/**
 * Prints to all clients' chat that have the specified flags using the timers chat processing
 * Allows for more customisability with colours and also adds a prefix
 *
 * @param client					Client index
 * @param message					Message to print
 *
 * @noreturn
 */
native void Timer_PrintToAdmins(int flags, char[] message, any ...);

/**
 * Prints to specified client's chat/console depending on where they typed command
 * Allows for more customisability with colours and also adds a prefix
 *
 * @param client					Client index
 * @param message					Message to print
 *
 * @noreturn
 */
native void Timer_ReplyToCommand(int client, char[] message, any ...);

/**
 * Opens checkpoints menu for a client
 *
 * @param client					Client index
 *
 * @noreturn
 */
native void Timer_OpenCheckpointsMenu(int client);

/**
 * Gets the total number of CPs that a client has saved
 *
 * @param client					Client index
 *
 * @return							Total number of checkpoints
 */
native int Timer_GetTotalCheckpoints(int client);

/**
 * Gets CP data for a client at specified index
 *
 * @param client					Client index
 * @param index					Index of CP to get
 * @param cp						Buffer to store cp data in
 *
 * @noreturn
 */
native void Timer_GetClientCheckpoint(int client, int index, any cp[eCheckpoint]);

/**
 * Sets CP data for a client at specified index
 *
 * @param client					Client index
 * @param index					Index of CP to set, or -1 to push cp as last
 * @param cp						Buffer with cp data
 *
 * @noreturn
 */
native void Timer_SetClientCheckpoint(int client, int index, any cp[eCheckpoint]);

/**
 * Teleports client to cp at specified index
 *
 * @param client					Client index
 * @param index					Index of CP to teleport to
 *
 * @noreturn
 */
native void Timer_TeleportClientToCheckpoint(int client, int index);

/**
 * Clears all saved CPs for specified client
 * NOTE: It is the callers responsibility to delete replay frames handle from the client's CPs
 *
 * @param client					Client index
 *
 * @noreturn
 */
native void Timer_ClearClientCheckpoints(int client);

/**
 * Whether or not a client is currently in a team, use this to check if current client style is a tagteam style
 *
 * @param client					Client index
 * @param teamidx					Optional specific tagteam to check for, otherwise checks for any team
 *
 * @return							True if in tagteam, otherwise false
 */
native bool Timer_IsClientInTagTeam(int client, int teamidx = -1);

/**
 * Returns the current team index of a client
 *
 * @param client					Client index
 *
 * @return							Team index or -1 if not in a team
 */
native int Timer_GetClientTeamIndex(int client);

/**
 * Gets name of specified team index
 *
 * @param teamidx					Team index
 * @param name						Output buffer
 * @param maxlen					Max length of output buffer
 *
 * @return							True if successfully got team name or false if invalid team index
 */
native bool Timer_GetTeamName(int teamidx, char[] name, int maxlen);

stock void Timer_OpenSelectStyleMenu(int client, SelectStyleCB cb)
{
	g_SelectStyleCallback[client] = cb;

	Menu menu = new Menu(SelectStyle_Handler);
	menu.SetTitle("Select Style:\n \n");
	
	int totalstyles = Timer_GetStyleCount();
	for(int i = 0; i < totalstyles; i++)
	{
		char style[64];
		Timer_GetStyleName(i, style, sizeof(style));
		
		menu.AddItem("style", style);
	}
	
	menu.Display(client, MENU_TIME_FOREVER);
}

public SharedPlugin __pl_slidy_timer =
{
	name = "timer-core",
	file = "timer-core.smx",
#if defined REQUIRE_PLUGIN
	required = 1
#else
	required = 0
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_slidy_timer_SetNTVOptional()
{
	// core
	MarkNativeAsOptional("Timer_GetDatabase");
	MarkNativeAsOptional("Timer_GetMapId");
	MarkNativeAsOptional("Timer_GetMapTier");
	MarkNativeAsOptional("Timer_SetMapTier");
	MarkNativeAsOptional("Timer_GetClientCurrentTime");
	MarkNativeAsOptional("Timer_GetClientCurrentJumps");
	MarkNativeAsOptional("Timer_GetClientCurrentStrafes");
	MarkNativeAsOptional("Timer_GetClientCurrentSync");
	MarkNativeAsOptional("Timer_GetClientCurrentStrafeTime");
	MarkNativeAsOptional("Timer_GetClientCurrentSSj");
	MarkNativeAsOptional("Timer_GetClientTimerStatus");
	MarkNativeAsOptional("Timer_SetClientStyle");
	MarkNativeAsOptional("Timer_GetClientStyle");
	MarkNativeAsOptional("Timer_GetClientTimerData");
	MarkNativeAsOptional("Timer_SetClientTimerData");
	MarkNativeAsOptional("Timer_GetStyleCount");
	MarkNativeAsOptional("Timer_GetStyleSettings");
	MarkNativeAsOptional("Timer_GetStyleName");
	MarkNativeAsOptional("Timer_GetStylePrefix");
	MarkNativeAsOptional("Timer_StyleHasSetting");
	MarkNativeAsOptional("Timer_IsTimerRunning");
	MarkNativeAsOptional("Timer_StopTimer");
	MarkNativeAsOptional("Timer_BlockTimer");
	
	// records
	MarkNativeAsOptional("Timer_IsClientLoaded");
	MarkNativeAsOptional("Timer_GetClientMapRank");
	MarkNativeAsOptional("Timer_GetClientPBTime");
	MarkNativeAsOptional("Timer_GetWRTime");
	MarkNativeAsOptional("Timer_GetWRName");
	MarkNativeAsOptional("Timer_GetRecordsCount");
	MarkNativeAsOptional("Timer_CallOnRecordDeleted");
	
	// zones
	MarkNativeAsOptional("Timer_GetClientZoneTrack");
	MarkNativeAsOptional("Timer_GetClientZoneType");
	MarkNativeAsOptional("Timer_TeleportClientToZone");
	MarkNativeAsOptional("Timer_IsClientInsideZone");
	
	// replays
	MarkNativeAsOptional("Timer_GetReplayRecordId");
	MarkNativeAsOptional("Timer_GetReplayTime");
	MarkNativeAsOptional("Timer_GetReplayBotTrack");
	MarkNativeAsOptional("Timer_GetReplayBotStyle");
	MarkNativeAsOptional("Timer_GetReplayBotType");
	MarkNativeAsOptional("Timer_GetReplayBotCurrentFrame");
	MarkNativeAsOptional("Timer_GetReplayBotTotalFrames");
	MarkNativeAsOptional("Timer_GetReplayBotPlayerName");
	MarkNativeAsOptional("Timer_SetReplayBotPlayerName");
	MarkNativeAsOptional("Timer_GetClientReplayFrames");
	MarkNativeAsOptional("Timer_SetClientReplayFrames");
	
	// chat
	MarkNativeAsOptional("Timer_PrintToChat");
	MarkNativeAsOptional("Timer_PrintToChatAll");
	MarkNativeAsOptional("Timer_PrintToAdmins");
	MarkNativeAsOptional("Timer_ReplyToCommand");
	
	// cp
	MarkNativeAsOptional("Timer_OpenCheckpointsMenu");
	MarkNativeAsOptional("Timer_GetTotalCheckpoints");
	MarkNativeAsOptional("Timer_GetClientCheckpoint");
	MarkNativeAsOptional("Timer_SetClientCheckpoint");
	MarkNativeAsOptional("Timer_TeleportClientToCheckpoint");
	MarkNativeAsOptional("Timer_ClearClientCheckpoints");
	
	// tagteam
	MarkNativeAsOptional("Timer_IsClientInTagTeam");
	MarkNativeAsOptional("Timer_GetClientTeamIndex");
	MarkNativeAsOptional("Timer_GetTeamName");
}
#endif