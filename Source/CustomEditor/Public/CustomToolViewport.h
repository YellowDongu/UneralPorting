#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"

class FEditorViewportClient;
class FWaypointEditorData;

class CUSTOMEDITOR_API SCustomToolViewport : public SEditorViewport
{
public:
    SLATE_BEGIN_ARGS(SCustomToolViewport) : _WaypointData(nullptr) { }
        SLATE_ARGUMENT(TSharedPtr<FWaypointEditorData>, WaypointData)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    
    TSharedPtr<class FCustomToolViewportClient> GetCustomViewportClient() const;

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:
    // 툴 전용 미리보기 월드
    TUniquePtr<class FPreviewScene> PreviewScene;

    // 뷰포트 동작 및 카메라 제어
    TSharedPtr<FEditorViewportClient> ViewportClient;
    TSharedPtr<FWaypointEditorData> WaypointData;
};