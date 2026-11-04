// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamManagementSystem.h"
#include "VehicleBase.h"

UTeamManagementSystem::UTeamManagementSystem()
{
	//bShowMouseCursor = true;
	//bEnableClickEvents = true;
	//bEnableMouseOverEvents = true;

}

void UTeamManagementSystem::TeamAdd(TeamData& target, const TWeakObjectPtr<AVehicleBase>& Leader)
{
	if (Leader == nullptr)
	{
		auto pointer = MakeUnique<SquadSystem>();
		pointer->Leader = target.MyVehicle;
		target.Leader = &pointer->Leader;
		pointer->TeamList.Add(target.MyVehicle, &target);
		SquadList.Add(&pointer->Leader, MoveTemp(pointer));
		return;
	}

	TUniquePtr<SquadSystem>* squad = GetSquad(Leader);
	if (squad == nullptr)
	{
		TeamAdd(Leader->GetSquadData());
		squad = GetSquad(Leader);

		// Leader가 nullptr인 채로 남는 것은 비정상 생성
		if (!ensureMsgf(squad != nullptr, TEXT("[UTeamManagementSystem::TeamAdd] Failed to create squad for leader")))
			return;
	}

	int index = (*squad)->TeamList.Num();
	target.Leader = &(*squad)->Leader;
	(*squad)->TeamList.Add(target.MyVehicle, &target);
	target.ChangeSquadNumber(index);
}

void UTeamManagementSystem::TeamRemove(TeamData& target)
{
	if (target.Leader == nullptr)
		return;
	
	TUniquePtr<SquadSystem>* pointer = SquadList.Find(target.Leader);
	if (pointer == nullptr)
		return;
	TUniquePtr<SquadSystem>& squad = *pointer;
	bool isLeader = target.IsLeader();
	squad->TeamList.Remove(target.MyVehicle);

	if (squad->TeamList.IsEmpty())
	{
		SquadList.Remove(&squad->Leader);	// squad는 여기서 해제됨, 이후 접근 금지
		target.Reset();
		return;
	}

	if(isLeader)
		squad->Leader = squad->TeamList.begin()->Key;

	int i = 0;
	for (auto iterator = squad->TeamList.CreateIterator(); iterator; ++iterator)
		iterator->Value->ChangeSquadNumber(i++);

	target.Reset();
}

#define LOG(Text, ...) UE_LOG(LogTemp, Error, Text, ##__VA_ARGS__)
bool UTeamManagementSystem::LeaderCheck(TeamData& team)
{
	// Reset 되었거나 스쿼드에 속하지 않은 상태
	if (team.Leader == nullptr)
		return false;

	if (team.Leader->IsValid())
		return true;

	TUniquePtr<SquadSystem>* pointer = SquadList.Find(team.Leader);
	if (pointer == nullptr)
	{
		LOG(TEXT("[UTeamManagementSystem::LeaderCheck] squad == nullptr"));
		return false;
	}

	TUniquePtr<SquadSystem>& squad = *pointer;
	if (squad->TeamList.IsEmpty())
	{
		LOG(TEXT("[UTeamManagementSystem::LeaderCheck] squad->TeamList.IsEmpty()"));
		SquadList.Remove(&squad->Leader);
		team.Reset();
		return false;
	}

	for (auto iterator = squad->TeamList.CreateIterator(); iterator; ++iterator)
	{
		if (!iterator->Key.IsValid())
		{
			iterator.RemoveCurrent();
			continue;
		}

		if (iterator->Value == nullptr)
			iterator->Value = &iterator->Key->GetSquadData();
	}

	if (squad->TeamList.IsEmpty())
	{
		LOG(TEXT("[UTeamManagementSystem::LeaderCheck] squad->TeamList.IsEmpty()"));
		SquadList.Remove(&squad->Leader);
		team.Reset();
		return false;
	}

	int i = 0;
	squad->Leader = squad->TeamList.begin()->Key;
	for (auto iterator = squad->TeamList.CreateIterator(); iterator; ++iterator)
		iterator->Value->ChangeSquadNumber(i++);

	return true;
}
