// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectFactory.h"
#include "Aircraft.h"
#include "BluePrintCollector.h"
#include "PlayerPilot.h"
#include "AIPilot.h"
#include "GameMaster.h"
#include "Missile.h"
#include "MissionMode.h"
#include "Flare.h"
#include "Bullet.h"
#include "AIAircraftPilot.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraParameterStore.h"
#include "NiagaraFunctionLibrary.h"

void UObjectFactory::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ActivedPool.SetNum(5);
}

void UObjectFactory::Deinitialize()
{
	Super::Deinitialize();
}

void UObjectFactory::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	BluePrintCollector = InWorld.GetGameInstance<UGameMaster>()->GetCollector();
	ExplosionParticle = BluePrintCollector->CreateExplosionSystem();
}

void UObjectFactory::PostInitProperties()
{
	Super::PostInitProperties();
}

void UObjectFactory::PostInitialize()
{
	Super::PostInitialize();
}

UBluePrintCollector* UObjectFactory::GetCollector()
{
	if (BluePrintCollector == nullptr)
		BluePrintCollector = GetWorld()->GetGameInstance<UGameMaster>()->GetCollector();

	return BluePrintCollector;
}

TWeakObjectPtr<AFlare> UObjectFactory::GetFlare(USceneComponent* flarePod)
{
	TWeakObjectPtr<AFlare> newInstance = Get<AFlare>(FlarePool);

	if (newInstance == nullptr)
		newInstance = GetCollector()->CreateFlare(GetWorld());
	else
	{
		newInstance->SetActorEnableCollision(true);
		newInstance->SetActorHiddenInGame(false);
		newInstance->SetActorTickEnabled(true);
	}

	check(newInstance.Get());
	check(flarePod);

	newInstance->Shoot(flarePod);
	return newInstance;
}

void UObjectFactory::Explosion(UWorld* world, const FVector& position, float Size, float lifeTime)
{
	auto newParticle = UNiagaraFunctionLibrary::SpawnSystemAtLocation(world, ExplosionParticle, position, FRotator::ZeroRotator, FVector::OneVector, true, true, ENCPoolMethod::AutoRelease);
	newParticle->SetVariableFloat(TEXT("User.Lifetime"), lifeTime);
	newParticle->SetVariableFloat(TEXT("User.Size"), Size);
}

TWeakObjectPtr<AVehicleBase> UObjectFactory::Instantiate(UWorld& world, FSpawnInfomation& spawnInfomation, TWeakObjectPtr<AController>& output)
{
	output = nullptr;
	TWeakObjectPtr<AVehicleBase> newInstance = nullptr;

	if (AMissionMode::GetSpectatorMode())
		spawnInfomation.bIsPlayer = false;

	if (bIsPlayerSpawned == spawnInfomation.bIsPlayer)
		spawnInfomation.bIsPlayer = false;
	if (!ActivedPool.IsValidIndex(spawnInfomation.TeamIndex))
		spawnInfomation.TeamIndex = 0;

	if (spawnInfomation.bIsPlayer)
	{
		spawnInfomation.playerInfoPointer = &GetWorld()->GetGameInstance<UGameMaster>()->GetPlayerInfomation();

		newInstance = Instantiate(world, spawnInfomation.playerInfoPointer->VehicleID);
		if (newInstance == nullptr) return nullptr;

		newInstance->SystemInitialize(spawnInfomation);
		APlayerPilot* player = Cast<APlayerPilot>(world.GetFirstPlayerController());
		if (player == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[UObjectFactory::Instantiate] %s"), world.GetFirstPlayerController() == nullptr ? TEXT("No Controller") : *(FString(TEXT("Wrong Controller : ") + world.GetFirstPlayerController()->GetClass()->GetName())));
			Release(newInstance.Get());
			return nullptr;
		}

		player->Attach(static_cast<AAircraft*>(newInstance.Get()));
		output = world.GetFirstPlayerController();

		ActivedPool[spawnInfomation.TeamIndex].Add(newInstance);
		bIsPlayerSpawned = true;
	}
	else
	{
		newInstance = Instantiate(world, spawnInfomation.VehicleID);
		if (newInstance == nullptr) return nullptr;
		newInstance->SystemInitialize(spawnInfomation);

		TWeakObjectPtr<AAIPilot> aiPilot = Instantiate(&world, spawnInfomation);
		if (aiPilot == nullptr)
		{
			Release(newInstance.Get());
			return nullptr;
		}

		aiPilot->Attach(newInstance.Get());
		output = aiPilot;

		ActivedPool[spawnInfomation.TeamIndex].Add(newInstance);
		if (spawnInfomation.IsTGT)
			TGTCount++;
	}

	return newInstance;
}

