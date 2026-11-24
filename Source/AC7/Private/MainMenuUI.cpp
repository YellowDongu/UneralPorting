// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuUI.h"

#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameMaster.h"
#include "LevelLoader.h"
#include "BluePrintCollector.h"
#include "StageChangeUIUnit.h"
#include "UIHandler.h"
#include "SoundSubsystem.h"

void UMainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	TArray<UWidget*> children = MainListCanvas->GetAllChildren();
	UCanvasPanel* SelectBar = Cast<UCanvasPanel>(children[0]);
	UVerticalBox* ListPanel = Cast<UVerticalBox>(children[1]);
	MainListController.Initialize(SelectBar, MainListCanvas, ListPanel);
	MainListCanvas->SetVisibility(ESlateVisibility::Visible);
	CurrentListController = &MainListController;

	children = MissionListCanvas->GetAllChildren();
	SelectBar = Cast<UCanvasPanel>(children[0]);
	ListPanel = Cast<UVerticalBox>(children[1]);
	MissionListCanvas->SetVisibility(ESlateVisibility::Hidden);
	MissionListController.Initialize(SelectBar, MissionListCanvas, ListPanel);

	USoundSubsystem* sound = GetGameInstance()->GetSubsystem<USoundSubsystem>();
	Selected = sound->GetSound(TEXT("MenuSelected"));
	MenuChange = sound->GetSound(TEXT("MenuChange"));
	MenuCancel = sound->GetSound(TEXT("MenuCancel"));

	MainBGM = MakeShared<FSoundSet>();
	MainBGM->Initialize(TEXT("main"), TEXT("BGMChannel"), sound);


	USoundWave* SoundWave = Cast<USoundWave>(MainBGM->Sound);
	if (SoundWave != nullptr)
	{
		SoundWave->bLooping = 1;
		//SoundWave->Volume = 0.5f;
	}
}

void UMainMenuUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMainMenuUI::NativeTick(const FGeometry& InGeometry, float DeltaTime)
{
	Super::NativeTick(InGeometry, DeltaTime);

	if (!bActive) return;

	APlayerController* controller = GetWorld()->GetFirstPlayerController();

	bool up = controller->WasInputKeyJustPressed(EKeys::W) || controller->WasInputKeyJustPressed(EKeys::Up);
	bool down = controller->WasInputKeyJustPressed(EKeys::S) || controller->WasInputKeyJustPressed(EKeys::Down);
	bool select = controller->WasInputKeyJustPressed(EKeys::Enter) || controller->WasInputKeyJustPressed(EKeys::SpaceBar);
	bool deselect = controller->WasInputKeyJustPressed(EKeys::Escape);

	int delta = (down ? 1 : 0) - (up ? 1 : 0);

	if (delta != 0)
	{
		UGameplayStatics::PlaySound2D(this, MenuChange);
		CurrentListController->ChangeIndex(CurrentListController->GetCurrentIndex() + delta);
	}

	if (select)
		Select();
	if(deselect)
		Deselect();
}

void UMainMenuUI::Select()
{
	UGameplayStatics::PlaySound2D(this, Selected);
	
	switch (Phase)
	{
		case 0:
		{
			switch (CurrentListController->GetCurrentIndex())
			{
			case 0:
				MainListCanvas->SetVisibility(ESlateVisibility::Hidden);

				MissionListCanvas->SetVisibility(ESlateVisibility::Visible);
				CurrentListController = &MissionListController;
				CurrentListController->ChangeIndex(0);
				Phase++;
				break;
			case 1:
				break;
			case 2:
				UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, false);
				//FGenericPlatformMisc::RequestExit(false);
				break;
			default:
				break;
			}

			break;
		}
		case 1:
			TransitLevel();
			break;
		default:
		{
			MainListCanvas->SetVisibility(ESlateVisibility::Visible);
			//WeaponListCanvas->SetVisibility(ESlateVisibility::Hidden);

			CurrentListController = &MainListController;
			CurrentListController->ChangeIndex(0);
			Phase = 0;
			break;
		}
	}

}

