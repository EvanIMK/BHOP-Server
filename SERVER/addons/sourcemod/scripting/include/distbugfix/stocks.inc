
// stocks for distbugfix plugin

#if defined _distbugfix_stocks_included
	#endinput
#endif
#define _distbugfix_stocks_included

/**
 * Determines the point of intersection between a plane defined by a point and a normal vector and a line defined by a point and a direction vector.
 *
 * @param planePoint    A point on the plane.
 * @param planeNormal   The normal vector of the plane.
 * @param linePoint     A point on the line.
 * @param lineDirection The direction vector of the line.
 * @param result		The resultant vector.
 */
stock void lineIntersection(const float planePoint[3], const float planeNormal[3], const float linePoint[3], const float lineDirection[3], float result[3])
{
	if (GetVectorDotProduct(planeNormal, lineDirection) == 0)
	{
		return;
	}
	
	float t = (GetVectorDotProduct(planeNormal, planePoint)
			 - GetVectorDotProduct(planeNormal, linePoint))
			 / GetVectorDotProduct(planeNormal, lineDirection);
	
	float lineDir[3];
	CopyVector(lineDirection, lineDir);
	NormalizeVector(lineDir, lineDir);
	
	ScaleVector(lineDir, t);
	
	AddVectors(linePoint, lineDirection, result);
}

stock bool TraceLandPos(int client, const float pos[3], const float velocity[3], float result[3], float fGravity, bool bugged = false)
{
	float mins[3];
	float maxs[3];
	float pos2[3];
	float newVel[3];
	
	CopyVector(velocity, newVel);
	
	if (bugged)
	{
		// add 0.5 gravity
		newVel[2] -= fGravity * GetTickInterval() / 2.0;
	}
	else
	{
		// add 1.5 gravity
		newVel[2] -= fGravity * GetTickInterval() / 2.0 * 3.0;
	}
	
	ScaleVector(newVel, GetTickInterval() * 2.0);
	AddVectors(pos, newVel, pos2);
	
	GetClientMins(client, mins);
	GetClientMaxs(client, maxs);
	
	Handle trace = TR_TraceHullFilterEx(pos, pos2, mins, maxs, MASK_PLAYERSOLID, TraceEntityFilterPlayer);
	
	if (!TR_DidHit(trace))
	{
		CopyVector(pos, result);
		CloseHandle(trace);
		return false;
	}
	
	TR_GetEndPosition(result, trace);
	CloseHandle(trace);
	
	return true;
}

stock int BlockDirection(const float jumpoff[3], const float position[3])
{
	return FloatAbs(position[1] - jumpoff[1]) > FloatAbs(position[0] - jumpoff[0]);
}

stock bool TraceGround(int client, const float pos[3], float result[3])
{
	float mins[3];
	float maxs[3];
	
	GetClientMins(client, mins);
	GetClientMaxs(client, maxs);
	
	float startpos[3];
	float endpos[3];
	
	startpos = pos;
	endpos = pos;
	endpos[2] -= 2.0;
	
	Handle trace = TR_TraceHullFilterEx(startpos, endpos, mins, maxs, MASK_PLAYERSOLID, TraceEntityFilterPlayer);
	
	if (TR_DidHit(trace))
	{
		TR_GetEndPosition(result, trace);
		CloseHandle(trace);
		return true;
	}
	CloseHandle(trace);
	return false;
}

stock bool TraceBlock(const float pos2[3], const float position[3], float result[3])
{
	float mins[3] =  { -16.0, -16.0, -1.0 };
	float maxs[3] =  { 16.0, 16.0, 0.0 };
	
	Handle trace = TR_TraceHullFilterEx(pos2, position, mins, maxs, MASK_PLAYERSOLID, TraceEntityFilterPlayer);
	
	if(!TR_DidHit(trace))
	{
		CloseHandle(trace);
		return false;
	}
	
	TR_GetEndPosition(result, trace);
	CloseHandle(trace);
	
	return true;
}

stock bool IsOffset(float z1, float z2, float tolerance)
{
	return (FloatAbs(z1 - z2) > tolerance);
}

stock float GetVectorHorLength(const float vec[3])
{
	float tempVec[3];
	tempVec = vec;
	tempVec[2] = 0.0;
	
	return GetVectorLength(tempVec);
}

stock float GetVectorHorDistance(const float x[3], const float y[3])
{
	float x2[3];
	float y2[3];
	
	x2 = x;
	y2 = y;
	
	x2[2] = 0.0;
	y2[2] = 0.0;
	
	return GetVectorDistance(x2, y2);
}

stock bool IsValidClient(int client)
{
	return (client >= 0 && client <= MaxClients && IsValidEntity(client) && IsClientConnected(client) && IsClientInGame(client));
}

stock bool TraceEntityFilterPlayer(int entity, any data)
{
	return entity > MAXPLAYERS;
}

stock int abs(x)
{
   return x >= 0 ? x : -x;
}

stock bool IsOverlapping(int &buttons)
{
	return buttons & IN_MOVERIGHT && buttons & IN_MOVELEFT
}

stock bool IsStrafeSynced(float speed, float lastspeed)
{
	return speed > lastspeed;
}

stock bool IsDeadAirtime(int &buttons)
{
	return !(buttons & IN_MOVERIGHT) && !(buttons & IN_MOVELEFT);
}

stock bool CheckMaxSpeed(float speed, float lastspeed)
{
	return speed > lastspeed;
}

stock bool IsFailstat(float z1, float z2, float tolerance)
{
	return z1 < z2 && z1 > z2 - tolerance;
}

stock bool IsFloatInRange(float distance, float min, float max)
{
	return distance >= min && distance <= max;
}

public float NormaliseAngle(float angle)
{
	while (angle <= -180.0)
	{
		angle += 360.0;
	}
	
	while (angle > 180.0)
	{
		angle -= 360.0;
	}
	
	return angle;
}

stock float GetEyeRayPosition(int client, float end[3])
{
	float start[3];
	float angle[3];
	
	GetClientEyePosition(client, start);
	GetClientEyeAngles(client, angle);
	
	TR_TraceRayFilter(start, angle, MASK_PLAYERSOLID, RayType_Infinite, TraceEntityFilterPlayer, client);
	
	if (TR_DidHit(INVALID_HANDLE))
	{
		TR_GetEndPosition(end, INVALID_HANDLE);
	}
}

stock float CalcIntPercentage(int total, int fraction)
{
	if (total == 0)
	{
		return 0.0;
	}
	
	return float(fraction) / float(total) * 100.0;
}

stock void PrintToChatAdmins(const char[] format, any ...)
{
	char buffer[254];
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i))
		{
			AdminId id = GetUserAdmin(i);
			if (!GetAdminFlag(id, Admin_Generic))
			{
				continue;
			}
			SetGlobalTransTarget(i);
			VFormat(buffer, sizeof(buffer), format, 2);
			PrintToChat(i, "%s", buffer);
		}
	}
}

stock void CopyVector(const float source[3], float dest[3])
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
}