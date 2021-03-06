#include "cbase.h"
#include "pf_loadout.h"
#include "pf_loadout_parse.h"
#include "tf_playerclass_shared.h"
#include "filesystem.h"

#ifdef CLIENT_DLL
#define LoadoutCVar( playerclass , type ) \
ConVar pf_loadout_##playerclass##_##type ("pf_loadout_" #playerclass "_" #type, "0", FCVAR_USERINFO | FCVAR_HIDDEN);
#define LoadoutClassCVar(__playerclass__) \
LoadoutCVar( __playerclass__ , w1) \
LoadoutCVar( __playerclass__ , w2) \
LoadoutCVar( __playerclass__ , w3) \
LoadoutCVar( __playerclass__ , w4) \
LoadoutCVar( __playerclass__ , w5) \
LoadoutCVar( __playerclass__ , g1) \
LoadoutCVar( __playerclass__ , g2)

#define LoadoutGetCVarWeapon( playerclass,  type , slot ) return pf_loadout_##playerclass##_##type##slot .GetInt();break
#else
#define LoadoutGetCVarWeapon( playerclass , type , slot ) return V_atoi(engine->GetClientConVarValue(entindex, "pf_loadout_" #playerclass "_" #type #slot));break
#endif
#define LoadoutGetCVarWeaponClass( playerclass , PLAYERCLASS , type , slot ) \
case TF_CLASS_##PLAYERCLASS :\
	switch ( type )\
	{\
		case PF_WPN_TYPE_WEAPON:\
			switch ( slot )\
			{\
				case 0:\
					LoadoutGetCVarWeapon( playerclass, w , 1);\
				case 1:\
					LoadoutGetCVarWeapon( playerclass, w , 2);\
				case 2:\
					LoadoutGetCVarWeapon( playerclass, w , 3);\
				case 3:\
					LoadoutGetCVarWeapon( playerclass, w , 4);\
				case 4:\
					LoadoutGetCVarWeapon( playerclass, w , 5);\
			}\
		case PF_WPN_TYPE_GRENADE:\
			switch ( slot )\
			{\
				case 0:\
					LoadoutGetCVarWeapon( playerclass, g , 1);\
				case 1:\
					LoadoutGetCVarWeapon( playerclass, g , 2);\
			}\
		default:\
			return -1;\
	}\
break;
#ifdef CLIENT_DLL
LoadoutClassCVar(scout)
LoadoutClassCVar(sniper)
LoadoutClassCVar(soldier)
LoadoutClassCVar(demoman)
LoadoutClassCVar(medic)
LoadoutClassCVar(heavyweapons)
LoadoutClassCVar(pyro)
LoadoutClassCVar(spy)
LoadoutClassCVar(engineer)
LoadoutClassCVar(civilian)
#endif

static PFLoadoutWeaponInfo_t *m_PFLoadoutWeaponInfoDatabase[128];
static bool b_PFLoadoutWeaponInfoDatabaseParsed;

int PFLoadout_PlayerGetWantedWeapon(int playerclass, int type, int slot, int entindex)
{
	switch (playerclass)
	{
		LoadoutGetCVarWeaponClass(scout, SCOUT, type, slot)
		LoadoutGetCVarWeaponClass(sniper, SNIPER, type, slot)
		LoadoutGetCVarWeaponClass(soldier, SOLDIER, type, slot)
		LoadoutGetCVarWeaponClass(demoman, DEMOMAN, type, slot)
		LoadoutGetCVarWeaponClass(medic, MEDIC, type, slot)
		LoadoutGetCVarWeaponClass(heavyweapons, HEAVYWEAPONS, type, slot)
		LoadoutGetCVarWeaponClass(pyro, PYRO, type, slot)
		LoadoutGetCVarWeaponClass(spy, SPY, type, slot)
		LoadoutGetCVarWeaponClass(engineer, ENGINEER, type, slot)
		LoadoutGetCVarWeaponClass(civilian, CIVILIAN, type, slot)
	default:
		return 0;
	}
}
int PFLoadout_PlayerGetWeapon(int playerclass, int type, int slot, int entindex, bool ignoreDisabled)
{
	int weapon = PFLoadout_PlayerGetWantedWeapon(playerclass, type, slot, entindex);
	if (!PFLoadout_ClassHasWeapon(playerclass, weapon, type, slot, ignoreDisabled))
	{
		return PFLoadout_PlayerGetDefaultWeapon(playerclass, type, slot);
	}
	return weapon;
}
int PFLoadout_PlayerGetDefaultWeapon(int playerclass, int type, int slot)
{
	TFPlayerClassData_t* classdata = GetPlayerClassData(playerclass);
	if (classdata)
	{
		switch (type)
		{
		default:
		case PF_WPN_TYPE_WEAPON:
			return classdata->m_aWeapons[slot];
		case PF_WPN_TYPE_GRENADE:
			return classdata->m_aGrenades[slot];
		case PF_WPN_TYPE_BUILDABLE:
			return classdata->m_aBuildable[slot];
		}
	}
	return 0;
}
PFLoadoutWeaponClassInfo_t* PFLoadout_GetWeapon(int weapon, int playerclass)
{
	if (!b_PFLoadoutWeaponInfoDatabaseParsed)
	{
		PFLoadout_Parse();
	}
	if (weapon < 0 || weapon >= sizeof(m_PFLoadoutWeaponInfoDatabase))
		return NULL;
	if (!m_PFLoadoutWeaponInfoDatabase[weapon])
		return NULL;
	if (!m_PFLoadoutWeaponInfoDatabase[weapon]->bParsedScript)
		return NULL;
	return m_PFLoadoutWeaponInfoDatabase[weapon]->pClass[playerclass - 1];
}
bool PFLoadout_ClassHasWeapon(int playerclass, int weapon, int type, int slot, bool ignoreDisabled)
{
	if (!b_PFLoadoutWeaponInfoDatabaseParsed)
	{
		PFLoadout_Parse();
	}
	if (weapon < 0 || weapon >= sizeof(m_PFLoadoutWeaponInfoDatabase))
		return false;
	if (!m_PFLoadoutWeaponInfoDatabase[weapon])
		return false;
	if (!m_PFLoadoutWeaponInfoDatabase[weapon]->bParsedScript)
		return false;
	PFLoadoutWeaponClassInfo_t* c = m_PFLoadoutWeaponInfoDatabase[weapon]->pClass[playerclass - 1];
	TFPlayerClassData_t* classdata = GetPlayerClassData(playerclass);
	bool a;
	switch (type)
	{
	case PF_WPN_TYPE_GRENADE:
		a = classdata->m_aGrenades[slot] > 0;
	default:
		a = classdata->m_aWeapons[slot] > 0;
	}
	return a && ((!c->bDisabled) || ignoreDisabled) && (c->iItemType == type) && (c->iItemSlot-1 == slot);
}
void PFLoadout_Parse()
{
	KeyValues* pKeyValuesData = ReadEncryptedKVFile(filesystem, "scripts/items/item_classes", g_pGameRules->GetEncryptionKey());
	if (!pKeyValuesData)
		Error("Could not load item file. Check your shit next time.");
	int i = 1;
	for (KeyValues* pk_fire = pKeyValuesData; pk_fire; pk_fire = pk_fire->GetNextKey())
	{
		PFLoadoutWeaponInfo_t* p = new PFLoadoutWeaponInfo_t();
		p->Parse(pk_fire);
		m_PFLoadoutWeaponInfoDatabase[i] = p;
		i++;
	}
	b_PFLoadoutWeaponInfoDatabaseParsed = true;
}
