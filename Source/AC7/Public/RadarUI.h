// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Defined.h"
#include "RadarUI.generated.h"

/**
 * 
 */
class URadarTargetUI;
UCLASS()
class AC7_API URadarUI : public UUserWidget
{
	GENERATED_BODY()
public:
	//URadarUI(const FObjectInitializer& ObjectInitializer);
	void BindPlayer(class AAircraft* vehicle, class APlayerPilot* player);
	void NativeTick(const FGeometry& MyGeometry, float DeltaTime);

	void Refresh(float DeltaTime);
	void ChangeRadar();

	TObjectPtr<URadarTargetUI> GetTargetUI();
	TObjectPtr<URadarTargetUI> GetTargetUI(class UCanvasPanel* AttachingPanel);
	void RetriveTargetUI(TObjectPtr<URadarTargetUI> target);
	void ChangeStatus(bool value) { bIsExpended = value; }

	UFUNCTION(BlueprintCallable)
	static void SetBoarder(const FVector& leftBottom, const FVector& rightTop) { LeftBottomBoarder = leftBottom; RightTopBoarder = rightTop; }

private:
	TWeakObjectPtr<class AAircraft> Player{ nullptr };
	TWeakObjectPtr<class URadar> PlayerRadar{ nullptr };

	bool bIsExpended{ false };
	float Timer{ 0.0f }, RefreshTime{ 0.25f };
	float CloseRangeSize{ ConvertFromDistance(5000.0f) };
	TArray<TObjectPtr<URadarTargetUI>> Alive, Stored;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<URadarTargetUI> TargetBluePrint;

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvas{ nullptr };

	class UCanvasPanel* CurrentCanvas{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* CloseRangePanel{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvasS{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* LongRangePanel{ nullptr };
	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvasL{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	//FVector LargeRadarSize{ 410.0 , 410.0 , 0.0 };
	FVector LargeRadarSize;

	FDelegateHandle AddHandle;
	FDelegateHandle RemoveHandle;

	static FVector LeftBottomBoarder;
	static FVector RightTopBoarder;
};
