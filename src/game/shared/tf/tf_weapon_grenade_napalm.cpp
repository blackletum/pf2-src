//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Napalm Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_napalm.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#include "particle_parse.h"
#include "trigger_area_capture.h"
#endif

#define NAPALM_BURN_TIME 5
#define NAPALM_BURN_TIME_ALT 1 //will only burn twice
#define NAPALM_FIRE_PARTICLE_COUNT 20

//=============================================================================
//
// TF Napalm Grenade tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED(TFGrenadeNapalm, DT_TFGrenadeNapalm)

BEGIN_NETWORK_TABLE(CTFGrenadeNapalm, DT_TFGrenadeNapalm)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CTFGrenadeNapalm)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(tf_weapon_grenade_napalm, CTFGrenadeNapalm);
PRECACHE_WEAPON_REGISTER(tf_weapon_grenade_napalm);

//=============================================================================
//
// TF Napalm Grenade functions.
//

// Server specific.
#ifdef GAME_DLL
ConVar pf_napalm_bomb( "pf_napalm_bomb", "0", FCVAR_NOTIFY, "Alternative napalm grenade functionality. Behaves more like a standard frag grenade, but sets people on fire " );

BEGIN_DATADESC(CTFGrenadeNapalm)
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFWeaponBaseGrenadeProj* CTFGrenadeNapalm::EmitGrenade(Vector vecSrc, QAngle vecAngles, Vector vecVel,
	AngularImpulse angImpulse, CBasePlayer* pPlayer, float flTime, int iflags)
{
	return CTFGrenadeNapalmProjectile::Create(vecSrc, vecAngles, vecVel, angImpulse,
		pPlayer, GetTFWpnData(), flTime);
}

#endif
IMPLEMENT_NETWORKCLASS_ALIASED(TFGrenadeNapalmProjectile, DT_TFGrenadeNapalmProjectile)

BEGIN_NETWORK_TABLE(CTFGrenadeNapalmProjectile, DT_TFGrenadeNapalmProjectile)
END_NETWORK_TABLE()

//=============================================================================
//
// TF Normal Grenade functions.
//
CTFGrenadeNapalmProjectile::CTFGrenadeNapalmProjectile()
{
}
CTFGrenadeNapalmProjectile::~CTFGrenadeNapalmProjectile()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

//=============================================================================
//
// TF Napalm Grenade Projectile functions (Server specific).
//
#ifdef GAME_DLL

#define GRENADE_MODEL "models/weapons/w_models/w_grenade_napalm.mdl"
#define GRENADE_MODEL_ALT "models/weapons/w_models/w_grenade_napalm_bomb.mdl"

