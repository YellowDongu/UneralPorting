// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPilot.h"
#include "VehicleBase.h"

//unsigned int AAIPilot::GlobalID = 0;
unsigned int AAIPilot::GlobalID{ 1 };

AAIPilot::AAIPilot()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.SetTickFunctionEnable(false);
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    ID = GlobalID++; // 네트워크 환경 내에선 서버에만 이 녀석이 존재한다고는 하는데 흠
}

void AAIPilot::BeginPlay()
{
    Super::BeginPlay();
}

void AAIPilot::PostInitProperties()
{
    Super::PostInitProperties();
    States.Add(AIState::State::END, &NullState);
	CurrentState = &NullState;
    NullState.Owner = this;
    currentState_debug = AIState::State::END;
}

void AAIPilot::PreInitializeComponents()
{
    Super::PreInitializeComponents();
}

void AAIPilot::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

void AAIPilot::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState->Update())
        ChangeCurrentState(CurrentState->GetNextState());
}

void AAIPilot::ChangeCurrentState(EAIState nextState)
{
    static int32 AntiRecursionCounter = 0;

    UE_LOG(LogTemp, Error, TEXT("%s // ChangeState : %s, NextChangeState : %s"), *Vehicle->GetName(), *StateString(currentState_debug), *StateString(nextState));
    TSharedPtr<AIState::DataSetterBlock> dataBlock = CurrentState->GetDataBlock();
    AIState** findedState = States.Find(nextState);

    while (findedState != nullptr)
    {
        if (AntiRecursionCounter >= 10)
        {
            UE_LOG(LogTemp, Error, TEXT("[AAIPilot::ChangeCurrentState] AntiRecursionCounter OverFlow"));
            CurrentState = &NullState;
            // need? add : end process
            break;
        }

        CurrentState = *findedState;
        currentState_debug = nextState;

        if ((*findedState)->Start(dataBlock))
            break;

        nextState = (*findedState)->GetNextState();
        dataBlock = (*findedState)->GetDataBlock();
        findedState = States.Find(nextState);
        UE_LOG(LogTemp, Error, TEXT("%s // StateStartFailed : %s, nextState : %s"), *Vehicle->GetName(), *StateString(currentState_debug), *StateString(nextState));
        AntiRecursionCounter++;
    }

    AntiRecursionCounter = 0;
}

void AAIPilot::Attach(AVehicleBase* vehicle)
{
    check(vehicle);
    Super::Possess(vehicle);
    SquadData = &vehicle->GetSquadData();
    SquadData->MyVehicle = Vehicle = vehicle;
    NullState.Owner = Vehicle;

    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);

}

void AAIPilot::Release()
{
    Vehicle->SquadDataReset();
    SquadData = nullptr;
    Vehicle.Reset();
    NullState.Owner = this;
}