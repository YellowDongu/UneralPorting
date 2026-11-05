// Fill out your copyright notice in the Description page of Project Settings.


#include "FrontHUD.h"

#include "Aircraft.h"
#include "PlayerPilot.h"
#include "Radar.h"
#include "LockHUD.h"
#include "IFFHandler.h"
#include "AttitudeIndicator.h"
#include "RadarUI.h"
#include "StatusUI.h"
#include "SoundSubsystem.h"

#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/AudioComponent.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"

void UFrontHUD::BindVehicle(AAircraft* target, APlayerPilot* player)
{
	if (target == nullptr) return;
	bBinded = true;
	
	FControl* poineter;
	target->GetControl(poineter);
	Velocity = &poineter->Velocity;
	GetPosition = [target]() -> FVector { return target->GetActorLocation(); };
	GetRotation = [target]() -> FVector { return target->GetActorRotation().Euler(); };
	PlayerRadar = target->GetRadar();

	IFFHudController->BindPlayer(target, player);
	LockHUD->BindPlayer(target, player);
	AttitudeIndicator->BindPlayer(target, player);
	RadarUI->BindPlayer(target, player);
	Status->BindPlayer(target, player);

	USoundSubsystem* sound = GetGameInstance()->GetSubsystem<USoundSubsystem>();
	SoundSet = MakeShared<FSoundSet>();
	SoundSet->Sound = sound->GetSound(TEXT("MissileWarn"));
	SoundSet->Channel = sound->GetChannel(TEXT("MissileWarnChannel"), target->GetWorld());
	SoundSet->Channel->SetSound(SoundSet->Sound);
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
	if (!bBinded)
		return;

	SpeedMeterText->SetText(FText::FromString(GetSpeedText()));
	AltitudeIndicatorText->SetText(FText::FromString(GetAltitudeText()));
	AttitudeIndicator->SetRenderTransformAngle(GetRotation().X);

	if (PlayerRadar->Warning())
	{
		float duration = SoundSet->IsPlaying(false) ? SoundSet->Sound->Duration : (PlayerRadar->Alart() ? 0.5f : 0.2f);
		SoundSet->BlinkSound(duration, InDeltaTime);
	}
	else
		SoundSet->Stop();
}
