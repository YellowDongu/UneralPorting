// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpawnHandler.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API USpawnHandler : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT( UMyWorldSubsystem, STATGROUP_Tickables); }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize(void) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;

	void Enlist(class ASpawnTrigger* trigger);

	const TArray<TWeakObjectPtr<class ASpawnTrigger>>& GetActiveList(void) { return ActiveList; }
	const TArray<TWeakObjectPtr<class ASpawnTrigger>>& GetStandbyList(void) { return StandbyList; }
private:
	TArray<TWeakObjectPtr<class ASpawnTrigger>> ActiveList;
	TArray<TWeakObjectPtr<class ASpawnTrigger>> StandbyList;
};
