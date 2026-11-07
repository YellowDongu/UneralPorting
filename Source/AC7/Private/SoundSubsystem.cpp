#include "SoundSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Tasks/Task.h"
#include "Async/TaskGraphInterfaces.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Components/AudioComponent.h"
//#include "UObject/UObjectGlobals.h"

void USoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    IAssetRegistry& assetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    FARFilter filter;
    filter.ClassPaths.Add(USoundBase::StaticClass()->GetClassPathName());
    filter.bRecursiveClasses = true;

    TArray<FAssetData> foundedAssets;
    assetRegistry.GetAssets(filter, foundedAssets);
    SoundAssets.Reserve(foundedAssets.Num());

    for (const FAssetData& Asset : foundedAssets)
    {
        TSoftObjectPtr<USoundBase> Sound(Asset.ToSoftObjectPath());
        SoundAssets.Add(Asset.AssetName.ToString(), MoveTemp(Sound));
    }
}

void USoundSubsystem::Deinitialize()
{
    Super::Deinitialize();
    //FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
}

TObjectPtr<USoundBase> USoundSubsystem::LoadSound(const FString& name)
{
    TSoftObjectPtr<USoundBase>* pointer = SoundAssets.Find(name);
    if (pointer == nullptr)
        return nullptr;

    TObjectPtr<USoundBase>& result = Sounds.FindOrAdd(name);

    if (result == nullptr)
        result = UAssetManager::GetStreamableManager().LoadSynchronous(*pointer);

    return result;
}

void USoundSubsystem::AsyncLoadSound(const TArray<FString>& nameList)
{
    TArray<FSoftObjectPath> pathList;
    TMap<FSoftObjectPath, FString> pathMap;
    for (auto& name : nameList)
    {
        auto pointer = SoundAssets.Find(name);
        if (pointer == nullptr)
            continue;

        TObjectPtr<USoundBase> result = Sounds.FindOrAdd(name);
        if (result != nullptr)
            continue;

        pathList.Add(pointer->ToSoftObjectPath());
        pathMap.Add(pointer->ToSoftObjectPath(), name);
    }

    UAssetManager::GetStreamableManager().RequestAsyncLoad(pathList, [pathMap, WeakThis = TWeakObjectPtr<USoundSubsystem>(this)]()
                                                           {
                                                               USoundSubsystem* self = WeakThis.Get();
                                                               if (self == nullptr)
                                                                   return;

                                                               for (const auto& item : pathMap)
                                                               {
                                                                   USoundBase* Sound = Cast<USoundBase>(item.Key.ResolveObject());

                                                                   if (Sound == nullptr)
                                                                       continue;

                                                                   TObjectPtr<USoundBase>* result = self->Sounds.Find(item.Value);
                                                                   *result = Sound;
                                                               }
                                                           });
}

bool USoundSubsystem::PlaySound(const FString& name, TObjectPtr<USoundBase>* output, TWeakObjectPtr<UAudioComponent>* outputChannel)
{
    TObjectPtr<USoundBase> sound = GetSound(name);
    if (sound == nullptr)
        return false;

    if (output != nullptr)
        *output = sound;

    auto channel = UGameplayStatics::SpawnSound2D(this, sound, 1.0f, 1.0f, 0.0f, nullptr, false, false);
    if (outputChannel != nullptr)
        *outputChannel = channel;

    return true;
}

TObjectPtr<USoundBase> USoundSubsystem::GetSound(const FString& name)
{
	auto result = Sounds.Find(name);
	if (result == nullptr)
        return LoadSound(name);

    return *result;
}

TWeakObjectPtr<UAudioComponent> USoundSubsystem::GetChannel(const FString& name, UWorld* world)
{
    TObjectPtr<UAudioComponent>& output = Channels.FindOrAdd(name);

    if (output == nullptr)
    {
        output = NewObject<UAudioComponent>(this);
        output->bAutoActivate = false;
    }

    if (!output->IsRegistered() || IsValid(output->GetWorld()))
        output->RegisterComponentWithWorld(world);

    return TWeakObjectPtr<UAudioComponent>(output.Get());
}

void FSoundSet::Initialize(const FString& soundName, const FString& channelName, USoundSubsystem* sound)
{
    Channel = sound->GetChannel(channelName, sound->GetWorld());
    Sound = sound->GetSound(soundName);
}

void FSoundSet::BlinkSound(float maxTime, float deltaTime)
{
    Timer += deltaTime;
    if (Timer < maxTime)
    {
        if (!bIsPlaying || !Channel->IsPlaying())
            return;
    }

    Timer = 0.0f;
    bIsPlaying = !bIsPlaying;

    if (bIsPlaying)
        Channel->Stop();
    else
        Channel->Play();
}

void FSoundSet::Stop(void) { if (!Channel->IsPlaying()) return; bIsPlaying = false; Channel->Stop(); }
void FSoundSet::Play(void) { if (Channel->IsPlaying()) return; bIsPlaying = true; Channel->Play(); }
bool FSoundSet::IsPlaying(bool atChannel) { return atChannel ? Channel->IsPlaying() : bIsPlaying; }