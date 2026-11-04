// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuMode.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AMenuMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMenuMode() = default;
	//virtual ~AMenuMode() = default;

	//virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	//virtual void PreInitializeComponents() override;
	//virtual void PostInitializeComponents() override;
	//virtual void PostLogin(APlayerController* NewPlayer) override;
	//virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	//virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	//virtual APlayerController* SpawnPlayerControllerCommon(ENetRole InRemoteRole, FVector const& SpawnLocation, FRotator const& SpawnRotation, TSubclassOf<APlayerController> InPlayerControllerClass) override;

private:

};