LINK_ENTITY_TO_CLASS(tf_weapon_grenade_napalm_projectile, CTFGrenadeNapalmProjectile);
PRECACHE_WEAPON_REGISTER(tf_weapon_grenade_napalm_projectile);

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeNapalmProjectile* CTFGrenadeNapalmProjectile::Create( const Vector& position, const QAngle& angles,
																const Vector& velocity, const AngularImpulse& angVelocity,
																CBaseCombatCharacter* pOwner, const CTFWeaponInfo& weaponInfo, float timer, int iFlags)
{
	CTFGrenadeNapalmProjectile* pGrenade = static_cast<CTFGrenadeNapalmProjectile*>(CTFWeaponBaseGrenadeProj::Create("tf_weapon_grenade_napalm_projectile", position, angles, velocity, angVelocity, pOwner, weaponInfo, iFlags));
	if (pGrenade)
	{
		pGrenade->ApplyLocalAngularVelocityImpulse(angVelocity);
		pGrenade->SetDetonateTimerLength(timer);
		pGrenade->m_flInitialDetTime = gpGlobals->curtime + timer;
		pGrenade->ChangeTeam(pOwner->GetTeamNumber());
	}

	return pGrenade;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::Spawn()
{
	if( pf_napalm_bomb.GetBool())
		SetModel(GRENADE_MODEL_ALT);
	else
		SetModel(GRENADE_MODEL);

	RegisterThinkContext("BurningContext");

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::Precache()
{
	PrecacheModel(GRENADE_MODEL);
	PrecacheModel(GRENADE_MODEL_ALT);
	PrecacheParticleSystem( "napalm_water" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::BounceSound(void)
{
	EmitSound( "Weapon_Grenade_Nail.Bounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::DetonateThink(void)
{
	if ( !pf_napalm_bomb.GetBool() && !(UTIL_PointContents( GetAbsOrigin() ) & CONTENTS_WATER ) )
	{
		// if we're past the detonate time but still moving, delay the detonate
		if (gpGlobals->curtime > GetDetonateTime() && VPhysicsGetObject())
		{
			Vector vel;
			VPhysicsGetObject()->GetVelocity(&vel, NULL);
			// we've failed to find a ground entity so wait for the grenade to stop moving or to find ground
			// this also helps in cases where we've somehow not registered that we've touched the ground
			if (!GetGroundEntity() && vel.Length() > 25.0)
			{
				// check here a lot
				SetTimer( gpGlobals->curtime + 0.05f);
			}
		}
	}
	BaseClass::DetonateThink();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeNapalmProjectile::Detonate()
{
	if (ShouldNotDetonate())
	{
		RemoveGrenade();
		return;
	}

	m_bIsFlaming = true;

	BaseClass::Detonate();

	CTFGrenadeNapalmFire* fire = NULL;

	if( UTIL_PointContents( GetAbsOrigin() ) & CONTENTS_WATER )
	{
		DispatchParticleEffect( "napalm_water", GetAbsOrigin(), QAngle( 0, 0, 0 ) );
	}
	else
	{
		fire = CTFGrenadeNapalmFire::Create( GetAbsOrigin(), GetThrower(), m_DmgRadius, pf_napalm_bomb.GetBool() ? NAPALM_BURN_TIME_ALT : NAPALM_BURN_TIME);
		fire->ChangeTeam( GetTeamNumber() );
	}
}

void CTFGrenadeNapalmProjectile::VPhysicsCollision(int index, gamevcollisionevent_t* pEvent)
{
	BaseClass::VPhysicsCollision( index, pEvent );

	// If we're not exploding try to find ground
	if (!m_bIsFlaming)
	{
		int otherIndex = !index;
		CBaseEntity* pHitEntity = pEvent->pEntities[otherIndex];

		if (pEvent->deltaCollisionTime < 0.2f && (pHitEntity == this))
			return;

		Vector vel;
		VPhysicsGetObject()->GetVelocity(&vel, NULL);
		float flSpeedSqr = DotProduct(vel, vel);
		if (flSpeedSqr < 1200)
		{
			if (pHitEntity->IsStandable())
			{
				SetGroundEntity(pHitEntity);
			}
		}
		return;
	}
	VPhysicsGetObject()->EnableMotion(false);
}

void CTFGrenadeNapalmProjectile::ExplodeInHand(CTFPlayer* pPlayer)
{
	SetTimer(gpGlobals->curtime+0.2);
}

bool CTFGrenadeNapalmProjectile::ShouldNotDetonate(void)
{
	CBaseEntity* pTempEnt = NULL;

	while ((pTempEnt = gEntList.FindEntityByClassname(pTempEnt, "trigger_capture_area")) != NULL)
	{
		CTriggerAreaCapture* pZone = dynamic_cast<CTriggerAreaCapture*>(pTempEnt);

		if (pZone->IsTouching(this))
		{
			return true;
		}
	}
	return BaseClass::ShouldNotDetonate();
}

LINK_ENTITY_TO_CLASS(tf_weapon_grenade_napalm_fire, CTFGrenadeNapalmFire);
PRECACHE_REGISTER(tf_weapon_grenade_napalm_fire);

CTFGrenadeNapalmFire* CTFGrenadeNapalmFire::Create(const Vector& position, CBaseCombatCharacter* pOwner, float radius, float burnTime)
{
	CTFGrenadeNapalmFire* pFire = static_cast<CTFGrenadeNapalmFire*>(CBaseEntity::Create("tf_weapon_grenade_napalm_fire", position, QAngle(0, 0, 0), pOwner));

	if (pFire)
	{
		pFire->m_DmgRadius = radius;
		pFire->burnTime = gpGlobals->curtime + burnTime;
	}

	return pFire;
}

void CTFGrenadeNapalmFire::Precache()
{
	PrecacheParticleSystem("napalm_explosion_flames");
	PrecacheParticleSystem( "napalm_fire_red" );
	PrecacheParticleSystem("napalm_fire_blue");
	PrecacheScriptSound("Weapon_Grenade_Napalm.Fire");
	
	BaseClass::Precache();
}

void CTFGrenadeNapalmFire::Spawn()
{
	BaseClass::Spawn();
	SetNextThink(gpGlobals->curtime);

	EmitSound("Weapon_Grenade_Napalm.Fire");
	if ( pf_napalm_bomb.GetBool() )
	{
		DispatchParticleEffect( "napalm_explosion_flames", GetAbsOrigin(), QAngle( 0, 0, 0 ) );
	}
	else
	{
		if ( GetOwnerEntity()->GetTeamNumber() == TF_TEAM_RED )
			DispatchParticleEffect( "napalm_fire_red", GetAbsOrigin(), QAngle( 0, 0, 0 ) );
		else
			DispatchParticleEffect( "napalm_fire_blue", GetAbsOrigin(), QAngle( 0, 0, 0 ) );
	}
}

void CTFGrenadeNapalmFire::Think()
{
	if (gpGlobals->curtime >= burnTime || !GetOwnerEntity() || !GetOwnerEntity()->IsPlayer())
	{
		Remove();
		return;
	}

	float flDamage = GetTFWeaponInfo( TF_WEAPON_GRENADE_NAPALM )->m_WeaponData[TF_WEAPON_SECONDARY_MODE].m_nDamage;

	CBaseEntity *pEntity = NULL;
	for (CEntitySphereQuery sphere( GetAbsOrigin(), m_DmgRadius ); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (pEntity->m_takedamage == DAMAGE_NO)
			continue;

		// check for valid player
		if (!pEntity->IsPlayer())
			continue;

		CTFPlayer* pPlayer = ToTFPlayer( pEntity );

		if (pPlayer && (pPlayer->GetTeamNumber() != ToTFPlayer(GetOwnerEntity())->GetTeamNumber() || pPlayer == ToTFPlayer(GetOwnerEntity())))
		{
			trace_t c;
			UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, 16), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_DEBRIS, &c);
			trace_t trace;
			UTIL_TraceLine(GetAbsOrigin() + Vector(0, 0, 16 * c.fraction), pPlayer->GetAbsOrigin(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_DEBRIS, &trace);
			if (trace.fraction >= 1)
			{
				CTFPlayer* attacker = ToTFPlayer(GetOwnerEntity());
				if (!attacker)
				{
					attacker = pPlayer;
				}
				if (!pPlayer->m_Shared.InCond( TF_COND_INVULNERABLE ))
				{
					CTakeDamageInfo info( this, attacker, vec3_origin, pPlayer->GetAbsOrigin(), flDamage, DMG_IGNITE | DMG_BURN | DMG_PREVENT_PHYSICS_FORCE );
					info.SetDamageCustom( TF_DMG_CUSTOM_NAPALM_BURNING );
					pPlayer->TakeDamage( info );
				}
			}
		}
	}
	SetNextThink(gpGlobals->curtime + 0.5);
}

#endif
