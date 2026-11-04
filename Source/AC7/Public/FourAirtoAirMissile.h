// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Missile.h"
#include "FourAirtoAirMissile.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AFourAirtoAirMissile : public AMissile
{
	GENERATED_BODY()
public:
	AFourAirtoAirMissile();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void Release() override;

};
