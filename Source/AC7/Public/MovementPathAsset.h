// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Defined.h"
#include "MovementPathAsset.generated.h"


/**
 * 
 */
UCLASS()
class AC7_API UMovementPathAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    friend class FWaypointEditorData;
    static UMovementPathAsset* GetOrCreateAsset(const FString& PackageName);
    static UMovementPathAsset* Load(const FString& ObjectPath);
    static void Load(UWorld& world, const FString& name, TArray<FWaypointData>& output);

protected:
    UPROPERTY(EditAnywhere) TArray<FWaypointData> SavedPoints;
    UPROPERTY(EditAnywhere) TArray<FWaypointData> SavedWaypoints;
    //UPROPERTY(Transient) TArray<FWaypointPoint> SavedWaypoints; // 이렇게 하면 저장되지 않음
};
