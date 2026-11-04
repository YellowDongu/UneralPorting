// Fill out your copyright notice in the Description page of Project Settings.

#include "Aircraft.h"

#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "MissionMode.h"
#include "Radar.h"
#include "FlightMovement.h"
#include "FireControlSystem.h"
#include "AircraftAnimator.h"

AAircraft::AAircraft()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bTickEvenWhenPaused = false;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	//ThirdCameraArm->SetupAttachment(RootComponent);
	//ThirdCameraArm->TargetArmLength = armLength;
	//ThirdCameraArm->SetRelativeRotation(FRotator(armRotation, 0.f, 0.f));
}

void AAircraft::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	Radar->ResetWarning();
	FlightMovement->Update(deltaTime);
	FireControlSystem->Update(deltaTime);
	AircraftAnimator->Update(deltaTime);
}

void AAircraft::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAircraft::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AAircraft::BeginPlay()
{
	Super::BeginPlay();
	SystemIntegration();
	//AGameModeBase* GM = GetWorld()->GetAuthGameMode();

	
}

void AAircraft::SystemInitialize(const FSpawnInfomation& spawnInfomation)
{
	AVehicleBase::SystemInitialize(spawnInfomation);

	if (spawnInfomation.bIsPlayer && spawnInfomation.playerInfoPointer != nullptr)
	{
		FireControlSystem->SystemInitialize(spawnInfomation.playerInfoPointer->SpecialWeaponIndex);

	}
	else
		FireControlSystem->SystemInitialize(-1);
}

void AAircraft::SystemIntegration()
{
	if (bSystemIntegrated || !bReadyBlueprint)
		return;

	USceneComponent* WeaponContainer = nullptr;
	const TArray<USceneComponent*>& RootChildren = GetRootComponent()->GetAttachChildren();

	for (USceneComponent* Child : RootChildren)
	{
		if (Child->GetFName() == TEXT("Weapon"))
			WeaponContainer = Child;
		else if (Child->GetFName() == TEXT("FirstView"))
			FirstCameraArm = FindComponentByClass<USceneComponent>();
	}

	ThirdCameraArm = FindComponentByClass<USpringArmComponent>();
	if (ThirdCameraArm == nullptr || FirstCameraArm == nullptr)
		bReadyBlueprint = false;

	Audio = FindComponentByClass<UAudioComponent>();

	AircraftAnimator = FindComponentByClass<UAircraftAnimator>();
	if (AircraftAnimator != nullptr)
		AircraftAnimator->SystemIntegration(&Control);

	Radar = FindComponentByClass<URadar>();
	if (Radar != nullptr)
		Radar->SystemIntegration(FindComponentByClass<USphereComponent>(), Team, SPEC.RaderDistance);

	FlightMovement = FindComponentByClass<UFlightMovement>();
	if (FlightMovement != nullptr)
		FlightMovement->SystemIntegration(&Control, &SPEC);

	FireControlSystem = FindComponentByClass<UFireControlSystem>();
	if (FireControlSystem != nullptr && WeaponContainer != nullptr)
		FireControlSystem->SystemIntegration(Radar, WeaponContainer, AircraftAnimator == nullptr ? FBoxSphereBounds() : AircraftAnimator->GetBodyBoundary());

	bSystemIntegrated = true;

	if (Radar == nullptr || FlightMovement == nullptr || FireControlSystem == nullptr || AircraftAnimator == nullptr)
		return;

	PrimaryActorTick.bStartWithTickEnabled = true;
	SetActorTickEnabled(true);
}

void AAircraft::AttachCamera(TWeakObjectPtr<UCameraComponent>& camera, bool thridViewAttach)
{
	if (ThirdCameraArm == nullptr || FirstCameraArm == nullptr)
		return;

	if (camera == nullptr)
		camera = NewObject<UCameraComponent>(this, TEXT("BaseCamera"));

	if (thridViewAttach)
	{
		camera->SetupAttachment(ThirdCameraArm, USpringArmComponent::SocketName);
		camera->RegisterComponent();

		camera->SetRelativeLocation(FVector::ZeroVector);
		camera->SetRelativeRotation(FRotator::ZeroRotator);

		ThirdCameraArm->SetUsingAbsoluteRotation(false);
		ThirdCameraArm->bUsePawnControlRotation = false;

		ThirdCameraArm->TargetArmLength = TargetArmLength;
		ThirdCameraArm->SocketOffset = SocketOffset;

		ThirdCameraArm->bInheritYaw = true;
		ThirdCameraArm->bInheritPitch = true;
		ThirdCameraArm->bInheritRoll = true;
		ThirdCameraArm->bDoCollisionTest = false;       // 장애물에 가려도 카메라 안 당겨지게 하려면 false
		ThirdCameraArm->ProbeChannel = ECC_Camera;
		ThirdCameraArm->SetComponentTickEnabled(true);
	}
	else
	{
		camera->SetupAttachment(FirstCameraArm);
		camera->RegisterComponent();

		camera->SetRelativeLocation(FVector::ZeroVector);
		camera->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

void AAircraft::ChangeTarget() { FireControlSystem->ChangeTarget(); }
bool AAircraft::ShootMissile(TArray<int>& output) { return FireControlSystem->ShootMissile(output); }
bool AAircraft::ChangeMissile() { return FireControlSystem->ChangeMissile(); }
bool AAircraft::ShootMissile() { return FireControlSystem->ShootMissile(); }
bool AAircraft::ShootGun() { return FireControlSystem->ShootGun(); }
void AAircraft::SetTarget(const TWeakObjectPtr<class AVehicleBase>& target) { FireControlSystem->SetTarget(target); }
const TArray<FTargetInfo>& AAircraft::GetSelectedTargets() const { return FireControlSystem->GetCurrentTargets(); }
bool AAircraft::GetSelectSpecial() const { return FireControlSystem->GetSelectSpecial(); }
int AAircraft::GetSpecial() const { return FireControlSystem->GetSpecial(); }
void AAircraft::StartDeployFlare() { Radar->StartDeployFlare(); }
float AAircraft::GetRaderDistance() const { return Radar->GetRaderDistance(); }
const TSet<TWeakObjectPtr<class AVehicleBase>>& AAircraft::GetInRadarTargets() const { return Radar->GetTargets(); }

void AAircraft::SetGear(bool value, bool instantSet) { return AircraftAnimator->DeployGear(value, instantSet); }
void AAircraft::DeployGear(bool instantSet) { return AircraftAnimator->DeployGear(!AircraftAnimator->IsGearActive(), instantSet); }