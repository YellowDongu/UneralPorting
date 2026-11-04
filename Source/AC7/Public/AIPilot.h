// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Defined.h"
#include "AIPilot.generated.h"

/**
 * 
 */

// 위치 이전할 것

class AC7_API AIState
{
public:
	AIState() = default;
	virtual ~AIState() = default;

	enum class State
	{
		Chase,
		Follow,
		Search,
		Combat,
		Override_Altitude,
		Override_Defend,
		Override,
		AircraftStateEnd,
		END
	};

	struct DataSetterBlock { };
	struct OverridedDataBlock : DataSetterBlock
	{
		State PriviousState{ State::END };
		TSharedPtr<DataSetterBlock> PreviousData{ nullptr };
	};
	struct DefaultDataBlock { };

	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) { return true; }
	virtual bool Update(void) { return true; }
	virtual void Release(void) { }
	inline float ClampRange(float value) { return FMath::Clamp(value, -1.0f, 1.0f); }// -11clamp

	virtual void SetDefaultData(DefaultDataBlock* data) = 0;
	virtual TSharedPtr<OverridedDataBlock> SetOverrideData(void) { return nullptr; }
	
	//State GetState(void) { return MyState; }
	State GetNextState(void) { State Output = NextState; NextState = State::END; return Output; }
	TSharedPtr<DataSetterBlock> GetDataBlock() { TSharedPtr<DataSetterBlock> output = DataToSend; DataToSend.Reset(); return output; }

protected:
	//State MyState{ State::END };
	State NextState{ State::END };
	TSharedPtr<DataSetterBlock> DataToSend{ nullptr };
};
typedef AIState::State EAIState;

class AINullState : public AIState
{
public:
	virtual bool Start(const TSharedPtr<DataSetterBlock>& data = nullptr) { return true; }
	virtual bool Update() { UE_LOG(LogTemp, Error, TEXT("%s AISTATE ERROR"), *Owner->GetName()); return false; }

	virtual void SetDefaultData(DefaultDataBlock* data) {}

	TWeakObjectPtr<class AActor> Owner;
};

UCLASS()
class AC7_API AAIPilot : public AAIController
{
	GENERATED_BODY()

public:
	AAIPilot();

	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual void Release();

	void ChangeCurrentState(EAIState state);
	void ChangeCurrentState() { ChangeCurrentState(CurrentState->GetNextState()); }
	virtual void Attach(class AVehicleBase* vehicle);

protected:
	TWeakObjectPtr<class AVehicleBase> Vehicle{ nullptr };

	AIState* CurrentState{ nullptr };
	TeamData* SquadData{ nullptr };

	AINullState NullState;
	EAIState currentState_debug;
	TMap<EAIState, AIState*> States;


	unsigned int ID{ 0 };
	static unsigned int GlobalID;
};

inline FString StateString(EAIState nextState)
{
	switch (nextState)
	{
	case EAIState::Chase:              return TEXT("Chase");
	case EAIState::Follow:             return TEXT("Follow");
	case EAIState::Search:             return TEXT("Search");
	case EAIState::Combat:             return TEXT("Combat");
	case EAIState::Override_Altitude:  return TEXT("Override_Altitude");
	case EAIState::Override_Defend:    return TEXT("Override_Defend");
	case EAIState::Override:           return TEXT("Override");
	case EAIState::AircraftStateEnd:   return TEXT("AircraftStateEnd");
	case EAIState::END:                return TEXT("END");
	default:                            return TEXT("Unknown");
	}
}
