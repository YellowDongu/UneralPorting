// Fill out your copyright notice in the Description page of Project Settings.


#include "LockInfomation.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
//#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/WidgetComponent.h"

#include "Aircraft.h"
//#include "PlayerPilot.h"
#include "FireControlSystem.h"
#include "UnGuidedBomb.h"
#include "GuidedBomb.h"

TSharedPtr<LockSet> LockSet::Create(IndicatorCreationData& data)
{
	TSharedPtr<LockSet> indicator;
//	LockSet* result = nullptr;
	switch (data.Weapon->GetWeaponID())
	{
	case EWeaponID::Standard:
		indicator = MakeShared<LockIndicator>();
		break;
	case EWeaponID::FAAM:
		indicator = MakeShared<LockIndicator>();
		break;
	case EWeaponID::SAAM:
		indicator = MakeShared<SAAMLockIndicator>();
		break;
	case EWeaponID::UGB:
		indicator = MakeShared<BombIndicator>();
		break;
	case EWeaponID::LGB:
		indicator = MakeShared<LockableBombIndicator>();
		break;
	default:
		return nullptr;
	}

	if (indicator == nullptr)
	return nullptr;
	indicator->Initialize(data);
	indicator->bIsActive = data.Weapon->GetWeaponID() != EWeaponID::Standard;
	indicator->SetActive(false);
	return indicator;
}



void LockIndicator::Initialize(IndicatorCreationData& data)
{
	UCanvasPanel* canvasPanel = data.CanvasPanel;
	TSubclassOf<UUserWidget>& LockBluePrint = data.LockBluePrint;
	check(canvasPanel);

	LockRectangle = CreateWidget<UUserWidget>(canvasPanel->GetWorld(), LockBluePrint);
	CanvasSlot = canvasPanel->AddChildToCanvas(LockRectangle);
	PlayerController = canvasPanel->GetOwningPlayer();

	check(PlayerController.Get());
	check(LockRectangle);
	check(CanvasSlot.Get());

	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
	//CanvasSlot->SetAnchors(AnchorPresets);
	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	//CanvasSlot->SetZOrder(10);
}

void LockIndicator::Initialize(UCanvasPanel* canvasPanel, TSubclassOf<UUserWidget>& LockBluePrint)
{
	check(canvasPanel);

	LockRectangle = CreateWidget<UUserWidget>(canvasPanel->GetWorld(), LockBluePrint);
	CanvasSlot = canvasPanel->AddChildToCanvas(LockRectangle);
	PlayerController = canvasPanel->GetOwningPlayer();

	check(PlayerController.Get());
	check(LockRectangle);
	check(CanvasSlot.Get());

	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
	//CanvasSlot->SetAnchors(AnchorPresets);
	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	//CanvasSlot->SetZOrder(10);
}

void LockIndicator::Update(float deltaTime, FTargetInfo* target)
{
	if (target == nullptr)
	{
		SetActive(false);
		return;
	}

	SetActive(true);
	TraceTarget(*target);
}

void LockIndicator::SetActive(bool value)
{
	if (bIsActive == value)
		return;

	bIsActive = value;
	LockRectangle->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void LockIndicator::TraceTarget(FTargetInfo& target)
{
	if (!target.Target.IsValid() || target.Lock >= 0.99f)
	{
		SetActive(false);
		return;
	}

	if (Target != target.Target)
		StartTraceTarget(target);

	FVector2D screenPos;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController.Get(), Target->GetActorLocation(), screenPos, true))
	{
		SetActive(true);
		CanvasSlot->SetPosition(FMath::Lerp(screenPos, InitialPosition, target.Lock));
		SetColor(target.Lock >= 0.01f ? FColor::Green : FColor::Red);
	}
	else
		SetActive(false);
}

void LockIndicator::StartTraceTarget(FTargetInfo& target)
{
	if (!target.Target.IsValid())
	{
		SetActive(false);
		return;
	}

	FVector2D ScreenCenter = UWidgetLayoutLibrary::GetViewportSize(PlayerController->GetWorld()) * 0.5f;
	float Range = FMath::Min(ScreenCenter.X, ScreenCenter.Y) * 0.5f;
	ScreenCenter.X += FMath::RandRange(-Range, Range);
	ScreenCenter.Y += FMath::RandRange(-Range, Range);

	InitialPosition = ScreenCenter;
	Target = target.Target;
	CanvasSlot->SetPosition(InitialPosition);
}

