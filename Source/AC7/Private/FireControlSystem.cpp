// Fill out your copyright notice in the Description page of Project Settings.


#include "FireControlSystem.h"
#include "StandardMissile.h"
#include "Aircraft.h"
#include "Radar.h"
#include "ObjectFactory.h"
#include "UnGuidedBomb.h"
#include "Components/ChildActorComponent.h"
#include "Components/StaticMeshComponent.h"

void FSpecialMissileSlot::Initialize(USceneComponent* slotRoot, bool active)
{
	ID = EWeaponID::END;
	Root = slotRoot;
	//UE_LOG(LogTemp, Error, TEXT("[FSpecialMissileSlot::Initialize] Root - %s"), *Root->GetName())
	TArray<UChildActorComponent*> slot;
	for (USceneComponent* innerChild : slotRoot->GetAttachChildren())
	{
		UChildActorComponent* ChildActor = Cast<UChildActorComponent>(innerChild);
		if (ChildActor != nullptr)
		{
			slot.Add(ChildActor);
			continue;
		}

		UStaticMeshComponent* ChildMesh = Cast<UStaticMeshComponent>(innerChild);
		if (ChildMesh != nullptr)
		{
			MeshSlot.Add(ChildMesh);
			continue;
		}
	}

	slot.Sort([](const UChildActorComponent& A, const UChildActorComponent& B)
			  {
				  if (A.GetRelativeLocation().Y == B.GetRelativeLocation().Y)
					  return A.GetRelativeLocation().X < B.GetRelativeLocation().X;
				  return A.GetRelativeLocation().Y < B.GetRelativeLocation().Y;
			  });

	MissileSlot.Reserve(slot.Num());

	for (int i = 0; i < slot.Num(); i++)
	{
		AMissile* missile = Cast<AMissile>(slot[i]->GetChildActor());
		if (missile != nullptr)
		{
			//UE_LOG(LogTemp, Error, TEXT("[FSpecialMissileSlot::Initialize] SpelcialMissileAdded - %s"), *missile->GetName())
			MissileSlot.Add(missile);
		}
	}

	bIsActive = !active;
	SetActive(active);
	ID = MissileSlot[0]->GetWeaponID();
}

void FSpecialMissileSlot::SetActive(bool value)
{
	if (bIsActive == value)
		return;

	bIsActive = value;

	if (Root.IsValid())
		Root->SetVisibility(value, true);

	for (auto& missile : MissileSlot)
	{
		if (missile.IsValid())
			missile->SetActorHiddenInGame(!value);
	}
}


UFireControlSystem::UFireControlSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = false;

	PrimaryComponentTick.bCanEverTick = false;
}

void UFireControlSystem::Update(float DeltaTime)
{
	bulletTime = FMath::Min(bulletTime + DeltaTime, GunRPM);

	if (currentTargets.IsEmpty())
		return;

	FVector forward = GetOwner()->GetActorForwardVector();
	FVector position = GetOwner()->GetActorLocation();

	if (bSpecialSelected)
	{
		//float distance = specialSlots[special].MissileSlot[0]->GetMaxRange();
		//distance *= distance;
		//FVector direction;
		for (auto& item : currentTargets)
			specialSlots[SpecialWeapon].MissileSlot[0]->Lock(item, DeltaTime, forward, position);
		//{
		//	direction = item.Target->GetActorLocation() - position;
		//
		//	if (direction.SizeSquared() > distance)
		//		item.Lock = FMath::Clamp(item.Lock + DeltaTime, 0.0f, 1.0f);
		//
		//	if (FVector::DotProduct(direction.GetSafeNormal(), forward) > standardCos)
		//		item.Lock = FMath::Clamp(item.Lock - DeltaTime, 0.0f, 1.0f);
		//}
	}
	else
	{
		standard[0]->Lock(currentTargets[0], DeltaTime, forward, position);
		UE_LOG(LogTemp, Warning, TEXT("[UFireControlSystem::Update] %.3f"), currentTargets[0].Lock);
		//FVector direction = currentTargets[0].Target->GetActorLocation() - position;
		//float distance = standard[0]->GetMaxRange();
		//
		//if (direction.SizeSquared() > distance * distance)
		//	currentTargets[0].Lock = FMath::Clamp(currentTargets[0].Lock + DeltaTime, 0.0f, 1.0f);
		//else if (FVector::DotProduct(direction.GetSafeNormal(), forward) > standardCos)
		//	currentTargets[0].Lock = FMath::Clamp(currentTargets[0].Lock - DeltaTime, 0.0f, 1.0f);
	}
}

