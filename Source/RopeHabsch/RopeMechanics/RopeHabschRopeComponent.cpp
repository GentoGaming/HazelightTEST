#include "RopeHabschRopeComponent.h"
#include "CableComponent.h"
#include "RopeHabschAttachPoint.h"
#include "RopeHabschCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "DataAssets/HooksDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


void URopeHabschRopeComponent::TurnOnOffMovement(bool Condition)
{
	if (Condition)
	{
		MovementComponent->GravityScale = 1;
		Player->SetActorEnableCollision(true);
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_Walking);
		Player->KeyBoardEnabled = true;
	}
	else
	{
		MovementComponent->GravityScale = 0;
		Player->SetActorEnableCollision(false);
		MovementComponent->StopMovementImmediately();
		MovementComponent->SetMovementMode(MOVE_Falling);
		Player->KeyBoardEnabled = false;
		
	}
}

// Sets default values for this component's properties
URopeHabschRopeComponent::URopeHabschRopeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void URopeHabschRopeComponent::StartHook()
{
	if (ClosestAttachPoint != nullptr && CurrentInUseAttachPoint == nullptr)
	{
		// If The Point is Ready And in Focus and in range 100 
		bool bCanUSe = ClosestAttachPoint->UseAttachPoint();
		if (!bCanUSe) return;
		// AttackPoint currently used
		CurrentInUseAttachPoint = ClosestAttachPoint;

		// Prepare for Hook
		FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(Player->GetActorLocation(),
		                                                          CurrentInUseAttachPoint->ArrowComponent->
		                                                          GetComponentLocation());
		Rotator.Pitch = 0;
		Rotator.Roll = 0;
		Player->SetActorRotation(Rotator);
		const bool bPlayerInAir = MovementComponent->IsFalling();

		TurnOnOffMovement(false);

		PlayerToAttachPointDirection = (Player->GetActorLocation() - CurrentInUseAttachPoint->GetActorLocation());
		PlayerToAttachPointDirection.Normalize();

		FinalDestination = CurrentInUseAttachPoint->ArrowComponent->GetComponentLocation();

		CurrentInUseAttachPoint->AttachPointKind == GrapplingAttachPoint
			? Player->PlayerAnimation(GrapplingAnimation, bPlayerInAir)
			: Player->PlayerAnimation(LerpAnimation, bPlayerInAir);

		// End of preperation
	}
}


ARopeHabschAttachPoint* URopeHabschRopeComponent::CheckForAttachPoints() const
{
	ARopeHabschAttachPoint* AttachPoint = nullptr;

	// Ray Casting Sphere of specific radius , Activate Attach Points
	TArray<FHitResult> Hits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Player->GetActorLocation(), Player->GetActorLocation(),
	                                                 RayCastRadius,
	                                                 ObjectTypesArray, false, TArray<AActor*>(),
	                                                 bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
	                                                 Hits, true, FLinearColor::Red, FLinearColor::Green, 1);

	float MinDotProduct = 1.0f; // Max 1

	// Prepare to send Event in case found at least one attach point

	FAttachPointStruct NewAttachPtStruct = FAttachPointStruct{nullptr, TArray<AActor*>()};

	for (auto Hit : Hits)
	{
		if (Hit.bBlockingHit)
		{
			ARopeHabschAttachPoint* TempAttachPoint = Cast<ARopeHabschAttachPoint>(Hit.GetActor());
			NewAttachPtStruct.AttachPointsInRange.Add(Hit.GetActor());
			FVector CameraForwardVector = CameraComponent->GetForwardVector();
			FVector DirectionFromPlayerToAttachPoint = (Player->GetActorLocation() - Hit.GetActor()->GetActorLocation())
				.GetSafeNormal();
			const float DotValue = FVector::DotProduct(DirectionFromPlayerToAttachPoint, CameraForwardVector);

			if ((DotValue <= MinDotProduct) && !(TempAttachPoint->AttachPointState == InBlockViewState))
			{
				MinDotProduct = DotValue;
				AttachPoint = TempAttachPoint;
			}
		}
	}

	NewAttachPtStruct.AttachPointInFocus = AttachPoint;

	// Send Event Only If 1 Valid Attach Point Found 
	if (NewAttachPtStruct.AttachPointsInRange.Num() != 0)
	{
		OnScanningForAttachPoints.Broadcast(NewAttachPtStruct);
	}


	return AttachPoint;
}


