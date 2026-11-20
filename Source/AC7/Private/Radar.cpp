// Fill out your copyright notice in the Description page of Project Settings.


#include "Radar.h"
//#include "Components/BoxComponent.h"
//#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameMaster.h"
#include "BluePrintCollector.h"
#include "Aircraft.h"
#include "ObjectFactory.h"
#include "Missile.h"
#include "Flare.h"
#include "IFFHandler.h"
#include "SoundSubsystem.h"

URadar::URadar()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URadar::BeginPlay()
{
	Super::BeginPlay();

	for (USceneComponent* Child : GetOwner()->GetRootComponent()->GetAttachChildren())
		if (Child->GetFName().ToString().Contains(TEXT("FlarePod")))
			flarePod.Add(Child);
}

void URadar::SystemIntegration(TWeakObjectPtr<class USphereComponent> collider, int _team, float _raderDistance)
{
	if (collider == nullptr)
		return;

	AAircraft* Owner = Cast<AAircraft>(GetOwner());
	if (Owner != nullptr)
		Owner->GetControl(Control);

	Collider = collider;
	Team = _team;
	RadarRange = _raderDistance;
	FlareRPM = 1.0f / (FlareRPM / 60.0f);

	//layerMask = LayerMask.GetMask("MovingVehicle");
	//targetLayer = LayerMask.NameToLayer("MovingVehicle");
	//raderDistance = GameMaster.ConvertWorldScale(raderDistance);
	//TSet<AActor*> OverlappingActors;
	//Collider->GetOverlappingActors(OverlappingActors, AVehicleBase::StaticClass());
	//
	//for (auto& item : OverlappingActors)
	//{
	//AVehicleBase* vehicle = Cast<AVehicleBase>(item);
	//
	//if (vehicle != nullptr)
	//{
	//	if (GetOwner() == vehicle)
	//		return;
	//	if (static_cast<AVehicleBase*>(GetOwner())->GetTeam() == vehicle->GetTeam()) return;
	//
	//	bool isAleadyIn;
	//	InDistance.Add(vehicle, &isAleadyIn);
	//	if (!isAleadyIn)
	//		AddTarget.Broadcast(vehicle);
	//
	//}
	//}

	Collider->OnComponentBeginOverlap.AddDynamic(this, &URadar::OnTriggerEnter);
	Collider->OnComponentEndOverlap.AddDynamic(this, &URadar::OnTriggerExit);
	//Collider->OnComponentHit.AddDynamic(this, &URadar::OnHit);
	Collider->SetSphereRadius(RadarRange);

	Audio = Cast<AVehicleBase>(GetOwner())->FindComponentByClass<UAudioComponent>();
	FlareFire = GetWorld()->GetGameInstance()->GetSubsystem<USoundSubsystem>()->GetSound(TEXT("FlareFire"));
}

void URadar::BindUI(UIFFHandler* handler)
{
	Collider->OnComponentBeginOverlap.AddDynamic(handler, &UIFFHandler::OnTriggerEnter);
	Collider->OnComponentEndOverlap.AddDynamic(handler, &UIFFHandler::OnTriggerExit);
}

void URadar::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UPrimitiveComponent* Comp = OtherComp;
	AActor* Actor = OtherActor;

	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);

	if (vehicle != nullptr)
	{
		if (GetOwner() == vehicle)
			return;

		if (!InRange.Contains(vehicle))
			InRange.Add(vehicle);
		if (static_cast<AVehicleBase*>(GetOwner())->GetTeam() == vehicle->GetTeam()) return;

		bool isAleadyIn;
		InDistance.Add(vehicle, &isAleadyIn);
		if (!isAleadyIn)
		{
			AddTarget.Broadcast(vehicle);
			UE_LOG(LogTemp, Error, TEXT("[HoldState::Update] Spotted %s -> %s"), *GetOwner()->GetName(), *OtherActor->GetName());
		}
	}
}

void URadar::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);

	InRange.Remove(vehicle);

	if (vehicle != nullptr && InDistance.Contains(vehicle))
	{
		InDistance.Remove(vehicle);
		RemoveTarget.Broadcast(vehicle);
	}
}

