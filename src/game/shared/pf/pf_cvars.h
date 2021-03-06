#ifndef PF_CVARS_H
#define PF_CVARS_H

#ifdef CLIENT_DLL
extern ConVar pf_muzzleflash;
extern ConVar pf_muzzlelight;
extern ConVar pf_projectilelight;
extern ConVar pf_muzzlelightsprops;
extern ConVar pf_burningplayerlight;
extern ConVar pf_showchatbubbles;
extern ConVar pf_cheapbulletsplash;
extern ConVar pf_team_colored_spy_cloak;
extern ConVar pf_discord_rpc;
extern ConVar pf_discord_class;
extern ConVar pf_blood_impact_disable;
#else
extern ConVar pf_cp_flag_reset_on_cap;
extern ConVar pf_grenadepack_respawn_time;
extern ConVar pf_grenades_infinite;
extern ConVar pf_concuss_effect_disable;
#endif
extern ConVar pf_aprilfools;
extern ConVar pf_grenades;
extern ConVar pf_delayed_knife;
extern ConVar pf_enable_civilian;
extern ConVar pf_armor_enabled;
extern ConVar pf_grenade_holstering;
#endif