// Fill out your copyright notice in the Description page of Project Settings.


#include "AircraftAnimator.h"
#include "Aircraft.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "AircraftAnimation.h"

UAircraftAnimator::UAircraftAnimator()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = false;
}

void UAircraftAnimator::SystemIntegration(FControl* control)
{
	if (control == nullptr)
		return;

	Control = control;
	ReadComponent();
	if (!bComponentBinded)
		return;

	CalculatePosition();

	if (BodyMesh->GetAnimInstance() == nullptr)
		UE_LOG(LogTemp, Error, TEXT("AnimInstance == nullptr"));

	UAircraftAnimation* Animation = nullptr;
	if (GearMesh != nullptr)
	{
		Animation = Cast<UAircraftAnimation>(GearMesh->GetAnimInstance());

		if (Animation != nullptr)
		{
			Gear.Initialize(Animation->GetGear(), Animation->GetMaxLength(), animationSpeed * 0.25f);
			Gear.SetFrameTime(0.0f);
			Gear.Update();
		}
		else
			UE_LOG(LogTemp, Error, TEXT("[UAircraftAnimator::SystemIntegration] No Skeletal Gear Animation"));
	}
	else
		UE_LOG(LogTemp, Error, TEXT("[UAircraftAnimator::SystemIntegration] No Skeletal Gear Mesh"));

	Animation = Cast<UAircraftAnimation>(BodyMesh->GetAnimInstance());
	if (Animation == nullptr)
	{
		PrimaryComponentTick.bStartWithTickEnabled = false;
		PrimaryComponentTick.SetTickFunctionEnable(false);
		UE_LOG(LogTemp, Error, TEXT("[UAircraftAnimator::SystemIntegration] No Skeletal Body Animation"));
		return;
	}

	Aileron.Initialize(Animation->GetAileron(), Animation->GetMaxLength(), animationSpeed);
	Aileron.SetFrameTime(0.5f);

	Rudder.Initialize(Animation->GetRudder(), Animation->GetMaxLength(), animationSpeed);
	Rudder.SetFrameTime(0.5f);

	Elevator.Initialize(Animation->GetElevator(), Animation->GetMaxLength(), animationSpeed);
	Elevator.SetFrameTime(0.5f);

	SPVP.Initialize(Animation->GetSpeedParts(), Animation->GetMaxLength(), animationSpeed);
	SPVP.SetFrameTime(0.0f);

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.SetTickFunctionEnable(true);
	bAutoActivate = true;
}

void UAircraftAnimator::ReadComponent()
{
	if (bComponentBinded)
		return;

	auto list = GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), TEXT("Body"));
	if (list.IsEmpty()) return;
	BodyMesh = Cast<USkeletalMeshComponent>(list[0]);

	list = GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), TEXT("Gear"));
	if (!list.IsEmpty()) GearMesh = Cast<USkeletalMeshComponent>(list[0]);

	bNoGear = GearMesh == nullptr;

	list = GetOwner()->GetComponentsByTag(UStaticMeshComponent::StaticClass(), TEXT("GearOnSM"));
	if (!list.IsEmpty()) GearOnStaticMesh = Cast<UStaticMeshComponent>(list[0]);

	list = GetOwner()->GetComponentsByTag(UStaticMeshComponent::StaticClass(), TEXT("GearOffSM"));
	if (!list.IsEmpty()) GearOffStaticMesh = Cast<UStaticMeshComponent>(list[0]);

	bNoStaticGear = GearOnStaticMesh == nullptr || GearOffStaticMesh == nullptr;

	bStaticGearActive = false;
	if (!bNoGear)
	{
		GearOffStaticMesh->SetVisibility(false);
		GearOnStaticMesh->SetVisibility(false);

		bStaticGearActive = !bNoGear ? false : GearOnStaticMesh != nullptr;
	}
	else if(!bNoStaticGear)
	{
		GearOffStaticMesh->SetVisibility(true);
		GearOnStaticMesh->SetVisibility(false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UAircraftAnimator::ReadComponent] No Gear Mesh"));

		if (GearOnStaticMesh != nullptr)
			GearOnStaticMesh->SetVisibility(false);
	}

	bComponentBinded = true;
}

