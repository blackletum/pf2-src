#include "cbase.h"
#include "tf_dialogpanelbase.h"
#include "tf_mainmenu.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_advpanellistpanel.h"
#include "controls/tf_scriptobject.h"
#include "controls/tf_cvartogglecheckbutton.h"
#include "controls/tf_cvarslider.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Tooltip.h"
#include "inputsystem/iinputsystem.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include "tier1/convar.h"
#include <stdio.h>
#include <vgui_controls/TextEntry.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define DEFAULT_SHOWDURATION 0.2f
#define DEFAULT_HIDEDURATION 0.0f
#define DEFAULT_ANIMDELAY 0.0f
#define DEFAULT_SHOWINTERP INTERPOLATOR_SIMPLESPLINE
#define DEFAULT_HIDEINTERP INTERPOLATOR_LINEAR

using namespace vgui;

CTFDialogPanelBase::CTFDialogPanelBase(vgui::Panel *parent, const char *panelName) : CTFMenuPanelBase(parent, panelName)
{
	Init();
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CTFDialogPanelBase::~CTFDialogPanelBase()
{
	DestroyControls();
}

bool CTFDialogPanelBase::Init()
{
	BaseClass::Init();
	SetKeyBoardInputEnabled(true);
	m_pListPanel = NULL;
	bEmbedded = false;
	bShadedBackground = false;
	bHideMenu = false;

	fShowDelay = 0;
	fShowDuration = DEFAULT_SHOWDURATION;
	fHideDelay = DEFAULT_ANIMDELAY;
	fHideDuration = DEFAULT_HIDEDURATION;
	nShowXStart = 0;
	nShowYStart = 0;
	nHideXEnd = 0;
	nHideYEnd = 0;

	bShowMoves = false;
	bHideMoves = false;
	bDurationAlphaOverride = false;
	bDurationMoveOverride = false;

	bDelayedVisible = false;
	flDelayedVisibleTime = 0.0f;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: sets background color & border
//-----------------------------------------------------------------------------
void CTFDialogPanelBase::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	if (bEmbedded)
	{
		OnCreateControls();
	}
	else
	{
		//Show();
	}

}


void CTFDialogPanelBase::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	bShowMoves = false;
	bHideMoves = false;
	bDurationAlphaOverride = false;
	bDurationMoveOverride = false;
	bHideMenu = false;

	for (KeyValues *pData = inResourceData->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey())
	{
		if (!Q_stricmp( pData->GetName(), "animation" ))
		{
			bShadedBackground = pData->GetBool( "shadedbackground", false );
			bHideMenu = pData->GetBool( "hide_menu", false );

			fShowDelay = pData->GetFloat( "show_delay", DEFAULT_ANIMDELAY );
			fShowDuration = pData->GetFloat( "show_duration", DEFAULT_SHOWDURATION );

			fShowAlphaDuration = pData->GetFloat( "show_alpha_duration", DEFAULT_SHOWDURATION );
			fShowMoveDuration = pData->GetFloat( "show_move_duration", DEFAULT_SHOWDURATION );
			if (fShowAlphaDuration != fShowDuration)
				bDurationAlphaOverride = true;
			if (fShowMoveDuration != fShowDuration)
				bDurationMoveOverride = true;


			fHideDelay = pData->GetFloat( "hide_delay", DEFAULT_ANIMDELAY );
			fHideDuration = pData->GetFloat( "hide_duration", DEFAULT_SHOWDURATION );

			Q_strncpy( pShowAlphaInterp, pData->GetString( "show_alpha_interp", "" ), sizeof( pShowAlphaInterp ) );
			Q_strncpy( pShowMoveInterp, pData->GetString( "show_move_interp", "" ), sizeof( pShowMoveInterp ) );

			nShowXStart = pData->GetInt( "show_start_x", 0 );
			nShowYStart = pData->GetInt( "show_start_y", 0 );
			if (nShowXStart != 0 || nShowYStart != 0)
				bShowMoves = true;

			nHideXEnd = pData->GetInt( "hide_end_x", 0 );
			nHideYEnd = pData->GetInt( "hide_end_y", 0 );
			if (nHideXEnd != 0 || nHideYEnd != 0)
				bHideMoves = true;

		}
	}

	m_eAlphaInterpType = GetInterpolatorType( pShowAlphaInterp );
	m_eMoveInterpType = GetInterpolatorType( pShowMoveInterp );

	//InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

AnimationController::Interpolators_e CTFDialogPanelBase::GetInterpolatorType( char *type)
{
	/* just assume linear by default */
	AnimationController::Interpolators_e interpolator = AnimationController::INTERPOLATOR_LINEAR;

	if (!Q_strcmp( type, "accel" ))
	{
		interpolator = AnimationController::INTERPOLATOR_ACCEL;
	}
	else if (!Q_strcmp( type, "deaccel" ))
	{
		interpolator = AnimationController::INTERPOLATOR_DEACCEL;
	}
	else if (!Q_strcmp( type, "pulse" ))
	{
		interpolator = AnimationController::INTERPOLATOR_PULSE;
	}
	else if (!Q_strcmp( type, "flicker" ))
	{
		interpolator = AnimationController::INTERPOLATOR_FLICKER;
	}
	else if (!Q_strcmp( type, "spline" ))
	{
		interpolator = AnimationController::INTERPOLATOR_SIMPLESPLINE;
	}
	else if (!Q_strcmp( type, "bounce" ))
	{
		interpolator = AnimationController::INTERPOLATOR_BOUNCE;
	}

	return interpolator;
}


void CTFDialogPanelBase::PerformLayout()
{
	BaseClass::PerformLayout();
	m_cShotcutKeys.RemoveAll();
};

void CTFDialogPanelBase::OnCommand(const char* command)
{
	if (!Q_strcmp(command, "vguicancel"))
	{
		PostActionSignal(new KeyValues("CancelPressed"));
		OnResetData();
		Hide();
	}
	else if (!stricmp(command, "Ok"))
	{
		PostActionSignal(new KeyValues("OkPressed"));
		OnApplyChanges();
		Hide();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CTFDialogPanelBase::DelayedVisible(float delay)
{
	bDelayedVisible = true;
	flDelayedVisibleTime = gpGlobals->curtime + delay;
}

void CTFDialogPanelBase::Show()
{
	if (fShowDelay <= 0.0f)
		BaseClass::Show();
	else
		DelayedVisible(fShowDelay);

	if (!bEmbedded)
	{
		RequestFocus();
		MakePopup();
	}

	if (bShowMoves)
	{
		int _x, _y;
		GetPos(_x, _y);
		SetPos( _x - XRES( nShowXStart ), _y - YRES( nShowYStart ));
		AnimationController::PublicValue_t p_AnimHover(_x, _y);
		vgui::GetAnimationController()->RunAnimationCommand( this, "Position", p_AnimHover, fShowDelay, bDurationMoveOverride ? fShowMoveDuration : fShowDuration, m_eMoveInterpType, NULL );
	}

	vgui::GetAnimationController()->RunAnimationCommand( this, "Alpha", 255, fShowDelay, bDurationAlphaOverride ? fShowAlphaDuration : fShowDuration, m_eAlphaInterpType );

	if (bHideMenu)
	{
		if (fShowDelay <= 0.0f)
			MAINMENU_ROOT->HidePanel(CURRENT_MENU);
		else
			MAINMENU_ROOT->SetDelayedHide(CURRENT_MENU, fShowDelay);
	}

	if (bShadedBackground)
	{
		if (fShowDelay <= 0.0f)
			MAINMENU_ROOT->ShowPanel(SHADEBACKGROUND_MENU);
		else
			MAINMENU_ROOT->SetDelayedShow(SHADEBACKGROUND_MENU, fShowDelay);
	}
};

void CTFDialogPanelBase::Hide()
{
	BaseClass::Hide();
	vgui::GetAnimationController()->RunAnimationCommand(this, "Alpha", 0, 0.0f, 0.1f, vgui::AnimationController::INTERPOLATOR_LINEAR);
	if (bHideMenu)
		MAINMENU_ROOT->ShowPanel( CURRENT_MENU );

	if (bShadedBackground)
	{
		MAINMENU_ROOT->HidePanel(SHADEBACKGROUND_MENU);
	}

	if (bShowSingle)
	{
		engine->ClientCmd("gameui_hide");
	}

};

void CTFDialogPanelBase::OnKeyCodePressed(vgui::KeyCode code)
{
	BaseClass::OnKeyCodePressed(code);

	if (code == KEY_ESCAPE)
	{
		Hide();
	}
	else if (!bEmbedded)
	{
		const char *keyName = g_pInputSystem->ButtonCodeToString(code);
		if (Q_strlen(keyName) == 1)
		{
			unsigned int id = m_cShotcutKeys.Find(g_pInputSystem->ButtonCodeToString(code));
			if (id < m_cShotcutKeys.Count())
			{
				const char* cCommand = m_cShotcutKeys[id];
				if (Q_strcmp(cCommand, ""))
					OnCommand(cCommand);
			}
		}
	}
	
}

void CTFDialogPanelBase::AddControl( vgui::Panel* panel, int iType, const char* text )
{
	if ( !m_pListPanel )
		return;

	mpcontrol_t	*pCtrl = new mpcontrol_t( m_pListPanel, "mpcontrol_t" );
	HFont hFont = GETSCHEME()->GetFont( m_pListPanel->GetFontString(), true );

	switch ( iType )
	{
	case O_CATEGORY:
	{
		Label *pTitle = assert_cast<Label*>( panel );
		pTitle->MakeReadyForUse();

		pTitle->SetFont( GETSCHEME()->GetFont( ADVBUTTON_DEFAULT_FONT, true ) );
		pTitle->SetBorder( GETSCHEME()->GetBorder( "AdvSettingsTitleBorder" ) );
		pTitle->SetFgColor( GETSCHEME()->GetColor( ADVBUTTON_DEFAULT_COLOR, COLOR_WHITE ) );
		break;
	}
	case O_BOOL:
	{
		CTFCheckButton *pBox = assert_cast<CTFCheckButton*>( panel );
		pBox->MakeReadyForUse();

		pBox->SetFont( hFont );
		//pBox->SetToolTip(dynamic_cast<CTFAdvCheckButton*>(panel)->GetName());
		break;
	}
	case O_SLIDER:
	{
		CTFSlider *pScroll = assert_cast<CTFSlider*>( panel );
		pScroll->MakeReadyForUse();

		pScroll->SetFont( hFont );
		//pScroll->SetToolTip(dynamic_cast<CTFAdvSlider*>(panel)->GetName());
		break;
	}
	case O_LIST:
	{
		ComboBox *pCombo = assert_cast<ComboBox*>( panel );
		pCombo->MakeReadyForUse();
		pCombo->SetFont( hFont );

		pCtrl->pPrompt = new vgui::Label( pCtrl, "DescLabel", "" );
		pCtrl->pPrompt->MakeReadyForUse();

		pCtrl->pPrompt->SetFont( hFont );
		pCtrl->pPrompt->SetContentAlignment( vgui::Label::a_west );
		pCtrl->pPrompt->SetTextInset( 5, 0 );
		pCtrl->pPrompt->SetText( text );
		break;
	}
	default:
		break;
	}

	panel->SetParent( pCtrl );
	pCtrl->pControl = panel;
	int h = m_pListPanel->GetTall() / 13.0; //(float)GetParent()->GetTall() / 15.0;
	pCtrl->SetSize( 800, h );
	m_pListPanel->AddItem( pCtrl );
}

void CTFDialogPanelBase::CreateControls()
{
	DestroyControls();
}

void CTFDialogPanelBase::DestroyControls()
{
	if (!m_pListPanel)
		return;

	m_pListPanel->DeleteAllItems();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDialogPanelBase::OnResetData()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDialogPanelBase::OnApplyChanges()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFDialogPanelBase::OnSetDefaults()
{

}

void CTFDialogPanelBase::OnThink()
{
}

void CTFDialogPanelBase::OnTick()
{
	if (bDelayedVisible)
	{
		if (flDelayedVisibleTime <= gpGlobals->curtime)
		{
			bDelayedVisible = false;
			SetVisible(true);
		}
	}
};