#if defined _shavitchallenge_included_
	#endinput
#endif
#define _shavitchallenge_included_

/**
 * Checks client's freeze status
 *
 * @param client    Client index
 * @return          Boolean value
 */
native bool Challenge_IsClientFrozen(int client);

/**
 * Checks client's race status
 *
 * @param client    Client index
 * @return          Boolean value
 */
native bool Challenge_IsClientInRace(int client);

public SharedPlugin:__pl_shavitchallenge =
{
	name = "shavit-challenge",
	file = "shavit-challenge.smx",
	#if defined REQUIRE_PLUGIN
		required = 1
	#else
		required = 0
	#endif
};

public void __pl_shavitchallenge_SetNTVOptional()
{
	MarkNativeAsOptional("Challenge_IsClientFrozen");
	MarkNativeAsOptional("Challenge_IsClientInRace");
}