#include <sourcemod>
#include <regex>
#include <output_info_plugin>

#define PLUGIN_VERSION 3

public Plugin myinfo =
{
	name = "Output Info Plugin",
	author = "KiD Fearless",
	description = "Plugin Alternative To Output Info",
	version = "2.0.3",
	url = "https://github.com/kidfearless"
}

#pragma dynamic 1048576
#pragma semicolon 1

ArrayList gA_Entites;
StringMap gSM_EntityList;
GlobalForward gF_OnEntitiesReady;

bool gB_Ready;

#if PLUGIN_VERSION != INCLUDE_VERSION
// Closest thing to a compile time error that we can get
// The include file for this plugin contains code that may need to be updated.
Please__update__your__include__to__match__plugins__version
#endif

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	#if USE_NATIVES == 1
	// current natives
	if(GetFeatureStatus(FeatureType_Native, "GetOutputCount") != FeatureStatus_Available)
	{
		CreateNative("GetOutputCount", Native_GetOutputCount);
		CreateNative("GetOutputTarget", Native_GetOutputTarget);
		CreateNative("GetOutputTargetInput", Native_GetOutputTargetInput);
		CreateNative("GetOutputParameter", Native_GetOutputParameter);
		CreateNative("GetOutputDelay", Native_GetOutputDelay);
	}
	
	// old natives
	if(GetFeatureStatus(FeatureType_Native, "GetOutputActionCount") != FeatureStatus_Available)
	{
		CreateNative("GetOutputActionCount", Native_GetOutputCount);
		CreateNative("GetOutputActionTarget", Native_GetOutputTarget);
		CreateNative("GetOutputActionTargetInput", Native_GetOutputTargetInput);
		CreateNative("GetOutputActionParameter", Native_GetOutputParameter);
		CreateNative("GetOutputActionDelay", Native_GetOutputDelay);
	}
	#endif

	CreateNative("GetOutputEntity", Native_GetOutputEntity);
	CreateNative("GetOutputEntities", Native_GetOutputEntities);
	CreateNative("AreEntitiesReady", Native_AreEntitiesReady);

	RegPluginLibrary("output_info_plugin");

	if(late)
	{
		gB_Ready = false;
	}

	return APLRes_Success;
}

public void OnPluginStart()
{
	gA_Entites = new ArrayList(sizeof(Entity));
	gSM_EntityList = new StringMap();

	gF_OnEntitiesReady = new GlobalForward("OnEntitiesReady", ET_Event);
}

public Action OnLevelInit(const char[] mapName, char mapEntities[2097152])
{
	gB_Ready = false;

	for(int i = 0; i < gA_Entites.Length; ++i)
	{
		Entity e;
		gA_Entites.GetArray(i, e);
		e.CleanUp();
	}

	gA_Entites.Clear();
	gSM_EntityList.Clear();

	for(int current = 0, next = 0; (next = FindNextKeyChar(mapEntities[current], '}')) != -1; current += next)
	{
		char[] entity = new char[next+1];
		strcopy(entity, next, mapEntities[current]);

		Entity ent;
		ent.Parse(entity);

		if(ent.OutputList.Length > 0)
		{
			// associate the index with the entities hammerid
			int index = gA_Entites.PushArray(ent);
			gSM_EntityList.SetValue(ent.HammerID, index);
		}
		else
		{
			ent.CleanUp();
		}
	}
	
	gB_Ready = true;
	Call_StartForward(gF_OnEntitiesReady);
	Call_Finish();

	return Plugin_Continue;
}

public void OnMapEnd()
{
	gB_Ready = false;

	for(int i = 0; i < gA_Entites.Length; ++i)
	{
		Entity e;
		gA_Entites.GetArray(i, e);
		e.CleanUp();
	}

	gA_Entites.Clear();
	gSM_EntityList.Clear();
}

