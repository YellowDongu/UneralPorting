// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireControlSystem.h"
#include "WeaponInterface.generated.h"

/**
 * for Ground AI
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class AC7_API UWeaponInterface : public UFireControlSystem
{
	GENERATED_BODY()
public:
	UWeaponInterface();

	virtual void Update(float DeltaTime) override;
	virtual void SystemIntegration(TWeakObjectPtr<class URadar> radar, USceneComponent* WeaponContainer, const FBoxSphereBounds& bodyBoundary = FBoxSphereBounds()) override;

	void TrackGun(float DeltaTime);
	void TrackMissile(float DeltaTime);
	void TurretSet(float DeltaTime);

	bool IsMissileEmpty(void) { return standard.IsEmpty(); }
	bool CanShootGun(void) { return bGunEnable; }
	bool Aimed(void) { return TargetAngleCos > AngleCos; }
	bool Locked(void) { return !currentTargets.IsEmpty() && currentTargets[0].IsLock(); }

	const float& GetTargetYaw() const { return TargetYaw; }
	const float& GetTargetPitch() const { return TargetPitch; }
	const float& GetTurretYaw() const { return *TurretYaw; }
	const float& GetTurretPitch() const { return *TurretPitch; }

private:
	bool bEnemySpotted{ false };
	float TargetYaw{ 0.0f }, TargetPitch{ 0.0f };
	float YawDelta{ 0.0f }, PitchDelta{ 0.0f };
	double AngleCos{ 0.0f }, TargetAngleCos{-1.0f};
	FTickEvent UpdateEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bGunEnable{ false };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float TurretYawRotateSpeed{ 1.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float TurretPitchRotateSpeed{ 1.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FName TurretBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	double Range{ 0.0 }; // 런타임 시 제곱된 형태로 바꿈

	float* TurretYaw{ nullptr },* TurretPitch{ nullptr };
	TWeakObjectPtr<USkeletalMeshComponent> BodyMesh{ nullptr };
	FVector TurretPosition, TargetDirection;
};
