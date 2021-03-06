//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// TF Nail
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_tranq.h"
#include "tf_projectile_nail.h"

#ifdef CLIENT_DLL
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"
#include "c_te_effect_dispatch.h"
#include "input.h"
#include "c_tf_player.h"
#include "cliententitylist.h"
#else GAME_DLL
#include "tf_player.h"
#endif

#define SYRINGE_MODEL				"models/weapons/w_models/w_syringe_proj.mdl"
#define SYRINGE_DISPATCH_EFFECT		"ClientProjectile_Syringe"
#define SYRINGE_GRAVITY	0.3f

#define NAIL_MODEL				"models/weapons/w_models/w_nail.mdl"
#define NAIL_DISPATCH_EFFECT	"ClientProjectile_Nail"
#define NAIL_GRAVITY			0.3f

#define DART_MODEL				"models/weapons/w_models/w_dart_proj.mdl"
#define DART_DISPATCH_EFFECT	"ClientProjectile_Dart"
#define DART_GRAVITY			0.001f

//=============================================================================
//
// TF Syringe Projectile functions (Server specific).
//

LINK_ENTITY_TO_CLASS(tf_projectile_syringe, CTFProjectile_Syringe);
PRECACHE_REGISTER(tf_projectile_syringe);

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Syringe, DT_TFProjectile_Syringe )

BEGIN_NETWORK_TABLE( CTFProjectile_Syringe, DT_TFProjectile_Syringe )
END_NETWORK_TABLE()

short g_sModelIndexSyringe;
void PrecacheSyringe(void* pUser)
{
	g_sModelIndexSyringe = modelinfo->GetModelIndex(SYRINGE_MODEL);
}

PRECACHE_REGISTER_FN(PrecacheSyringe);

CTFProjectile_Syringe::CTFProjectile_Syringe()
{
}

CTFProjectile_Syringe::~CTFProjectile_Syringe()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

