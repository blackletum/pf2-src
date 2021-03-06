//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_tranq.h"
#include "tf_fx_shared.h"
#include "basecombatweapon_shared.h"
#include "tf_viewmodel.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Tranq tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFTranq, DT_WeaponTranq )

BEGIN_NETWORK_TABLE( CTFTranq, DT_WeaponTranq )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFTranq )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_tranq, CTFTranq );
PRECACHE_WEAPON_REGISTER( tf_weapon_tranq );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFTranq )
END_DATADESC()
#endif

#define TRANQ_RELOAD_TIME 1.7

//=============================================================================
//
// Weapon Tranq functions.
//

void CTFTranq::Spawn()
{
	BaseClass::Spawn();
	ReloadCheck();
}

bool CTFTranq::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (BaseClass::Holster(pSwitchingTo))
	{
		if (m_bShouldReloadWhileHolstered)
			m_flHolsterReloadTime = gpGlobals->curtime + TRANQ_RELOAD_TIME;
		return true;
	}

	return false;
}

void CTFTranq::ReloadCheck(void)
{
	if (GetModelPtr())
	{
		CTFPlayer *pOwner = GetTFPlayerOwner();
		if (pOwner && pOwner->GetActiveTFWeapon() != this)
		{
			if (!m_bShouldReloadWhileHolstered)
			{
				if (Clip1() <= 0)
				{
					m_flHolsterReloadTime = gpGlobals->curtime + TRANQ_RELOAD_TIME;
					m_bShouldReloadWhileHolstered = true;
				}
			}
			else
			{
				if (m_flHolsterReloadTime < gpGlobals->curtime)
				{
					ReloadWhileHolstered();
					m_bShouldReloadWhileHolstered = false;
				}
			}
		}
	}
	SetContextThink(&CTFTranq::ReloadCheck, gpGlobals->curtime, "ReloadCheck");

}

void CTFTranq::ReloadWhileHolstered(void)
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if (pOwner && pOwner->GetActiveTFWeapon() != this && pOwner->GetAmmoCount(GetTFWpnData().iAmmoType) >= 1 && Clip1() == 0)
	{
		pOwner->RemoveAmmo(1, GetTFWpnData().iAmmoType);
		m_iClip1++;
	}
}
