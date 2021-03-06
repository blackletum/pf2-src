#include "cbase.h"
#include "tf_loadoutpanel.h"
#include "tf_mainmenu.h"
#include "controls/tf_advbutton.h"
#include "controls/tf_advmodelpanel.h"
#include "tf_rgbpanel.h"
#include "basemodelpanel.h"
#include <vgui/ILocalize.h>
#include "c_script_parser.h"

using namespace vgui;
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#define PANEL_WIDE YRES(140)
#define PANEL_TALL YRES(70)
#define PANEL_X_OFFSET YRES(10)
#define PANEL_Y_OFFSET YRES(5)
#define TF_WEAPON_SAPPER "TF_WEAPON_SAPPER"

static const char *pszClassModels[TF_CLASS_COUNT_ALL] =
{
	"",
	"models/player/scout.mdl",
	"models/player/sniper.mdl",
	"models/player/soldier.mdl",
	"models/player/demo.mdl",
	"models/player/medic.mdl",
	"models/player/heavy.mdl",
	"models/player/pyro.mdl",
	"models/player/spy.mdl",
	"models/player/engineer.mdl",
	"",
};

struct _WeaponData
{
	char szWorldModel[64];
	char iconActive[64];
	char iconInactive[64];
	char szPrintName[64];
	int m_iWeaponType;
	bool bHasTeamSkins;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFWeaponSetPanel::CTFWeaponSetPanel( vgui::Panel* parent, const char *panelName ) : EditablePanel( parent, panelName )
{
}

void CTFWeaponSetPanel::OnCommand( const char* command )
{
	GetParent()->OnCommand( command );
}

class CTFWeaponScriptParser : public C_ScriptParser
{
public:
	DECLARE_CLASS_GAMEROOT( CTFWeaponScriptParser, C_ScriptParser );

	void Parse( KeyValues *pKeyValuesData, bool bWildcard, const char *szFileWithoutEXT )
	{
		_WeaponData sTemp;
		Q_strncpy( sTemp.szWorldModel, pKeyValuesData->GetString( "playermodel", "" ), sizeof( sTemp.szWorldModel ) );
		Q_strncpy( sTemp.szPrintName, pKeyValuesData->GetString( "printname", "" ), sizeof( sTemp.szPrintName ) );
		const char *pszWeaponType = pKeyValuesData->GetString( "WeaponType" );
		sTemp.m_iWeaponType = 0;
		if (!Q_strcmp(pszWeaponType, "primary"))
		{
			sTemp.m_iWeaponType = TF_WPN_TYPE_PRIMARY;
		}
		else if (!Q_strcmp(pszWeaponType, "secondary"))
		{
			sTemp.m_iWeaponType = TF_WPN_TYPE_SECONDARY;
		}
		else if (!Q_strcmp(pszWeaponType, "melee"))
		{
			sTemp.m_iWeaponType = TF_WPN_TYPE_MELEE;
		}
		else if (!Q_strcmp(pszWeaponType, "grenade"))
		{
			sTemp.m_iWeaponType = TF_WPN_TYPE_GRENADE;
		}
		else if (!Q_strcmp(pszWeaponType, "building"))
		{
			sTemp.m_iWeaponType = TF_WPN_TYPE_BUILDING;
		}
		else if (!Q_strcmp(pszWeaponType, "pda"))
		{
			sTemp.m_iWeaponType = TF_WPN_TYPE_PDA;
		}
		else if (!Q_strcmp(pszWeaponType, "scoutshotgun"))
		{
			sTemp.m_iWeaponType = TF_WPN_TYPE_SCOUTSHOTGUN;
		}
		for (KeyValues *pData = pKeyValuesData->GetFirstSubKey(); pData != NULL; pData = pData->GetNextKey())
		{
			if (!Q_stricmp(pData->GetName(), "TextureData"))
			{
				for (KeyValues *pTextureData = pData->GetFirstSubKey(); pTextureData != NULL; pTextureData = pTextureData->GetNextKey())
				{
					if (!Q_stricmp(pTextureData->GetName(), "weapon"))
					{
						Q_strncpy(sTemp.iconInactive, pTextureData->GetString("file", ""), sizeof(sTemp.iconInactive));
					}
					if (!Q_stricmp( pTextureData->GetName(), "weapon_s" ))
					{
						Q_strncpy( sTemp.iconActive, pTextureData->GetString( "file", "" ), sizeof( sTemp.iconActive ) );
					}
				}
			}
		}
		m_WeaponInfoDatabase.Insert(szFileWithoutEXT, sTemp);
	};

