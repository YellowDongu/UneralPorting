// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"

//#include "Misc/TVariant.h"
#include "Blueprint/UserWidget.h"
#include "LockInfomation.h"

#include "LockHUD.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API ULockHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	ULockHUD(const FObjectInitializer& ObjectInitializer);

	void BindPlayer(class AAircraft* vehicle, class APlayerPilot* player);
	void SetSound(UWorld* world);
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
	virtual void NativeConstruct(void) override;

	void ChangeLockUI(void);
	void PlayLockSound(float DeltaTime, float maxTime);

	const TArray<TSharedPtr<LockSet>>& GetSpecialIndicator() const { return SpecialIndicators; }
private:
	TWeakObjectPtr<class AAircraft> Player{ nullptr };
	TWeakObjectPtr<class URadar> PlayerRadar{ nullptr };

	const bool* bSelectSpecial{ nullptr };
	bool bSelect{ false }, bSoundActive{ false };
	float SoundTimer{ 10.0f };

	TArray<FTargetInfo>* currentTargets{ nullptr };
	TMap<TWeakObjectPtr<class AVehicleBase>, TObjectPtr<UUserWidget>> Alive;

	TSharedPtr<LockIndicator> StandardIndicator;
	TArray<TSharedPtr<LockSet>> SpecialIndicators;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> StandardLockBluePrint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> SpecialLockBluePrint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> BombIndicatorBluePrint;

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvas{ nullptr };

	FDelegateHandle AddHandle, RemoveHandle;

	TObjectPtr<class USoundBase> StandbyLockSound;
	TSharedPtr<struct FSoundSet> LockSoundSet{ nullptr };

	float LockSoundMin{ 0.05f };
	float LockSoundMax{ 0.3f };
	float LockSoundDuration{ 0.75f };
};
