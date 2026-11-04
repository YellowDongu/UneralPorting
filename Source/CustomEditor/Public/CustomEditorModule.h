#pragma once

#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#include "EditorModeRegistry.h"


#define LOCTEXT_NAMESPACE "CustomEditor"

class CUSTOMEDITOR_API FCustomEditorModule : public IModuleInterface
{
public:
    FCustomEditorModule() = default;
    ~FCustomEditorModule() = default;

    virtual void StartupModule(void) override;
    virtual void ShutdownModule(void) override;

    TSharedPtr<class FWaypointEditorData> GetWaypointData(void) const { return WaypointData; }
private:
    void RegisterMenus(void); // 메뉴에 툴 실행 항목 추가
    void OpenCustomTool(void); // 메뉴 클릭 시 탭 열기
    void OpenWaypointEditorTabs(void);
    void SaveWaypointEditorLayout(void);
    void SaveWaypointAsset(void);
    void LoadWaypointAsset(void);
    TSharedRef<SDockTab> SpawnWaypointWindow(const FSpawnTabArgs& Args);
    void SaveLastPath(const FString& Path, const FString& SaveSlot);
    void SaveLastPath(const FString& Path, const TArray<FAssetData>& SaveSlot);
    void LoadLastPath(const FString& Path, FString& LoadSlot);

    TSharedPtr<class FWaypointEditorData> WaypointData;
    TSharedPtr<FTabManager> WaypointEditorTabManager;

    TSharedPtr<class SCustomToolViewport> CustomToolViewport{ nullptr };
    TSharedPtr<class SWaypointList> WaypointList{ nullptr };
    TSharedPtr<class SWaypointDetails> WaypointDetails{ nullptr };
    TSharedPtr<class SWaypointBatchEdit> WaypointBatchEdit{ nullptr };

    FString LastSavePath, LastLoadPath;
};

//IMPLEMENT_MODULE(FDefaultModuleImpl, CustomEditor)
IMPLEMENT_MODULE(FCustomEditorModule, CustomEditor)

#undef LOCTEXT_NAMESPACE