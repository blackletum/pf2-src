#ifndef TFMAINMENURGBPANEL_H
#define TFMAINMENURGBPANEL_H

#include "tf_menupanelbase.h"

class CTFCvarSlider;
class CCvarComboBox;
class CTFAdvModelPanel;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFRGBPanel : public CTFMenuPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFRGBPanel, CTFMenuPanelBase);

public:
	CTFRGBPanel(vgui::Panel* parent, const char *panelName);
	bool Init();
	virtual ~CTFRGBPanel();

	void PerformLayout();
	void OnCommand(const char* command);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:

	CTFCvarSlider	*m_pRedScrollBar;
	CTFCvarSlider	*m_pGrnScrollBar;
	CTFCvarSlider	*m_pBluScrollBar;
	vgui::ImagePanel *m_pColorBG;
	CCvarComboBox *m_pCombo;
	CTFAdvModelPanel *m_pParticlesPanel;
	MESSAGE_FUNC(OnDataChanged, "ControlModified");
};

#endif // TFMAINMENURGBPANEL_H