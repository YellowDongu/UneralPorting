#include "RadarUI.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/AudioComponent.h"
#include "EngineUtils.h"

#include "Radar.h"
#include "Aircraft.h"
#include "PlayerPilot.h"
#include "RadarTargetUI.h"
#include "SoundSubsystem.h"

FVector URadarUI::LeftBottomBoarder{ FVector::ZeroVector };
FVector URadarUI::RightTopBoarder{ FVector::OneVector };

void URadarUI::BindPlayer(AAircraft* vehicle, APlayerPilot* player)
{
	Player = vehicle;
	PlayerRadar = vehicle->GetRadar();

	bIsExpended = true;
	RefreshTime = 0.025f;
	ChangeRadar();

	for (int i = 0; i < 50; i++)
		Stored.Add(GetTargetUI(CurrentCanvas));
}

void URadarUI::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	Refresh(DeltaTime);
}

void URadarUI::Refresh(float DeltaTime)
{
	Timer += DeltaTime;
	if (Timer < RefreshTime)
		return;

	Timer -= RefreshTime;
	int i = 0;
	TObjectPtr<URadarTargetUI> targetUI;

	if (bIsExpended)
	{
		for (TActorIterator<AVehicleBase> iterator(Player->GetWorld()); iterator; ++iterator, i++)
		{
			if (i < Alive.Num())
				targetUI = Alive[i];
			else
			{
				Alive.Add(GetTargetUI(CurrentCanvas));
				targetUI = Alive.Last();
			}

			AVehicleBase* target = *iterator;

			if (!IsAlive(target))
			{
				i--;
				continue;
			}

			FVector relatedPosition = target->GetActorLocation() - LeftBottomBoarder;
			relatedPosition.X = relatedPosition.X / (RightTopBoarder.X - LeftBottomBoarder.X) * LargeRadarSize.X - LargeRadarSize.X * 0.5;
			relatedPosition.Y = relatedPosition.Y / (RightTopBoarder.Y - LeftBottomBoarder.Y) * LargeRadarSize.Y - LargeRadarSize.Y * 0.5;

			//FVector relatedPosition = target->GetActorLocation() - LeftBottomBoarder;
			//FVector size = RightTopBoarder - LeftBottomBoarder;
			//relatedPosition.X /= size.X;
			//relatedPosition.Y /= size.Y;
			
			//relatedPosition.X *= LargeRadarSize.X;
			//relatedPosition.Y *= LargeRadarSize.Y;
			//relatedPosition.X -= LargeRadarSize.X * 0.5;
			//relatedPosition.Y -= LargeRadarSize.Y * 0.5;

			targetUI->Update(FVector2D(relatedPosition.Y, -relatedPosition.X), 0.0f, target);
		}

		if (i < Alive.Num())
		{
			for (int j = i; j < Alive.Num(); j++)
				RetriveTargetUI(Alive[j]);

			Alive.SetNum(i, false);
		}
	}
	else
	{
		const TArray<TWeakObjectPtr<AVehicleBase>>& Targets = PlayerRadar->GetAllInRange();
		FVector center = Player->GetActorLocation();
		FRotator rotator = FRotator(0.0f, -Player->GetActorRotation().Yaw, 0.0f);

		for (const TWeakObjectPtr<AVehicleBase>& target : PlayerRadar->GetAllInRange())
		{
			if (i < Alive.Num())
				targetUI = Alive[i];
			else
			{
				Alive.Add(GetTargetUI(CurrentCanvas));
				targetUI = Alive.Last();
			}

			FVector relatedPosition = rotator.RotateVector((target->GetActorLocation() - center) / CloseRangeSize * 150.0f);
			targetUI->Update(FVector2D(relatedPosition.Y, -relatedPosition.X) + FVector2D(0.0f, 50.0f), Player->GetActorRotation().Yaw, target.Get());
			i++;
		}
		
		if (i < Alive.Num())
		{
			for (int j = i; j < Alive.Num(); j++)
				RetriveTargetUI(Alive[j]);

			Alive.SetNum(i, false);
		}
	}
}

void URadarUI::ChangeRadar()
{
	bIsExpended = !bIsExpended;
	Timer = RefreshTime;

	if (bIsExpended)
	{
		LongRangePanel->SetVisibility(ESlateVisibility::Visible);
		CloseRangePanel->SetVisibility(ESlateVisibility::Hidden);
		CurrentCanvas = BaseCanvasL;
	}
	else
	{
		LongRangePanel->SetVisibility(ESlateVisibility::Hidden);
		CloseRangePanel->SetVisibility(ESlateVisibility::Visible);
		CurrentCanvas = BaseCanvasS;
	}

	for (auto& item : Alive)
		RetriveTargetUI(item);

	Alive.Reset();
}

TObjectPtr<URadarTargetUI> URadarUI::GetTargetUI()
{
	TObjectPtr<URadarTargetUI> newInstnace;

	while (newInstnace == nullptr && !Stored.IsEmpty())
		newInstnace = Stored.Pop(false);

	if (newInstnace == nullptr)
		newInstnace = CreateWidget<URadarTargetUI>(GetWorld(), TargetBluePrint);

	return newInstnace;
}

TObjectPtr<class URadarTargetUI> URadarUI::GetTargetUI(UCanvasPanel* AttachingPanel)
{
	TObjectPtr<URadarTargetUI> newInstnace;

	while (newInstnace == nullptr && !Stored.IsEmpty())
		newInstnace = Stored.Pop(false);

	if (newInstnace == nullptr)
		newInstnace = CreateWidget<URadarTargetUI>(GetWorld(), TargetBluePrint);

	if (newInstnace != nullptr)
	{
		newInstnace->Attach(AttachingPanel);
		newInstnace->SetVisibility(ESlateVisibility::Visible);
	}

	return newInstnace;
}

void URadarUI::RetriveTargetUI(TObjectPtr<URadarTargetUI> target)
{
	target->SetVisibility(ESlateVisibility::Hidden);
	Stored.Add(target);
}
