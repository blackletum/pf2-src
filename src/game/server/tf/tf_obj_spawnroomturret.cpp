//========= Copyright © Valve Corporation, All rights reserved. ============//
//
// Purpose: Spawnroom turret
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"

#include "tf_obj_spawnroomturret.h"
#include "engine/IEngineSound.h"
#include "tf_player.h"
#include "tf_team.h"
#include "world.h"
#include "te_effect_dispatch.h"
#include "tf_gamerules.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool IsInCommentaryMode();

// Ground placed version
//#define TURRET_MODEL			"models/combine_turrets/ceiling_turret.mdl"
#define TURRET_MODEL			"models/buildables/sentry1.mdl"
#define SENTRYGUN_MINS			Vector(-20, -20, 0)
#define SENTRYGUN_MAXS			Vector( 20,  20, 66)

#define SENTRYGUN_MAX_HEALTH	150

#define SENTRYGUN_ADD_SHELLS	40
#define SENTRYGUN_ADD_ROCKETS	8

#define SENTRY_THINK_DELAY	0.05

#define	SENTRYGUN_CONTEXT	"SentrygunContext"

enum target_ranges
{
	RANGE_MELEE,
	RANGE_NEAR,
	RANGE_MID,
	RANGE_FAR,
};

#define VECTOR_CONE_TF_SENTRY		Vector( 0.1, 0.1, 0 )

ConVar tf_spawnturret_notarget( "tf_spawnturret_notarget", "0", FCVAR_CHEAT );
ConVar tf_spawnturret_damage( "tf_spawnturret_damage", "60", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );
BEGIN_DATADESC( CTFSpawnroomTurret )
END_DATADESC()

