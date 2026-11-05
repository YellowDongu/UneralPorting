#include "WeaponInterface.h"
#include "StandardMissile.h"
#include "VehicleBase.h"
#include "VehicleAnimation.h"

UWeaponInterface::UWeaponInterface()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoRegister = false;
	bWantsInitializeComponent = false;
	bAutoActivate = false;

	PrimaryComponentTick.bCanEverTick = false;
}



void UWeaponInterface::Update(float DeltaTime)
{
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
}

void UWeaponInterface::SystemIntegration(TWeakObjectPtr<class URadar> radar, USceneComponent* WeaponContainer, const FBoxSphereBounds& bodyBoundary)
{
	BodyMesh = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), TEXT("Body"))[0]);
	//BodyMesh = Cast<USkeletalMeshComponent>(GetOwner()->GetRootComponent());
	UVehicleAnimation* Animation = Cast<UVehicleAnimation>(BodyMesh->GetAnimInstance());
	check(Animation);

	Range = Range * Range;
	TurretYaw = Animation->GetTurretYaw();
	TurretPitch = Animation->GetTurretPitch();

	int BoneIndex = BodyMesh->GetBoneIndex(TurretBone);
	if (BoneIndex == -1) check(nullptr);

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

	if (!standard.IsEmpty())
		standardCos = standard[0]->GetLockAngle();
}
