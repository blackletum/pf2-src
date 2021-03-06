#ifndef TFDIALOGPANELBASE_H
#define TFDIALOGPANELBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_menupanelbase.h"

class CPanelListPanel;

class CTFDialogPanelBase : public CTFMenuPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFDialogPanelBase, CTFMenuPanelBase);

public:
	CTFDialogPanelBase(vgui::Panel* parent, const char *panelName);
	~CTFDialogPanelBase();
	virtual bool Init();
	virtual void OnResetData();
	virtual void OnApplyChanges();
	virtual void OnSetDefaults();
	virtual void OnCommand(const char *command);
	virtual void OnCreateControls() { CreateControls(); OnResetData(); };
	virtual void OnDestroyControls() { DestroyControls(); };
	virtual void OnThink();
	virtual void OnTick();
	virtual void DelayedVisible(float delay);
	virtual void Show();
	virtual void Hide();
	virtual void SetEmbedded(bool bState) { bEmbedded = bState; };
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void AddShortcut(const char *sCode, const char *sCommand){ m_cShotcutKeys.Insert(sCode, sCommand); };

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void PerformLayout();
	virtual void CreateControls();
	virtual void DestroyControls();
	virtual void AddControl(vgui::Panel* panel, int iType, const char* text = "");
	virtual vgui::AnimationController::Interpolators_e GetInterpolatorType( char *type );

	bool			bShadedBackground;

	float			fShowDelay;
	float			fShowDuration;

	bool			bDurationAlphaOverride;
	float			fShowAlphaDuration;
	bool			bDurationMoveOverride;
	float			fShowMoveDuration;

	char			pShowAlphaInterp[64];
	char			pShowMoveInterp[64];

	int				nShowXStart;
	int				nShowYStart;

	bool			bShowMoves;

	float			fHideDelay;
	float			fHideDuration;
	int				nHideInterp;
	int				nHideXEnd;
	int				nHideYEnd;

	bool			bDelayedVisible;
	float			flDelayedVisibleTime;

	bool			bHideMoves;

	vgui::AnimationController::Interpolators_e m_eMoveInterpType;
	vgui::AnimationController::Interpolators_e m_eAlphaInterpType;


	bool			bEmbedded;
	bool			bHideMenu;
	CPanelListPanel *m_pListPanel;
	CUtlDict<const char*, unsigned short> m_cShotcutKeys;
};



#endif // TFDIALOGPANELBASE_H