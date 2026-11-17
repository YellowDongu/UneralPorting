#include "StatusUI.h"
#include "GaugeUI.h"
#include "Aircraft.h"
#include "PlayerPilot.h"
#include "FireControlSystem.h"
#include "Components/CanvasPanel.h"

void UStatusUI::BindPlayer(class AAircraft* vehicle, class APlayerPilot* player)
{
	Player = vehicle;

	//int SpecialWeapon = vehicle->GetFireControlSystem()->GetSpecial();
	//EVehicleID vehicleID = vehicle->GetVehicleID();
	//EWeaponID specialWeaponID = vehicle->GetFireControlSystem()->GetWeaponID(true);

	Gauge->BindPlayer(vehicle, player);

	//for (auto& item : HPCanvas->GetAllChildren())
	//{
	//	item->SetVisibility(ESlateVisibility::Hidden);
	//	UGaugeUI* gauge = Cast<UGaugeUI>(item);
	//	if (gauge == nullptr)
	//		continue;
	//
	//	if (gauge->GetVehicleID() != vehicleID)
	//		continue;
	//
	//	HPGauge = gauge;
	//	HPGauge->SetVisibility(ESlateVisibility::Visible);
	//}
	//
	//if (HPGauge == nullptr)
	//{
	//	RemoveFromParent();
	//	return;
	//}
	//
	//if (specialWeaponID == EWeaponID::NULLID || specialWeaponID == EWeaponID::END)
	//	return;
	//
	//for (auto& item : WeaponCanvas->GetAllChildren())
	//{
	//	item->SetVisibility(ESlateVisibility::Hidden);
	//	UGaugeUI* gauge = Cast<UGaugeUI>(item);
	//	if (gauge == nullptr)
	//		continue;
	//
	//	if (gauge->GetMissileID() != specialWeaponID)
	//		continue;
	//
	//	SpecialGauge = gauge;
	//	SpecialGauge->SetVisibility(ESlateVisibility::Visible);
	//}
	//
	//if (SpecialGauge == nullptr)
	//{
	//	RemoveFromParent();
	//	return;
	//}
	//
	//HPCanvas->SetVisibility(ESlateVisibility::Visible);
	//WeaponCanvas->SetVisibility(ESlateVisibility::Visible);
}

void UStatusUI::SetColor(FLinearColor NewColor)
{
	Gauge->SetColor(NewColor);

	//class UCanvasPanel* BaseCanvas{ nullptr };
	//class UCanvasPanel* TextPanel{ nullptr };
	//class UCanvasPanel* GaugePanel{ nullptr };
}