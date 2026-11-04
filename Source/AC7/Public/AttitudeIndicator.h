// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttitudeIndicator.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UAttitudeIndicator : public UUserWidget
{
	GENERATED_BODY()
private:
	enum class PitchFlag
	{
		PitchCenter,
		PitchNeg,
		PitchNegHalf,
		PitchPos,
		PitchPosHalf
	};

public:
	//UAttitudeIndicator();
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	void BindPlayer(class AAircraft* vehicle, class APlayerPilot* player);

	void NativeTick(const FGeometry& MyGeometry, float DeltaTime);

	TObjectPtr<class UPitchBar> AddPitchBar(int pitchLevel, bool back = true);
	TObjectPtr<class UPitchBar> Get(PitchFlag flag);
	void Release(const TObjectPtr<class UPitchBar>& ui, PitchFlag flag);

	FORCEINLINE int FlagConvert(PitchFlag flag);

	DECLARE_DELEGATE_RetVal(FRotator, FGetEularAngle);
	FGetEularAngle GetEularAngle;


	void ChangeColor(FLinearColor color = FLinearColor::Green);

private:
	bool bBinded{ false };
	bool bInitialized{ false };
	float MaxHeight{ 200.0f };
	float PitchSpace{ 2.5f };
	int HalfHeight{ 4 };

	int Pitch{ 0 };
	float space{ 0.0f };

	TArray<TPair<TObjectPtr<class UPitchBar>, PitchFlag>> Alive;
	TArray<TObjectPtr<class UPitchBar>> Pool[4];

	FLinearColor currentColor{ FLinearColor::Green };

	//UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	//class UTextBlock* Text{nullptr};
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* BasePanel;
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UPitchBar* PitchCenter{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = BluePrints, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPitchBar> PitchNeg{ nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = BluePrints, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPitchBar> PitchNegHalf{ nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = BluePrints, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPitchBar> PitchPos{ nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = BluePrints, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UPitchBar> PitchPosHalf{ nullptr };
};
