// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AircraftAnimator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AC7_API UAircraftAnimator : public UActorComponent
{
	GENERATED_BODY()
	struct FAnimationContainer
	{
	public:
		inline void Update() { if (ExplicitTime != nullptr) *ExplicitTime = FrameTime * MaxTime; }

		void Initialize(float* timePointer, float maxTime, float animationSpeed) { ExplicitTime = timePointer; if (MaxTime == -1.0f) MaxTime = maxTime; AnimationSpeed = animationSpeed; bActived = ExplicitTime != nullptr; }

		void SetFrameTime(float value) { FrameTime = FMath::Clamp(value, 0.0f, 1.0f); }
		//bool PlayFrameTime(float deltaTime, float targetFrameTime) { FrameTime = FMath::Clamp(FrameTime + FMath::Sign(targetFrameTime - FrameTime) * deltaTime * AnimationSpeed, 0.0f, 1.0f); return FMath::Abs(targetFrameTime - FrameTime) < 0.01f; }
		bool PlayFrameTime(float deltaTime, float targetFrameTime) { FrameTime = FMath::FInterpTo(FrameTime, targetFrameTime, deltaTime, AnimationSpeed); return FMath::Abs(targetFrameTime - FrameTime) < 0.01f; }
		float GetFrameTime() { return FrameTime; }

		bool bActived = false;
		//private:
		float* ExplicitTime{ nullptr };
		float FrameTime{ 0.0f };
		float MaxTime{ -1.0f };
		float AnimationSpeed{ 20.0f };
	};

public:
	UAircraftAnimator(); // 원랜 void 키워드를 컴파일러가 끼워넣는 작업을 알아서 해서 넣어도 되고 안넣어도 되는건데 언리얼 자체적인 해석에서 이 녀석을 문자 그대로 읽는 방법을 써서 void 키워드를 쓰면 안되는듯?

	virtual void SystemIntegration(struct FControl* control);
	virtual void Update(float DeltaTime) { FUpdate.Broadcast(DeltaTime); }

	void ReadComponent(void);
	void StaticGearActive(bool value);
	void DeployGear(bool value, bool instantSet = false);

	const FVector& GetDisplayCenterPosition(void) const { return CenterFloor; }
	const FBoxSphereBounds& GetBodyBoundary(void) const { return MeshBoundary; }

	bool IsGearActive(void) const { return bGearActive; }

private:
	void CalculatePosition(void);

protected:
	bool bGearActive{ true };
	float animationSpeed{ 10.0f };
	struct FControl* Control{ nullptr };

	FTickEvent FUpdate;
	FTimerHandle GearTimerHandle;
	FDelegateHandle GearUpdate;
	FVector CenterFloor, BodyCenter, GearCenter;
	FBoxSphereBounds MeshBoundary;

	FAnimationContainer Aileron;
	FAnimationContainer Rudder;
	FAnimationContainer Elevator;
	FAnimationContainer Gear;
	/*Variable Part->Speed*/
	FAnimationContainer SPVP;

	TWeakObjectPtr<class USkeletalMeshComponent> BodyMesh{ nullptr };
	TWeakObjectPtr<class USkeletalMeshComponent> GearMesh{ nullptr };

	TWeakObjectPtr<class UStaticMeshComponent> GearOnStaticMesh{ nullptr };
	TWeakObjectPtr<class UStaticMeshComponent> GearOffStaticMesh{ nullptr };
private:
	bool bComponentBinded, bNoStaticGear, bStaticGearActive, bNoGear;
};
