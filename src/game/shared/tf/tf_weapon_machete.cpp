//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_machete.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
// Server specific.
#else
#include "tf_player.h"
#endif

//=============================================================================
//
// Weapon Machete tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFMachete, DT_TFWeaponMachete )

BEGIN_NETWORK_TABLE( CTFMachete, DT_TFWeaponMachete )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFMachete )
END_PREDICTION_DATA()

//LINK_ENTITY_TO_CLASS( tf_weapon_machete, CTFMachete );
//PRECACHE_WEAPON_REGISTER( tf_weapon_machete );

//=============================================================================
//
// Weapon Machete functions.
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFMachete::CTFMachete()
{
}
