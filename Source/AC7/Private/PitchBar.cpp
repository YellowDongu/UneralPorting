// Fill out your copyright notice in the Description page of Project Settings.


#include "PitchBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UPitchBar::SetNumber(int number)
{
	if (Number == nullptr)
		return;
	Number->SetText(FText::FromString(FString::FromInt(number)));
}

UPitchBar* UPitchBar::SetUp(UCanvasPanel* AttachedCanvasSlot)
{
	if (AttachedCanvasSlot == nullptr)
		return nullptr;

	CanvasSlot = AttachedCanvasSlot->AddChildToCanvas(this);
	if (CanvasSlot == nullptr)
		return nullptr;

	FAnchors AnchorPresets;
	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
	//CanvasSlot->SetAnchors(AnchorPresets);
	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	//CanvasSlot->SetZOrder(10);
	return this;
}


UPitchBar* UPitchBar::SetUp()
{
	CanvasSlot = Cast<UCanvasPanelSlot>(this->Slot);
	if (CanvasSlot == nullptr)
		return nullptr;

	FAnchors AnchorPresets;
	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
	//CanvasSlot->SetAnchors(AnchorPresets);
	CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	//CanvasSlot->SetZOrder(10);
	return this;
}

void UPitchBar::SetPosition(const FVector2D& value)
{
	CanvasSlot->SetPosition(value);
}

void UPitchBar::SetUIColor(FLinearColor value)
{
	if (Image == nullptr)
		return;
	Image->SetColorAndOpacity(value);
}

void UPitchBar::SetSideTextColor(FSlateColor value)
{
	if (Number == nullptr)
		return;
	Number->SetColorAndOpacity(value);
}