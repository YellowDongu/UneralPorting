// Fill out your copyright notice in the Description page of Project Settings.

#include "StageChangeUIUnit.h"

#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"

#include "GameMaster.h"
#include "LevelLoader.h"
#include "BluePrintCollector.h"
#include "SoundSubsystem.h"

static constexpr float timerDeltaTime = 1.0f / 60.0f;

float UStageChangeUIUnit::GetDeltaTime()
{
	return FMath::Max(timerDeltaTime, GetWorld()->GetDeltaSeconds());
}

void UStageChangeUIUnit::NativeConstruct()
{
	Super::NativeConstruct();

	LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
	MissionSuccessPanel->SetVisibility(ESlateVisibility::Hidden);
	bIsFadeOut = true;
	FadeOut(false);

	InterSelect();
	InterMission();
	ExitSelect();
	StartSelectLevel();
}


void UStageChangeUIUnit::InterSelect(void)
{
	auto& sequence = Sequences.FindOrAdd(TEXT("InterSelect"));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			MissionPanel->SetVisibility(ESlateVisibility::Visible);
			MissionText->SetVisibility(ESlateVisibility::Visible);

			MissionLoadingPanel->SetVisibility(ESlateVisibility::Hidden);
			MainText->SetVisibility(ESlateVisibility::Hidden);
			SubText->SetVisibility(ESlateVisibility::Hidden);

			FadeOut(true);
			LoadingCanvas->SetVisibility(ESlateVisibility::Visible);
			FadeOut(false, 0.5f);
			UE_LOG(LogTemp, Warning, TEXT("[UStageChangeUIUnit::InterSelect] Phase 1"));
			return true;
		}));
	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			UE_LOG(LogTemp, Warning, TEXT("[UStageChangeUIUnit::InterSelect] Phase 2"));
			return true;
		}));
	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (accumulatedTime <= 5.0f)
				return false;

			FadeOut(true, 0.5f);
			UE_LOG(LogTemp, Warning, TEXT("[UStageChangeUIUnit::InterSelect] Phase 3"));
			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			FadeOut(false, 0.5f);
			LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
			CallBack.Broadcast();
			CallBack.Clear();

			UE_LOG(LogTemp, Warning, TEXT("[UStageChangeUIUnit::InterSelect] Phase 4"));
			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			UE_LOG(LogTemp, Warning, TEXT("[UStageChangeUIUnit::InterSelect] Phase END"));
			return true;
		}));
}

void UStageChangeUIUnit::InterMission(void)
{
	static FString functionName = TEXT("EnterMission");
	auto& sequence = Sequences.FindOrAdd(functionName);

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			MissionPanel->SetVisibility(ESlateVisibility::Hidden);
			MissionText->SetVisibility(ESlateVisibility::Hidden);

			MissionLoadingPanel->SetVisibility(ESlateVisibility::Visible);
			MainText->SetVisibility(ESlateVisibility::Visible);
			SubText->SetVisibility(ESlateVisibility::Visible);

			UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
			FStreamableDelegate delegate = FStreamableDelegate::CreateUObject(this, &UStageChangeUIUnit::GetLoadingEndFlag);
			gameMaster->ReserveLevel(FString(TEXT("MS01")));
			gameMaster->GetSubsystem<ULevelLoader>()->AssetAsyncLoad(TEXT("MissionLevel"), gameMaster->GetReserveLevel().ToSoftObjectPath(), true, delegate);

			FadeOut(true, 0.5f);
			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			LoadingCanvas->SetVisibility(ESlateVisibility::Visible);
			FadeOut(false, 0.5f);

			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			return !FadeOutTimerHandle.IsValid();
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			static float LoadingTime = 10.0f;
			if (!bLoadingEndFlag || accumulatedTime <= LoadingTime)
				return false;

			FadeOut(true, 0.5f);
			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);
			LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
			UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
			gameMaster->ChangeReservedLevel();
			ActivedFunction.Remove(functionName);
			return true;
		}));
}

