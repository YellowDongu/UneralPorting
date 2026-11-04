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
}

void UTitleUI::NativeTick(const FGeometry& myGeometry, float DeltaTime)
{
	switch (Flag)
	{
	case 0:
		Timer -= DeltaTime;

		if (Timer > 0.0f)
			break;

		FrontUI->FadeOut(true, 2.5f);
		Flag++;
		break;
	case 1:
		if (FrontUI->IsFadingOut())
			break;

		TitleCanvas->SetVisibility(ESlateVisibility::Visible);
		PreTitleCanvas->SetVisibility(ESlateVisibility::Hidden);
		FrontUI->FadeOut(false, 0.5f);
		Flag++;
		break;
	case 2:
		if (FrontUI->IsFadingOut())
			break;

		if (GetWorld()->GetFirstPlayerController()->WasInputKeyJustPressed(EKeys::Enter))
		{
			Timer = 2.5f;
			Flag++;
			UGameplayStatics::PlaySound2D(this, TurnToMainMenu);
		}
	case 3:
		Timer -= DeltaTime;
		if (Timer <= 0.0f)
		{
			TWeakObjectPtr<UMainMenuUI> ui = GetWorld()->GetFirstPlayerController()->GetHUD<AUIHandler>()->CreateMainMenuUI();
			check(ui.Get());
			ui->Active();

			RemoveFromParent();
			Flag++;
		}
		break;
	default: // ÃÊ±âÈ­
		check(TitleCanvas);
		check(PreTitleCanvas);
		check(Title);
		check(PreTitle1);
		check(NextText);
		check(FrontUI.Get());

		Timer = 5.0f;
		Flag = 0;
		TitleCanvas->SetVisibility(ESlateVisibility::Hidden);
		PreTitleCanvas->SetVisibility(ESlateVisibility::Visible);
		break;
	}
}
