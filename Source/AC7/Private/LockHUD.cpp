#include "LockHUD.h"
#include "Aircraft.h"
#include "FireControlSystem.h"
#include "Missile.h"
#include "SoundSubsystem.h"
#include "Components/AudioComponent.h"

ULockHUD::ULockHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void ULockHUD::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	ChangeLockUI();
	if (*bSelectSpecial)
	{
		int length = SpecialIndicators.Num();
		bool aimed = false;
		for (int i = 0; i < length; i++)
		{
			if (currentTargets->Num() <= i)
			{
				SpecialIndicators[i]->SetActive(false);
				continue;
			}

			FTargetInfo& target = (*currentTargets)[i];

			aimed |= target.IsLock();
			SpecialIndicators[i]->Update(0.0f, &target);
		}

		if (aimed != LockSoundChannel->IsPlaying())
		{
			if (aimed)
				LockSoundChannel = UGameplayStatics::SpawnSound2D(this, SpecialLock, 1.0f, 1.0f, 0.0f, nullptr, false, false);
			else
				LockSoundChannel->Stop();
		}
	}
	else
	{
		if (currentTargets->IsEmpty())
		{
			StandardIndicator->SetActive(false);
			if (LockSoundChannel->IsPlaying())
				LockSoundChannel->Stop();
			return;
		}

		FTargetInfo& target = (*currentTargets)[0];
		StandardIndicator->Update(0.0f, &target);

		if (target.Lock >= 0.9f)
			LockSoundSet->Stop();
		else if (target.Lock < 0.1f)
			LockSoundSet->Play();
		else
		{
			float duration = LockSoundSet->Sound->Duration;
			duration = LockSoundSet->IsPlaying(false) ? duration : FMath::Max(LockSoundMin, duration * (*currentTargets)[0].Lock) * 1.25f;
			LockSoundSet->BlinkSound(duration, DeltaTime);
		}

		/*
		if (target.Lock >= 0.9f)
		{
			if (LockSoundChannel->IsPlaying())
			{
				LockSoundChannel->Stop();
				bSoundActive = false;
				SoundTimer = 50.0f;
			}

		}
		else if (target.Lock < 0.1f)
		{
			if (!LockSoundChannel->IsPlaying())
			{
				LockSoundChannel->Play();
				bSoundActive = false;
				SoundTimer = 50.0f;
			}
		}
		else
			PlayLockSound(DeltaTime, bSoundActive ? Lock->Duration : FMath::Max(LockSoundMin, Lock->Duration * (*currentTargets)[0].Lock) * 1.25f);
		*/
	}
}

void ULockHUD::NativeConstruct(void)
{
	Super::NativeConstruct();

	SetSound(GetWorld());
}

void ULockHUD::BindPlayer(AAircraft* vehicle, APlayerPilot* player)
{
	TWeakObjectPtr<UFireControlSystem> fcs = vehicle->GetFireControlSystem();
	if (fcs == nullptr)
		return;

	fcs->GetCurrentTargets(currentTargets);
	fcs->GetSelectSpecial(bSelectSpecial);
	int multishoot = fcs->GetMultiShoot();
	check(BaseCanvas);

	LockSet::IndicatorCreationData data;

	data.CanvasPanel = BaseCanvas;
	data.FCS = fcs;
	data.RangeIndicatorBluePrint = BombIndicatorBluePrint;
	data.LockBluePrint = StandardLockBluePrint;
	data.SpecialLockBluePrint = SpecialLockBluePrint;
	data.Weapon = fcs->GetMissile(false);

	StandardIndicator = StaticCastSharedPtr<LockIndicator>(LockSet::Create(data));
	SpecialIndicators.Reserve(multishoot);

	data.Weapon = fcs->GetMissile(true);
	if (data.Weapon == nullptr)
		return;

	for (int i = 0; i < multishoot; i++)
		SpecialIndicators.Add(LockSet::Create(data));

	bSelect = !*bSelectSpecial;

	SetSound(GetWorld());
}

void ULockHUD::SetSound(UWorld* world)
{
	if (bSelectSpecial == nullptr || world == nullptr)
		return;

	if (LockSoundChannel != nullptr)
		return;

	USoundSubsystem* soundSystem = GetGameInstance()->GetSubsystem<USoundSubsystem>();
	LockSoundSet = MakeShared<FSoundSet>();
	LockSoundSet->Sound = soundSystem->GetSound(TEXT("StandardLock"));

	SpecialLock = soundSystem->GetSound(TEXT("336031978"));
	USoundWave* SoundWave = Cast<USoundWave>(SpecialLock);
	if (SoundWave != nullptr)
	{
		SoundWave->bLooping = 1;
		//SoundWave->Volume = 0.5f;
	}
	//UE_LOG(LogTemp, Log, TEXT("class=%s path=%s looping=%d"), *SoundWave->GetClass()->GetName(), *SoundWave->GetPathName(), SoundWave ? (int)SoundWave->bLooping : -1);

	SoundWave = Cast<USoundWave>(LockSoundSet->Sound);
	if (SoundWave != nullptr)
	{
		SoundWave->bLooping = 1;
		//SoundWave->Volume = 0.5f;
	}
	//UE_LOG(LogTemp, Log, TEXT("class=%s path=%s looping=%d"), *SoundWave->GetClass()->GetName(), *SoundWave->GetPathName(), SoundWave ? (int)SoundWave->bLooping : -1);

	LockSoundSet->Channel = LockSoundChannel = soundSystem->GetChannel(TEXT("LockSoundChannel"), world).Get();

	if (!LockSoundChannel->IsRegistered())
		LockSoundChannel->RegisterComponentWithWorld(world);

	LockSoundChannel->bAutoActivate = false;
	LockSoundChannel->SetSound(LockSoundSet->Sound);

	bSoundActive = false;
	SoundTimer = 50.0f;
}

void ULockHUD::ChangeLockUI(void)
{
	if (bSelect == *bSelectSpecial)
		return;

	bSelect = *bSelectSpecial;
	StandardIndicator->SetActive(!bSelect);

	for (auto& item : SpecialIndicators)
		item->SetActive(bSelect);

	LockSoundChannel->Stop();
	LockSoundChannel->SetSound(bSelect ? SpecialLock : LockSoundSet->Sound);
	SoundTimer = 100.0f;
}

void ULockHUD::PlayLockSound(float deltaTime, float maxTime)
{
	SoundTimer += deltaTime;
	if (SoundTimer < maxTime)
		return;
	SoundTimer = 0.0f;

	bSoundActive = !bSoundActive;
	UE_LOG(LogTemp, Log, TEXT("[ULockHUD::PlayLockSound] Active = %d, SoundTimer = %.3f, Lock = %.3f, maxtime = %.3f"), bSoundActive, SoundTimer, (*currentTargets)[0].Lock, maxTime);

	if (bSoundActive)
		LockSoundChannel->Stop();
	else
	{
		if (LockSoundChannel->IsPlaying())
			return;

		LockSoundChannel->Play();
	}
}