void LockIndicator::SetColor(const FLinearColor& color)
{
	if (CurrentColor == color) return;

	CurrentColor = color;
	LockRectangle->SetColorAndOpacity(color);
}


void BombIndicator::Initialize(IndicatorCreationData& data)
{
	TSubclassOf<AActor>& RangeIndicatorBluePrint = data.RangeIndicatorBluePrint;
	AUnGuidedBomb* weapon = Cast<AUnGuidedBomb>(data.Weapon);
	double area = weapon->GetBlastArea();

	FCS = data.FCS;
	RangeIndicator = FCS->GetWorld()->SpawnActor<AActor>(RangeIndicatorBluePrint);
	check(RangeIndicator);
	RangeIndicatorUI = static_cast<UWidgetComponent*>(RangeIndicator->GetComponentByClass(UWidgetComponent::StaticClass()));
	check(RangeIndicatorUI);
	area *= 2.0;
	RangeIndicatorUI->SetRelativeScale3D(FVector(1.0, area, area));
	CalculateBombLandingPosition.BindUObject(FCS.Get(), &UFireControlSystem::CalculateBombLandingPosition);

	auto slot = FCS->GetActivedSpecialSlot();

	for (auto& item : slot->MissileSlot)
	{
		auto ugb = Cast<AUnGuidedBomb>(item);
		if (ugb != nullptr)
		{
			ugb->SetBombIndicatorHUD(RangeIndicator);
			continue;
		}
	}
	SetColor(GreenHUD);
	SetActive(CalculateBombLandingPosition.IsBound());
}

void BombIndicator::Initialize(TWeakObjectPtr<UFireControlSystem> fcs, TSubclassOf<AActor>& RangeIndicatorBluePrint, double area)
{
	FCS = fcs;

	RangeIndicator = fcs->GetWorld()->SpawnActor<AActor>(RangeIndicatorBluePrint);
	check(RangeIndicator);
	RangeIndicatorUI = static_cast<UWidgetComponent*>(RangeIndicator->GetComponentByClass(UWidgetComponent::StaticClass()));
	check(RangeIndicatorUI);
	area *= 2.0;
	RangeIndicatorUI->SetRelativeScale3D(FVector(1.0, area, area));
	CalculateBombLandingPosition.BindUObject(fcs.Get(), &UFireControlSystem::CalculateBombLandingPosition);

	auto slot = FCS->GetActivedSpecialSlot();

	for (auto& item : slot->MissileSlot)
	{
		auto ugb = Cast<AUnGuidedBomb>(item);
		if (ugb != nullptr)
		{
			ugb->SetBombIndicatorHUD(RangeIndicator);
			continue;
		}
	}
	SetColor(GreenHUD);
	SetActive(CalculateBombLandingPosition.IsBound());
}

void BombIndicator::SetColor(const FLinearColor& color)
{
	if (CurrentColor == color) return;

	CurrentColor = color;
	if (RangeIndicatorUI->GetMaterialInstance() == nullptr)
		return;
	RangeIndicatorUI->GetMaterialInstance()->SetVectorParameterValue(TEXT("Color"), color);
	//RangeIndicatorUI->GetUserWidgetObject()->SetColorAndOpacity(color);
}

void BombIndicator::Update(float deltaTime, FTargetInfo* target)
{
	DisplayExpactPosition();
	SetColor(GreenHUD);
}

