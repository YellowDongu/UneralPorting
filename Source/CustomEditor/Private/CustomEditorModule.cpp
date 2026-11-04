#include "CustomEditorModule.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "WaypointPathAsset.h"

//#include "WaypointEditorMode.h"
#include "CustomToolViewport.h"
#include "CustomToolEditorWidget.h"
#include "WaypointEditorData.h"
#include "WaypointListWidget.h"
#include "WaypointDetailsWidget.h"
#include "WaypointBatchEditWidget.h"

static const FName WaypointToolID(TEXT("CustomTool.WaypointList"));
//static const FName WaypointToolID(TEXT("Waypoint Tool"));

static const FName WaypointViewportTabId(TEXT("CustomTool.WaypointViewport"));  //deprecated
static const FName WaypointDetailsTabId(TEXT("CustomTool.WaypointDetails"));  //deprecated
static const FName CustomToolTabName(TEXT("CustomEditorToolTab"));  //deprecated

void FCustomEditorModule::StartupModule(void)
{
    WaypointData = MakeShared<FWaypointEditorData>();

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(WaypointToolID, FOnSpawnTab::CreateRaw(this, &FCustomEditorModule::SpawnWaypointWindow))
        .SetDisplayName(FText::FromString(TEXT("Waypoint Tool")));

    // 에디터 메뉴 등록
    //UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCustomEditorModule::RegisterMenus));
    return;

    //FGlobalTabmanager::Get()->RegisterNomadTabSpawner(WaypointViewportTabId, FOnSpawnTab::CreateRaw(this, &FCustomEditorModule::SpawnWaypointViewportTab))
    //    .SetDisplayName(FText::FromString(TEXT("Waypoint Viewport")));
    
    //FGlobalTabmanager::Get()->RegisterNomadTabSpawner(WaypointDetailsTabId, FOnSpawnTab::CreateRaw(this, &FCustomEditorModule::SpawnWaypointDetailsTab))
    //    .SetDisplayName(FText::FromString(TEXT("Waypoint Details")));
}

void FCustomEditorModule::ShutdownModule(void)
{
    CustomToolViewport.Reset();
    WaypointList.Reset();
    WaypointDetails.Reset();
    WaypointBatchEdit.Reset();

    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    //FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CustomToolTabName);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(WaypointToolID);
    WaypointData.Reset();

    if (WaypointEditorTabManager.IsValid())
        WaypointEditorTabManager->UnregisterAllTabSpawners();
    WaypointEditorTabManager.Reset();
}

void FCustomEditorModule::RegisterMenus(void)  //deprecated
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
    FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("CustomEditor"));

    Section.AddMenuEntry(TEXT("OpenCustomTool"), FText::FromString(TEXT("Custom Tool")), FText::FromString(TEXT("Open the custom editor tool")),
                         FSlateIcon(), FUIAction(FExecuteAction::CreateRaw(this, &FCustomEditorModule::OpenWaypointEditorTabs)));
}

void FCustomEditorModule::OpenCustomTool(void)  //deprecated
{
    FGlobalTabmanager::Get()->TryInvokeTab(CustomToolTabName);
}

void FCustomEditorModule::SaveWaypointEditorLayout()
{
    if (!WaypointEditorTabManager.IsValid())
        return;

    WaypointEditorTabManager->PersistLayout();
}