void UFireControlSystem::SystemInitialize(EWeaponID specialSelected)
{
	for (auto& item : specialSlots)
		item.SetActive(false);

	SpecialWeapon = INDEX_NONE;

	for (int i = 0; i < specialSlots.Num(); i++)
	{
		if(specialSlots[i].ID == specialSelected)
		SpecialWeapon = i;
	}

	if (SpecialWeapon == INDEX_NONE)
		return;

	specialSlots[SpecialWeapon].SetActive(true);
	if (specialSlots[SpecialWeapon].MissileSlot.IsEmpty())
		return;

	specialCos = FMath::Cos(FMath::DegreesToRadians(specialSlots[SpecialWeapon].MissileSlot[0]->GetLockAngle()));
	maxDegree = FMath::Max(standardCos, specialCos);
	MaxCount = specialSlots[SpecialWeapon].MissileSlot.Num();
	MultiShoot = specialSlots[SpecialWeapon].MissileSlot[0]->GetMultiShoot();
}

void UFireControlSystem::SystemIntegration(TWeakObjectPtr<class URadar> _radar, USceneComponent* WeaponContainer, const FBoxSphereBounds& bodyBoundary)
{
	if (_radar == nullptr || WeaponContainer == nullptr)
		return;

	AAircraft* Owner = Cast<AAircraft>(GetOwner());
	if (Owner != nullptr)
		Owner->GetControl(Control);

	Radar = _radar;
	AddHandle = Radar->AddTarget.AddUObject(this, &UFireControlSystem::AddTarget);
	RemoveHandle = Radar->RemoveTarget.AddUObject(this, &UFireControlSystem::RemoveTarget);

	GunPosition = bodyBoundary.Origin;
	GunPosition.X += bodyBoundary.BoxExtent.X * 1.25;
	GunRPM = 1.0f / (GunRPM / 60.0f);
	refreshTimer = -1.0f;

	WeaponIntegration(WeaponContainer);

	//GetWorld()->GetTimerManager().SetTimer(
	//	RefreshFunctionHandle, FTimerDelegate::CreateWeakLambda(this, [this, Vehicle = TWeakObjectPtr<AVehicleBase>(Cast<AVehicleBase>(GetOwner()))]()
	//															{
	//																if (IsAlive(Vehicle))
	//																	RefreshTarget(false);
	//															}),
	//									   1.0f, true);
	//
}

bool UFireControlSystem::Lockable(const AVehicleBase* vehicle, const FWeaponMask& mask) const { return vehicle->GetMask().MaskMatch(mask); }
bool UFireControlSystem::CalculateBombLandingPosition(FVector& OutLandingPosition) { return CalculateBombLandingPosition(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorForwardVector() * static_cast<double>(ConvertFromVelocity(static_cast<AAircraft*>(GetOwner())->GetControl().Velocity)), OutLandingPosition, CalculatedBombTime); }
const FWeaponMask& UFireControlSystem::GetMissileAimLayer() const { return bSpecialSelected ? specialSlots[SpecialWeapon].MissileSlot[0]->GetAimLayer() : standard[0]->GetAimLayer(); }
const FWeaponMask& UFireControlSystem::GetMissileAimLayer(bool isSpecial) const
{
	if (isSpecial && SpecialWeapon != -1)
		return specialSlots[SpecialWeapon].MissileSlot[0]->GetAimLayer();
	return standard[0]->GetAimLayer();
}

EWeaponID UFireControlSystem::GetWeaponID(bool specialMissile) const
{
	if (specialMissile)
	{
		if (SpecialWeapon == -1)
			return EWeaponID::NULLID;
		return specialSlots[SpecialWeapon].MissileSlot[0]->GetWeaponID();
	}
	return EWeaponID::Standard;
}


TWeakObjectPtr<class AMissile> UFireControlSystem::GetWeapon(int specialWeaponIndex)
{
	if (specialWeaponIndex < 0 || specialWeaponIndex >= specialSlots.Num())
		return nullptr;

	return specialSlots[specialWeaponIndex].MissileSlot[0];
}


