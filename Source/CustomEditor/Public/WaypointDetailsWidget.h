#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "WaypointEditorData.h"

class CUSTOMEDITOR_API SWaypointDetails : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SWaypointDetails) {}
        SLATE_ARGUMENT(TSharedPtr<FWaypointEditorData>, WaypointData)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TSharedPtr<FWaypointEditorData> WaypointData;

    float GetCoordinate(int32 Axis) const;
    float GetRotationAngle(int32 Axis) const;

    void SetCoordinate(float Value, int32 Axis);
    void SetRotationAngle(float Value, int32 Axis);
};