void UStageChangeUIUnit::ExitSelect(void)
{
	static FString functionName = TEXT("ExitSelect");
	auto& sequence = Sequences.FindOrAdd(functionName);

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			MissionPanel->SetVisibility(ESlateVisibility::Visible);
			MissionText->SetVisibility(ESlateVisibility::Visible);

			MissionLoadingPanel->SetVisibility(ESlateVisibility::Hidden);
			MainText->SetVisibility(ESlateVisibility::Hidden);
			SubText->SetVisibility(ESlateVisibility::Hidden);

			FadeOut(true, 0.5f);
			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			LoadingCanvas->SetVisibility(ESlateVisibility::Visible);
			FadeOut(false, 0.5f);
			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (accumulatedTime <= 5.0f)
				return false;

			FadeOut(true, 0.5f);
			return true;
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
			//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);

			ActiveCallBack(functionName);
			ActivedFunction.Remove(functionName);
			return true;
		}));
}

void UStageChangeUIUnit::StartSelectLevel(void)
{
	static FString functionName = TEXT("EnterSelect");
	auto& sequence = Sequences.FindOrAdd(functionName);

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
			FadeOut(true);
			FadeOut(false, 0.5f);
			return true;
			UE_LOG(LogTemp, Warning, TEXT("[UStageChangeUIUnit::StartSelectLevel] Phase 1"));
		}));

	sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FadeOutTimerHandle.IsValid())
				return false;

			ActiveCallBack(functionName);
			ActivedFunction.Remove(functionName);
			UE_LOG(LogTemp, Warning, TEXT("[UStageChangeUIUnit::StartSelectLevel] Phase END"));
			//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);
			return true;
		}));
}



void UStageChangeUIUnit::EnterSelect(const FCallBackFunction::FDelegate& CallBackDelegate)
{
	static FString functionName = TEXT("EnterSelect");
	if (ActivedFunction.Contains(functionName))
		return;

	ActivedFunction.Add(functionName);
	CallBacks.FindOrAdd(functionName).Add(CallBackDelegate);
	//if (InterSelectHandle.IsValid())
	//	return;

	//TickFunctions.Add([this, phase = 0, phaseTime = 0.0f](float deltaTime) mutable -> bool { return StartSelectLevel(phase, phaseTime); });
	//return;

	auto sequence = Sequences.Find(functionName);
	if (sequence == nullptr)
		return;

	TickFunctions.Add([this, sequence](float deltaTime) -> bool { sequence->Update(deltaTime); return sequence->SequenceList.Num() <= sequence->currentIndex; });
}

void UStageChangeUIUnit::EnterMission()
{
	static FString functionName = TEXT("EnterMission");
	if (ActivedFunction.Contains(functionName))
		return;

	ActivedFunction.Add(functionName);
	//if (InterSelectHandle.IsValid())
	//	return;

	bLoadingEndFlag = false;
	FadeOut(true);

	//TickFunctions.Add([this, phase = 0, phaseTime = 0.0f](float deltaTime) mutable -> bool { return InterMission(phase, phaseTime, deltaTime); });
	//GetWorld()->GetTimerManager().SetTimer(InterSelectHandle, this, &UStageChangeUIUnit::InterMission, timerDeltaTime, true);

	auto sequence = Sequences.Find(functionName);
	if (sequence == nullptr)
		return;

	TickFunctions.Add([this, sequence](float deltaTime) -> bool { sequence->Update(deltaTime); return sequence->SequenceList.Num() <= sequence->currentIndex; });
}

