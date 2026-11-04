#include "WaypointDetailsWidget.h"

#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SWaypointDetails::Construct(const FArguments& InArgs)
{
    static constexpr float PositionLimitValue{ 10000.0f };
    static constexpr float AngleLimitValue{ 180.0f };

    WaypointData = InArgs._WaypointData;

    ChildSlot
        [
            SNew(SBorder).Padding(8.0f)
                [
                    SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
                        [
                            SNew(STextBlock).Text(FText::FromString(TEXT("Waypoint Details")))
                        ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
                        [
                            SNew(STextBlock).Text(FText::FromString(TEXT("Position")))
                        ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                        [
                            SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("X")))
                                ]

                                + SHorizontalBox::Slot().FillWidth(1.0f)
                                [
                                    SNew(SSpinBox<float>).MinValue(-PositionLimitValue).MaxValue(PositionLimitValue)
                                        .Value_Lambda([this]()
                                                      {
                                                          return GetCoordinate(0);
                                                      })
                                        .OnValueChanged_Lambda([this](float Value)
                                                               {
                                                                   SetCoordinate(Value, 0);
                                                               })
                                ]
                        ]

                    + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                        [
                            SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Y")))
                                ]

                                + SHorizontalBox::Slot().FillWidth(1.0f)
                                [
                                    SNew(SSpinBox<float>).MinValue(-PositionLimitValue).MaxValue(PositionLimitValue)
                                        .Value_Lambda([this]()
                                                      {
                                                          return GetCoordinate(1);
                                                      })
                                        .OnValueChanged_Lambda([this](float Value)
                                                               {
                                                                   SetCoordinate(Value, 1);
                                                               })
                                ]
                        ]

                    + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                        [
                            SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Z")))
                                ]

                                + SHorizontalBox::Slot().FillWidth(1.0f)
                                [
                                    SNew(SSpinBox<float>).MinValue(-PositionLimitValue).MaxValue(PositionLimitValue)
                                        .Value_Lambda([this]()
                                                      {
                                                          return GetCoordinate(2);
                                                      })
                                        .OnValueChanged_Lambda([this](float Value)
                                                               {
                                                                   SetCoordinate(Value, 2);
                                                               })
                                ]
                        ]

                    
                    +SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
                        [
                            SNew(STextBlock).Text(FText::FromString(TEXT("Rotation")))
                        ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                        [
                            SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Roll")))
                                ]

                                + SHorizontalBox::Slot().FillWidth(1.0f)
                                [
                                    SNew(SSpinBox<float>).MinValue(-AngleLimitValue).MaxValue(AngleLimitValue)
                                        .Value_Lambda([this]()
                                                      {
                                                          return GetRotationAngle(0);
                                                      })
                                        .OnValueChanged_Lambda([this](float Value)
                                                               {
                                                                   SetRotationAngle(Value, 0);
                                                               })
                                ]
                        ]

                    + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                        [
                            SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Pitch")))
                                ]

                                + SHorizontalBox::Slot().FillWidth(1.0f)
                                [
                                    SNew(SSpinBox<float>).MinValue(-AngleLimitValue).MaxValue(AngleLimitValue)
                                        .Value_Lambda([this]()
                                                      {
                                                          return GetRotationAngle(1);
                                                      })
                                        .OnValueChanged_Lambda([this](float Value)
                                                               {
                                                                   SetRotationAngle(Value, 1);
                                                               })
                                ]
                        ]

                    + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                        [
                            SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
                                [
                                    SNew(STextBlock).Text(FText::FromString(TEXT("Yaw")))
                                ]

                                + SHorizontalBox::Slot().FillWidth(1.0f)
                                [
                                    SNew(SSpinBox<float>).MinValue(-AngleLimitValue).MaxValue(AngleLimitValue)
                                        .Value_Lambda([this]()
                                                      {
                                                          return GetRotationAngle(2);
                                                      })
                                        .OnValueChanged_Lambda([this](float Value)
                                                               {
                                                                   SetRotationAngle(Value, 2);
                                                               })
                                ]
                        ]
                    
                ]
        ];
}

float SWaypointDetails::GetCoordinate(int32 Axis) const
{
    if (!WaypointData.IsValid() || !WaypointData->HasSelection())
        return 0.0f;

    const FVector& Position = WaypointData->GetSelectedPosition();

    switch (Axis)
    {
    case 0: return Position.X;
    case 1: return Position.Y;
    case 2: return Position.Z;
    default: return 0.0f;
    }
}

float SWaypointDetails::GetRotationAngle(int32 Axis) const
{
    if (!WaypointData.IsValid() || !WaypointData->HasSelection())
        return 0.0f;

    const FRotator& Position = WaypointData->GetSelectedRotation();

    switch (Axis)
    {
    case 0: return Position.Roll;
    case 1: return Position.Pitch;
    case 2: return Position.Yaw;
    default: return 0.0f;
    }
}

void SWaypointDetails::SetCoordinate(float Value, int32 Axis)
{
    if (!WaypointData.IsValid() || !WaypointData->HasSelection())
        return;

    FVector Position = WaypointData->GetSelectedPosition();

    switch (Axis)
    {
    case 0: Position.X = Value; break;
    case 1: Position.Y = Value; break;
    case 2: Position.Z = Value; break;
    default: return;
    }

    WaypointData->SetSelectedPosition(Position);
}

void SWaypointDetails::SetRotationAngle(float Value, int32 Axis)
{
    if (!WaypointData.IsValid() || !WaypointData->HasSelection())
        return;

    FRotator Rotation = WaypointData->GetSelectedRotation();

    switch (Axis)
    {
    case 0: Rotation.Roll = Value; break;
    case 1: Rotation.Pitch = Value; break;
    case 2: Rotation.Yaw = Value; break;
    default: return;
    }

    WaypointData->SetSelectedRotation(Rotation);
}
