//=============================================================================
//
// Purpose: Discord Presence support.
//
//=============================================================================



#include "cbase.h"
#include "pf_discord_rpc.h"
#include "c_team_objectiveresource.h"
#include "tf_gamerules.h"
#include "c_tf_team.h"
#include "c_tf_playerresource.h"
#include <inetchannelinfo.h>
#include "discord_rpc.h"
#include "discord_register.h"
#include "tf_gamerules.h"
#include "pf_cvars.h"
#include <ctime>
#include "tier0/icommandline.h"
#include "ilocalize.h"
#include <stdlib.h>

//Yup i used a string deal with it
//-Nbc66
#include <string>
#include <algorithm>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#ifdef WIN32

ConVar cl_richpresence_printmsg("cl_richpresence_printmsg", "0", FCVAR_ARCHIVE, "");

#define DISCORD_APP_ID	"658004706337226791"

// update once every 10 seconds. discord has an internal rate limiter of 15 seconds as well
#define DISCORD_UPDATE_RATE 10.0f

//not needed anymore
//-Nbc66
//#define MAP_COUNT 10



const char* g_aClassImageRED[] =
{
	"undefined",
	"red_scout",
	"red_sniper",
	"red_soldier",
	"red_demoman",
	"red_medic",
	"red_heavy",
	"red_pyro",
	"red_spy",
	"red_engineer"
};

const char* g_aClassImageBLU[] =
{
	"undefined",
	"blu_scout",
	"blu_sniper",
	"blu_soldier",
	"blu_demoman",
	"blu_medic",
	"blu_heavy",
	"blu_pyro",
	"blu_spy",
	"blu_engineer"
};

const char* g_aClassNameLocalized[] =
{
	"#TF_Class_Name_Undefined",
	"#TF_Class_Name_Scout",
	"#TF_Class_Name_Sniper",
	"#TF_Class_Name_Soldier",
	"#TF_Class_Name_Demoman",
	"#TF_Class_Name_Medic",
	"#TF_Class_Name_HWGuy",
	"#TF_Class_Name_Pyro",
	"#TF_Class_Name_Spy",
	"#TF_Class_Name_Engineer"
};


//Replaced by discord_rpc.txt
//-Nbc66
/*
const char* g_aMapList[] =
{
	"tc_hydro",
	"ad_dustbowl",
	"ad_gravelpit",
	"cp_granary",
	"cp_well",
	"ctf_2fort",
	"ctf_well",
	"cp_powerhouse",
	"ad_dustbowl2",
	"ctf_badlands"
};
*/

extern const char* GetMapDisplayName(const char* mapName);

CTFDiscordRPC g_discordrpc;

CTFDiscordRPC::CTFDiscordRPC()
{
	Q_memset(m_szLatchedMapname, 0, MAX_MAP_NAME);
	m_bInitializeRequested = false;
}

CTFDiscordRPC::~CTFDiscordRPC()
{
}

void CTFDiscordRPC::Shutdown()
{
	Discord_Shutdown();
}

void CTFDiscordRPC::Init()
{
	if (pf_discord_rpc.GetBool()==0)
	{
		Shutdown();
	}
	InitializeDiscord();
	m_bInitializeRequested = true;

	// make sure to call this after game system initialized
	ListenForGameEvent("server_spawn");
}

void CTFDiscordRPC::RunFrame()
{
	if (pf_discord_rpc.GetBool()==0)
	{
		Shutdown();
	}

	if (m_bErrored)
		return;

	// NOTE: we want to run this even if they have use_discord off, so we can clear
	// any previous state that may have already been sent
	UpdateRichPresence();
	Discord_RunCallbacks();

	// always run this, otherwise we will chicken & egg waiting for ready
	//if (Discord_RunCallbacks)
	//	Discord_RunCallbacks();
}

