// MOAR COLORS
// By Dr. McKay
// Inspired by: https://forums.alliedmods.net/showthread.php?t=96831

#if defined _colors_included
 #endinput
#endif
#define _colors_included

#include <regex>

#define MORE_COLORS_VERSION		"1.9.1"
#define MAX_MESSAGE_LENGTH		256
#define MAX_BUFFER_LENGTH		(MAX_MESSAGE_LENGTH * 4)

#define COLOR_RED				0xFF4040
#define COLOR_BLUE				0x99CCFF
#define COLOR_GRAY				0xCCCCCC
#define COLOR_GREEN				0x3EFF3E

#define GAME_DODS				0

new bool:CSkipList[MAXPLAYERS + 1];
new Handle:CTrie;
new CTeamColors[][] = {{0xCCCCCC, 0x4D7942, 0xFF4040}}; // Multi-dimensional array for games that don't support SayText2. First index is the game index (as defined by the GAME_ defines), second index is team. 0 = spectator, 1 = team1, 2 = team2

/**
 * Prints a message to a specific client in the chat area.
 * Supports color tags.
 * 
 * @param client		Client index.
 * @param message		Message (formatting rules).
 * @noreturn
 * 
 * On error/Errors:		If the client is not connected an error will be thrown.
 */
stock CPrintToChat(client, const String:message[], any:...) {
	CCheckTrie();
	if(client <= 0 || client > MaxClients) {
		ThrowError("Invalid client index %i", client);
	}
	if(!IsClientInGame(client)) {
		ThrowError("Client %i is not in game", client);
	}
	decl String:buffer[MAX_BUFFER_LENGTH], String:buffer2[MAX_BUFFER_LENGTH];
	SetGlobalTransTarget(client);
	Format(buffer, sizeof(buffer), "\x01%s", message);
	VFormat(buffer2, sizeof(buffer2), buffer, 3);
	CReplaceColorCodes(buffer2);
	CSendMessage(client, buffer2);
}

/**
 * Prints a message to all clients in the chat area.
 * Supports color tags.
 * 
 * @param client		Client index.
 * @param message		Message (formatting rules).
 * @noreturn
 */
stock CPrintToChatAll(const String:message[], any:...) {
	CCheckTrie();
	decl String:buffer[MAX_BUFFER_LENGTH], String:buffer2[MAX_BUFFER_LENGTH];
	for(new i = 1; i <= MaxClients; i++) {
		if(!IsClientInGame(i) || CSkipList[i]) {
			CSkipList[i] = false;
			continue;
		}
		SetGlobalTransTarget(i);
		Format(buffer, sizeof(buffer), "\x01%s", message);
		VFormat(buffer2, sizeof(buffer2), buffer, 2);
		CReplaceColorCodes(buffer2);
		CSendMessage(i, buffer2);
	}
}

/**
 * Prints a message to a specific client in the chat area.
 * Supports color tags and teamcolor tag.
 * 
 * @param client		Client index.
 * @param author		Author index whose color will be used for teamcolor tag.
 * @param message		Message (formatting rules).
 * @noreturn
 * 
 * On error/Errors:		If the client or author are not connected an error will be thrown
 */
stock CPrintToChatEx(client, author, const String:message[], any:...) {
	CCheckTrie();
	if(client <= 0 || client > MaxClients) {
		ThrowError("Invalid client index %i", client);
	}
	if(!IsClientInGame(client)) {
		ThrowError("Client %i is not in game", client);
	}
	if(author <= 0 || author > MaxClients) {
		ThrowError("Invalid client index %i", author);
	}
	if(!IsClientInGame(author)) {
		ThrowError("Client %i is not in game", author);
	}
	decl String:buffer[MAX_BUFFER_LENGTH], String:buffer2[MAX_BUFFER_LENGTH];
	SetGlobalTransTarget(client);
	Format(buffer, sizeof(buffer), "\x01%s", message);
	VFormat(buffer2, sizeof(buffer2), buffer, 4);
	CReplaceColorCodes(buffer2, author);
	CSendMessage(client, buffer2, author);
}

/**
 * Prints a message to all clients in the chat area.
 * Supports color tags and teamcolor tag.
 *
 * @param author	  Author index whose color will be used for teamcolor tag.
 * @param message   Message (formatting rules).
 * @noreturn
 * 
 * On error/Errors:   If the author is not connected an error will be thrown.
 */
