// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIPilot.h"
#include "AIAircraftState.h"
#include "AIAircraftPilot.generated.h"

/**
 * 
 */

UCLASS()
class AC7_API AAIAircraftPilot : public AAIPilot
{
	GENERATED_BODY()
public:
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override { Super::Tick(DeltaTime); }

	virtual void Attach(class AVehicleBase* vehicle) override;

	virtual void Release(void) override;

private:
	FControl* Control{ nullptr };
	CombatState Combat;
	ChaseState Chase;
	SearchState Search;
	InFomationState InFomation;
	AltitudeSetterState AltitudeSetter;
	DefendState Defend;
};
