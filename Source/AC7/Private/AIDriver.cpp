// Fill out your copyright notice in the Description page of Project Settings.


#include "AIDriver.h"

#include "GroundVehicle.h"

typedef AIState::State EAIState;
void AAIDriver::PostInitProperties()
{
    Super::PostInitProperties();
    States.Add(EAIState::Combat, &Attack);
    States.Add(EAIState::Search, &Hold);
}

void AAIDriver::BeginPlay()
{
    Super::BeginPlay();
}

void AAIDriver::Attach(AVehicleBase* vehicle)
{
    AGroundVehicle* car = Cast<AGroundVehicle>(vehicle);
    check(car);

    AAIPilot::Attach(vehicle);
    
    GroundAIState::AircraftDefaultData defaultData;
    defaultData.Owner = car;
    defaultData.SquadData = SquadData;

    Attack.SetDefaultData(&defaultData);
    Hold.SetDefaultData(&defaultData);

    //AltitudeSetter.SetDefaultData(&defaultData);
    //Search.SetDefaultData(&defaultData);
    //InFomation.SetDefaultData(&defaultData);
    //Defend.SetDefaultData(&defaultData);

    CurrentState = &Hold;
    currentState_debug = EAIState::Search;
    if (!CurrentState->Start())
        ChangeCurrentState();
}