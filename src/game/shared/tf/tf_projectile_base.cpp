//====== Copyright  1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TF Base Rockets.
//
//=============================================================================//
#include "cbase.h"
#include "tf_projectile_base.h"
#include "effect_dispatch_data.h"
#include "tf_shareddefs.h"

#ifdef GAME_DLL
#include "te_effect_dispatch.h"
#else
#include "c_te_effect_dispatch.h"
#endif
#ifdef CLIENT_DLL
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"
#include "c_te_effect_dispatch.h"
#include "input.h"
#include "c_tf_player.h"
#else
#include "tf_player.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( TFBaseProjectile, DT_TFBaseProjectile )

BEGIN_NETWORK_TABLE( CTFBaseProjectile, DT_TFBaseProjectile )
#ifdef CLIENT_DLL
	RecvPropVector( RECVINFO( m_vInitialVelocity ) )
#else
	SendPropVector( SENDINFO( m_vInitialVelocity ), 20 /*nbits*/, 0 /*flags*/, -3000 /*low value*/, 3000 /*high value*/	)
#endif
END_NETWORK_TABLE()

// Server specific.
#ifdef GAME_DLL

BEGIN_DATADESC( CTFBaseProjectile )
	DEFINE_ENTITYFUNC( ProjectileTouch ),
	DEFINE_THINKFUNC( FlyThink ),
END_DATADESC()

