#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
class FWaypointEditorData;
class CUSTOMEDITOR_API SCustomToolEditorWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCustomToolEditorWidget) : _WaypointData(nullptr) {}
        SLATE_ARGUMENT(TSharedPtr<FWaypointEditorData>, WaypointData)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    TSharedPtr<class FCustomToolViewportClient> GetCustomViewportClient(void) const;
    float GetCoordinate(int32 Axis) const;
    void SetCoordinate(float Value, int32 Axis);

private:
    TSharedPtr<FWaypointEditorData> WaypointData;
    TSharedPtr<class SCustomToolViewport> ViewportWidget;
};