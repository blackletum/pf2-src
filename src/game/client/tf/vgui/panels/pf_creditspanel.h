//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef PFCREDITSPANEL_H
#define PFCREDITSPANEL_H
#ifdef _WIN32
#pragma once
#endif


#include "tf_dialogpanelbase.h"

//-----------------------------------------------------------------------------
// Purpose: Displays a game-specific list of options
//-----------------------------------------------------------------------------
class CTFCreditsPanel : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE( CTFCreditsPanel, CTFDialogPanelBase );

public:
	CTFCreditsPanel( vgui::Panel* parent, const char* panelName );
	~CTFCreditsPanel();
	bool Init();
	void OnCommand( const char* command );
	void OnKeyCodeTyped( vgui::KeyCode code );
	void OnApplyChanges();
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void Show();

protected:
	void ApplySchemeSettings( vgui::IScheme* pScheme );
	void CreateControls();
	void DestroyControls();

	KeyValues	*m_pCreditsKV;
};


#endif // MULTIPLAYERADVANCEDDIALOG_H