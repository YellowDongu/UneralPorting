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
	
	Velocity = &target->GetControl().Velocity;
	GetPosition = [target]() -> FVector { return target->GetActorLocation(); };
	GetRotation = [target]() -> FVector { return target->GetActorRotation().Euler(); };
	PlayerRadar = target->GetRadar();

	AttitudeIndicator->BindPlayer(target, player);
	IFFHudController->BindPlayer(target, player);
	LockHUD->BindPlayer(target, player);
	RadarUI->BindPlayer(target, player);
	Status->BindPlayer(target, player);

	SoundSet = MakeShared<FSoundSet>();
	SoundSet->Initialize(TEXT("MissileWarn"), TEXT("MissileWarnChannel"), GetGameInstance()->GetSubsystem<USoundSubsystem>(), target->GetWorld());

	ShortDuration = SoundSet->Sound->Duration * 3.0f;
	LongDuration = SoundSet->Sound->Duration * 1.1f;

	SetUIColor.AddUObject(AttitudeIndicator, &UAttitudeIndicator::ChangeColor);
	SetUIColor.AddUObject(IFFHudController, &UIFFHandler::SetColor);
	//SetUIColor.AddUObject(LockHUD, &ULockHUD::SetColor);
	//SetUIColor.AddUObject(RadarUI, &URadarUI::SetColor);
	SetUIColor.AddUObject(Status, &UStatusUI::SetColor);

	//class UIFFHandler* IFFHudController{ nullptr };
	//class ULockHUD* LockHUD{ nullptr };
	//class URadarUI* RadarUI{ nullptr };
	//class UStatusUI* Status{ nullptr };

	ChangeColor(BaseColor);
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
}

void UFrontHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bBinded)
		return;

	SpeedMeterText->SetText(FText::FromString(GetSpeedText()));
	AltitudeIndicatorText->SetText(FText::FromString(GetAltitudeText()));
	AttitudeIndicator->SetRenderTransformAngle(GetRotation().X);

	if (bWarning)
		SoundSet->BlinkSound(PlayerRadar->Alart() ? LongDuration : ShortDuration, InDeltaTime);
	else
		SoundSet->Stop();

	if (bWarning == PlayerRadar->Warning())
		return;

	bWarning = PlayerRadar->Warning();
	ChangeColor(bWarning ? AlartColor : BaseColor);
}
