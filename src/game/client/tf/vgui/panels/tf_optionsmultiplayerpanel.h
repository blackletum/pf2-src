//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MULTIPLAYERADVANCEDDIALOG_H
#define MULTIPLAYERADVANCEDDIALOG_H
#ifdef _WIN32
#pragma once
#endif


#include "tf_dialogpanelbase.h"

class CInfoDescription;
class mpcontrol_t;

//-----------------------------------------------------------------------------
// Purpose: Displays a game-specific list of options
//-----------------------------------------------------------------------------
class CTFOptionsMultiplayerPanel : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFOptionsMultiplayerPanel, CTFDialogPanelBase);

public:
	CTFOptionsMultiplayerPanel(vgui::Panel *parent, const char *panelName);
	~CTFOptionsMultiplayerPanel();
	bool Init();
	void OnCommand(const char *command);
	void OnKeyCodeTyped(vgui::KeyCode code);
	void OnApplyChanges();

protected:
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void CreateControls();
	void DestroyControls();
	void GatherCurrentValues();
	void SaveValues();

	MESSAGE_FUNC( OnControlModified, "ControlModified" );

	CInfoDescription *m_pDescription;

	mpcontrol_t *m_pList;
};


#endif // MULTIPLAYERADVANCEDDIALOG_H