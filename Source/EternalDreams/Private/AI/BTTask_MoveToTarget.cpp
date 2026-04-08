// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_MoveToTarget.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
	NodeName = TEXT("Move To Target");
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToTarget, BlackboardKey), AActor::StaticClass());
	BlackboardKey.SelectedKeyName = TEXT("TargetActor");
}
