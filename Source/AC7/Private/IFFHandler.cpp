// Fill out your copyright notice in the Description page of Project Settings.


#include "IFFHandler.h"
#include "Radar.h"
#include "IFFHUD.h"
#include "Aircraft.h"
#include "PlayerPilot.h"
#include "FireControlSystem.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "LockHUD.h"

UIFFHandler::UIFFHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UIFFHandler::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	for (auto& pair : Alive)
	{
		if (pair.Value != nullptr)
			pair.Value->Update(DeltaTime);
	}
}

//void UIFFHandler::AddTarget(AVehicleBase* Target)
//{
//	TObjectPtr<UIFFHUD> newInstnace{ nullptr };
//	while (!Stored.IsEmpty())
//	{
//		newInstnace = Stored.Pop();
//		if (newInstnace != nullptr)
//			break;
//	}
//
//	if (newInstnace == nullptr)
//	{
//		newInstnace = CreateWidget<UIFFHUD>(GetWorld(), BluePrint);
//		if (newInstnace == nullptr)
//			return;
//
//		newInstnace->RetriveUI.AddUObject(this, &UIFFHandler::RemoveTarget);
//	}
//
//	UCanvasPanelSlot* CanvasSlot = BaseCanvas->AddChildToCanvas(newInstnace);
//	if (CanvasSlot == nullptr)
//	{
//		Stored.Add(newInstnace);
//		return;
//	}
//
//	FAnchors AnchorPresets;
//	//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
//	//CanvasSlot->SetAnchors(AnchorPresets);
//	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
//	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
//	CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
//	//CanvasSlot->SetZOrder(10);
//
//	newInstnace->StartTrack(Target, Player.Get(), CanvasSlot);
//	UE_LOG(LogTemp, Log, TEXT("StartTrack: %s"), *Target->GetName());
//	Alive.Add({ Target, newInstnace });
//}

void UIFFHandler::RemoveTarget(UIFFHUD* target)
{
	if (target == nullptr)
		return;

	target->DeselectTarget();
	target->Active(false);
	target->SetVisibility(ESlateVisibility::Hidden);
	//Target->RemoveFromParent();
	Stored.Add(target);
	Alive.Remove(target->GetTargetVehicle());
}

//void UIFFHandler::RemoveTarget(AVehicleBase* Target)
//{
//	TObjectPtr<UIFFHUD>* removeTarget = Alive.Find(Target);
//
//	if (removeTarget == nullptr)
//		return;
//
//	(*removeTarget)->DeselectTarget();
//	(*removeTarget)->Active(false);
//	(*removeTarget)->SetVisibility(ESlateVisibility::Hidden);
//
//	//(*removeTarget)->RemoveFromParent();
//	Stored.Add(*removeTarget);
//	Alive.Remove(Target);
//	UE_LOG(LogTemp, Log, TEXT("EndTrack: %s"), *Target->GetName());
//}

void UIFFHandler::SelectTarget(AVehicleBase* target, bool isSelected)
{
	TObjectPtr<UIFFHUD>* removeTarget = Alive.Find(target);

	if (removeTarget == nullptr)
		return;

	if (isSelected)
		(*removeTarget)->SelectTarget();
	else
		(*removeTarget)->DeselectTarget();
}

void UIFFHandler::BindPlayer(AAircraft* vehicle, APlayerPilot* player)
{
	Player = vehicle;
	PlayerRadar = vehicle->GetRadar();
	if (PlayerRadar == nullptr)
	{
		Player->SystemIntegration();
		if (PlayerRadar == nullptr)
			return;
	}

	PlayerRadar->BindUI(this);

	//AddHandle = PlayerRadar->AddTarget.AddUObject(this, &UIFFHandler::AddTarget);
	//RemoveHandle = PlayerRadar->RemoveTarget.AddUObject(this, &UIFFHandler::RemoveTarget);

	player->SetSelectTargetUI.AddUObject(this, &UIFFHandler::SelectTarget);
	//vehicle->GetFireControlSystem()->SetSelectTargetUI.AddUObject(this, &UIFFHandler::SelectTarget);
	BaseCanvas->SetClipping(EWidgetClipping::ClipToBoundsWithoutIntersecting);
}

void UIFFHandler::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);

	if (vehicle != nullptr)
	{
		if (Player == vehicle)
			return;

		if (Alive.Contains(vehicle))
			return;

		TObjectPtr<UIFFHUD> newInstnace{ nullptr };
		while (!Stored.IsEmpty())
		{
			newInstnace = Stored.Pop();
			if (IsValid(newInstnace))
				break;
		}

		if (!IsValid(newInstnace))
		{
			newInstnace = CreateWidget<UIFFHUD>(GetWorld(), BluePrint);
			if (newInstnace == nullptr)
				return;

			newInstnace->RetriveUI.AddUObject(this, &UIFFHandler::RemoveTarget);
		}

		
		UCanvasPanelSlot* CanvasSlot = BaseCanvas->AddChildToCanvas(newInstnace);
		if (!IsValid(CanvasSlot))
		{
			Stored.Add(newInstnace);
			return;
		}

		FAnchors AnchorPresets;
		//AnchorPresets.Maximum = AnchorPresets.Minimum = FVector2D(0.5f, 0.5f);
		//CanvasSlot->SetAnchors(AnchorPresets);
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
		//CanvasSlot->SetZOrder(10);

		newInstnace->SetVisibility(ESlateVisibility::Visible);
		newInstnace->StartTrack(vehicle, Player.Get(), CanvasSlot);
		Alive.Add({ vehicle, newInstnace });
	}
}

void UIFFHandler::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AVehicleBase* vehicle = Cast<AVehicleBase>(OtherActor);

	if (vehicle != nullptr)
	{
		TObjectPtr<UIFFHUD>* removeTarget = Alive.Find(vehicle);

		if (removeTarget == nullptr)
			return;
		Alive.Remove(vehicle);

		if (!IsValid(*removeTarget))
			return;

		(*removeTarget)->DeselectTarget();
		(*removeTarget)->Active(false);
		(*removeTarget)->SetVisibility(ESlateVisibility::Hidden);

		//(*removeTarget)->RemoveFromParent();
		Stored.Add(*removeTarget);

		//UE_LOG(LogTemp, Log, TEXT("EndTrack: %s"), *Target->GetName());
		//InDistance.Remove(vehicle);
		//RemoveTarget.Broadcast(vehicle);
	}
}
