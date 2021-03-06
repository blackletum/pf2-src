//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Mirv Grenade.
//
//=============================================================================//
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "tf_weapon_grenade_mirv.h"

// Server specific.
#ifdef GAME_DLL
#include "tf_player.h"
#include "items.h"
#include "tf_weaponbase_grenadeproj.h"
#include "soundent.h"
#include "KeyValues.h"
#endif

#define GRENADE_MIRV_TIMER	3.0f // seconds
#define	GRENADE_MIRV_LEADIN	2.0f 

//=============================================================================
//
// TF Mirv Grenade tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFGrenadeMirv, DT_TFGrenadeMirv )

BEGIN_NETWORK_TABLE( CTFGrenadeMirv, DT_TFGrenadeMirv )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFGrenadeMirv )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_mirv, CTFGrenadeMirv );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_mirv );

IMPLEMENT_NETWORKCLASS_ALIASED(TFGrenadeMirvProjectile, DT_TFGrenadeMirvProjectile)

BEGIN_NETWORK_TABLE(CTFGrenadeMirvProjectile, DT_TFGrenadeMirvProjectile)
END_NETWORK_TABLE()

//=============================================================================
//
// TF Normal Grenade functions.
//

CTFGrenadeMirvProjectile::CTFGrenadeMirvProjectile()
{
}
CTFGrenadeMirvProjectile::~CTFGrenadeMirvProjectile()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

//=============================================================================
//
// TF Mirv Grenade functions.
//

// Server specific.
#ifdef GAME_DLL

BEGIN_DATADESC( CTFGrenadeMirv )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFWeaponBaseGrenadeProj *CTFGrenadeMirv::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, 
							      AngularImpulse angImpulse, CBasePlayer *pPlayer, float flTime, int iflags )
{
	return CTFGrenadeMirvProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse,
									  pPlayer, GetTFWpnData(), flTime );
}

#endif

/*const char *CTFGrenadeMirv::GetViewModel(int iViewModel) const
{
	if (!GetTFPlayerOwner())
		return BaseClass::GetViewModel();

	//if (GetTFPlayerOwner()->IsPlayerClass(TF_CLASS_HEAVYWEAPONS))
	//	return "models/weapons/v_models/v_grenade_mirv_heavy.mdl";

	return "models/weapons/v_models/v_grenade_mirv_demo.mdl";
}*/

//=============================================================================
//
// TF Mirv Grenade Projectile functions (Server specific).
//
#ifdef GAME_DLL

BEGIN_DATADESC( CTFGrenadeMirvProjectile )
DEFINE_THINKFUNC( DetonateThink ),
END_DATADESC()

