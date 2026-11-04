// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Defined.h"
#include "DisplayCase.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API ADisplayCase : public AActor
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	void Initialize();
	void ChangeDisplay(EVehicleID id);
	void CreateAircraft(EVehicleID id, UBluePrintCollector* collector, UWorld* world);
	const TArray<TWeakObjectPtr<class AVehicleBase>>& GetVehicleList() { return Vehicles; }
private:
	bool bInitialized{ false };

	UPROPERTY()
	AVehicleBase* CurrentDisplay;

	TArray<TWeakObjectPtr<class AVehicleBase>> Vehicles;
};
	