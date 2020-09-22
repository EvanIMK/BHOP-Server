#if defined _smc_included_
	#endinput
#endif
#define _smc_included_

/**
 * Called when a player RTV's.
 *
 * @param client					Client index.
 * @noreturn
 */
forward void SMC_OnRTV(int client);

/**
 * Called when a player UNRTV's.
 *
 * @param client					Client index.
 * @noreturn
 */
forward void SMC_OnUnRTV(int client);

/**
 * Called when the map changes from an RTV.
 *
 * @noreturn
 */
forward void SMC_OnSuccesfulRTV();

public SharedPlugin __pl_smc =
{
	name = "shavit - MapChooser",
	file = "shavit_mapchooser.smx",
	#if defined REQUIRE_PLUGIN
	required = 1,
	#else
	required = 0,
	#endif
};