bool UFireControlSystem::WeaponIntegration(USceneComponent* WeaponComp)
{
	if (WeaponComp == nullptr)
		return false;

	standard.Reset();
	standard.Reserve(2);

	for (USceneComponent* Child : WeaponComp->GetAttachChildren())
	{
		if (Child->GetFName() == TEXT("Standard"))
		{
			for (USceneComponent* innerChild : Child->GetAttachChildren())
			{
				UChildActorComponent* ChildActor = Cast<UChildActorComponent>(innerChild);
				if (ChildActor == nullptr)
					continue;

				AStandardMissile* Missile = Cast<AStandardMissile>(ChildActor->GetChildActor());
				if (Missile != nullptr)
					standard.Add(Missile);
			}
		}
		if (Child->GetFName() == TEXT("Special"))
		{
			int index = 0;
			for (USceneComponent* innerChild : Child->GetAttachChildren())
			{
				index++;
				specialSlots.Add({});
				specialSlots.Last().Initialize(innerChild, false);
			}
		}

	}

	if (standard.Num() > 0)
		maxDegree = standardCos = FMath::Cos(FMath::DegreesToRadians(standard[0]->GetLockAngle()));

	return standard.Num() > 0;
}

void UFireControlSystem::RefreshTarget(bool changeCurrentTarget)
{
	Targets.Reset();
	currentTargets.Reset();
	const auto& targets = Radar->GetTargets();
	if (targets.IsEmpty()) return;

	FVector forward = GetOwner()->GetActorForwardVector();
	FVector position = GetOwner()->GetActorLocation();

	for (auto& item : targets)
		AddTarget(item, forward, position);

	if (!changeCurrentTarget)
		return;

	auto beginIterator = Targets.begin();

	FTargetInfo targetInfo;

	targetInfo.LockReset();
	targetInfo.Key = (*beginIterator).Key;
	targetInfo.Target = (*beginIterator).Value;
	currentTargets.Add(MoveTemp(targetInfo));

	if (bSpecialSelected)
		RefreshSubTargets();
}

void UFireControlSystem::RefreshSubTargets(void)
{
	if (currentTargets.IsEmpty())
		return;

	currentTargets.SetNum(1);

	TSet<TWeakObjectPtr<AVehicleBase>> containedTargets;
	containedTargets.Add(currentTargets[0].Target);
	TArray<float> keys;
	Targets.GetKeys(keys);

	int startIndex = INDEX_NONE, length = Targets.Num();
	const float* keyPtr = Targets.FindKey(currentTargets[0].Target);
	if (keyPtr == nullptr || !keys.Find(*keyPtr, startIndex))
	{
		currentTargets.Reset();
		return;
	}

	FTargetInfo targetInfo;
	for (int i = 0; i < length && currentTargets.Num() < MultiShoot; i++)
	{
		float key = keys[(i + startIndex) % length];
		if (key > 1.0f || key < -1.0f)
			continue;

		const auto& item = *Targets.Find(key);
		if (containedTargets.Contains(item))
			continue;

		containedTargets.Add(item);

		targetInfo.Key = key;
		targetInfo.LockReset();
		targetInfo.Target = item;
		currentTargets.Add(MoveTemp(targetInfo));
		//SetSelectTargetUI.Broadcast(targetInfo.Target.Get(), true);
	}

	if constexpr (bInsertMode)
		return;

	if (currentTargets.Num() >= MultiShoot)
		return;

	startIndex = currentTargets.Num();
	for (int i = startIndex; i < MultiShoot; i++)
	{
		const FTargetInfo& item = currentTargets[(i - startIndex) % startIndex];
		currentTargets.Add(item);
	}
}

TWeakObjectPtr<class AMissile> UFireControlSystem::GetMissile(bool isSpecial)
{
	if (!isSpecial) return Cast<AMissile>(standard[0]);
	else if (SpecialWeapon != -1) return Cast<AMissile>(specialSlots[SpecialWeapon].MissileSlot[0]);
	return nullptr;
}

void UFireControlSystem::ChangeTarget()
{
	if(currentTargets.IsEmpty())
	{
		RefreshTarget();
		return;
	}

	auto currentTarget = (*currentTargets.begin()).Target;

	auto targetIterator = Targets.CreateIterator();
	while (targetIterator && targetIterator->Value != currentTarget)
		++targetIterator;

	if (targetIterator) ++targetIterator;
	if (!targetIterator)
	{
		RefreshTarget();
		return;
	}

	FTargetInfo targetInfo;
	currentTargets.Reset();

	targetInfo.LockReset();
	targetInfo.Key = targetIterator->Key;
	targetInfo.Target = targetIterator->Value;
	currentTargets.Add(MoveTemp(targetInfo));

	if (bSpecialSelected)
		RefreshSubTargets();
}

