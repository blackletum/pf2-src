//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef PF_ADVOPTIONSPANEL_H
#define PF_ADVOPTIONSPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "scriptobject.h"

namespace vgui
{
	class Label;
};

class CPFAdvOptionsPanel : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE( CPFAdvOptionsPanel, vgui::EditablePanel );

public:
	CPFAdvOptionsPanel();	 
	CPFAdvOptionsPanel( vgui::Panel *parent );
	virtual ~CPFAdvOptionsPanel();	 

	void Init( void );

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

	void ShowModal();
	virtual void OnCommand( const char* command );

private:
	void CreateControls();
	void DestroyControls();
	void GatherCurrentValues();
	void SaveValues();
	//virtual void OnKeyCodeTyped( vgui::KeyCode code );

private:
	CInfoDescription* m_pDescription;
	mpcontrol_t* m_pList;
	CPanelListPanel* m_pListPanel;

	CPanelAnimationVarAliasType( int, m_iControlWidth, "control_w", "500", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iControlHeight, "control_h", "25", "proportional_int" );
};


CPFAdvOptionsPanel *GPFAdvOptionsPanel();
void DestroyPFAdvOptionsPanel();

#endif // PF_ADVOPTIONSPANEL_H
