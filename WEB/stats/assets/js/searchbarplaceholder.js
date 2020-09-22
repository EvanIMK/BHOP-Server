function steamidsearchtype(select) {
	var string = "";
	var out = "";
	if (select === 0) {
		string = "Player Name";
	} else if (select === 1) {
		string = "SteamID (ex. STEAM_1:0:11223344)";
	} else if (select === 2) {
		string = "SteamID3 (ex. [U:1:11223344] )";
	} else if (select === 3) {
		string = "SteamID64 (ex. 76561179760625728)";
	} else if (select === 4) {
		string = "Steamcommunity.com Profile URL (ex. http://www.steamcommunity.com/id/your_profile_name_here)";
	}		} else if (select === 5) {		string = "Player Name";	}
	out = document.getElementsByClassName('username-input');
	out[0].placeholder = string;
	return;
}