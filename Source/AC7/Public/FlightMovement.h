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
	void ClearWayPoint(void) { WaypointData.Reset(); }

	bool IsFollowingWayPoint() { return !WaypointData.IsEmpty(); }
private:
	TArray<FWaypointData> WaypointData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spec", meta = (AllowPrivateAccess = "true"))
	double WaypointScale{1.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spec", meta = (AllowPrivateAccess = "true"))
	float NonExceedSpeed{ 130.0 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spec", meta = (AllowPrivateAccess = "true"))
	float IdleSpeed{ 33.0 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spec", meta = (AllowPrivateAccess = "true"))
	float enginePower{ 100.0 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spec", meta = (AllowPrivateAccess = "true"))
	float airbrakePower{ 2.0 };

	struct FControl* Control{ nullptr };

	FVector force;
	FVector rotationDelta = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spec", meta = (AllowPrivateAccess = "true"))
	FVector dragFactor{ 0.25, 0.0001, 0.0001 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spec", meta = (AllowPrivateAccess = "true"))
	FVector rotationSpeed{ 55.0, 55.0, 2.5 };
};
