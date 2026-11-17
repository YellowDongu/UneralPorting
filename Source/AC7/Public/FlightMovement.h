// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Defined.h"
#include "FlightMovement.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AC7_API UFlightMovement : public UFloatingPawnMovement
{
	GENERATED_BODY()
public:
	UFlightMovement();

	virtual void Update(float deltaTime);
	virtual void BeginPlay() override;
	//virtual void PostInitializeComponents() override;

	void CalculateVelocity(float deltaTime);
	void RotationCalculation(float deltaTime);
	float CalculateThrottleForTargetVelocity(float targetVelocity);
	void SystemIntegration(struct FControl* poineter, struct FAircraftSpec* specPointer);

	void FollowWaypoint(float deltaTime);
	void SetWayPoint(const TArray<FWaypointData>& data, bool NeedCompensatePosition = false);
	void InjectWayPoint(const TArray<FWaypointData>& data) { WaypointData = data; }
	void InjectWayPoint(const FWaypointData& data) { WaypointData.Add(data); }
	void ClearWayPoint(void) { WaypointData.Reset(); }
	bool IsFollowingWayPoint() { return !WaypointData.IsEmpty(); }

private:
	TArray<FWaypointData> WaypointData;

	double WaypointScale{ 1.0 };
	float NonExceedSpeed{ 130.0f }, IdleSpeed{ 33.0f }, EnginePower{ 100.0f }, AirbrakePower{ 2.0f };

	struct FControl* Control{ nullptr };

	FVector Force{ FVector::ZeroVector }, RotationDelta{ FVector::ZeroVector }, DragFactor{ 0.25, 0.0001, 0.0001 }, RotationSpeed{ 55.0, 55.0, 2.5 };
};
