#include "GaugeUI.h"
#include "VehicleBase.h"
#include "Aircraft.h"
#include "PlayerPilot.h"
#include "Missile.h"
#include "FireControlSystem.h"
#include "Components/CanvasPanel.h"
#include "Components/ProgressBar.h"


const float UGaugeUI::FGauge::FrameTime = 1.0f / 60.0f;

void UGaugeUI::FGauge::StartTimer(UWorld* world) // deprecated
{
	if (TimerHandle.IsValid() || world == nullptr)
		return;

	Timer = TimeMax;
	World = world;

	world->GetTimerManager().SetTimer(TimerHandle, [this]()->void { ActiveTimer(); }, FrameTime, true);
	//world->GetTimerManager().SetTimer(TimerHandle, Delegate, FrameTime, true);
}

bool UGaugeUI::FGauge::Tick(float deltaTime)
{
	Timer -= deltaTime;
	if (Timer < 0.0f)
	{
		Timer = 0.0f;
		return true;
	}

	float progress = 1.0f - Timer / TimeMax;
	ProgressBar->SetPercent(progress);
	return false;
}

void UGaugeUI::FGauge::ActiveTimer()
{
	float deltaTime = FMath::Max(FrameTime, World->GetDeltaSeconds());

	Timer -= deltaTime;
	if (Timer < 0.0f)
	{
		Timer = 0.0f;
		World->GetTimerManager().ClearTimer(TimerHandle);
	}

	float progress = 1.0f - Timer / TimeMax;
	ProgressBar->SetPercent(progress);
}

void UGaugeUI::FGauge::EditGauge(float value) { Timer = FMath::Clamp(value, 0.0f, TimeMax); ProgressBar->SetPercent(1.0f - Timer / TimeMax); } // 값 강제 조정
void UGaugeUI::FGauge::EditGaugePercentage(float value) { Timer = FMath::Clamp(value, 0.0f, 1.0f) * TimeMax; ProgressBar->SetPercent(1.0f - Timer / TimeMax); }

void UGaugeUI::FGauge::SetGaugeImage(const FGaugeImageSet& imageSet)
{
	FSlateBrush Brush, BackBrush;
	FProgressBarStyle Style = ProgressBar->GetWidgetStyle();

	Brush.SetResourceObject(imageSet.InnerImage);
	BackBrush.SetResourceObject(imageSet.Image);

	Style.SetFillImage(Brush);
	Style.SetBackgroundImage(BackBrush);

	ProgressBar->SetWidgetStyle(Style);
}

void UGaugeUI::FGaugeSet::CreateGaugeSet(class UCanvasPanel* gaugeCanvas, TSubclassOf<class AMissile> targetBluePrint)
{
	if (gaugeCanvas == nullptr || targetBluePrint == nullptr)
		return;

	BaseCanvas = gaugeCanvas;
	AMissile* missile = targetBluePrint->GetDefaultObject<AMissile>();
	float cooltime = missile->GetMaxCoolTime();

	for (auto& item : gaugeCanvas->GetAllChildren())
	{
		UProgressBar* bar = Cast<UProgressBar>(item);
		if (bar == nullptr)
			continue;

		GaugeBars.Emplace(cooltime, bar);
		GaugeBars.Last().EditGauge(0.0f);

	}

	ActivedGauge.Reserve(GaugeBars.Num());
}

void UGaugeUI::FGaugeSet::CreateGaugeSet(UCanvasPanel* gaugeCanvas, TSubclassOf<class AMissile> targetBluePrint, const FGaugeImageSet& imageSet)
{
	if (gaugeCanvas == nullptr || targetBluePrint == nullptr)
		return;

	CreateGaugeSet(gaugeCanvas, targetBluePrint);
	for (auto& item : GaugeBars)
		item.SetGaugeImage(imageSet);
}



