// Fill out your copyright notice in the Description page of Project Settings.


#include "AIAircraftPilot.h"
#include "Aircraft.h"
#include "FlightMovement.h"
#include "FireControlSystem.h"
#include "AIAircraftState.h"

typedef AIState::State EAIState;
void AAIAircraftPilot::PostInitProperties()
{
	Super::PostInitProperties();

    States.Add(EAIState::Chase, &Chase);
    States.Add(EAIState::Override_Altitude, &AltitudeSetter);
    States.Add(EAIState::Override_Defend, &Defend);
    States.Add(EAIState::Search, &Search);
    States.Add(EAIState::Follow, &InFomation);
    States.Add(EAIState::Combat, &Combat);

    Defend.AddAviliableManeuver(TEXT("ImmelmannTurnRight"));
    Defend.AddAviliableManeuver(TEXT("ImmelmannTurnLeft"));
    Defend.AddAviliableManeuver(TEXT("HorizontalTurn"));
    Defend.AddAviliableManeuver(TEXT("VerticalTurn"));
}

void AAIAircraftPilot::BeginPlay()
{
    Super::BeginPlay();
}

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

void AAIAircraftPilot::Attach(AVehicleBase* vehicle)
{
	AAircraft* aircraft = Cast<AAircraft>(vehicle);
    check(aircraft);

    AAIPilot::Attach(vehicle);

    aircraft->GetControl(Control);
    aircraft->SetGear(false, true);
    {
        bool debug = true;
        if (debug)
        {
            TWeakObjectPtr<UCameraComponent> camera;
            aircraft->AttachCamera(camera);
        }
    }

	AIAircraftState::AircraftDefaultData defaultData;
	defaultData.Control = Control;
	defaultData.Owner = aircraft;
    defaultData.SquadData = SquadData;

	Combat.SetDefaultData(&defaultData);
    Chase.SetDefaultData(&defaultData);
    AltitudeSetter.SetDefaultData(&defaultData);
    Search.SetDefaultData(&defaultData);
    InFomation.SetDefaultData(&defaultData);
    Defend.SetDefaultData(&defaultData);


    currentState_debug = SquadData->IsLeader() ? AIState::State::Search : AIState::State::Follow;
    UE_LOG(LogTemp, Error, TEXT("[AAIAircraftPilot::BeginPlay] %s// Start AAIAircraftPilot : %s"), *Vehicle->GetName(), *StateString(currentState_debug));

    AIState** findedState = States.Find(currentState_debug);
    if (findedState != nullptr && *findedState != nullptr)
    {
        CurrentState = *findedState;

        if (!CurrentState->Start())
            ChangeCurrentState();
    }
    else // for error
    {
        CurrentState = &NullState;

        if (findedState == nullptr) { UE_LOG(LogTemp, Error, TEXT("AAIAircraftPilot AIStateError -> findedState == nullptr")); }
        else if (*findedState == nullptr) { UE_LOG(LogTemp, Error, TEXT("AAIAircraftPilot AIStateError -> *findedState == nullptr")); }
        else UE_LOG(LogTemp, Error, TEXT("AAIAircraftPilot AIStateError"));
    }

}

void AAIAircraftPilot::Release(void)
{
    for (auto& item : States)
    {
        item.Value->Release();
    }
    AAIPilot::Release();

    Control = nullptr;
}