	_WeaponData GetTFWeaponInfo(const char *name)
	{
		return m_WeaponInfoDatabase[m_WeaponInfoDatabase.Find(name)];
	}

private:
	CUtlDict< _WeaponData, unsigned short > m_WeaponInfoDatabase;
};
CTFWeaponScriptParser g_TFWeaponScriptParser;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFLoadoutPanel::CTFLoadoutPanel( Panel *parent, const char *panelName ) : CTFDialogPanelBase( parent, panelName )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTFLoadoutPanel::~CTFLoadoutPanel()
{

}

bool CTFLoadoutPanel::Init()
{
	BaseClass::Init();

	iCurrentClass = TF_CLASS_SCOUT;
	iCurrentSlot = TF_WPN_TYPE_PRIMARY;
	iCurrentPreset = 0;
	m_pClassModelPanel = new CTFAdvModelPanel(this, "classmodelpanel");
	m_pGameModelPanel = new CModelPanel(this, "gamemodelpanel");
	m_pWeaponSetPanel = new CTFWeaponSetPanel(this, "weaponsetpanel");
	g_TFWeaponScriptParser.InitParser("scripts/tf_weapon_*.txt", true, false);

	for (int i = 0; i < INVENTORY_VECTOR_NUM; i++){
		m_pWeaponIcons.AddToTail(new CTFButton(m_pWeaponSetPanel, "WeaponIcons", "DUK"));
	}

	return true;
}

void CTFLoadoutPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("resource/UI/main_menu/LoadoutPanel.res");
}

void CTFLoadoutPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	for (int i = 0; i < INVENTORY_VECTOR_NUM; i++){
		m_pWeaponIcons[i]->SetSize( PANEL_WIDE, PANEL_TALL );
		m_pWeaponIcons[i]->SetPaintBorderEnabled( false );
		m_pWeaponIcons[i]->SetImageInset( YRES(20), YRES(-15) );
		m_pWeaponIcons[i]->SetContentAlignment( CTFButton::a_south );
		m_pWeaponIcons[i]->SetTextInset( 0, -10 );
		m_pWeaponIcons[i]->SetImageSize( YRES(100), YRES(100));
		//m_pWeaponIcons[i]->SetImageSize( wide, wide )
		//[i]->SetImageInset( inset / 2, -1 * wide / 5 );
		m_pWeaponIcons[i]->SetBordersByName( "AdvRoundedButtonDefault", "AdvRoundedButtonArmed", "AdvRoundedButtonDepressed" );
	}
};


void CTFLoadoutPanel::SetCurrentClass(int iClass)
{
	if (iCurrentClass == iClass)
		return;

	iCurrentClass = iClass; 	
	DefaultLayout(); 
};


