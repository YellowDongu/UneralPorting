// Fill out your copyright notice in the Description page of Project Settings.


#include "BluePrintCollector.h"

#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraParameterStore.h"
#include "NiagaraFunctionLibrary.h"

#include "LevelLoader.h"
#include "Aircraft.h"
#include "AIPilot.h"
#include "PlayerPilot.h"
#include "Flare.h"
#include "Bullet.h"
#include "MovementPathAsset.h"


UBluePrintCollector::FVehicleInfo::FVehicleInfo(const TSubclassOf<class AVehicleBase>& vehicle)
{
	VehiclePtr = vehicle;
	VehiclePath = VehiclePtr->GetPathName();
	VehicleName = VehiclePtr->GetFName().ToString();
	AVehicleBase* BaseClass = vehicle->GetDefaultObject<AVehicleBase>();
	VehicleID = BaseClass->GetVehicleID();
	bIsLand = BaseClass->IsLand();
}

UBluePrintCollector::FLevelInfo::FLevelInfo(const TSoftObjectPtr<UWorld>& level)
{
	LevelPtr = level;
	LevelPath = LevelPtr.ToSoftObjectPath();
	LevelName = LevelPath.GetAssetName();
}


void UBluePrintCollector::PostInitProperties()
{
	Super::PostInitProperties();

	for (auto& item : Levels)
	{
		FLevelInfo levelInfo(item);
		LevelInfomation.Add(levelInfo.LevelName, levelInfo);
	}

	for (auto& item : Vehicles)
	{
		FVehicleInfo vehicleInfo(item);
		VehicleInfomation.Add(vehicleInfo.VehicleID, vehicleInfo);
	}

	SoundAssets = ULevelLoader::GetAllSoundAsset();
	ManeuverAssets = ULevelLoader::GetAllWaypointAsset();
}

TWeakObjectPtr<AVehicleBase> UBluePrintCollector::Create(EVehicleID ID, UWorld* world, bool& isAircraft)
{
	TWeakObjectPtr<AVehicleBase> newInstance;
	AAircraft* aircraft = nullptr;

	FVehicleInfo* infomation = VehicleInfomation.Find(ID);
	if (infomation == nullptr)
		return nullptr;

	isAircraft = !infomation->bIsLand;
	FActorSpawnParameters parameter;
	parameter.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	newInstance = world->SpawnActor<AVehicleBase>(infomation->VehiclePtr, FVector::ZeroVector, FRotator::ZeroRotator, parameter);
	if (newInstance != nullptr)
		newInstance->SystemIntegration();
    return newInstance;
}

TWeakObjectPtr<AAIPilot> UBluePrintCollector::CreateAIPilot(UWorld* world, const FSpawnInfomation& infomation)
{
	TWeakObjectPtr<AAIPilot> newInstance;

	if(infomation.VehicleID < EVehicleID::AirEND)
		newInstance = world->SpawnActor<AAIPilot>(AIAircraftPilotBluePrint);
	else if (infomation.VehicleID > EVehicleID::Ground && infomation.VehicleID < EVehicleID::GroundEND)
		newInstance = world->SpawnActor<AAIPilot>(AIDriverPilot);

	return newInstance;
}

TWeakObjectPtr<AFlare> UBluePrintCollector::CreateFlare(UWorld* world) { return world->SpawnActor<AFlare>(FlareBluePrint); }
TWeakObjectPtr<ABullet> UBluePrintCollector::CreateBullet(UWorld* world) { return world->SpawnActor<ABullet>(BulletBluePrint); }

TObjectPtr<class UNiagaraSystem> UBluePrintCollector::CreateExplosionSystem()
{
	return ExplosionBluePrint;
	//FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	//return Cast<UNiagaraSystem>(Streamable.LoadSynchronous(FSoftObjectPath(ExplosionBluePrint)));
}

FString UBluePrintCollector::GetMissionLevelName(int index)
{
	if (index < 0 || index >= Levels.Num())
		return FString();

	return Levels[index].ToSoftObjectPath().GetAssetName();
}

USoundBase* UBluePrintCollector::GetSound(const FString& name)
{
	auto result = Sounds.Find(name);
	if (result != nullptr)
		return result->Get();

	auto pointer = SoundAssets.Find(name);
	if (pointer == nullptr)
		return nullptr;

	return UAssetManager::GetStreamableManager().LoadSynchronous(*pointer);
}