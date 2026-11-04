// Fill out your copyright notice in the Description page of Project Settings.


#include "FrontHUD.h"

#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Aircraft.h"
#include "PlayerPilot.h"
#include "IFFHandler.h"
#include "LockHUD.h"
#include "AttitudeIndicator.h"
#include "RadarUI.h"
#include "StatusUI.h"
#include "SoundSubsystem.h"

void UFrontHUD::BindVehicle(AAircraft* target, APlayerPilot* player)
{
	if (target == nullptr) return;
	binded = true;
	
	FControl* poineter;
	target->GetControl(poineter);
	Velocity = &poineter->Velocity;
	GetPosition = [target]() -> FVector { return target->GetActorLocation(); };
	GetRotation = [target]() -> FVector { return target->GetActorRotation().Euler(); };
	IFFHudController->BindPlayer(target, player);
	LockHUD->BindPlayer(target, player);
	AttitudeIndicator->BindPlayer(target, player);
	RadarUI->BindPlayer(target, player);
	Status->BindPlayer(target, player);
	MissileWarnSound = GetGameInstance()->GetSubsystem<USoundSubsystem>()->GetSound(TEXT("MissileWarn"));
}

void UFrontHUD::NativeConstruct()
{
	Super::NativeConstruct();

	auto BindFunction = [&](UWidget* Widget)
		{
			if (UImage* Image = Cast<UImage>(Widget))
				SetUIColor.AddUObject(Image, &UImage::SetColorAndOpacity);
			else if (UTextBlock* Text = Cast<UTextBlock>(Widget))
				SetTextColor.AddUObject(Text, &UTextBlock::SetColorAndOpacity);
		};

	WidgetTree->ForEachWidget(BindFunction);
	SetUIColor.AddUObject(AttitudeIndicator, &UAttitudeIndicator::ChangeColor);

	SetUIColor.Broadcast(BaseColor);
	SetTextColor.Broadcast(BaseColor);
}

void UFrontHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!binded)
		return;

	SpeedMeterText->SetText(FText::FromString(GetSpeedText()));
	AltitudeIndicatorText->SetText(FText::FromString(GetAltitudeText()));
	AttitudeIndicator->SetRenderTransformAngle(GetRotation().X);
}
