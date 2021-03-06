//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PFADVOPTIONSDDIALOG_H
#define PFADVOPTIONSDDIALOG_H
#ifdef _WIN32
#pragma once
#endif


#include "tf_dialogpanelbase.h"

class CInfoDescription;
class mpcontrol_t;

//-----------------------------------------------------------------------------
// Purpose: Displays a game-specific list of options
//-----------------------------------------------------------------------------
class CTFPFOptionsPanel : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFPFOptionsPanel, CTFDialogPanelBase);

public:
	CTFPFOptionsPanel(vgui::Panel* parent, const char* panelName);
	~CTFPFOptionsPanel();
	bool Init();
	void OnCommand(const char* command);
	void OnKeyCodeTyped(vgui::KeyCode code);
	void OnApplyChanges();

protected:
	void ApplySchemeSettings(vgui::IScheme* pScheme);
	void CreateControls();
	void DestroyControls();
	void GatherCurrentValues();
	void SaveValues();

	MESSAGE_FUNC(OnControlModified, "ControlModified");

	CInfoDescription* m_pDescription;

	mpcontrol_t* m_pList;
};


#endif // MULTIPLAYERADVANCEDDIALOG_H