void BombIndicator::SetActive(bool value)
{
	if (bIsActive == value)
		return;

	bIsActive = value;
	RangeIndicator->SetActorHiddenInGame(!bIsActive);
	//RangeIndicator->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void BombIndicator::DisplayExpactPosition()
{
	FVector position;
	if (!CalculateBombLandingPosition.IsBound())
		return;


	if (CalculateBombLandingPosition.Execute(position))
	{
		SetActive(true);
		RangeIndicator->SetActorLocation(position);
	}
	else
		SetActive(false);
}


void LockableBombIndicator::Initialize(IndicatorCreationData& data)
{
	UCanvasPanel* canvasPanel = data.CanvasPanel;
	TSubclassOf<class UUserWidget>& LockBluePrint = data.LockBluePrint;
	TSubclassOf<AActor>& RangeIndicatorBluePrint = data.RangeIndicatorBluePrint;
	AGuidedBomb* weapon = Cast<AGuidedBomb>(data.Weapon);
	double area = weapon->GetBlastArea();

	check(canvasPanel);

	UWorld* world = canvasPanel->GetWorld();
	PlayerController = canvasPanel->GetOwningPlayer();
	RangeIndicator = world->SpawnActor<AActor>(RangeIndicatorBluePrint);
	LockRectangle = CreateWidget<UUserWidget>(canvasPanel->GetWorld(), LockBluePrint);
	check(RangeIndicator);
	check(LockRectangle);

	CanvasSlot = canvasPanel->AddChildToCanvas(LockRectangle);
	RangeIndicatorUI = static_cast<UWidgetComponent*>(RangeIndicator->GetComponentByClass(UWidgetComponent::StaticClass()));
	check(RangeIndicatorUI);
	check(CanvasSlot.Get());

	area *= 2.0;
	RangeIndicatorUI->SetRelativeScale3D(FVector(1.0, area, area));

	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
	//CanvasSlot->SetAnchors(AnchorPresets);
	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	//CanvasSlot->SetZOrder(10);

	FCS = data.FCS;
	auto slot = FCS->GetActivedSpecialSlot();

	for (auto& item : slot->MissileSlot)
	{
		auto ugb = Cast<AGuidedBomb>(item);
		if (ugb != nullptr)
		{
			ugb->SetBombIndicatorHUD(RangeIndicator);
			continue;
		}
	}

	SetColor(GreenHUD);

}


void LockableBombIndicator::Initialize(TWeakObjectPtr<class UFireControlSystem> fcs, class UCanvasPanel* canvasPanel, TSubclassOf<class UUserWidget>& LockBluePrint, TSubclassOf<AActor>& RangeIndicatorBluePrint, double area)
{
	check(canvasPanel);

	UWorld* world = canvasPanel->GetWorld();
	PlayerController = canvasPanel->GetOwningPlayer();
	RangeIndicator = world->SpawnActor<AActor>(RangeIndicatorBluePrint);
	LockRectangle = CreateWidget<UUserWidget>(canvasPanel->GetWorld(), LockBluePrint);
	check(RangeIndicator);
	check(LockRectangle);

	CanvasSlot = canvasPanel->AddChildToCanvas(LockRectangle);
	RangeIndicatorUI = static_cast<UWidgetComponent*>(RangeIndicator->GetComponentByClass(UWidgetComponent::StaticClass()));
	check(RangeIndicatorUI);
	check(CanvasSlot.Get());

	area *= 2.0;
	RangeIndicatorUI->SetRelativeScale3D(FVector(1.0, area, area));

	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
	//CanvasSlot->SetAnchors(AnchorPresets);
	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	//CanvasSlot->SetZOrder(10);

	FCS = fcs;
	auto slot = FCS->GetActivedSpecialSlot();

	for (auto& item : slot->MissileSlot)
	{
		auto ugb = Cast<AGuidedBomb>(item);
		if (ugb != nullptr)
		{
			ugb->SetBombIndicatorHUD(RangeIndicator);
			continue;
		}
	}

	SetColor(GreenHUD);
}

void LockableBombIndicator::Update(float deltaTime, FTargetInfo* target)
{
	if (target == nullptr)
	{
		SetActive(false);
		return;
	}

	SetActive(true);
	TraceTarget(*target);
}

void LockableBombIndicator::SetColor(const FLinearColor& color)
{
	if (CurrentColor == color)
		return;

	CurrentColor = color;
	LockRectangle->SetColorAndOpacity(color);

	if (RangeIndicatorUI->GetMaterialInstance() == nullptr)
		return;
	RangeIndicatorUI->GetMaterialInstance()->SetVectorParameterValue(TEXT("Color"), color);
	//RangeIndicatorUI->GetUserWidgetObject()->SetColorAndOpacity(color);
}

void LockableBombIndicator::SetActive(bool value)
{
	if (bIsActive == value)
		return;

	bIsActive = value;
	RangeIndicator->SetActorHiddenInGame(!bIsActive);
	//RangeIndicator->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	LockRectangle->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void LockableBombIndicator::TraceTarget(FTargetInfo& target)
{
	if (!target.Target.IsValid() || target.Lock >= 0.99f)
	{
		SetActive(false);
		return;
	}

	SetActive(true);

	if (Target != target.Target)
		StartTraceTarget(target);

	RangeIndicator->SetActorLocation(target.Target->GetActorLocation());

	FVector2D screenPos;
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerController.Get(), Target->GetActorLocation(), screenPos, true))
		CanvasSlot->SetPosition(FMath::Lerp(screenPos, InitialPosition, target.Lock));

	bool lock = target.IsLock();
	if (bIsLock == lock)
		return;
	bIsLock = lock;
	SetColor(bIsLock ? GreenHUD : RedHUD);
}

