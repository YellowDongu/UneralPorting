#include "CustomToolEditorWidget.h"

#include "CustomToolViewport.h"
#include "CustomToolViewportClient.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"

void SCustomToolEditorWidget::Construct(const FArguments& InArgs)
{
    WaypointData = InArgs._WaypointData;

    ChildSlot[
        SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().Padding(5.0f) // 왼쪽: 좌표 편집 패널
            [
                SNew(SBorder).Padding(10.0f)
                    [
                        SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
                            [
                                SNew(STextBlock).Text(FText::FromString(TEXT("Waypoint")))
                            ]

                            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                            [
                                SNew(STextBlock).Text(FText::FromString(TEXT("X")))
                            ]

                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SNew(SSpinBox<float>).MinValue(-1000000.0f).MaxValue(1000000.0f).Delta(1.0f)
                                    .Value_Lambda([this]()
                                                  {
                                                      return GetCoordinate(0);
                                                  })
                                    .OnValueChanged_Lambda([this](float Value)
                                                           {
                                                               SetCoordinate(Value, 0);
                                                           })
                                    .IsEnabled_Lambda([this]()
                                                      {
                                                          const auto Client = GetCustomViewportClient();
                                                          return Client.IsValid() && Client->HasSelectedWaypoint();
                                                      })
                            ]

                        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                            [
                                SNew(STextBlock).Text(FText::FromString(TEXT("Y")))
                            ]

                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SNew(SSpinBox<float>).MinValue(-1000000.0f).MaxValue(1000000.0f).Delta(1.0f)
                                    .Value_Lambda([this]()
                                                  {
                                                      return GetCoordinate(1);
                                                  })
                                    .OnValueChanged_Lambda([this](float Value)
                                                           {
                                                               SetCoordinate(Value, 1);
                                                           })
                                    .IsEnabled_Lambda([this]()
                                                      {
                                                          const auto Client = GetCustomViewportClient();
                                                          return Client.IsValid() && Client->HasSelectedWaypoint();
                                                      })
                            ]

                        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f)
                            [
                                SNew(STextBlock).Text(FText::FromString(TEXT("Z")))
                            ]

                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SNew(SSpinBox<float>).MinValue(-1000000.0f).MaxValue(1000000.0f).Delta(1.0f)
                                    .Value_Lambda([this]()
                                                  {
                                                      return GetCoordinate(2);
                                                  })
                                    .OnValueChanged_Lambda([this](float Value)
                                                           {
                                                               SetCoordinate(Value, 2);
                                                           })
                                    .IsEnabled_Lambda([this]()
                                                      {
                                                          const auto Client = GetCustomViewportClient();
                                                          return Client.IsValid() && Client->HasSelectedWaypoint();
                                                      })
                            ]
                    ]
            ]

        + SHorizontalBox::Slot().FillWidth(1.0f)[SAssignNew(ViewportWidget, SCustomToolViewport).WaypointData(WaypointData)] // 오른쪽: 기존 3D 뷰포트
    ];
}


TSharedPtr<FCustomToolViewportClient> SCustomToolEditorWidget::GetCustomViewportClient() const
{
    if (!ViewportWidget.IsValid())
        return nullptr;

    return ViewportWidget->GetCustomViewportClient();
}

float SCustomToolEditorWidget::GetCoordinate(int32 Axis) const
{
    const auto Client = GetCustomViewportClient();

    if (!Client.IsValid() || !Client->HasSelectedWaypoint())
        return 0.0f;

    //const FVector Position = Client->GetSelectedWaypointPosition();
    const FVector Position = WaypointData->GetSelectedPosition();

    switch (Axis)
    {
    case 0:
        return Position.X;
    case 1:
        return Position.Y;
    case 2:
        return Position.Z;
    default:
        return 0.0f;
    }
}

void SCustomToolEditorWidget::SetCoordinate(float Value, int32 Axis)
{
    const auto Client = GetCustomViewportClient();

    if (!Client.IsValid() || !Client->HasSelectedWaypoint())
        return;

    //FVector Position = Client->GetSelectedWaypointPosition();
    FVector Position = WaypointData->GetSelectedPosition();

    switch (Axis)
    {
    case 0:
        Position.X = Value;
        break;
    case 1:
        Position.Y = Value;
        break;
    case 2:
        Position.Z = Value;
        break;
    default:
        return;
    }

    //Client->SetSelectedWaypointPosition(Position);
    WaypointData->SetSelectedPosition(Position);
    if (ViewportWidget.IsValid())
        ViewportWidget->Invalidate();
}