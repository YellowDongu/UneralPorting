// Fill out your copyright notice in the Description page of Project Settings.


#include "Defined.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "TeamManagementSystem.h"
#include "VehicleBase.h"

FStringListPanelController::FStringListPanelController(UCanvasPanel* selectedBar, UCanvasPanel* listCanvas, UVerticalBox* listPanel) : SelectedBar(selectedBar), ListCanvas(listCanvas), ListPanel(listPanel)
{
	Initialize();
}

void FStringListPanelController::Initialize(class UCanvasPanel* selectedBar, class UCanvasPanel* listCanvas, class UVerticalBox* listPanel)
{
	SelectedBar = selectedBar;
	ListCanvas = listCanvas;
	ListPanel = listPanel;

	Initialize();
}

void FStringListPanelController::Initialize()
{
	check(SelectedBar);
	check(ListCanvas);
	check(ListPanel);

	UCanvasPanelSlot* PanelSlot = CastChecked<UCanvasPanelSlot>(SelectedBar->Slot);

	FVector2D position = PanelSlot->GetPosition();
	FVector2D size = PanelSlot->GetSize();
	BaseYValue = position.Y;
	CellYValue = size.Y;

	auto list = ListPanel->GetAllChildren();
	TextBlockList.Reserve(list.Num());

	for (auto& item : list)
	{
		UTextBlock* textBlock = Cast<UTextBlock>(item);
		if (textBlock != nullptr)
			TextBlockList.Add(textBlock);
	}

	MaxIndex = TextBlockList.Num();
	UE_LOG(LogTemp, Error, TEXT("[FStringListPanelController::Initialize] TextBlockList Number = %d"), TextBlockList.Num());
}

void FStringListPanelController::ChangeIndex(int index)
{
	if (TextBlockList.IsEmpty())
	{
		Index = -1;
		return;
	}

	index = index >= MaxIndex ? MaxIndex - 1 : (index < 0 ? 0 : index);
	if (Index == index)
		return;
	Index = index;

	UCanvasPanelSlot* canvasSlot = Cast<UCanvasPanelSlot>(SelectedBar->Slot);
	check(canvasSlot);
	FVector2D newPosition = FVector2D(canvasSlot->GetPosition().X, BaseYValue + CellYValue * static_cast<float>(Index));
	canvasSlot->SetPosition(newPosition);
}

void FStringListPanelController::ChangeChildsText(TFunction<void(TArray<TObjectPtr<class UTextBlock>>&, int&)> childTextChangeFunction) { childTextChangeFunction(TextBlockList, MaxIndex); }

void TeamData::Reset()
{
	SquadNumber = 0;
	MyVehicle.Reset();
	Offset = FVector::ZeroVector;
	Leader = nullptr;
}

#include "HAL/IConsoleManager.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

FString ExtractFileName(const FString& FilePath) // 메모용
{
	return FPaths::GetBaseFilename(FilePath);
	int32 LastSlashIndex;
	if (FilePath.FindLastChar(TEXT('/'), LastSlashIndex) || FilePath.FindLastChar(TEXT('\\'), LastSlashIndex))
	{
		return FilePath.Mid(LastSlashIndex + 1);
	}
	return FilePath; // 슬래시가 없는 경우 전체 경로를 반환
}
FString ExtractFileExtension(const FString& FilePath) { return FPaths::GetExtension(FilePath); } // 메모용
FString ExtractFileFullName(const FString& FilePath) { return FPaths::GetCleanFilename(FilePath); } // 메모용

#include "ObjectFactory.h"
void CheatDeleteEnemy(UWorld* world)
{
	UObjectFactory* objectFactory = world->GetSubsystem<UObjectFactory>();
	if (objectFactory == nullptr)
		return;

	TSet<TWeakObjectPtr<class AVehicleBase>> listSet = objectFactory->GetAllVehicle()[2];
	for (auto& item : listSet)
	{
		static constexpr unsigned int intmax = static_cast<unsigned int>(-1) / 2;
		item->HPDamage(TNumericLimits<int>::Max());
		if (item->IsAlive())
			item->HPDamage(TNumericLimits<int>::Max());
	}

	listSet = objectFactory->GetAllVehicle()[0];
	for (auto& item : listSet)
	{
		static constexpr unsigned int intmax = static_cast<unsigned int>(-1) / 2;
		item->HPDamage(TNumericLimits<int>::Max());
		if (item->IsAlive())
			item->HPDamage(TNumericLimits<int>::Max());
	}
}

const static FString FileExtension = TEXT("meta");
const static FString FileFilter = TEXT("*.") + FileExtension;
const static FString DeleteFilesCommandName = TEXT("MyTools.DeleteTmpFiles");
const static FString DeleteFilesCommandExplain = (TEXT("Delete all %s files under the Content directory."), *FileExtension);
const static FString ContentPath = FPaths::ProjectContentDir(); // 프로젝트의 Content 폴더 경로


void DeleteFiles()
{
	TArray<FString> FoundFiles;

	// 하위 폴더까지 재귀적으로 검색
	IFileManager::Get().FindFilesRecursive(FoundFiles, *ContentPath, *FileFilter, true, /*파일 검색*/ false, /*폴더 검색 안 함*/ true /*기존 배열 초기화*/);

	int32 DeletedCount = 0;

	for (const FString& FilePath : FoundFiles)
	{
		if (IFileManager::Get().Delete(*FilePath, false, true, false))
		{
			UE_LOG(LogTemp, Display, TEXT("Delete %s // %s"), *ExtractFileFullName(FilePath), *FilePath);

			++DeletedCount;
		}
	}

	UE_LOG(LogTemp, Display, TEXT("Delete[%s]Files : %d / %d files deleted."), *FileExtension, DeletedCount, FoundFiles.Num());
}

static FAutoConsoleCommand DeleteFilesCommand(*DeleteFilesCommandName, *DeleteFilesCommandExplain, FConsoleCommandDelegate::CreateLambda([]() { Async(EAsyncExecution::ThreadPool, []() { DeleteFiles(); }); })); // 내부에서 에디터 전용 API나 UObject를 조작한다면 해당 부분은 게임 스레드에서 처리해야됨
//static FAutoConsoleCommand DeleteFilesCommand(*DeleteFilesCommandName, *DeleteFilesCommandExplain, FConsoleCommandDelegate::CreateLambda([]() { DeleteFiles(); }));

void FTargetInfo::CalculateDistance(const TWeakObjectPtr<AVehicleBase>& onwer, bool OnlySqared)
{
	if (!IsAlive(Target) || !IsAlive(onwer))
		return;

	CalculateDistance(onwer->GetActorLocation(), true, OnlySqared);
}

void FTargetInfo::CalculateDistance(const FVector& position, bool isWorldPosition, bool OnlySqared)
{
	DistanceSquared = isWorldPosition ? (Target->GetActorLocation() - position).SizeSquared() : position.SizeSquared();

	if (OnlySqared)
		return;

	Distance = FMath::Sqrt(Distance);
}
