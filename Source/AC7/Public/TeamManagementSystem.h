// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Defined.h"
#include "TeamManagementSystem.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UTeamManagementSystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	struct SquadSystem
	{
		TWeakObjectPtr<class AVehicleBase> Leader{ nullptr };
		TMap<TWeakObjectPtr<class AVehicleBase>, TeamData*> TeamList;
	};

	UTeamManagementSystem();

	//virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//virtual void Deinitialize() override;
	//virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	//virtual void PostInitProperties() override;
	//virtual void PostInitialize() override;

	void TeamAdd(TeamData& target, const TWeakObjectPtr<AVehicleBase>& Leader = nullptr);
	void TeamRemove(TeamData& target);
	bool LeaderCheck(TeamData& team);
	const SquadSystem* GetSquad(const TeamData& team) const
	{
		if (team.Leader == nullptr)
			return nullptr;
		auto pointer = SquadList.Find(team.Leader);
		if (pointer == nullptr)
			return nullptr;
		return pointer->Get();
	};

private:
	TUniquePtr<SquadSystem>* GetSquad(const TWeakObjectPtr<class AVehicleBase>& teamLeader)
	{
		for (auto& item : SquadList)
		{
			if (*item.Key != teamLeader)
				continue;

			return &item.Value;
		}

		return nullptr;
	};
	

	TMap<TWeakObjectPtr<class AVehicleBase>*, TUniquePtr<SquadSystem>> SquadList;
	//TArray<SquadSystem> SquadList[3];
};
