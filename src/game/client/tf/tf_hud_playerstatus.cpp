//========= Copyright � 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IImage.h>
#include <vgui_controls/Label.h>

#include "hud_numericdisplay.h"
#include "c_team.h"
#include "c_tf_player.h"
#include "tf_shareddefs.h"
#include "tf_hud_playerstatus.h"
#include "pf_cvars.h"
#include "tf_gamerules.h"

using namespace vgui;


extern ConVar tf_max_health_boost;


static char *g_szBlueClassImages[] = 
{ 
	"",
	"../hud/class_scoutblue", 
	"../hud/class_sniperblue",
	"../hud/class_soldierblue",
	"../hud/class_demoblue",
	"../hud/class_medicblue",
	"../hud/class_heavyblue",
	"../hud/class_pyroblue",
	"../hud/class_spyblue",
	"../hud/class_engiblue",
	"",
};

static char *g_szRedClassImages[] = 
{ 
	"",
	"../hud/class_scoutred", 
	"../hud/class_sniperred",
	"../hud/class_soldierred",
	"../hud/class_demored",
	"../hud/class_medicred",
	"../hud/class_heavyred",
	"../hud/class_pyrored",
	"../hud/class_spyred",
	"../hud/class_engired",
	"",
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHudPlayerClass::CTFHudPlayerClass( Panel *parent, const char *name ) : EditablePanel( parent, name )
{
	m_pClassImage = new CTFClassImage( this, "PlayerStatusClassImage" );
	m_pSpyImage = new CTFImagePanel( this, "PlayerStatusSpyImage" );
	m_pSpyOutlineImage = new CTFImagePanel( this, "PlayerStatusSpyOutlineImage" );

	m_nTeam = TEAM_UNASSIGNED;
	m_nClass = TF_CLASS_UNDEFINED;
	m_nDisguiseTeam = TEAM_UNASSIGNED;
	m_nDisguiseClass = TF_CLASS_UNDEFINED;
	m_flNextThink = 0.0f;

	ListenForGameEvent( "localplayer_changedisguise" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerClass::Reset()
{
	m_flNextThink = gpGlobals->curtime + 0.05f;

	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HudSpyDisguiseHide" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerClass::ApplySchemeSettings( IScheme *pScheme )
{
	// load control settings...
	LoadControlSettings( "resource/UI/HudPlayerClass.res" );

	m_nTeam = TEAM_UNASSIGNED;
	m_nClass = TF_CLASS_UNDEFINED;
	m_nDisguiseTeam = TEAM_UNASSIGNED;
	m_nDisguiseClass = TF_CLASS_UNDEFINED;
	m_flNextThink = 0.0f;
	m_nCloakLevel = 0;

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerClass::OnThink()
{
	if ( m_flNextThink < gpGlobals->curtime )
	{
		C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
		bool bTeamChange = false;

		if ( pPlayer )
		{
			// set our background colors
			if ( m_nTeam != pPlayer->GetTeamNumber() )
			{
				bTeamChange = true;
				m_nTeam = pPlayer->GetTeamNumber();
			}

			int nCloakLevel = 0;
			bool bCloakChange = false;
			float flInvis = pPlayer->GetPercentInvisible();

			if ( flInvis > 0.9 )
			{
				nCloakLevel = 2;
			}
			else if ( flInvis > 0.1 )
			{
				nCloakLevel = 1;
			}

			if ( nCloakLevel != m_nCloakLevel )
			{
				m_nCloakLevel = nCloakLevel;
				bCloakChange = true;
			}

			// set our class image
			if ( m_nClass != pPlayer->GetPlayerClass()->GetClassIndex() || bTeamChange || bCloakChange ||
				( m_nClass == TF_CLASS_SPY && m_nDisguiseClass != pPlayer->m_Shared.GetDisguiseClass() ) ||
				( m_nClass == TF_CLASS_SPY && m_nDisguiseTeam != pPlayer->m_Shared.GetDisguiseTeam() ) )
			{
				m_nClass = pPlayer->GetPlayerClass()->GetClassIndex();

				if ( m_nClass == TF_CLASS_SPY && pPlayer->m_Shared.InCond( TF_COND_DISGUISED ) )
				{
					if ( !pPlayer->m_Shared.InCond( TF_COND_DISGUISING ) )
					{
						m_nDisguiseTeam = pPlayer->m_Shared.GetDisguiseTeam();
						m_nDisguiseClass = pPlayer->m_Shared.GetDisguiseClass();
					}
				}
				else
				{
					m_nDisguiseTeam = TEAM_UNASSIGNED;
					m_nDisguiseClass = TF_CLASS_UNDEFINED;
				}

				if ( m_pClassImage && m_pSpyImage )
				{
					int iCloakState = 0;
					if ( pPlayer->IsPlayerClass( TF_CLASS_SPY ) )
					{
						iCloakState = m_nCloakLevel;
					}

					if ( m_nDisguiseTeam != TEAM_UNASSIGNED || m_nDisguiseClass != TF_CLASS_UNDEFINED )
					{
						m_pSpyImage->SetVisible( true );
						m_pClassImage->SetClass( m_nDisguiseTeam, m_nDisguiseClass, iCloakState );
					}
					else
					{
						m_pSpyImage->SetVisible( false );
						m_pClassImage->SetClass( m_nTeam, m_nClass, iCloakState );
					}
				}
			}
		}

		m_flNextThink = gpGlobals->curtime + 0.05f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerClass::FireGameEvent( IGameEvent * event )
{
	if ( FStrEq( "localplayer_changedisguise", event->GetName() ) )
	{
		if ( m_pSpyImage && m_pSpyOutlineImage )
		{
			bool bFadeIn = event->GetBool( "disguised", false );

			if ( bFadeIn )
			{
				m_pSpyImage->SetAlpha( 0 );
			}
			else
			{
				m_pSpyImage->SetAlpha( 255 );
			}

			m_pSpyOutlineImage->SetAlpha( 0 );
			
			m_pSpyImage->SetVisible( true );
			m_pSpyOutlineImage->SetVisible( true );

			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( bFadeIn ? "HudSpyDisguiseFadeIn" : "HudSpyDisguiseFadeOut" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHealthPanel::CTFHealthPanel( Panel *parent, const char *name ) : CTFImagePanel( parent, name )
{
	m_flPercent = 1.0f;

	m_iMaterialIndex = surface()->DrawGetTextureId(GetHealthMaterialName());
	if ( m_iMaterialIndex == -1 ) // we didn't find it, so create a new one
	{
		m_iMaterialIndex = surface()->CreateNewTextureID();	
	}

	surface()->DrawSetTextureFile( m_iMaterialIndex, GetHealthMaterialName(), true, false );

	m_iDeadMaterialIndex = surface()->DrawGetTextureId( "hud/health_dead" );
	if ( m_iDeadMaterialIndex == -1 ) // we didn't find it, so create a new one
	{
		m_iDeadMaterialIndex = surface()->CreateNewTextureID();	
	}
	surface()->DrawSetTextureFile( m_iDeadMaterialIndex, "hud/health_dead", true, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHealthPanel::Paint()
{
	BaseClass::Paint();

	int x, y, w, h;
	GetBounds( x, y, w, h );

	Vertex_t vert[4];	
	float uv1 = 0.0f;
	float uv2 = 1.0f;
	int xpos = 0, ypos = 0;

	if ( m_flPercent <= 0 )
	{
		// Draw the dead material
		surface()->DrawSetTexture( m_iDeadMaterialIndex );
		
		vert[0].Init( Vector2D( xpos, ypos ), Vector2D( uv1, uv1 ) );
		vert[1].Init( Vector2D( xpos + w, ypos ), Vector2D( uv2, uv1 ) );
		vert[2].Init( Vector2D( xpos + w, ypos + h ), Vector2D( uv2, uv2 ) );				
		vert[3].Init( Vector2D( xpos, ypos + h ), Vector2D( uv1, uv2 ) );

		surface()->DrawSetColor( Color(255,255,255,255) );
	}
	else
	{
		float flDamageY = h * ( 1.0f - m_flPercent);

		// blend in the red "damage" part
		surface()->DrawSetTexture( m_iMaterialIndex );

		Vector2D uv11( uv1, uv2 - m_flPercent);
		Vector2D uv21( uv2, uv2 - m_flPercent);
		Vector2D uv22( uv2, uv2 );
		Vector2D uv12( uv1, uv2 );

		vert[0].Init( Vector2D( xpos, flDamageY ), uv11 );
		vert[1].Init( Vector2D( xpos + w, flDamageY ), uv21 );
		vert[2].Init( Vector2D( xpos + w, ypos + h ), uv22 );				
		vert[3].Init( Vector2D( xpos, ypos + h ), uv12 );

		surface()->DrawSetColor( GetFgColor() );
	}

	surface()->DrawTexturedPolygon( 4, vert );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFArmorPanel::CTFArmorPanel(Panel* parent, const char* name) : CTFImagePanel(parent, name)
{
	m_flPercent = 1.0f;

	m_iMaterialIndex = surface()->DrawGetTextureId( "hud/armor_light_blue" );
	if (m_iMaterialIndex == -1) // we didn't find it, so create a new one
	{
		m_iMaterialIndex = surface()->CreateNewTextureID();
	}

	surface()->DrawSetTextureFile( m_iMaterialIndex, "hud/armor_light_blue", true, false );
}

void CTFArmorPanel::SetArmorMaterial( int iTeam, int iType )
{
	const char* pArmorTeam = g_aTeamNamesLower[ iTeam ];
	const char* pArmorType = g_aArmorTypesLower[ iType ];

	const char* pArmorMaterial = VarArgs( "hud/armor_%s_%s", pArmorType, pArmorTeam );
	m_iMaterialIndex = surface()->DrawGetTextureId( pArmorMaterial );
	if (m_iMaterialIndex == -1) // we didn't find it, so create a new one
	{
		m_iMaterialIndex = surface()->CreateNewTextureID();
	}

	surface()->DrawSetTextureFile( m_iMaterialIndex, pArmorMaterial, true, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFArmorPanel::Paint()
{
	BaseClass::Paint();

	int x, y, w, h;
	GetBounds(x, y, w, h);

	Vertex_t vert[4];
	float uv1 = 0.0f;
	float uv2 = 1.0f;
	int xpos = 0, ypos = 0;

	float flDamageY = h * (1.0f - m_flPercent);

	// blend in the red "damage" part
	surface()->DrawSetTexture(m_iMaterialIndex);

	Vector2D uv11(uv1, uv2 - m_flPercent);
	Vector2D uv21(uv2, uv2 - m_flPercent);
	Vector2D uv22(uv2, uv2);
	Vector2D uv12(uv1, uv2);

	vert[0].Init(Vector2D(xpos, flDamageY), uv11);
	vert[1].Init(Vector2D(xpos + w, flDamageY), uv21);
	vert[2].Init(Vector2D(xpos + w, ypos + h), uv22);
	vert[3].Init(Vector2D(xpos, ypos + h), uv12);

	surface()->DrawSetColor(GetFgColor());

	surface()->DrawTexturedPolygon(4, vert);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHudPlayerHealth::CTFHudPlayerHealth( Panel *parent, const char *name ) : EditablePanel( parent, name )
{
	m_pHealthImage = new CTFHealthPanel( this, "PlayerStatusHealthImage" );
	m_pHealthImageBG = new ImagePanel( this, "PlayerStatusHealthImageBG" );
	m_pHealthBonusImage = new CTFImagePanel( this, "PlayerStatusHealthBonusImage" );

	m_flNextThink = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::Reset()
{
	m_flNextThink = gpGlobals->curtime + 0.05f;
	m_nHealth = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::ApplySchemeSettings( IScheme *pScheme )
{
	// load control settings...
	LoadControlSettings( GetResFilename() );

	if ( m_pHealthBonusImage )
	{
		m_pHealthBonusImage->GetBounds( m_nBonusHealthOrigX, m_nBonusHealthOrigY, m_nBonusHealthOrigW, m_nBonusHealthOrigH );
	}

	m_flNextThink = 0.0f;

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::SetHealth( int iNewHealth, int iMaxHealth, int	iMaxBuffedHealth )
{
	int nPrevHealth = m_nHealth;

	// set our health
	m_nHealth = iNewHealth;
	m_nMaxHealth = iMaxHealth;
	m_pHealthImage->SetPercentage( (float)(m_nHealth) / (float)(m_nMaxHealth) );

	if ( m_pHealthImage )
	{
		m_pHealthImage->SetFgColor( Color( 255, 255, 255, 255 ) );
	}

	if ( m_nHealth <= 0 )
	{
		if ( m_pHealthImageBG->IsVisible() )
		{
			m_pHealthImageBG->SetVisible( false );
		}
		HideHealthBonusImage();
	}
	else
	{
		if ( !m_pHealthImageBG->IsVisible() )
		{
			m_pHealthImageBG->SetVisible( true );
		}

		// are we getting a health bonus?
		if ( m_nHealth > m_nMaxHealth )
		{
			if ( m_pHealthBonusImage )
			{
				if ( !m_pHealthBonusImage->IsVisible() )
				{
					m_pHealthBonusImage->SetVisible( true );
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( this, "HudHealthBonusPulse" );
				}

				m_pHealthBonusImage->SetFgColor( Color( 255, 255, 255, 255 ) );

				// scale the flashing image based on how much health bonus we currently have
				float flBoostMaxAmount = ( iMaxBuffedHealth ) - m_nMaxHealth;
				//Never display the bonus image at larger than 150% its size
				float flPercent = MIN(( m_nHealth - m_nMaxHealth ) / flBoostMaxAmount, 1.5f);

				int nPosAdj = RoundFloatToInt( flPercent * m_nHealthBonusPosAdj );
				int nSizeAdj = 2 * nPosAdj;

				m_pHealthBonusImage->SetBounds( m_nBonusHealthOrigX - nPosAdj, 
					m_nBonusHealthOrigY - nPosAdj, 
					m_nBonusHealthOrigW + nSizeAdj,
					m_nBonusHealthOrigH + nSizeAdj );
			}
		}
		// are we close to dying?
		else if ( m_nHealth < m_nMaxHealth * m_flHealthDeathWarning )
		{
			if ( m_pHealthBonusImage )
			{
				if ( !m_pHealthBonusImage->IsVisible() )
				{
					m_pHealthBonusImage->SetVisible( true );
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( this, "HudHealthDyingPulse" );
				}

				m_pHealthBonusImage->SetFgColor( m_clrHealthDeathWarningColor );

				// scale the flashing image based on how much health bonus we currently have
				float flBoostMaxAmount = m_nMaxHealth * m_flHealthDeathWarning;
				float flPercent = ( flBoostMaxAmount - m_nHealth ) / flBoostMaxAmount;

				int nPosAdj = RoundFloatToInt( flPercent * m_nHealthBonusPosAdj );
				int nSizeAdj = 2 * nPosAdj;

				m_pHealthBonusImage->SetBounds( m_nBonusHealthOrigX - nPosAdj, 
					m_nBonusHealthOrigY - nPosAdj, 
					m_nBonusHealthOrigW + nSizeAdj,
					m_nBonusHealthOrigH + nSizeAdj );
			}

			if ( m_pHealthImage )
			{
				m_pHealthImage->SetFgColor( m_clrHealthDeathWarningColor );
			}
		}
		// turn it off
		else
		{
			HideHealthBonusImage();
		}
	}

	// set our health display value
	if ( nPrevHealth != m_nHealth )
	{
		if ( m_nHealth > 0 )
		{
			SetDialogVariable( "Health", m_nHealth );
		}
		else
		{
			SetDialogVariable( "Health", "" );
		}	
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::HideHealthBonusImage( void )
{
	if ( m_pHealthBonusImage && m_pHealthBonusImage->IsVisible() )
	{
		m_pHealthBonusImage->SetBounds( m_nBonusHealthOrigX, m_nBonusHealthOrigY, m_nBonusHealthOrigW, m_nBonusHealthOrigH );
		m_pHealthBonusImage->SetVisible( false );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( this, "HudHealthBonusPulseStop" );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( this, "HudHealthDyingPulseStop" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerHealth::OnThink()
{
	if ( m_flNextThink < gpGlobals->curtime )
	{
		C_TFPlayer *pPlayer = ToTFPlayer( C_BasePlayer::GetLocalPlayer() );

		if ( pPlayer )
		{
			SetHealth( pPlayer->GetHealth(), pPlayer->GetPlayerClass()->GetMaxHealth(), pPlayer->m_Shared.GetMaxBuffedHealth() );
		}

		m_flNextThink = gpGlobals->curtime + 0.05f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHudPlayerArmor::CTFHudPlayerArmor(Panel* parent, const char* name) : EditablePanel(parent, name)
{
	m_pArmorImage = new CTFArmorPanel(this, "PlayerStatusArmorImage");
	m_pArmorImageBG = new ImagePanel(this, "PlayerStatusArmorImageBG");

	m_nTeam = TEAM_UNASSIGNED;
	m_nType = ARMOR_NONE;
	m_flNextThink = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerArmor::Reset()
{
	m_flNextThink = gpGlobals->curtime + 0.05f;
	m_nArmor = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerArmor::ApplySchemeSettings(IScheme* pScheme)
{
	// load control settings...
	LoadControlSettings(GetResFilename());

	m_flNextThink = 0.0f;

	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerArmor::SetArmor(int iNewArmor, int iMaxArmor)
{
	int nPrevArmor = m_nArmor;

	// set our health
	m_nArmor = iNewArmor;
	m_nMaxArmor = iMaxArmor;
	m_pArmorImage->SetPercentage((float)(m_nArmor) / (float)(m_nMaxArmor));

	if (m_pArmorImage)
	{
		m_pArmorImage->SetFgColor(Color(255, 255, 255, 255));
	}

	if (!m_pArmorImageBG->IsVisible())
	{
		m_pArmorImageBG->SetVisible(true);
	}
	// are we close to losing armor?
	else if (m_nArmor < m_nMaxArmor * m_flArmorDeathWarning)
	{
		if (m_pArmorImage)
		{
			m_pArmorImage->SetFgColor(m_clrArmorDeathWarningColor);
		}
	}

	// set our health display value
	if (nPrevArmor != m_nArmor)
	{
		if (m_nArmor > 0)
		{
			SetDialogVariable("Armor", m_nArmor);
		}
		else
		{
			SetDialogVariable("Armor", "0");
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerArmor::SetArmorImageTeam( int iTeam, int iType )
{
	if ( m_pArmorImage )
	{
		m_nTeam = iTeam;
		m_nType = iType;
		m_pArmorImage->SetArmorMaterial( iTeam, iType );
		m_pArmorImageBG->SetImage( VarArgs( "../hud/armor_%s_bg", g_aArmorTypesLower[ iType ] ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerArmor::OnThink()
{	
	if (m_flNextThink < gpGlobals->curtime)
	{
		C_TFPlayer* pPlayer = ToTFPlayer(C_BasePlayer::GetLocalPlayer());

		if (pPlayer)
		{
			SetArmor(pPlayer->ArmorValue(), pPlayer->GetPlayerClass()->GetMaxArmor());
			int nType = pPlayer->GetPlayerClass()->GetArmorType();

			if (m_nTeam != pPlayer->GetTeamNumber() || m_nType != nType )
			{
				SetArmorImageTeam( pPlayer->GetTeamNumber(), nType );
			}
		}

		m_flNextThink = gpGlobals->curtime + 0.05f;
	}

	// Set armor element material
}


DECLARE_HUDELEMENT( CTFHudPlayerStatus );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFHudPlayerStatus::CTFHudPlayerStatus( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudPlayerStatus" ) 
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_pHudPlayerClass = new CTFHudPlayerClass( this, "HudPlayerClass" );
	m_pHudPlayerHealth = new CTFHudPlayerHealth( this, "HudPlayerHealth" );
	m_pHudPlayerArmor = new CTFHudPlayerArmor(this, "HudPlayerArmor");

	m_flNextThink = 0.0f;

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerStatus::OnThink()
{
	if ( m_flNextThink < gpGlobals->curtime )
	{
		if ( TFGameRules() )
		{
			if ( TFGameRules()->GetArmorMode() )
			{
				if ( !m_pHudPlayerArmor->IsVisible() )
				{
					m_pHudPlayerArmor->InvalidateLayout( false, true );
					m_pHudPlayerArmor->SetVisible( true );
				}
			}
			else
			{
				if ( m_pHudPlayerArmor->IsVisible() )
					m_pHudPlayerArmor->SetVisible( false );
			}
		}
		m_flNextThink = gpGlobals->curtime + 0.25f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerStatus::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// HACK: Work around the scheme application order failing
	// to reload the player class hud element's scheme in minmode.
	ConVarRef cl_hud_minmode( "cl_hud_minmode", true );
	if ( cl_hud_minmode.IsValid() && cl_hud_minmode.GetBool() )
	{
		m_pHudPlayerClass->InvalidateLayout( false, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFHudPlayerStatus::Reset()
{
	if ( m_pHudPlayerClass )
	{
		m_pHudPlayerClass->Reset();
	}

	if ( m_pHudPlayerHealth )
	{
		m_pHudPlayerHealth->Reset();
	}

	if (m_pHudPlayerArmor)
	{
		m_pHudPlayerArmor->Reset();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFClassImage::SetClass( int iTeam, int iClass, int iCloakstate )
{
	char szImage[128];
	szImage[0] = '\0';

	if ( iTeam == TF_TEAM_BLUE )
	{
		Q_strncpy( szImage, g_szBlueClassImages[ iClass ], sizeof(szImage) );
	}
	else
	{
		Q_strncpy( szImage, g_szRedClassImages[ iClass ], sizeof(szImage) );
	}

	switch( iCloakstate )
	{
	case 2:
		Q_strncat( szImage, "_cloak", sizeof(szImage), COPY_ALL_CHARACTERS );
		break;
	case 1:
		Q_strncat( szImage, "_halfcloak", sizeof(szImage), COPY_ALL_CHARACTERS );
		break;
	default:
		break;
	}

	if ( Q_strlen( szImage ) > 0 )
	{
		SetImage( szImage );
	}
}