void CTFLoadoutPanel::OnCommand(const char* command)
{
	if (!Q_strcmp(command, "back") || (!Q_strcmp(command, "vguicancel")))
	{
		Hide();
	}
	else if (!Q_strcmp(command, "select_scout"))
	{
		SetCurrentClass(TF_CLASS_SCOUT);
	}
	else if (!Q_strcmp(command, "select_soldier"))
	{
		SetCurrentClass(TF_CLASS_SOLDIER);
	}
	else if (!Q_strcmp(command, "select_pyro"))
	{
		SetCurrentClass(TF_CLASS_PYRO);
	}
	else if (!Q_strcmp(command, "select_demoman"))
	{
		SetCurrentClass(TF_CLASS_DEMOMAN);
	}
	else if (!Q_strcmp(command, "select_heavyweapons"))
	{
		SetCurrentClass(TF_CLASS_HEAVYWEAPONS);
	}
	else if (!Q_strcmp(command, "select_engineer"))
	{
		SetCurrentClass(TF_CLASS_ENGINEER);
	}
	else if (!Q_strcmp(command, "select_medic"))
	{
		SetCurrentClass(TF_CLASS_MEDIC);
	}
	else if (!Q_strcmp(command, "select_sniper"))
	{
		SetCurrentClass(TF_CLASS_SNIPER);
	}
	else if (!Q_strcmp(command, "select_spy"))
	{
		SetCurrentClass(TF_CLASS_SPY);
	}
	else
	{
		const char* szPrimary = GetTFInventory()->GetSlotName(TF_WPN_TYPE_PRIMARY);
		const char* szSecondary = GetTFInventory()->GetSlotName(TF_WPN_TYPE_SECONDARY);
		const char* szMelee = GetTFInventory()->GetSlotName(TF_WPN_TYPE_MELEE);
		char strPrimary[40];
		Q_strncpy(strPrimary, command, Q_strlen(szPrimary) + 1);
		char strSecondary[40];
		Q_strncpy(strSecondary, command, Q_strlen(szSecondary) + 1);
		char strMelee[40];
		Q_strncpy(strMelee, command, Q_strlen(szMelee) + 1);
		char buffer[64];
		bool bValid = false;

		if (!Q_strcmp(strPrimary, szPrimary))
		{
			SetCurrentSlot(TF_WPN_TYPE_PRIMARY);
			Q_snprintf(buffer, sizeof(buffer), command + Q_strlen(szPrimary));
			bValid = true;
		}
		else if (!Q_strcmp(strSecondary, szSecondary))
		{
			SetCurrentSlot(TF_WPN_TYPE_SECONDARY);
			Q_snprintf(buffer, sizeof(buffer), command + Q_strlen(szSecondary));
			bValid = true;
		}
		else if (!Q_strcmp(strMelee, szMelee))
		{
			SetCurrentSlot(TF_WPN_TYPE_MELEE);
			Q_snprintf(buffer, sizeof(buffer), command + Q_strlen(szMelee));
			bValid = true;
		}

		if (bValid)
		{
			SetCurrentPreset(atoi(buffer));
			SetWeaponPreset(iCurrentClass, iCurrentSlot, iCurrentPreset);
		}
		else
		{
			BaseClass::OnCommand(command);
		}
	}
}

void CTFLoadoutPanel::SetModelWeapon(int iClass, int iSlot, int iPreset)
{
	int iWeapon = GetTFInventory()->GetWeapon(iClass, iSlot, iPreset);
	if (iWeapon)
	{
		_WeaponData pData;
		if (iWeapon == TF_WEAPON_BUILDER && iClass == TF_CLASS_SPY)
		{
			pData = g_TFWeaponScriptParser.GetTFWeaponInfo(TF_WEAPON_SAPPER);
		}
		else
		{
			pData = g_TFWeaponScriptParser.GetTFWeaponInfo(WeaponIdToAlias(iWeapon));
		}
		m_pClassModelPanel->SetAnimationIndex(pData.m_iWeaponType);
		m_pClassModelPanel->ClearMergeMDLs();
		if (pData.szWorldModel[0] != '\0')
			m_pClassModelPanel->SetMergeMDL( pData.szWorldModel, NULL, iCurrentSkin );
	}
	else
	{
		m_pClassModelPanel->SetAnimationIndex(iSlot);
		m_pClassModelPanel->ClearMergeMDLs();
	}
	m_pClassModelPanel->Update();
}

void CTFLoadoutPanel::Show()
{
	// Hide all other dialog panels
	for (int i = STATSUMMARY_MENU; i < COUNT_MENU; i++)
	{
		if (GetMenuPanel(i)->IsVisible())
			GetMenuPanel(i)->Hide();
	}

	BaseClass::Show();

	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if (pPlayer)
	{
		int iClass = pPlayer->m_Shared.GetDesiredPlayerClassIndex();
		if (iClass >= TF_CLASS_SCOUT)
			SetCurrentClass( pPlayer->m_Shared.GetDesiredPlayerClassIndex() );
	}
	DefaultLayout();
};

void CTFLoadoutPanel::Hide()
{
	BaseClass::Hide();
};


void CTFLoadoutPanel::OnTick()
{
	BaseClass::OnTick();
};

void CTFLoadoutPanel::OnThink()
{
	BaseClass::OnThink();
};

void CTFLoadoutPanel::SetModelClass(int iClass)
{
	int len = Q_strlen(pszClassModels[iClass]) + 1;
	char *pAlloced = new char[len];
	Assert(pAlloced);
	Q_strncpy(pAlloced, pszClassModels[iClass], len);
	m_pClassModelPanel->m_BMPResData.m_pszModelName = pAlloced;
	m_pClassModelPanel->m_BMPResData.m_nSkin = iCurrentSkin;
}

