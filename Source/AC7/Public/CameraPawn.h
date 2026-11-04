// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"


#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


#include "CameraPawn.generated.h"

UCLASS()
class AC7_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACameraPawn();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;// Called to bind functionality to input

protected:
	virtual void BeginPlay() override;
	void Movement(float deltaTime);

	APlayerController* controller{ nullptr };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* cameraArm{ nullptr };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* camera{ nullptr };
	UPROPERTY(EditAnywhere, Category = "Camera")
	float MoveSpeed{ 1000.f };
	UPROPERTY(EditAnywhere, Category = "Camera")
	float armLength{ 1200.f };
	UPROPERTY(EditAnywhere, Category = "Camera")
	float armRotation{ -45.0f };
};