void CTFDiscordRPC::OnReady(const DiscordUser* user)
{

	//Obtain coordinated universal time:
	//-Nbc66
	std::string str;
	time_t t = time(NULL);
    
	str = asctime(gmtime(&t));

	//Fuck ctime for adding a \n at the fucking end you fucking piece of shit
	//-Nbc66
	str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Ready!\n");
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] User %s#%s - %s\n", user->username, user->discriminator, user->userId);

	//Yup this is literaly usless but hey it's a novelty lol
	//-Nbc66
	ConColorMsg(Color( 0, 255, 30, 255), "This build was compiled on %s UTC\n", str);

	g_discordrpc.Reset();
}

void CTFDiscordRPC::OnDiscordError(int errorCode, const char* szMessage)
{
	g_discordrpc.m_bErrored = true;
	char buff[1024];
	Q_snprintf(buff, 1024, "[Rich Presence] Init failed. code %d - error: %s\n", errorCode, szMessage);
	Warning(buff);
}


void CTFDiscordRPC::OnJoinGame(const char* joinSecret)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Game: %s\n", joinSecret);

	char szCommand[128];
	Q_snprintf(szCommand, sizeof(szCommand), "connect %s\n", joinSecret);
	engine->ExecuteClientCmd(szCommand);
}

//Gota figgure out how to get the source tv ip adress of a server for spectating to work

/*void CTFDiscordRPC::OnSpectateGame(const char* spectateSecret)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Spectate Game: %s\n", spectateSecret);
}*/

void CTFDiscordRPC::OnJoinRequest(const DiscordUser* joinRequest)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Request: %s#%s\n", joinRequest->username, joinRequest->discriminator);
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Request Accepted\n");
	Discord_Respond(joinRequest->userId, DISCORD_REPLY_YES);
}

void CTFDiscordRPC::SetLogo(void)
{

	const char* pszGameType = "BETA!";
	const char* pszImageLarge = "pf2_icon";
	const char* pMapIcon = "missing";

	//string for setting the picture of the class
	//you should name the small picture affter the class itself ex: Scout.jpg, Soldier.jpg, Pyro.jpg ...
	//you get it
	//-Nbc66
	const char* pszImageSmall = "";
	const char* pszImageText = "";

	C_TFPlayer* pTFPlayer = ToTFPlayer(C_BasePlayer::GetLocalPlayer());


	if (engine->IsConnected())
	{

		if (pszImageLarge != m_szLatchedMapname)
		{
			//stolen from KaydemonLP
			//-Nbc66
			pMapIcon = GetRPCMapImage(m_szLatchedMapname, pMapIcon);


			//old function that uses hard coded maps inside this cpp file
			//-Nbc66
			/*
					if (pszImageLarge != m_szLatchedMapname)
					{
						for (int i = 0; i < MAP_COUNT; i++)
						{
							if (V_strcmp(g_aMapList[i], m_szLatchedMapname) == 0)
							{
								pMapIcon = m_szLatchedMapname;
								break;
							}
						}
					}
			*/

			

			pszImageLarge = pMapIcon;
		}
	}

	if (pf_discord_class.GetBool() == 1)
	{
		//checks the players class
		if (pTFPlayer)
		{
			int iClass = pTFPlayer->GetPlayerClass()->GetClassIndex();

			if (pTFPlayer->GetTeamNumber() == TF_TEAM_RED)
			{

				pszImageSmall = g_aClassImageRED[iClass];
				pszImageText  = LocalizeDiscordString(g_aClassNameLocalized[iClass]);

			}
			else if (pTFPlayer->GetTeamNumber() == TF_TEAM_BLUE)
			{

				pszImageSmall = g_aClassImageBLU[iClass];
				pszImageText = LocalizeDiscordString(g_aClassNameLocalized[iClass]);

			}
			else
			{
				pszImageSmall = "spectator";
				pszImageText = "Spectating";
			}
		}
	}

	
	
	//-Nbc66
	if (TFGameRules())
    {
		if (TFGameRules()->GetGameType() == TF_GAMETYPE_UNDEFINED)
		{
			pszGameType = "";
		}
		else if (TFGameRules()->GetGameType() == TF_GAMETYPE_CTF)
		{
			pszGameType = LocalizeDiscordString("#TF_GAMETYPE_CTF");
		}
		else if (V_strnicmp(m_szLatchedMapname, "tc_", 3) == 0)
		{
			pszGameType = LocalizeDiscordString("#TF_GAMETYPE_TC");
		}
		else if (V_strnicmp(m_szLatchedMapname, "ad_", 3) == 0)
		{
			pszGameType = LocalizeDiscordString("#TF_AttackDefend");
		}
		else if (TFGameRules()->GetGameType() == TF_GAMETYPE_CP)
		{
			pszGameType = LocalizeDiscordString("#TF_GAMETYPE_CP");
		}
	}

	if (engine->IsConnected() == false)
	{
		pszGameType = "BETA!";
	}

	m_sDiscordRichPresence.largeImageKey = pszImageLarge;
	m_sDiscordRichPresence.largeImageText = pszGameType;
	m_sDiscordRichPresence.smallImageKey = pszImageSmall;
	m_sDiscordRichPresence.smallImageText = pszImageText;
}

