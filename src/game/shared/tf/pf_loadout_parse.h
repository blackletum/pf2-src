#ifndef PF_LOADOUT_PARSE_H
#define PF_LOADOUT_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#define PF_MAX_WEAPON_STRING	80
#define PF_MAX_WEAPON_PREFIX	16

#define PF_WEAPON_PRINTNAME_MISSING "missingno"

class PFLoadoutWeaponClassInfo_t
{
public:

	PFLoadoutWeaponClassInfo_t();

	virtual void Parse(KeyValues* pKeyValuesData, bool disabled);

public:
	char	szItemClass[PF_MAX_WEAPON_STRING];
	char	szItemName[PF_MAX_WEAPON_STRING];
	char	szItemTypeName[PF_MAX_WEAPON_STRING];
	char	szItemDescription[PF_MAX_WEAPON_STRING];
	int		iItemType;
	int		iItemSlot;
	char	szModel[PF_MAX_WEAPON_STRING];
	int  	iAnimType;
	char	szBucket[PF_MAX_WEAPON_STRING];
	char	szBucketBlue[PF_MAX_WEAPON_STRING];
	bool	bDisabled;
};
class PFLoadoutWeaponInfo_t
{
public:

	PFLoadoutWeaponInfo_t();

	virtual void Parse(KeyValues* pKeyValuesData);

public:
	bool	bParsedScript;
	PFLoadoutWeaponClassInfo_t *pGeneral;
	PFLoadoutWeaponClassInfo_t *pClass[TF_CLASS_COUNT];
};

#endif