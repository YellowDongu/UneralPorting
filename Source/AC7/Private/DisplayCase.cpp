// Fill out your copyright notice in the Description page of Project Settings.


#include "DisplayCase.h"
#include "GameMaster.h"
#include "BlueprintCollector.h"
#include "Aircraft.h"
#include "AircraftAnimator.h"

void ADisplayCase::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
}

void ADisplayCase::Initialize()
{
	if (bInitialized)
		return;

	Vehicles.Empty();
	Vehicles.Reserve(static_cast<int>(EVehicleID::AirEND) - static_cast<int>(EVehicleID::Aircraft));
	UBluePrintCollector* collector = GetGameInstance<UGameMaster>()->GetCollector();
	UWorld* world = GetWorld();	
	for (unsigned int id = static_cast<unsigned int>(EVehicleID::Aircraft) + 1; id < static_cast<unsigned int>(EVehicleID::AirEND); id++)
		CreateAircraft(static_cast<EVehicleID>(id), collector, world);

	CurrentDisplay = nullptr;
	ChangeDisplay(static_cast<EVehicleID>(1));
	bInitialized = true;
}

void ADisplayCase::ChangeDisplay(EVehicleID id)
{
	if (id >= EVehicleID::AirEND) return;

	if (CurrentDisplay != nullptr)
	{
		if (id == CurrentDisplay->GetVehicleID())
			return;

		CurrentDisplay->SetActorHiddenInGame(true);
		//CurrentDisplay->SetActorEnableCollision(false);
		//CurrentDisplay->SetActorTickEnabled(false);
	}

	if (Vehicles[static_cast<int>(id) - 1] != nullptr)
		CurrentDisplay = Vehicles[static_cast<int>(id) - 1].Get();

	CurrentDisplay->SetActorHiddenInGame(false);
}

void ADisplayCase::CreateAircraft(EVehicleID id, UBluePrintCollector* collector, UWorld* world)
{
	static float scaleSize = 2.5f;
	bool checker;
	TWeakObjectPtr<AVehicleBase> result = collector->Create(static_cast<EVehicleID>(id), world, checker);

	if (result == nullptr)
		return;

	result->SetActorHiddenInGame(true);
	result->SetActorEnableCollision(false);
	result->SetActorTickEnabled(false);
	result->SetActorScale3D(FVector::OneVector * scaleSize);

	AAircraft* aircraft = Cast<AAircraft>(result);
	if (aircraft == nullptr)
	{
		result->Destroy();
		return;
	}

	FVector center = aircraft->GetAircraftAnimator()->GetDisplayCenterPosition();
	result->SetActorLocation(center * -scaleSize);
	Vehicles.Add(result);
}