void UStageChangeUIUnit::ExitSelect(const FCallBackFunction::FDelegate& CallBackDelegate)
{
	static FString functionName = TEXT("ExitSelect");
	if (ActivedFunction.Contains(functionName))
		return;

	const FString& name = GetGameInstance<UGameMaster>()->GetReserveLevelName();
	if (!name.IsEmpty())
	{
		FText text = FText::FromString(name);
		MissionText->SetText(text);
		MainText->SetText(text);
	}

	ActivedFunction.Add(functionName);
	CallBacks.FindOrAdd(functionName).Add(CallBackDelegate);

	//if (InterSelectHandle.IsValid())
	//	return;

	//TickFunctions.Add([this, phase = 0, phaseTime = 0.0f](float deltaTime) mutable -> bool { return ExitSelect(phase, phaseTime, deltaTime); });
	//GetWorld()->GetTimerManager().SetTimer(InterSelectHandle, this, &UStageChangeUIUnit::ExitSelect, timerDeltaTime, true);
	auto sequence = Sequences.Find(functionName);
	if (sequence == nullptr)
		return;

	TickFunctions.Add([this, sequence](float deltaTime) -> bool { sequence->Update(deltaTime); return sequence->SequenceList.Num() <= sequence->currentIndex; });
}




void UStageChangeUIUnit::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	for (auto iterator = TickFunctions.CreateIterator(); iterator; )
	{
		if ((*iterator)(DeltaTime))
			iterator.RemoveCurrent();
		else
			++iterator;
	}
}

bool UStageChangeUIUnit::StartSelectLevel(int& phase, float& phaseTime)
{
	static FString functionName = TEXT("EnterSelect");
	//static int phase = 0;
	//static float phaseTime = 0.0f;
	switch (phase)
	{
	case 0:
		LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
		FadeOut(true);
		FadeOut(false, 0.5f);
		phase++;
		break;
	case 1:
		if (FadeOutTimerHandle.IsValid())
			break;

		phase++;
		ActiveCallBack(functionName);
		ActivedFunction.Remove(functionName);
		//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);
		return true;
	default:
		phase = 0;
		break;
	}
	return false;
}

bool UStageChangeUIUnit::ReturnMenu(int& phase, float& phaseTime, float deltaTime)
{
	switch (phase)
	{
	case 0:
		phaseTime -= deltaTime;
		if (phaseTime <= 3.5f)
		{
			MissionSuccessPanel->SetVisibility(ESlateVisibility::Visible);
			GetGameInstance()->GetSubsystem<USoundSubsystem>()->PlaySound(TEXT("MissionSuccess"));
			phase++;
		}
		break;
	case 1:
		phaseTime -= deltaTime;
		if (phaseTime <= 0.0f)
		{
			FadeOut(true, 1.5f);
			phase++;
		}
		break;
	case 2:
	{
		if (FadeOutTimerHandle.IsValid())
			break;

		MissionSuccessPanel->SetVisibility(ESlateVisibility::Hidden);
		UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
		check(gameMaster);
		UBluePrintCollector* bluePrintCollector = gameMaster->GetCollector();
		check(bluePrintCollector);

		ULevelLoader* levelLoader = gameMaster->GetSubsystem<ULevelLoader>();
		levelLoader->Unload(levelLoader->GetCurrentLevel().ToString());

		gameMaster->CancelReservationLevel();
		levelLoader->ChangeLevel(bluePrintCollector->GetMainMenuLevel());

		phase++;
		//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);
	}
	break;
	case 3:
		ActivedFunction.Remove(TEXT("ReturnMenu"));
		return true;
	default:
		phase = 0;
		break;
	}

	return false;
}

void UStageChangeUIUnit::ReturnMainMenu(bool EndMission)
{
	static FString functionName = TEXT("ReturnMainMenu");
	if (ActivedFunction.Contains(functionName))
		return;

	ActivedFunction.Add(functionName);
	TickFunctions.Add([this, phase = EndMission ? 0 : 1, timer = EndMission ? 5.0f : 3.5f](float deltaTime) mutable -> bool { return ReturnMenu(phase, timer, deltaTime); });

	//if (InterSelectHandle.IsValid())
	//	return;

	//GetWorld()->GetTimerManager().SetTimer(InterSelectHandle, this, &UStageChangeUIUnit::ReturnMenu, timerDeltaTime, true);
}