#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CTFBaseProjectile::CTFBaseProjectile()
{
	m_vInitialVelocity.Init();

	SetWeaponID( TF_WEAPON_NONE );

	// Client specific.
#ifdef CLIENT_DLL

	m_flSpawnTime = 0.0f;
	m_bCreated = false;

	// Server specific.
#else

	m_flDamage = 0.0f;

#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor.
//-----------------------------------------------------------------------------
CTFBaseProjectile::~CTFBaseProjectile()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBaseProjectile::Precache( void )
{
#ifdef GAME_DLL
	PrecacheModel( GetProjectileModelName() );
#endif
	BaseClass::Precache();
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBaseProjectile::Spawn( void )
{
	// Client specific.
#ifdef CLIENT_DLL

	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();

	// Server specific.
#else

	// Precache.
	Precache();

	SetModel( GetProjectileModelName() );

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	AddEFlags( EFL_NO_WATER_VELOCITY_CHANGE );
	AddEffects( EF_NOSHADOW );

	UTIL_SetSize( this, -Vector( 1.0f, 1.0f, 1.0f ), Vector( 1.0f, 1.0f, 1.0f ) );

	// Setup attributes.
	SetGravity( GetGravity() );
	m_takedamage = DAMAGE_NO;
	SetDamage( 25.0f );

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	// Setup the touch and think functions.
	SetTouch( &CTFBaseProjectile::ProjectileTouch );
	SetThink( &CTFBaseProjectile::FlyThink );
	SetNextThink( gpGlobals->curtime );
#endif
}

const char *CTFBaseProjectile::GetProjectileModelName( void )
{
	// should not try to init a base projectile
	Assert( 0 );
	return "";
}

//=============================================================================
//
// Client specific functions.
//
#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBaseProjectile::PostDataUpdate( DataUpdateType_t type )
{
	// Pass through to the base class.
	BaseClass::PostDataUpdate( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		// Now stick our initial velocity and angles into the interpolation history.
		CInterpolatedVar<Vector> &interpolator = GetOriginInterpolator();
		interpolator.ClearHistory();

		CInterpolatedVar<QAngle> &rotInterpolator = GetRotationInterpolator();
		rotInterpolator.ClearHistory();

		float flChangeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

		// Add a sample 1 second back.
		Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
		interpolator.AddToHead( flChangeTime - 1.0f, &vCurOrigin, false );

		QAngle vCurAngles = GetLocalAngles();
		rotInterpolator.AddToHead( flChangeTime - 1.0f, &vCurAngles, false );

		// Add the current sample.
		vCurOrigin = GetLocalOrigin();
		interpolator.AddToHead( flChangeTime, &vCurOrigin, false );

		rotInterpolator.AddToHead( flChangeTime - 1.0, &vCurAngles, false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFBaseProjectile::DrawModel( int flags )
{
	// During the first 0.2 seconds of our life, don't draw ourselves.
	if ( gpGlobals->curtime - m_flSpawnTime < 0.05f )
		return 0;

	m_bModelVisible = true;
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBaseProjectile::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	//if (updateType == DATA_UPDATE_CREATED && )
	//{
	//	m_bCreated = true;
	//}
	if (!m_bCreated && m_bModelVisible )
	{
		m_bCreated = true;
		CreateTrails();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBaseProjectile::CreateTrails( void )
{
	if (IsDormant())
		return;

	Assert( 0 );
}

//=============================================================================
//
// Server specific functions.
//
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFBaseProjectile *CTFBaseProjectile::Create( const char *pszClassname, const Vector &vecOrigin, 
											 const QAngle &vecAngles, CBaseEntity *pOwner, float flVelocity, short iProjModelIndex,
											CBaseEntity *pScorer, bool bCritical )
{
	CTFBaseProjectile *pProjectile = NULL;

	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

	Vector vecVelocity = vecForward * flVelocity;

	pProjectile = static_cast<CTFBaseProjectile*>( CBaseEntity::Create( pszClassname, vecOrigin, vecAngles, pOwner ) );
	if ( !pProjectile )
		return NULL;

	// Initialize the owner.
	pProjectile->SetOwnerEntity( pOwner );

	pProjectile->SetScorer( pScorer );

	// Spawn.
	pProjectile->Spawn();

	pProjectile->SetAbsVelocity( vecVelocity );	
	pProjectile->SetupInitialTransmittedGrenadeVelocity( vecVelocity );

	// Setup the initial angles.
	QAngle angles;
	VectorAngles( vecVelocity, angles );
	pProjectile->SetAbsAngles( angles );

	// Set team.
	pProjectile->ChangeTeam( pOwner->GetTeamNumber() );
	pProjectile->m_nSkin = pOwner->GetTeamNumber() == TF_TEAM_RED ? 0 : 1;

	return pProjectile;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
unsigned int CTFBaseProjectile::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBaseProjectile::ProjectileTouch( CBaseEntity *pOther )
{
	// Verify a correct "other."
	Assert( pOther );
	if ( pOther->IsSolidFlagSet( FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS ) )
		return;

	// Handle hitting skybox (disappear).
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	trace_t *pNewTrace = const_cast<trace_t*>( pTrace );

	if( pTrace->surface.flags & SURF_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	// pass through ladders
	if( pTrace->surface.flags & CONTENTS_LADDER )
		return;

	// Invisible.
	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_takedamage = DAMAGE_NO;

	// determine the inflictor, which is the weapon which fired this projectile
	CBaseEntity *pInflictor = NULL;
	CBaseEntity *pOwner = GetScorer();
	CTFPlayer *pTFPlayer = nullptr;
	if ( pOwner )
	{
		pTFPlayer = ToTFPlayer( pOwner );
		if ( pTFPlayer )
		{
			pInflictor = pTFPlayer->Weapon_OwnsThisID( GetWeaponID() );
		}
	}

	CTakeDamageInfo info;
	info.SetAttacker( GetScorer() );		// the player who operated the thing that emitted nails
	info.SetInflictor( pInflictor );	// the weapon that emitted this projectile
	info.SetDamage( GetDamage() );
	info.SetDamageForce( GetDamageForce() );
	info.SetDamagePosition( GetAbsOrigin() );
	info.SetDamageType( GetDamageType() );

	Vector dir;
	AngleVectors( GetAbsAngles(), &dir );

	pOther->DispatchTraceAttack( info, dir, pNewTrace );
	ApplyMultiDamage();

	if (IsNail() ? pOther->IsSolid() : pOther->IsWorld() )
	{
		SetThink(&CTFBaseProjectile::SUB_Remove);
		SetNextThink(gpGlobals->curtime + 2.0f);

		//FIXME: We actually want to stick (with hierarchy) to what we've hit
		SetMoveType(MOVETYPE_NONE);

		Vector vForward;

		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);

		CEffectData	data;

		data.m_vOrigin = pNewTrace->endpos;
		data.m_vNormal = vForward;
		data.m_nEntIndex = 0;
		data.m_nMaterial = pOwner ? pOwner->GetTeamNumber() : 0;

		if ( IsNail() )
		{
			if ( pOther->IsWorld() )
			{
				DispatchEffect( GetImpactEffect(), data );
			}
		}
		else
			DispatchEffect(GetImpactEffect(), data);

		UTIL_ImpactTrace(pNewTrace, DMG_BULLET);

		AddEffects(EF_NODRAW);
		SetTouch(NULL);
		SetThink(&CTFBaseProjectile::SUB_Remove);
		SetNextThink(gpGlobals->curtime + 2.0f);
	}

	UTIL_Remove( this );
}

Vector CTFBaseProjectile::GetDamageForce( void )
{
	Vector vecVelocity = GetAbsVelocity();
	VectorNormalize( vecVelocity );
	return (vecVelocity * GetDamage());
}

void CTFBaseProjectile::FlyThink( void )
{
	QAngle angles;

	VectorAngles( GetAbsVelocity(), angles );

	SetAbsAngles( angles );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CTFBaseProjectile::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

CBasePlayer *CTFBaseProjectile::GetScorer( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTFBaseProjectile::GetDamageType( void )
{
	Assert( GetWeaponID() != TF_WEAPON_NONE );
	int iDmgType = g_aWeaponDamageTypes[ GetWeaponID() ];
	if ( m_bCritical )
	{
		iDmgType |= DMG_CRITICAL;
	}
	return iDmgType;
}

#endif
