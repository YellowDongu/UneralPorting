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
	virtual void NativePreConstruct(void) override;
	virtual void NativeConstruct(void) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;

	void BindPlayer(class AAircraft* vehicle, class APlayerPilot* player);
	void InitializeHUD(void);

	void ChangeColor(FLinearColor color);

	DECLARE_DELEGATE_RetVal(FRotator, FGetEularAngle);
	FGetEularAngle GetEularAngle;

private:
	TObjectPtr<class UPitchBar> AddPitchBar(int pitchLevel, bool back = true);
	TObjectPtr<class UPitchBar> Get(PitchFlag flag);
	void Release(const TObjectPtr<class UPitchBar>& ui, PitchFlag flag);
	FORCEINLINE int FlagConvert(PitchFlag flag);


	bool bBinded{ false }, bInitialized{ false };
	float MaxHeight{ 200.0f }, PitchSpace{ 2.5f }, Space{ 0.0f };
	int HalfHeight{ 4 }, Pitch{ 0 };

	TArray<TPair<TObjectPtr<class UPitchBar>, PitchFlag>> Alive;
	TArray<TObjectPtr<class UPitchBar>> Pool[4];

	FLinearColor CurrentColor{ FLinearColor::White };

	//UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	//class UTextBlock* Text{nullptr};
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UCanvasPanel* BasePanel{ nullptr };
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
