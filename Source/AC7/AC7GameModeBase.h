// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AC7GameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API AAC7GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAC7GameModeBase();

	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APlayerController> PlayerController;
	class APlayerPilot* player;

	bool flag = false;
};
