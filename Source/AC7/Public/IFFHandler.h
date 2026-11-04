// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IFFHandler.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UIFFHandler : public UUserWidget
{
	GENERATED_BODY()
public:
	UIFFHandler(const FObjectInitializer& ObjectInitializer);
	void BindPlayer(class AAircraft* vehicle, class APlayerPilot* player);

	void NativeTick(const FGeometry& MyGeometry, float DeltaTime);

	UFUNCTION()
	void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//void AddTarget(class AVehicleBase* Target);
	void RemoveTarget(class UIFFHUD* target);
	//void RemoveTarget(class AVehicleBase* Target);
	void SelectTarget(class AVehicleBase* target, bool isSelected);



private:
	TWeakObjectPtr<class AAircraft> Player{ nullptr };
	TWeakObjectPtr<class URadar> PlayerRadar{ nullptr };

	//TSet<TObjectPtr<class UIFFHUD>> Alive;
	TMap<TWeakObjectPtr<class AVehicleBase>, TObjectPtr<class UIFFHUD>> Alive;
	TArray<TObjectPtr<class UIFFHUD>> Stored;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UIFFHUD> BluePrint;

	UPROPERTY(EditAnywhere, meta = (BindWidget, Category = "UI", AllowPrivateAccess = "true"))
	class UCanvasPanel* BaseCanvas{ nullptr };


	FDelegateHandle AddHandle;
	FDelegateHandle RemoveHandle;
};
