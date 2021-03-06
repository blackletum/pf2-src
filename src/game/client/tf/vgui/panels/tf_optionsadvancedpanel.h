//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OPTIONS_SUB_ADVANCED_H
#define OPTIONS_SUB_ADVANCED_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_dialogpanelbase.h"

class CTFCvarSlider;
class CTFCheckButton;
class AdvancedCrosshairImagePanel;


enum ConversionErrorType
{
	CE_SUCCESS,
	CE_MEMORY_ERROR,
	CE_CANT_OPEN_SOURCE_FILE,
	CE_ERROR_PARSING_SOURCE,
	CE_SOURCE_FILE_FORMAT_NOT_SUPPORTED,
	CE_ERROR_WRITING_OUTPUT_FILE,
	CE_ERROR_LOADING_DLL,
	CE_ERROR_COUNT
};

//=============================================================================
//
// Crosshair panel
//
//-----------------------------------------------------------------------------
class AdvancedCrosshairImagePanel : public vgui::ImagePanel
{
	DECLARE_CLASS_SIMPLE(AdvancedCrosshairImagePanel, vgui::ImagePanel);
public:
	AdvancedCrosshairImagePanel(Panel *parent, const char *name);
	virtual ~AdvancedCrosshairImagePanel();

	virtual void Paint();

	void UpdateCrosshair(int r, int g, int b, float scale, const char *crosshairname);

	void PopulateCrosshairBGList(void);

	void ChangeCrosshairBG(void);

protected:
	int m_R, m_G, m_B;
	float m_flScale;

	// material
	int				m_iCrosshairTextureID;
	IVguiMatInfo	*m_pAdvCrosshair;

	// animation
	IVguiMatInfoVar	*m_pFrameVar;
	float			m_flNextFrameChange;
	int				m_nNumFrames;
	bool			m_bAscending;	// animating forward or in reverse?

	CUtlVector<CUtlSymbol> 	m_CrosshairBGs;		// array of selected rows
	int						n_nCrosshairCurrentBG;

};

//-----------------------------------------------------------------------------
// Purpose: Audio Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class CTFOptionsAdvancedPanel : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFOptionsAdvancedPanel, CTFDialogPanelBase);

public:
	CTFOptionsAdvancedPanel(vgui::Panel* parent, const char *panelName);
	~CTFOptionsAdvancedPanel();
	virtual bool Init();
	virtual void OnResetData();
	virtual void OnApplyChanges();
	virtual void OnCommand(const char *command);
	bool RequiresRestart();
protected:
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void CreateControls();
	void DestroyControls();

private:
	void InitAdvCrosshairStyleList( vgui::ComboBox* cb );

	void InitSprayList(vgui::ComboBox *cb);
	void RemapSpray();

	// Begin Spray Import Functions
	//ConversionErrorType ConvertJPEGToTGA(const char *jpgPath, const char *tgaPath);
	ConversionErrorType ConvertImageToTGA(const char *imgPath, const char *tgaPath);
	ConversionErrorType ConvertTGAToVTF(const char *tgaPath);
	ConversionErrorType WriteSprayVMT(const char *vtfPath);
	ConversionErrorType WriteTGACompileParams(const char *tgaPath);
	void SelectSpray(const char *logoName);
	// End Spray Import Functions

	// --- advanced crosshair controls
	void RedrawAdvCrosshairImage();
	// -----

private:
	MESSAGE_FUNC( OnControlModified, "ControlModified" );
	MESSAGE_FUNC( OnTextChanged, "TextChanged" );
	MESSAGE_FUNC( OnSliderMoved, "SliderMoved");
	MESSAGE_FUNC_CHARPTR( OnFileSelected, "FileSelected", fullpath);

	//Actually used vars
	vgui::ComboBox			*m_pCrosshairCombo;
	CTFCvarSlider			*m_pCrosshairScale;
	CTFCvarSlider			*m_pCrosshairRGB[3];
	AdvancedCrosshairImagePanel *m_pAdvCrosshairImage;

	vgui::FileOpenDialog	*m_hImportSprayDialog;

	vgui::ImagePanel *m_pSprayImage;
	vgui::ComboBox *m_pSprayList;
};

#endif // OPTIONS_SUB_MULTIPLAYER_H
