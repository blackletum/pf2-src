//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "tf_optionsadvancedpanel.h"
#include <stdio.h>

#include "controls/tf_cvarslider.h"
#include "tf_mainmenu.h"
#include "controls/tf_scriptobject.h"
#include "controls/tf_cvarslider.h"
#include "controls/tf_advbutton.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/QueryBox.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/FileOpenDialog.h"
#include "vgui/ILocalize.h"
#include "vgui/ISystem.h"

#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include <vgui/IInput.h>
#include <tier1/strtools.h>

#include "materialsystem/imaterial.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialvar.h"

#include <filesystem.h>


#define FREEIMAGE_LIB
#include "freeimage/FreeImage.h"

#include <setjmp.h>
#include "ivtex.h"

#ifdef _WIN32
#include <io.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Linux can't do DXT compression so we'll have to cap the height and width
#ifdef _WIN32
#define MAX_VTF_SIZE 512
#else
#define MAX_VTF_SIZE 256
#endif

using namespace vgui;

static char *g_szConversionErrors[] =
{
	"",
	"#GameUI_Spray_Import_Error_Memory",
	"#GameUI_Spray_Import_Error_Reading_Image",
	"#GameUI_Spray_Import_Error_Image_File_Corrupt",
	"#GameUI_Spray_Import_Error_TGA_Format_Not_Supported",
	"#GameUI_Spray_Import_Error_Writing_Temp_Output",
	"#GameUI_Spray_Import_Error_Cant_Load_VTEX_DLL"
};

