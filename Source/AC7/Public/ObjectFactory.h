// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Defined.h"
#include "ObjectFactory.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UObjectFactory : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize(void) override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void PostInitProperties(void) override;
	virtual void PostInitialize(void) override;
	//virtual void PreInitializeComponents() override;
	//virtual void PostInitializeComponents() override;
	//virtual void PostLogin(APlayerController* NewPlayer) override;

	TWeakObjectPtr<class AVehicleBase> Instantiate(FSpawnInfomation& spawnInfomation) { TWeakObjectPtr<class AController> dummy; return Instantiate(*GetWorld(), spawnInfomation, dummy); }
	TWeakObjectPtr<class AVehicleBase> Instantiate(UWorld& world, FSpawnInfomation& spawnInfomation, TWeakObjectPtr<class AController>& output);
	TWeakObjectPtr<class AVehicleBase> Instantiate(UWorld& world, EVehicleID vehicleID);
	TWeakObjectPtr<class AAIPilot> Instantiate(UWorld* world, const FSpawnInfomation& infomation);

	TWeakObjectPtr<class AFlare> GetFlare(USceneComponent* flarePod);
	void Explosion(UWorld* world, const FVector& position, float Size = 50.0f, float lifeTime = 3.0f);
	TWeakObjectPtr<class ABullet> Instantiate(UWorld& world, const FVector& position, const FRotator& Rotation, float velocity);
	TObjectPtr<class AMissile> Instantiate(class AMissile* origin);

	void Release(class AVehicleBase* instance);
	void Release(class AMissile* instance);
	void Release(class AFlare* instance);
	void Release(class ABullet* instance);

	bool IsTGTEmpty(void) const { return TGTCount == 0; }
	int GetTGTCount(void) const { return TGTCount; }
	const TArray<TSet<TWeakObjectPtr<class AVehicleBase>>>& GetAllVehicle() const { return ActivedPool; }

private:
	class UBluePrintCollector* GetCollector();
	template<typename Type>
	inline Type* Get(TArray<TObjectPtr<Type>>& Pool)
	{
		Type* returnPointer = nullptr;
		while (returnPointer == nullptr && !Pool.IsEmpty())
			returnPointer = Pool.Pop(false);

		if (returnPointer != nullptr)
		{
			if constexpr (std::is_base_of_v<AController, Type>)
				returnPointer->PrimaryActorTick.SetTickFunctionEnable(false);
			if constexpr (std::is_base_of_v<AActor, Type>)
			{
				returnPointer->SetActorEnableCollision(true);
				returnPointer->SetActorHiddenInGame(false);
				returnPointer->SetActorTickEnabled(true);
			}
		}

		return returnPointer;
	}

	template<typename Type>
	inline void Release(TArray<TObjectPtr<Type>>& pool, Type* pointer)
	{
		//static_assert(std::is_base_of_v<AActor, Type>, "Only for UserWidget's function, do not use this function");
		if (pointer == nullptr)
			return;

		if constexpr (std::is_base_of_v<AController, Type>)
		{
			pointer->UnPossess();
			pointer->PrimaryActorTick.SetTickFunctionEnable(false);
		}
		if constexpr (std::is_base_of_v<AActor, Type>)
		{
			pointer->SetActorEnableCollision(false);
			pointer->SetActorHiddenInGame(true);
			pointer->SetActorTickEnabled(false);
		}

		pool.Add(pointer);
	}


	bool bIsPlayerSpawned{ false };
	int TGTCount{ 0 };

	UPROPERTY() class UBluePrintCollector* BluePrintCollector{ nullptr };
	UPROPERTY() TArray<TObjectPtr<class AFlare>> FlarePool;
	UPROPERTY() TArray<TObjectPtr<class ABullet>> BulletPool;
	UPROPERTY()	TObjectPtr<class UNiagaraSystem> ExplosionParticle;

	TArray<TObjectPtr<class AVehicleBase>> VehiclePool[static_cast<int>(EVehicleID::END)];
	TArray<TObjectPtr<class AController>> ControllerPool[4];
	TArray<TObjectPtr<class AMissile>> MissilePool[static_cast<int>(EWeaponID::END)];
	TArray<TSet<TWeakObjectPtr<class AVehicleBase>>> ActivedPool;
};