void UFireControlSystem::AddTarget(AVehicleBase* target) { AddTarget(TWeakObjectPtr<AVehicleBase>(target), GetOwner()->GetActorForwardVector(), GetOwner()->GetActorLocation()); }
void UFireControlSystem::AddTarget(const TWeakObjectPtr<AVehicleBase>& target, const FVector& forward, const FVector& position)
{
	if (!IsAlive(target))
		return;

	FVector direction = target->GetActorLocation() - position;
	float dot = FVector::DotProduct(direction.GetSafeNormal(), forward);

	Targets.Add(dot < maxDegree ? dot + direction.SizeSquared() : -dot, target);
}

void UFireControlSystem::SetTarget(const TWeakObjectPtr<AVehicleBase>& target)
{
	currentTargets.Reset();
	currentTargets.Emplace(target);
}

void UFireControlSystem::RemoveTarget(AVehicleBase* target)
{
	const float* keyPtr = Targets.FindKey(target);
	if (keyPtr)
		Targets.Remove(*keyPtr);

	int index = INDEX_NONE;

	for (int i = 0; i < currentTargets.Num(); i++)
	{
		if (currentTargets[i].Target != target)
			continue;

		index = i;
		break;
	}

	if (index == INDEX_NONE)
		return;

	if (index == 0)
		ChangeTarget();
	else
		RefreshSubTargets();
}

bool UFireControlSystem::ShootMissile()
{
#ifndef BlockAISpecial
	if(!bSpecialSelected)
	{
#endif
	if (currentTargets.IsEmpty() || currentTargets[0].Lock > 0.001f)
		return false;

	for (auto& item : standard)
	{
		if (item->IsCoolTime())
			continue;

		item->Shoot(static_cast<AVehicleBase*>(GetOwner()), currentTargets[0].Target.Get());
		return true;
	}
	return false;
#ifndef BlockAISpecial
	}
	else
	{
		int count = 0, outputCount = 0;

		for (int i = 0; i < specialSlots[special].MissileSlot.Num(); i++)
		{
			int result = specialSlots[special].MissileSlot[i]->Shoot(static_cast<AVehicleBase*>(GetOwner()), count < currentTargets.Num() ? &currentTargets[count] : nullptr);

			if (result == 0)
			{ // not lock
				count++;
				i--;
			}
			else if (result == 1)
			{ // shoot
				count++;
				outputCount++;
				if (outputCount >= MultiShoot)
					break;
			}
		}

		return outputCount != 0;
	}
#endif
}

bool UFireControlSystem::ShootMissile(TArray<int>& output)
{
	output.Reset();
	if (bSpecialSelected)
	{
		int count = 0, outputCount = 0;

		for (int i = 0; i < specialSlots[SpecialWeapon].MissileSlot.Num(); i++)
		{
			int result = specialSlots[SpecialWeapon].MissileSlot[i]->Shoot(static_cast<AVehicleBase*>(GetOwner()), count < currentTargets.Num() ? &currentTargets[count] : nullptr);

			if (result == 0)
			{ // not lock
				count++;
				i--;
			}
			else if (result == 1)
			{ // shoot
				count++;
				outputCount++;
				output.Add(i);

				if (outputCount >= MultiShoot)
					break;
			}
		}

		if (outputCount != 0) // if shooted more then one
			return true;

		for (int i = 0; i < specialSlots[SpecialWeapon].MissileSlot.Num(); i++) //shoot at empty space
			if (specialSlots[SpecialWeapon].MissileSlot[i]->Shoot(static_cast<AVehicleBase*>(GetOwner()), TWeakObjectPtr<AVehicleBase>()))
				return true;

		return false; // all missile is in cooltime
	}
	else
	{
		int max = standard.Num();
		TWeakObjectPtr<AVehicleBase> target{ nullptr };
		if (!currentTargets.IsEmpty() && currentTargets[0].Lock <= 0.001f)
			target = currentTargets[0].Target;

		for (int i = 0; i < max; i++)
		{
			if (!standard[i]->Shoot(static_cast<AVehicleBase*>(GetOwner()), target))
				continue;

			output.Add(i);
			return true;

			//if (standard[i]->IsCoolTime())
			//	continue;
			//
			//standard[i]->Shoot(currentTargets[0].Target.Get());
			//output.Add(i);
			//return true;

		}
		return false;
	}
}
#include "Bullet.h"
bool UFireControlSystem::ShootGun()
{
	if (bulletTime < GunRPM)
	    return false;
	
	UWorld* world = GetWorld();
	UObjectFactory* factory = world->GetSubsystem<UObjectFactory>();
	check(factory);

	bulletTime = 0.0f;
	FRotator rotation = GetOwner()->GetActorRotation();
	//factory->Instantiate(GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 50.0f, GetOwner()->GetActorRotation(), 4500.0f, GetOwner());
	auto instance = factory->Instantiate(*world, GetOwner()->GetActorLocation() + rotation.RotateVector(GunPosition * 0.5), rotation, ConvertFromVelocity(Control->Velocity) * 10.0f);
	UE_LOG(LogTemp, Log, TEXT("[UFireControlSystem::ShootGun] shoot, %s"), *instance->GetName());
	return true;
}

