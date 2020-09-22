#if defined _output_info_plugin_defined_
#endinput
#endif
#define _output_info_plugin_defined_

#define OUTPUT_SIZE 2048
#define MEMBER_SIZE 128
#define INCLUDE_VERSION 3

// set to 0 to use stocks instead of native
#define USE_NATIVES 1

#include <regex>

enum
{
	TARGETENTITY,
	OUTPUTNAME,
	PARAMETERS,
	DELAY,
	ONCE,
	OUTPUTSIZE
};

enum struct Output
{
	char Output[MEMBER_SIZE];// OnStartTouch
	char Target[MEMBER_SIZE];// !Activator 
	char Input[MEMBER_SIZE];// Addoutput
	char Parameters[MEMBER_SIZE];// gravity 0.1
	float Delay;// 0.1
	bool Once;// 0

	void Dump()
	{
		PrintToServer("output: %s\ntarget: %s\ninput: %s\nParameters: %s\ndelay: %f\nonce: %i", this.Output, this.Target, this.Input, this.Parameters, this.Delay, this.Once);
	}
	
	void ToString(char[] input, int length = OUTPUT_SIZE)
	{
		Format(input, length, "%s;%s;%s;%s;%f;%i", this.Output, this.Target, this.Input, this.Parameters, this.Delay, this.Once);
	}

	void Parse(char[] buffer)
	{
		// Break it up into more managable parts
		char entity[OUTPUTSIZE][MEMBER_SIZE];
		ExplodeString(buffer, "\e", entity, OUTPUTSIZE, MEMBER_SIZE);

		this.Target = entity[TARGETENTITY];
		this.Input = entity[OUTPUTNAME];
		this.Parameters = entity[PARAMETERS];
		
		this.Delay = StringToFloat(entity[DELAY]);
		this.Once = (StringToInt(entity[ONCE]) > 0);
	}
}

enum struct Entity
{
	char HammerID[MEMBER_SIZE];
	float Wait;
	char Classname[MEMBER_SIZE];
	ArrayList OutputList;

	void Dump()
	{
		char outputs[OUTPUT_SIZE];
		for(int i = 0; i < this.OutputList.Length; ++i)
		{
			Output temp;
			this.OutputList.GetArray(i, temp);
			char buffer[OUTPUT_SIZE];
			temp.ToString(buffer);
			Format(outputs, OUTPUT_SIZE, "\t\t%s\n\t\t'%s'", outputs, buffer);
		}
		PrintToServer("\n\"%s\"\n{\n\t\"wait\" \"%f\"\n\t\"classname\" \"%s\"\n\t\"outputs\"\n\t{\n\t%s\n\t}\n}\n", this.HammerID, this.Wait, this.Classname, outputs);
	}

	void ToString(char[] input, int length = OUTPUT_SIZE)
	{
		char outputs[OUTPUT_SIZE];
		for(int i = 0; i < this.OutputList.Length; ++i)
		{
			Output temp;
			this.OutputList.GetArray(i, temp);
			char buffer[OUTPUT_SIZE];
			temp.ToString(buffer);
			Format(outputs, OUTPUT_SIZE, "%s|%s", outputs, buffer);
		}
		Format(input, length, "%s;%f;%s;{%s}", this.HammerID, this.Wait, this.Classname, outputs);
	}

	void CleanUp()
	{
		delete this.OutputList;
	}

	void Parse(const char[] input, int inputLength = OUTPUT_SIZE)
	{
		// #region hammer id
		int id = StrContains(input, "\"hammerid\" \"");
		if(id != -1)
		{
			id += 12;
			int end = FindCharInString(input[id], '"');
			if(end != -1)
			{
				strcopy(this.HammerID, end+1, input[id]);
			}
		}
		// #endregion
		
		// #region classname
		int className = StrContains(input, "\"classname\" \"");
		if(className != -1)
		{
			className += 13;
			int end = FindCharInString(input[className], '"');
			if(end != -1)
			{
				strcopy(this.Classname, end+1, input[className]);
			}
		}
		// #endregion
		// #region wait

		int wait = StrContains(input, "\"wait\" \"");
		if(wait != -1)
		{
			wait += 13;
			int end = FindCharInString(input[wait], '"');
			if(end != -1)
			{
				char waitString[16];

				strcopy(waitString, end+1, input[wait]);
				this.Wait = StringToFloat(waitString);
			}
		}
		// #endregion
		// #region outputs
		delete this.OutputList;

		this.OutputList = new ArrayList(sizeof(Output));
		Regex outputMatch = new Regex("(\"On\\w*\") (\"[^\"]+\")");

		for(int i = 0; i < inputLength && outputMatch.Match(input[i]) > 0; i += outputMatch.MatchOffset())
		{
			Output output;

			outputMatch.GetSubString(1, output.Output, MEMBER_SIZE);
			StripQuotes(output.Output);
			
			char parameters[OUTPUT_SIZE];
			outputMatch.GetSubString(2, parameters, OUTPUT_SIZE);
			
			StripQuotes(parameters);
			output.Parse(parameters);
			
			this.OutputList.PushArray(output);
		}


		delete outputMatch;

		// #endregion
	}
}

