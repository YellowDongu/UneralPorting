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
		UE_LOG(LogTemp, Error, TEXT("[UAircraftAnimator::SystemIntegration] No Skeletal Body Animation"));
		DestroyComponent();
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

	FUpdate.AddWeakLambda(this, [this](float DeltaTime) -> void // basic
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
						  });

	FUpdate.AddWeakLambda(this, [this](float DeltaTime) -> void // 만약 속도 기반 가변 파츠가 있다면 추가
						  {
							  static constexpr float preMach = 600.0f; // 661kn
							  bool HighSpeed = Control->Velocity > preMach;
							  SPVP.PlayFrameTime(DeltaTime, HighSpeed ? 1.0f : 0.0f);
							  SPVP.Update();
						  });
}

void UAircraftAnimator::ReadComponent()
{
	if (bComponentBinded)
		return;

	auto list = GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), TEXT("Body"));
	if (list.IsEmpty())
	{
		DestroyComponent();
		return;
	}

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
		if(GearOffStaticMesh != nullptr) GearOffStaticMesh->SetVisibility(false);
		if (GearOnStaticMesh != nullptr) GearOnStaticMesh->SetVisibility(false);

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
	if (bGearActive == value)
		return;

	bGearActive = value;

	if (bNoGear)
	{
		StaticGearActive(value);
		return;
	}

	if (instantSet)
	{
		Gear.SetFrameTime(bGearActive ? 1.0f : 0.0f);
		Gear.Update();
		return;
	}

	if (GearUpdate.IsValid())
		return;

	GearUpdate = FUpdate.AddWeakLambda(this, [this](float DeltaTime) -> void
						  {
										   if (Gear.PlayFrameTime(DeltaTime, bGearActive ? 1.0f : 0.0f))
											   FUpdate.Remove(GearUpdate);
										   Gear.Update();
						  });
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

