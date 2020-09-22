#if defined _CKSurf_included
 #endinput
#endif
#define _CKSurf_included

enum RunType
{
	RT_Map,
	RT_Stage,
	RT_Bonus
}



/* Natives */

/**
 * Checks clients timer status
 *
 * @param client    Client's id
 * @return          True / False
 */
native bool ckSurf_GetTimerStatus(int client);

/**
 * Stops client's timer.
 *
 * @param client    Client's id
 * @return          No return
 */
native void ckSurf_StopTimer(int client);

/**
 * Starts client's timer
 *
 * @param client    Client's id
 * @return          No return
 */
native void ckSurf_EmulateStartButtonPress(int client);

/**
 * Finishes a client's run.
 *
 * @param client    Client's id
 * @return          No return
 */
native void ckSurf_EmulateStopButtonPress(int client);

/**
 * Get client's current runtime.
 *
 * @param client     Client's id
 * @return           Client's curren runtime as a float.
 */
native float ckSurf_GetCurrentTime(int client);

/**
 * Checks if the client has VIP status
 *
 * @param client     Client's id
 * @return           True / False
 */
native bool ckSurf_ClientIsVIP(int client);

/**
 * Gets the server rank of a client
 *
 * @param client     Client's id
 * @return           Server rank as an int
 */
native void ckSurf_GetServerRank(int client);

/**
 * Safely teleports the client. Does not trigger end touches
 * and sets client location correctly.
 *
 * @param client    		Client's id
 * @param fDestination 		Teleport location
 * @param fAngle 			Teleport angle
 * @param fVelocity 		Teleport velocity
 * @param bStopTimer 		Stop the client's timer, default false
 * @return           true on success / false on failure
 */

native bool ckSurf_SafeTeleport(int client, float fDestination[3], float fAngle[3], float fVelocity[3], bool bStopTimer = false);

/**
 * Gets zone type count for a zonegroup
 *
 * @param zoneGroup    		ZoneGroup
 * @param type 		 Type to be counted
 * @return           zone type count for zone group (int)
 */
native int ckSurf_CountZones(int zonegroup, int type);

/**
 * Gets ammount of zone groups in current map
 *
 * @return           Zone group count (int)
 */
native int ckSurf_CountZoneGroups();

/**
 * Gets points for a client id
 *
 * @param client    		Client's id
 * @return           Player points
 */
native int ckSurf_GetPlayerPoints(int client);


/*
		FORWARDS
						*/

/**
 * Called when a player finishes a map
 *
 * @param client     Player index.
 * @return           Plugin_Handled; to stop the player from searching.
 */
forward Action ckSurf_OnMapFinished(int client, float fRunTime, char RunTime[54], int rank, int total, bool improved);

/**
 * Called when a player finishes a bonus
 *
 * @param client     Player index.
 * @return           Plugin_Handled; to stop the player from searching.
 */
forward Action ckSurf_OnBonusFinished(int client, float fRunTime, char sRunTime[54], int rank, int total, int bonusid, bool improved);

/**
 * Called when a player finishes a map in practice mode
 *
 * @param client     Player index.
 * @return           Plugin_Handled; to stop the player from searching.
 */
 forward Action ckSurf_OnPracticeFinished(int client, float fRunTime, char sRunTime[54]);


 /**
  * Called when a player finishes a stage
  *
  * @param client 	Player index.
  * @param fRuntime Run time in seconds
  * @param sRunTime Run time formated as a string
  * @param stage 	Stage
  * @param rank 	Rank of the run
  */
forward void ckSurf_OnStageFinished(int client, float fRuntime, char[] sRuntime, int stage, int rank);



/**
 * Called when a player leaves a start zone
 *
 * @param client 	Player index
 * @param runtype	Type of the run player is trying to start
 * @return 			Plugin_Handled to prevent timer from starting
 */
forward Action ckSurf_OnTimerStarted(int client, RunType runtype);


public SharedPlugin __pl_ckSurf =
{
	name = "ckSurf",
	file = "ckSurf.smx",
	#if defined REQUIRE_PLUGIN
	required = 1,
	#else
	required = 0,
	#endif
};

public void __pl_ckSurf_SetNTVOptional()
{
	MarkNativeAsOptional("ckSurf_GetTimerStatus");
	MarkNativeAsOptional("ckSurf_StopTimer");
	MarkNativeAsOptional("ckSurf_EmulateStartButtonPress");
	MarkNativeAsOptional("ckSurf_EmulateStopButtonPress");
	MarkNativeAsOptional("ckSurf_GetCurrentTime");
	MarkNativeAsOptional("ckSurf_ClientIsVIP");
	MarkNativeAsOptional("ckSurf_GetServerRank");
	MarkNativeAsOptional("ckSurf_SafeTeleport");
}
