#include "AttitudeIndicator.h"
#include "PitchBar.h"
#include "Aircraft.h"
#include "PlayerPilot.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"

void UAttitudeIndicator::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	float pitch = GetEularAngle.Execute().Euler().Y / PitchSpace;
	int pitchRate = static_cast<int>(pitch);
	float nextSpace = (pitchRate - pitch) * space;
	int index = HalfHeight;

	if (Pitch != pitchRate)
	{
		if (Pitch > pitchRate)
		{
			Pitch--;
			auto item = Alive[0];
			Alive.RemoveAt(0);
			Release(item.Key, item.Value);
			AddPitchBar(pitchRate - 4, true);
		}
		else
		{
			Pitch++;
			auto item = Alive.Pop(false);
			Release(item.Key, item.Value);
			AddPitchBar(pitchRate + 4, false);
		}

	}

	//UE_LOG(LogClass, Warning, TEXT("%2f, %d, %d"), nextSpace, index, Alive.Num());
	for (auto& item : Alive)
		item.Key->SetPosition(FVector2D(0.0f, static_cast<float>(index--) * space + nextSpace));
}

TObjectPtr<UPitchBar> UAttitudeIndicator::AddPitchBar(int pitchLevel, bool back)
{
	TObjectPtr<UPitchBar> newInstance;
	PitchFlag flag;

	if (pitchLevel == 0)
		flag = PitchFlag::PitchCenter;
	else
		flag = (pitchLevel < 0) ? ((pitchLevel % 2 == 0) ? PitchFlag::PitchNeg : PitchFlag::PitchNegHalf) : ((pitchLevel % 2 == 0) ? PitchFlag::PitchPos : PitchFlag::PitchPosHalf);

	newInstance = Get(flag);
	if (newInstance == nullptr)
		return nullptr;

	newInstance->SetNumber(pitchLevel / 2 * 5);

	newInstance->SetUIColor(currentColor);
	newInstance->SetSideTextColor(currentColor);

	if (back)
		Alive.Add({ newInstance, flag });
	else
		Alive.Insert({ newInstance, flag }, 0);

	return newInstance;
}

void UAttitudeIndicator::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (PitchCenter == nullptr || PitchCenter->SetUp() == nullptr)
	{
		RemoveFromParent();
		return;
	}

	Pitch = 0;

	int number = -HalfHeight;
	space = -MaxHeight / static_cast<float>(HalfHeight);
	TObjectPtr<UPitchBar> newInstance{ nullptr };

	newInstance = AddPitchBar(-number);
	if (newInstance == nullptr) return;
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));
	newInstance = AddPitchBar(-number);
	if (newInstance == nullptr) return;
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));

	newInstance = AddPitchBar(-number);
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));
	newInstance = AddPitchBar(-number);
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));

	newInstance = AddPitchBar(-number);
	if (newInstance == nullptr) return;
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));
	//UE_LOG(LogClass, Warning, TEXT("%2f"), static_cast<float>(number - 1) * space);

	newInstance = AddPitchBar(-number);
	if (newInstance == nullptr) return;
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));
	newInstance = AddPitchBar(-number);
	if (newInstance == nullptr) return;
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));

	newInstance = AddPitchBar(-number);
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number++) * space));
	newInstance = AddPitchBar(-number);
	newInstance->SetPosition(FVector2D(0.0f, static_cast<float>(number) * space));

	bInitialized = true;
}

void UAttitudeIndicator::NativeConstruct()
{
	Super::NativeConstruct();
	return;

	if (!bBinded || !bInitialized)
	{
		RemoveFromParent();
		return;
	}

}

void UAttitudeIndicator::BindPlayer(class AAircraft* vehicle, class APlayerPilot* player)
{
	if (vehicle == nullptr || player == nullptr)
	{
		RemoveFromParent();
		return;
	}

	GetEularAngle.BindUObject(vehicle, &AAircraft::GetActorRotation);
	bBinded = true;
}

TObjectPtr<UPitchBar> UAttitudeIndicator::Get(PitchFlag enumFlag)
{
	int flag = 0;
	switch (enumFlag)
	{
	case PitchFlag::PitchCenter:
		PitchCenter->SetVisibility(ESlateVisibility::Visible);
		return PitchCenter;
	default:
		flag = FlagConvert(enumFlag);
		break;
	}

	TObjectPtr<UPitchBar> returnPointer;
	if (Pool[flag].IsEmpty())
	{
		switch (enumFlag)
		{
		case PitchFlag::PitchNeg:
			returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchNeg);
			break;
		case PitchFlag::PitchNegHalf:
			returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchNegHalf);
			break;
		case PitchFlag::PitchPos:
			returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchPos);
			break;
		case PitchFlag::PitchPosHalf:
			returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchPosHalf);
			break;
		default:
			return nullptr;
		}

		if (returnPointer == nullptr)
			return nullptr;

		returnPointer = returnPointer->SetUp(BasePanel);
		if (returnPointer == nullptr)
			return nullptr;

		return returnPointer;
	}
	else
	{
		while (returnPointer != nullptr)
			returnPointer = Pool[flag].Pop(false);

		if (returnPointer == nullptr) // 함수화 안함. CTRL CV가 편해
		{
			switch (enumFlag)
			{
			case PitchFlag::PitchNeg:
				returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchNeg);
				break;
			case PitchFlag::PitchNegHalf:
				returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchNegHalf);
				break;
			case PitchFlag::PitchPos:
				returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchPos);
				break;
			case PitchFlag::PitchPosHalf:
				returnPointer = CreateWidget<UPitchBar>(GetWorld(), PitchPosHalf);
				break;
			default:
				return nullptr;
			}

			if (returnPointer == nullptr)
				return nullptr;
			return returnPointer->SetUp(BasePanel);
		}

		returnPointer->SetVisibility(ESlateVisibility::Visible);
		return returnPointer;
	}
}

void UAttitudeIndicator::Release(const TObjectPtr<UPitchBar>& ui, PitchFlag flag)
{
	if (ui == nullptr)
		return;

	ui->SetVisibility(ESlateVisibility::Hidden);

	if (flag == PitchFlag::PitchCenter)
		return;

	Pool[FlagConvert(flag)].Add(ui);
}

FORCEINLINE int UAttitudeIndicator::FlagConvert(PitchFlag flag)
{
	switch (flag)
	{
	case PitchFlag::PitchNeg:
		return 0;
	case PitchFlag::PitchNegHalf:
		return 1;
	case PitchFlag::PitchPos:
		return 2;
	case PitchFlag::PitchPosHalf:
		return 3;
	case PitchFlag::PitchCenter:
		return 4;
	default:
		return 0;
	}
}

void UAttitudeIndicator::ChangeColor(FLinearColor color)
{
	if (currentColor == color)
		return;

	currentColor = color;
	for (auto& item : Alive)
	{
		item.Key->SetUIColor(currentColor);
		item.Key->SetSideTextColor(currentColor);
	}
}