void UAircraftAnimator::StaticGearActive(bool value)
{
	if (bStaticGearActive == value || bNoStaticGear)
		return;

	bStaticGearActive = value;
	GearOffStaticMesh->SetVisibility(!bStaticGearActive);
	GearOnStaticMesh->SetVisibility(bStaticGearActive);
}

void UAircraftAnimator::DeployGear(bool value, bool instantSet)
{
	if (bNoGear)
		StaticGearActive(value);

	bGearActive = value;
	if (instantSet)
	{
		Gear.SetFrameTime(bGearActive ? 0.0f : 1.0f);
		Gear.Update();
		return;
	}

	if (!GearTimerHandle.IsValid())
		GetWorld()->GetTimerManager().SetTimer(GearTimerHandle, this, &UAircraftAnimator::MoveGear, 1.0f / 60.0f, true);
}

void UAircraftAnimator::MoveGear()
{
	if (Gear.PlayFrameTime(GetWorld()->GetDeltaSeconds(), bGearActive ? 0.0f : 1.0f))
		GetWorld()->GetTimerManager().ClearTimer(GearTimerHandle);
	Gear.Update();
}

void UAircraftAnimator::CalculatePosition()
{
	MeshBoundary = BodyMesh->GetSkeletalMeshAsset()->GetBounds();
	CenterFloor = BodyCenter = MeshBoundary.Origin;

	FBoxSphereBounds Bounds;
	if (GearMesh != nullptr)
		Bounds = GearMesh->GetSkeletalMeshAsset()->GetBounds();
	else if (GearOnStaticMesh != nullptr)
		Bounds = GearOnStaticMesh->GetStaticMesh()->GetBounds();
	else return;

	GearCenter = Bounds.Origin;
	FVector Size = Bounds.BoxExtent * 2;
	float zMin = GearCenter.Z - (Size.Z * 0.5f);
	CenterFloor.Z = zMin;
}

void UAircraftAnimator::Update(float DeltaTime)
{
	//UE_LOG(LogTemp, Log, TEXT("Tick %3f : %3f : %3f"), Aileron.GetFrameTime(), Rudder.GetFrameTime(), Elevator.GetFrameTime());
	//Aileron.FrameTime = FMath::Clamp(Aileron.FrameTime + ((1.0f + Control->Yoke.X) * 0.5f - Aileron.FrameTime) * DeltaTime * animationSpeed, 0.0f, 1.0f);
	//Rudder.FrameTime = FMath::Clamp(Rudder.FrameTime + ((1.0f + Control->Yoke.Z) * 0.5f - Rudder.FrameTime) * DeltaTime * animationSpeed, 0.0f, 1.0f);
	//Elevator.FrameTime = FMath::Clamp(Elevator.FrameTime + ((1.0f + Control->Yoke.Y) * 0.5f - Elevator.FrameTime) * DeltaTime * animationSpeed, 0.0f, 1.0f);
	Aileron.PlayFrameTime(DeltaTime, (1.0f + Control->Yoke.X) * 0.5f);
	Rudder.PlayFrameTime(DeltaTime, (1.0f - Control->Yoke.Y) * 0.5f);
	Elevator.PlayFrameTime(DeltaTime, (1.0f + Control->Yoke.Z) * 0.5f);

	Aileron.Update();
	Rudder.Update();
	Elevator.Update();

	static constexpr float preMach = 600.0f; // 661kn
	bool HighSpeed = Control->Velocity > preMach;
	SPVP.PlayFrameTime(DeltaTime, HighSpeed ? 1.0f : 0.0f);
	SPVP.Update();
}
