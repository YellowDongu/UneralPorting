// Fill out your copyright notice in the Description page of Project Settings.


#include "RadarTargetUI.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "VehicleBase.h"
//#include "Defined.h"

float URadarTargetUI::Size{ 48.0f };

void URadarTargetUI::Update(const FVector2D& position, float angle, AVehicleBase* target)
{
	CanvasSlot->SetPosition(position);
	SetRenderTransformAngle(target->GetActorRotation().Yaw - angle);

	ChangeImage(target);
}


void URadarTargetUI::Attach(UCanvasPanel* Canvas)
{
	if (GetParent() == Canvas)
		return;

	Mask = -1;
	Air->SetVisibility(ESlateVisibility::Hidden);
	AirTGT->SetVisibility(ESlateVisibility::Hidden);
	GroundTGT->SetVisibility(ESlateVisibility::Hidden);
	Ground->SetVisibility(ESlateVisibility::Hidden);

	Current = Air;

	if (Canvas == nullptr)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	CanvasSlot = Canvas->AddChildToCanvas(this);

	//FAnchors AnchorPresets;
	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
	//CanvasSlot->SetAnchors(AnchorPresets);
	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	//CanvasSlot->SetZOrder(10);
}

void URadarTargetUI::ChangeImage(AVehicleBase* target)
{
	const FWeaponMask& mask = target->GetMask();
	int teamMask = target->GetTeam();

	if (TeamMask == teamMask && Mask == mask.GetMask())
		return;

	Current->SetVisibility(ESlateVisibility::Hidden);
	TeamMask = teamMask;
	Mask = mask.GetMask();

	if (TeamMask == 1)
	{
		Current = mask.IsAir() ? Air : Ground;

		Current->SetVisibility(ESlateVisibility::Visible);
		Current->SetColorAndOpacity(CyanHUD);
	}
	else
	{
		Current = mask.IsAir() ? ((TeamMask != 1 && target->IsTGT()) ? AirTGT : Air) : ((TeamMask != 1 && target->IsTGT()) ? GroundTGT : Ground);

		Current->SetVisibility(ESlateVisibility::Visible);
		Current->SetColorAndOpacity(target->IsTGT() ? RedHUD : GreenHUD);
	}
}


