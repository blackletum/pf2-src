//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_MACHETE_H
#define TF_WEAPON_MACHETE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"

#ifdef CLIENT_DLL
#define CTFMachete C_TFMachete
#endif

//=============================================================================
//
// Machete class.
//
class CTFMachete : public CTFWeaponBaseMelee
{
public:

	DECLARE_CLASS( CTFMachete, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFMachete();
	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_MACHETE; }

private:

	CTFMachete( const CTFMachete & ) {}
};

#endif // TF_WEAPON_MACHETE_H
