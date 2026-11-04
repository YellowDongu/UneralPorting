// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VehicleBase.h"
#include "NiagaraComponent.h"
#include "Flare.generated.h"

/**
 *
 */
UCLASS()
class AC7_API AFlare final : public AVehicleBase
{
	GENERATED_BODY()
public:
	AFlare();
	virtual ~AFlare() = default;

	virtual void BeginPlay(void) override;
	virtual void PostInitProperties(void) override;
	virtual void PostInitializeComponents(void) override;
	virtual void Tick(float DeltaTime) override;
	//virtual void OnConstruction();
	//virtual void PreInitializeComponents() override;
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void SystemIntegration(void) override {}

	void Disappear();
	void RetriveObject();
	void Shoot(USceneComponent* flarePod, const FVector& DirectionVector = FVector::ZeroVector);
private:
	float LifeTime{ 5.0f }, CurrentLifeTime{0.0f};
	double MinimumSpeed{50.0f}, ShootSpeed{ 330.0f }, CurrentSpeed;

	TWeakObjectPtr<UNiagaraComponent> Body{nullptr};
	TWeakObjectPtr<UNiagaraComponent> Trail{nullptr};
	FNiagaraParameterDirectBinding<FVector3f> FlareBodyDirection;
};