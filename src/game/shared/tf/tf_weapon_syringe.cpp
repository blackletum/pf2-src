//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_syringe.h"
#ifdef GAME_DLL
#include "tf_player.h"
#include "engine/IEngineSound.h"
#include "tf_gamestats.h"
#else
#include "c_tf_player.h"
#endif

//=============================================================================
//
// Weapon Syringe tables.
//

#define TF_SYRINGE_HEAL_SOUND	"Weapon_Syringe.Heal"

IMPLEMENT_NETWORKCLASS_ALIASED( TFSyringe, DT_TFWeaponSyringe )

BEGIN_NETWORK_TABLE( CTFSyringe, DT_TFWeaponSyringe )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFSyringe )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_syringe, CTFSyringe );
PRECACHE_WEAPON_REGISTER(tf_weapon_syringe);

#ifdef GAME_DLL
void CTFSyringe::OnEntityHit(CBaseEntity *pEntity)
{
	CTFPlayer* pPlayer = ToTFPlayer(pEntity);
	CTFPlayer* pOwner = GetTFPlayerOwner();
	if (!pPlayer)
		return;
	if (!pOwner)
		return;
	if (pPlayer->GetUserID() == pOwner->GetUserID())
		return;

	if (pPlayer->GetTeamNumber() != pOwner->GetTeamNumber())
	{
		pPlayer->m_Shared.Infect(pOwner);
		return;
	}

	pPlayer->m_Shared.RemoveCond(TF_COND_BURNING);
	pPlayer->m_Shared.RemoveCond(TF_COND_INFECTED);
	pPlayer->m_Shared.RemoveCond(TF_COND_TRANQUILIZED);
	pPlayer->m_Shared.RemoveCond(TF_COND_LEG_DAMAGED);


	int iHealthNeeded = pPlayer->GetMaxHealth() - pPlayer->GetHealth();

	if (iHealthNeeded > 0)
	{
		int iHealthIncreased = min( iHealthNeeded, m_pWeaponInfo->GetWeaponData(m_iWeaponMode).m_nHeal );
		pPlayer->TakeHealth(iHealthIncreased, DMG_GENERIC);

		CPASAttenuationFilter filter( pPlayer->WorldSpaceCenter() );
		EmitSound( filter, entindex(), TF_SYRINGE_HEAL_SOUND );

		CTF_GameStats.Event_PlayerHealedOther( pOwner, iHealthIncreased, false );

		IGameEvent* event = gameeventmanager->CreateEvent( "player_healed" );
		if ( event )
		{
			event->SetInt( "priority", 1 );
			event->SetInt( "patient", pPlayer->GetUserID() );
			event->SetInt( "healer", pOwner->GetUserID() );
			event->SetInt( "amount", iHealthIncreased );

			gameeventmanager->FireEvent( event );
		}
	}
}
#endif

bool CTFSyringe::DoSwingTrace( trace_t &trace )
{
	// Setup a volume for the melee weapon to be swung - approx size, so all melee behave the same.
	static Vector vecSwingMins( -18, -18, -18 );
	static Vector vecSwingMaxs( 18, 18, 18 );

	// Get the current player.
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if (!pPlayer)
		return false;

	// Setup the swing range.
	Vector vecForward;
	AngleVectors( pPlayer->EyeAngles() + pPlayer->ConcAngles(), &vecForward );
	Vector vecSwingStart = pPlayer->Weapon_ShootPosition();
	Vector vecSwingEnd = vecSwingStart + vecForward * 48;

	// See if we hit anything. To do this while ignoring teammates I've used 2 traces
	// If we don't hit anything with the team excluded trace it uses the standard one

	UTIL_TraceLine( vecSwingStart, vecSwingEnd, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace );
	if (trace.fraction >= 1.0)
	{
		UTIL_TraceHull( vecSwingStart, vecSwingEnd, vecSwingMins, vecSwingMaxs, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &trace );


		if (trace.fraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = trace.m_pEnt;
			if (!pHit || pHit->IsBSPModel())
			{
				// Why duck hull min/max?
				FindHullIntersection( vecSwingStart, trace, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, pPlayer );
			}
		}
	}

	return (trace.fraction < 1.0f);
}
