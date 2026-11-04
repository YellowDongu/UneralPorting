// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Missile.h"
#include "GuidedBomb.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AGuidedBomb : public AMissile
{
	GENERATED_BODY()
public:
	AGuidedBomb();

	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

	virtual bool Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked, AMissile*& output) override;
	//virtual int Shoot(TWeakObjectPtr<class AVehicleBase> owner, FTargetInfo* locked) override;

	void Explode();
	double GetBlastArea(void) { return BlastArea; }

protected:
	float ElapsedTime{ 0.0f }, LandingTime{ 10.0f };
	double BlastArea{ 500.0 };

	FVector TrackedPosition, StartPosition;
};
