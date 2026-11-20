// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectPanel.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

#include "GameMaster.h"
#include "DisplayCase.h"
#include "LevelLoader.h"
#include "Aircraft.h"
#include "BluePrintCollector.h"
#include "UIHandler.h"
#include "StageChangeUIUnit.h"
#include "FireControlSystem.h"
#include "Missile.h"

void USelectPanel::NativeConstruct()
{
	Super::NativeConstruct();

	WeaponListController.Initialize(WeaponListSelectedBar, WeaponListCanvas, WeaponListPanel);
	CurrentListController = &VehicleListController;

	VehicleListCanvas->SetVisibility(ESlateVisibility::Visible);
	WeaponListCanvas->SetVisibility(ESlateVisibility::Hidden);

	UBluePrintCollector* collector = GetGameInstance<UGameMaster>()->GetCollector();
	Selected = collector->GetSound(TEXT("MenuSelected"));
	MenuChange = collector->GetSound(TEXT("MenuChange"));
	MenuCancel = collector->GetSound(TEXT("MenuCancel"));
	AircraftSelected = collector->GetSound(TEXT("AircraftSelected"));
	MainBGM = collector->GetSound(TEXT("Select"));
	USoundWave* SoundWave = Cast<USoundWave>(MainBGM);
	if (SoundWave != nullptr)
	{
		SoundWave->bLooping = 1;
		//SoundWave->Volume = 0.5f;
	}
}

void USelectPanel::LinkDisplayCase(ADisplayCase* target)
{
	if (target == nullptr)
		return;

	target->Initialize();
	ChangeSelectDelegate = FChangeDisplay::CreateUObject(target, &ADisplayCase::ChangeDisplay);
	const TArray<TWeakObjectPtr<AVehicleBase>>& vehicleList = target->GetVehicleList();
	SelectableVehicleList = &vehicleList;

	VehicleListController.Initialize(VehicleListSelectedBar, VehicleListCanvas, VehicleListPanel);
	VehicleListController.ChangeChildsText(
		[vehicleList](TArray<TObjectPtr<UTextBlock>>& TextBlockList, int& MaxChildCount) -> void
		{
			int max = FMath::Min(TextBlockList.Num(), vehicleList.Num());
			int i = 0;
			for (; i < max; i++)
			{
				if (vehicleList[i] == nullptr)
					continue;

				TextBlockList[i]->SetText(FText::FromString(vehicleList[i]->GetVehicleName()));
				TextBlockList[i]->SetVisibility(ESlateVisibility::Visible);
			}

			MaxChildCount = i;
			max = TextBlockList.Num();

			for (; i < max; i++)
				TextBlockList[i]->SetVisibility(ESlateVisibility::Hidden);
		}
	);
}

void USelectPanel::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (!bActive) return;

	APlayerController* controller = GetWorld()->GetFirstPlayerController();

	bool up = controller->WasInputKeyJustPressed(EKeys::W) || controller->WasInputKeyJustPressed(EKeys::Up);
	bool down = controller->WasInputKeyJustPressed(EKeys::S) || controller->WasInputKeyJustPressed(EKeys::Down);
	bool select = controller->WasInputKeyJustPressed(EKeys::Enter) || controller->WasInputKeyJustPressed(EKeys::SpaceBar);
	bool deselect = controller->WasInputKeyJustPressed(EKeys::Escape) || controller->WasInputKeyJustPressed(EKeys::Z);

	int delta = (down ? 1 : 0) - (up ? 1 : 0);
	if (delta != 0)
	{
		UGameplayStatics::PlaySound2D(this, MenuChange);
		CurrentListController->ChangeIndex(CurrentListController->GetCurrentIndex() + delta);
		if (Phase == 0)
		{
			EVehicleID vehicleID = (*SelectableVehicleList)[CurrentListController->GetCurrentIndex()]->GetVehicleID();
			ChangeSelectDelegate.Execute(vehicleID);
		}
	}

	if (select)
	{
		Select();
		//StageChangeUI->FadeOut(!StageChangeUI->IsFadeOut(), 2.5f);
	}

	if (deselect)
		Deselect();
}

