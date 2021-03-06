#include "cbase.h"
#ifdef CLIENT_DLL
ConVar pf_muzzleflash( "pf_muzzleflash", "0", FCVAR_ARCHIVE, "Enable/Disable beta muzzleflash");
ConVar pf_muzzlelight( "pf_muzzlelight", "0", FCVAR_ARCHIVE, "Enable dynamic lights for muzzleflashes and the flamethrower" );
ConVar pf_projectilelight( "pf_projectilelight", "0", FCVAR_ARCHIVE, "Enable dynamic lights for grenades and rockets" );
ConVar pf_muzzlelightsprops( "pf_muzzlelightsprops", "0", FCVAR_ARCHIVE, "Enable dynamic lights affecting props" );
ConVar pf_burningplayerlight( "pf_burningplayerlight", "0", FCVAR_ARCHIVE, "Enable burning players emitting light" );
ConVar pf_showchatbubbles( "pf_showchatbubbles", "1", FCVAR_ARCHIVE, "Show bubble icons over typing players" );
ConVar pf_cheapbulletsplash( "pf_cheapbulletsplash", "0", FCVAR_ARCHIVE | FCVAR_DONTRECORD, "Use the new, less intensive, bullet splash particle" );
ConVar pf_team_colored_spy_cloak( "pf_team_colored_spy_cloak", "0", FCVAR_ARCHIVE | FCVAR_DONTRECORD );
ConVar pf_discord_rpc("pf_discord_rpc", "1", FCVAR_ARCHIVE, "Enable/Disable Discord Rich Presence");
ConVar pf_discord_class("pf_discord_class", "1", FCVAR_ARCHIVE, "Enable/Disable Discord rpc Class icons (REQUIRES GAME RESTART)");
ConVar pf_blood_impact_disable( "pf_blood_impact_disable", "0", FCVAR_ARCHIVE | FCVAR_DONTRECORD, "Disable blood impact effects");
#else
ConVar pf_cp_flag_reset_on_cap("pf_cp_flag_reset_on_cap", "1", FCVAR_GAMEDLL | FCVAR_NOTIFY, "Resets the flag when capturing a control point");
ConVar pf_grenadepack_respawn_time( "pf_grenadepack_respawn_time", "20", FCVAR_NOTIFY, "Respawn time for grenade packs." );
ConVar pf_grenades_infinite("pf_grenades_infinite", "0", FCVAR_NOTIFY, "Player can throw an unlimited amount of grenades");
ConVar pf_concuss_effect_disable("pf_concuss_effect_disable", "0", FCVAR_NOTIFY | FCVAR_SERVER_CAN_EXECUTE, "Disables the camera shake from concussion grenade");
#endif
ConVar pf_aprilfools("pf_aprilfools", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Toggles the April Fools holiday");
ConVar pf_grenades("pf_grenades", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "Toggle the grenades in-game");
ConVar pf_delayed_knife( "pf_delayed_knife", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Toggles the old delayed knife backstab" );
ConVar pf_enable_civilian( "pf_enable_civilian", "0", FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_CHEAT, "Enables joining civilian through join_class" );
ConVar pf_grenade_holstering("pf_grenade_holstering", "1", FCVAR_NOTIFY | FCVAR_REPLICATED);