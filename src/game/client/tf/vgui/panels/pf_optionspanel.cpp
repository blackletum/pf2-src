//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <time.h>

#include "pf_optionspanel.h"
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
#include <vgui/ISurface.h>
#include <vgui_controls/ListPanel.h>
#include <KeyValues.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/TextEntry.h>
#include <vgui/IInput.h>

#include <tier0/vcrmode.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define OPTIONS_DIR "cfg"
#define DEFAULT_OPTIONS_FILE OPTIONS_DIR "/pf_default.scr"
#define OPTIONS_FILE OPTIONS_DIR "/pf.scr"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFPFOptionsPanel::CTFPFOptionsPanel(vgui::Panel* parent, const char* panelName) : CTFDialogPanelBase(parent, panelName)
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFPFOptionsPanel::~CTFPFOptionsPanel()
{
	delete m_pDescription;
}


bool CTFPFOptionsPanel::Init()
{
	BaseClass::Init();

	m_pListPanel = new CPanelListPanel(this, "PanelListPanel");
	m_pList = NULL;

	m_pDescription = new CInfoDescription(m_pListPanel);
	m_pDescription->InitFromFile(DEFAULT_OPTIONS_FILE);
	m_pDescription->TransferCurrentValues(NULL);

	return true;
}

void CTFPFOptionsPanel::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/UI/main_menu/OptionsPF2Panel.res");
}

void CTFPFOptionsPanel::OnCommand(const char* command)
{
	BaseClass::OnCommand(command);
}

