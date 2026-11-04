// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "SoundSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class AC7_API USoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TObjectPtr<USoundBase> LoadSound(const FString& name);
	void AsyncLoadSound(const TArray<FString>& name);

	bool PlaySound(const FString& name, TObjectPtr<USoundBase>* output = nullptr, TWeakObjectPtr<UAudioComponent>* outputChannel = nullptr); // simple sound play
	TObjectPtr<USoundBase> GetSound(const FString& name);
	TWeakObjectPtr<UAudioComponent> GetChannel(const FString& name) { return GetChannel(name, GetWorld()); }
	TWeakObjectPtr<UAudioComponent> GetChannel(const FString& name, UWorld* world);
	//void RegisterChannel(const FString& name, TObjectPtr<UAudioComponent> pointer) { if (Channels.Contains(name)) pointer = *Channels.Find(name); else Channels.Add(name, pointer); }

private:
	UPROPERTY() TMap<FString, TObjectPtr<USoundBase>> Sounds;
	UPROPERTY() TMap<FString, TSoftObjectPtr<USoundBase>> SoundAssets;
	UPROPERTY() TMap<FString, TObjectPtr<UAudioComponent>> Channels; // 강한 참조로 GC 수거 방지

};