stock CPrintToChatAllEx(author, const String:message[], any:...) {
	CCheckTrie();
	if(author <= 0 || author > MaxClients) {
		ThrowError("Invalid client index %i", author);
	}
	if(!IsClientInGame(author)) {
		ThrowError("Client %i is not in game", author);
	}
	decl String:buffer[MAX_BUFFER_LENGTH], String:buffer2[MAX_BUFFER_LENGTH];
	for(new i = 1; i <= MaxClients; i++) {
		if(!IsClientInGame(i) || CSkipList[i]) {
			CSkipList[i] = false;
			continue;
		}
		SetGlobalTransTarget(i);
		Format(buffer, sizeof(buffer), "\x01%s", message);
		VFormat(buffer2, sizeof(buffer2), buffer, 3);
		CReplaceColorCodes(buffer2, author);
		CSendMessage(i, buffer2, author);
	}
}

/**
 * Sends a SayText2 usermessage
 * 
 * @param client	Client to send usermessage to
 * @param message	Message to send
 * @noreturn
 */
stock CSendMessage(client, const String:message[], author=0) {
	if(author == 0) {
		author = client;
	}
	decl String:buffer[MAX_MESSAGE_LENGTH], String:game[16];
	GetGameFolderName(game, sizeof(game));
	strcopy(buffer, sizeof(buffer), message);
	new UserMsg:index = GetUserMessageId("SayText2");
	if(index == INVALID_MESSAGE_ID) {
		if(StrEqual(game, "dod")) {
			new team = GetClientTeam(author);
			if(team == 0) {
				ReplaceString(buffer, sizeof(buffer), "\x03", "\x04", false); // Unassigned gets green
			} else {
				decl String:temp[16];
				Format(temp, sizeof(temp), "\x07%06X", CTeamColors[GAME_DODS][team - 1]);
				ReplaceString(buffer, sizeof(buffer), "\x03", temp, false);
			}
		}
		PrintToChat(client, "%s", buffer);
		return;
	}
	new Handle:buf = StartMessageOne("SayText2", client, USERMSG_RELIABLE|USERMSG_BLOCKHOOKS);
	if(GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available && GetUserMessageType() == UM_Protobuf) {
		PbSetInt(buf, "ent_idx", author);
		PbSetBool(buf, "chat", true);
		PbSetString(buf, "msg_name", buffer);
		PbAddString(buf, "params", "");
		PbAddString(buf, "params", "");
		PbAddString(buf, "params", "");
		PbAddString(buf, "params", "");
	} else {
		BfWriteByte(buf, author); // Message author
		BfWriteByte(buf, true); // Chat message
		BfWriteString(buf, buffer); // Message text
	}
	EndMessage();
}

/**
 * This function should only be used right in front of
 * CPrintToChatAll or CPrintToChatAllEx. It causes those functions
 * to skip the specified client when printing the message.
 * After printing the message, the client will no longer be skipped.
 * 
 * @param client   Client index
 * @noreturn
 */
stock CSkipNextClient(client) {
	if(client <= 0 || client > MaxClients) {
		ThrowError("Invalid client index %i", client);
	}
	CSkipList[client] = true;
}

/**
 * Checks if the colors trie is initialized and initializes it if it's not (used internally)
 * 
 * @return			No return
 */
stock CCheckTrie() {
	if(CTrie == INVALID_HANDLE) {
		CTrie = InitColorTrie();
	}
}

/**
 * Replaces color tags in a string with color codes (used internally by CPrintToChat, CPrintToChatAll, CPrintToChatEx, and CPrintToChatAllEx
 *
 * @param buffer		String.
 * @param author		Optional client index to use for {teamcolor} tags, or 0 for none
 * @param removeTags	Optional boolean value to determine whether we're replacing tags with colors, or just removing tags, used by CRemoveTags
 * @param maxlen		Optional value for max buffer length, used by CRemoveTags
 * @noreturn
 * 
 * On error/Errors:		If the client index passed for author is invalid or not in game.
 */
