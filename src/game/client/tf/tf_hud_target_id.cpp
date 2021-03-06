//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: HUD Target ID element
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tf_hud_target_id.h"
#include "c_tf_playerresource.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "c_baseobject.h"
#include "c_team.h"
#include "tf_gamerules.h"
#include "tf_hud_statpanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT( CMainTargetID );
DECLARE_HUDELEMENT( CSpectatorTargetID );
DECLARE_HUDELEMENT( CSecondaryTargetID );

using namespace vgui;

extern ConVar pf_armor_enabled;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTargetID::CTargetID( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_hFont = g_hFontTrebuchet24;
	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	SetHiddenBits( HIDEHUD_MISCSTATUS );

	m_pTargetNameLabel = NULL;
	m_pTargetDataLabel = NULL;
	m_pTargetNameArmorLabel = NULL;
	m_pTargetDataArmorLabel = NULL;
	m_pBGPanel = NULL;
	m_pTargetHealth = new CTFSpectatorGUIHealth( this, "SpectatorGUIHealth" );
	m_pTargetArmor = new CTFSpectatorGUIArmor( this, "SpectatorGUIArmor" );
	m_bLayoutOnUpdate = false;
	m_bShowArmor = false;

	RegisterForRenderGroup( "mid" );
	RegisterForRenderGroup( "commentary" );

	m_iRenderPriority = 5;
}

