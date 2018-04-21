#pragma once
class CPed;

class CVisibilityPluginsRH
{
public:
	/*!
		Renders weapons without muzzle flash.
	*/
	static void RenderWeaponPedsNoMuzzleFlash();
	/*!
		Adds ped with weapon to list.
	*/
	static void AddPedWithWeapons(CPed*);
	/*!
		Clears ped weapon list.
	*/
	static void ClearWeaponPedsList();
	/*!
		Injects methods in memory.
	*/
	static void Patch();
private:
	static std::list<CPed*> ms_weaponPeds;
};

