// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VehicleAnimation.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UVehicleAnimation : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;




	float GetMaxLength() { return MaxLength; }

	float* GetTurretYaw() { return &TurretYaw; }
	float* GetTurretPitch() { return &TurretPitch; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UAnimSequence* BaseAnimation{ nullptr };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float MaxLength{ 0.0f };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float TurretYaw{ 0.0f };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float TurretPitch{ 0.0f };

};
