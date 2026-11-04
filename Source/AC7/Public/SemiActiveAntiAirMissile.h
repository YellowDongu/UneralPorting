// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Missile.h"
#include "SemiActiveAntiAirMissile.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API ASemiActiveAntiAirMissile : public AMissile
{
	GENERATED_BODY()

public:
	ASemiActiveAntiAirMissile();

	//virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

	virtual bool Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked, AMissile*& output) override;
	void Rebind(TWeakObjectPtr<class AVehicleBase> newTarget);

protected:
	virtual void Release() override;

	const TArray<FTargetInfo>* FCSInfo;
};
