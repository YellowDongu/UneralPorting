// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundVehicle.h"

#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "MissionMode.h"
#include "Radar.h"
#include "FireControlSystem.h"

AGroundVehicle::AGroundVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bTickEvenWhenPaused = false;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AGroundVehicle::BeginPlay()
{
	Super::BeginPlay();
	SystemIntegration();
}

void AGroundVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGroundVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Radar->ResetWarning();
	FireControlSystem->Update(DeltaTime);
}

void AGroundVehicle::SystemInitialize(const FSpawnInfomation& spawnInfomation)
{
	Super::SystemInitialize(spawnInfomation);
}

void AGroundVehicle::SystemIntegration()
{
	if (bSystemIntegrated)
		return;

	USceneComponent* WeaponContainer = nullptr;
	const TArray<USceneComponent*>& RootChildren = GetRootComponent()->GetAttachChildren();
	
	for (USceneComponent* Child : RootChildren)
	{
		if (Child->GetFName() == TEXT("Weapon"))
		{
			WeaponContainer = Child;
			break;
		}
		else if (Child->GetFName() == TEXT("FirstView"))
			FirstCameraArm = FindComponentByClass<USceneComponent>();
	}

	Radar = FindComponentByClass<URadar>();
	if (Radar != nullptr)
		Radar->SystemIntegration(FindComponentByClass<USphereComponent>(), Team, ConvertFromDistance(RadarDistance));
	



	FireControlSystem = FindComponentByClass<UFireControlSystem>();
	if (FireControlSystem != nullptr)
		FireControlSystem->SystemIntegration(Radar, WeaponContainer, FBoxSphereBounds());

	bSystemIntegrated = true;

	//if (Radar == nullptr || FlightMovement == nullptr || FireControlSystem == nullptr || AircraftAnimator == nullptr)
	if (Radar == nullptr)
		return;

	PrimaryActorTick.bStartWithTickEnabled = true;
	SetActorTickEnabled(true);
}

//inline void AGroundVehicle::ChangeTarget() { FireControlSystem->ChangeTarget(); }
//inline bool AGroundVehicle::ShootMissile(TArray<int>& output) { return FireControlSystem->ShootMissile(output); }
//inline bool AGroundVehicle::ChangeMissile() { return FireControlSystem->ChangeMissile(); }
//inline bool AGroundVehicle::ShootMissile() { return FireControlSystem->ShootMissile(); }
//inline bool AGroundVehicle::ShootGun() { return FireControlSystem->ShootGun(); }
//inline void AGroundVehicle::SetTarget(const TWeakObjectPtr<class AVehicleBase>& target) { FireControlSystem->SetTarget(target); }
//inline const TArray<FTargetInfo>& AGroundVehicle::GetSelectedTargets() const { return FireControlSystem->GetCurrentTargets(); }
//inline bool AGroundVehicle::GetSelectSpecial() const { return FireControlSystem->GetSelectSpecial(); }
//inline int AGroundVehicle::GetSpecial() const { return FireControlSystem->GetSpecial(); }
//inline void AGroundVehicle::StartDeployFlare() { Radar->StartDeployFlare(); }

inline float AGroundVehicle::GetRaderDistance() const { return Radar->GetRaderDistance(); }
inline const TSet<TWeakObjectPtr<class AVehicleBase>>& AGroundVehicle::GetInRadarTargets() const { return Radar->GetTargets(); }
