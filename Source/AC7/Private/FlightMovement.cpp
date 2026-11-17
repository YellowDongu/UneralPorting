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
		Owner->SetActorLocation(Owner->GetActorLocation() + Owner->GetActorForwardVector() * ConvertFromVelocity(Control->Velocity) * deltaTime);
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
	float thrust = EnginePower * throttleInput + IdleSpeed;

	float speedRatio = (Control->Velocity - IdleSpeed) / (NonExceedSpeed - IdleSpeed);
	float drag = thrust * speedRatio * speedRatio * FMath::Sign(speedRatio) + IdleSpeed;
	//if(control.velocity < MinSpeed)
	//    drag -= MinSpeed;
	drag *= rotationFactor;

	if (Control->isAirBrakeOn)
		drag *= AirbrakePower;

	Control->Velocity = FMath::Max<float>(100.0f, Control->Velocity + (thrust - drag) * deltaTime * 0.5f);
}

float UFlightMovement::CalculateThrottleForTargetVelocity(float targetVelocity)
{
	if (targetVelocity >= NonExceedSpeed)
		return 1.0f;

	float speedRatio = (targetVelocity - IdleSpeed) / (NonExceedSpeed - IdleSpeed);
	float k = speedRatio * speedRatio * FMath::Sign(speedRatio);

	float denom = (1.0f - k) * EnginePower;
	if (FMath::IsNearlyZero(denom))
		return 1.0f;

	return ((k * IdleSpeed) / denom) + 0.2f;
}

void UFlightMovement::RotationCalculation(float deltaTime)
{
	const float minDelta = 0.01f;
	RotationDelta.X = FMath::Clamp(RotationDelta.X + (Control->Yoke.X * RotationSpeed.X /* * rotationSpeed.z*/ * 5.0f * deltaTime - RotationDelta.X /* * rotationSpeed.z * 0.5f*/ * 2.5f * deltaTime), -RotationSpeed.X, RotationSpeed.X);
	RotationDelta.Y = FMath::Clamp(RotationDelta.Y + (Control->Yoke.Y * RotationSpeed.Y /* * rotationSpeed.y*/ * 5.0f * deltaTime - RotationDelta.Y /* * rotationSpeed.y * 0.5f*/ * 2.5f * deltaTime), -RotationSpeed.Y, RotationSpeed.Y);
	RotationDelta.Z = FMath::Clamp(RotationDelta.Z + (Control->Yoke.Z * RotationSpeed.Z /* * rotationSpeed.x*/ * 5.0f * deltaTime - RotationDelta.Z /* * rotationSpeed.x * 0.5f*/ * 2.5f * deltaTime), -RotationSpeed.Z, RotationSpeed.Z);

	//if (rotationDelta.IsNearlyZero())
	//	rotationDelta = FVector3d::ZeroVector;

	if (FMath::Abs(RotationDelta.X) < minDelta) RotationDelta.X = 0.0f;
	if (FMath::Abs(RotationDelta.Y) < minDelta) RotationDelta.Y = 0.0f;
	if (FMath::Abs(RotationDelta.Z) < minDelta) RotationDelta.Z = 0.0f;

	FRotator rotator = FRotator(RotationDelta.Z * deltaTime * Control->GetHighGTurn(deltaTime), RotationDelta.Y * deltaTime, RotationDelta.X * deltaTime);
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
	EnginePower = specPointer->EnginePower;

	AirbrakePower = specPointer->AirbrakePower;
	RotationDelta = FVector::ZeroVector;
	DragFactor = specPointer->DragFactor;
	RotationSpeed = specPointer->RotationSpeed;
	//control->CalculateThrottle = FControl::FCalculateThrottle::CreateUObject(this, &UFlightMovement::CalculateThrottleForTargetVelocity);
	Control->CalculateThrottle.BindUObject(this, &UFlightMovement::CalculateThrottleForTargetVelocity);
}

void UFlightMovement::FollowWaypoint(float deltaTime)
{
	double leftDistance = ConvertFromVelocity(static_cast<double>(Control->Velocity)) * static_cast<double>(deltaTime);
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