void URopeHabschRopeComponent::ChangeAnimationState(const EAnimationStates State)
{
	// If Cable Visibility On OFf, Turn Cable on Off and Return
	if (State == CableVisibilityOn)
	{
		TurnCableVisibility(true);
		const FVector StartLocation = Player->GetMesh()->GetSocketLocation("index_02_l");
		Player->Cable->SetWorldLocation(StartLocation);
		Player->EndOfCable->SetWorldLocation(StartLocation);
		FullRopeLengthToDestination = (StartLocation - CurrentInUseAttachPoint->GetActorLocation()).Size();
		bRopeAttached = true;
		return;
	}
	else if (State == CableVisibilityOff)
	{
		TurnCableVisibility(false);
		bRopeAttached = false;
		return;
	}

	if (CurrentInUseAttachPoint->AttachPointKind == LerpAttachPoint)
	{
		switch (State)
		{
		case LerpToDestination:
			{
				TurnOnOffMovement(false);
				MovementComponent->AddImpulse(PlayerToAttachPointDirection * -180000);
				bPlayerLerp = true;
				break;
			}
		case EndOfAnimation:
			{
				bPlayerLerp = false;
				Player->SetActorEnableCollision(true);
				Player->KeyBoardEnabled = true;
				bGravityChange = true;
				bRopeAttached = false;
				TurnCableVisibility(false);
				CurrentInUseAttachPoint = nullptr;
				break;
			}
		default: break;
		}
	}
	else if (CurrentInUseAttachPoint->AttachPointKind == GrapplingAttachPoint)
	{
		switch (State)
		{
		case LerpToDestination:
			{
				bPlayerLerp = true;
				break;
			}
		case EndOfAnimation:
			{
				bPlayerLerp = false;
				TurnOnOffMovement(true);
				CurrentInUseAttachPoint = nullptr;
				break;
			}
		default: break;
		}
	}
	else if (CurrentInUseAttachPoint->AttachPointKind == SwingAttachPoint)
	{
		switch (State)
		{
		case LerpToDestination:
			{
				bPlayerLerp = true;
				break;
			}
		case EndOfAnimation:
			{
				break;
			}
		default: break;
		}
	}
}

void URopeHabschRopeComponent::TurnCableVisibility(const bool Condition)
{
	Player->EndOfCable->SetVisibility(Condition);
	Player->Cable->SetVisibility(Condition);
}

void URopeHabschRopeComponent::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<ARopeHabschCharacter>(GetOwner());
	MovementComponent = Player->GetCharacterMovement();
	CameraComponent = Player->FollowCamera;
	AnimInstance = Player->GetMesh()->GetAnimInstance();
}

void URopeHabschRopeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ClosestAttachPoint = CheckForAttachPoints();

	// Only When Hooked to Attach Point we will fire the following Task
	if (CurrentInUseAttachPoint == nullptr && !bGravityChange) return;
	
	const FVector HandLocation = Player->GetMesh()->GetSocketLocation("index_02_l");
	const float AnimationCurrentTime = AnimInstance->Montage_GetPosition(AnimInstance->GetCurrentActiveMontage());
	float AlphaTime1, AlphaTime2;
	
	// Update the Rope
	if (bRopeAttached)
	{
		Player->Cable->SetWorldLocation(HandLocation);
		AlphaTime1 = HooksDAsset->CableLengthFloatCurve->GetFloatValue(AnimationCurrentTime);
		AlphaTime2 = HooksDAsset->CableEndMovementFloatCurve->GetFloatValue(AnimationCurrentTime);
		// Cable Length
		const float CableSegmentValue = FullRopeLengthToDestination * AlphaTime1;
		Player->Cable->CableLength = CableSegmentValue;
		// End Of Cable Location
		const FVector NewLocation = UKismetMathLibrary::VLerp(HandLocation, CurrentInUseAttachPoint->GetActorLocation(),AlphaTime2);
		Player->EndOfCable->SetWorldLocation(NewLocation);
	}

	
	// Lerp Variations
	if (bPlayerLerp)
	{
		if (CurrentInUseAttachPoint->AttachPointKind == GrapplingAttachPoint)
		{
			AlphaTime1 = HooksDAsset->PlayerGrapplingFloatCurve->GetFloatValue(AnimationCurrentTime);
			const FVector NewLocation = UKismetMathLibrary::VLerp(Player->GetActorLocation(), FinalDestination,AlphaTime1);
			Player->SetActorLocation(NewLocation);
		}
		else if (CurrentInUseAttachPoint->AttachPointKind == LerpAttachPoint)
		{
			FRotator Rotation = GetOwner()->GetActorRotation();
			FQuat CurrentRot = FQuat::FindBetweenVectors(-1.f * PlayerToAttachPointDirection, -1.f * FVector::UpVector);
			CurrentAngle = CurrentRot.GetAngle();
			Rotation.Pitch = CurrentAngle - 60;
			GetOwner()->SetActorRotation(Rotation);
			if ((Player->GetActorLocation() - CurrentInUseAttachPoint->GetActorLocation()).Size() < 200)
			{
				bPlayerLerp = false;
				ChangeAnimationState(EndOfAnimation);
				Player->PlayerAnimation(RollAnimation, true);
			}
		}
	}
	else if(bGravityChange)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message!"));

		AlphaTime1 = HooksDAsset->PlayerGravityGainFloatCurve->GetFloatValue(AnimationCurrentTime);
		float LerpValue = UKismetMathLibrary::Lerp(MovementComponent->GravityScale, 1, AlphaTime1);
		MovementComponent->GravityScale = LerpValue;

		FRotator TempRotator = Player->GetActorRotation();
		LerpValue = UKismetMathLibrary::Lerp(TempRotator.Pitch, 0, AlphaTime1);
		TempRotator.Pitch = LerpValue;

		Player->SetActorRotation(TempRotator);
		
		
		if (!MovementComponent->IsFalling())
		{
			TempRotator.Pitch = 0;
			Player->SetActorRotation(TempRotator);
			TurnOnOffMovement(true);
			bGravityChange = false;
		}
	}
}