void UMainMenuUI::Deselect(void)
{
	UGameplayStatics::PlaySound2D(this, MenuCancel);

	switch (Phase)
	{
	case 0:
	{
		AUIHandler* handler = GetWorld()->GetFirstPlayerController()->GetHUD<AUIHandler>();
		handler->GetStageChangeUIUnit()->ReturnMainMenu();
		break;
	}
	case 1:
	{
		MainListCanvas->SetVisibility(ESlateVisibility::Hidden);
		//WeaponListCanvas->SetVisibility(ESlateVisibility::Visible);
		//CurrentListController = &WeaponListController;
		//CurrentListController->ChangeIndex(0);
		Phase++;
		break;
	}
	default:
	{
		MainListCanvas->SetVisibility(ESlateVisibility::Visible);
		//WeaponListCanvas->SetVisibility(ESlateVisibility::Hidden);

		CurrentListController = &MainListController;
		CurrentListController->ChangeIndex(0);
		Phase = 0;
		break;
	}
	}
}

void UMainMenuUI::TransitLevel(void)
{
	//if (TimerHandle.IsValid())
	//	return;
	//auto delegate = [this, stageChangeUIUnit, levelLoader, bluePrintCollector]() -> void
	//	{
	//		if (!stageChangeUIUnit->IsFadingOut())
	//		{
	//			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	//			levelLoader->ChangeLevel(bluePrintCollector->GetSelectLevel());
	//		}
	//	};
	//GetWorld()->GetTimerManager().SetTimer(TimerHandle, delegate, 1.0f / 60.0f, true);

	constexpr int vehicleCount = static_cast<int>(EVehicleID::AirEND) + static_cast<int>(EVehicleID::GroundEND) - static_cast<int>(EVehicleID::Ground) - 2;
	UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
	UBluePrintCollector* bluePrintCollector = gameMaster->GetCollector();
	ULevelLoader* levelLoader = gameMaster->GetSubsystem<ULevelLoader>();
	TWeakObjectPtr<UStageChangeUIUnit> stageChangeUIUnit = GetWorld()->GetFirstPlayerController()->GetHUD<AUIHandler>()->CreateStageChangeUIUnit();

	check(gameMaster);
	check(bluePrintCollector);
	check(levelLoader);

	FString levelName = gameMaster->GetMissionName(CurrentListController->GetCurrentIndex());
	if (levelName.IsEmpty())
		return;

	Deactive();
	UE_LOG(LogTemp, Error, TEXT("[UMainMenuUI::TransitLevel] %s Level reserved"), *levelName);
	gameMaster->ReserveLevel(levelName);
	stageChangeUIUnit->FadeOut(true, 2.5f);

	TArray<FSoftObjectPath> pathList;
	pathList.Reserve(vehicleCount);

	for (int i = static_cast<int>(EVehicleID::Aircraft) + 1; i < static_cast<int>(EVehicleID::AirEND); i++)
	{
		TSubclassOf<AVehicleBase> bluePrint = bluePrintCollector->GetVehicleBluePrint(static_cast<EVehicleID>(i));
		if (bluePrint)
			pathList.Emplace(bluePrint.Get());
	}

	for (int i = static_cast<int>(EVehicleID::Ground) + 1; i < static_cast<int>(EVehicleID::GroundEND); i++)
	{
		TSubclassOf<AVehicleBase> bluePrint = bluePrintCollector->GetVehicleBluePrint(static_cast<EVehicleID>(i));
		if (bluePrint)
			pathList.Emplace(bluePrint.Get());
	}


	levelLoader->AssetAsyncLoad(TEXT("VehicleSet"), pathList, true);
	levelLoader->AssetAsyncLoad(TEXT("SelectLevel"), gameMaster->GetReserveLevel().ToSoftObjectPath(), true);
	stageChangeUIUnit->ExitSelect(UStageChangeUIUnit::FCallBackFunction::FDelegate::CreateWeakLambda(
		this, [levelLoader, bluePrintCollector]() -> void
		{
			levelLoader->ChangeLevel(bluePrintCollector->GetSelectLevel());
			levelLoader->Unload(TEXT("SelectLevel")); // 나중에 따로 언로드하지 않게 앵커를 없애서 레벨 전환이 되면 자연스럽게 없어지게 하도록
		}));
}

void UMainMenuUI::Active(void)
{
	bActive = true;
	MainBGM->Play();
}

void UMainMenuUI::Deactive(void)
{
	bActive = false;
	MainBGM->Stop();
}