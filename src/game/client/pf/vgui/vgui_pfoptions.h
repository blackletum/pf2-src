#ifndef VGUI_PF_OPTIONS_H
#define VGUI_PF_OPTIONS_H

#include "cbase.h"
#include "vgui_controls/Controls.h"

#include "vgui_controls/Frame.h"
#include "vgui_controls/PropertySheet.h"
#include <vgui/KeyCode.h>

class CVGUIPreFortressOptions : public vgui::Frame
{
public:
	DECLARE_CLASS_SIMPLE( CVGUIPreFortressOptions, vgui::Frame );

	CVGUIPreFortressOptions( vgui::VPANEL parent, const char *pName );
	~CVGUIPreFortressOptions();

	void OnCommand( const char *cmd );

	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel );
	//MESSAGE_FUNC_PTR( OnSliderMoved, "SliderMoved", panel );
	//MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pKV );
	//MESSAGE_FUNC_PARAMS( OnPicked, "ColorPickerPicked", pKV );
	void ReadValues( bool write );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void PerformLayout();

private:
	virtual void OnKeyCodeTyped( vgui::KeyCode code );
	

	vgui::PropertySheet		*m_pPropertySheet;
	vgui::PropertyPage		*pPageGameplay;
	vgui::PropertyPage		*pPagePerformance;

	//Game
	vgui::CheckButton	*m_pCheck_MuzzleFlash;
	vgui::CheckButton	*m_pCheck_ColoredCloak;
	vgui::CheckButton	*m_pCheck_UseMinVM;
	vgui::CheckButton	*m_pCheck_RPCEnabled;
	vgui::CheckButton	*m_pCheck_RPCClassIcons;

	//Graphics
	vgui::CheckButton	*m_pCheck_CheapSplashes;
	vgui::CheckButton	*m_pCheck_MuzzleLight;
	vgui::CheckButton	*m_pCheck_MuzzleLightProps;
	vgui::CheckButton	*m_pCheck_BurningLight;
	vgui::CheckButton	*m_pCheck_ProjectileLight;
	vgui::Slider		*m_pSlider_PicmipLevel;
	vgui::CheckButton	*m_pCheck_Violence;
	vgui::Slider		*m_pSlider_BloodDrops;
	vgui::Slider		*m_pSlider_BloodSpray;
	vgui::Slider		*m_pSlider_BloodPuff;

	//vgui::Label			*m_pLabel_Unused;
	//vgui::ComboBox		*m_pCBox_Unused;
};


#endif