#include "Body.h"
#include <Helpers/Macro.h>

void _fastcall PayloadFix(FootClass* pThis)
{
	if (!pThis ||
		pThis->WhatAmI() == AbstractType::Infantry)
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt ||
		pTypeExt->InitialPayload_Types.empty() ||
		pThis->Transporter ||
		pThis->GetTechnoType()->Passengers <= 0 ||
		pThis->Passengers.NumPassengers > 0)
		return;

	for (size_t idx = 0; idx < pTypeExt->InitialPayload_Types.size(); idx++)
	{
		auto pType = pTypeExt->InitialPayload_Types.at(idx);

		if (!pType ||
			pType->WhatAmI() == AbstractType::AircraftType ||
			pType->WhatAmI() == AbstractType::BuildingType)
			continue;

		int pNum = pTypeExt->InitialPayload_Nums.size() > idx ?
			pTypeExt->InitialPayload_Nums.at(idx) : 1;

		if (pNum <= 0)
			continue;

		for (int i = 0; i < pNum; i++)
		{
			TechnoClass* pTechno = abstract_cast<TechnoClass*>(pType->CreateObject(pThis->Owner));
			FootClass* pFoot = abstract_cast<FootClass*>(pTechno);

			pTechno->OnBridge = pThis->OnBridge;
			Unsorted::IKnowWhatImDoing++;
			pTechno->Unlimbo(pThis->GetCoords(), DirType::North);
			Unsorted::IKnowWhatImDoing--;

			pTechno->SetLocation(pThis->GetCoords());
			pTechno->Limbo();
			pTechno->Transporter = pThis;

			const auto old = VocClass::VoicesEnabled ? true : false;
			VocClass::VoicesEnabled = false;
			pThis->AddPassenger(pFoot);
			VocClass::VoicesEnabled = old;

			if (pThis->GetTechnoType()->OpenTopped)
				pThis->EnteredOpenTopped(pTechno);

			if (pThis->GetTechnoType()->Gunner &&
				pThis->Passengers.NumPassengers == 1)
				pThis->ReceiveGunner(pFoot);
		}
	}
}

DEFINE_HOOK(0x65D995, TeamTypeClass_CreateInstance_InitialPayload, 0x6)
{
	GET(FootClass*, pThis, EBP);
	enum { SkipGameCode = 0x65DD1B, Continue = 0x65D9A9 };

	for (FootClass* pFoot = pThis; pFoot; pFoot = abstract_cast<FootClass*>(pFoot->NextObject))
	{
		PayloadFix(pFoot);
	}

	if (pThis && pThis->Team)
		pThis->Team->IsTransient = false;

	R->Stack(STACK_OFFSET(0x30, -0x18), pThis);
	return !pThis ? SkipGameCode : Continue;
}

DEFINE_HOOK(0x65ECD2, TeamTypeClass_CreateTeamChrono_Fix, 0x6)
{
	GET(FootClass*, pThis, EDI);
	enum { SkipGameCode = 0x65F301, Continue = 0x65ECE2 };

	for (FootClass* pFoot = pThis; pFoot; pFoot = abstract_cast<FootClass*>(pFoot->NextObject))
	{
		PayloadFix(pFoot);
	}

	if (pThis && pThis->Team)
		pThis->Team->IsTransient = false;

	return !pThis ? SkipGameCode : Continue;
}
