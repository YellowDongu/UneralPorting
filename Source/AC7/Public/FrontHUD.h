// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Defined.h"
#include "FrontHUD.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UFrontHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void BindVehicle(class AAircraft* target, class APlayerPilot* player);

	inline FString GetSpeedText() { return FString::FromInt(static_cast<int32>(ConvertToVelocity(*Velocity))); }
	inline FString GetAltitudeText() { return FString::FromInt(static_cast<int32>(ConvertToAltitude(GetPosition().Z))); }

	class URadarUI* GetRadarUI() { return RadarUI; }

	void ChangeColor(const FLinearColor& NewColor) { SetUIColor.Broadcast(NewColor); SetTextColor.Broadcast(NewColor); }

	DECLARE_MULTICAST_DELEGATE_OneParam(FSetUIColor, FLinearColor);
	FSetUIColor SetUIColor;
	DECLARE_MULTICAST_DELEGATE_OneParam(FSetTextColor, FSlateColor);
	FSetTextColor SetTextColor;

private:
	bool bBinded{ false };
	float* Velocity{ nullptr };
	FLinearColor HUDColor = FLinearColor::Green;
	FLinearColor BaseColor = FLinearColor::Green;

	TFunction<FVector()> GetPosition{};
	TFunction<FVector()> GetRotation{};

	TArray<class UImage*> AttitudeIndicatorMarks[4];
	TWeakObjectPtr<class URadar> PlayerRadar{ nullptr };
	TSharedPtr<struct FSoundSet> SoundSet{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* SpeedMeterText{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UTextBlock* AltitudeIndicatorText{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UAttitudeIndicator* AttitudeIndicator{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UIFFHandler* IFFHudController{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class ULockHUD* LockHUD{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class URadarUI* RadarUI{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UStatusUI* Status{ nullptr };
};
