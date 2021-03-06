//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// TF Nail Projectile
//
//=============================================================================
#ifndef TF_PROJECTILE_NAIL_H
#define TF_PROJECTILE_NAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tf_projectile_base.h"

//-----------------------------------------------------------------------------
// Purpose: Identical to a nail except for model used
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
#define CTFProjectile_Syringe C_TFProjectile_Syringe
#endif
class CTFProjectile_Syringe : public CTFBaseProjectile
{
	DECLARE_CLASS( CTFProjectile_Syringe, CTFBaseProjectile );
	DECLARE_NETWORKCLASS();

public:

	CTFProjectile_Syringe();
	~CTFProjectile_Syringe();
#ifdef GAME_DLL
	// Creation.
	static CTFProjectile_Syringe *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, CBaseEntity *pScorer = NULL, bool bCritical = false );
	virtual const char *GetProjectileModelName( void );
	virtual float	GetGravity( void );
	static float	GetInitialVelocity( void ) { return 1000.0; }
	virtual const char *GetImpactEffect(void) { return "SyringeImpact"; }
	virtual bool IsNail() { return true; }
#else
	virtual void CreateTrails( void );
#endif
};

//-----------------------------------------------------------------------------
// Purpose: Nail
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
#define CTFProjectile_Nail C_TFProjectile_Nail
#endif
class CTFProjectile_Nail : public CTFBaseProjectile
{
	DECLARE_CLASS( CTFProjectile_Nail, CTFBaseProjectile );
	DECLARE_NETWORKCLASS();

public:

	CTFProjectile_Nail();
	~CTFProjectile_Nail();
	// Creation.
#ifdef GAME_DLL
	static CTFProjectile_Nail* Create(const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity* pOwner = NULL, CBaseEntity* pScorer = NULL, bool bCritical = false);
	virtual const char *GetProjectileModelName( void );
	virtual float	GetGravity( void );
	static float	GetInitialVelocity( void ) { return 1000.0; }
	virtual const char *GetImpactEffect(void) { return "NailImpact"; }
	virtual bool IsNail() { return true; }
#else
	virtual void CreateTrails( void );
#endif

};

//-----------------------------------------------------------------------------
// Purpose: Tranq Dart
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
	#define CTFProjectile_Dart C_TFProjectile_Dart
#endif

class CTFProjectile_Dart : public CTFBaseProjectile
{
	DECLARE_CLASS( CTFProjectile_Dart, CTFBaseProjectile );
	DECLARE_NETWORKCLASS();

public:

	CTFProjectile_Dart();
	~CTFProjectile_Dart();
	// Creation.
#ifdef GAME_DLL
	static CTFProjectile_Dart* Create(const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity* pOwner = NULL, CBaseEntity* pScorer = NULL, bool bCritical = false);
	virtual const char* GetProjectileModelName( void );
	virtual float GetGravity( void );
	static float GetInitialVelocity( void ) { return 3000.0; }
	virtual void ProjectileTouch( CBaseEntity* pOther );
	virtual const char *GetImpactEffect(void) { return "DartImpact"; }
	virtual bool IsNail() { return true; }
#else
	virtual void CreateTrails( void );
#endif

	
};
#endif	//TF_PROJECTILE_NAIL_H