bool UStageChangeUIUnit::InterSelect(int& phase, float& phaseTime)
{
	//static int phase = 0;
	//static float phaseTime = 0.0f;
	switch (phase)
	{
	case 0:
		MissionPanel->SetVisibility(ESlateVisibility::Visible);
		MissionText->SetVisibility(ESlateVisibility::Visible);

		MissionLoadingPanel->SetVisibility(ESlateVisibility::Hidden);
		MainText->SetVisibility(ESlateVisibility::Hidden);
		SubText->SetVisibility(ESlateVisibility::Hidden);

		FadeOut(true);
		LoadingCanvas->SetVisibility(ESlateVisibility::Visible);
		FadeOut(false, 0.5f);
		phase++;
		break;
	case 1:
		if (FadeOutTimerHandle.IsValid())
			break;

		phase++;
		phaseTime = 5.0f;
		break;
	case 2:
		phaseTime -= timerDeltaTime;
		if (phaseTime <= 0.0f)
		{
			phase++;
			phaseTime = 0.0f;
		}
		break;
	case 3:
		FadeOut(true, 0.5f);
		phase++;
		break;
	case 4:
		if (FadeOutTimerHandle.IsValid())
			break;

		FadeOut(false, 0.5f);
		LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
		CallBack.Broadcast();
		CallBack.Clear();
		phase++;
		break;
	case 5:
		if (FadeOutTimerHandle.IsValid())
			break;

		//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);
		phase++;
		return true;
	default:
		phase = 0;
		break;
	}
	return false;
}


bool UStageChangeUIUnit::ExitSelect(int& phase, float& phaseTime, float deltaTime)
{
	static FString functionName = TEXT("ExitSelect");
	//static int phase = 0;
	//static float phaseTime = 0.0f;
	switch (phase)
	{
	case 0:
		MissionPanel->SetVisibility(ESlateVisibility::Visible);
		MissionText->SetVisibility(ESlateVisibility::Visible);

		MissionLoadingPanel->SetVisibility(ESlateVisibility::Hidden);
		MainText->SetVisibility(ESlateVisibility::Hidden);
		SubText->SetVisibility(ESlateVisibility::Hidden);

		FadeOut(true, 0.5f);
		phase++;
		break;
	case 1:
		if (FadeOutTimerHandle.IsValid())
			break;

		LoadingCanvas->SetVisibility(ESlateVisibility::Visible);
		FadeOut(false, 0.5f);
		phase++;
		break;
	case 2:
		if (FadeOutTimerHandle.IsValid())
			break;

		phase++;
		phaseTime = 5.0f;
		break;
	case 3:
		phaseTime -= deltaTime;
		if (phaseTime <= 0.0f)
		{
			phase++;
			phaseTime = 0.0f;
		}
		break;
	case 4:
		FadeOut(true, 0.5f);
		phase++;
		break;
	case 5:
		if (FadeOutTimerHandle.IsValid())
			break;

		LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
		//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);

		ActiveCallBack(functionName);
		ActivedFunction.Remove(functionName);

		phase++;
		return true;
	default:
		phase = 0;
		break;
	}
	return false;
}

void UStageChangeUIUnit::GetLoadingEndFlag()
{
	UE_LOG(LogTemp, Warning, TEXT("UStageChangeUIUnit::GetLoadingEndFlag : true"));
	bLoadingEndFlag = true;
}

