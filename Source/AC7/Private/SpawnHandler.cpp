// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnHandler.h"
#include "SpawnTrigger.h"
#include "MissionMode.h"
void USpawnHandler::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USpawnHandler::Deinitialize()
{
	Super::Deinitialize();
}

void USpawnHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (auto iterator = ActiveList.CreateIterator(); iterator; iterator++)
	{
		if (!iterator->IsValid())
		{
			iterator.RemoveCurrent();
			continue;
		}
		if (!(*iterator)->Update(0.0f))
			continue;

		UE_LOG(LogTemp, Error, TEXT("[ASpawnTrigger::ActiveNext] %s Started"), *(*iterator)->GetName());

		(*iterator)->Active(GetWorld());

		for (auto& item : (*iterator)->GetNextTriggers())
		{
			StandbyList.Remove(item);
			ActiveList.Add(item);
			UE_LOG(LogTemp, Error, TEXT("[ASpawnTrigger::ActiveNext] %s Actived"), *item->GetName());
		}

		iterator.RemoveCurrent();
	}
}

void USpawnHandler::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	int length = StandbyList.Num();
	TArray<int> list;

	TMap<TWeakObjectPtr<ASpawnTrigger>, int> map;
	list.SetNum(length, false);

	for (int i = 0; i < length; i++)
		map.Add(StandbyList[i], i);

	for (int i = 0; i < length; i++)
	{
		for (auto& item : StandbyList[i]->GetNextTriggers())
		{
			int* index = map.Find(item);
			if (index == nullptr)
				continue;
			list[*index]++;
		}
	}

	for (int i = 0; i < length; i++)
		if (list[i] == 0)
			ActiveList.Add(StandbyList[i]);

	for (auto& item : ActiveList)
	{
		UE_LOG(LogTemp, Error, TEXT("[ASpawnTrigger::OnWorldBeginPlay] %s Started"), *item->GetActorNameOrLabel());

		StandbyList.Remove(item);
	}

	for (auto iterator = ActiveList.CreateIterator(); iterator; iterator++)
	{
		if (!iterator->IsValid())
		{
			iterator.RemoveCurrent();
			continue;
		}

		if (!(*iterator)->Update(0.0f))
			continue;


		(*iterator)->Active(GetWorld());

		for (auto& item : (*iterator)->GetNextTriggers())
		{
			StandbyList.Remove(item);
			ActiveList.Add(item);
			UE_LOG(LogTemp, Error, TEXT("[ASpawnTrigger::OnWorldBeginPlay] %s Actived"), *item->GetName());
		}

		iterator.RemoveCurrent();
	}
}

void USpawnHandler::Enlist(ASpawnTrigger* trigger)
{
	//UE_LOG(LogTemp, Error, TEXT("[USpawnHandler::Enlist] Trigger Enlisted : %s"), *trigger->GetName());
	StandbyList.Add(trigger);
}
