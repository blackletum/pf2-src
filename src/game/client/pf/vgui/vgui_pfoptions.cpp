//========= Copyright LOLOL, All rights reserved. ============//
// Code is from primarily from https://github.com/Biohazard90/g-string_2013
// Purpose:	Seperate menu purely for PF2 specifc settings
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tier1/KeyValues.h"
#include "vgui/vgui_pfoptions.h"
#include "ienginevgui.h"
#include "vgui_controls/Panel.h"

#include "vgui_controls/CheckButton.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Slider.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/PropertyPage.h"
#include "pf_cvars.h"

#define RESOURCE_FILE "resource/pf_options.res"
#define RESOURCE_FILE_GAME "resource/pf_options_game.res"
#define RESOURCE_FILE_GRAPHICS "resource/pf_options_performance.res"

extern ConVar pf_picmip;

// Singleton
static CVGUIPreFortressOptions *g_pAdvOptions = NULL;

vgui::Panel *GetSDKRootPanel();

using namespace vgui;

CVGUIPreFortressOptions *GetAdvOptions()
{
	Assert( g_pAdvOptions );
	return g_pAdvOptions;
}

static void AdvOptions_f()
{
	CVGUIPreFortressOptions *options = GetAdvOptions();
	if (options)
	{
		options->ReadValues(false);
		if (!options->IsVisible())
			options->SetVisible( true );
		options->MoveToFront();
	}
}
void PF2Options_Create( vgui::VPANEL parent )
{
	Assert( !g_pAdvOptions );

	new CVGUIPreFortressOptions( parent, "PF2Options" );
}

void PF2Options_Destroy()
{
	if (g_pAdvOptions)
	{
		g_pAdvOptions->MarkForDeletion();
		g_pAdvOptions = NULL;
	}
}

static ConCommand vgui_showBetaOptions( "vgui_showBetaOptions", AdvOptions_f, "Show/hide advanced options UI." );


CVGUIPreFortressOptions::CVGUIPreFortressOptions( VPANEL parent, const char *pName ) : BaseClass( NULL, pName )
{
	g_pAdvOptions = this;

	SetParent( parent );

	Activate();

	SetTitle( "#title", true );

	m_pPropertySheet = new PropertySheet( this, "property_sheet" );

	pPageGameplay = new PropertyPage( m_pPropertySheet, "" );
	pPagePerformance = new PropertyPage( m_pPropertySheet, "" );

	LoadControlSettings( RESOURCE_FILE );

	m_pPropertySheet->AddPage( pPageGameplay, "Game" );
	m_pPropertySheet->AddPage( pPagePerformance, "Graphics" );

#define CREATE_VGUI_SLIDER( var, name, page, minRange, maxRange, ticks ) var = new Slider( page, name ); \
	var->SetRange( minRange, maxRange ); \
	var->SetNumTicks( ticks ); \
	var->AddActionSignalTarget( this )

#define CREATE_VGUI_CHECKBOX( var, name, page ) var = new CheckButton( page, name, "" ); \
	var->AddActionSignalTarget( this )

#define CREATE_VGUI_TEXTENTRY( var, name, page) var = new TextEntry( page, name ); \
	var->SetAllowNumericInputOnly(true); \
	var->AddActionSignalTarget( this )

	// GRAPHICS SETTINGS
	
	CREATE_VGUI_CHECKBOX( m_pCheck_MuzzleLight, "check_muzzlelight", pPagePerformance );
	CREATE_VGUI_CHECKBOX( m_pCheck_MuzzleLightProps, "check_muzzlelightprops", pPagePerformance );
	CREATE_VGUI_CHECKBOX( m_pCheck_CheapSplashes, "check_cheapsplashes", pPagePerformance );
	CREATE_VGUI_CHECKBOX( m_pCheck_BurningLight, "check_burninglight", pPagePerformance );
	CREATE_VGUI_CHECKBOX( m_pCheck_ProjectileLight, "check_projectilelight", pPagePerformance );
	CREATE_VGUI_SLIDER( m_pSlider_PicmipLevel, "slider_picmip", pPagePerformance, -10, 10, 20 );

	CREATE_VGUI_CHECKBOX( m_pCheck_Violence, "check_violence", pPagePerformance );
	CREATE_VGUI_SLIDER( m_pSlider_BloodSpray, "slider_bloodspray", pPagePerformance, 0, 4, 4 );
	CREATE_VGUI_SLIDER( m_pSlider_BloodPuff, "slider_bloodsmoke", pPagePerformance, 0, 4, 4 );
	CREATE_VGUI_SLIDER( m_pSlider_BloodDrops, "slider_blooddrops", pPagePerformance, 0, 32, 8 );

	pPagePerformance->LoadControlSettings( RESOURCE_FILE_GRAPHICS );

	// GAME SETTINGS

	CREATE_VGUI_CHECKBOX( m_pCheck_MuzzleFlash, "check_muzzleflash", pPageGameplay );
	CREATE_VGUI_CHECKBOX( m_pCheck_ColoredCloak, "check_teamcoloredcloak", pPageGameplay );
	CREATE_VGUI_CHECKBOX( m_pCheck_RPCEnabled, "check_rpc_enabled", pPageGameplay );
	CREATE_VGUI_CHECKBOX( m_pCheck_RPCClassIcons, "check_rpc_classicons", pPageGameplay );

	pPageGameplay->LoadControlSettings( RESOURCE_FILE_GAME );

#ifndef WIN32
	m_pCheck_RPCEnabled->SetEnabled( false );
	m_pCheck_RPCClassIcons->SetEnabled( false );
#endif

	SetVisible( false );
	SetSizeable( false );
	SetMoveable( true );
}

