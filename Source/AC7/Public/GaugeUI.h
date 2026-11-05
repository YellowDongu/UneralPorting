// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Defined.h"
#include "GaugeUI.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FGaugeImageSet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UTexture2D* Image;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UTexture2D* InnerImage;
};

UCLASS()
class AC7_API UGaugeUI : public UUserWidget
{
	GENERATED_BODY()

	struct FGauge
	{
		friend class UGaugeUI;
		friend struct FGaugeSet;
	public:
		FGauge() = default;
		FGauge(float timeMax, TWeakObjectPtr<class UProgressBar> progressBar) : TimeMax(timeMax), ProgressBar(progressBar) {}

		bool Tick(float deltaTime);

		void StartTimer(UWorld* world);
		void ActiveTimer();
		void EditGauge(float value); // 값 강제 조정
		void EditGaugePercentage(float value); // 값 강제 조정
		void SetGaugeImage(const FGaugeImageSet& imageSet);

		static const float FrameTime;
		float TimeMax{ 0.0f };
		float Timer{ 0.0f };
		
		UWorld* World{ nullptr };
		TWeakObjectPtr<class UProgressBar> ProgressBar{ nullptr };

		FTimerHandle TimerHandle;
		FDelegateHandle DelegateHandle;
	};

	struct FGaugeSet
	{
	public:
		FGaugeSet() = default;

		void Tick(float deltaTime)
		{
			for (auto iterator = ActivedGauge.CreateIterator(); iterator; iterator++)
				if ((*iterator)->Tick(deltaTime))
					iterator.RemoveCurrent();
		}

		void CreateGaugeSet(class UCanvasPanel* gaugeCanvas, TSubclassOf<class AMissile> targetBluePrint);
		void CreateGaugeSet(class UCanvasPanel* gaugeCanvas, TSubclassOf<class AMissile> targetBluePrint, const FGaugeImageSet& imageSet);
		void SetGaugePercentage(float value, int index = 0) { if (GaugeBars.IsValidIndex(index)) GaugeBars[index].EditGaugePercentage(value); }

		void StartTimer(UWorld* world, int gaugeNumber, bool Descend = true)
		{
			if (gaugeNumber < 0 || gaugeNumber >= GaugeBars.Num())
				return;

			FGauge& gauge = GaugeBars[gaugeNumber];
			//GaugeBars[gaugeNumber].StartTimer(world);
			///GaugeBars[gaugeNumber].Descend = Descend;

			ActivedGauge.Add(&gauge);
		}

		TArray<FGauge> GaugeBars;
		TWeakObjectPtr<class UCanvasPanel> BaseCanvas{ nullptr };
	private:
		TArray<FGauge*> ActivedGauge;
	};

public:
	virtual void NativeTick(const FGeometry& myGeometry, float deltaTime) override;

	void BindPlayer(class AAircraft* vehicle, class APlayerPilot* player);
	void MissileFired(const TArray<int>& list, bool isSpecial);
	void ChangeGauge(bool isSpecial);

private:
	FGauge HPGauge;
	FGaugeSet StandardMissileGauge;
	FGaugeSet SpecialMissileGauge;

	bool bGaugeSelection;

	TWeakObjectPtr<class AAircraft> Player{ nullptr };
	TWeakObjectPtr<class UFireControlSystem> PlayerFCS{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvas{ nullptr };

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* MissileGaugeCanvas{ nullptr };;
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* StandardGaugeCanvas{ nullptr };


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class AMissile>> MissileBluprints;

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* Gauge4Canvas{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* Gauge2Canvas{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UProgressBar* HPGaugeBar{ nullptr };


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TMap<EVehicleID, FGaugeImageSet> HPGaugeImageSet;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TMap<EWeaponID, FGaugeImageSet> WeaponGaugeImageSet;
};
