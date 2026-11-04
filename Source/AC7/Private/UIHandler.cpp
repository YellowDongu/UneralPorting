// Fill out your copyright notice in the Description page of Project Settings.


#include "UIHandler.h"
#include "FrontHUD.h"
#include "PlayerPilot.h"
#include "SelectPanel.h"
#include "StageChangeUIUnit.h"
#include "TitleUI.h"
#include "MainMenuUI.h"

bool AUIHandler::bFirstTime{ true };

void AUIHandler::InitializeTitle()
{
    CreateUI<UMainMenuUI>(MainMenuUIBlueprint, MainMenuUI);
    check(MainMenuUI.Get());

    if (StageChangeUIUnit == nullptr)
    {
        CreateStageChangeUIUnit();
        check(StageChangeUIUnit.Get());
    }

    if (bFirstTime)
    {
        CreateUI<UTitleUI>(TitleUIBlueprint, TitleUI);
        check(TitleUI.Get());
        MainMenuUI->Deactive();
        bFirstTime = false;
    }
    else
    {
        StageChangeUIUnit->FadeOut(true);
        MainMenuUI->Active();
        StageChangeUIUnit->FadeOut(false, 0.5f);
    }
}

void AUIHandler::InitializeSelectUI(USelectPanel*& output)
{
    CreateUI(SelectPanelBlueprint, SelectPanel);
    check(SelectPanel.Get());

    if (StageChangeUIUnit == nullptr)
    {
        CreateStageChangeUIUnit();
        check(StageChangeUIUnit.Get());
    }

    output = SelectPanel.Get();

    SelectPanel->LinkFunction(USelectPanel::FVoidDelegate::CreateUObject(StageChangeUIUnit.Get(), &UStageChangeUIUnit::EnterMission));
    StageChangeUIUnit->EnterSelect(UStageChangeUIUnit::FCallBackFunction::FDelegate::CreateUObject(SelectPanel.Get(), &USelectPanel::Active));
    //SelectPanel->LinkFunction(USelectPanel::FVoidDelegate::FDelegate::CreateUObject(StageChangeUI.Get(), &UStageChangeUIUnit::EnterMission));
}

void AUIHandler::PostInitializeComponents(void)
{
    Super::PostInitializeComponents();

    CreateStageChangeUIUnit();
}

void AUIHandler::BeginPlay(void)
{
    Super::BeginPlay();

    if (StageChangeUIUnit == nullptr)
    {
        check(StageChangeUIUnit.Get());
    }
}

void AUIHandler::SetFadeOut(bool value) { StageChangeUIUnit->FadeOut(value); }
void AUIHandler::SetFadeOut(bool value, float floatValue) { StageChangeUIUnit->FadeOut(value, floatValue); }

template<typename Type>
void AUIHandler::CreateUI(TSubclassOf<Type>& blueprint, TWeakObjectPtr<Type>& output, int order)
{
    static_assert(std::is_base_of_v<UUserWidget, Type>, "Only for UserWidget's function, do not use this function");

    if (output.IsValid())
        return;

    check(blueprint);
    check(GetWorld());

    output = CreateWidget<Type>(GetWorld(), blueprint);
    check(output.Get());
    output->AddToViewport(order);
}