/**
*	stock function to copy all the data from one 'Entity' to another
*	Output ArrayLists are copies as well and need to be freed.
*	
*	return: noreturn
**/
stock void CloneEntity(const Entity input, Entity out, Handle newOwner = INVALID_HANDLE)
{
	out.HammerID = input.HammerID;
	out.Classname = input.Classname;
	out.Wait = input.Wait;

	ArrayList list = input.OutputList.Clone();
	out.OutputList = view_as<ArrayList>(CloneHandle(list, newOwner));
	delete list;
}

/**
*	Fired when all map entities have been processed and is safe to call natives.
*	
*	return: noreturn
**/
forward void OnEntitiesReady();

/**
*	Retrieves a copy of the 'Entity' enum struct, for the given entity index.
*	The output ArrayList will not be cleared automatically.
*	
*	Param: index		Entity index.
*	Param: entity		Entity enum struct.
*	
*	return: true if successful, false otherwise.
**/
native bool GetOutputEntity(int index, any[] entity);

/**
*	Retrieves a copy of all the entities inside an ArrayList
*	The ArrayList will not be cleared automatically.
*	
*	
*	return: ArrayList full of 'Entities' if successful, INVALID_HANDLE otherwise.
**/
native ArrayList GetOutputEntities();

/**
*	Returns whether or not it's safe to call any natives
*	
*	return: true/false.
**/
native bool AreEntitiesReady();


#if USE_NATIVES == 1

/**
*	Retrieves the number of outputs that have the given trigger
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	
*	return: Output count
**/
native int GetOutputCount(int index, const char[] output = "");
native int GetOutputActionCount(int index, const char[] output = "");

/**
*	Retrieves the target at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	Param: target		Buffer to store the outputs target
*	Param: length		Size of the buffer, default is the max size of the enum structs members
*	
*	return: True if operation was successful, false otherwise.
**/
native bool GetOutputTarget(int index, const char[] output, int num, char[] target, int length = MEMBER_SIZE);
native bool GetOutputActionTarget(int index, const char[] output, int num, char[] target, int length = MEMBER_SIZE);

/**
*	Retrieves the input at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	Param: input		Buffer to store the targets input
*	Param: length		Size of the buffer, default is the max size of the enum structs members
*	
*	return: True if operation was successful, false otherwise.
**/
native bool GetOutputTargetInput(int index, const char[] output, int num, char[] input, int length = MEMBER_SIZE);
native bool GetOutputActionTargetInput(int index, const char[] output, int num, char[] input, int length = MEMBER_SIZE);

/**
*	Retrieves the output parameters at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	Param: parameters	Buffer to store the outputs parameters
*	Param: length		Size of the buffer, default is the max size of the enum structs members
*	
*	return: True if operation was successful, false otherwise.
**/
native bool GetOutputParameter(int index, const char[] output, int num, char[] parameters, int length = MEMBER_SIZE);
native bool GetOutputActionParameter(int index, const char[] output, int num, char[] parameters, int length = MEMBER_SIZE);

/**
*	Retrieves the output delay at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	
*	return: output delay
**/
native float GetOutputDelay(int index, const char[] output, int num);
native float GetOutputActionDelay(int index, const char[] output, int num);


#else

//////////////////////////////////////////////////////////////////////////
//			OutputInfo /* stock */s used for use in plugins that it			//
//					I do not recommend using these						//
//////////////////////////////////////////////////////////////////////////

/**
*	Retrieves the number of outputs that have the given trigger
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	
*	return: Output count
**/
stock int GetOutputCount(int index, const char[] output = "")
{
	Entity ent;
	if(!GetOutputEntity(index, ent))
	{
		ent.CleanUp();
		return -1;
	}

	int count;
	if(output[0] == 0)
	{
		count = ent.OutputList.Length;
	}
	else
	{
		for(int i = 0; i < ent.OutputList.Length; ++i)
		{
			Output out;
			ent.OutputList.GetArray(i, out);
			if(StrEqual(output, out.Output, false))
			{
				++count;
			}
		}
	}
	ent.CleanUp();
	return count;
}

