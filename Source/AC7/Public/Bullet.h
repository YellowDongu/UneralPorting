// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class AC7_API ABullet : public AActor
{
	GENERATED_BODY()
	
public:	
	ABullet();

	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void OnTriggerHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Initialize();
	void Disappear();
	void Release();
	void Shoot(const FVector& position, const FRotator& Rotation, float velocity);
protected:
	float LifeTime{ 5.0f }, CurrentLifeTime{ 0.0f };
	double MinimumSpeed{ 50.0 }, ShootSpeed{ 330.0 }, CurrentSpeed{ 0.0 };

	TWeakObjectPtr<UStaticMeshComponent> Body{ nullptr };
	//TWeakObjectPtr<UNiagaraComponent> Body{ nullptr };
	//TWeakObjectPtr<UNiagaraComponent> Trail{ nullptr };
	//FNiagaraParameterDirectBinding<FVector3f> FlareBodyDirection;
};
