//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <time.h>

#include "pf_creditspanel.h"
#include "tf_mainmenu.h"
#include "tf_menupanelbase.h"
#include "controls/tf_cvarslider.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_advpanellistpanel.h"
#include "controls/tf_cvartogglecheckbutton.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_scriptobject.h"
#include "filesystem.h"

#include <vgui/ILocalize.h>
#include <vgui_controls/ListPanel.h>
#include <KeyValues.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>

#include <tier0/vcrmode.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define CREDITS_DIR "scripts/pf2beta_credits.txt"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFCreditsPanel::CTFCreditsPanel( vgui::Panel* parent, const char* panelName ) : CTFDialogPanelBase( parent, panelName )
{
	Init();
	m_pCreditsKV = nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFCreditsPanel::~CTFCreditsPanel()
{
}


bool CTFCreditsPanel::Init()
{
	BaseClass::Init();

	m_pListPanel = new CPanelListPanel( this, "CreditsListPanel" );

	return true;
}

void CTFCreditsPanel::ApplySchemeSettings( vgui::IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/UI/main_menu/CreditsPanel.res" );
}

void CTFCreditsPanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	// Check for credits file
	KeyValues *kv = new KeyValues( "Credits" );
	if (kv->LoadFromFile( filesystem, "scripts/credits.txt", "MOD" ))
		m_pCreditsKV = kv;
}



void CTFCreditsPanel::OnCommand( const char* command )
{
	BaseClass::OnCommand( command );
}

void CTFCreditsPanel::OnKeyCodeTyped( KeyCode code )
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Hide();
	}
	else
	{
		BaseClass::OnKeyCodeTyped( code );
	}
}

void CTFCreditsPanel::Show()
{
	BaseClass::Show();
	OnCreateControls();
}


// excludes "other" as we handle that differently
static const char *s_CreditKeyStrings[][2] =
{
	{ "lead", "#PF_Credit_Lead" },
	{ "code", "#PF_Credit_Code" },
	{ "models", "#PF_Credit_Models" },
	{ "textures", "#PF_Credit_Textures" },
	{ "animation", "#PF_Credit_Animation" },
	{ "particles", "#PF_Credit_Particles" },
	{ "mapper", "#PF_Credit_Mapper" },
	{ "tester", "#PF_Credit_Tester" },
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCreditsPanel::CreateControls()
{
	BaseClass::CreateControls();

	// Create credit entries
	if (m_pCreditsKV)
	{
		Label *pCreditEntry = nullptr;
		Label *pTitle = nullptr;
		for (KeyValues *kv = m_pCreditsKV->GetFirstSubKey(); kv; kv = kv->GetNextKey())
		{
			char szTitleName[ 32 ];
			if (!Q_stricmp( kv->GetName(), "developers" ))
				Q_snprintf( szTitleName, sizeof( szTitleName ), "#PF_Credit_Title_Developers" );
			else if (!Q_stricmp( kv->GetName(), "contributors" ))
				Q_snprintf( szTitleName, sizeof( szTitleName ), "#PF_Credit_Title_Contributors" );
			else if (!Q_stricmp( kv->GetName(), "software" ))
				Q_snprintf( szTitleName, sizeof( szTitleName ), "#PF_Credit_Title_Software" );
			else
				continue; // ignore all other subkeys

			pTitle = new Label( this, "Title", "" );
			wchar_t *pText = g_pVGuiLocalize->Find( szTitleName );
			if (pText)
				wcstombs( szTitleName, pText, sizeof( szTitleName ) );
			pTitle->SetText( szTitleName );
			
			pTitle->SetBorder( GETSCHEME()->GetBorder( "AdvSettingsTitleBorder" ) );
			pTitle->SetFont( GETSCHEME()->GetFont( "HudFontMediumBold", true ) );
			pTitle->SetFgColor( GETSCHEME()->GetColor( ADVBUTTON_DEFAULT_COLOR, COLOR_WHITE ) );
			int h = m_pListPanel->GetTall() / 12.0; //(float)GetParent()->GetTall() / 15.0;
			pTitle->SetSize( pTitle->GetWide(), h );
			m_pListPanel->AddItem( pTitle );

			for (KeyValues *kvDevs = kv->GetFirstSubKey(); kvDevs; kvDevs = kvDevs->GetNextKey())
			{
				const char *name = kvDevs->GetName();
				if (name)
				{
					char roles[512] = "";
					bool bHasRole = false;
					for (int i = 0; i < ARRAYSIZE( s_CreditKeyStrings ); ++i)
					{
						if (kvDevs->GetString( s_CreditKeyStrings[ i ][ 0 ], nullptr ))
						{
							char szStringToLocalize[ 64 ];
							Q_snprintf( szStringToLocalize, sizeof( szStringToLocalize ), s_CreditKeyStrings[ i ][ 1 ] );

							char szLocalized[ 512 ];
							g_pVGuiLocalize->ConvertUnicodeToANSI( g_pVGuiLocalize->Find( szStringToLocalize ), szLocalized, sizeof( szLocalized ) );
							Q_strcat( roles, bHasRole ? VarArgs( ", %s", szLocalized ) : szLocalized, sizeof( roles ) );
							bHasRole = true;
						}
					}
					// Add specific credits if we need it
					if (kvDevs->GetBool( "other", false ))
					{
						const char *otherText = kvDevs->GetString( "other_text", nullptr );
						Q_strcat( roles, bHasRole ? VarArgs( ", %s", otherText ) : VarArgs( "%s", otherText ), sizeof( roles ) );
						bHasRole = true;
					}

					pCreditEntry = new Label( this, "CreditName", bHasRole ? VarArgs( "%s - %s", name, roles ) : name );
					pCreditEntry->SetFont( GETSCHEME()->GetFont( "HudFontSmall", true ) );
					pCreditEntry->SetFgColor( GETSCHEME()->GetColor( ADVBUTTON_DEFAULT_COLOR, COLOR_WHITE ) );
					int h = m_pListPanel->GetTall() / 16.0; //(float)GetParent()->GetTall() / 15.0;
					pCreditEntry->SetSize( pCreditEntry->GetWide(), h );
					m_pListPanel->AddItem( pCreditEntry );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFCreditsPanel::DestroyControls()
{
	BaseClass::DestroyControls();
}

void CTFCreditsPanel::OnApplyChanges()
{
	BaseClass::OnApplyChanges();
}