void LockableBombIndicator::StartTraceTarget(FTargetInfo& target)
{
	if (!target.Target.IsValid())
	{
		SetActive(false);
		return;
	}

	FVector2D ScreenCenter = UWidgetLayoutLibrary::GetViewportSize(PlayerController->GetWorld()) * 0.5f;
	float Range = FMath::Min(ScreenCenter.X, ScreenCenter.Y) * 0.5f;
	ScreenCenter.X += FMath::RandRange(-Range, Range);
	ScreenCenter.Y += FMath::RandRange(-Range, Range);

	InitialPosition = ScreenCenter;
	Target = target.Target;
	CanvasSlot->SetPosition(InitialPosition);
}




void SAAMLockIndicator::Initialize(IndicatorCreationData& data)
{
	LockIndicator::Initialize(data.CanvasPanel, data.SpecialLockBluePrint);

	FCS = data.FCS;
	Owner = FCS->GetOwner();

	RangeIndicator = FCS->GetWorld()->SpawnActor<AActor>(data.RangeIndicatorBluePrint);
	check(RangeIndicator);
	RangeIndicatorUI = static_cast<UWidgetComponent*>(RangeIndicator->GetComponentByClass(UWidgetComponent::StaticClass()));
	check(RangeIndicatorUI);

	double distance = static_cast<double>(ConvertFromDistance(data.Weapon->GetMaxRange()));
	double radius = distance * FMath::Tan(FMath::DegreesToRadians(static_cast<double>(data.Weapon->GetLockAngle())));
	
	RangeIndicatorUI->SetRelativeLocationAndRotation(FVector(distance, 0.0, 0.0), FQuat::Identity);
	RangeIndicatorUI->SetRelativeScale3D(FVector(1.0, radius, radius));
}

void SAAMLockIndicator::Update(float deltaTime, FTargetInfo* target)
{
	if (!target->Target.IsValid() || target->Lock >= 0.99f)
	{
		SetActive(false);
		return;
	}

	if (target == nullptr)
	{
		LockIndicator::SetActive(false);
		return;
	}

	LockIndicator::SetActive(true);
	LockIndicator::Update(deltaTime, target);

	RangeIndicator->SetActorLocationAndRotation(Owner->GetActorLocation(), Owner->GetActorRotation());
}

void SAAMLockIndicator::SetActive(bool value)
{
	if (bIsActive == value)
		return;

	LockIndicator::SetActive(value);
	//UE_LOG(LogTemp, Error, TEXT("[SAAMLockIndicator::SetActive] Set Special : %d"), bIsActive);
	RangeIndicator->SetActorHiddenInGame(!bIsActive);
	//UE_LOG(LogTemp, Error, TEXT("[SAAMLockIndicator::SetActive] Active : %d"), static_cast<int>(LockRectangle->GetVisibility()));
	//UE_LOG(LogTemp, Error, TEXT("[SAAMLockIndicator::SetActive] Active : %d"), static_cast<int>(RangeIndicator->IsHidden()));
	//RangeIndicator->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void SAAMLockIndicator::SetColor(const FLinearColor& color)
{
	if (CurrentColor == color) return;

	LockIndicator::SetColor(color);

	CurrentColor = color;
	RangeIndicatorUI->GetMaterialInstance()->SetVectorParameterValue(TEXT("Color"), color);
	//RangeIndicatorUI->GetUserWidgetObject()->SetColorAndOpacity(color);
}