//Fucke me this function sucks but if you got a better solution im all ears
//Techno you better make this look like a actualy good working function which can get
//unicode charachters to be converted to a char now that would be fucking epic
//but also imposible, for the time being we are stuck with this sorry
//-Nbc66
const char* CTFDiscordRPC::LocalizeDiscordString(const char *LocalizedString)
{

	const wchar_t* WcharLocalizedString = g_pVGuiLocalize->Find(LocalizedString);
	//char array is set this way to account for ASCII's
	//characters which are generaly 256 charachetrs with Windows-1252 8-bit charachter encoding
	//just dont fuck with the array size or you are going to have a bad time man
	//-Nbc66
	char CharLocalizedArray[256];
	g_pVGuiLocalize->ConvertUnicodeToANSI(WcharLocalizedString, CharLocalizedArray, sizeof(CharLocalizedArray));
	const char* FinalCharLocalizedString = V_strdup(CharLocalizedArray);

	return FinalCharLocalizedString;

	delete[] FinalCharLocalizedString;
}

void CTFDiscordRPC::InitializeDiscord()
{
	DiscordEventHandlers handlers;
	Q_memset(&handlers, 0, sizeof(handlers));
	handlers.ready = &CTFDiscordRPC::OnReady;
	handlers.errored = &CTFDiscordRPC::OnDiscordError;
	handlers.joinGame = &CTFDiscordRPC::OnJoinGame;
	//handlers.spectateGame = &CTFDiscordRPC::OnSpectateGame;
	handlers.joinRequest = &CTFDiscordRPC::OnJoinRequest;

	char command[512];
	V_snprintf(command, sizeof(command), "%s -game \"%s\" -novid -steam\n", CommandLine()->GetParm(0), CommandLine()->ParmValue("-game"));
	Discord_Register(DISCORD_APP_ID, command);
	Discord_Initialize(DISCORD_APP_ID, &handlers, false, "");
	Reset();
}

bool CTFDiscordRPC::NeedToUpdate()
{
	if (m_bErrored || m_szLatchedMapname[0] == '\0')
		return false;

	return gpGlobals->realtime >= m_flLastUpdatedTime + DISCORD_UPDATE_RATE;
}

void CTFDiscordRPC::Reset()
{

	if (pf_discord_rpc.GetBool()==0)
	{
		Shutdown();
	}

	Q_memset(&m_sDiscordRichPresence, 0, sizeof(m_sDiscordRichPresence));
	m_sDiscordRichPresence.details = LocalizeDiscordString("#Discord_State_Menu");
	const char* pszState = "";

	m_sDiscordRichPresence.state = pszState;

	m_sDiscordRichPresence.endTimestamp;

	SetLogo();
	Discord_UpdatePresence(&m_sDiscordRichPresence);
}