CTFProjectile_Syringe* CTFProjectile_Syringe::Create(const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity* pOwner, CBaseEntity* pScorer, bool bCritical)
{
	return static_cast<CTFProjectile_Syringe*>(CTFBaseProjectile::Create("tf_projectile_syringe", vecOrigin, vecAngles, pOwner, CTFProjectile_Syringe::GetInitialVelocity(), g_sModelIndexSyringe, pScorer, bCritical));
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char* CTFProjectile_Syringe::GetProjectileModelName( void )
{
	return SYRINGE_MODEL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFProjectile_Syringe::GetGravity( void )
{
	return SYRINGE_GRAVITY;
}
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Syringe::CreateTrails( void )
{
	if (IsDormant())
		return;

	if (GetTeamNumber() == TF_TEAM_BLUE)
	{
		ParticleProp()->Create( IsCritical() ? "nailtrails_medic_blue_crit" : "nailtrails_medic_blue", PATTACH_ABSORIGIN_FOLLOW );
	}
	else
	{
		ParticleProp()->Create( IsCritical() ? "nailtrails_medic_red_crit" : "nailtrails_medic_red", PATTACH_ABSORIGIN_FOLLOW );
	}

}
#endif

//=============================================================================
//
// TF Nail Projectile functions (Server specific).
//

LINK_ENTITY_TO_CLASS(tf_projectile_nail, CTFProjectile_Nail);
PRECACHE_REGISTER(tf_projectile_nail);

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Nail, DT_TFProjectile_Nail )

BEGIN_NETWORK_TABLE( CTFProjectile_Nail, DT_TFProjectile_Nail )
END_NETWORK_TABLE()

short g_sModelIndexNail;
void PrecacheNail(void* pUser)
{
	g_sModelIndexNail = modelinfo->GetModelIndex(NAIL_MODEL);
}

PRECACHE_REGISTER_FN(PrecacheNail);

CTFProjectile_Nail::CTFProjectile_Nail()
{
}

CTFProjectile_Nail::~CTFProjectile_Nail()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#ifdef GAME_DLL
CTFProjectile_Nail* CTFProjectile_Nail::Create(const Vector & vecOrigin, const QAngle & vecAngles, CBaseEntity * pOwner, CBaseEntity * pScorer, bool bCritical)
{
	return static_cast<CTFProjectile_Nail*>(CTFBaseProjectile::Create("tf_projectile_nail", vecOrigin, vecAngles, pOwner, CTFProjectile_Nail::GetInitialVelocity(), g_sModelIndexNail, pScorer, bCritical));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char* CTFProjectile_Nail::GetProjectileModelName( void )
{
	return NAIL_MODEL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFProjectile_Nail::GetGravity( void )
{
	return NAIL_GRAVITY;
}

#else
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Nail::CreateTrails( void )
{
	if (IsDormant())
		return;

	if (GetTeamNumber() == TF_TEAM_BLUE)
	{
		ParticleProp()->Create( IsCritical() ? "nailtrails_medic_blue_crit" : "nailtrails_medic_blue", PATTACH_ABSORIGIN_FOLLOW );
	}
	else
	{
		ParticleProp()->Create( IsCritical() ? "nailtrails_medic_red_crit" : "nailtrails_medic_red", PATTACH_ABSORIGIN_FOLLOW );
	}

}
#endif

//=============================================================================
//
// TF Dart Projectile functions (Server specific).
//

LINK_ENTITY_TO_CLASS(tf_projectile_dart, CTFProjectile_Dart);
PRECACHE_REGISTER(tf_projectile_dart);

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Dart, DT_TFProjectile_Dart )

BEGIN_NETWORK_TABLE( CTFProjectile_Dart, DT_TFProjectile_Dart )
END_NETWORK_TABLE()

short g_sModelIndexDart;
void PrecacheDart(void* pUser)
{
	g_sModelIndexDart = modelinfo->GetModelIndex(DART_MODEL);
}

PRECACHE_REGISTER_FN(PrecacheDart);

CTFProjectile_Dart::CTFProjectile_Dart()
{
}

CTFProjectile_Dart::~CTFProjectile_Dart()
{
#ifdef CLIENT_DLL
	ParticleProp()->StopEmission();
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#ifdef GAME_DLL
CTFProjectile_Dart* CTFProjectile_Dart::Create(const Vector & vecOrigin, const QAngle & vecAngles, CBaseEntity * pOwner, CBaseEntity * pScorer, bool bCritical)
{
	return static_cast<CTFProjectile_Dart*>(CTFBaseProjectile::Create("tf_projectile_dart", vecOrigin, vecAngles, pOwner, CTFProjectile_Dart::GetInitialVelocity(), g_sModelIndexDart, pScorer, bCritical));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char* CTFProjectile_Dart::GetProjectileModelName(void)
{
	return DART_MODEL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CTFProjectile_Dart::GetGravity(void)
{
	return DART_GRAVITY;
}

void CTFProjectile_Dart::ProjectileTouch( CBaseEntity * pOther )
{
	CTFPlayer* pPlayer = ToTFPlayer( pOther );
	if (pPlayer && pPlayer->GetTeamNumber() != GetOwnerEntity()->GetTeamNumber() && !pPlayer->m_Shared.InCond( TF_COND_INVULNERABLE ))
	{
		//if they're already tranquilized add 2 seconds onto the current duration
		if (pPlayer->m_Shared.InCond( TF_COND_TRANQUILIZED ))
			pPlayer->m_Shared.AddCond(
			TF_COND_TRANQUILIZED,
			Clamp<float>(
			pPlayer->m_Shared.GetConditionDuration( TF_COND_TRANQUILIZED ) + 2.0f, 0.0f, TRANQ_TIME
			) );
		else
			pPlayer->m_Shared.AddCond( TF_COND_TRANQUILIZED, TRANQ_TIME );
	}
	BaseClass::ProjectileTouch( pOther );
}
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Dart::CreateTrails( void )
{
	if (IsDormant())
		return;

	if (GetTeamNumber() == TF_TEAM_BLUE)
	{
		ParticleProp()->Create( "tranq_tracer_teamcolor_blue", PATTACH_ABSORIGIN_FOLLOW );
	}
	else
	{
		ParticleProp()->Create( "tranq_tracer_teamcolor_red", PATTACH_ABSORIGIN_FOLLOW );
	}

}
#endif