void CTFLoadoutPanel::UpdateModelPanels()
{
	int iClassIndex = iCurrentClass;

	m_pClassModelPanel->SetVisible(true);
	m_pGameModelPanel->SetVisible(false);
	m_pWeaponSetPanel->SetVisible(true);

	SetModelClass(iClassIndex);
	int iWeaponPreset = GetTFInventory()->GetWeaponPreset(filesystem, iClassIndex, iCurrentSlot);
	SetModelWeapon(iClassIndex, iCurrentSlot, iWeaponPreset);
}

void CTFLoadoutPanel::DefaultLayout()
{
	BaseClass::DefaultLayout();

	C_TFPlayer *pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	if (pLocalPlayer && pLocalPlayer->GetTeamNumber() >= TF_TEAM_RED)
	{
		iCurrentSkin = pLocalPlayer->GetTeamNumber() - 2;
	}
	else
	{
		iCurrentSkin = 0;
	}

	UpdateModelPanels();

	int iClassIndex = iCurrentClass;
	SetDialogVariable( "classname", g_pVGuiLocalize->Find( g_aPlayerClassNames[iClassIndex] ) );
	int iColCount = 0;
	for (int iSlot = 0; iSlot < INVENTORY_ROWNUM; iSlot++)
	{
		int iCols = 0;
		for (int iPreset = 0; iPreset < INVENTORY_COLNUM; iPreset++)
		{
			int iWeapon = GetTFInventory()->GetWeapon( iClassIndex, iSlot, iPreset );
			CTFButton *m_pWeaponButton = m_pWeaponIcons[INVENTORY_COLNUM * iSlot + iPreset];
			if (iWeapon > 0)
			{
				iCols++;
				if (iCols > iColCount) iColCount = iCols;
				m_pWeaponButton->SetVisible( true );
				m_pWeaponButton->SetPos( iPreset * (PANEL_WIDE + PANEL_X_OFFSET), iSlot * (PANEL_TALL + PANEL_Y_OFFSET) );

				int iWeapon = GetTFInventory()->GetWeapon( iCurrentClass, iSlot, iPreset );

				_WeaponData pData;
				if (iWeapon == TF_WEAPON_BUILDER && iCurrentClass == TF_CLASS_SPY)
				{
					pData = g_TFWeaponScriptParser.GetTFWeaponInfo( TF_WEAPON_SAPPER );
				}
				else
				{
					pData = g_TFWeaponScriptParser.GetTFWeaponInfo( WeaponIdToAlias( iWeapon ) );
				}

				char szIcon[64];
				Q_snprintf( szIcon, sizeof( szIcon ), "../%s", (iCurrentSkin == 1 ? pData.iconActive : pData.iconInactive) );
				m_pWeaponButton->SetImage( szIcon );

				char szWeaponName[32];
				Q_snprintf( szWeaponName, sizeof( szWeaponName ), "#%s", pData.szPrintName );
				m_pWeaponButton->SetText( szWeaponName );

				int iWeaponPreset = GetTFInventory()->GetWeaponPreset( filesystem, iClassIndex, iSlot );
				m_pWeaponButton->SetPaintBorderEnabled( (iPreset == iWeaponPreset) );
				m_pWeaponButton->SetSelected( (iPreset == iWeaponPreset) );

				char szCommand[64];
				Q_snprintf( szCommand, sizeof( szCommand ), "%s%i", GetTFInventory()->GetSlotName( iSlot ), iPreset );
				m_pWeaponButton->SetCommand( szCommand );
			}
			else
			{
				m_pWeaponButton->SetVisible( false );
			}
		}
	}
};

void CTFLoadoutPanel::GameLayout()
{
	BaseClass::GameLayout();

};

void CTFLoadoutPanel::SetWeaponPreset(int iClass, int iSlot, int iPreset)
{
	KeyValues* pInventoryKeys = GetTFInventory()->GetInventory(filesystem);
	KeyValues* pClass = pInventoryKeys->FindKey(g_aPlayerClassNames_NonLocalized[iClass], true);
	pClass->SetInt(GetTFInventory()->GetSlotName(iSlot), iPreset);
	GetTFInventory()->SetInventory(filesystem, pInventoryKeys);

	C_TFPlayer *pPlayer = C_TFPlayer::GetLocalTFPlayer();
	if (pPlayer)
	{
		char szCmd[64];
		Q_snprintf(szCmd, sizeof(szCmd), "weaponpreset %d %d", iSlot, iPreset); //; pf2_weaponset_show 0
		engine->ExecuteClientCmd(szCmd);
	}

	DefaultLayout();
}