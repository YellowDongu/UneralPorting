// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "SoundSubsystem.generated.h"

/**
 * 
 */

class UAudioComponent;
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

USTRUCT()
struct AC7_API FSoundSet
{
	GENERATED_BODY()

	FSoundSet() = default;

	void Initialize(const FString& soundName, const FString& channelName, USoundSubsystem* sound);
	void BlinkSound(float maxTime, float deltaTime);
	void Stop(void);
	void Play(void);
	bool IsPlaying(bool atChannel = true);

	TWeakObjectPtr<UAudioComponent> Channel{ nullptr };
	TObjectPtr<USoundBase> Sound{ nullptr };

private:
	bool bIsPlaying{ false };
	float Timer{ 0.0f };
};