bool LocalGetOutputEntity(int index, Entity ent)
{
	if(!gB_Ready)
	{
		return false;
	}
	if(gA_Entites.Length < 1 || gSM_EntityList.Size < 1)
	{
		return false;
	}

	int hammer = GetHammerFromIndex(index);
	char id[MEMBER_SIZE];
	IntToString(hammer, id, MEMBER_SIZE);

	int position = -1;
	if(!gSM_EntityList.GetValue(id, position))
	{
		// LogError("Could not find entity with the index '%i', hammmerid '%i'.", index, hammer);
		return false;
	}

	if(position >= gA_Entites.Length || position < 0)
	{
		// LogError("List position out of range");
		return false;
	}

	gA_Entites.GetArray(position, ent);

	return (ent.OutputList != null);
}

// native bool GetOutputEntity(int index, any[] entity);
public any Native_GetOutputEntity(Handle plugin, int numParams)
{
	if(!gB_Ready)
	{
		return false;
	}
	if(gA_Entites.Length < 1 || gSM_EntityList.Size < 1)
	{
		return false;
	}

	int index = GetNativeCell(1);
	int hammer = GetHammerFromIndex(index);
	char id[MEMBER_SIZE];
	IntToString(hammer, id, MEMBER_SIZE);

	int position = -1;
	if(!gSM_EntityList.GetValue(id, position))
	{
		//LogError("Could not find entity with with the index '%i', hammmerid '%i'.", index, hammer);
		return false;
	}

	if(position >= gA_Entites.Length || position < 0)
	{
		//LogError( "List position out of range");
		return false;
	}

	Entity temp;
	gA_Entites.GetArray(position, temp);

	Entity ent;
	CloneEntity(temp, ent, plugin);
	SetNativeArray(2, ent, sizeof(Entity));

	return (ent.OutputList != null);
}

// native ArrayList GetOutputEntities();
public any Native_GetOutputEntities(Handle plugin, int numParams)
{
	if(!gB_Ready)
	{
		//LogError("Native called before dump file has been processed.");
		return INVALID_HANDLE;
	}
	if(gA_Entites.Length < 1 || gSM_EntityList.Size < 1)
	{
		//LogError("Entity lists are empty.");
		return INVALID_HANDLE;
	}

	ArrayList temp = new ArrayList(sizeof(Entity));

	ArrayList list = view_as<ArrayList>(CloneHandle(temp, plugin));
	delete temp;

	for(int i = 0; i < gA_Entites.Length; ++i)
	{
		Entity original;
		gA_Entites.GetArray(i, original);

		Entity cloned;
		CloneEntity(original, cloned, plugin);

		list.PushArray(cloned);
	}

	return list;
}

// native bool AreEntitiesReady();
public any Native_AreEntitiesReady(Handle plugin, int numParams)
{
	return gB_Ready;
}

#if USE_NATIVES == 1

// native int GetOutputCount(int index, const char[] output = "");
// native int GetActionOutputCount(int index, const char[] output = "");
public any Native_GetOutputCount(Handle plugin, int numParams)
{
	int index = GetNativeCell(1);
	Entity ent;

	if(!LocalGetOutputEntity(index, ent))
	{
		return -1;
	}

	int count = 1;
	char buffer[MEMBER_SIZE];
	GetNativeString(2, buffer, MEMBER_SIZE);

	if(buffer[0] == 0)
	{
		return ent.OutputList.Length;
	}

	char output[MEMBER_SIZE];
	if(StrContains(buffer, "m_") == 0)
	{
		strcopy(output, MEMBER_SIZE, buffer[2]);
	}
	else
	{
		strcopy(output, MEMBER_SIZE, buffer);
	}

	for(int i = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(output, out.Output, false))
		{
			++count;
		}
	}

	// since we own the arraylist we don't need to clear the arraylist

	return count;
}

