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
    static bool Load(UWorld& world, const FString& name, TArray<FWaypointData>& output);

    const TArray<FWaypointData>& GetSavedPoints(void) { return SavedPoints; }
    const TArray<FWaypointData>& GetWaypoints(void) { return SavedWaypoints; }

protected:
    static UMovementPathAsset* Load(UWorld& world, const FString& name);



    UPROPERTY(EditAnywhere) TArray<FWaypointData> SavedPoints; // 보간된 것들
    UPROPERTY(EditAnywhere) TArray<FWaypointData> SavedWaypoints; // 원래 웨이포인트
    //UPROPERTY(Transient) TArray<FWaypointPoint> SavedWaypoints; // 이렇게 하면 저장되지 않음
};
