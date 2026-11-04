// Fill out your copyright notice in the Description page of Project Settings.


#include "FlightMovement.h"
#include "Aircraft.h"

UFlightMovement::UFlightMovement()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoRegister = false;
	bWantsInitializeComponent = false;
	bAutoActivate = false;
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlightMovement::BeginPlay()
{
	Super::BeginPlay();
}

//void UFlightMovement::PostInitializeComponents()
//{
//	Super::PostInitializeComponents();
//}

void UFlightMovement::Update(float deltaTime)
{
	if (Control == nullptr)
		return;

	CalculateVelocity(deltaTime);
	if (WaypointData.IsEmpty())
	{
		AActor* Owner = GetOwner();
		Owner->SetActorLocation(Owner->GetActorLocation() + Owner->GetActorForwardVector() * Control->Velocity * deltaTime * 2.0f);
		RotationCalculation(deltaTime);
	}
	else
		FollowWaypoint(deltaTime);
}

void UFlightMovement::CalculateVelocity(float deltaTime)
{
	//float rotationFactor = 1.0f + dragFactor.x * Mathf.Clamp01(Mathf.Abs(rotationDelta.x));
	float rotationFactor = 1.0f;

	float throttleInput = FMath::Max<float>(0.0f, Control->Throttle - 0.2f);
	float thrust = enginePower * throttleInput + IdleSpeed;

	float speedRatio = (Control->Velocity - IdleSpeed) / (NonExceedSpeed - IdleSpeed);
	float drag = thrust * speedRatio * speedRatio * FMath::Sign(speedRatio) + IdleSpeed;
	//if(control.velocity < MinSpeed)
	//    drag -= MinSpeed;
	drag *= rotationFactor;

	if (Control->isAirBrakeOn)
		drag *= airbrakePower;

	Control->Velocity = FMath::Max<float>(100.0f, Control->Velocity + (thrust - drag) * deltaTime * 0.5f);
}

float UFlightMovement::CalculateThrottleForTargetVelocity(float targetVelocity)
{
	if (targetVelocity >= NonExceedSpeed)
		return 1.0f;

	float speedRatio = (targetVelocity - IdleSpeed) / (NonExceedSpeed - IdleSpeed);
	float k = speedRatio * speedRatio * FMath::Sign(speedRatio);

	float denom = (1.0f - k) * enginePower;
	if (FMath::IsNearlyZero(denom))
		return 1.0f;

	return ((k * IdleSpeed) / denom) + 0.2f;
}

void UFlightMovement::RotationCalculation(float deltaTime)
{
	const float minDelta = 0.01f;
	rotationDelta.X = FMath::Clamp(rotationDelta.X + (Control->Yoke.X * rotationSpeed.X /* * rotationSpeed.z*/ * 5.0f * deltaTime - rotationDelta.X /* * rotationSpeed.z * 0.5f*/ * 2.5f * deltaTime), -rotationSpeed.X, rotationSpeed.X);
	rotationDelta.Y = FMath::Clamp(rotationDelta.Y + (Control->Yoke.Y * rotationSpeed.Y /* * rotationSpeed.y*/ * 5.0f * deltaTime - rotationDelta.Y /* * rotationSpeed.y * 0.5f*/ * 2.5f * deltaTime), -rotationSpeed.Y, rotationSpeed.Y);
	rotationDelta.Z = FMath::Clamp(rotationDelta.Z + (Control->Yoke.Z * rotationSpeed.Z /* * rotationSpeed.x*/ * 5.0f * deltaTime - rotationDelta.Z /* * rotationSpeed.x * 0.5f*/ * 2.5f * deltaTime), -rotationSpeed.Z, rotationSpeed.Z);

	//if (rotationDelta.IsNearlyZero())
	//	rotationDelta = FVector3d::ZeroVector;

	if (FMath::Abs(rotationDelta.X) < minDelta) rotationDelta.X = 0.0f;
	if (FMath::Abs(rotationDelta.Y) < minDelta) rotationDelta.Y = 0.0f;
	if (FMath::Abs(rotationDelta.Z) < minDelta) rotationDelta.Z = 0.0f;

	FRotator rotator = FRotator(rotationDelta.Z * deltaTime * Control->GetHighGTurn(deltaTime), rotationDelta.Y * deltaTime, rotationDelta.X * deltaTime);
	GetOwner()->AddActorLocalRotation(FQuat(rotator));
}

void UFlightMovement::SystemIntegration(FControl* poineter, FAircraftSpec* specPointer)
{
	if (poineter == nullptr || specPointer == nullptr)
		return;

	if (GetOwner() == nullptr || Cast<APawn>(GetOwner()) == nullptr)
		return;

	Control = poineter;

	NonExceedSpeed = specPointer->NonExceedSpeed;
	Control->Velocity = IdleSpeed = specPointer->IdleSpeed;
	enginePower = specPointer->EnginePower;

	airbrakePower = specPointer->AirbrakePower;
	rotationDelta = FVector::ZeroVector;
	dragFactor = specPointer->DragFactor;
	rotationSpeed = specPointer->RotationSpeed;
	//control->CalculateThrottle = FControl::FCalculateThrottle::CreateUObject(this, &UFlightMovement::CalculateThrottleForTargetVelocity);
	Control->CalculateThrottle.BindUObject(this, &UFlightMovement::CalculateThrottleForTargetVelocity);
}

void UFlightMovement::FollowWaypoint(float deltaTime)
{
	double leftDistance = static_cast<double>(Control->Velocity) * static_cast<double>(deltaTime) * 2.0;
	double leftDistanceSqare = leftDistance * leftDistance;
	FVector position = GetOwner()->GetActorLocation();
	FQuat rotation = GetOwner()->GetActorQuat();

	while (!WaypointData.IsEmpty())
	{
		FWaypointData& data = WaypointData.Last();
		double sqrDisatnce = (data.Position - position).SquaredLength();

		if (FMath::IsNearlyZero(sqrDisatnce))
			return;

		if (leftDistanceSqare <= sqrDisatnce)
		{
			double leftPercentage = FMath::Min(FMath::Sqrt(leftDistanceSqare / sqrDisatnce), 1.0);
			FVector finalPosition = FMath::Lerp(position, data.Position, leftPercentage);
			FQuat finalRotation = FQuat::Slerp(rotation, data.Rotation, leftPercentage);
			GetOwner()->SetActorLocationAndRotation(finalPosition, finalRotation);

			leftDistanceSqare = 0.0;
			return;
		}

		position = data.Position;
		rotation = data.Rotation;
		WaypointData.Pop(false);
		leftDistance -= FMath::Sqrt(sqrDisatnce);
		leftDistanceSqare = leftDistance * leftDistance;
	}

	leftDistance = FMath::Sqrt(leftDistanceSqare);
	if (FMath::IsNearlyZero(leftDistance))
		return;

	GetOwner()->SetActorLocationAndRotation(rotation.GetForwardVector() * leftDistance + position, rotation);
}

void UFlightMovement::SetWayPoint(const TArray<FWaypointData>& data, bool NeedCompensatePosition)
{
	WaypointData.Reset(data.Num());

	if (NeedCompensatePosition)
	{
		FVector position = GetOwner()->GetActorLocation();

		for (int32 i = data.Num() - 1; i >= 0; --i)
		{
			const FWaypointData& item = data[i];
			WaypointData.Emplace(item.Position * WaypointScale + position, item.Rotation, item.DistanceValue);
		}
	}
	else
		for (int32 i = data.Num() - 1; i >= 0; --i)
			WaypointData.Add(data[i]);
}