// native bool GetOutputTarget(int index, const char[] output, int num, char[] target, int length = MEMBER_SIZE);
public any Native_GetOutputTarget(Handle plugin, int numParams)
{
	int index = GetNativeCell(1);
	Entity ent;
	if(!LocalGetOutputEntity(index, ent))
	{
		return 0;
	}

	char output[MEMBER_SIZE];
	char buffer[MEMBER_SIZE];
	GetNativeString(2, buffer, MEMBER_SIZE);
	
	if(StrContains(buffer, "m_") == 0)
	{
		strcopy(output, MEMBER_SIZE, buffer[2]);
	}
	else
	{
		strcopy(output, MEMBER_SIZE, buffer);
	}
	
	int num = GetNativeCell(3);
	for(int i = 0, count = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(output, out.Output, false))
		{
			if(count++ == num)
			{
				int length = MEMBER_SIZE;
				if(numParams > 4)
				{
					length = GetNativeCell(5);
				}
				SetNativeString(4, out.Target, length);
				return true;
			}
		}
	}

	return false;
}

// native bool GetOutputTargetInput(int index, const char[] output, int num, char[] input, int length = MEMBER_SIZE);
public any Native_GetOutputTargetInput(Handle plugin, int numParams)
{
	int index = GetNativeCell(1);
	Entity ent;
	if(!LocalGetOutputEntity(index, ent))
	{
		return false;
	}

	char output[MEMBER_SIZE];
	char buffer[MEMBER_SIZE];
	GetNativeString(2, buffer, MEMBER_SIZE);
	
	if(StrContains(buffer, "m_") == 0)
	{
		strcopy(output, MEMBER_SIZE, buffer[2]);
	}
	else
	{
		strcopy(output, MEMBER_SIZE, buffer);
	}

	int num = GetNativeCell(3);
	for(int i = 0, count = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(output, out.Output, false))
		{
			if(count++ == num)
			{
				int length = MEMBER_SIZE;
				if(numParams > 4)
				{
					length = GetNativeCell(5);
				}
				SetNativeString(4, out.Input, length);
				return true;
			}
		}
	}

	return false;
}

// native bool GetOutputParameter(int index, const char[] output, int num, char[] parameters, int length = MEMBER_SIZE);
public any Native_GetOutputParameter(Handle plugin, int numParams)
{
	int index = GetNativeCell(1);
	Entity ent;
	if(!LocalGetOutputEntity(index, ent))
	{
		// LogError("Failed to get local output entity");
		return false;
	}

	char output[MEMBER_SIZE];
	char buffer[MEMBER_SIZE];
	GetNativeString(2, buffer, MEMBER_SIZE);
	
	if(StrContains(buffer, "m_") == 0)
	{
		strcopy(output, MEMBER_SIZE, buffer[2]);
	}
	else
	{
		strcopy(output, MEMBER_SIZE, buffer);
	}

	int num = GetNativeCell(3);
	for(int i = 0, count = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(output, out.Output, false))
		{
			if(count++ == num)
			{
				int length = MEMBER_SIZE;
				if(numParams > 4)
				{
					length = GetNativeCell(5);
				}
				SetNativeString(4, out.Parameters, length);
				return true;
			}
		}
	}

	return false;
}

// native float GetOutputDelay(int index, const char[] output, int num);
public any Native_GetOutputDelay(Handle plugin, int numParams)
{
	int index = GetNativeCell(1);
	Entity ent;
	if(!LocalGetOutputEntity(index, ent))
	{
		return -1.0;
	}
	
	char output[MEMBER_SIZE];
	char buffer[MEMBER_SIZE];
	GetNativeString(2, buffer, MEMBER_SIZE);
	
	if(StrContains(buffer, "m_") == 0)
	{
		strcopy(output, MEMBER_SIZE, buffer[2]);
	}
	else
	{
		strcopy(output, MEMBER_SIZE, buffer);
	}
	
	int num = GetNativeCell(3);
	for(int i = 0, count = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(buffer, out.Output, false))
		{
			if(count++ == num)
			{
				return out.Delay;
			}
		}
	}

	return -1.0;
}
#endif

stock int FindNextKeyChar(const char[] input, char key)
{
	int i;
	while(input[i] != key && input[i] != 0)
	{
		++i;
	}

	if(!input[i])
	{
		return -1;
	}

	return i+2;
}