void CTFDiscordRPC::UpdatePlayerInfo()
{
	C_TF_PlayerResource* pResource = GetTFPlayerResource();
	if (!pResource)
		return;

	int maxPlayers = gpGlobals->maxClients;
	int curPlayers = 0;

	const char* pzePlayerName = NULL;

	for (int i = 1; i < maxPlayers; i++)
	{
		if (pResource->IsConnected(i))
		{
			curPlayers++;
			if (pResource->IsLocalPlayer(i))
			{
				pzePlayerName = pResource->GetPlayerName(i);
			}
		}
	}

	//int iTimeLeft = TFGameRules()->GetTimeLeft();

	if (m_szLatchedHostname[0] != '\0')
	{
		if (cl_richpresence_printmsg.GetBool())
		{
			ConColorMsg(Color(114, 137, 218, 255), "[Discord] sending details of\n '%s'\n", m_szServerInfo);
		}
		m_sDiscordRichPresence.partySize = curPlayers;
		m_sDiscordRichPresence.partyMax = maxPlayers;
		m_sDiscordRichPresence.state = m_szLatchedHostname;
		//m_sDiscordRichPresence.state = szStateBuffer;
	}
}

void CTFDiscordRPC::FireGameEvent(IGameEvent* event)
{
	const char* type = event->GetName();

	if (Q_strcmp(type, "server_spawn") == 0)
	{
		Q_strncpy(m_szLatchedHostname, event->GetString("hostname"), 255);
	}
}

void CTFDiscordRPC::UpdateRichPresence()
{
	//The elapsed timer function using <ctime>
	//this is for setting up the time when the player joins a server
	//-Nbc66
	time_t iSysTime;
	time(&iSysTime);
	struct tm* tStartTime = NULL;
	tStartTime = localtime(&iSysTime);
	tStartTime->tm_sec += 0 - gpGlobals->curtime;

	if (!NeedToUpdate())
		return;

	m_flLastUpdatedTime = gpGlobals->realtime;

	if (engine->IsConnected())
	{
		V_snprintf(szStateBuffer, sizeof(szStateBuffer), "%s : %s", LocalizeDiscordString("#Discord_Map"), m_szLatchedMapname);
		UpdatePlayerInfo();
		UpdateNetworkInfo();
		//starts the elapsed timer for discord rpc
		//-Nbc66
		m_sDiscordRichPresence.startTimestamp = mktime(tStartTime);
		//sets the map name
		m_sDiscordRichPresence.details = szStateBuffer;
	}

	//checks if the loading bar is being drawn
	//and sets the discord status to "Currently is loading..."
	//-Nbc66
	if (engine->IsDrawingLoadingImage() == true)
	{
		m_sDiscordRichPresence.state = "";
		m_sDiscordRichPresence.details = "Currently loading...";
	}

	SetLogo();

	Discord_UpdatePresence(&m_sDiscordRichPresence);
}


void CTFDiscordRPC::UpdateNetworkInfo()
{
	INetChannelInfo* ni = engine->GetNetChannelInfo();

	char partyId[128];
	sprintf(partyId, "%s-party", ni->GetAddress()); // adding -party here because secrets cannot match the party id

	m_sDiscordRichPresence.partyId = partyId;

	m_sDiscordRichPresence.joinSecret = ni->GetAddress();
	//dosent work untill i can figgure out how to get the source tv ip
	//m_sDiscordRichPresence.spectateSecret = "Spectate";
}

void CTFDiscordRPC::LevelInit(const char* szMapname)
{
	Reset();
	// we cant update our presence here, because if its the first map a client loaded,
	// discord api may not yet be loaded, so latch
	Q_strcpy(m_szLatchedMapname, szMapname);
	//V_snprintf(szStateBuffer, sizeof(szStateBuffer), "MAP: %s", m_szLatchedMapname);
	// important, clear last update time as well
	m_flLastUpdatedTime = max(0, gpGlobals->realtime - DISCORD_UPDATE_RATE);
}
#endif