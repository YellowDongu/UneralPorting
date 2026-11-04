#include "WaypointBatchEditWidget.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SWaypointBatchEdit::Construct(const FArguments& InArgs)
{
    static constexpr float PositionLimitValue{ 10000.0f };
    static constexpr float ScaleLimitValue{ 100.0f };
    static constexpr float AngleLimitValue{ 180.0f };

    WaypointData = InArgs._WaypointData;

    ChildSlot
        [
            SNew(SBorder).Padding(8.0f)
                [
                    // 패널 높이가 작아도 전체 내용을 볼 수 있도록 스크롤 박스로 감싼다
                    SNew(SScrollBox)

                        + SScrollBox::Slot()
                        [
                            SNew(SVerticalBox)

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Batch Edit")))
                                ]

                                // ---------------- Offset ----------------
                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Offset All Positions")))
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                                [
                                    MakeAxisRow(FText::FromString(TEXT("X")), &PositionDelta, 0, -PositionLimitValue, PositionLimitValue, 1.0f)
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                                [
                                    MakeAxisRow(FText::FromString(TEXT("Y")), &PositionDelta, 1, -PositionLimitValue, PositionLimitValue, 1.0f)
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                                [
                                    MakeAxisRow(FText::FromString(TEXT("Z")), &PositionDelta, 2, -PositionLimitValue, PositionLimitValue, 1.0f)
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 16.0f)
                                [
                                    SNew(SHorizontalBox)

                                        + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                        [
                                            SNew(SButton).Text(FText::FromString(TEXT("Apply")))
                                                .IsEnabled_Raw(this, &SWaypointBatchEdit::CanApplyOffset)
                                                .OnClicked(this, &SWaypointBatchEdit::OnApplyOffsetClicked)
                                        ]

                                        + SHorizontalBox::Slot().AutoWidth()
                                        [
                                            SNew(SButton).Text(FText::FromString(TEXT("Reset")))
                                                .OnClicked(this, &SWaypointBatchEdit::OnResetOffsetClicked)
                                        ]
                                ]

                                // ---------------- Scale ----------------
                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Scale All Positions (Pivot: Local 0,0,0)")))
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                                [
                                    MakeAxisRow(FText::FromString(TEXT("X")), &ScaleFactor, 0, -ScaleLimitValue, ScaleLimitValue, 0.01f)
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                                [
                                    MakeAxisRow(FText::FromString(TEXT("Y")), &ScaleFactor, 1, -ScaleLimitValue, ScaleLimitValue, 0.01f)
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                                [
                                    MakeAxisRow(FText::FromString(TEXT("Z")), &ScaleFactor, 2, -ScaleLimitValue, ScaleLimitValue, 0.01f)
                                ]

                                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
                                [
                                    SNew(SHorizontalBox)

                                        + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                        [
                                            SNew(SButton).Text(FText::FromString(TEXT("Apply")))
                                                .IsEnabled_Raw(this, &SWaypointBatchEdit::CanApplyScale)
                                                .OnClicked(this, &SWaypointBatchEdit::OnApplyScaleClicked)
                                        ]

                                        + SHorizontalBox::Slot().AutoWidth()
                                        [
                                            SNew(SButton).Text(FText::FromString(TEXT("Reset")))
                                                .OnClicked(this, &SWaypointBatchEdit::OnResetScaleClicked)
                                        ]
                                ]
                        ]
                ]
        ];
}

TSharedRef<SWidget> SWaypointBatchEdit::MakeAxisRow(const FText& Label, FVector* Target, int32 Axis, float MinValue, float MaxValue, float Delta)
{
    // Target은 이 위젯의 멤버를 가리키므로 스핀박스(이 위젯의 자식)보다 수명이 길다
    return SNew(SHorizontalBox)

        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
        [
            SNew(STextBlock).Text(Label)
        ]

        + SHorizontalBox::Slot().FillWidth(1.0f)
        [
            SNew(SSpinBox<float>).MinValue(MinValue).MaxValue(MaxValue).Delta(Delta)
                .Value_Lambda([Target, Axis]()
                              {
                                  return static_cast<float>((*Target)[Axis]);
                              })
                .OnValueChanged_Lambda([Target, Axis](float Value)
                                       {
                                           (*Target)[Axis] = Value; // 여기서는 값만 저장하고, 실제 적용은 Apply 버튼에서 한 번만 수행
                                       })
        ];
}

bool SWaypointBatchEdit::CanApplyOffset(void) const
{
    return WaypointData.IsValid() && !WaypointData->GetWaypointList().IsEmpty() && !PositionDelta.IsNearlyZero();
}

FReply SWaypointBatchEdit::OnApplyOffsetClicked(void)
{
    if (WaypointData.IsValid())
        WaypointData->OffsetAllPositions(PositionDelta);

    return FReply::Handled();
}

FReply SWaypointBatchEdit::OnResetOffsetClicked(void)
{
    PositionDelta = FVector::ZeroVector;
    return FReply::Handled();
}

bool SWaypointBatchEdit::CanApplyScale(void) const
{
    return WaypointData.IsValid() && !WaypointData->GetWaypointList().IsEmpty() && !ScaleFactor.Equals(FVector::OneVector);
}

FReply SWaypointBatchEdit::OnApplyScaleClicked(void)
{
    if (WaypointData.IsValid())
        WaypointData->ScaleAllPositions(ScaleFactor);

    return FReply::Handled();
}

FReply SWaypointBatchEdit::OnResetScaleClicked(void)
{
    ScaleFactor = FVector::OneVector;
    return FReply::Handled();
}