//-----------------------------------------------------------------------------
// Purpose: Setup
//-----------------------------------------------------------------------------
void CTargetID::Reset( void )
{
	m_pTargetHealth->Reset();
	m_pTargetArmor->Reset();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTargetID::ApplySchemeSettings( vgui::IScheme *scheme )
{
	LoadControlSettings( "resource/UI/TargetID.res" );

	BaseClass::ApplySchemeSettings( scheme );

	m_pTargetNameLabel = dynamic_cast<Label *>(FindChildByName("TargetNameLabel"));
	m_pTargetDataLabel = dynamic_cast<Label *>(FindChildByName("TargetDataLabel"));
	m_pTargetNameArmorLabel = dynamic_cast<Label *>(FindChildByName("TargetNameArmorLabel"));
	m_pTargetDataArmorLabel = dynamic_cast<Label *>(FindChildByName("TargetDataArmorLabel"));

	m_pBGPanel = dynamic_cast<CTFImagePanel*>(FindChildByName("TargetIDBG"));
	m_cBlueColor = scheme->GetColor( "TeamBlue", Color( 255, 64, 64, 255 ) );
	m_cRedColor = scheme->GetColor( "TeamRed", Color( 255, 64, 64, 255 ) );
	m_cSpecColor = scheme->GetColor( "TeamSpec", Color( 255, 160, 0, 255 ) );
	m_hFont = scheme->GetFont( "TargetID", true );

	//SetPaintBackgroundEnabled( true );
	//SetBgColor( Color( 0, 0, 0, 90 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTargetID::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	m_iRenderPriority = inResourceData->GetInt( "priority" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CTargetID::GetRenderGroupPriority( void )
{
	return m_iRenderPriority;
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CTargetID::VidInit()
{
	CHudElement::VidInit();

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTargetID::ShouldDraw( void )
{
	if ( !CHudElement::ShouldDraw() )
		return false;

	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalTFPlayer )
		return false;

	// Get our target's ent index
	m_iTargetEntIndex = CalculateTargetIndex(pLocalTFPlayer);
	if ( !m_iTargetEntIndex )
	{
		// Check to see if we should clear our ID
		if ( m_flLastChangeTime && ( gpGlobals->curtime > m_flLastChangeTime ) )
		{
			m_flLastChangeTime = 0;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			m_iTargetEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	bool bReturn = false;
	if ( m_iTargetEntIndex )
	{
		C_BaseEntity *pEnt = cl_entitylist->GetEnt( m_iTargetEntIndex );
		if ( pEnt )
		{
			if ( IsPlayerIndex( m_iTargetEntIndex ) )
			{
				C_TFPlayer *pPlayer = static_cast<C_TFPlayer*>( pEnt );
				if (pPlayer && !pPlayer->InSameTeam(pLocalTFPlayer) && (pPlayer->m_Shared.InCond(TF_COND_SMOKE_BOMB) || pPlayer->m_Shared.InCond(TF_COND_STEALTHED)))
					return bReturn;
				bool bDisguisedEnemy = false;
				if ( pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) && // they're disguised
					!pPlayer->m_Shared.InCond( TF_COND_DISGUISING ) && // they're not in the process of disguising
					!(pPlayer->m_Shared.InCond( TF_COND_STEALTHED ) ||
					pPlayer->m_Shared.InCond( TF_COND_SMOKE_BOMB ))) // they're not cloaked
				{
					bDisguisedEnemy = (ToTFPlayer( pPlayer->m_Shared.GetDisguiseTarget() ) != NULL);
				}

				bReturn = ( pLocalTFPlayer->GetTeamNumber() == TEAM_SPECTATOR || pLocalTFPlayer->InSameTeam( pEnt ) || bDisguisedEnemy || pLocalTFPlayer->IsPlayerClass(TF_CLASS_SPY) );
			}
			else if ( pEnt->IsBaseObject() && ( pLocalTFPlayer->InSameTeam( pEnt ) || pLocalTFPlayer->IsPlayerClass(TF_CLASS_SPY) ) )
			{
				bReturn = true;
			}
		}
	}

	if ( bReturn )
	{
		if ( !IsVisible() || (m_iTargetEntIndex != m_iLastEntIndex) )
		{
			m_iLastEntIndex = m_iTargetEntIndex;
			m_bLayoutOnUpdate = true;
		}

		UpdateID();
	}

	return bReturn;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTargetID::PerformLayout( void )
{
	int iXIndent = XRES(5);
	int iXPostdent = XRES(10);
	int iWidth;

	m_pTargetNameLabel->SetVisible( !m_bShowArmor );
	m_pTargetDataLabel->SetVisible( !m_bShowArmor );
	m_pTargetNameArmorLabel->SetVisible( m_bShowArmor );
	m_pTargetDataArmorLabel->SetVisible( m_bShowArmor );

	int iTextW, iTextH;
	int iDataW, iDataH;
	if ( m_bShowArmor )
	{
		iWidth = m_pTargetHealth->GetWide() + m_pTargetArmor->GetWide() + iXIndent + iXPostdent;
		m_pTargetNameArmorLabel->GetContentSize( iTextW, iTextH );
		m_pTargetDataArmorLabel->GetContentSize( iDataW, iDataH );
	}
	else
	{
		iWidth = m_pTargetHealth->GetWide() + iXIndent + iXPostdent;
		m_pTargetNameLabel->GetContentSize( iTextW, iTextH );
		m_pTargetDataLabel->GetContentSize( iDataW, iDataH );
	}

	iWidth += max(iTextW,iDataW);

	SetSize( iWidth, GetTall() );

	int iX,iY;
	GetPos( iX, iY );
	SetPos( (ScreenWidth() - iWidth) * 0.5, iY );

	if ( m_pBGPanel )
	{
		m_pBGPanel->SetSize( iWidth, GetTall() );
	}
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Color CTargetID::GetColorForTargetTeam( int iTeamNumber )
{
	switch( iTeamNumber )
	{
	case TF_TEAM_BLUE:
		return m_cBlueColor;
		break;

	case TF_TEAM_RED:
		return m_cRedColor;
		break;

	default:
		return m_cSpecColor;
		break;
	}
} 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTargetID::CalculateTargetIndex( C_TFPlayer *pLocalTFPlayer ) 
{ 
	int iIndex = pLocalTFPlayer->GetIDTarget(); 

	// If our target entity is already in our secondary ID, don't show it in primary.
	CSecondaryTargetID *pSecondaryID = GET_HUDELEMENT( CSecondaryTargetID );
	if ( pSecondaryID && pSecondaryID != this && pSecondaryID->GetTargetIndex() == iIndex )
	{
		iIndex = 0;
	}

	return iIndex;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTargetID::UpdateID( void )
{
	wchar_t sIDString[ MAX_ID_STRING ] = L"";
	wchar_t sDataString[ MAX_ID_STRING ] = L"";

	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalTFPlayer )
		return;

	// Get our target's ent index
	Assert( m_iTargetEntIndex );

	// Is this an entindex sent by the server?
	if ( m_iTargetEntIndex )
	{
		C_BaseEntity *pEnt = cl_entitylist->GetEnt( m_iTargetEntIndex );
		if ( !pEnt )
			return;

		bool bShowHealth = false;
		float flHealth = 0;
		float flMaxHealth = 1;
		int iMaxBuffedHealth = 0;

		float flArmor = 0;
		float flMaxArmor = 1;

		// Some entities we always want to check, cause the text may change
		// even while we're looking at it
		// Is it a player?
		if ( IsPlayerIndex( m_iTargetEntIndex ) )
		{
			const char *printFormatString = NULL;
			wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];
			bool bDisguisedTarget = false;
			bool bDisguisedEnemy = false;

			m_bShowArmor = TFGameRules()->GetArmorMode();

			C_TFPlayer *pPlayer = static_cast<C_TFPlayer*>( pEnt );
			if ( !pPlayer )
				return;

			C_TFPlayer *pDisguiseTarget = NULL;
			g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );

			// determine if the target is a disguised spy (either friendly or enemy)
			if ( pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) && // they're disguised
				//!pPlayer->m_Shared.InCond( TF_COND_DISGUISING ) && // they're not in the process of disguising
				(!pPlayer->m_Shared.InCond( TF_COND_STEALTHED ) ||
				!pPlayer->m_Shared.InCond( TF_COND_SMOKE_BOMB ))) // they're not cloaked
			{
				bDisguisedTarget = true;
				pDisguiseTarget = ToTFPlayer( pPlayer->m_Shared.GetDisguiseTarget() );
			}

			if ( bDisguisedTarget )
			{
				// is the target a disguised enemy spy?
				if ( pPlayer->IsEnemyPlayer() )
				{
					if ( pDisguiseTarget )
					{
						bDisguisedEnemy = true;
						// change the player name
						g_pVGuiLocalize->ConvertANSIToUnicode( pDisguiseTarget->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );
						// change the team  / team color
					}
				}
				else
				{
					// The target is a disguised friendly spy.  They appear to the player with no disguise.  Add the disguise
					// team & class to the target ID element.
					bool bDisguisedAsEnemy = ( pPlayer->m_Shared.GetDisguiseTeam() != pPlayer->GetTeamNumber() );
					const wchar_t *wszAlignment = g_pVGuiLocalize->Find( bDisguisedAsEnemy ? "#TF_enemy" : "#TF_friendly" );
					
					int classindex = pPlayer->m_Shared.GetDisguiseClass();
					const wchar_t *wszClassName = g_pVGuiLocalize->Find( g_aPlayerClassNames[classindex] );

					// build a string with disguise information
					g_pVGuiLocalize->ConstructString( sDataString, sizeof(sDataString), g_pVGuiLocalize->Find( "#TF_playerid_friendlyspy_disguise" ), 
						2, wszAlignment, wszClassName );
				}
			}

			if ( pPlayer->IsPlayerClass( TF_CLASS_MEDIC ) )
			{
				wchar_t wszChargeLevel[ 10 ];
				_snwprintf( wszChargeLevel, ARRAYSIZE(wszChargeLevel) - 1, L"%.0f", pPlayer->MedicGetChargeLevel() * 100 );
				wszChargeLevel[ ARRAYSIZE(wszChargeLevel)-1 ] = '\0';
				g_pVGuiLocalize->ConstructString( sDataString, sizeof(sDataString), g_pVGuiLocalize->Find( "#TF_playerid_mediccharge" ), 1, wszChargeLevel );
			}

			int iTeamNum;
			if ( bDisguisedEnemy )
				iTeamNum = pDisguiseTarget->GetTeamNumber();
			else
				iTeamNum = pPlayer->GetTeamNumber();

			m_pBGPanel->SetImage( m_pBGPanel->m_szTeamBG[ iTeamNum ] );
			m_pTargetArmor->SetArmorImageTeam( iTeamNum, pPlayer->GetPlayerClass()->GetArmorType() );
			
			if ( pLocalTFPlayer->GetTeamNumber() == TEAM_SPECTATOR || pPlayer->InSameTeam( pLocalTFPlayer ) || bDisguisedEnemy || pLocalTFPlayer->IsPlayerClass(TF_CLASS_SPY) )
			{
				printFormatString = "#TF_playerid_sameteam";
				bShowHealth = true;
			}
			else if ( pLocalTFPlayer->m_Shared.GetState() == TF_STATE_DYING )
			{
				// We're looking at an enemy who killed us.
				printFormatString = "#TF_playerid_diffteam";
				bShowHealth = true;
			}			

			if ( bShowHealth )
			{
				C_TF_PlayerResource *tf_PR = dynamic_cast<C_TF_PlayerResource *>(g_PR);

				if ( tf_PR )
				{
					flMaxHealth = tf_PR->GetMaxHealth( m_iTargetEntIndex );
					iMaxBuffedHealth = pPlayer->m_Shared.GetMaxBuffedHealth();
					flMaxArmor = pPlayer->GetPlayerClass()->GetMaxArmor();

					if ( bDisguisedEnemy )
					{
						flHealth = (float)pPlayer->m_Shared.GetDisguiseHealth();
						flArmor = ( float )pPlayer->m_Shared.GetDisguiseArmor();
					}
					else
					{
						flHealth = (float)pPlayer->GetHealth();
						flArmor = ( float )pPlayer->ArmorValue();
					}
					
				}
				else
				{
					bShowHealth = false;
				}
			}

			if ( printFormatString )
			{
				wchar_t *pszPrepend = GetPrepend();
				if ( !pszPrepend || !pszPrepend[0] )
				{
					pszPrepend = L"";
				}
				g_pVGuiLocalize->ConstructString( sIDString, sizeof(sIDString), g_pVGuiLocalize->Find(printFormatString), 2, pszPrepend, wszPlayerName );
			}
		}
		else	
		{
			// see if it is an object
			if ( pEnt->IsBaseObject() )
			{
				C_BaseObject *pObj = assert_cast<C_BaseObject *>( pEnt );
				m_bShowArmor = false;
				pObj->GetTargetIDString( sIDString, sizeof(sIDString) );
				pObj->GetTargetIDDataString( sDataString, sizeof(sDataString) );
				if (m_pBGPanel->m_szTeamBG[pObj->GetTeamNumber()][0])
					m_pBGPanel->SetImage(m_pBGPanel->m_szTeamBG[pObj->GetTeamNumber()]);
				bShowHealth = true;
				flHealth = pObj->GetHealth();
				flMaxHealth = pObj->GetMaxHealth();
			}
		}

		// Setup health icon
		if ( !pEnt->IsAlive() )
		{
			flHealth = 0;	// fixup for health being 1 when dead

		}

		m_pTargetHealth->SetHealth( flHealth, flMaxHealth, iMaxBuffedHealth );
		m_pTargetHealth->SetVisible( bShowHealth );

		m_pTargetArmor->SetArmor( flArmor, flMaxArmor );
		m_pTargetArmor->SetVisible( bShowHealth && m_bShowArmor );

		int iNameW, iDataW, iIgnored;

		vgui::Label* pNameLabel = m_bShowArmor ? m_pTargetNameArmorLabel : m_pTargetNameLabel;
		vgui::Label* pDataLabel = m_bShowArmor ? m_pTargetDataArmorLabel : m_pTargetDataLabel;

		pNameLabel->GetContentSize( iNameW, iIgnored );
		pDataLabel->GetContentSize( iDataW, iIgnored );

		// Target name
		if ( sIDString[0] )
		{
			sIDString[ ARRAYSIZE(sIDString)-1 ] = '\0';
			pNameLabel->SetVisible(true);

			// TODO: Support	if( hud_centerid.GetInt() == 0 )
			SetDialogVariable( "targetname", sIDString );
		}
		else
		{
			pNameLabel->SetVisible(false);
			pNameLabel->SetText("");
		}

		// Extra target data
		if ( sDataString[0] )
		{
			sDataString[ ARRAYSIZE(sDataString)-1 ] = '\0';
			pDataLabel->SetVisible(true);
			SetDialogVariable( "targetdata", sDataString );
		}
		else
		{
			pDataLabel->SetVisible(false);
			pDataLabel->SetText("");
		}

		int iPostNameW, iPostDataW;
		pNameLabel->GetContentSize( iPostNameW, iIgnored );
		pDataLabel->GetContentSize( iPostDataW, iIgnored );

		if ( m_bLayoutOnUpdate || (iPostDataW != iDataW) || (iPostNameW != iNameW) )
		{
			InvalidateLayout( true );
			m_bLayoutOnUpdate = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSecondaryTargetID::CSecondaryTargetID( const char *pElementName ) : CTargetID( pElementName )
{
	m_wszPrepend[0] = '\0';

	RegisterForRenderGroup( "mid" );

	m_bWasHidingLowerElements = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CSecondaryTargetID::ShouldDraw( void )
{
	bool bDraw = BaseClass::ShouldDraw();

	if ( bDraw )
	{
		if ( !m_bWasHidingLowerElements )
		{
			HideLowerPriorityHudElementsInGroup( "mid" );
			m_bWasHidingLowerElements = true;
		}
	}
	else 
	{
		if ( m_bWasHidingLowerElements )
		{
			UnhideLowerPriorityHudElementsInGroup( "mid" );
			m_bWasHidingLowerElements = false;
		}
	}

	return bDraw;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CSecondaryTargetID::CalculateTargetIndex( C_TFPlayer *pLocalTFPlayer )
{
	// If we're a medic & we're healing someone, target him.
	CBaseEntity *pHealTarget = pLocalTFPlayer->MedicGetHealTarget();
	if ( pHealTarget )
	{
		if ( pHealTarget->entindex() != m_iTargetEntIndex )
		{
			g_pVGuiLocalize->ConstructString( m_wszPrepend, sizeof(m_wszPrepend), g_pVGuiLocalize->Find("#TF_playerid_healtarget" ), 0 );
		}
		return pHealTarget->entindex();
	}

	// If we have a healer, target him.
	C_TFPlayer *pHealer;
	float flHealerChargeLevel;
	pLocalTFPlayer->GetHealer( &pHealer, &flHealerChargeLevel );
	if ( pHealer )
	{
		if ( pHealer->entindex() != m_iTargetEntIndex )
		{
			g_pVGuiLocalize->ConstructString( m_wszPrepend, sizeof(m_wszPrepend), g_pVGuiLocalize->Find("#TF_playerid_healer" ), 0 );
		}
		return pHealer->entindex();
	}

	if ( m_iTargetEntIndex )
	{
		m_wszPrepend[0] = '\0';
	}
	return 0;
}

// Separately declared versions of the hud element for alive and dead so they
// can have different positions

bool CMainTargetID::ShouldDraw( void )
{
	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalTFPlayer )
		return false;

	if ( pLocalTFPlayer->GetObserverMode() > OBS_MODE_NONE )
		return false;

	return BaseClass::ShouldDraw();
}

bool CSpectatorTargetID::ShouldDraw( void )
{
	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	if ( !pLocalTFPlayer )
		return false;

	if ( pLocalTFPlayer->GetObserverMode() <= OBS_MODE_NONE ||
		 pLocalTFPlayer->GetObserverMode() == OBS_MODE_FREEZECAM )
		return false;

	return BaseClass::ShouldDraw();
}

int	CSpectatorTargetID::CalculateTargetIndex( C_TFPlayer *pLocalTFPlayer ) 
{ 
	int iIndex = BaseClass::CalculateTargetIndex( pLocalTFPlayer );

	if ( pLocalTFPlayer->GetObserverMode() == OBS_MODE_IN_EYE && pLocalTFPlayer->GetObserverTarget() )
	{
		iIndex = pLocalTFPlayer->GetObserverTarget()->entindex();
	}

	return iIndex;
}