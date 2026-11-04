// Fill out your copyright notice in the Description page of Project Settings.


#include "F15EAnimator.h"
#include "Aircraft.h"
#include "AircraftAnimation.h"

UF15EAnimator::UF15EAnimator()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = false;
	PrimaryComponentTick.bCanEverTick = false;
}