TWeakObjectPtr<AVehicleBase> UObjectFactory::Instantiate(UWorld& world, EVehicleID vehicleID)
{
	bool isAir = static_cast<int>(vehicleID) < static_cast<int>(EVehicleID::Ground);
	TWeakObjectPtr<AVehicleBase> newInstance = Get(VehiclePool[static_cast<int>(vehicleID)]);

	if (newInstance == nullptr)
	{
		newInstance = GetCollector()->Create(vehicleID, &world, isAir);
		if (newInstance == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[UObjectFactory::Instantiate] Create Failed"));
			return nullptr;
		}
	}

	return newInstance;
}

TObjectPtr<AMissile> UObjectFactory::Instantiate(AMissile* origin)
{
	int index = static_cast<int>(origin->GetWeaponID());
	if (index < 0 && index >= static_cast<int>(EWeaponID::END))
		return nullptr;

	TObjectPtr<AMissile> newInstance = Get<AMissile>(MissilePool[index]);

	if (newInstance == nullptr)
	{
		FActorSpawnParameters Params;
		newInstance = GetWorld()->SpawnActor<AMissile>(origin->GetClass(), origin->GetActorTransform(), Params);
		if (newInstance == nullptr)
			return nullptr;
		newInstance->PrimaryActorTick.bStartWithTickEnabled = false;
	}

	newInstance->SetActorEnableCollision(true);
	newInstance->SetActorHiddenInGame(false);
	newInstance->SetActorTickEnabled(true);

	return newInstance;
}

TWeakObjectPtr<ABullet> UObjectFactory::Instantiate(UWorld& world, const FVector& position, const FRotator& Rotation, float velocity)
{
	TWeakObjectPtr<ABullet> newInstance = Get<ABullet>(BulletPool);

	if (newInstance == nullptr)
	{
		newInstance = GetCollector()->CreateBullet(&world);
		if (newInstance == nullptr)
			return nullptr;
		newInstance->Initialize();
	}
	newInstance->Shoot(position, Rotation, velocity);
	return newInstance;
}

TWeakObjectPtr<AAIPilot> UObjectFactory::Instantiate(UWorld* world, const FSpawnInfomation& infoData)
{
	int i = 0;

	if (infoData.VehicleID < EVehicleID::AirEND)
		i = 0;
	else if (infoData.VehicleID > EVehicleID::Ground && infoData.VehicleID < EVehicleID::GroundEND)
		i = 1;

	TWeakObjectPtr<AAIPilot> newInstance = StaticCast<AAIPilot*>(Get<AController>(ControllerPool[i]));

	if (newInstance == nullptr)
		newInstance = GetCollector()->CreateAIPilot(world, infoData);

	return newInstance;
}

void UObjectFactory::Release(AVehicleBase* instance)
{
	int i = 0;
	AAIPilot* controller = Cast<AAIPilot>(instance->GetController());
	if (controller != nullptr)
	{
		controller->Release();
		if (Cast<AAIAircraftPilot>(controller) == nullptr)
			i = 1;
	}

	if (instance->IsTGT())
		TGTCount--;

	for (auto& list : ActivedPool)
		if (list.Remove(instance) > 0)
			break;

	if (Cast<APlayerPilot>(instance->GetController()) != nullptr)
		bIsPlayerSpawned = false;

	Release(ControllerPool[i], instance->GetController());
	Release(VehiclePool[static_cast<int>(instance->GetVehicleID())], instance);
}

void UObjectFactory::Release(AMissile* instance) { Release(MissilePool[static_cast<int>(instance->GetWeaponID())], instance); }
void UObjectFactory::Release(AFlare* instance) { Release(FlarePool, instance); }
void UObjectFactory::Release(ABullet* instance) { Release(BulletPool, instance); }