stock CReplaceColorCodes(String:buffer[], author=0, bool:removeTags=false, maxlen=MAX_BUFFER_LENGTH) {
	CCheckTrie();
	if(!removeTags) {
		ReplaceString(buffer, maxlen, "{default}", "\x01", false);
	} else {
		ReplaceString(buffer, maxlen, "{default}", "", false);
		ReplaceString(buffer, maxlen, "{teamcolor}", "", false);
	}
	if(author != 0 && !removeTags) {
		if(author < 0 || author > MaxClients) {
			ThrowError("Invalid client index %i", author);
		}
		if(!IsClientInGame(author)) {
			ThrowError("Client %i is not in game", author);
		}
		ReplaceString(buffer, maxlen, "{teamcolor}", "\x03", false);
	}
	new cursor = 0;
	new value;
	decl String:tag[32], String:buff[32], String:output[maxlen];
	strcopy(output, maxlen, buffer);
	// Since the string's size is going to be changing, output will hold the replaced string and we'll search buffer
	
	new Handle:regex = CompileRegex("{[a-zA-Z0-9]+}");
	for(new i = 0; i < 1000; i++) { // The RegEx extension is quite flaky, so we have to loop here :/. This loop is supposed to be infinite and broken by return, but conditions have been added to be safe.
		if(MatchRegex(regex, buffer[cursor]) < 1) {
			CloseHandle(regex);
			strcopy(buffer, maxlen, output);
			return;
		}
		GetRegexSubString(regex, 0, tag, sizeof(tag));
		CStrToLower(tag);
		cursor = StrContains(buffer[cursor], tag, false) + cursor + 1;
		strcopy(buff, sizeof(buff), tag);
		ReplaceString(buff, sizeof(buff), "{", "");
		ReplaceString(buff, sizeof(buff), "}", "");
		
		if(!GetTrieValue(CTrie, buff, value)) {
			continue;
		}
		
		if(removeTags) {
			ReplaceString(output, maxlen, tag, "", false);
		} else {
			Format(buff, sizeof(buff), "\x07%06X", value);
			ReplaceString(output, maxlen, tag, buff, false);
		}
	}
	LogError("[MORE COLORS] Infinite loop broken.");
}

/**
 * Gets a part of a string
 * 
 * @param input			String to get the part from
 * @param output		Buffer to write to
 * @param maxlen		Max length of output buffer
 * @param start			Position to start at
 * @param numChars		Number of characters to return, or 0 for the end of the string
 * @noreturn
 */
stock CSubString(const String:input[], String:output[], maxlen, start, numChars=0) {
	new i = 0;
	for(;;) {
		if(i == maxlen - 1 || i >= numChars || input[start + i] == '\0') {
			output[i] = '\0';
			return;
		}
		output[i] = input[start + i];
		i++;
	}
}

/**
 * Converts a string to lowercase
 * 
 * @param buffer		String to convert
 * @noreturn
 */
stock CStrToLower(String:buffer[]) {
	new len = strlen(buffer);
	for(new i = 0; i < len; i++) {
		buffer[i] = CharToLower(buffer[i]);
	}
}

/**
 * Adds a color to the colors trie
 *
 * @param name			Color name, without braces
 * @param color			Hexadecimal representation of the color (0xRRGGBB)
 * @return				True if color was added successfully, false if a color already exists with that name
 */
stock bool:CAddColor(const String:name[], color) {
	CCheckTrie();
	new value;
	if(GetTrieValue(CTrie, name, value)) {
		return false;
	}
	decl String:newName[64];
	strcopy(newName, sizeof(newName), name);
	CStrToLower(newName);
	SetTrieValue(CTrie, newName, color);
	return true;
}

/**
 * Removes color tags from a message
 * 
 * @param message		Message to remove tags from
 * @param maxlen		Maximum buffer length
 * @noreturn
 */
stock CRemoveTags(String:message[], maxlen) {
	CReplaceColorCodes(message, 0, true, maxlen);
}

/**
 * Replies to a command with colors
 * 
 * @param client		Client to reply to
 * @param message		Message (formatting rules)
 * @noreturn
 */
stock CReplyToCommand(client, const String:message[], any:...) {
	decl String:buffer[MAX_BUFFER_LENGTH];
	SetGlobalTransTarget(client);
	VFormat(buffer, sizeof(buffer), message, 3);
	if(GetCmdReplySource() == SM_REPLY_TO_CONSOLE) {
		CRemoveTags(buffer, sizeof(buffer));
		PrintToConsole(client, "%s", buffer);
	} else {
		CPrintToChat(client, "%s", buffer);
	}
}

/**
 * Replies to a command with colors
 * 
 * @param client		Client to reply to
 * @param author		Client to use for {teamcolor}
 * @param message		Message (formatting rules)
 * @noreturn
 */
stock CReplyToCommandEx(client, author, const String:message[], any:...) {
	decl String:buffer[MAX_BUFFER_LENGTH];
	SetGlobalTransTarget(client);
	VFormat(buffer, sizeof(buffer), message, 4);
	if(GetCmdReplySource() == SM_REPLY_TO_CONSOLE) {
		CRemoveTags(buffer, sizeof(buffer));
		PrintToConsole(client, "%s", buffer);
	} else {
		CPrintToChatEx(client, author, "%s", buffer);
	}
}

