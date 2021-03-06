//========= Copyright © Valve Corporation, All rights reserved. ============//
//
// Purpose: Spawnroom turret
//
// $NoKeywords: $
//=============================================================================//

#ifndef TF_OBJ_SPAWNROOMTURRET_H
#define TF_OBJ_SPAWNROOMTURRET_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "tf_obj.h"

class CTFPlayer;

// ------------------------------------------------------------------------ //
// Spawnroom turret object
// ------------------------------------------------------------------------ //
class CTFSpawnroomTurret : public CBaseCombatCharacter
{
	DECLARE_CLASS( CTFSpawnroomTurret, CBaseCombatCharacter );

public:

	CTFSpawnroomTurret();

	static CTFSpawnroomTurret* Create(const Vector &vOrigin, const QAngle &vAngles);

	virtual void	Spawn();
	virtual void	Precache();
	virtual int		DrawDebugTextOverlays(void);
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual void	SetModel( const char *pModel );
	virtual int		GetTracerAttachment( void );

private:

	// Main think
	void SentryThink( void );

	// Target acquisition
	bool FindTarget( void );
	bool ValidTargetPlayer( CTFPlayer *pPlayer, const Vector &vecStart, const Vector &vecEnd );
	bool ValidTargetObject( CBaseObject *pObject, const Vector &vecStart, const Vector &vecEnd );
	void FoundTarget( CBaseEntity *pTarget, const Vector &vecSoundCenter );
	bool FInViewCone ( CBaseEntity *pEntity );

	// Rotations
	void SentryRotate( void );
	bool MoveTurret( void );

	// Attack
	void Attack( void );
	bool Fire( void );
	void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );

	int GetBaseTurnRate( void );
	
private:
	CNetworkVar( int, m_iState );

	float m_flNextAttack;

	// Rotation
	int m_iRightBound;
	int m_iLeftBound;
	int	m_iBaseTurnRate;
	bool m_bTurningRight;

	QAngle m_vecCurAngles;
	QAngle m_vecGoalAngles;

	float m_flTurnRate;

	// Ammo
	CNetworkVar( int, m_iAmmoShells );
	CNetworkVar( int, m_iMaxAmmoShells );

	int	m_iAmmoType;

	// Target player / object
	CHandle<CBaseEntity> m_hEnemy;

	int m_iPitchPoseParameter;
	int m_iYawPoseParameter;

	float m_flLastAttackedTime;

	int m_iPlacementBodygroup;

	DECLARE_DATADESC();
};
#endif