void UGaugeUI::NativeTick(const FGeometry& myGeometry, float deltaTime)
{
	Super::NativeTick(myGeometry, deltaTime);

	StandardMissileGauge.Tick(deltaTime);
	SpecialMissileGauge.Tick(deltaTime);
}

void UGaugeUI::BindPlayer(AAircraft* vehicle, APlayerPilot* player)
{
	TSubclassOf<AMissile>* standardMissile{ nullptr }, * specialMissile{ nullptr };
	TWeakObjectPtr<UFireControlSystem> fcs = vehicle->GetFireControlSystem();
	EVehicleID vehicleID = vehicle->GetVehicleID();
	EWeaponID weaponID = fcs->GetWeaponID(true);

	player->FiredMissile.BindUObject(this, &UGaugeUI::MissileFired);
	player->ChangeWeaponUI.BindUObject(this, &UGaugeUI::ChangeGauge);

	for (auto& item : MissileBluprints)
	{
		EWeaponID id = Cast<AMissile>(item->GetDefaultObject())->GetWeaponID();

		if (id == weaponID)
			specialMissile = &item;
		else if (id == EWeaponID::Standard)
			standardMissile = &item;
	}

	HPGauge.Timer = HPGauge.TimeMax = vehicle->GetMaxHp();
	HPGauge.ProgressBar = HPGaugeBar;


	FGaugeImageSet* gaugeSet = HPGaugeImageSet.Find(vehicleID);
	if (gaugeSet == nullptr) { UE_LOG(LogTemp, Error, TEXT("[UGaugeUI::BindPlayer] #1 gaugeSet == nullptr")); RemoveFromParent(); return; }
	HPGauge.SetGaugeImage(*gaugeSet);

	if (standardMissile == nullptr) { UE_LOG(LogTemp, Error, TEXT("[UGaugeUI::BindPlayer] standardMissile == nullptr")); RemoveFromParent(); return; }

	StandardMissileGauge.CreateGaugeSet(Cast<UCanvasPanel>(StandardGaugeCanvas), *standardMissile);
	StandardMissileGauge.BaseCanvas->SetVisibility(bGaugeSelection ? ESlateVisibility::Hidden : ESlateVisibility::Visible);


	if (specialMissile == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UGaugeUI::BindPlayer] No SpecialMissile"));

		MissileGaugeCanvas->SetVisibility(ESlateVisibility::Hidden);
		return;
	}



	gaugeSet = WeaponGaugeImageSet.Find(weaponID);
	if (gaugeSet == nullptr) { UE_LOG(LogTemp, Error, TEXT("[UGaugeUI::BindPlayer] #2 gaugeSet == nullptr")); RemoveFromParent(); return; }

	UCanvasPanel* panel = nullptr;

	switch (fcs->GetWeaponCount())
	{
	case 2:
		panel = Gauge2Canvas;
		Gauge4Canvas->RemoveFromParent();
		break;
	case 4:
		panel = Gauge4Canvas;
		Gauge2Canvas->RemoveFromParent();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[UGaugeUI::BindPlayer] fcs->GetWeaponCount() Error"));
		RemoveFromParent();
		return;
	}

	SpecialMissileGauge.CreateGaugeSet(panel, *specialMissile, *gaugeSet);
	SpecialMissileGauge.BaseCanvas->SetVisibility(bGaugeSelection ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	bGaugeSelection = false;
}

void UGaugeUI::MissileFired(const TArray<int>& list, bool isSpecial)
{
	UWorld* world = GetWorld();
	if (isSpecial)
		for (auto& item : list)
			SpecialMissileGauge.StartTimer(world, item);
	else
		StandardMissileGauge.StartTimer(world, list[0]);
}

void UGaugeUI::ChangeGauge(bool isSpecial)
{
	if (bGaugeSelection == isSpecial)
		return;

	bGaugeSelection = isSpecial;
	StandardMissileGauge.BaseCanvas->SetVisibility(bGaugeSelection ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	SpecialMissileGauge.BaseCanvas->SetVisibility(bGaugeSelection ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