CVGUIPreFortressOptions::~CVGUIPreFortressOptions()
{
}

void CVGUIPreFortressOptions::OnKeyCodeTyped( KeyCode code )
{
	// force ourselves to be closed if the escape key it pressed
	if ( code == KEY_ESCAPE )
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped( code );
	}
}

void CVGUIPreFortressOptions::OnCommand( const char *cmd )
{
	if ( !Q_stricmp( cmd, "save" ) )
	{
		ReadValues( true );
		engine->ClientCmd( "host_writeconfig" );;
		Close();
	}
	else
	{
		BaseClass::OnCommand( cmd );
	}
}

void CVGUIPreFortressOptions::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	ReadValues(false); 
}

void CVGUIPreFortressOptions::OnCheckButtonChecked( Panel *panel )
{
	if (panel == m_pCheck_Violence)
	{
		m_pSlider_BloodSpray->SetEnabled( !m_pCheck_Violence->IsSelected() );
		m_pSlider_BloodPuff->SetEnabled( !m_pCheck_Violence->IsSelected() );
		m_pSlider_BloodDrops->SetEnabled( !m_pCheck_Violence->IsSelected() );
	}
}

void CVGUIPreFortressOptions::ReadValues(bool write)
{
#define CVAR_CHECK_SELECTED( x, y ) (write ? x.SetValue(y->IsSelected() ? 1 : 0) : y->SetSelected( x.GetBool() ) )
#define CVAR_SLIDER_INTEGER( x, y ) (write ? x.SetValue(y->GetValue()) : y->SetValue( x.GetInt(), false ) )
//#define CVAR_TEXTENTRY_INTEGER( x, y ) ( y->SetText(m_pVarTexts[i]->GetString()))

	CVAR_CHECK_SELECTED( pf_muzzlelight, m_pCheck_MuzzleLight );
	CVAR_CHECK_SELECTED( pf_muzzleflash, m_pCheck_MuzzleFlash );
	CVAR_CHECK_SELECTED( pf_team_colored_spy_cloak, m_pCheck_ColoredCloak );
	CVAR_CHECK_SELECTED( pf_expensivebulletsplash, m_pCheck_CheapSplashes );
	CVAR_CHECK_SELECTED( pf_blood_impact_disable, m_pCheck_Violence );
	CVAR_CHECK_SELECTED( pf_muzzlelightsprops, m_pCheck_MuzzleLightProps );
	CVAR_CHECK_SELECTED( pf_burningplayerlight, m_pCheck_BurningLight );
	CVAR_CHECK_SELECTED( pf_projectilelight, m_pCheck_ProjectileLight );
#ifdef WIN32
	CVAR_CHECK_SELECTED( pf_discord_rpc, m_pCheck_RPCEnabled );
	CVAR_CHECK_SELECTED( pf_discord_class, m_pCheck_RPCClassIcons );
#endif

	CVAR_SLIDER_INTEGER( pf_picmip, m_pSlider_PicmipLevel );
	CVAR_SLIDER_INTEGER( pf_blood_spray, m_pSlider_BloodSpray );
	CVAR_SLIDER_INTEGER( pf_blood_puff, m_pSlider_BloodPuff );
	CVAR_SLIDER_INTEGER( pf_blood_droplets, m_pSlider_BloodDrops );

	//LoadControlSettings( RESOURCE_FILE );
	//pPagePerformance->LoadControlSettings( RESOURCE_FILE_GRAPHICS );
	//pPageGameplay->LoadControlSettings( RESOURCE_FILE_GAME );
}

void CVGUIPreFortressOptions::PerformLayout()
{
	BaseClass::PerformLayout();

	MoveToCenterOfScreen();
}