void USelectPanel::Select()
{
	int Index = CurrentListController->GetCurrentIndex();
	if (!CurrentListController->IsValidIndex(Index)) return;

	switch (Phase)
	{
	case 0:
		UpdateWeaponSelection();
		Phase++;

		if (WeaponListController.GetMaxIndex() == 0)
		{
			Phase++;
			EnterStage();
		}

		UGameplayStatics::PlaySound2D(this, Selected);
		break;
	case 1:
	{
		Deactive();
		EnterStage();
		//UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
		//FPlayerSpawnInfomation info;
		//info.VehicleID = (*SelectableVehicleList)[VehicleListController.GetCurrentIndex()]->GetVehicleID();
		//info.SpecialWeaponIndex = CurrentListController->GetCurrentIndex();
		//
		//// DEBUG/SAFEMODE
		////info.VehicleID = EVehicleID::F15E;
		////info.SpecialWeaponIndex = -1;
		////gameMaster->ReserveLevel(FString(TEXT("MS01")));
		//
		//if (gameMaster->GetReserveLevel() == nullptr)
		//{
		//	UE_LOG(LogTemp, Error, TEXT("[ERROR] No level selected"));
		//	gameMaster->ReserveLevel(FString(TEXT("MS01")));
		//}
		//
		//gameMaster->SetPlayerInfomation(info);
		//
		//EnterMission.ExecuteIfBound();
		break;
	}
	default:
		VehicleListCanvas->SetVisibility(ESlateVisibility::Visible);
		WeaponListCanvas->SetVisibility(ESlateVisibility::Hidden);

		CurrentListController = &VehicleListController;
		CurrentListController->ChangeIndex(0);
		Phase = 0;
		break;
	}
}

void USelectPanel::Deselect()
{
	UGameplayStatics::PlaySound2D(this, MenuCancel);

	switch (Phase)
	{
	case 0:
	{
		TWeakObjectPtr<UStageChangeUIUnit> stageChangeUIUnit = GetWorld()->GetFirstPlayerController()->GetHUD<AUIHandler>()->CreateStageChangeUIUnit();
		ULevelLoader* levelLoader = GetGameInstance()->GetSubsystem<ULevelLoader>();
		levelLoader->Unload(TEXT("VehicleSet"));
		levelLoader->Unload(TEXT("SelectLevel"));
		stageChangeUIUnit->ReturnMainMenu();
		Deactive();
		Phase = 0;
	}
	break;
	case 1:
	{
		VehicleListCanvas->SetVisibility(ESlateVisibility::Visible);
		WeaponListCanvas->SetVisibility(ESlateVisibility::Hidden);

		CurrentListController = &VehicleListController;
		CurrentListController->ChangeIndex(0);
		Phase--;
	}
	break;
	default:
		VehicleListCanvas->SetVisibility(ESlateVisibility::Visible);
		WeaponListCanvas->SetVisibility(ESlateVisibility::Hidden);

		CurrentListController = &VehicleListController;
		CurrentListController->ChangeIndex(0);
		Phase = 0;

		break;
	}
}

void USelectPanel::ReturnMenu(void)
{
	for (FConstPlayerControllerIterator iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
	{
		AUIHandler* hud = (*iterator)->GetHUD<AUIHandler>();

		hud->GetStageChangeUIUnit()->FadeOut(false);
		hud->GetStageChangeUIUnit()->ReturnMainMenu();
	}
}

void USelectPanel::EnterStage(void)
{
	UGameplayStatics::PlaySound2D(this, AircraftSelected);
	UGameMaster* gameMaster = GetGameInstance<UGameMaster>();
	FPlayerSpawnInfomation info;

	const TWeakObjectPtr<AVehicleBase>& vehicle = (*SelectableVehicleList)[VehicleListController.GetCurrentIndex()];
	TWeakObjectPtr<AAircraft> aircraft = Cast<AAircraft>(vehicle);
	TWeakObjectPtr<AMissile> weapon = WeaponListController.GetMaxIndex() == 0 ? nullptr : aircraft->GetFireControlSystem()->GetWeapon(WeaponListController.GetCurrentIndex());

	info.VehicleID = (*SelectableVehicleList)[VehicleListController.GetCurrentIndex()]->GetVehicleID();
	info.SpecialWeapon = weapon.IsValid() ? weapon->GetWeaponID() : EWeaponID::END;

	if (gameMaster->GetReserveLevel().IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("[ERROR] No level selected, Select Default Level"));
		gameMaster->ReserveLevel(FString(TEXT("MS01")));
	}

	gameMaster->SetPlayerInfomation(info);
	EnterMission.ExecuteIfBound();
}

