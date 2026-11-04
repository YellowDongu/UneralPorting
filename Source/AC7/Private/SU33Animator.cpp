// Fill out your copyright notice in the Description page of Project Settings.


#include "SU33Animator.h"
#include "Aircraft.h"
#include "AircraftAnimation.h"

USU33Animator::USU33Animator()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	bAutoRegister = true;
	bWantsInitializeComponent = true;
	bAutoActivate = false;
	PrimaryComponentTick.bCanEverTick = false;
}
