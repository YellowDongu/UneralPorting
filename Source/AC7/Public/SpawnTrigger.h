// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Defined.h"

#include "SpawnTrigger.generated.h"


class AC7_API USpawnCondition
{
public:
	USpawnCondition() = default;
	virtual ~USpawnCondition() = default;
	inline virtual bool Update(float deltaTime) { UE_LOG(LogTemp, Error, TEXT("[USpawnCondition::Update] No Condition")); return true; }
};
	
class AC7_API UTimeCondition : public USpawnCondition
{
public:
	UTimeCondition() = default;
	virtual ~UTimeCondition() = default;

	inline virtual bool Update(float deltaTime) override { if (!bIsStarted) { StartTimer(); return false; } return !Handle.IsValid(); }
	void SetTime(float value, FTimerManager& timeManager) { Time = value; Timer = &timeManager; }
	bool StartTimer(void)
	{
		if (bIsStarted)
			return false;

		UE_LOG(LogTemp, Error, TEXT("[UTimeCondition::StartTimer] ActiveTimer"));
		bIsStarted = true;
		Timer->SetTimer(Handle, [this]()-> void
		{
			UE_LOG(LogTemp, Error, TEXT("[UTimeCondition::Update] Condition Achived"));
			bIsStarted = false;
			Handle.Invalidate();
		}, Time, false);

		return true;
	}

private:
	bool bIsStarted{ false };
	FTimerManager* Timer{ nullptr };
	FTimerHandle Handle;
	float Time{ 0 };
};

class AC7_API UTGTCondition : public USpawnCondition
{
public:
	UTGTCondition() = default;
	virtual ~UTGTCondition() = default;

	virtual bool Update(float deltaTime) override;
	void Initialize(UWorld* world);

private:
	class UObjectFactory* Factory{nullptr};
};

class AC7_API UKillCondition : public USpawnCondition
{
public:
	UKillCondition() = default;
	virtual ~UKillCondition() = default;
	//inline virtual bool Update(float deltaTime) override { return true; }
private:
};

class AC7_API UTGTKillEndCondition : public USpawnCondition
{
public:
	UTGTKillEndCondition() = default;
	virtual ~UTGTKillEndCondition() = default;
	virtual bool Update(float deltaTime) override;

	void Initialize(UWorld* world, const TArray<TWeakObjectPtr<class ASpawnTrigger>>* activeList, const TArray<TWeakObjectPtr<class ASpawnTrigger>>* standbyList);
private:
	class UObjectFactory* Factory{ nullptr };
	const TArray<TWeakObjectPtr<class ASpawnTrigger>>* ActiveList{ nullptr };
	const TArray<TWeakObjectPtr<class ASpawnTrigger>>* StandbyList{ nullptr };
};

//class AC7_API UTimeEndCondition : public USpawnCondition
//{
//public:
//	UTGTKillEndCondition() = default;
//	virtual ~UTGTKillEndCondition() = default;
//	virtual bool Update(float deltaTime) override;
//
//	void Initialize(UWorld* world, const TArray<TWeakObjectPtr<class ASpawnTrigger>>* activeList, const TArray<TWeakObjectPtr<class ASpawnTrigger>>* standbyList);
//private:
//	bool bEnd{ false };
//	class UObjectFactory* Factory{ nullptr };
//	const TArray<TWeakObjectPtr<class ASpawnTrigger>>* ActiveList{ nullptr };
//	const TArray<TWeakObjectPtr<class ASpawnTrigger>>* StandbyList{ nullptr };
//};


UENUM()
enum class ESpawnConditionType
{
	Time,
	LocalTime,
	TGTCleared,
	AllCleared,
	Immidiate,
	UTGTKillEnd,
	END
};

UCLASS()
class AC7_API ASpawnTrigger : public AActor
{
	GENERATED_BODY()
public:
	ASpawnTrigger();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents(void) override;
	//virtual void BeginPlay(void) override;
	
	bool Update(float DeltaTime);
	void Active(UWorld* world);

	const TArray<TObjectPtr<ASpawnTrigger>>& GetNextTriggers(void) { return NextTriggers; }
	ESpawnConditionType GetSpawnCondition(void) { return SpawnCondition; }
	bool IsActiveReady(void){ return bActiveReady; }
	bool IsActived(void){ return bIsActive; }
private:
#if WITH_EDITOR
	USkeletalMesh* GetModel(UWorld* world, EVehicleID id);
#endif

	TSharedPtr<USpawnCondition> CreateSpawnCondition();
	void CreateInstance(int index, class UObjectFactory* objectFactory, class UTeamManagementSystem* teamSystem, TArray<TWeakObjectPtr<class AVehicleBase>>& instanceList);
	void UnionFind();
	int UnionFindParentFind(int index, TArray<int>& parent);

	bool bActiveReady{ false }, bIsActive{ false };
	TSharedPtr<USpawnCondition> ConditionChecker;

	UPROPERTY(EditAnywhere, Category = "Conditions", meta = (AllowPrivateAccess = "true"))
	bool bStartWithTickEnabled{ false };

	UPROPERTY(EditAnywhere, Category = "Conditions", meta = (AllowPrivateAccess = "true"))
	ESpawnConditionType SpawnCondition{ ESpawnConditionType::END };

	UPROPERTY(EditAnywhere, Category = "Conditions", meta = (AllowPrivateAccess = "true", EditCondition = "SpawnCondition == ESpawnConditionType::Time", EditConditionHides))
	float Timer{ 0.0f };

	UPROPERTY(EditAnywhere, Category = "ActiveBehavior", meta = (AllowPrivateAccess = "true"))
	TArray<FSpawnInfomation> list;
	UPROPERTY(EditAnywhere, Category = "ActiveBehavior", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<ASpawnTrigger>> NextTriggers;
	
	UPROPERTY(EditAnywhere, Category = "DEBUG", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<USkeletalMesh> PreviewMesh;
	TArray<TWeakObjectPtr<USkeletalMeshComponent>> ActiveMesh;

	UPROPERTY(EditAnywhere, Category = "DEBUG", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UBluePrintCollector> bp;
};
