#ifndef PF_LOADOUT_H
#define PF_LOADOUT_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_shareddefs.h"
#include "pf_loadout_parse.h"

enum
{
	PF_WPN_TYPE_WEAPON,
	PF_WPN_TYPE_GRENADE,
	PF_WPN_TYPE_BUILDABLE,
};
	

int PFLoadout_PlayerGetWantedWeapon(int playerclass, int type, int slot, int entindex);
int PFLoadout_PlayerGetWeapon(int playerclass, int type, int slot, int entindex, bool ignoreDisabled = false);
int PFLoadout_PlayerGetDefaultWeapon(int playerclass, int type, int slot);
PFLoadoutWeaponClassInfo_t* PFLoadout_GetWeapon(int weapon, int playerclass);
bool PFLoadout_ClassHasWeapon(int playerclass, int weapon, int type, int slot, bool ignoreDisabled = false);
void PFLoadout_Parse();

#endif