void USelectPanel::UpdateWeaponSelection(void)
{
	/*
	constexpr int MaxWeaponCount = 5;
	AAircraft* vehicle = StaticCast<AAircraft*>((*SelectableVehicleList)[VehicleListController.GetCurrentIndex()].Get());
	if (vehicle == nullptr) return;
	TWeakObjectPtr<UFireControlSystem> fcs = vehicle->GetFireControlSystem();
	if (fcs == nullptr) return;

	TArray<EWeaponID> idList;
	TArray<UTextBlock*> textBlockList;

	idList.Reserve(MaxWeaponCount);
	textBlockList.Reserve(MaxWeaponCount);

	for (int i = 0; i < MaxWeaponCount; i++)
	{
		TWeakObjectPtr<AMissile> missile = fcs->GetWeapon(i);
		if (missile == nullptr)
			continue;

		idList.Add(missile->GetWeaponID());
	}
	
	for (auto& item : WeaponListPanel->GetAllChildren())
	{
		UTextBlock* textBlock = Cast<UTextBlock>(item);
		if (textBlock != nullptr)
			textBlockList.Add(textBlock);
	}

	for (int i = 0; i < MaxWeaponCount; i++)
	{
		if (idList.Num() <= i)
		{
			for (; i < textBlockList.Num(); i++)
				textBlockList[i]->SetVisibility(ESlateVisibility::Hidden);

			break;
		}

		if (textBlockList.Num() <= i || idList.Num() <= i)
			break;

		textBlockList[i]->SetVisibility(ESlateVisibility::Visible);
		textBlockList[i]->SetText(FText::FromString(WeaponIDToString(idList[i])));
	}

	WeaponListController.ChangeChildCount(idList.Num());
	*/

	VehicleListCanvas->SetVisibility(ESlateVisibility::Hidden);
	WeaponListCanvas->SetVisibility(ESlateVisibility::Visible);

	CurrentListController = &WeaponListController;

	AAircraft* vehicle = StaticCast<AAircraft*>((*SelectableVehicleList)[VehicleListController.GetCurrentIndex()].Get());
	if (vehicle == nullptr) return;
	TWeakObjectPtr<UFireControlSystem> fcs = vehicle->GetFireControlSystem();
	if (fcs == nullptr) return;

	WeaponListController.ChangeChildsText(
		[fcs, this](TArray<TObjectPtr<class UTextBlock>>& TextBlockList, int& MaxChildCount) -> void
		{
			constexpr int MaxWeaponCount = 5;

			TArray<EWeaponID> idList;
			idList.Reserve(MaxWeaponCount);

			for (int i = 0; i < MaxWeaponCount; i++)
			{
				TWeakObjectPtr<AMissile> missile = fcs->GetWeapon(i);
				if (missile == nullptr)
					continue;

				idList.Add(missile->GetWeaponID());
			}

			MaxChildCount = idList.Num();

			for (int i = 0; i < MaxWeaponCount; i++)
			{
				if (idList.Num() <= i)
				{
					for (; i < TextBlockList.Num(); i++)
						TextBlockList[i]->SetVisibility(ESlateVisibility::Hidden);

					break;
				}

				if (TextBlockList.Num() <= i || idList.Num() <= i)
					break;

				TextBlockList[i]->SetVisibility(ESlateVisibility::Visible);
				TextBlockList[i]->SetText(FText::FromString(WeaponIDToString(idList[i])));
			}
		}
	);

	WeaponListController.ChangeIndex(0);
}

//void USelectPanel::LinkDisplayCase(ADisplayCase* target)
//{
//	if (target == nullptr)
//		return;
//
//	target->Initialize();
//	ChangeSelectDelegate = FChangeDisplay::CreateUObject(target, &ADisplayCase::ChangeDisplay);
//	auto list = VehicleListPanel->GetAllChildren();
//	const TArray<TWeakObjectPtr<AVehicleBase>>& vehicleList = target->GetVehicleList();
//	SelectableVehicleList = &vehicleList;
//	TArray<UTextBlock*> textBlockList;
//	textBlockList.Reserve(list.Num());
//
//	for (auto& item : list)
//	{
//		UTextBlock* textBlock = Cast<UTextBlock>(item);
//		if (textBlock != nullptr)
//			textBlockList.Add(textBlock);
//	}
//
//	int max = FMath::Min(textBlockList.Num(), vehicleList.Num());
//	int i;
//	for (i = 0; i < max; i++)
//	{
//		if (vehicleList[i] == nullptr)
//			continue;
//
//		textBlockList[i]->SetText(FText::FromString(vehicleList[i]->GetVehicleName()));
//	}
//
//	max = textBlockList.Num();
//	for (; i < max; i++)
//		textBlockList[i]->RemoveFromParent();
//
//	VehicleListController.Initialize(VehicleListSelectedBar, VehicleListCanvas, VehicleListPanel);
//}