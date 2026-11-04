// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AircraftAnimation.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UAircraftAnimation : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	float GetMaxLength() { return MaxLength; }
	float* GetAileron() { return &Aileron; }
	float* GetLAileron() { return &LAileron; }
	float* GetRAileron() { return &RAileron; }
	float* GetElevator() { return &Elevator; }
	float* GetLElevator() { return &LElevator; }
	float* GetRElevator() { return &RElevator; }
	float* GetRudder() { return &Rudder; }
	float* GetLRudder() { return &LRudder; }
	float* GetRRudder() { return &RRudder; }
	float* GetGear() { return &Gear; }
	float* GetSpeedParts() { return &SpeedParts; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UAnimSequence* BaseAnimation{ nullptr };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float MaxLength{ 0.0f };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Gear{ 0.0f };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Aileron{ 0.5f };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float LAileron{ 0.5f };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float RAileron{ 0.5f };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Elevator{ 0.5f };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float LElevator{ 0.5f };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float RElevator{ 0.5f };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float Rudder{ 0.5f };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float LRudder{ 0.5f };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float RRudder{ 0.5f };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float SpeedParts{ 0.0f };

};
