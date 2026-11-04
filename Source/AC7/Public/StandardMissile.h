// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Missile.h"
#include "StandardMissile.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AStandardMissile : public AMissile
{
	GENERATED_BODY()
public:
	AStandardMissile();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void Release() override;
};