TSharedRef<SDockTab> FCustomEditorModule::SpawnWaypointWindow(const FSpawnTabArgs& Args)
{
    TSharedRef<SDockTab> MainTab = SNew(SDockTab).TabRole(ETabRole::NomadTab)
        .OnTabClosed_Lambda(
            [this](TSharedRef<SDockTab> ClosedTab)
            {
                SaveWaypointEditorLayout();
                WaypointEditorTabManager.Reset();
            });

    WaypointEditorTabManager = FGlobalTabmanager::Get()->NewTabManager(MainTab);

    static FString WaypointListName = TEXT("Waypoint List"); // FName("WaypointEditor.Inner.Viewport")
    static FString WaypointViewportName = TEXT("Waypoint Viewport"); // FName("WaypointEditor.Inner.List")
    static FString WaypointDetailsName = TEXT("Waypoint Details"); // FName("WaypointEditor.Inner.Details")
    static FString WaypointBatchName = TEXT("Waypoint Batch Edit");

    // 내부 탭 등록
    WaypointEditorTabManager->RegisterTabSpawner(FName(WaypointViewportName), FOnSpawnTab::CreateLambda(
        //WaypointEditorTabManager->RegisterTabSpawner(FName("WaypointEditor.Inner.Viewport"), FOnSpawnTab::CreateLambda(
        [this](const FSpawnTabArgs& InnerArgs)
        {
            return SNew(SDockTab)[SAssignNew(CustomToolViewport, SCustomToolViewport).WaypointData(WaypointData)];
            //return SNew(SDockTab)[SNew(SCustomToolViewport).WaypointData(WaypointData)];
        }));

    WaypointEditorTabManager->RegisterTabSpawner(FName(WaypointListName), FOnSpawnTab::CreateLambda(
        //WaypointEditorTabManager->RegisterTabSpawner(FName("WaypointEditor.Inner.List"), FOnSpawnTab::CreateLambda(
        [this](const FSpawnTabArgs& InnerArgs)
        {
            return SNew(SDockTab)[SAssignNew(WaypointList, SWaypointList).WaypointData(WaypointData)];
        }));

    WaypointEditorTabManager->RegisterTabSpawner(FName(WaypointDetailsName), FOnSpawnTab::CreateLambda(
        //WaypointEditorTabManager->RegisterTabSpawner(FName("WaypointEditor.Inner.Details"), FOnSpawnTab::CreateLambda(
        [this](const FSpawnTabArgs& InnerArgs)
        {
            return SNew(SDockTab)[SAssignNew(WaypointDetails, SWaypointDetails).WaypointData(WaypointData)];
        }));

    WaypointEditorTabManager->RegisterTabSpawner(FName(WaypointBatchName), FOnSpawnTab::CreateLambda(
        [this](const FSpawnTabArgs& InnerArgs)
        {
            return SNew(SDockTab)[SAssignNew(WaypointBatchEdit, SWaypointBatchEdit).WaypointData(WaypointData)];
        }));


    // 도킹 레이아웃 구성
    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(FName("WaypointEditor.Layout.v1"))->AddArea(
        FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)

        ->Split(FTabManager::NewStack()->AddTab(FName(WaypointListName), ETabState::OpenedTab)->SetSizeCoefficient(0.22f)) // 왼쪽: 목록
        ->Split(FTabManager::NewStack()->AddTab(FName(WaypointViewportName), ETabState::OpenedTab)->SetSizeCoefficient(0.56f)) // 중앙: 뷰포트
        ->Split(FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)->SetSizeCoefficient(0.22f) // 오른쪽: 상세 정보 + 일괄 편집
            ->Split(FTabManager::NewStack()->AddTab(FName(WaypointDetailsName), ETabState::OpenedTab)->SetSizeCoefficient(0.65f))
            ->Split(FTabManager::NewStack()->AddTab(FName(WaypointBatchName), ETabState::OpenedTab)->SetSizeCoefficient(0.35f)))
    );

    TSharedRef<SWidget> LayoutWidget = WaypointEditorTabManager->RestoreFrom(Layout, TSharedPtr<SWindow>()).ToSharedRef();
    //MainTab->SetContent(LayoutWidget);
    MainTab->SetContent(
        SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)

                + SHorizontalBox::Slot().AutoWidth().Padding(4.f)
                [
                    SNew(SButton).Text(FText::FromString(TEXT("Save")))
                        .OnClicked_Lambda([this]() { SaveWaypointAsset(); return FReply::Handled(); })
                ]

                + SHorizontalBox::Slot().AutoWidth().Padding(4.f)
                [
                    SNew(SButton).Text(FText::FromString(TEXT("Load")))
                        .OnClicked_Lambda([this]() { LoadWaypointAsset(); return FReply::Handled(); })
                ]
        ]
        + SVerticalBox::Slot().FillHeight(1.f)
            [
                LayoutWidget
            ]
        );


    return MainTab;
    /*
    return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SWaypointList).WaypointData(WaypointData)];
    return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SCustomToolViewport).WaypointData(WaypointData)];
    return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SWaypointDetails).WaypointData(WaypointData)];
    return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SCustomToolEditorWidget).WaypointData(WaypointData)];
    return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SCustomToolViewport)];
    return SNew(SDockTab).TabRole(ETabRole::NomadTab) // 테스트 UI 세팅
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(12.0f)[SNew(STextBlock).Text(FText::FromString(TEXT("Custom Editor Tool")))]
                + SVerticalBox::Slot().AutoHeight().Padding(12.0f)[SNew(SButton).Text(FText::FromString(TEXT("Test Button"))).OnClicked_Lambda([]() -> FReply { UE_LOG(LogTemp, Warning, TEXT("Custom Tool Button Clicked")); return FReply::Handled(); })]
        ];
    */
}

