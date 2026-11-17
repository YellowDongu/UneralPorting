#include "WeaponInterface.h"
#include "StandardMissile.h"
#include "VehicleBase.h"
#include "VehicleAnimation.h"

UWeaponInterface::UWeaponInterface()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoRegister = true;
	bWantsInitializeComponent = false;
	bAutoActivate = false;

	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponInterface::Update(float DeltaTime)
{
	bEnemySpotted = !currentTargets.IsEmpty() && IsAlive(currentTargets[0].Target);

	UpdateEvent.Broadcast(DeltaTime);
	/*
	bulletTime = FMath::Min(bulletTime + DeltaTime, GunRPM);

	if (currentTargets.IsEmpty() || !IsAlive(currentTargets[0].Target))
	{
		*TurretYaw += FMath::Sign(FMath::UnwindDegrees(-*TurretYaw)) * DeltaTime * TurretYawRotateSpeed;
		*TurretPitch += FMath::Sign(FMath::UnwindDegrees(-*TurretPitch)) * DeltaTime * TurretPitchRotateSpeed;

		return;
	}

	FVector position = GetOwner()->GetActorLocation();

	FVector direction = currentTargets[0].Target->GetActorLocation() - position;
	FRotator rotator = direction.Rotation();

	TargetYaw = FMath::UnwindDegrees(FMath::UnwindDegrees(rotator.Yaw) - FMath::UnwindDegrees(*TurretYaw));
	TargetPitch = FMath::UnwindDegrees(FMath::UnwindDegrees(rotator.Pitch) - FMath::UnwindDegrees(*TurretPitch));

	*TurretYaw += FMath::Sign(TargetYaw) * DeltaTime * TurretYawRotateSpeed;
	*TurretPitch += FMath::Sign(TargetPitch) * DeltaTime * TurretPitchRotateSpeed;

	if (standard.IsEmpty())
		return;

	if (direction.SizeSquared() > Range || FMath::Abs(TargetYaw) > standardCos)
		currentTargets[0].Lock = FMath::Clamp(currentTargets[0].Lock - DeltaTime, 0.0f, 1.0f);
	else
		currentTargets[0].Lock = FMath::Clamp(currentTargets[0].Lock + DeltaTime, 0.0f, 1.0f);
		*/
}

void UWeaponInterface::SystemIntegration(TWeakObjectPtr<class URadar> radar, USceneComponent* WeaponContainer, const FBoxSphereBounds& bodyBoundary)
{
	auto list = GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), TEXT("Body"));
	if (list.IsEmpty() || !IsValid(list[0]))
		return;

	BodyMesh = StaticCast<USkeletalMeshComponent*>(list[0]); // Cast<USkeletalMeshComponent>(GetOwner()->GetRootComponent());
	UVehicleAnimation* Animation = Cast<UVehicleAnimation>(BodyMesh->GetAnimInstance());

	if (!IsValid(Animation))
		return;

	Range = Range * Range;
	AngleCos = FMath::Cos(FMath::DegreesToRadians(2.5));
	TurretYaw = Animation->GetTurretYaw();
	TurretPitch = Animation->GetTurretPitch();

	int BoneIndex = BodyMesh->GetBoneIndex(TurretBone);
	if (BoneIndex == -1) return;

	FTransform turretTransform = BodyMesh->GetBoneTransform(BoneIndex);
	//FTransform WorldTransform = BodyMesh->GetBoneTransform(BoneIndex, BodyMesh->GetComponentTransform());
	TurretPosition = turretTransform.GetLocation();

	FBoxSphereBounds MeshBoundary = BodyMesh->GetSkeletalMeshAsset()->GetBounds();
	//FVector BodyCenter = MeshBoundary.Origin;
	//FVector CenterFloor = BodyCenter;
	//FBoxSphereBounds Bounds = BodyMesh->GetSkeletalMeshAsset()->GetBounds();
	//
	//FVector GearCenter = Bounds.Origin;
	//FVector Size = Bounds.BoxExtent * 2;
	//float zMin = GearCenter.Z - (Size.Z * 0.5f);
	//CenterFloor.Z = zMin;

	Super::SystemIntegration(radar, WeaponContainer, MeshBoundary);

	UpdateEvent.AddUObject(this, &UWeaponInterface::TurretSet);

	if (bGunEnable)
		UpdateEvent.AddUObject(this, &UWeaponInterface::TrackGun);
	if (!standard.IsEmpty())
		UpdateEvent.AddUObject(this, &UWeaponInterface::TrackMissile);
}

void UWeaponInterface::TrackGun(float DeltaTime) { bulletTime = FMath::Min(bulletTime + DeltaTime, GunRPM); }

void UWeaponInterface::TrackMissile(float DeltaTime)
{
	if (bEnemySpotted)
		standard[0]->Lock(currentTargets[0], DeltaTime, GetOwner()->GetActorForwardVector(), GetOwner()->GetActorLocation());
}

void UWeaponInterface::TurretSet(float DeltaTime)
{
	if (bEnemySpotted)
	{
		FVector direction = currentTargets[0].Target->GetActorLocation() - GetOwner()->GetActorLocation();
		FRotator rotator = direction.Rotation();
		if (bGunEnable)
			TargetAngleCos = FVector::DotProduct(direction.GetSafeNormal(), GetOwner()->GetActorForwardVector());

		TargetYaw = FMath::UnwindDegrees(FMath::UnwindDegrees(rotator.Yaw) - FMath::UnwindDegrees(*TurretYaw));
		TargetPitch = FMath::UnwindDegrees(FMath::UnwindDegrees(rotator.Pitch) - FMath::UnwindDegrees(*TurretPitch));

		//YawDelta = FMath::Sign(TargetYaw) * DeltaTime * TurretYawRotateSpeed;
		//PitchDelta = FMath::Sign(TargetPitch) * DeltaTime * TurretPitchRotateSpeed;
	}
	else
	{
		TargetYaw = FMath::UnwindDegrees(-*TurretYaw);
		TargetPitch = FMath::UnwindDegrees(-*TurretPitch);

		//YawDelta = FMath::Sign(FMath::UnwindDegrees(-*TurretYaw)) * DeltaTime * TurretYawRotateSpeed;
		//PitchDelta = FMath::Sign(FMath::UnwindDegrees(-*TurretPitch)) * DeltaTime * TurretPitchRotateSpeed;
	}

	YawDelta = (FMath::Abs(TargetYaw) <= TurretYawRotateSpeed ? TargetYaw : FMath::Sign(TargetYaw) * TurretYawRotateSpeed) * DeltaTime;
	PitchDelta = (FMath::Abs(TargetPitch) <= TurretPitchRotateSpeed ? TargetPitch : FMath::Sign(TargetPitch) * TurretPitchRotateSpeed) * DeltaTime;

	*TurretYaw += YawDelta;
	*TurretPitch += PitchDelta;
}
