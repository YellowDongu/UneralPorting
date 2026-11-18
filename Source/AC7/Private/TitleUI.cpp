// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleUI.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "UIHandler.h"
#include "StageChangeUIUnit.h"
#include "MainMenuUI.h"
#include "GameMaster.h"
#include "BluePrintCollector.h"

void UTitleUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UTitleUI::NativeConstruct()
{
	Super::NativeConstruct();

	TurnToMainMenu = GetGameInstance<UGameMaster>()->GetCollector()->GetSound(TEXT("TurnToMainMenu"));
	FrontUI = GetWorld()->GetFirstPlayerController()->GetHUD<AUIHandler>()->CreateStageChangeUIUnit();
	Flag = -1;

	Sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			check(TitleCanvas);
			check(PreTitleCanvas);
			check(Title);
			check(PreTitle1);
			check(NextText);
			check(FrontUI.Get());

			TitleCanvas->SetVisibility(ESlateVisibility::Hidden);
			PreTitleCanvas->SetVisibility(ESlateVisibility::Visible);
			return true;
		}));

	Sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if(accumulatedTime <= 5.0f)
				return false;
			FrontUI->FadeOut(true, 2.5f);
			return true;
		}));

	Sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this](float accumulatedTime)->bool
		{
			if (FrontUI->IsFadingOut())
				return false;

			TitleCanvas->SetVisibility(ESlateVisibility::Visible);
			PreTitleCanvas->SetVisibility(ESlateVisibility::Hidden);
			FrontUI->FadeOut(false, 0.5f);
			return true;
		}));

	Sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this, controller = GetWorld()->GetFirstPlayerController()](float accumulatedTime)->bool
		{
			if (FrontUI->IsFadingOut())
				return false;

			if (controller->WasInputKeyJustPressed(EKeys::Enter))
			{
				UGameplayStatics::PlaySound2D(this, TurnToMainMenu);
				return true;
			}

			return false;
		}));

	Sequence.AddSequence(FSequenceTask::FSequenceFunction::CreateWeakLambda(
		this, [this, controller = GetWorld()->GetFirstPlayerController()](float accumulatedTime)->bool
		{
			if (accumulatedTime < 2.5f)
				return false;

			TWeakObjectPtr<UMainMenuUI> ui = controller->GetHUD<AUIHandler>()->CreateMainMenuUI();
			check(ui.Get());
			ui->Active();

			RemoveFromParent();
			return true;

		}));

	Sequence.currentIndex = 0;
}

void UTitleUI::NativeTick(const FGeometry& myGeometry, float DeltaTime) { Super::NativeTick(myGeometry, DeltaTime); Sequence.Update(DeltaTime); }
