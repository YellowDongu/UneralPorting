// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIPilot.h"
#include "GroundAIState.h"
#include "AIDriver.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AAIDriver : public AAIPilot
{
	GENERATED_BODY()

public:
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;

	virtual void Attach(class AVehicleBase* vehicle) override;

	//virtual void Release() override
	//{
	//	AAIPilot::Release();
	//	Control = nullptr;
	//}

private:
	//CombatState Combat;
	//ChaseState Chase;
	//SearchState Search;
	//InFomationState InFomation;
	//AltitudeSetterState AltitudeSetter;
	//DefendState Defend;


	AttackState Attack;
	HoldState Hold;
};