bool UStageChangeUIUnit::InterMission(int& phase, float& phaseTime, float deltaTime)
{
	//static int phase = 0;
	//static float phaseTime = 0.0f;
	static float LoadingTime = 10.0f;
	switch (phase)
	{
	case 0:
	{
		FadeOut(true, 0.5f);
		phase++;

		MissionPanel->SetVisibility(ESlateVisibility::Hidden);
		MissionText->SetVisibility(ESlateVisibility::Hidden);

		MissionLoadingPanel->SetVisibility(ESlateVisibility::Visible);
		MainText->SetVisibility(ESlateVisibility::Visible);
		SubText->SetVisibility(ESlateVisibility::Visible);

		UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
		FStreamableDelegate delegate = FStreamableDelegate::CreateUObject(this, &UStageChangeUIUnit::GetLoadingEndFlag);
		gameMaster->ReserveLevel(FString(TEXT("MS01")));
		gameMaster->GetSubsystem<ULevelLoader>()->AssetAsyncLoad(TEXT("MissionLevel"), gameMaster->GetReserveLevel().ToSoftObjectPath(), true, delegate);
		break;
	}
	case 1:
		if (FadeOutTimerHandle.IsValid())
			break;

		LoadingCanvas->SetVisibility(ESlateVisibility::Visible);
		FadeOut(false, 0.5f);
		phase++;
		break;
	case 2:
		if (FadeOutTimerHandle.IsValid())
			break;

		phase++;
		phaseTime = LoadingTime;
		break;
	case 3:
		phaseTime -= deltaTime;
		if (bLoadingEndFlag && phaseTime <= 0.0f)
		{
			phase++;
			phaseTime = 0.0f;
		}


		break;
	case 4:
		FadeOut(true, 0.5f);
		phase++;
		break;
	case 5:
	{
		if (FadeOutTimerHandle.IsValid())
			break;

		//GetWorld()->GetTimerManager().ClearTimer(InterSelectHandle);
		LoadingCanvas->SetVisibility(ESlateVisibility::Hidden);
		UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
		gameMaster->ChangeReservedLevel();
		ActivedFunction.Remove(TEXT("EnterMission"));
		phase++;
		return true;
	}
	default:
		phase = 0;
		break;
	}
	return false;
}

void UStageChangeUIUnit::FadeOut(bool active, float fadeoutTime)
{
	if (bIsFadeOut == active)
		return;
	if (fadeoutTime <= 0.0f)
	{
		FadeOut(active);
		return;
	}

	bIsFadeOut = active;
	FadeOutTime = fadeoutTime;
	FadeOutTimer = 0.0f;

	if (bIsFadeOut)
	{
		if (FadeOutTimerHandle.IsValid())
		{
			FadeOutTimer = FadeOutTime * BlackFadeOut->ColorAndOpacity.A;
			GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		}

		BlackFadeOut->SetVisibility(ESlateVisibility::Visible);
		GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &UStageChangeUIUnit::FadeOutCoroutine, timerDeltaTime, true);
	}
	else
	{
		if (FadeOutTimerHandle.IsValid())
		{
			FadeOutTimer = FadeOutTime * (1.0f - BlackFadeOut->ColorAndOpacity.A);
			GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		}

		GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &UStageChangeUIUnit::FadeInCoroutine, timerDeltaTime, true);
	}
}

void UStageChangeUIUnit::FadeOut(bool active)
{
	if (bIsFadeOut == active)
		return;

	bIsFadeOut = active;

	if (FadeOutTimerHandle.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);

	FLinearColor color = BlackFadeOut->ColorAndOpacity;
	if (bIsFadeOut)
	{
		color.A = 1.0f;
		BlackFadeOut->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		color.A = 0.0f;
		BlackFadeOut->SetVisibility(ESlateVisibility::Hidden);
	}
	BlackFadeOut->SetColorAndOpacity(color);
}

void UStageChangeUIUnit::FadeOutCoroutine()
{
	FadeOutTimer += timerDeltaTime;
	FLinearColor color = BlackFadeOut->ColorAndOpacity;
	color.A = FadeOutTimer / FadeOutTime;

	if (color.A >= 1.0f)
	{
		color.A = 1.0f;
		GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
	}
	BlackFadeOut->SetColorAndOpacity(color);
}

void UStageChangeUIUnit::FadeInCoroutine()
{
	FadeOutTimer += timerDeltaTime;
	FLinearColor color = BlackFadeOut->ColorAndOpacity;
	color.A = 1.0f - FadeOutTimer / FadeOutTime;

	if (color.A <= 0.0f)
	{
		color.A = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		BlackFadeOut->SetVisibility(ESlateVisibility::Hidden);
	}
	BlackFadeOut->SetColorAndOpacity(color);
}
