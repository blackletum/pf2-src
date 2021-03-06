//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"

#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/QueryBox.h>
#include <vgui_controls/ListPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "ienginevgui.h"
#include "cvartogglecheckbutton.h"
#include "cvarslider.h"
#include "cvartextentry.h"
#include "panellistpanel.h"
#include "pf_advoptionspanel.h"
#include "filesystem.h"

#include <convar.h>
#include "fmtstr.h"
#include <time.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <gameui/gameui_interface.h>

using namespace vgui;

#define OPTIONS_DIR "cfg"
#define DEFAULT_OPTIONS_FILE OPTIONS_DIR "/user_default.scr"
#define OPTIONS_FILE OPTIONS_DIR "/user.scr"

CPFAdvOptionsPanel *g_pPFAdvOptionsPanel = NULL;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPFAdvOptionsPanel *GPFAdvOptionsPanel()
{
	if ( NULL == g_pPFAdvOptionsPanel )
	{
		g_pPFAdvOptionsPanel = new CPFAdvOptionsPanel();
	}
	return g_pPFAdvOptionsPanel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DestroyPFAdvOptionsPanel()
{
	if ( NULL != g_pPFAdvOptionsPanel )
	{
		delete g_pPFAdvOptionsPanel;
		g_pPFAdvOptionsPanel = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPFAdvOptionsPanel::CPFAdvOptionsPanel() : BaseClass( NULL, "TFAdvancedOptionsDialog", 
													 vgui::scheme()->LoadSchemeFromFile( "Resource/ClientScheme.res", "ClientScheme" ) )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::Init( void )
{
	vgui::VPANEL pParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( pParent );
	MakePopup( true );

	m_pListPanel = new CPanelListPanel( this, "PanelListPanel" );

	m_pList = NULL;

	m_pDescription = new CInfoDescription( m_pListPanel );
	m_pDescription->InitFromFile( DEFAULT_OPTIONS_FILE );
	m_pDescription->InitFromFile( OPTIONS_FILE );
	m_pDescription->TransferCurrentValues( NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPFAdvOptionsPanel::CPFAdvOptionsPanel( vgui::Panel *parent ) : BaseClass( parent, "TFAdvancedOptionsDialog", 
																		  vgui::scheme()->LoadSchemeFromFile( "Resource/ClientScheme.res", "ClientScheme" ) )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPFAdvOptionsPanel::~CPFAdvOptionsPanel()
{
	delete m_pDescription;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}


//-----------------------------------------------------------------------------
// Purpose: Applies scheme settings
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetProportional( true );
	LoadControlSettings( "Resource/UI/tfadvancedoptionsdialog.res" );
	CreateControls();
}

//-----------------------------------------------------------------------------
// Purpose: Command handler
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::OnCommand( const char* command )
{
	if ( 0 == Q_stricmp( command, "Close" ) )
	{
		SetVisible( false );
		DestroyControls();
	}
	else if ( 0 == Q_stricmp( command, "Ok" ) )
	{
		SaveValues();
		SetVisible( false );
		DestroyControls();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::GatherCurrentValues()
{
	if ( !m_pDescription )
		return;

	// OK
	CCvarToggleCheckButton* pBox;
	CCvarTextEntry* pEdit;
	ComboBox* pCombo;
	CCvarSlider* pSlider;

	mpcontrol_t* pList;

	CScriptObject* pObj;
	CScriptListItem* pItem;

	char szValue[ 256 ];
	char strValue[ 256 ];

	pList = m_pList;
	while ( pList )
	{
		pObj = pList->pScrObj;

		if ( !pList->pControl )
		{
			pObj->SetCurValue( pObj->defValue );
			pList = pList->next;
			continue;
		}

		switch ( pObj->type )
		{
		case O_BOOL:
			pBox = ( CCvarToggleCheckButton* )pList->pControl;
			sprintf( szValue, "%s", pBox->IsSelected() ? "1" : "0" );
			break;
		case O_NUMBER:
			pEdit = ( CCvarTextEntry* )pList->pControl;
			pEdit->GetText( strValue, sizeof( strValue ) );
			sprintf( szValue, "%s", strValue );
			break;
		case O_SLIDER:
			pSlider = ( CCvarSlider* )pList->pControl;
			sprintf( szValue, "%f", pSlider->GetSliderValue() );
			break;
		case O_STRING:
			pEdit = ( CCvarTextEntry* )pList->pControl;
			pEdit->GetText( strValue, sizeof( strValue ) );
			sprintf( szValue, "%s", strValue );
			break;
		case O_CATEGORY:
			break;
		case O_LIST:
			pCombo = ( ComboBox* )pList->pControl;
			// pCombo->GetText( strValue, sizeof( strValue ) );
			int activeItem = pCombo->GetActiveItem();

			pItem = pObj->pListItems;
			//			int n = (int)pObj->fdefValue;

			while ( pItem )
			{
				if ( !activeItem-- )
					break;

				pItem = pItem->pNext;
			}

			if ( pItem )
			{
				sprintf( szValue, "%s", pItem->szValue );
			}
			else  // Couln't find index
			{
				//assert(!("Couldn't find string in list, using default value"));
				sprintf( szValue, "%s", pObj->defValue );
			}
			break;
		}

		// Remove double quotes and % characters
		UTIL_StripInvalidCharacters( szValue, sizeof( szValue ) );

		strcpy( strValue, szValue );

		pObj->SetCurValue( strValue );

		pList = pList->next;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::CreateControls()
{
	DestroyControls();

	// Go through desciption creating controls
	CScriptObject* pObj;

	pObj = m_pDescription->pObjList;

	mpcontrol_t* pCtrl;

	CCvarToggleCheckButton* pBox;
	CCvarTextEntry* pEdit;
	ComboBox* pCombo;
	CCvarSlider* pSlider;
	Label* pTitle;
	CScriptListItem* pListItem;

	IScheme *pScheme = scheme()->GetIScheme( GetScheme() );
	HFont hFont = pScheme->GetFont( "HudFontSmallestBold", true );
	IBorder *iBorder = pScheme->GetBorder( "OptionsCategoryBorder" );

	Panel* objParent = m_pListPanel;

	while ( pObj )
	{
		if ( pObj->type == O_OBSOLETE )
		{
			pObj = pObj->pNext;
			continue;
		}

		pCtrl = new mpcontrol_t( objParent, "mpcontrol_t" );
		pCtrl->type = pObj->type;

		switch ( pCtrl->type )
		{
		case O_BOOL:
			pBox = new CCvarToggleCheckButton( pCtrl, "DescCheckButton", pObj->prompt, pObj->cvarname );
			pBox->SetFont( hFont );
			pCtrl->pControl = ( Panel* )pBox;
			break;
		case O_STRING:
		case O_NUMBER:
			pEdit = new CCvarTextEntry( pCtrl, "DescTextEntry", pObj->cvarname );
			pCtrl->pControl = ( Panel* )pEdit;
			break;
		case O_SLIDER:
			pSlider = new CCvarSlider( pCtrl, "DescSlider", "", pObj->fMin, pObj->fMax, pObj->cvarname, false );
			pCtrl->pControl = ( Panel* )pSlider;
			break;
		case O_LIST:
			pCombo = new ComboBox( pCtrl, "DescComboBox", 5, false );
			pCombo->SetFont( hFont );
			pListItem = pObj->pListItems;
			while ( pListItem )
			{
				pCombo->AddItem( pListItem->szItemText, NULL );
				pListItem = pListItem->pNext;
			}
			pCombo->ActivateItemByRow( ( int )pObj->fdefValue );

			pCtrl->pControl = ( Panel* )pCombo;
			break;
		case O_CATEGORY:
			pTitle = new Label( pCtrl, "DescTextTitle", pObj->prompt );
			pTitle->SetFont( vgui::scheme()->GetIScheme( GetScheme() )->GetFont( "HudFontSmallBold", true ) );
			pTitle->SetBorder( iBorder );
			pCtrl->pControl = pTitle;
			break;
		default:
			break;
		}

		if ( pCtrl->type != O_BOOL && pCtrl->type != O_CATEGORY )
		{
			pCtrl->pPrompt = new vgui::Label( pCtrl, "DescLabel", "" );
			pCtrl->pPrompt->SetFont( hFont );
			pCtrl->pPrompt->SetContentAlignment( vgui::Label::a_west );
			pCtrl->pPrompt->SetTextInset( 5, 0 );
			pCtrl->pPrompt->SetText( pObj->prompt );
		}

		pCtrl->pScrObj = pObj;
		pCtrl->SetProportional( true );
		pCtrl->SetSize( m_iControlWidth, m_iControlHeight );
		m_pListPanel->AddItem( pCtrl );

		// Link it in
		if ( !m_pList )
		{
			m_pList = pCtrl;
			pCtrl->next = NULL;
		}
		else
		{
			mpcontrol_t* p;
			p = m_pList;
			while ( p )
			{
				if ( !p->next )
				{
					p->next = pCtrl;
					pCtrl->next = NULL;
					break;
				}
				p = p->next;
			}
		}

		pObj = pObj->pNext;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::DestroyControls()
{
	m_pListPanel->DeleteAllItems();
	m_pList = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::SaveValues()
{
	// Get the values from the controls:
	GatherCurrentValues();

	// Create the game.cfg file
	if ( m_pDescription )
	{
		FileHandle_t fp;

		// Add settings to config.cfg
		m_pDescription->WriteToConfig();

		g_pFullFileSystem->CreateDirHierarchy( OPTIONS_DIR );
		fp = g_pFullFileSystem->Open( OPTIONS_FILE, "wb" );
		if ( fp )
		{
			m_pDescription->WriteToScriptFile( fp );
			g_pFullFileSystem->Close( fp );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor, load/save client settings object
//-----------------------------------------------------------------------------
CInfoDescription::CInfoDescription( CPanelListPanel* panel )
	: CDescription( panel )
{
	setHint( "// NOTE:  THIS FILE IS AUTOMATICALLY REGENERATED, \r\n\
//DO NOT EDIT THIS HEADER, YOUR COMMENTS WILL BE LOST IF YOU DO\r\n\
// User options script\r\n\
//\r\n\
// Format:\r\n\
//  Version [float]\r\n\
//  Options description followed by \r\n\
//  Options defaults\r\n\
//\r\n\
// Option description syntax:\r\n\
//\r\n\
//  \"cvar\" { \"Prompt\" { type [ type info ] } { default } }\r\n\
//\r\n\
//  type = \r\n\
//   BOOL   (a yes/no toggle)\r\n\
//   STRING\r\n\
//   NUMBER\r\n\
//   LIST\r\n\
//\r\n\
// type info:\r\n\
// BOOL                 no type info\r\n\
// NUMBER       min max range, use -1 -1 for no limits\r\n\
// STRING       no type info\r\n\
// LIST         "" delimited list of options value pairs\r\n\
//\r\n\
//\r\n\
// default depends on type\r\n\
// BOOL is \"0\" or \"1\"\r\n\
// NUMBER is \"value\"\r\n\
// STRING is \"value\"\r\n\
// LIST is \"index\", where index \"0\" is the first element of the list\r\n\r\n\r\n" );

	setDescription( "INFO_OPTIONS" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInfoDescription::WriteScriptHeader( FileHandle_t fp )
{
	char am_pm[] = "AM";
	tm newtime;
	VCRHook_LocalTime( &newtime );

	// Write out the comment and Cvar Info:
	g_pFullFileSystem->FPrintf( fp, "// Half-Life User Info Configuration Layout Script (stores last settings chosen, too)\r\n" );
	g_pFullFileSystem->FPrintf( fp, "// File generated:  %.19s %s\r\n", asctime( &newtime ), am_pm );
	g_pFullFileSystem->FPrintf( fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n" );
	g_pFullFileSystem->FPrintf( fp, "VERSION %.1f\r\n\r\n", SCRIPT_VERSION );
	g_pFullFileSystem->FPrintf( fp, "DESCRIPTION INFO_OPTIONS\r\n{\r\n" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CInfoDescription::WriteFileHeader( FileHandle_t fp )
{
	char am_pm[] = "AM";
	tm newtime;
	VCRHook_LocalTime( &newtime );

	g_pFullFileSystem->FPrintf( fp, "// Half-Life User Info Configuration Settings\r\n" );
	g_pFullFileSystem->FPrintf( fp, "// DO NOT EDIT, GENERATED BY HALF-LIFE\r\n" );
	g_pFullFileSystem->FPrintf( fp, "// File generated:  %.19s %s\r\n", asctime( &newtime ), am_pm );
	g_pFullFileSystem->FPrintf( fp, "//\r\n//\r\n// Cvar\t-\tSetting\r\n\r\n" );
}

//-----------------------------------------------------------------------------
// Purpose: Shows this dialog as a modal dialog
//-----------------------------------------------------------------------------
void CPFAdvOptionsPanel::ShowModal()
{
	InvalidateLayout( false, true );
	SetVisible( true );
	MoveToFront();
}

CON_COMMAND( OpenAdvOptions, "Shows the advanced options dialog" )
{
	GPFAdvOptionsPanel()->ShowModal();
}