void FCustomEditorModule::OpenWaypointEditorTabs(void)
{
    FGlobalTabmanager::Get()->TryInvokeTab(WaypointToolID);
    /*
    FGlobalTabmanager::Get()->TryInvokeTab(WaypointViewportTabId);
    FGlobalTabmanager::Get()->TryInvokeTab(WaypointDetailsTabId);
    */
}

static const FString PathSaveSlotName = TEXT("LastAssetPath");

void FCustomEditorModule::SaveWaypointAsset(void)
{
    FSaveAssetDialogConfig Config;
    Config.DialogTitleOverride = FText::FromString(TEXT("Save Waypoint Path"));
    //Config.DefaultPath = TEXT("/Game/");
    LoadLastPath(PathSaveSlotName, Config.DefaultPath);
    Config.DefaultAssetName = TEXT("NewWaypointPath");
    Config.AssetClassNames.Add(UMovementPathAsset::StaticClass()->GetClassPathName());
    Config.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;   // 덮어쓰기 시 경고

    FContentBrowserModule& CB = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    const FString ObjectPath = CB.Get().CreateModalSaveAssetDialog(Config);
    if (ObjectPath.IsEmpty()) return; // 취소

    SaveLastPath(PathSaveSlotName, FPackageName::GetLongPackagePath(ObjectPath));
    UWaypointPathAsset::Save(FPackageName::ObjectPathToPackageName(ObjectPath), WaypointData.Get());
}

void FCustomEditorModule::LoadWaypointAsset(void)
{
    FOpenAssetDialogConfig Config;
    Config.DialogTitleOverride = FText::FromString(TEXT("Load Waypoint Path"));
    //Config.DefaultPath = TEXT("/Game/");
    LoadLastPath(PathSaveSlotName, Config.DefaultPath);
    Config.bAllowMultipleSelection = false;
    Config.AssetClassNames.Add(UWaypointPathAsset::StaticClass()->GetClassPathName());
    Config.AssetClassNames.Add(UMovementPathAsset::StaticClass()->GetClassPathName());

    FContentBrowserModule& CB = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    TArray<FAssetData> Picked = CB.Get().CreateModalOpenAssetDialog(Config);
    if (Picked.IsEmpty()) return; // 취소

    SaveLastPath(PathSaveSlotName, Picked);
    UWaypointPathAsset::Load(Picked[0].GetObjectPathString(), WaypointData.Get());
    if (WaypointList.IsValid())
        WaypointList->OnDataChanged();
}

void FCustomEditorModule::SaveLastPath(const FString& Path, const FString& SaveSlot)
{
    GConfig->SetString(TEXT("MyEditor"), *Path, *SaveSlot, GEditorPerProjectIni);
    //GConfig->SetString(TEXT("MyEditor"), TEXT("LastAssetPath"), *SaveSlot, GEditorPerProjectIni);

    GConfig->Flush(false, GEditorPerProjectIni);

}
void FCustomEditorModule::SaveLastPath(const FString& Path, const TArray<FAssetData>& SelectedAssets)
{
    if (SelectedAssets.IsEmpty())
        return;

    SaveLastPath(Path, SelectedAssets[0].PackagePath.ToString());
}

void FCustomEditorModule::LoadLastPath(const FString& Path, FString& LoadSlot)
{
    GConfig->GetString(TEXT("MyEditor"), *Path, LoadSlot, GEditorPerProjectIni);
    //GConfig->GetString(TEXT("MyEditor"), TEXT("LastAssetPath"), LoadSlot, GEditorPerProjectIni);

    if (LoadSlot.IsEmpty())
        LoadSlot = TEXT("/Game/");
}