void CTFPFOptionsPanel::OnKeyCodeTyped(KeyCode code)
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Hide();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPFOptionsPanel::GatherCurrentValues()
{
	if (!m_pDescription)
		return;

	// OK
	CTFCheckButton* pBox;
	TextEntry* pEdit;
	ComboBox* pCombo;
	CTFSlider* pScroll;

	mpcontrol_t* pList;

	CScriptObject* pObj;
	CScriptListItem* pItem;

	char szValue[256];

	pList = m_pList;
	while (pList)
	{
		pObj = pList->pScrObj;

		if (!pList->pControl)
		{
			pObj->SetCurValue(pObj->curValue);
			pList = pList->next;
			continue;
		}

		switch (pObj->type)
		{
		case O_BOOL:
			pBox = (CTFCheckButton*)pList->pControl;
			sprintf(szValue, "%d", pBox->IsChecked() ? 1 : 0);
			break;
		case O_NUMBER:
			pEdit = (TextEntry*)pList->pControl;
			pEdit->GetText(szValue, sizeof(szValue));
			break;
		case O_SLIDER:
			pScroll = (CTFSlider*)pList->pControl;
			V_strncpy(szValue, pScroll->GetFinalValue(), sizeof(szValue));
			break;
		case O_STRING:
			pEdit = (TextEntry*)pList->pControl;
			pEdit->GetText(szValue, sizeof(szValue));
			break;
		case O_CATEGORY:
			break;
		case O_LIST:
			pCombo = (ComboBox*)pList->pControl;
			pCombo->GetText(szValue, sizeof(szValue));
			int activeItem = pCombo->GetActiveItem();

			pItem = pObj->pListItems;
			//			int n = (int)pObj->fcurValue;

			while (pItem)
			{
				if (!activeItem--)
					break;

				pItem = pItem->pNext;
			}

			if (pItem)
			{
				sprintf(szValue, "%s", pItem->szValue);
			}
			else  // Couln't find index
			{
				//assert(!("Couldn't find string in list, using default value"));
				sprintf(szValue, "%s", pObj->curValue);
			}
			break;
		}

		// Remove double quotes and % characters
		UTIL_StripInvalidCharacters(szValue, sizeof(szValue));

		pObj->SetCurValue(szValue);

		pList = pList->next;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPFOptionsPanel::CreateControls()
{
	BaseClass::CreateControls();

	// Go through desciption creating controls
	CScriptObject* pObj;
	pObj = m_pDescription->pObjList;

	mpcontrol_t* pCtrl;
	CTFCvarToggleCheckButton* pBox;
	TextEntry* pEdit;
	ComboBox* pCombo;
	CTFCvarSlider* pScroll;
	Label* pTitle;
	CScriptListItem* pListItem;

	HFont hFont = GETSCHEME()->GetFont(m_pListPanel->GetFontString(), true);

	Panel* objParent = m_pListPanel;
	while (pObj)
	{
		//Msg("\nAdded: %s %s %f %f %i\n", pObj->prompt, pObj->cvarname, pObj->fcurValue, pObj->fcurValue, pObj->type);

		if (pObj->type == O_OBSOLETE)
		{
			pObj = pObj->pNext;
			continue;
		}

		pCtrl = new mpcontrol_t(objParent, "mpcontrol_t");
		pCtrl->type = pObj->type;

		switch (pCtrl->type)
		{
		case O_BOOL:
			pBox = new CTFCvarToggleCheckButton(pCtrl, "DescCheckButton", pObj->prompt, pObj->cvarname);
			pBox->MakeReadyForUse();

			pBox->SetFont(hFont);
			pBox->SetToolTip(pObj->tooltip);

			pCtrl->pControl = pBox;
			break;
		case O_STRING:
		case O_NUMBER:
			pEdit = new TextEntry(pCtrl, "DescTextEntry");
			pEdit->MakeReadyForUse();

			pEdit->InsertString(pObj->curValue);

			pCtrl->pControl = pEdit;
			break;
		case O_SLIDER:
			pScroll = new CTFCvarSlider(pCtrl, "DescScrollEntry", pObj->prompt, pObj->fMin, pObj->fMax, pObj->cvarname);
			pScroll->MakeReadyForUse();

			pScroll->SetFont(hFont);
			pScroll->GetButton()->SetToolTip(pObj->tooltip);

			pCtrl->pControl = pScroll;
			break;
		case O_LIST:
			pCombo = new ComboBox(pCtrl, "DescComboBox", 5, false);
			pCombo->MakeReadyForUse();
			pCombo->SetFont(hFont);

			pListItem = pObj->pListItems;
			while (pListItem)
			{
				pCombo->AddItem(pListItem->szItemText, NULL);
				pListItem = pListItem->pNext;
			}

			pCombo->ActivateItemByRow((int)pObj->fcurValue);

			pCtrl->pControl = pCombo;
			break;
		case O_CATEGORY:
			pTitle = new Label(pCtrl, "DescTextTitle", pObj->prompt);
			pTitle->MakeReadyForUse();

			pTitle->SetBorder(GETSCHEME()->GetBorder("AdvSettingsTitleBorder"));
			pTitle->SetFont(GETSCHEME()->GetFont("MenuSmallFont", true));
			pTitle->SetFgColor(GETSCHEME()->GetColor(ADVBUTTON_DEFAULT_COLOR, COLOR_WHITE));

			pCtrl->pControl = pTitle;
			break;
		default:
			break;
		}

		if (pCtrl->type != O_BOOL && pCtrl->type != O_SLIDER && pCtrl->type != O_CATEGORY)
		{
			pCtrl->pPrompt = new Label(pCtrl, "DescLabel", "");
			pCtrl->pPrompt->MakeReadyForUse();

			pCtrl->pPrompt->SetFont(hFont);
			pCtrl->pPrompt->SetContentAlignment(vgui::Label::a_west);
			pCtrl->pPrompt->SetTextInset(5, 0);
			pCtrl->pPrompt->SetText(pObj->prompt);
		}

		pCtrl->pScrObj = pObj;
		int h = m_pListPanel->GetTall() / 13.0; //(float)GetParent()->GetTall() / 15.0;
		pCtrl->SetSize(800, h);
		//pCtrl->SetBorder( scheme()->GetBorder(1, "DepressedButtonBorder") );
		m_pListPanel->AddItem(pCtrl);

		// Link it in
		if (!m_pList)
		{
			m_pList = pCtrl;
			pCtrl->next = NULL;
		}
		else
		{
			mpcontrol_t* p;
			p = m_pList;
			while (p)
			{
				if (!p->next)
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
void CTFPFOptionsPanel::DestroyControls()
{
	BaseClass::DestroyControls();
	m_pList = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPFOptionsPanel::SaveValues()
{
	// Get the values from the controls:
	GatherCurrentValues();

	// Create the game.cfg file
	if (m_pDescription)
	{
		FileHandle_t fp;

		// Add settings to config.cfg
		m_pDescription->WriteToConfig();

		g_pFullFileSystem->CreateDirHierarchy(OPTIONS_DIR);
		fp = g_pFullFileSystem->Open(OPTIONS_FILE, "wb");
		if (fp)
		{
			m_pDescription->WriteToScriptFile(fp);
			g_pFullFileSystem->Close(fp);
		}
	}
}

void CTFPFOptionsPanel::OnApplyChanges()
{
	BaseClass::OnApplyChanges();
	SaveValues();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFPFOptionsPanel::OnControlModified(void)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}