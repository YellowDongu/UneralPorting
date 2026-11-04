// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleAnimation.h"


void UVehicleAnimation::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if (BaseAnimation != nullptr)
        MaxLength = BaseAnimation->GetPlayLength();
}

void UVehicleAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

}