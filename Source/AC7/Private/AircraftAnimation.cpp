// Fill out your copyright notice in the Description page of Project Settings.


#include "AircraftAnimation.h"

void UAircraftAnimation::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    if (BaseAnimation != nullptr)
        MaxLength = BaseAnimation->GetPlayLength();
}

void UAircraftAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

}