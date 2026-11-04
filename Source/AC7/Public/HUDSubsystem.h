// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HUDSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API UHUDSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void CreateHUD();
	void LinkHUD(class AAircraft* aircraft, class APlayerPilot* pilot);
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UFrontHUD> FrontHUDBlueprint{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UFrontHUD> FrontHUD{ nullptr };


};