void URadar::DeployFlare()
{
	if (flarePod.IsEmpty())
		return;

	UObjectFactory* objectFactory = GetWorld()->GetSubsystem<UObjectFactory>();
	AVehicleBase* owner = StaticCast<AVehicleBase*>(GetOwner());
	float velocity = Control != nullptr ? Control->Velocity : 0.0f;

	int TotalCount = Tracing.Num();
	int SubFunctionCount = flarePod.Num();
	int TotalCalls = DeployAmount * SubFunctionCount;
	int BaseCount = TotalCount / TotalCalls;
	int Remainder = TotalCount % TotalCalls;

	check(objectFactory);
	check(owner);

	auto iterator = Tracing.CreateConstIterator();
	TArray<TWeakObjectPtr<AMissile>> list;

	for (int i = 0; i < SubFunctionCount; ++i)
	{
		const int ObjectCount = BaseCount + (i < Remainder ? 1 : 0);

		for (int j = 0; j < ObjectCount; ++j)
		{
			if (!iterator)
				break;

			list.Add(*iterator);
			++iterator;
		}
	}

	BaseCount = list.Num() / SubFunctionCount;
	Remainder = list.Num() % SubFunctionCount;
	int StartIndex = 0;

	for (int i = 0; i < SubFunctionCount; ++i)
	{
		auto& item = flarePod[i];
		TWeakObjectPtr<AFlare> newInstance = objectFactory->GetFlare(owner, item.Get(), velocity);
		const int Length = BaseCount + (i < Remainder ? 1 : 0);

		for (int j = 0; j < Length; ++j)
		{
			if (!list[StartIndex + j].IsValid()) continue;
			list[StartIndex + j]->ForceChangeTarget(newInstance);
		}

		StartIndex += Length;
	}

	UGameplayStatics::PlaySoundAtLocation(this, FlareFire, GetOwner()->GetActorLocation());

	DeployAmount--;
	if (DeployAmount <= 0)
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	/*
	DeployAmount--;

	int TargetNum = Tracing.Num();
	if (TargetNum > 4) TargetNum /= 2;

	TArray<AMissile> untraceTarget;
	untraceTarget.Reserve(TargetNum);

	int SubTargetNum = TargetNum / flarePod.Num();
	int length = 0, i = 0;


	for (auto& item : flarePod)
	{
		TWeakObjectPtr<AFlare> newInstance = objectFactory->GetFlare(owner, item.Get(), velocity);
		length = FMath::Min(i + SubTargetNum, Tracing.Num());
		for (auto& missile : Tracing)
		{
			if (i++ >= length) break;
	
			missile->ForceChangeTarget(newInstance);
		}
	}
	//UGameplayStatics::PlaySound2D(this, FlareFire);
	UGameplayStatics::PlaySoundAtLocation(this, FlareFire, GetOwner()->GetActorLocation());

	if (DeployAmount < 0)
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		*/
}

void URadar::StartDeployFlare()
{
	if (TimerHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[URadar::StartDeployFlare] Flare Deploying : %s"), *GetOwner()->GetName());
	}
	else if (FlareCount <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[URadar::StartDeployFlare] Flare CoolTime : %s"), *GetOwner()->GetName());
	}


	if (TimerHandle.IsValid() || FlareCount <= 0)
		return;

	FTimerHandle dummy;
	FlareCount--;
	DeployAmount = FlareDeployCount;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &URadar::DeployFlare, FlareRPM, true);
	GetWorld()->GetTimerManager().SetTimer(dummy, [this]() -> void { FlareCount++; }, FlareCoolTime, false);
}

void URadar::Remove(TArray<AVehicleBase*>& targets)
{
	int length = targets.Num();
	for (int i = 0; i < length; i++)
		InDistance.Remove(targets[i]);
}

void URadar::CallWarning(double squareDistance)
{
	static constexpr double alartDistance{ ConvertToDistance(1000.0) * ConvertToDistance(1000.0) };

	if (squareDistance <= alartDistance)
		bAlart = true;

	//UE_LOG(LogTemp, Warning, TEXT("[URadar::CallWarning] %s to %s"), bAlart ? TEXT("Alart") : TEXT("Warning"), *GetOwner()->GetName());
	bWarning = true;
}
