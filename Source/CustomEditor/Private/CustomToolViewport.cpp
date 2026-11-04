#include "CustomToolViewport.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"


#include "CustomToolViewportClient.h"
#include "PreviewScene.h"
#include "EditorViewportClient.h"

#include "WaypointEditorData.h"


void SCustomToolViewport::Construct(const FArguments& InArgs)
{
    // 독립된 미리보기 씬 생성
    PreviewScene = MakeUnique<FPreviewScene>(FPreviewScene::ConstructionValues());
    WaypointData = InArgs._WaypointData;
    
    /*
    UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")); // 기본 평면 메시 로드

    if (FloorMesh)
    {
        UStaticMeshComponent* FloorComponent = NewObject<UStaticMeshComponent>(GetTransientPackage());

        FloorComponent->SetStaticMesh(FloorMesh);
        FloorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        FloorComponent->SetCastShadow(false);

        const FTransform FloorTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, -2.0f), FVector(100.0f, 100.0f, 1.0f)); // 평면 크기를 넓게 설정
        PreviewScene->AddComponent(FloorComponent, FloorTransform);
    }
    */

    SEditorViewport::Construct(SEditorViewport::FArguments()); // 뷰포트와 클라이언트 생성
}

TSharedRef<FEditorViewportClient> SCustomToolViewport::MakeEditorViewportClient()
{
    ViewportClient = MakeShared<FCustomToolViewportClient>(PreviewScene.Get(), SharedThis(this), WaypointData);

    return ViewportClient.ToSharedRef();
}

TSharedPtr<FCustomToolViewportClient> SCustomToolViewport::GetCustomViewportClient() const
{
    return StaticCastSharedPtr<FCustomToolViewportClient>(GetViewportClient());
}