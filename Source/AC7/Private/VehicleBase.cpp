// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "ObjectFactory.h"
#include "TeamManagementSystem.h"

AVehicleBase::AVehicleBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AVehicleBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	VehicleMask.Initialize();
}

void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AVehicleBase::AttachCamera(TWeakObjectPtr<class UCameraComponent>& camera, bool thridViewAttach)
{
	if (camera == nullptr)
		camera = NewObject<UCameraComponent>(this, TEXT("BaseCamera"));

	if (thridViewAttach)
	{
		if (ThirdCameraArm == nullptr)
		{
			ThirdCameraArm = FindComponentByClass<USpringArmComponent>();
			//ThirdCameraArm->bUsePawnControlRotation = false;
			//ThirdCameraArm->bInheritYaw = true;
			//ThirdCameraArm->bInheritPitch = true;
			//ThirdCameraArm->bInheritRoll = true;
			ThirdCameraArm->bDoCollisionTest = true;
		}

		camera->SetupAttachment(ThirdCameraArm);
		camera->RegisterComponent();
	}
	else
	{
		if (FirstCameraArm == nullptr)
			FirstCameraArm = FindComponentByClass<USceneComponent>();

		camera->SetupAttachment(FirstCameraArm);
		camera->RegisterComponent();
	}
}

void AVehicleBase::SystemInitialize(const FSpawnInfomation& spawnInfomation)
{
	Team = spawnInfomation.TeamIndex;
	bIsTGT = spawnInfomation.IsTGT;
	if (spawnInfomation.bIsInvincible)
		HP = TNumericLimits<int>::Max();
	else
		HP = MaxHp;

	bIsAlive = true;
	UE_LOG(LogTemp, Warning, TEXT("[AVehicleBase::SystemInitialize] %s / HP %d, Team %d, TGT %d"), *GetName(), HP, Team, bIsTGT);
}

void AVehicleBase::Release()
{
	UObjectFactory* factory = GetWorld()->GetSubsystem<UObjectFactory>();
	factory->Release(this);

	DamageEvent.Clear();
	bIsAlive = false;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
}

void AVehicleBase::HPDamage(int damage)
{
	if (HP <= 0)
		return;
	UE_LOG(LogTemp, Warning, TEXT("[AVehicleBase::HPDamage] %s HP %d - damage %d = %d"), *GetName(), HP, damage, HP - damage);
	HP -= damage;
	DamageEvent.Broadcast(this);
	if (HP <= 0)
	{
		GetWorld()->GetSubsystem<UTeamManagementSystem>()->TeamRemove(SquadSystem);
		GetWorld()->GetSubsystem<UObjectFactory>()->Release(this);
		bIsAlive = false;
		UE_LOG(LogTemp, Warning, TEXT("[AVehicleBase::HPDamage] %s DEAD"), *GetName());
		//gameObject.SetActive(false);
		//GameMaster.Instance.Factory.Explosion(gameObject.transform.position, 10.0f);
		//Release();
	}
}