/**
*	Retrieves the target at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	Param: target		Buffer to store the outputs target
*	Param: length		Size of the buffer, default is the max size of the enum structs members
*	
*	return: True if operation was successful, false otherwise.
**/
stock bool GetOutputTarget(int index, const char[] output, int num, char[] target, int length = MEMBER_SIZE)
{
	Entity ent;
	if(!GetOutputEntity(index, ent))
	{
		ent.CleanUp();
		return false;
	}
	
	bool ret;
	char buffer[MEMBER_SIZE];
	if(StrContains(output, "m_") == 0)
	{
		strcopy(buffer, MEMBER_SIZE, output[2]);
	}
	else
	{
		strcopy(buffer, MEMBER_SIZE, output);
	}

	for(int i = 0, count = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(buffer, out.Output, false))
		{
			if(count++ == num)
			{
				strcopy(target, length, out.Target);
				ret = true
				break;
			}
		}
	}

	ent.CleanUp();
	return ret;
}

/**
*	Retrieves the input at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	Param: input		Buffer to store the targets input
*	Param: length		Size of the buffer, default is the max size of the enum structs members
*	
*	return: True if operation was successful, false otherwise.
**/
stock bool GetOutputTargetInput(int index, const char[] output, int num, char[] input, int length = MEMBER_SIZE)
{
	Entity ent;
	if(!GetOutputEntity(index, ent))
	{
		ent.CleanUp();
		return false;
	}

	int count;
	bool ret;
	char buffer[MEMBER_SIZE];
	if(StrContains(output, "m_") == 0)
	{
		strcopy(buffer, MEMBER_SIZE, output[2]);
	}
	else
	{
		strcopy(buffer, MEMBER_SIZE, output);
	}

	for(int i = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(buffer, out.Output, false))
		{
			if(count++ == num)
			{
				strcopy(input, length, out.Input);
				ret = true
				break;
			}
		}
	}

	ent.CleanUp();
	return ret;
}

/**
*	Retrieves the output parameters at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	Param: parameters	Buffer to store the outputs parameters
*	Param: length		Size of the buffer, default is the max size of the enum structs members
*	
*	return: True if operation was successful, false otherwise.
**/
stock bool GetOutputParameter(int index, const char[] output, int num, char[] parameters, int length = MEMBER_SIZE)
{
	Entity ent;
	if(!GetOutputEntity(index, ent))
	{
		ent.CleanUp();
		return false;
	}

	int count;
	bool ret;
	char buffer[MEMBER_SIZE];
	if(StrContains(output, "m_") == 0)
	{
		strcopy(buffer, MEMBER_SIZE, output[2]);
	}
	else
	{
		strcopy(buffer, MEMBER_SIZE, output);
	}

	for(int i = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(buffer, out.Output, false))
		{
			if(count++ == num)
			{
				strcopy(parameters, length, out.Parameters);
				ret = true
				break;
			}
		}
	}

	ent.CleanUp();
	return ret;
}

/**
*	Retrieves the output delay at the current index for the given output.
*	Not recommended as outputs aren't organized and aren't guarenteed to be the same between sessions and servers.
*	
*	Param: index		Ent index of the entity.
*	Param: output		Output that you want to count. ("OnStartTouch" or empty for all)
*	Param: num			Index/occurance of that given output to return
*	
*	return: output delay
**/
stock float GetOutputDelay(int index, const char[] output, int num)
{
	Entity ent;
	if(!GetOutputEntity(index, ent))
	{
		ent.CleanUp();
		return -1.0;
	}
	
	float delay;
	int count;
	char buffer[MEMBER_SIZE];
	if(StrContains(output, "m_") == 0)
	{
		strcopy(buffer, MEMBER_SIZE, output[2]);
	}
	else
	{
		strcopy(buffer, MEMBER_SIZE, output);
	}

	for(int i = 0; i < ent.OutputList.Length; ++i)
	{
		Output out;
		ent.OutputList.GetArray(i, out);
		if(StrEqual(buffer, out.Output, false))
		{
			if(count++ == num)
			{
				delay = out.Delay;
				break;
			}
		}
	}

	ent.CleanUp();
	return delay;
}
#endif

#if !defined REQUIRE_PLUGIN
public void __pl_output_info_plugin_SetNTVOptional()
{
	MarkNativeAsOptional("GetOutputEntity");
	MarkNativeAsOptional("GetOutputEntities");
	MarkNativeAsOptional("AreEntitiesReady");
	
	#if USE_NATIVES == 1
	MarkNativeAsOptional("GetOutputCount");
	MarkNativeAsOptional("GetOutputTarget");
	MarkNativeAsOptional("GetOutputTargetInput");
	MarkNativeAsOptional("GetOutputParameter");
	MarkNativeAsOptional("GetOutputDelay");

	MarkNativeAsOptional("GetOutputActionCount");
	MarkNativeAsOptional("GetOutputActionTarget");
	MarkNativeAsOptional("GetOutputActionTargetInput");
	MarkNativeAsOptional("GetOutputActionParameter");
	MarkNativeAsOptional("GetOutputActionDelay");
	#endif
}
#endif


stock int GetHammerFromIndex(int index)
{
	if(!IsValidEntity(index))
	{
		return 0;
	}

	return GetEntProp(index, Prop_Data, "m_iHammerID");
}