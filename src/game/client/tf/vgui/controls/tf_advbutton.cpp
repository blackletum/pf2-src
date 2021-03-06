#include "cbase.h"
#include "tf_advbutton.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "tf_controls.h"
#include <filesystem.h>
#include <vgui_controls/AnimationController.h>
#include "basemodelpanel.h"
#include "panels/tf_dialogpanelbase.h"
#include "inputsystem/iinputsystem.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CTFButton, CTFButton );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFButton::CTFButton( Panel *parent, const char *panelName, const char *text ) : CTFButtonBase( parent, panelName, text )
{
	m_iXShift = 0;
	m_iYShift = 0;

	m_bGlowing = false;
	m_bAnimationIn = false;
	m_flActionThink = -1.0f;
	m_flAnimationThink = -1.0f;

	// button movement
	m_iOrigX = 0;
	m_iOrigY = 0;
	m_iMoveX = 0;
	m_iMoveY = 0;
	m_flMoveDuration = 0.0f;
	m_flResetDelay = 0.0f;
	m_bMoved = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// Set default colors.
	SetDefaultColor( pScheme->GetColor( ADVBUTTON_DEFAULT_COLOR, COLOR_WHITE ), Color( 0, 0, 0, 0 ) );
	SetArmedColor( pScheme->GetColor( ADVBUTTON_ARMED_COLOR, COLOR_WHITE ), Color( 0, 0, 0, 0 ) );
	SetDepressedColor( pScheme->GetColor( ADVBUTTON_DEPRESSED_COLOR, COLOR_WHITE ), Color( 0, 0, 0, 0 ) );
	SetSelectedColor( pScheme->GetColor( ADVBUTTON_DEPRESSED_COLOR, COLOR_WHITE ), Color( 0, 0, 0, 0 ) );

	m_pButtonImage->SetDrawColor( m_colorImageDefault );

	// Bah, let's not bother with proper sound overriding for now.
	SetArmedSound( "ui/buttonrollover.wav" );
	SetDepressedSound( "ui/buttonclick.wav" );
	SetReleasedSound( "ui/buttonclickrelease.wav" );

	// Save the original position for animations.
	GetTextInset(&m_iTextOrigX, &m_iTextOrigY);
	GetPos(m_iOrigX, m_iOrigY);

	// Add keyboard shortcut if it's contained in the string... We should really come up with a better way.
	if ( GetCommand() )
	{
		const char *pszCommand = GetCommand()->GetString( "command" );

		CTFDialogPanelBase *pParent = dynamic_cast<CTFDialogPanelBase *>( GetParent() );

		if ( pParent )
		{
			char sText[64];
			GetText( sText, sizeof( sText ) );
			if ( Q_strcmp( sText, "" ) )
			{
				char * pch;
				pch = strchr( sText, '&' );
				if ( pch != NULL )
				{
					int id = pch - sText + 1;
					//pch = strchr(pch + 1, '&');
					char* cTest = &sText[id];
					cTest[1] = '\0';
					pParent->AddShortcut( cTest, pszCommand );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	for (KeyValues* pData = inResourceData->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey())
	{
		if (!Q_stricmp(pData->GetName(), "animation"))
		{
			m_flMoveDuration = pData->GetFloat("move_duration", 0.0f);
			m_iMoveX = pData->GetInt("move_x", 0);
			m_iMoveY = pData->GetInt("move_y", 0);
			m_flResetDelay = pData->GetFloat("move_reset_delay", m_flMoveDuration);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::PerformLayout()
{
	BaseClass::PerformLayout();
	
	if ( m_pButtonImage )
	{
		// Set image color based on our state.
		if ( IsDepressed() )
		{
			m_pButtonImage->SetDrawColor( m_colorImageDepressed );
		}
		else if ( IsArmed() || IsSelected() )
		{
			m_pButtonImage->SetDrawColor( m_colorImageArmed );
		}
		else
		{
			m_pButtonImage->SetDrawColor( m_colorImageDefault );
		}
	}
}

void CTFButton::RunAnimation(void)
{
	if (m_iMoveX != 0 || m_iMoveY != 0)
	{
		// Move button
		AnimationController::PublicValue_t p_AnimHover(m_iOrigX + XRES(m_iMoveX), m_iOrigY + YRES(m_iMoveY));
		vgui::GetAnimationController()->RunAnimationCommand(this, "Position", p_AnimHover, 0.0f, m_flMoveDuration, vgui::AnimationController::INTERPOLATOR_LINEAR, NULL);
		// Set reset time
		flResetTime = gpGlobals->curtime + m_flResetDelay;

		m_bMoved = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::OnThink()
{
	BaseClass::OnThink();

	if ( m_bGlowing && m_flAnimationThink < gpGlobals->curtime )
	{
		float m_fAlpha = ( m_bAnimationIn ? 50.0f : 255.0f );
		float m_fDelay = ( m_bAnimationIn ? 0.75f : 0.0f );
		float m_fDuration = ( m_bAnimationIn ? 0.15f : 0.25f );
		vgui::GetAnimationController()->RunAnimationCommand( this, "Alpha", m_fAlpha, m_fDelay, m_fDuration, vgui::AnimationController::INTERPOLATOR_LINEAR );
		m_bAnimationIn = !m_bAnimationIn;
		m_flAnimationThink = gpGlobals->curtime + 1.0f;
	}
	if (m_bMoved)
	{
		if (flResetTime < gpGlobals->curtime)
		{
			SetPos(m_iOrigX, m_iOrigY);
			m_bMoved = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::SetArmed( bool bState )
{

	BaseClass::SetArmed( bState );
	// Do animation if applicable.
	if (m_iXShift != 0)
	{
		vgui::GetAnimationController()->RunAnimationCommand(this, "textinsetx", bState ? m_iOrigX : m_iOrigX + m_iXShift, 0.0f, 0.1f, AnimationController::INTERPOLATOR_LINEAR);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::DoClick( void )
{
	BaseClass::DoClick();

	// Send message to the tabs manager.
	KeyValues *msg = new KeyValues( "ButtonPressed" );
	PostActionSignal( msg );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::SetGlowing( bool Glowing )
{
	m_bGlowing = Glowing;

	if ( !m_bGlowing )
	{
		float m_fAlpha = 255.0f;
		float m_fDelay = 0.0f;
		float m_fDuration = 0.0f;
		vgui::GetAnimationController()->RunAnimationCommand( this, "Alpha", m_fAlpha, m_fDelay, m_fDuration, AnimationController::INTERPOLATOR_LINEAR );
	}
}
