#include "cbase.h"
#include "tf_shareddefs.h"
#include "pf_loadout_parse.h"
#include "pf_loadout.h"
#include "weapon_parse.h"
#include "filesystem.h"

PFLoadoutWeaponClassInfo_t::PFLoadoutWeaponClassInfo_t()
{
	szItemClass[0] = 0;
	szItemName[0] = 0;
	szItemTypeName[0] = 0;
	szItemDescription[0] = 0;
	iItemType = 0;
	iItemSlot = 0;
	szModel[0] = 0;
	iAnimType = 0;
	szBucket[0] = 0;
	szBucketBlue[0] = 0;
	bDisabled = true;
}

#define PFGetString(var, str) const char * c##var## = pKeyValuesData->GetString(##str##); if (c##var##) Q_strncpy(##var##, c##var##, sizeof(##var##))
#define PFGetInt(var, str) iType = pKeyValuesData->GetInt(##str##); if (c##var##) Q_strncpy(##var##, c##var##, sizeof(##var##))

void PFLoadoutWeaponClassInfo_t::Parse(KeyValues* pKeyValuesData, bool disabled)
{
	const char* cszItemClass = pKeyValuesData->GetString("item_class"); if (cszItemClass && cszItemClass[0]) Q_strncpy(szItemClass, cszItemClass, sizeof(szItemClass));
	const char* cszItemName = pKeyValuesData->GetString("item_name"); if (cszItemName && cszItemName[0]) Q_strncpy(szItemName, cszItemName, sizeof(szItemName));
	const char* cszItemTypeName = pKeyValuesData->GetString("item_type_name"); if (cszItemTypeName && cszItemTypeName[0]) Q_strncpy(szItemTypeName, cszItemTypeName, sizeof(szItemTypeName));
	const char* cszItemDescription = pKeyValuesData->GetString("item_description"); if (cszItemDescription && cszItemDescription[0]) Q_strncpy(szItemDescription, cszItemDescription, sizeof(szItemDescription));
	const char* cszItemType = pKeyValuesData->GetString("item_type");
	if (cszItemType && cszItemType[0])
	{
		if (!Q_strcmp(cszItemType, "weapon")) iItemType = PF_WPN_TYPE_WEAPON;
		else if (!Q_strcmp(cszItemType, "grenade")) iItemType = PF_WPN_TYPE_GRENADE;
		else if (!Q_strcmp(cszItemType, "buildable")) iItemType = PF_WPN_TYPE_BUILDABLE;
	}
	iItemSlot = pKeyValuesData->GetInt("item_slot", iItemSlot);
	const char* cszModel = pKeyValuesData->GetString("model"); if (cszModel && cszModel[0]) Q_strncpy(szModel, cszModel, sizeof(szModel));
	const char* cszAnimType = pKeyValuesData->GetString("animtype", NULL);
	if (cszAnimType && cszAnimType[0])
	{
		if (!Q_strcmp(cszAnimType, "primary")) iAnimType = TF_WPN_TYPE_PRIMARY;
		else if (!Q_strcmp(cszAnimType, "secondary")) iAnimType = TF_WPN_TYPE_SECONDARY;
		else if (!Q_strcmp(cszAnimType, "melee")) iAnimType = TF_WPN_TYPE_MELEE;
		else if (!Q_strcmp(cszAnimType, "grenade")) iAnimType = TF_WPN_TYPE_GRENADE;
		else if (!Q_strcmp(cszAnimType, "building")) iAnimType = TF_WPN_TYPE_BUILDING;
		else if (!Q_strcmp(cszAnimType, "pda")) iAnimType = TF_WPN_TYPE_PDA;
	}
	const char* cszBucket = pKeyValuesData->GetString("bucket"); if (cszBucket && cszBucket[0]) Q_strncpy(szBucket, cszBucket, sizeof(szBucket));
	const char* cszBucketBlue = pKeyValuesData->GetString("bucket_blue", cszBucket); if (cszBucketBlue && cszBucketBlue[0]) Q_strncpy(szBucketBlue, cszBucketBlue, sizeof(szBucketBlue));
	bDisabled = disabled || pKeyValuesData->GetBool("disabled", disabled);
}

PFLoadoutWeaponInfo_t::PFLoadoutWeaponInfo_t()
{
	bParsedScript = false;
}

void PFLoadoutWeaponInfo_t::Parse(KeyValues* pKeyValuesData)
{
	pGeneral = new PFLoadoutWeaponClassInfo_t();
	pGeneral->Parse(pKeyValuesData, false);
	KeyValues* k = pKeyValuesData->FindKey("used_by_classes");
	bool usedByClassesExist = k != NULL;
	bool d = !(usedByClassesExist && !pGeneral->bDisabled);
	for (int i = 1; i < TF_CLASS_COUNT_ALL; i++)
	{
		pClass[i-1] = new PFLoadoutWeaponClassInfo_t();
		pClass[i-1]->Parse(pKeyValuesData, true);
		if (!d)
		{
			KeyValues* a = NULL;
			switch (i)
			{
			default: break;
			case TF_CLASS_SCOUT: a = k->FindKey("scout"); break;
			case TF_CLASS_SNIPER: a = k->FindKey("sniper"); break;
			case TF_CLASS_SOLDIER: a = k->FindKey("soldier"); break;
			case TF_CLASS_DEMOMAN: a = k->FindKey("demoman"); break;
			case TF_CLASS_MEDIC: a = k->FindKey("medic"); break;
			case TF_CLASS_HEAVYWEAPONS: a = k->FindKey("heavy"); break;
			case TF_CLASS_PYRO: a = k->FindKey("pyro"); break;
			case TF_CLASS_SPY: a = k->FindKey("spy"); break;
			case TF_CLASS_ENGINEER: a = k->FindKey("engineer"); break;
			}
			if(a)
				pClass[i - 1]->Parse(a, d);
		}
	}
	bParsedScript = true;
}
