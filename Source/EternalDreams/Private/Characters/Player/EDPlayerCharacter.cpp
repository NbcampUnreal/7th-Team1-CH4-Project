// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/EDPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
AEDPlayerCharacter::AEDPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Camera 관련 생성
	SpringArm=CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = SpringArmLength;
	SpringArm->SetupAttachment(RootComponent);
	Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	// ASC 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
}

// Called when the game starts or when spawned
void AEDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (BPBaseAttributeSet)
	{
		BaseAttributeSet=NewObject<UEDBaseAttributeSet>();
	}
	if (BPPlayerAttributeSet)
	{
		PlayerAttributeSet=NewObject<UEDPlayerAttributeSet>();
	}
	
	InitializeAbilitySystem();

}

// Called every frame
void AEDPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* AEDPlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEDPlayerCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

