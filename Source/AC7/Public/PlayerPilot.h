// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Defined.h"
#include "PlayerPilot.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API APlayerPilot : public APlayerController
{
	GENERATED_BODY()
public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FSelectTarget, class AVehicleBase*, bool);
	FSelectTarget SetSelectTargetUI;
	DECLARE_DELEGATE_TwoParams(FFiredMissile, const TArray<int>&, bool);
	FFiredMissile FiredMissile;
	DECLARE_DELEGATE_OneParam(FChangeWeaponUI, bool);
	FChangeWeaponUI ChangeWeaponUI;

	APlayerPilot();

	void ChangeTarget();
	void GearDeploy();
	void ChangeMissile();
	void DeployFlare();
	void ShootWeapon();
	void ShootGun();

	void ResetCamera();
	void YawCamRotatePlus(void) { YawCamRotate += 1.0; }
	void YawCamRotateMinus(void) { YawCamRotate -= 1.0; }
	void PitchCamRotatePlus(void) { PitchCamRotate += 1.0; }
	void PitchCamRotateMinus(void) { PitchCamRotate -= 1.0; }

	void YokeXPlus(void) { Control->Yoke.X += 1.0; }
	void YokeXMinus(void) { Control->Yoke.X -= 1.0; }
	void YokeYPlus(void) { Control->Yoke.Y += 1.0; }
	void YokeYMinus(void) { Control->Yoke.Y -= 1.0; }
	void YokeZPlus(void) { Control->Yoke.Z += 1.0; }
	void YokeZMinus(void) { Control->Yoke.Z -= 1.0; }

	//virtual void BeginPlay() override;
	//virtual void PostInitProperties() override;
	//virtual void PreInitializeComponents() override;
	//virtual void PostInitializeComponents() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	void CheatEndMission();

	void Attach(class AAircraft* aircraft);
	void LinkHUD(void);
	void CameraExplosionShake(float duration = 0.5f, float attenuation = 0.4f, float strength = 1.0f);
private:
	FControl* Control{ nullptr };
	TWeakObjectPtr<class AAircraft> Aircraft{ nullptr };

	TWeakObjectPtr<class UFrontHUD> FrontHUD{ nullptr };
	TWeakObjectPtr<class UCameraComponent> PlayerCamera{ nullptr };

	TWeakObjectPtr<class USceneComponent> FirstCameraArm;
	TWeakObjectPtr<class USpringArmComponent> ThirdCameraArm;

	bool bShootGun{false};
	double CamRotateSpeed{ 5.0 }, YawCamRotate{ 0.0 }, PitchCamRotate{ 0.0 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UFrontHUD> FrontHUDBlueprint{};
	TArray<int> Container;

	TWeakObjectPtr<UAudioComponent> GunChannel;
	TObjectPtr<class USoundBase> ShootSound, SpecialShootSound, GunSound;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	//TSubclassOf<class UCameraShakeBase> ExplosionShake;
};
