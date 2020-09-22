/*
* Simple Chat colors Include by PaxPlay
* 
* Credits to shavit.
* Supported Games: CSGO, CSS
*/
#if defined _SCC_PAX_included
 #endinput
#endif
#define _SCC_PAX_included

char gS_CSSRGBColorNames[][] =
{
	"{RGB}",	// this string must follow a hex color code (i.e. {RGB}00ff00 for green)
	"{RGBA}"	// same but with alpha channel
};

char gS_CSSRGBColors[][] =
{
	"\x07",	// this string must follow a hex color code (i.e. {RGB}00ff00 for green)
	"\x08"	// same but with alpha channel
};

char gS_GlobalColorNames[][] =
{
	"{default}",
	"{team}",
	"{green}"
};

char gS_GlobalColors[][] =
{
	"\x01",
	"\x03",
	"\x04"
};

char gS_CSGOColorNames[][] =
{
	"{blue}",
	"{bluegrey}",
	"{darkblue}",
	"{darkred}",
	"{gold}",
	"{grey}",
	"{grey2}",
	"{lightgreen}",
	"{lightred}",
	"{lime}",
	"{orchid}",
	"{palered}",
	"{yellow}"
};

char gS_CSGOColors[][] =
{
	"\x0B",
	"\x0A",
	"\x0C",
	"\x02",
	"\x10",
	"\x08",
	"\x0D",
	"\x05",
	"\x0F",
	"\x06",
	"\x0E",
	"\x07",
	"\x09"
};

/**
* Replaces chat color strings between "{}" with the color belonging to it.
*
* @param buffer			Buffer String.
* @param maxlength		Length of buffer.
*
*/
stock void SCC_ReplaceColors(char[] buffer, int maxlength)
{
	EngineVersion gEV_Type = GetEngineVersion();
	
	for(int i = 0; i < sizeof(gS_GlobalColorNames); i++)
	{
		ReplaceString(buffer, maxlength, gS_GlobalColorNames[i], gS_GlobalColors[i]);
	}
	
	if (gEV_Type == Engine_CSS)
	{
		for(int i = 0; i < sizeof(gS_CSSRGBColorNames); i++)
		{
			ReplaceString(buffer, maxlength, gS_CSSRGBColorNames[i], gS_CSSRGBColors[i]);
		}
	}
	else // Remove Tags anyways
	{
		for(int i = 0; i < sizeof(gS_CSSRGBColorNames); i++)
		{
			if (ReplaceString(buffer, maxlength, gS_CSSRGBColorNames[i], "") > 0)
				LogMessage("\"%s\" is not supported in this game.", gS_CSSRGBColorNames[i]);
		}
	}
	
	if (gEV_Type == Engine_CSGO)
	{
		for(int i = 0; i < sizeof(gS_CSGOColorNames); i++)
		{
			ReplaceString(buffer, maxlength, gS_CSGOColorNames[i], gS_CSGOColors[i]);
		}
	}
	else // Remove Tags anyways
	{
		for(int i = 0; i < sizeof(gS_CSGOColorNames); i++)
		{
			if (ReplaceString(buffer, maxlength, gS_CSGOColorNames[i], "") > 0)
				LogMessage("\"%s\" is not supported in this game.", gS_CSGOColorNames[i]);
		}
	}
}

/**
* Removes any chat colors from a string.
*
* @param buffer			Buffer String.
* @param maxlength		Length of buffer.
*
*/
stock void SCC_RemoveColors(char[] buffer, int maxlength)
{
	for(int i = 0; i < sizeof(gS_GlobalColors); i++)	// no need to check Engine, just remove everything, no need to remove css, since \x07 and \x08 are in csgo aswell
	{
		ReplaceString(buffer, maxlength, gS_GlobalColors[i], "");
	}
	for(int i = 0; i < sizeof(gS_CSGOColors); i++)
	{
		ReplaceString(buffer, maxlength, gS_CSGOColors[i], "");
	}
	
}

/**
* Prints a message to a specific client in the chat area. Replaces chat color strings between "{}" with the color belonging to it.
*
* @param client		client index.
* @param format		Formatting rules.
* @param ...		Variable number of format parameters.
*
*/
stock void SCC_PrintToChat(client, const char[] format, any ...)
{
	char buffer[254];
	VFormat(buffer, sizeof(buffer), format, 3);
	SCC_ReplaceColors(buffer, sizeof(buffer));
	PrintToChat(client, "%s", buffer);
}

/**
* Prints a message to all clients in the chat area.Replaces chat color strings between "{}" with the color belonging to it.
*
* @param format		Formatting rules.
* @param ...		Variable number of format parameters.
*
*/
stock void SCC_PrintToChatAll(const char[] format, any ...)
{
	char buffer[254];
	VFormat(buffer, sizeof(buffer), format, 3);
	SCC_ReplaceColors(buffer, sizeof(buffer));
	PrintToChatAll("%s", buffer);
}