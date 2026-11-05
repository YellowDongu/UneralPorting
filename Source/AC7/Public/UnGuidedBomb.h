// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Missile.h"
#include "UnGuidedBomb.generated.h"

	//static const FVector Gravity = FVector(0.0f, 0.0f, -980.0f); // Unreal Engine의 기본 중력 가속도라는 거 같다
static const FVector BombGravity{ 0.0, 0.0, -980.0 / 2.0 };
/**
 * 
 */
UCLASS()
class AC7_API AUnGuidedBomb : public AMissile
{
	GENERATED_BODY()
public:
	AUnGuidedBomb();

	virtual void Tick(float DeltaTime) override;

	virtual void Initialize(AMissile* origin, TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked) override;
	virtual bool Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked, AMissile*& output) override { return false; }
	virtual bool Shoot(TWeakObjectPtr<class AVehicleBase> owner, TWeakObjectPtr<class AVehicleBase> locked) override { return false; }
	virtual int Shoot(TWeakObjectPtr<class AVehicleBase> owner, FTargetInfo* locked) override;
	virtual void Lock(FTargetInfo& targetData, float DeltaTime, const FVector& forward, const FVector& position) override { targetData.Lock = 1.0f; }

	static FVector CalculatePosition(const FVector& StartPosition, const FVector& InitialVelocity, float deltaTime) { return StartPosition + InitialVelocity * 3.0 * deltaTime + 0.5 * BombGravity * deltaTime * deltaTime; }

	void Explode();

	double GetBlastArea(void) { return BlastArea; }

protected:
	bool bLanded{ false };
	float ElapsedTime{ 0.0f }, LandingTime{ 10.0f };
	double BlastArea{ 1000.0 };

	FVector StartPosition, InitialVelocity;
	FVector PreviousLocation;
};