bool UFireControlSystem::ShootGun(const FVector& shootPosition, const FRotator& rotation)
{
	if (bulletTime < GunRPM)
		return false;

	UWorld* world = GetWorld();
	UObjectFactory* factory = world->GetSubsystem<UObjectFactory>();
	check(factory);

	bulletTime = 0.0f;
	factory->Instantiate(*world, GetOwner()->GetActorLocation() + shootPosition, rotation, 4500.0f);
	return true;
}

bool UFireControlSystem::CalculateBombLandingPosition(UWorld* World, const FVector& StartPosition, const FVector& InitialVelocity, FVector& OutLandingPosition, float& landingTime)
{
	if (!World)
		return false;

	static constexpr float CoarseTimeStep = 0.5f;
	static constexpr float MaxTime = 60.0f;
	static constexpr float StartTime = 0.5f; // 시작시간을 정한 이유는 별거 없고 본인 검출 방지. 근데 딱히 그럴 필요까진 없고 계산 상 손해도 없어서 그냥 놔두는 중
	static constexpr int32 PrecisionIterations = 30;

#define CalculatePosition(timeValue) AUnGuidedBomb::CalculatePosition(StartPosition, InitialVelocity, timeValue)

	//auto CalculatePosition = [&StartPosition, &InitialVelocity](float Time) -> FVector
	//	{
	//		return StartPosition + InitialVelocity * Time + 0.5f * BombGravity * Time * Time;
	//	};

	auto CheckCollision = [World](const FVector& Start, const FVector& End, FHitResult& OutHit) -> bool
		{
			return World->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_WorldStatic);
		};

	float previousTime = StartTime;
	//FVector previousPosition = StartPosition;
	FVector previousPosition = CalculatePosition(previousTime);

	float currentTime = StartTime + CoarseTimeStep;

	FHitResult HitResult;
	while (currentTime <= MaxTime) // 시간별로 쪼개서 진행 -> StartTime부터 CoarseTimeStep 간격으로 MaxTime까지 진행
	{
		FVector currentPosition = CalculatePosition(currentTime);

		// 이전 위치에서 현재 위치까지 레이를 쏴서 지면(ECC_WorldStatic)을 통과했는지 검사
		if (CheckCollision(previousPosition, currentPosition, HitResult))
		{
			// 충돌이 발생한 시간 구간인 곳임, previousTime ~ currentTime
			// 이 사이를 이분 탐색

			float left = previousTime;
			float right = currentTime;

			FHitResult hit;
			for (int32 i = 0; i < PrecisionIterations; ++i)
			{
				float middle = (left + right) * 0.5f;
				FVector MiddlePosition = CalculatePosition(middle);

				if (CheckCollision(CalculatePosition(left), MiddlePosition, hit))
					right = middle;
				else
					left = middle;
			}

			// 최종 충돌 위치 좁히기
			FVector start = CalculatePosition(left);
			FVector end = CalculatePosition(right);

			landingTime = right;
			if (CheckCollision(start, end, hit))
				OutLandingPosition = hit.Location;
			else // 이론상 도달하지 않지만 안전장치
				OutLandingPosition = CalculatePosition(right);

			return true;
		}

		previousTime = currentTime;
		previousPosition = currentPosition;

		currentTime += CoarseTimeStep;
	}

	return false;
#undef CalculatePosition
}