//=============================================================================
//
// Advanced options panel
// Yes it's called Multiplayer but Multiplayer is taken by what should be Advanced
//
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFOptionsAdvancedPanel::CTFOptionsAdvancedPanel(vgui::Panel *parent, const char *panelName) : CTFDialogPanelBase(parent, panelName)
{
	Init();
	m_hImportSprayDialog = nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFOptionsAdvancedPanel::~CTFOptionsAdvancedPanel()
{
}

bool CTFOptionsAdvancedPanel::Init()
{
	BaseClass::Init();
	m_pCrosshairCombo = nullptr;
	m_pCrosshairScale = nullptr;
	for (int i = 0; i < ARRAYSIZE(m_pCrosshairRGB); ++i)
		m_pCrosshairRGB[i] = nullptr;
	m_pAdvCrosshairImage = new AdvancedCrosshairImagePanel(this, "AdvCrosshairImage");

	m_pSprayList = nullptr;
	m_pSprayImage = nullptr;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: sets background color & border
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/UI/main_menu/OptionsAdvPanel.res");
}

void CTFOptionsAdvancedPanel::CreateControls()
{
	BaseClass::CreateControls();

	//Crosshair stuffs
	m_pAdvCrosshairImage = dynamic_cast<AdvancedCrosshairImagePanel *>(FindChildByName("AdvCrosshairImage"));
	if (m_pAdvCrosshairImage)
		RedrawAdvCrosshairImage();

	m_pCrosshairCombo = dynamic_cast<ComboBox *>(FindChildByName("CrosshairComboBox"));
	if (m_pCrosshairCombo)
	{
		m_pCrosshairCombo->SetNumberOfEditLines(6);
		InitAdvCrosshairStyleList(m_pCrosshairCombo);
	}
	m_pCrosshairScale = dynamic_cast<CTFCvarSlider *>(FindChildByName("CrosshairScaleSlider"));
	m_pCrosshairRGB[0] = dynamic_cast<CTFCvarSlider *>(FindChildByName("CrosshairRedSlider"));
	m_pCrosshairRGB[1] = dynamic_cast<CTFCvarSlider *>(FindChildByName("CrosshairGreenSlider"));
	m_pCrosshairRGB[2] = dynamic_cast<CTFCvarSlider *>(FindChildByName("CrosshairBlueSlider"));

	//Spray stuffs
	m_pSprayImage = dynamic_cast<ImagePanel *>(FindChildByName("SprayImage"));

	m_pSprayList = dynamic_cast<ComboBox *>(FindChildByName("SprayComboBox"));
	if (m_pSprayList)
	{
		m_pSprayList->SetNumberOfEditLines(6);
		InitSprayList(m_pSprayList);
	}
}

void CTFOptionsAdvancedPanel::DestroyControls()
{
	BaseClass::DestroyControls();
}

//-----------------------------------------------------------------------------
// Purpose: Reloads data
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::OnResetData()
{
	BaseClass::OnResetData();
}

//-----------------------------------------------------------------------------
// Purpose: Applies changes
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::OnApplyChanges()
{
	BaseClass::OnApplyChanges();

	char newText[512];
	ConVarRef cl_crosshair_file("cl_crosshair_file");

	if (m_pCrosshairCombo)
	{
		// Set to default if the selected item is none
		if (m_pCrosshairCombo->GetActiveItem() == 0)
		{
			cl_crosshair_file.SetValue(cl_crosshair_file.GetDefault());
		}
		else
		{
			m_pCrosshairCombo->GetItemText( m_pCrosshairCombo->GetActiveItem(), newText, sizeof( newText ) );
			cl_crosshair_file.SetValue( newText );
		}
	}

	if (m_pCrosshairScale)
		m_pCrosshairScale->ApplyChanges();
	for (int i = 0; i < ARRAYSIZE(m_pCrosshairRGB); ++i)
	{
		if(m_pCrosshairRGB[i])
			m_pCrosshairRGB[i]->ApplyChanges();
	}

	if (m_pSprayList)
	{
		ConVarRef cl_logofile("cl_logofile");
		m_pSprayList->GetItemText(m_pSprayList->GetActiveItem(), newText, sizeof(newText));
		char logoDir[512];
		Q_snprintf(logoDir, sizeof(logoDir), "materials/vgui/logos/%s.vtf", newText);
		cl_logofile.SetValue(logoDir);
	}
}

//-----------------------------------------------------------------------------
// Purpose: takes the settings from the crosshair settings combo boxes and sliders
//          and apply it to the crosshair illustrations.
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::RedrawAdvCrosshairImage()
{
	// get the color selected in the combo box.
	int r = 0;
	int g = 0;
	int b = 0;
	if (m_pCrosshairRGB[0])
		r = clamp(m_pCrosshairRGB[0]->GetValue(), 0, 255);
	if (m_pCrosshairRGB[1])
		g = clamp(m_pCrosshairRGB[1]->GetValue(), 0, 255);
	if (m_pCrosshairRGB[2])
		b = clamp(m_pCrosshairRGB[2]->GetValue(), 0, 255);

	float scale;
	if (m_pCrosshairScale)
		scale = m_pCrosshairScale->GetValue();
	else
		scale = 32.0f;

	if (m_pAdvCrosshairImage && m_pCrosshairCombo)
	{
		char crosshairname[256];
		char texture[ 256 ];
		if (m_pCrosshairCombo->GetActiveItem() != 0)
		{
			m_pAdvCrosshairImage->SetVisible( true );
			m_pCrosshairCombo->GetItemText( m_pCrosshairCombo->GetActiveItem(), crosshairname, sizeof( crosshairname ) );
			Q_snprintf( texture, sizeof( texture ), "vgui/crosshairs/%s", crosshairname );
			m_pAdvCrosshairImage->UpdateCrosshair( r, g, b, scale, texture );
		}
		else
		{
			m_pAdvCrosshairImage->SetVisible( false );
		}
	}
}

void CTFOptionsAdvancedPanel::OnSliderMoved()
{
	RedrawAdvCrosshairImage();
}

//-----------------------------------------------------------------------------
// Purpose: Called on controls changing, enables the Apply button
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::OnControlModified()
{
	RedrawAdvCrosshairImage();
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::OnTextChanged()
{
	RemapSpray();
	RedrawAdvCrosshairImage();
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the engine needs to be restarted
//-----------------------------------------------------------------------------
bool CTFOptionsAdvancedPanel::RequiresRestart()
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::OnCommand( const char *command )
{
	if (!Q_stricmp("ImportSprayImage", command))
	{
		if (m_hImportSprayDialog == nullptr)
		{
			m_hImportSprayDialog = new FileOpenDialog(nullptr, "#GameUI_ImportSprayImage", true);
			m_hImportSprayDialog->AddFilter("*.bmp,*.png,*.tga,*.jpg,*.vtf", "#GameUI_All_Images", true);
			m_hImportSprayDialog->AddFilter("*.bmp", "#GameUI_BMP_Images", false);
			m_hImportSprayDialog->AddFilter("*.jpg", "#GameUI_JPEG_Images", false);
			m_hImportSprayDialog->AddFilter("*.png", "#GameUI_PNG_Images", false);
			m_hImportSprayDialog->AddFilter("*.tga", "#GameUI_TGA_Images", false);
			m_hImportSprayDialog->AddFilter("*.vtf", "#GameUI_VTF_Images", false);
			m_hImportSprayDialog->AddActionSignalTarget(this);
		}
		m_hImportSprayDialog->DoModal(false);
		m_hImportSprayDialog->Activate();
	}
	else if (!Q_stricmp("NextCrosshairBG", command))
	{
		if (m_pAdvCrosshairImage)
		{
			m_pAdvCrosshairImage->ChangeCrosshairBG();
		}
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

//-----------------------------------------------------------------------------
// Purpose: initialize the crosshair style list
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::InitAdvCrosshairStyleList( vgui::ComboBox* cb )
{
	// Find out images
	FileFindHandle_t fh;
	char directory[512];

	ConVarRef cl_crosshair_file( "cl_crosshair_file" );
	if ( !cl_crosshair_file.IsValid() )
		return;

	cb->DeleteAllItems();

	cb->AddItem( "#GameUI_None", nullptr );

	char crosshairfile[256];
	Q_snprintf( crosshairfile, sizeof( crosshairfile ), "materials/vgui/crosshairs/%s.vtf", cl_crosshair_file.GetString() );

	Q_snprintf( directory, sizeof( directory ), "materials/vgui/crosshairs/*.vtf" );
	const char* fn = g_pFullFileSystem->FindFirst( directory, &fh );
	int i = 1, initialItem = 0;
	while ( fn )
	{
		char filename[512];
		Q_snprintf( filename, sizeof( filename ), "materials/vgui/crosshairs/%s", fn );
		if ( strlen( filename ) >= 4 )
		{
			filename[strlen( filename ) - 4] = 0;
			Q_strncat( filename, ".vmt", sizeof( filename ), COPY_ALL_CHARACTERS );
			if ( g_pFullFileSystem->FileExists( filename ) )
			{
				// strip off the extension
				Q_strncpy( filename, fn, sizeof( filename ) );
				filename[strlen( filename ) - 4] = 0;
				cb->AddItem(filename, nullptr);

				// check to see if this is the one we have set
				Q_snprintf( filename, sizeof( filename ), "materials/vgui/crosshairs/%s", fn );
				if ( !Q_stricmp( filename, crosshairfile ) )
				{
					initialItem = i;
				}

				++i;
			}
		}

		fn = g_pFullFileSystem->FindNext( fh );
	}

	g_pFullFileSystem->FindClose( fh );
	cb->ActivateItem(initialItem);
}

//=============================================================================
//
// Stuff for handling sprays
//
// file selected.  This can only happen when someone selects an image to be imported as a spray logo.
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::RemapSpray()
{
	char logoname[256];

	if (m_pSprayList == nullptr)
		return;

	m_pSprayList->GetText(logoname, sizeof(logoname));
	if (!logoname[0])
		return;

	char fullLogoName[512];

	// make sure there is a version with the proper shader
	g_pFullFileSystem->CreateDirHierarchy("materials/vgui/logos/ui", "default_write_path");
	Q_snprintf(fullLogoName, sizeof(fullLogoName), "materials/vgui/logos/ui/%s.vmt", logoname);
	if (!g_pFullFileSystem->FileExists(fullLogoName))
	{
		FileHandle_t fp = g_pFullFileSystem->Open(fullLogoName, "wb");
		if (!fp)
			return;

		char data[1024];
		Q_snprintf(data, sizeof(data), "\"UnlitGeneric\"\n\
									   {\n\
									   	// Original shader: BaseTimesVertexColorAlphaBlendNoOverbright\n\
											\"$translucent\" 1\n\
												\"$basetexture\" \"vgui\\logos\\%s\"\n\
													\"$vertexcolor\" 1\n\
														\"$vertexalpha\" 1\n\
															\"$no_fullbright\" 1\n\
																\"$ignorez\" 1\n\
																}\n\
																", logoname);

		g_pFullFileSystem->Write(data, strlen(data), fp);
		g_pFullFileSystem->Close(fp);
	}

	Q_snprintf(fullLogoName, sizeof(fullLogoName), "logos/ui/%s", logoname);
	if (m_pSprayImage)
		m_pSprayImage->SetImage(fullLogoName);
}

void CTFOptionsAdvancedPanel::OnFileSelected(const char *fullpath)
{
	if ((fullpath == NULL) || (fullpath[0] == 0))
	{
		return;
	}

	ConversionErrorType errcode = CE_SUCCESS;

	// this can take a while, put up a waiting cursor
	surface()->SetCursor(dc_hourglass);

	// get the extension of the file we're to convert
	char extension[MAX_PATH];
	const char *constchar = fullpath + strlen(fullpath);
	while ((constchar > fullpath) && (*(constchar-1) != '.'))
	{
		--constchar;
	}
	Q_strncpy(extension, constchar, MAX_PATH);

	bool deleteIntermediateTGA = false;
	bool deleteIntermediateVTF = false;
	bool convertTGAToVTF = true;
	char tgaPath[MAX_PATH*2];
	char *c;
	bool failed = true;

	Q_strncpy(tgaPath, fullpath, sizeof(tgaPath));

	// Convert image if it's not already a tga or a vtf
	if (stricmp(extension, "vtf"))
	{
		// construct a .tga version of this file path.
		c = tgaPath + strlen(tgaPath);
		while ((c > tgaPath) && (*(c-1) != '\\') && (*(c-1) != '/'))
		{
			--c;
		}
		*c = 0;

		char origpath[MAX_PATH*2];
		Q_strncpy(origpath, tgaPath, sizeof(origpath));

		int index = 0;
		do {
			Q_snprintf(tgaPath, sizeof(tgaPath), "%stemp%d.tga", origpath, index);
			++index;
		} while (_access(tgaPath, 0) != -1);

		// All image reading and modifications are done here
		errcode = ConvertImageToTGA(fullpath, tgaPath);

		if (errcode == CE_SUCCESS)
		{
			failed = false;
			deleteIntermediateTGA = true;
		}
	}
	else if (!stricmp(extension, "vtf"))
	{
		// if the file is already in the vtf format there's no need to convert it.
		failed = false;
		convertTGAToVTF = false;
	}

	if (failed)
	{
		vgui::MessageBox *errorDialog = NULL;

		if (errcode > CE_SUCCESS && errcode < CE_ERROR_COUNT)
		{
			errorDialog = new vgui::MessageBox("#GameUI_Spray_Import_Error_Title", g_szConversionErrors[errcode]);
		}

		if (errorDialog != NULL)
		{
			errorDialog->DoModal();
		}
	}

	if (convertTGAToVTF && !failed)
	{
		char tempPath[MAX_PATH*2];
		Q_strncpy(tempPath, tgaPath, sizeof(tempPath));

		errcode = ConvertTGAToVTF(tempPath);
		if (errcode == CE_SUCCESS)
		{
			deleteIntermediateVTF = true;
		}
		else
		{
			failed = true;
			vgui::MessageBox *errorDialog = NULL;

			if (errcode > CE_SUCCESS && errcode < CE_ERROR_COUNT)
			{
				errorDialog = new vgui::MessageBox("#GameUI_Spray_Import_Error_Title", g_szConversionErrors[errcode]);
			}

			if (errorDialog != NULL)
			{
				errorDialog->DoModal();
			}
		}
	}

	char finalPath[MAX_PATH * 2];
	finalPath[0] = 0;
	char vtfPath[MAX_PATH * 2];
	vtfPath[0] = 0;

	if (!failed)
	{
		Q_strncpy(vtfPath, tgaPath, sizeof(vtfPath));

		// rename the tga file to be a vtf file.
		c = vtfPath + strlen(vtfPath);
		while ((c > vtfPath) && (*(c - 1) != '.'))
		{
			--c;
		}
		*c = 0;
		Q_strncat(vtfPath, "vtf", sizeof(vtfPath), COPY_ALL_CHARACTERS);

		// get the vtfFilename from the path.
		const char *vtfFilename = fullpath + strlen(fullpath);
		while ((vtfFilename > fullpath) && (*(vtfFilename - 1) != '\\') && (*(vtfFilename - 1) != '/'))
		{
			--vtfFilename;
		}

		Q_strncpy(finalPath, engine->GetGameDirectory(), sizeof(finalPath));
		Q_strncat(finalPath, "\\materials\\vgui\\logos\\", sizeof(finalPath), COPY_ALL_CHARACTERS);
		Q_FixSlashes(finalPath);
		Q_strncat(finalPath, vtfFilename, sizeof(finalPath), COPY_ALL_CHARACTERS);

		c = finalPath + strlen(finalPath);
		while ((c > finalPath) && (*(c - 1) != '.'))
		{
			--c;
		}
		*c = 0;
		Q_strncat(finalPath, "vtf", sizeof(finalPath), COPY_ALL_CHARACTERS);

		// make sure the directory exists before we try to copy the file.
		g_pFullFileSystem->CreateDirHierarchy("materials/vgui/logos/", "GAME");

		// write out the spray VMT file.
		errcode = WriteSprayVMT(finalPath);
		if (errcode != CE_SUCCESS)
		{
			failed = true;

			vgui::MessageBox *errorDialog = new vgui::MessageBox("#GameUI_Spray_Import_Error_Title", "#GameUI_Spray_Import_Error_Writing_Output");
			errorDialog->DoModal();
		}

		if (!failed)
		{
			// copy vtf file to the final location.
			engine->CopyLocalFile(vtfPath, finalPath);

			// refresh the logo list so the new spray shows up.
			if (m_pSprayList)
				InitSprayList(m_pSprayList);

			char rootFilename[MAX_PATH];
			Q_strncpy(rootFilename, vtfFilename, MAX_PATH);

			// get the root filename so we can select in the spray list.
			rootFilename[strlen(rootFilename) - 4] = 0;

			// automatically select the logo that was just imported.
			if (m_pSprayList)
				SelectSpray(rootFilename);
		}
	}

	// delete the intermediate VTF file if one was made.
	if (deleteIntermediateVTF)
	{
		remove(vtfPath);

		// the TGA->VTF conversion process generates a .txt file if one wasn't already there.
		// in this case, delete the .txt file.
		c = vtfPath + strlen(vtfPath);
		while ((c > vtfPath) && (*(c - 1) != '.'))
		{
			--c;
		}
		Q_strncpy(c, "txt", sizeof(vtfPath) - (c - vtfPath));
		remove(vtfPath);
	}

	// delete the intermediate TGA file if one was made.
	if (deleteIntermediateTGA)
	{
		remove(tgaPath);
	}

	// change the cursor back to normal
	surface()->SetCursor(dc_user);
}

ConversionErrorType CTFOptionsAdvancedPanel::ConvertImageToTGA(const char *imgPath, const char *tgaPath)
{
	FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(imgPath, 0);
	if (formato == FIF_UNKNOWN)
	{
		return CE_SOURCE_FILE_FORMAT_NOT_SUPPORTED;
	}
	FIBITMAP* imagen = FreeImage_Load(formato, imgPath);
	if (!imagen)
	{
		return CE_CANT_OPEN_SOURCE_FILE;
	}
	FIBITMAP* temp = FreeImage_ConvertTo32Bits(imagen);
	if (!imagen)
	{
		FreeImage_Unload(imagen);
		return CE_MEMORY_ERROR;
	}

	FreeImage_Unload(imagen);
	
	int tgaWidth = FreeImage_GetWidth(temp);
	int tgaHeight = FreeImage_GetHeight(temp);

	int paddedImageWidth, paddedImageHeight;

	// get the nearest power of two that is greater than the width of the image.
	paddedImageWidth = tgaWidth;
	if (!IsPowerOfTwo(paddedImageWidth))
	{
		// width is not a power of two, calculate the next highest power of two value.
		int i = 1;
		while (paddedImageWidth > 1)
		{
			paddedImageWidth = paddedImageWidth >> 1;
			++i;
		}

		paddedImageWidth = paddedImageWidth << i;
	}

	// make sure the width is less than or equal to 256
	if (paddedImageWidth > MAX_VTF_SIZE)
	{
		paddedImageWidth = MAX_VTF_SIZE;
	}

	// get the nearest power of two that is greater than the height of the image
	paddedImageHeight = tgaHeight;
	if (!IsPowerOfTwo(paddedImageHeight))
	{
		// height is not a power of two, calculate the next highest power of two value.
		int i = 1;
		while (paddedImageHeight > 1)
		{
			paddedImageHeight = paddedImageHeight >> 1;
			++i;
		}

		paddedImageHeight = paddedImageHeight << i;
	}

	// make sure the height is less than or equal to 256
	if (paddedImageHeight > MAX_VTF_SIZE)
	{
		paddedImageHeight = MAX_VTF_SIZE;
	}

	// compute the amount of stretching that needs to be done to both width and height to get the image to fit.
	float widthRatio = (float)paddedImageWidth / tgaWidth;
	float heightRatio = (float)paddedImageHeight / tgaHeight;

	int finalWidth;
	int finalHeight;

	// compute the final dimensions of the stretched image.
	if (widthRatio < heightRatio)
	{
		finalWidth = paddedImageWidth;
		finalHeight = (int)(tgaHeight * widthRatio + 0.5f);
		// i.e.  for 1x1 size pixels in the resized image we will take color from sourceRatio x sourceRatio sized pixels in the source image.
	}
	else if (heightRatio < widthRatio)
	{
		finalHeight = paddedImageHeight;
		finalWidth = (int)(tgaWidth * heightRatio + 0.5f);
	}
	else
	{
		finalHeight = paddedImageHeight;
		finalWidth = paddedImageWidth;
	}

	imagen = FreeImage_Rescale(temp, finalWidth, finalHeight, FILTER_BICUBIC);
	FreeImage_Unload(temp);

	// NEXT LEVEL OF STUPID TO MAKE THE CANVAS ENLARGEMENT TRANSPARENT
	FIBITMAP *hStupid = FreeImage_Clone(imagen);
	BYTE Transparency[256];
	for (unsigned i = 0; i < 256; i++)
		Transparency[i] = 0x00;
	FreeImage_SetTransparencyTable(hStupid, Transparency, 256);
	RGBQUAD *Palette = FreeImage_GetPalette(hStupid);
	FreeImage_Unload(hStupid);
	// NEXT LEVEL STUPID ENDS HERE

	// Get amount to resize canvas by
	int paddingWidth = (paddedImageWidth - finalWidth) / 2;
	int paddingHeight = (paddedImageHeight - finalHeight) / 2;

	// Incase we get an non ^2 image size
	int paddingErrorWidth = paddedImageWidth - (finalWidth + (2 * paddingWidth));
	int paddingErrorHeight = paddedImageHeight - (finalHeight + (2 * paddingHeight));

	temp = FreeImage_EnlargeCanvas(imagen, paddingWidth + paddingErrorWidth, paddingHeight + paddingErrorHeight, paddingWidth, paddingHeight, &Palette, FI_COLOR_IS_RGBA_COLOR);

	FreeImage_Unload(imagen);
	FreeImage_Save(FIF_TARGA, temp, tgaPath, 0);
	FreeImage_Unload(temp);

	WriteTGACompileParams(tgaPath);

	return CE_SUCCESS;
}

// write a TXT file for the TGA at the given path.
ConversionErrorType CTFOptionsAdvancedPanel::WriteTGACompileParams(const char *tgaPath)
{
	if (tgaPath == NULL)
	{
		return CE_ERROR_WRITING_OUTPUT_FILE;
	}
	
	// make the txt filename
	char txtPath[MAX_PATH * 4];
	Q_strncpy(txtPath, tgaPath, sizeof(txtPath));
	char *c = txtPath + strlen(txtPath);
	while ((c > txtPath) && (*(c - 1) != '.'))
	{
		--c;
	}
	Q_strncpy(c, "txt", sizeof(txtPath) - (c - txtPath));

	// Create a file
	CUtlBuffer outBuf = nullptr;
	g_pFullFileSystem->WriteFile(txtPath, NULL, outBuf);

	FileHandle_t fp = g_pFullFileSystem->Open(txtPath, "wb");
	if (!fp)
		return CE_ERROR_WRITING_OUTPUT_FILE;

	char data[32];
	// nolod ignores mat_picmip settings to always show the hq version, while still using mipmapping so there's no banding
	Q_snprintf(data, sizeof(data), "nolod 1\n");

	// write the contents of the file.
	g_pFullFileSystem->Write(data, strlen(data), fp);
	g_pFullFileSystem->Close(fp);

	return CE_SUCCESS;
}

// convert TGA file at the given location to a VTF file of the same root name at the same location.
ConversionErrorType CTFOptionsAdvancedPanel::ConvertTGAToVTF(const char *tgaPath)
{
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(tgaPath, 0);
	if (format == FIF_UNKNOWN)
	{
		return CE_MEMORY_ERROR;
	}
	FIBITMAP* pImage = FreeImage_Load(format, tgaPath);
	if (!pImage)
	{
		return CE_MEMORY_ERROR;
	}

	int tgaWidth = FreeImage_GetWidth(pImage);
	int tgaHeight = FreeImage_GetHeight(pImage);

	FreeImage_Unload(pImage);

	// check to make sure that the TGA has the proper dimensions and size.
	if (!IsPowerOfTwo(tgaWidth) || !IsPowerOfTwo(tgaHeight))
	{
		return CE_SOURCE_FILE_FORMAT_NOT_SUPPORTED;
	}
	
	// check to make sure that the TGA isn't too big.
	if ((tgaWidth > MAX_VTF_SIZE) || (tgaHeight > MAX_VTF_SIZE))
	{
		return CE_MEMORY_ERROR;
	}

	// load vtex_dll.dll and get the interface to it.
	CSysModule *vtexmod = Sys_LoadModule("vtex_dll");
	if (vtexmod == NULL)
	{
		return CE_ERROR_LOADING_DLL;
	}

	CreateInterfaceFn factory = Sys_GetFactory(vtexmod);
	if (factory == NULL)
	{
		Sys_UnloadModule(vtexmod);
		return CE_ERROR_LOADING_DLL;
	}

	IVTex *vtex = (IVTex *)factory(IVTEX_VERSION_STRING, NULL);
	if (vtex == NULL)
	{
		Sys_UnloadModule(vtexmod);
		return CE_ERROR_LOADING_DLL;
	}

	char *vtfParams[4];

	// the 0th entry is skipped cause normally thats the program name.
	vtfParams[0] = "";
	vtfParams[1] = "-quiet";
	vtfParams[2] = "-dontusegamedir";
	vtfParams[3] = (char *)tgaPath;

	// call vtex to do the conversion.
	vtex->VTex(4, vtfParams);

	Sys_UnloadModule(vtexmod);

	return CE_SUCCESS;
}

// write a VMT file for the spray VTF file at the given path.
ConversionErrorType CTFOptionsAdvancedPanel::WriteSprayVMT(const char *vtfPath)
{
	if (vtfPath == NULL)
	{
		return CE_ERROR_WRITING_OUTPUT_FILE;
	}

	// make the vmt filename
	char vmtPath[MAX_PATH*4];
	Q_strncpy(vmtPath, vtfPath, sizeof(vmtPath));
	char *c = vmtPath + strlen(vmtPath);
	while ((c > vmtPath) && (*(c-1) != '.'))
	{
		--c;
	}
	Q_strncpy(c, "vmt", sizeof(vmtPath) - (c - vmtPath));

	// get the root filename for the vtf file
	char filename[MAX_PATH];
	while ((c > vmtPath) && (*(c-1) != '/') && (*(c-1) != '\\'))
	{
		--c;
	}

	int i = 0;
	while ((*c != 0) && (*c != '.'))
	{
		filename[i++] = *(c++);
	}
	filename[i] = 0;

	// Create a file
	CUtlBuffer outBuf = nullptr;
	g_pFullFileSystem->WriteFile(vmtPath, NULL, outBuf);

	FileHandle_t fp = g_pFullFileSystem->Open(vmtPath, "wb");
	if (!fp)
		return CE_ERROR_WRITING_OUTPUT_FILE;

	char data[512];
	Q_snprintf(data, sizeof(data), "LightmappedGeneric\n{\n\t\"$basetexture\"	\"vgui\\logos\\%s\"\n\t\"$translucent\" \"1\"\n\t\"$decal\" \"1\"\n\t\"$decalscale\" \"0.250\"\n}\n", filename);

	// write the contents of the file.
	g_pFullFileSystem->Write(data, strlen(data), fp);
	g_pFullFileSystem->Close(fp);

	return CE_SUCCESS;
}

//-----------------------------------------------------------------------------
// Purpose: Builds the list of logos
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::InitSprayList(vgui::ComboBox *cb)
{
	// Find out images
	FileFindHandle_t fh;
	char directory[ 512 ];

	ConVarRef cl_logofile( "cl_logofile" );
	if ( !cl_logofile.IsValid() )
		return;

	cb->DeleteAllItems();

	const char *logofile = cl_logofile.GetString();
	Q_snprintf( directory, sizeof( directory ), "materials/vgui/logos/*.vtf" );
	const char *fn = g_pFullFileSystem->FindFirst( directory, &fh );
	int i = 0, initialItem = 0; 
	while (fn)
	{
		char filename[ 512 ];
		Q_snprintf( filename, sizeof(filename), "materials/vgui/logos/%s", fn );
		if ( strlen( filename ) >= 4 )
		{
			filename[ strlen( filename ) - 4 ] = 0;
			Q_strncat( filename, ".vmt", sizeof( filename ), COPY_ALL_CHARACTERS );
			if ( g_pFullFileSystem->FileExists( filename ) )
			{
				// strip off the extension
				Q_strncpy( filename, fn, sizeof( filename ) );
				filename[ strlen( filename ) - 4 ] = 0;
				cb->AddItem( filename, nullptr );

				// check to see if this is the one we have set
				Q_snprintf( filename, sizeof(filename), "materials/vgui/logos/%s", fn );
				if (!Q_stricmp(filename, logofile))
				{
					initialItem = i;
				}

				++i;
			}
		}

		fn = g_pFullFileSystem->FindNext( fh );
	}

	g_pFullFileSystem->FindClose( fh );
	cb->ActivateItem(initialItem);
}

//-----------------------------------------------------------------------------
// Purpose: Selects the given logo in the logo list.
//-----------------------------------------------------------------------------
void CTFOptionsAdvancedPanel::SelectSpray(const char *logoName)
{
	int numEntries = m_pSprayList->GetItemCount();
	int index;
	wchar_t itemText[MAX_PATH];
	wchar_t itemToSelectText[MAX_PATH];

	// convert the logo filename to unicode
	g_pVGuiLocalize->ConvertANSIToUnicode(logoName, itemToSelectText, sizeof(itemToSelectText));

	// find the index of the spray we want.
	for (index = 0; index < numEntries; ++index)
	{
		m_pSprayList->GetItemText(index, itemText, sizeof(itemText));
		if (!wcscmp(itemText, itemToSelectText))
		{
			break;
		}
	}

	if (index < numEntries)
	{
		// select the logo.
		m_pSprayList->ActivateItem(index);
	}
}

//=============================================================================
//
// Crosshair panel
//
//-----------------------------------------------------------------------------
AdvancedCrosshairImagePanel::AdvancedCrosshairImagePanel(Panel *parent, const char *name) : vgui::ImagePanel(parent, name)
{
	m_pAdvCrosshair = NULL;
	m_pFrameVar = NULL;

	m_iCrosshairTextureID = vgui::surface()->CreateNewTextureID();
	UpdateCrosshair(50, 250, 50, 32.0, "vgui/crosshairs/crosshair1");

	PopulateCrosshairBGList();

	n_nCrosshairCurrentBG = 0;
	if (m_CrosshairBGs.Count() > 0)
	{
		this->SetImage(m_CrosshairBGs[n_nCrosshairCurrentBG].String());
	}
}

AdvancedCrosshairImagePanel::~AdvancedCrosshairImagePanel()
{
	if (m_pFrameVar)
	{
		delete m_pFrameVar;
		m_pFrameVar = NULL;
	}

	if (m_pAdvCrosshair)
	{
		delete m_pAdvCrosshair;
		m_pAdvCrosshair = NULL;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void AdvancedCrosshairImagePanel::ChangeCrosshairBG(void)
{
	if (m_CrosshairBGs.Count() > 0)
	{
		++n_nCrosshairCurrentBG;
		if (n_nCrosshairCurrentBG >= m_CrosshairBGs.Count())
			n_nCrosshairCurrentBG = 0;
		this->SetImage(m_CrosshairBGs[n_nCrosshairCurrentBG].String());
	}

}

//-----------------------------------------------------------------------------
void AdvancedCrosshairImagePanel::UpdateCrosshair(int r, int g, int b, float scale, const char *crosshairname)
{
	m_R = r;
	m_G = g;
	m_B = b;

	m_flScale = scale;

	vgui::surface()->DrawSetTextureFile(m_iCrosshairTextureID, crosshairname, true, false);

	if (m_pAdvCrosshair)
	{
		delete m_pAdvCrosshair;
	}

	m_pAdvCrosshair = vgui::surface()->DrawGetTextureMatInfoFactory(m_iCrosshairTextureID);

	Assert(m_pAdvCrosshair);

	m_pFrameVar = m_pAdvCrosshair->FindVarFactory("$frame", NULL);
	m_nNumFrames = m_pAdvCrosshair->GetNumAnimationFrames();

	m_flNextFrameChange = system()->GetFrameTime() + 0.05 /*0.2*/;
	m_bAscending = true;
}

//-----------------------------------------------------------------------------
void AdvancedCrosshairImagePanel::Paint()
{
	BaseClass::Paint();

	int wide, tall;
	GetSize(wide, tall);

	int iClipX0, iClipY0, iClipX1, iClipY1;
	ipanel()->GetClipRect(GetVPanel(), iClipX0, iClipY0, iClipX1, iClipY1);

	// scroll through all frames
	if (m_pFrameVar)
	{
		float curtime = system()->GetFrameTime();

		if (curtime >= m_flNextFrameChange)
		{
			m_flNextFrameChange = curtime + 0.05/*0.2*/;

			int frame = m_pFrameVar->GetIntValue();

			if (m_bAscending)
			{
				frame++;
				if (frame >= m_nNumFrames)
				{
					m_bAscending = !m_bAscending;
					frame--;
				}
			}
			else
			{
				frame--;
				if (frame < 0)
				{
					m_bAscending = !m_bAscending;
					frame++;
				}
			}

			m_pFrameVar->SetIntValue(frame);
		}
	}

	float x, y;

	// assume square
	float flDrawWidth = (m_flScale / 48.0) * (float)wide;
	int flHalfWidth = (int)(flDrawWidth / 2);

	x = wide / 2 - flHalfWidth;
	y = tall / 2 - flHalfWidth;

	vgui::surface()->DrawSetColor(m_R, m_G, m_B, 255);
	vgui::surface()->DrawSetTexture(m_iCrosshairTextureID);
	vgui::surface()->DrawTexturedRect(x, y, x + flDrawWidth, y + flDrawWidth);
	vgui::surface()->DrawSetTexture(0);
}

void AdvancedCrosshairImagePanel::PopulateCrosshairBGList(void)
{
	FileFindHandle_t fh;
	char directory[512];

	m_CrosshairBGs.RemoveAll();

	Q_snprintf(directory, sizeof(directory), "materials/vgui/crosshairs_bg/*.vtf");
	const char* fn = g_pFullFileSystem->FindFirst(directory, &fh);
	while (fn)
	{
		char filename[512];
		Q_snprintf(filename, sizeof(filename), "materials/vgui/crosshairs_bg/%s", fn);
		if (strlen(filename) >= 4)
		{
			filename[strlen(filename) - 4] = 0;
			Q_strncat(filename, ".vmt", sizeof(filename), COPY_ALL_CHARACTERS);
			if (g_pFullFileSystem->FileExists(filename))
			{
				// strip off the extension
				Q_strncpy(filename, fn, sizeof(filename));

				// Writing this very late at night so forgive me if I miss the obvious solution here
				char why[512];
				Q_snprintf(why, sizeof(why), "crosshairs_bg/%s", filename);
				why[strlen(why) - 4] = 0;
				m_CrosshairBGs.AddToTail(why);
			}
		}

		fn = g_pFullFileSystem->FindNext(fh);
	}

	g_pFullFileSystem->FindClose(fh);
}