/**
 * Shows admin activity with colors
 * 
 * @param client		Client performing an action
 * @param message		Message (formatting rules)
 * @noreturn
 */
stock CShowActivity(client, const String:message[], any:...) {
	CCheckTrie();
	if(client < 0 || client > MaxClients) {
		ThrowError("Invalid client index %d", client);
	}
	if(client != 0 && !IsClientInGame(client)) {
		ThrowError("Client %d is not in game", client);
	}
	decl String:buffer[MAX_BUFFER_LENGTH], String:buffer2[MAX_BUFFER_LENGTH];
	Format(buffer, sizeof(buffer), "\x01%s", message);
	VFormat(buffer2, sizeof(buffer2), buffer, 3);
	CReplaceColorCodes(buffer2);
	ShowActivity(client, "%s", buffer2);
}

/**
 * Shows admin activity with colors
 * 
 * @param client		Client performing an action
 * @param tag			Tag to prepend to the message (color tags supported)
 * @param message		Message (formatting rules)
 * @noreturn
 */
stock CShowActivityEx(client, const String:tag[], const String:message[], any:...) {
	CCheckTrie();
	if(client < 0 || client > MaxClients) {
		ThrowError("Invalid client index %d", client);
	}
	if(client != 0 && !IsClientInGame(client)) {
		ThrowError("Client %d is not in game", client);
	}
	decl String:buffer[MAX_BUFFER_LENGTH], String:buffer2[MAX_BUFFER_LENGTH];
	Format(buffer, sizeof(buffer), "\x01%s", message);
	VFormat(buffer2, sizeof(buffer2), buffer, 4);
	CReplaceColorCodes(buffer2);
	strcopy(buffer, sizeof(buffer), tag);
	CReplaceColorCodes(buffer);
	ShowActivityEx(client, tag, "%s", buffer2);
}

/**
 * Shows admin activity with colors
 * 
 * @param client		Client performing an action
 * @param tag			Tag to prepend to the message (color tags supported)
 * @param message		Message (formatting rules)
 * @noreturn
 */
stock CShowActivity2(client, const String:tag[], const String:message[], any:...) {
	CCheckTrie();
	if(client < 0 || client > MaxClients) {
		ThrowError("Invalid client index %d", client);
	}
	if(client != 0 && !IsClientInGame(client)) {
		ThrowError("Client %d is not in game", client);
	}
	decl String:buffer[MAX_BUFFER_LENGTH], String:buffer2[MAX_BUFFER_LENGTH];
	Format(buffer, sizeof(buffer), "\x01%s", message);
	VFormat(buffer2, sizeof(buffer2), buffer, 4);
	CReplaceColorCodes(buffer2);
	strcopy(buffer, sizeof(buffer), tag);
	CReplaceColorCodes(buffer);
	ShowActivity2(client, buffer, "%s", buffer2);
}

/**
 * Determines whether a color name exists
 * 
 * @param color			The color name to check
 * @return				True if the color exists, false otherwise
 */
stock bool:CColorExists(const String:color[]) {
	CCheckTrie();
	new temp;
	return GetTrieValue(CTrie, color, temp);
}

/**
 * Returns the hexadecimal representation of a client's team color (will NOT initialize the trie)
 *
 * @param client		Client to get the team color for
 * @return				Client's team color in hexadecimal, or green if unknown
 * On error/Errors:		If the client index passed is invalid or not in game.
 */
stock CGetTeamColor(client) {
	if(client <= 0 || client > MaxClients) {
		ThrowError("Invalid client index %i", client);
	}
	if(!IsClientInGame(client)) {
		ThrowError("Client %i is not in game", client);
	}
	new value;
	switch(GetClientTeam(client)) {
		case 1: {
			value = COLOR_GRAY;
		}
		case 2: {
			value = COLOR_RED;
		}
		case 3: {
			value = COLOR_BLUE;
		}
		default: {
			value = COLOR_GREEN;
		}
	}
	return value;
}

stock Handle:InitColorTrie() {
	new Handle:hTrie = CreateTrie();
	SetTrieValue(hTrie, "prefix", 0x5e70d0);
	SetTrieValue(hTrie, "text", 0xffffff);
	SetTrieValue(hTrie, "variable", 0x7fd772);
	return hTrie;
}