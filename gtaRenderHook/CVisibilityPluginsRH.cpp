#include "stdafx.h"
#include "CVisibilityPluginsRH.h"
#include <game_sa\CPed.h>
#include <game_sa\common.h>
RwV3d &g_vParachuteOffset = *(RwV3d*)0x8D60B0;
RwV3d &g_vSecondWeaponOffset = *(RwV3d*)0x8D60A4;
RwV3d &g_vSecondWeaponAxis = *(RwV3d*)0x8D6098;

// hope this will be added to pluginsdk soon
RwV3d& CPedIK__YaxisIK = *(RwV3d *)0x8D2338;
std::list<CPed*> CVisibilityPluginsRH::ms_weaponPeds = {};
void CVisibilityPluginsRH::RenderWeaponPedsNoMuzzleFlash()
{
	for (CPed* pedWithWeapon: ms_weaponPeds)
		RenderWeaponsForPed(pedWithWeapon);
}

void CVisibilityPluginsRH::RenderWeaponsForPed(CPed * pedWithWeapon)
{
	if (pedWithWeapon->m_pWeaponObject != nullptr) {
		auto pedWeaponType = pedWithWeapon->m_aWeapons[pedWithWeapon->m_nActiveWeaponSlot].m_nType;
		bool isParachute = pedWeaponType == eWeaponType::WEAPON_PARACHUTE;

		auto boneID = !isParachute ? 24 : 3;
		auto animHierarchy = GetAnimHierarchyFromSkinClump(pedWithWeapon->m_pRwClump);
		auto matrixID = RpHAnimIDGetIndex(animHierarchy, boneID);
		auto matrixArray = RpHAnimHierarchyGetMatrixArray(animHierarchy);
		auto matrix = matrixArray[matrixID];
		auto weaponFrame = (RwFrame*)rwObjectGetParent(pedWithWeapon->m_pWeaponObject);

		memcpy(&weaponFrame->modelling, &matrix, sizeof(RwMatrix));

		if (isParachute) {
			RwMatrixTranslate(&weaponFrame->modelling, &g_vParachuteOffset, rwCOMBINEPRECONCAT);
			RwMatrixRotate(&weaponFrame->modelling, &CPedIK__YaxisIK, 90.0f, rwCOMBINEPRECONCAT);
		}
		DisableGunFlashAlpha(pedWithWeapon);
		//pedWithWeapon->SetGunFlashAlpha(false);
		RwFrameUpdateObjects(weaponFrame);
		RpClumpRender((RpClump *)pedWithWeapon->m_pWeaponObject);

		auto weapInfo = CWeaponInfo::GetWeaponInfo(pedWeaponType, pedWithWeapon->GetWeaponSkill());
		if (weapInfo->m_nFlags.bTwinPistol) {
			matrixID = RpHAnimIDGetIndex(animHierarchy, 34);
			matrix = matrixArray[matrixID];
			memcpy(&weaponFrame->modelling, &matrix, sizeof(RwMatrix));
			RwMatrixRotate(&weaponFrame->modelling, &g_vSecondWeaponOffset, 180.0, rwCOMBINEPRECONCAT);
			RwMatrixTranslate(&weaponFrame->modelling, &g_vSecondWeaponAxis, rwCOMBINEPRECONCAT);
			//pedWithWeapon->SetGunFlashAlpha(true);
			DisableGunFlashAlpha(pedWithWeapon);
			RwFrameUpdateObjects(weaponFrame);
			RpClumpRender((RpClump *)pedWithWeapon->m_pWeaponObject);
		}
		pedWithWeapon->ResetGunFlashAlpha();
	}
}

void CVisibilityPluginsRH::AddPedWithWeapons(CPed * ped)
{
	ms_weaponPeds.push_back(ped);
}

void CVisibilityPluginsRH::ClearWeaponPedsList()
{
	ms_weaponPeds.clear();
}

void CVisibilityPluginsRH::Patch()
{
	RedirectCall(0x5E7859, AddPedWithWeapons);
}

void CVisibilityPluginsRH::DisableGunFlashAlpha(CPed *ped)
{
	if (!ped->m_pGunflashObject)
		return;
	auto gunFlash = GetFirstObject(ped->m_pGunflashObject);
	if (gunFlash)
		CVehicle::SetComponentAtomicAlpha((RpAtomic*)gunFlash, 0);
}