#define GRENADE_MODEL "models/weapons/w_models/w_grenade_mirv.mdl"
//#define GRENADE_VM_HEAVY "models/weapons/v_models/v_grenade_mirv_heavy.mdl"

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_mirv_projectile, CTFGrenadeMirvProjectile );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_mirv_projectile );

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeMirvProjectile* CTFGrenadeMirvProjectile::Create( const Vector &position, const QAngle &angles, 
															const Vector &velocity, const AngularImpulse &angVelocity, 
															CBaseCombatCharacter *pOwner, const CTFWeaponInfo &weaponInfo, float timer, int iFlags )
{
	CTFGrenadeMirvProjectile *pGrenade = static_cast<CTFGrenadeMirvProjectile*>( CTFWeaponBaseGrenadeProj::Create( "tf_weapon_grenade_mirv_projectile", position, angles, velocity, angVelocity, pOwner, weaponInfo, timer, iFlags ) );
	return pGrenade;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvProjectile::Spawn()
{
	SetModel( GRENADE_MODEL );

	BaseClass::Spawn();

	m_bPlayedLeadIn = false;

	SetThink( &CTFGrenadeMirvProjectile::DetonateThink );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvProjectile::Precache()
{
	PrecacheModel( GRENADE_MODEL );
	//PrecacheModel( GRENADE_VM_HEAVY );
	PrecacheScriptSound( "Weapon_Grenade_Mirv.LeadIn" );
	PrecacheScriptSound( "Weapon_Grenade_Mirv.Fuse" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvProjectile::BounceSound( void )
{
	EmitSound( "Weapon_Grenade_Mirv.Bounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvProjectile::Detonate()
{
	if ( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	BaseClass::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvProjectile::DetonateThink( void )
{
	if ( !m_bPlayedLeadIn && gpGlobals->curtime > GetDetonateTime() - GRENADE_MIRV_LEADIN )
	{
		Vector soundPosition = GetAbsOrigin() + Vector( 0, 0, 5 );
		CPASAttenuationFilter filter( soundPosition );

		EmitSound( filter, entindex(), "Weapon_Grenade_Mirv.LeadIn" );
		m_bPlayedLeadIn = true;
	}

	BaseClass::DetonateThink();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvProjectile::Explode( trace_t *pTrace, int bitsDamageType )
{
	// Pass through.
	BaseClass::Explode( pTrace, bitsDamageType );

	m_bPlayedLeadIn = false;

// Server specific.
#ifdef GAME_DLL

	// Create the bomblets.
	for ( int iBomb = 0; iBomb < TF_WEAPON_GRENADE_MIRV_BOMB_COUNT; ++iBomb )
	{
		Vector vecSrc = pTrace->endpos + Vector( 0, 0, 1.0f ); 
		Vector vecVelocity( random->RandomFloat( -75.0f, 75.0f ) * 3.0f,
			                random->RandomFloat( -75.0f, 75.0f ) * 3.0f,
							random->RandomFloat( 30.0f, 70.0f ) * 5.0f );
		Vector vecZero( 0,0,0 );
		CTFPlayer *pPlayer = ToTFPlayer( GetThrower() );
		float flTime = 2.0f + random->RandomFloat( 0.0f, 1.0f );

		CTFGrenadeMirvBomb::Create( vecSrc, QAngle(0,90,0), vecVelocity, vecZero, pPlayer, flTime );
	}

#endif
}

//=============================================================================
//
// TF Mirv Bomb functions (Server specific).
//

#define GRENADE_MODEL_BOMBLET "models/weapons/w_models/w_grenade_bomblet.mdl"

#define TF_WEAPON_GRENADE_MIRV_BOMB_GRAVITY		0.5f
#define TF_WEAPON_GRENADE_MIRV_BOMB_FRICTION	0.8f
#define TF_WEAPON_GRENADE_MIRV_BOMB_ELASTICITY	0.45f

LINK_ENTITY_TO_CLASS( tf_weapon_grenade_mirv_bomb, CTFGrenadeMirvBomb );
PRECACHE_WEAPON_REGISTER( tf_weapon_grenade_mirv_bomb );

CTFGrenadeMirvBomb::CTFGrenadeMirvBomb()
{
}

CTFGrenadeMirvBomb::~CTFGrenadeMirvBomb()
{
	StopSound( "Weapon_Grenade_Mirv.Fuse" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFGrenadeMirvBomb *CTFGrenadeMirvBomb::Create( const Vector &position, const QAngle &angles, const Vector &velocity, 
							                    const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, float timer )
{
	CTFGrenadeMirvBomb *pBomb = static_cast<CTFGrenadeMirvBomb*>( CBaseEntity::Create( "tf_weapon_grenade_mirv_bomb", position, angles, pOwner ) );
	if ( pBomb )
	{
		pBomb->SetDetonateTimerLength( timer );
		pBomb->SetupInitialTransmittedGrenadeVelocity( velocity );
		pBomb->SetThrower( pOwner ); 
		pBomb->SetOwnerEntity( NULL );

		pBomb->SetGravity( TF_WEAPON_GRENADE_MIRV_BOMB_GRAVITY );
		pBomb->SetFriction( TF_WEAPON_GRENADE_MIRV_BOMB_GRAVITY );
		pBomb->SetElasticity( TF_WEAPON_GRENADE_MIRV_BOMB_ELASTICITY );

		pBomb->m_flDamage = GetTFWeaponInfo(TF_WEAPON_GRENADE_MIRV)->GetWeaponData(TF_WEAPON_SECONDARY_MODE).m_nDamage;
		pBomb->m_DmgRadius = GetTFWeaponInfo(TF_WEAPON_GRENADE_MIRV)->m_flDamageRadius;

		pBomb->ChangeTeam( pOwner->GetTeamNumber() );

		pBomb->SetCollisionGroup( TF_COLLISIONGROUP_GRENADES );

		IPhysicsObject *pPhysicsObject = pBomb->VPhysicsGetObject();
		if ( pPhysicsObject )
		{
			pPhysicsObject->AddVelocity( &velocity, &angVelocity );
		}
	}

	return pBomb;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvBomb::Spawn()
{
	SetModel( GRENADE_MODEL_BOMBLET );

	BaseClass::Spawn();

	EmitSound( "Weapon_Grenade_Mirv.Fuse" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvBomb::Precache()
{
	PrecacheModel( GRENADE_MODEL_BOMBLET );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvBomb::BounceSound( void )
{
	EmitSound( "Weapon_Grenade_MirvBomb.Bounce" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFGrenadeMirvBomb::Detonate()
{
	StopSound( "Weapon_Grenade_Mirv.Fuse" );
	if( ShouldNotDetonate() )
	{
		RemoveGrenade();
		return;
	}

	BaseClass::Detonate();
}

#endif
