#include "cbase.h"
#include "tf_backgroundpanel.h"
#include "tf_mainmenupanel.h"
#include "tf_mainmenu.h"

using namespace vgui;
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEFAULT_RATIO_1610 1920.0 / 1200.0
#define DEFAULT_RATIO_ULTRAWIDE 2560.0 / 1080.0
#define DEFAULT_RATIO_WIDE 1920.0 / 1080.0
#define DEFAULT_RATIO 1024.0 / 768.0 

static void OnBGToggle( IConVar *var, const char *pOldValue, float flOldValue )
{
	if (!MAINMENU_ROOT)
		return;

	if (((ConVar*)var)->GetBool())
		GET_MAINMENUPANEL( CTFBackgroundPanel )->VideoRestart(false);
	else
		GET_MAINMENUPANEL( CTFBackgroundPanel )->VideoRestart(true);
}
ConVar pf2_mainmenu_hidevideo( "pf2_mainmenu_hidevideo", "0", FCVAR_ARCHIVE, "Toggle background video in the main menu", OnBGToggle );
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFBackgroundPanel::CTFBackgroundPanel(vgui::Panel* parent, const char *panelName) : CTFMenuPanelBase(parent, panelName)
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFBackgroundPanel::~CTFBackgroundPanel()
{

}

bool CTFBackgroundPanel::Init()
{
	BaseClass::Init();

	m_pVideo = NULL;
	bInMenu = true;
	bInGame = false;
	return true;
}

void CTFBackgroundPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/UI/main_menu/BackgroundPanel.res");
	m_pVideo = dynamic_cast<CTFVideoPanel *>(FindChildByName("BackgroundVideo"));

	if (m_pVideo)
	{
		int width, height;
		surface()->GetScreenSize(width, height);

		float fRatio = (float)width / (float)height;
		float iRatio = DEFAULT_RATIO;
		//bool bWidescreen = (fRatio < 1.5 ? false : true);
		int nScreenMode = 0;
		if (fRatio > 1.5)
		{
			// Fudge the ratio a bit incase the resolution is decidedly a little bit off
			if (fRatio < 2.1 /*DEFAULT_RATIO_ULTRAWIDE*/)
			{
				nScreenMode = 1;
				iRatio = DEFAULT_RATIO_WIDE;
			}
			else
			{
				nScreenMode = 2;
				iRatio = DEFAULT_RATIO_ULTRAWIDE;
			}	
		}

		Q_strncpy( m_pzVideoLink, GetRandomVideo( nScreenMode ), sizeof( m_pzVideoLink ) );
		int iWide = (float)height * iRatio + 4;
		m_pVideo->SetBounds(-1, -1, iWide, iWide);
	}
}

void CTFBackgroundPanel::PerformLayout()
{
	BaseClass::PerformLayout();
	AutoLayout();
};


void CTFBackgroundPanel::OnCommand(const char* command)
{
	BaseClass::OnCommand(command);
}

void CTFBackgroundPanel::VideoRestart(bool state)
{
	if (!m_pVideo)
		return;

	if (!state)
	{
		m_pVideo->Shutdown();
	}
	else
	{
		int width, height;
		surface()->GetScreenSize( width, height );

		float fRatio = (float)width / (float)height;
		bool bWidescreen = (fRatio < 1.5 ? false : true);

		Q_strncpy( m_pzVideoLink, GetRandomVideo( bWidescreen ), sizeof( m_pzVideoLink ) );
		float iRatio = (bWidescreen ? DEFAULT_RATIO_WIDE : DEFAULT_RATIO);
		int iWide = (float)height * iRatio + 4;
		m_pVideo->SetBounds( -1, -1, iWide, iWide );
		VideoReplay();
	}
	
}

void CTFBackgroundPanel::VideoReplay()
{
	if (!m_pVideo)
		return;
	
	if (IsVisible() && m_pzVideoLink[0] != '\0' && !bInGameLayout && !pf2_mainmenu_hidevideo.GetBool())
	{
		m_pVideo->Activate();
		m_pVideo->BeginPlaybackNoAudio(m_pzVideoLink);
	}
	else
	{
		m_pVideo->Shutdown();
	}
}

void CTFBackgroundPanel::OnTick()
{
	BaseClass::OnTick();
};

void CTFBackgroundPanel::OnThink()
{
	BaseClass::OnThink();
};

void CTFBackgroundPanel::DefaultLayout()
{
	BaseClass::DefaultLayout();
	VideoReplay();
};

void CTFBackgroundPanel::GameLayout()
{
	BaseClass::GameLayout();
	VideoReplay();
};

char* CTFBackgroundPanel::GetRandomVideo(int iScreenMode)
{
	if (pf2_mainmenu_hidevideo.GetBool())
		return "";

	char* pszBasePath = "media/bg_0";
	int iCount = 0;

	for (int i = 0; i < 9; i++)
	{
		char szPath[MAX_PATH];
		char szNumber[5];
		Q_snprintf(szNumber, sizeof(szNumber), "%d", iCount + 1);
		Q_strncpy(szPath, pszBasePath, sizeof(szPath));
		Q_strncat(szPath, szNumber, sizeof(szPath));
		if (iScreenMode == 1)
			Q_strncat(szPath, "_widescreen", sizeof(szPath));
		else if(iScreenMode == 2)
			Q_strncat(szPath, "_ultrawide", sizeof(szPath));
#ifdef OSX
		Q_strncat(szPath, ".mov", sizeof(szPath));
#else
		Q_strncat( szPath, ".bik", sizeof( szPath ) );
#endif
		if (!g_pFullFileSystem->FileExists(szPath))
		{
			if (iCount)
				break;
			else
				return "";
		}
		iCount++;
	}

	int iRand = rand() % iCount;
	char szPath[MAX_PATH];
	char szNumber[5];
	Q_snprintf(szNumber, sizeof(szNumber), "%d", iRand + 1);
	Q_strncpy(szPath, pszBasePath, sizeof(szPath));
	Q_strncat(szPath, szNumber, sizeof(szPath));
	if (iScreenMode == 1)
		Q_strncat(szPath, "_widescreen", sizeof(szPath));
	else if (iScreenMode == 2)
		Q_strncat( szPath, "_ultrawide", sizeof( szPath ) );
#ifdef OSX
	Q_strncat( szPath, ".mov", sizeof( szPath ) );
#else
	Q_strncat( szPath, ".bik", sizeof( szPath ) );
#endif
	char *szResult = (char*)malloc(sizeof(szPath));
	Q_strncpy(szResult, szPath, sizeof(szPath));
	return szResult;
}