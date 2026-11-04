#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "WaypointEditorData.h"

// 모든 웨이포인트를 일괄 조작하는 패널 (좌표 일괄 이동, 좌표 일괄 스케일)
class CUSTOMEDITOR_API SWaypointBatchEdit : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SWaypointBatchEdit) {}
        SLATE_ARGUMENT(TSharedPtr<FWaypointEditorData>, WaypointData)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TSharedPtr<FWaypointEditorData> WaypointData;

    FVector PositionDelta = FVector::ZeroVector; // Apply 전까지 임시로 들고 있는 이동량
    FVector ScaleFactor = FVector::OneVector;    // Apply 전까지 임시로 들고 있는 스케일 (기준점은 로컬 원점)

    // Target 벡터의 Axis 성분을 편집하는 한 줄(라벨 + 스핀박스)을 만든다
    TSharedRef<SWidget> MakeAxisRow(const FText& Label, FVector* Target, int32 Axis, float MinValue, float MaxValue, float Delta);

    bool CanApplyOffset(void) const;
    FReply OnApplyOffsetClicked(void);
    FReply OnResetOffsetClicked(void);

    bool CanApplyScale(void) const;
    FReply OnApplyScaleClicked(void);
    FReply OnResetScaleClicked(void);
};