LINK_ENTITY_TO_CLASS( srt, CTFSpawnroomTurret);
PRECACHE_REGISTER( srt );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFSpawnroomTurret::CTFSpawnroomTurret()
{
	SetMaxHealth( SENTRYGUN_MAX_HEALTH );
	m_iHealth = SENTRYGUN_MAX_HEALTH;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpawnroomTurret::Spawn()
{
	m_iPitchPoseParameter = -1;
	m_iYawPoseParameter = -1;

	SetModel( TURRET_MODEL );
	
	m_takedamage = DAMAGE_NO;

	SetMaxHealth( SENTRYGUN_MAX_HEALTH );
	SetHealth( SENTRYGUN_MAX_HEALTH );

	// Rotate Details
	m_iRightBound = 45;
	m_iLeftBound = 315;
	m_iBaseTurnRate = 8;
	m_flFieldOfView = VIEW_FIELD_NARROW;

	// Give the Gun some ammo
	m_iAmmoShells = 0;
	m_iMaxAmmoShells = SENTRYGUN_MAX_SHELLS_1;

	m_iAmmoType = GetAmmoDef()->Index( "TF_AMMO_PRIMARY" );

	// Start searching for enemies
	m_hEnemy = NULL;

	// Pipes explode when they hit this
	m_takedamage = DAMAGE_AIM;

	m_flLastAttackedTime = 0;

	BaseClass::Spawn();

	SetViewOffset( SENTRYGUN_EYE_OFFSET_LEVEL_1 );

	UTIL_SetSize( this, SENTRYGUN_MINS, SENTRYGUN_MAXS );

	m_iState.Set( SENTRY_STATE_SEARCHING );

	SetContextThink( &CTFSpawnroomTurret::SentryThink, gpGlobals->curtime + SENTRY_THINK_DELAY, SENTRYGUN_CONTEXT );
}

void CTFSpawnroomTurret::SentryThink( void )
{
	switch( m_iState )
	{
	case SENTRY_STATE_INACTIVE:
		break;

	case SENTRY_STATE_SEARCHING:
		SentryRotate();
		break;

	case SENTRY_STATE_ATTACKING:
		Attack();
		break;

	default:
		Assert( 0 );
		break;
	}

	SetContextThink( &CTFSpawnroomTurret::SentryThink, gpGlobals->curtime + SENTRY_THINK_DELAY, SENTRYGUN_CONTEXT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpawnroomTurret::Precache()
{
	BaseClass::Precache();
	// Models

	PrecacheModel( TURRET_MODEL );
	PrecacheModel( "models/effects/sentry1_muzzle/sentry1_muzzle.mdl" );

	// Sounds
	PrecacheScriptSound( "Building_Sentrygun.Fire" );
	PrecacheScriptSound( "Building_Sentrygun.Fire2" );	// level 2 sentry
	PrecacheScriptSound( "Building_Sentrygun.Fire3" );	// level 3 sentry
	PrecacheScriptSound( "Building_Sentrygun.FireRocket" );
	PrecacheScriptSound( "Building_Sentrygun.Alert" );
	PrecacheScriptSound( "Building_Sentrygun.AlertTarget" );
	PrecacheScriptSound( "Building_Sentrygun.Idle" );
	PrecacheScriptSound( "Building_Sentrygun.Idle2" );	// level 2 sentry
	PrecacheScriptSound( "Building_Sentrygun.Idle3" );	// level 3 sentry
	PrecacheScriptSound( "Building_Sentrygun.Built" );
	PrecacheScriptSound( "Building_Sentrygun.Empty" );
	PrecacheScriptSound("Building_Sentry.Damaged");

	PrecacheParticleSystem( "sentrydamage_1" );
	PrecacheParticleSystem( "sentrydamage_2" );
	PrecacheParticleSystem( "sentrydamage_3" );
	PrecacheParticleSystem( "sentrydamage_4" );
}

//-----------------------------------------------------------------------------
// Debug infos
//-----------------------------------------------------------------------------
int CTFSpawnroomTurret::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		Vector vecSrc = EyePosition();
		Vector forward;

		// m_vecCurAngles
		AngleVectors( m_vecCurAngles, &forward );
		NDebugOverlay::Line( vecSrc, vecSrc + forward * 200, 0, 255, 0, false, 0.1 );

		// m_vecGoalAngles
		AngleVectors( m_vecGoalAngles, &forward );
		NDebugOverlay::Line( vecSrc, vecSrc + forward * 200, 0, 0, 255, false, 0.1 );
	}

	return text_offset;
}

//-----------------------------------------------------------------------------
// Look for a target
//-----------------------------------------------------------------------------
bool CTFSpawnroomTurret::FindTarget()
{
	// Disable the sentry guns for ifm.
	if ( tf_spawnturret_notarget.GetBool() )
		return false;

	if ( IsInCommentaryMode() )
		return false;

	// Sapper, etc.
	/*if ( IsDisabled() )
		return false;*/

	// Loop through players within 1100 units (sentry range).
	Vector vecSentryOrigin = EyePosition();

	// Until I decide to change this, the spawn turret has to be on an actual team
	if (GetTeamNumber() < TF_TEAM_RED)
		return false;

	// Brain damage
	CTFTeam* pTeam = (CTFTeam*)GetGlobalTeam(GetTeamNumber() == TF_TEAM_RED ? TF_TEAM_BLUE : TF_TEAM_RED);
	if ( !pTeam )
		return false;

	// If we have an enemy get his minimum distance to check against.
	Vector vecSegment;
	Vector vecTargetCenter;
	float flMinDist2 = 1100.0f * 1100.0f;
	CBaseEntity *pTargetCurrent = NULL;
	CBaseEntity *pTargetOld = m_hEnemy.Get();
	float flOldTargetDist2 = FLT_MAX;

	// Sentries will try to target players first, then objects.  However, if the enemy held was an object it will continue
	// to try and attack it first.
	int nTeamCount = pTeam->GetNumPlayers();
	for ( int iPlayer = 0; iPlayer < nTeamCount; ++iPlayer )
	{
		CTFPlayer *pTargetPlayer = static_cast<CTFPlayer*>( pTeam->GetPlayer( iPlayer ) );
		if ( pTargetPlayer == NULL )
			continue;

		// Make sure the player is alive.
		if ( !pTargetPlayer->IsAlive() )
			continue;

		if ( pTargetPlayer->GetFlags() & FL_NOTARGET )
			continue;

		vecTargetCenter = pTargetPlayer->GetAbsOrigin();
		vecTargetCenter += pTargetPlayer->GetViewOffset();
		VectorSubtract( vecTargetCenter, vecSentryOrigin, vecSegment );
		float flDist2 = vecSegment.LengthSqr();

		// Store the current target distance if we come across it
		if ( pTargetPlayer == pTargetOld )
		{
			flOldTargetDist2 = flDist2;
		}

		// Check to see if the target is closer than the already validated target.
		if ( flDist2 > flMinDist2 )
			continue;

		// It is closer, check to see if the target is valid.
		if ( ValidTargetPlayer( pTargetPlayer, vecSentryOrigin, vecTargetCenter ) )
		{
			flMinDist2 = flDist2;
			pTargetCurrent = pTargetPlayer;
		}
	}

	// If we already have a target, don't check objects.
	if ( pTargetCurrent == NULL )
	{
		int nTeamObjectCount = pTeam->GetNumObjects();
		for ( int iObject = 0; iObject < nTeamObjectCount; ++iObject )
		{
			CBaseObject *pTargetObject = pTeam->GetObject( iObject );
			if ( !pTargetObject )
				continue;

			vecTargetCenter = pTargetObject->GetAbsOrigin();
			vecTargetCenter += pTargetObject->GetViewOffset();
			VectorSubtract( vecTargetCenter, vecSentryOrigin, vecSegment );
			float flDist2 = vecSegment.LengthSqr();

			// Store the current target distance if we come across it
			if ( pTargetObject == pTargetOld )
			{
				flOldTargetDist2 = flDist2;
			}

			// Check to see if the target is closer than the already validated target.
			if ( flDist2 > flMinDist2 )
				continue;

			// It is closer, check to see if the target is valid.
			if ( ValidTargetObject( pTargetObject, vecSentryOrigin, vecTargetCenter ) )
			{
				flMinDist2 = flDist2;
				pTargetCurrent = pTargetObject;
			}
		}
	}

	// We have a target.
	if ( pTargetCurrent )
	{
		if ( pTargetCurrent != pTargetOld )
		{
			// flMinDist2 is the new target's distance
			// flOldTargetDist2 is the old target's distance
			// Don't switch unless the new target is closer by some percentage
			if ( flMinDist2 < ( flOldTargetDist2 * 0.5f ) )
			{
				FoundTarget( pTargetCurrent, vecSentryOrigin );
			}
		}
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSpawnroomTurret::ValidTargetPlayer( CTFPlayer *pPlayer, const Vector &vecStart, const Vector &vecEnd )
{
	// Keep shooting at spies that go invisible after we acquire them as a target.
	if ( pPlayer->m_Shared.GetPercentInvisible() > 0.5 )
		return false;

	// Keep shooting at spies that disguise after we acquire them as at a target.
	if ( pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) && pPlayer->m_Shared.GetDisguiseTeam() == GetTeamNumber() && pPlayer != m_hEnemy )
		return false;

	// Not across water boundary.
	if ( ( GetWaterLevel() == 0 && pPlayer->GetWaterLevel() >= 3 ) || ( GetWaterLevel() == 3 && pPlayer->GetWaterLevel() <= 0 ) )
		return false;

	// Ray trace!!!
	return FVisible( pPlayer, MASK_SHOT | CONTENTS_GRATE );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFSpawnroomTurret::ValidTargetObject( CBaseObject *pObject, const Vector &vecStart, const Vector &vecEnd )
{
	// Ignore objects being placed, they are not real objects yet.
	if ( pObject->IsPlacing() )
		return false;

	// Ignore sappers.
	if ( pObject->MustBeBuiltOnAttachmentPoint() )
		return false;

	// Not across water boundary.
	if ( ( GetWaterLevel() == 0 && pObject->GetWaterLevel() >= 3 ) || ( GetWaterLevel() == 3 && pObject->GetWaterLevel() <= 0 ) )
		return false;

	// Ray trace.
	return FVisible( pObject, MASK_SHOT | CONTENTS_GRATE );
}

//-----------------------------------------------------------------------------
// Found a Target
//-----------------------------------------------------------------------------
void CTFSpawnroomTurret::FoundTarget( CBaseEntity *pTarget, const Vector &vecSoundCenter )
{
	m_hEnemy = pTarget;

	// Play one sound to everyone but the target.
	CPASFilter filter( vecSoundCenter );

	if ( pTarget->IsPlayer() )
	{
		CTFPlayer *pPlayer = ToTFPlayer( pTarget );

		// Play a specific sound just to the target and remove it from the genral recipient list.
		CSingleUserRecipientFilter singleFilter( pPlayer );
		EmitSound( singleFilter, entindex(), "Building_Sentrygun.AlertTarget" );
		filter.RemoveRecipient( pPlayer );
	}

	EmitSound( filter, entindex(), "Building_Sentrygun.Alert" );

	// Update timers, we are attacking now!
	m_iState.Set( SENTRY_STATE_ATTACKING );
	m_flNextAttack = gpGlobals->curtime + SENTRY_THINK_DELAY;
}

//-----------------------------------------------------------------------------
// FInViewCone - returns true is the passed ent is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//-----------------------------------------------------------------------------
bool CTFSpawnroomTurret::FInViewCone ( CBaseEntity *pEntity )
{
	Vector forward;
	AngleVectors( m_vecCurAngles, &forward );

	Vector2D vec2LOS = ( pEntity->GetAbsOrigin() - GetAbsOrigin() ).AsVector2D();
	vec2LOS.NormalizeInPlace();

	float flDot = vec2LOS.Dot( forward.AsVector2D() );

	if ( flDot > m_flFieldOfView )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
// Make sure our target is still valid, and if so, fire at it
//-----------------------------------------------------------------------------
void CTFSpawnroomTurret::Attack()
{
	StudioFrameAdvance( );

	if ( !FindTarget() )
	{
		m_iState.Set( SENTRY_STATE_SEARCHING );
		m_hEnemy = NULL;
		return;
	}

	// Track enemy
	Vector vecMid = EyePosition();
	Vector vecMidEnemy = m_hEnemy->WorldSpaceCenter();
	Vector vecDirToEnemy = vecMidEnemy - vecMid;

	QAngle angToTarget;
	VectorAngles( vecDirToEnemy, angToTarget );

	angToTarget.y = UTIL_AngleMod( angToTarget.y );
	if (angToTarget.x < -180)
		angToTarget.x += 360;
	if (angToTarget.x > 180)
		angToTarget.x -= 360;

	// now all numbers should be in [1...360]
	// pin to turret limitations to [-50...50]
	if (angToTarget.x > 50)
		angToTarget.x = 50;
	else if (angToTarget.x < -50)
		angToTarget.x = -50;
	m_vecGoalAngles.y = angToTarget.y;
	m_vecGoalAngles.x = angToTarget.x;

	MoveTurret();

	// Fire on the target if it's within 10 units of being aimed right at it
	if ( m_flNextAttack <= gpGlobals->curtime && (m_vecGoalAngles - m_vecCurAngles).Length() <= 10 )
	{
		Fire();
		m_flNextAttack = gpGlobals->curtime + 0.25;
	}
}

//-----------------------------------------------------------------------------
// Fire on our target
//-----------------------------------------------------------------------------
bool CTFSpawnroomTurret::Fire()
{
	//NDebugOverlay::Cross3D( m_hEnemy->WorldSpaceCenter(), 10, 255, 0, 0, false, 0.1 );

	Vector vecAimDir;

	// All turrets fire shells
	if ( !IsPlayingGesture( ACT_RANGE_ATTACK1 ) )
	{
		RemoveGesture( ACT_RANGE_ATTACK1_LOW );
		AddGesture( ACT_RANGE_ATTACK1 );
	}

	Vector vecSrc;
	QAngle vecAng;
	//Vector vec3_spread(RandomFloat(-0.1, 0.2), RandomFloat(-0.1, 0.2), RandomFloat(-0.1, 0.2));

	GetAttachment( LookupAttachment("muzzle"), vecSrc, vecAng );

	Vector vecMidEnemy = m_hEnemy->WorldSpaceCenter();

	// If we cannot see their WorldSpaceCenter ( possible, as we do our target finding based
	// on the eye position of the target ) then fire at the eye position
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecMidEnemy, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	if ( !tr.m_pEnt || tr.m_pEnt->IsWorld() )
	{
		// Hack it lower a little bit..
		// The eye position is not always within the hitboxes for a standing TF Player
		vecMidEnemy = m_hEnemy->EyePosition() + Vector(0,0,-5);
	}

	vecAimDir = vecMidEnemy - vecSrc;

	float flDistToTarget = vecAimDir.Length();

	vecAimDir.NormalizeInPlace();

	//NDebugOverlay::Cross3D( vecSrc, 10, 255, 0, 0, false, 0.1 );

	FireBulletsInfo_t info;

	info.m_vecSrc = vecSrc;
	info.m_vecDirShooting = vecAimDir;
	info.m_iTracerFreq = 1;
	info.m_iShots = 1;
	info.m_pAttacker = this;
	info.m_vecSpread = vec3_origin/*vec3_spread*/;
	info.m_flDistance = flDistToTarget + 100;
	info.m_iAmmoType = m_iAmmoType;
	info.m_flDamage = tf_spawnturret_damage.GetFloat();

	FireBullets( info );

	//NDebugOverlay::Line( vecSrc, vecSrc + vecAimDir * 1000, 255, 0, 0, false, 0.1 );

	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = LookupAttachment("muzzle");
	data.m_fFlags = 1;
	data.m_vOrigin = vecSrc;
	DispatchEffect( "TF_3rdPersonMuzzleFlash_SentryGun", data );
	EmitSound( "Building_Sentrygun.Fire" );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpawnroomTurret::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	trace_t tmptrace;
	tmptrace.endpos = tr.endpos + RandomVector(-10,10);

	// Sentryguns are perfectly accurate, but this doesn't look good for tracers.
	// Add a little noise to them, but not enough so that it looks like they're missing.
	BaseClass::MakeTracer( vecTracerSrc, tmptrace, iTracerType );
}

//-----------------------------------------------------------------------------
// Purpose: MakeTracer asks back for the attachment index
//-----------------------------------------------------------------------------
int	CTFSpawnroomTurret::GetTracerAttachment( void )
{
	return LookupAttachment("muzzle");
}

//-----------------------------------------------------------------------------
// Rotate and scan for targets
//-----------------------------------------------------------------------------
void CTFSpawnroomTurret::SentryRotate( void )
{
	// if we're playing a fire gesture, stop it
	if ( IsPlayingGesture( ACT_RANGE_ATTACK1 ) )
	{
		RemoveGesture( ACT_RANGE_ATTACK1 );
	}

	if ( IsPlayingGesture( ACT_RANGE_ATTACK1_LOW ) )
	{
		RemoveGesture( ACT_RANGE_ATTACK1_LOW );
	}

	// animate
	StudioFrameAdvance();

	// Look for a target
	if ( FindTarget() )
		return;

	// Rotate
	if ( !MoveTurret() )
	{
		// Change direction

		/*if ( IsDisabled() )
		{
			EmitSound( "Building_Sentrygun.Disabled" );
			m_vecGoalAngles.x = 30;
		}
		else*/
		{
			//EmitSound( "Building_Sentrygun.Idle" );

			// Switch rotation direction
			if ( m_bTurningRight )
			{
				m_bTurningRight = false;
				m_vecGoalAngles.y = m_iLeftBound;
			}
			else
			{
				m_bTurningRight = true;
				m_vecGoalAngles.y = m_iRightBound;
			}

			// Randomly look up and down a bit
			if (random->RandomFloat(0, 1) < 0.3)
			{
				m_vecGoalAngles.x = (int)random->RandomFloat(-10,10);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CTFSpawnroomTurret::GetBaseTurnRate( void )
{
	return m_iBaseTurnRate;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CTFSpawnroomTurret::MoveTurret( void )
{
	bool bMoved = false;

	int iBaseTurnRate = GetBaseTurnRate();

	// any x movement?
	if ( m_vecCurAngles.x != m_vecGoalAngles.x )
	{
		float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1 ;

		m_vecCurAngles.x += SENTRY_THINK_DELAY * ( iBaseTurnRate * 5 ) * flDir;

		// if we started below the goal, and now we're past, peg to goal
		if ( flDir == 1 )
		{
			if (m_vecCurAngles.x > m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		} 
		else
		{
			if (m_vecCurAngles.x < m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}

		SetPoseParameter( m_iPitchPoseParameter, -m_vecCurAngles.x );

		bMoved = true;
	}

	if ( m_vecCurAngles.y != m_vecGoalAngles.y )
	{
		float flDir = m_vecGoalAngles.y > m_vecCurAngles.y ? 1 : -1 ;
		float flDist = fabs( m_vecGoalAngles.y - m_vecCurAngles.y );
		bool bReversed = false;

		if ( flDist > 180 )
		{
			flDist = 360 - flDist;
			flDir = -flDir;
			bReversed = true;
		}

		if ( m_hEnemy.Get() == NULL )
		{
			if ( flDist > 30 )
			{
				if ( m_flTurnRate < iBaseTurnRate * 10 )
				{
					m_flTurnRate += iBaseTurnRate;
				}
			}
			else
			{
				// Slow down
				if ( m_flTurnRate > (iBaseTurnRate * 5) )
					m_flTurnRate -= iBaseTurnRate;
			}
		}
		else
		{
			// When tracking enemies, move faster and don't slow
			if ( flDist > 30 )
			{
				if (m_flTurnRate < iBaseTurnRate * 30)
				{
					m_flTurnRate += iBaseTurnRate * 3;
				}
			}
		}

		m_vecCurAngles.y += SENTRY_THINK_DELAY * m_flTurnRate * flDir;

		// if we passed over the goal, peg right to it now
		if (flDir == -1)
		{
			if ( (bReversed == false && m_vecGoalAngles.y > m_vecCurAngles.y) ||
				(bReversed == true && m_vecGoalAngles.y < m_vecCurAngles.y) )
			{
				m_vecCurAngles.y = m_vecGoalAngles.y;
			}
		} 
		else
		{
			if ( (bReversed == false && m_vecGoalAngles.y < m_vecCurAngles.y) ||
                (bReversed == true && m_vecGoalAngles.y > m_vecCurAngles.y) )
			{
				m_vecCurAngles.y = m_vecGoalAngles.y;
			}
		}

		if ( m_vecCurAngles.y < 0 )
		{
			m_vecCurAngles.y += 360;
		}
		else if ( m_vecCurAngles.y >= 360 )
		{
			m_vecCurAngles.y -= 360;
		}

		if ( flDist < ( SENTRY_THINK_DELAY * 0.5 * iBaseTurnRate ) )
		{
			m_vecCurAngles.y = m_vecGoalAngles.y;
		}

		QAngle angles = GetAbsAngles();

		float flYaw = m_vecCurAngles.y - angles.y;

		SetPoseParameter( m_iYawPoseParameter, -flYaw );

		InvalidatePhysicsRecursive( ANIMATION_CHANGED );

		bMoved = true;
	}

	if ( !bMoved || m_flTurnRate <= 0 )
	{
		m_flTurnRate = iBaseTurnRate * 5;
	}

	return bMoved;
}

//-----------------------------------------------------------------------------
// Purpose: Note our last attacked time
//-----------------------------------------------------------------------------
int CTFSpawnroomTurret::OnTakeDamage( const CTakeDamageInfo &info )
{
	// It doesn't take damage
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFSpawnroomTurret::SetModel( const char *pModel )
{
	float flPoseParam0 = 0.0;
	float flPoseParam1 = 0.0;

	// Save pose parameters across model change
	if ( m_iPitchPoseParameter >= 0 )
	{
		flPoseParam0 = GetPoseParameter( m_iPitchPoseParameter );
	}

	if ( m_iYawPoseParameter >= 0 )
	{
		flPoseParam1 = GetPoseParameter( m_iYawPoseParameter );
	}

	BaseClass::SetModel( pModel );

	// Reset this after model change
	UTIL_SetSize( this, SENTRYGUN_MINS, SENTRYGUN_MAXS );
	SetSolid( SOLID_BBOX );

	// Restore pose parameters
	m_iPitchPoseParameter = LookupPoseParameter( "aim_pitch" );
	m_iYawPoseParameter = LookupPoseParameter( "aim_yaw" );

	SetPoseParameter( m_iPitchPoseParameter, flPoseParam0 );
	SetPoseParameter( m_iYawPoseParameter, flPoseParam1 );

	ResetSequenceInfo();
}