// Fill out your copyright notice in the Description page of Project Settings.


#include "IFFHUD.h"
#include "Aircraft.h"
#include "FireControlSystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "RadarTargetUI.h"

UIFFHUD::UIFFHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Super::NativeConstruct();
	//UpdateCanTick();
}

void UIFFHUD::StartTrack(AVehicleBase* vehicle, AAircraft* player, UCanvasPanelSlot* AttachedCanvasSlot)
{
	Player = player;
	TargetVehicle = vehicle;
	CanvasSlot = AttachedCanvasSlot;

	bIsSelected = true;
	DeselectTarget();
	Layer = -1;

	bool ally = player->GetTeam() == vehicle->GetTeam();

	Air->SetVisibility(ESlateVisibility::Hidden);
	Air_NG->SetVisibility(ESlateVisibility::Hidden);

	Ground->SetVisibility(ESlateVisibility::Hidden);
	Ground_NG->SetVisibility(ESlateVisibility::Hidden);
	const FWeaponMask& layer = vehicle->GetMask();
	if (layer.IsLand())
	{
		Box = Ground;
		Box_NG = Ground_NG;
	}
	else
	{
		Box = Air;
		Box_NG = Air_NG;
	}

	TypeNameText->SetText(FText::FromString(TargetVehicle->GetVehicleName()));
	Active(true);
	TGTText->SetVisibility(vehicle->IsTGT() ? (ally ? ESlateVisibility::Hidden : ESlateVisibility::Visible) : ESlateVisibility::Hidden);

	DistanceText->SetVisibility(ESlateVisibility::Hidden);
	TypeNameText->SetVisibility(ally ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	SetNegative(Player->GetFireControlSystem()->GetMissileAimLayer());
	SetColor(ally ? FLinearColor(0.0f, 1.0f, 1.0f, 1.0f) : FLinearColor::Green);

	bIsVisible = true;
	//SelectTarget();
	UE_LOG(LogTemp, Error, TEXT("[UIFFHUD::StartTrack] Track %s, %d/%d, %d"), *vehicle->GetName(), vehicle->IsAir(), vehicle->IsLand(), vehicle->GetMask().GetMask());
}

void UIFFHUD::Update(float DeltaTime)
{
	if (!TargetVehicle.IsValid())
	{
		TargetVehicle = nullptr;
		Active(false);
		RetriveUI.Broadcast(this);
		return;
	}

	FVector2D screenPos;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), TargetVehicle->GetActorLocation(), screenPos, true))
	{
		Active(true);
		CanvasSlot->SetPosition(screenPos);
		const FWeaponMask& layer = Player->GetFireControlSystem()->GetMissileAimLayer();
		if (Layer != layer.GetMask())
			SetNegative(layer);

		if (bIsSelected)
		{
			float distance = ConvertToDistance((TargetVehicle->GetActorLocation() - Player->GetActorLocation()).Length());
			DistanceText->SetText(FText::FromString(FString::FromInt(static_cast<int>(distance))));
		}
	}
	else
		Active(false);
}

void UIFFHUD::SelectTarget()
{
	if (bIsSelected)
		return;

	bIsSelected = true;
	DistanceText->SetVisibility(ESlateVisibility::Visible);
	TypeNameText->SetVisibility(ESlateVisibility::Visible);
	SetColor();
}

void UIFFHUD::DeselectTarget()
{
	if (!bIsSelected)
		return;

	bIsSelected = false;
	DistanceText->SetVisibility(ESlateVisibility::Hidden);
	TypeNameText->SetVisibility(ESlateVisibility::Hidden);
}

void UIFFHUD::Active(bool active)
{
	if (bIsVisible == active)
		return;

	bIsVisible = active;
	BaseCanvas->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	if (bIsVisible)
		SetColor();
}

void UIFFHUD::SetColor()
{
	Box->SetColorAndOpacity(Color);
	Box_NG->SetColorAndOpacity(Color);

	TGTText->SetColorAndOpacity(FLinearColor::Red);
	DistanceText->SetColorAndOpacity(Color);
	TypeNameText->SetColorAndOpacity(Color);
}

void UIFFHUD::SetNegative(const FWeaponMask& mask)
{
	Layer = mask.GetMask();

	if (mask.MaskMatch(TargetVehicle->GetMask()))
	{
		Box->SetVisibility(ESlateVisibility::Hidden);
		Box_NG->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Box->SetVisibility(ESlateVisibility::Visible);
		Box_NG->SetVisibility(ESlateVisibility::Hidden);
	}
	SetColor();
}
