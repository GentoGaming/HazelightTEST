#include "RopeHabschRopeComponent.h"
#include "CableComponent.h"
#include "DrawDebugHelpers.h"
#include "RopeHabschAttachPoint.h"
#include "RopeHabschCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "DataAssets/HooksDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
URopeHabschRopeComponent::URopeHabschRopeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void URopeHabschRopeComponent::StartHook()
{
	if (ClosestAttachPoint != nullptr && CurrentInUseAttachPoint == nullptr)
	{
		const bool bCanUSe = ClosestAttachPoint->UseAttachPoint();
		if (!bCanUSe) return;
		CurrentInUseAttachPoint = ClosestAttachPoint;
		Player->SetActorRotation(GetPlayerRotationTo(CurrentInUseAttachPoint->ArrowComponent->GetComponentLocation()));
		const bool bPlayerInAir = MovementComponent->IsFalling();
		TurnOnOffMovement(false);
		PlayerToAttachPointDirection = (Player->GetActorLocation() - CurrentInUseAttachPoint->GetActorLocation()).
			GetSafeNormal();
		FinalDestination = CurrentInUseAttachPoint->ArrowComponent->GetComponentLocation();
		CurrentInUseAttachPoint->AttachPointKind == GrapplingAttachPoint
			? Player->PlayerAnimation(GrapplingAnimation, bPlayerInAir)
			: Player->PlayerAnimation(LerpAnimation, bPlayerInAir);
		if (CurrentInUseAttachPoint->AttachPointKind == SwingAttachPoint) SwingKeyHold = true;
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
				CurrentInUseAttachPoint->ReleasePoint();
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
				CurrentInUseAttachPoint->ReleasePoint();
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
				TurnOnOffMovement(false);
				MovementComponent->AddImpulse(PlayerToAttachPointDirection * -180000);
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


void URopeHabschRopeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ClosestAttachPoint = CheckForAttachPoints();

	// Only When Hooked to Attach Point we will fire the following Task
	if (CurrentInUseAttachPoint == nullptr && !bGravityChange) return;

	const FVector HandLocation = Player->GetMesh()->GetSocketLocation("index_02_l");
	const float AnimationCurrentTime = AnimInstance->Montage_GetPosition(AnimInstance->GetCurrentActiveMontage());
	float AlphaTime1;

	// Update the Rope
	if (bRopeAttached)
	{
		Player->Cable->SetWorldLocation(HandLocation);
		AlphaTime1 = HooksDAsset->CableLengthFloatCurve->GetFloatValue(AnimationCurrentTime);
		const float AlphaTime2 = HooksDAsset->CableEndMovementFloatCurve->GetFloatValue(AnimationCurrentTime);
		// Cable Length
		const float CableSegmentValue = FullRopeLengthToDestination * AlphaTime1;
		Player->Cable->CableLength = CableSegmentValue;
		// End Of Cable Location
		const FVector NewLocation = UKismetMathLibrary::VLerp(HandLocation, CurrentInUseAttachPoint->GetActorLocation(),
		                                                      AlphaTime2);
		Player->EndOfCable->SetWorldLocation(NewLocation);
	}


	// Lerp Variations
	if (bPlayerLerp)
	{
		if (CurrentInUseAttachPoint->AttachPointKind == GrapplingAttachPoint)
		{
			AlphaTime1 = HooksDAsset->PlayerGrapplingFloatCurve->GetFloatValue(AnimationCurrentTime);
			const FVector NewLocation = UKismetMathLibrary::VLerp(Player->GetActorLocation(), FinalDestination,
			                                                      AlphaTime1);
			Player->SetActorLocation(NewLocation);
		}
		else if (CurrentInUseAttachPoint->AttachPointKind == LerpAttachPoint)
		{
			FRotator Rotation = GetOwner()->GetActorRotation();
			const FQuat CurrentRot = FQuat::FindBetweenVectors(-1.f * PlayerToAttachPointDirection,
			                                                   -1.f * FVector::UpVector);
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
		else if (CurrentInUseAttachPoint->AttachPointKind == SwingAttachPoint)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, TEXT("Some debug message!"));

			FRotator Rotation = GetOwner()->GetActorRotation();
			FQuat CurrentRot = FQuat::FindBetweenVectors(-1.f * PlayerToAttachPointDirection,
			                                                   -1.f * FVector::UpVector);
			CurrentAngle = CurrentRot.GetAngle();
			Rotation.Pitch = CurrentAngle - 60;
			GetOwner()->SetActorRotation(Rotation);
			if ((Player->GetActorLocation() - CurrentInUseAttachPoint->GetActorLocation()).Size() <=
				CurrentInUseAttachPoint->RopeLength)
			{
				bPlayerLerp = false;
				TurnOnOffMovement(false);
				
				FVector SwingLocation = CurrentInUseAttachPoint->GetActorLocation();
				FVector CharacterLocation = Player->GetActorLocation();

				float RopeLength = FMath::Min(FVector::Dist(SwingLocation, CharacterLocation) - 100.f, CurrentInUseAttachPoint->RopeLength);
				SwingDirection = (SwingLocation - CharacterLocation).GetSafeNormal();

				CurrentRot = FQuat::FindBetweenVectors(-1.f * SwingDirection, -1.f * FVector::UpVector);
				CurrentAngle = CurrentRot.GetAngle();

				GetOwner()->SetActorRotation(UKismetMathLibrary::MakeRotFromX(SwingDirection));
				
				if (SwingKeyHold) bSwinging = true;
			}
		}
	}
	else if (bSwinging)
	{
		float SwingSpeed = 90.f;
		float MaxAngle = 90.f;

		CurrentAngle -= FMath::DegreesToRadians(SwingSpeed) * DeltaTime;

		// Turn around if we've reached out maximum angle
		if (CurrentAngle <= -1.f * FMath::DegreesToRadians(MaxAngle))
		{
			CurrentAngle *= -1.f;
			SwingDirection *= -1.f;
			GetOwner()->SetActorRotation(UKismetMathLibrary::MakeRotFromX(SwingDirection));
		}

		// Update the position based on the angle
		FVector SwingLocation = CurrentInUseAttachPoint->ArrowComponent->GetComponentLocation();

		FVector SwingRightVector = FVector::CrossProduct(SwingDirection, -1.f * FVector::UpVector);
		FQuat SwingRotation = FQuat(SwingRightVector.GetSafeNormal(), CurrentAngle);

		FVector SwingOffset = FVector(0.f, 0.f, -1.f * CurrentInUseAttachPoint->RopeLength);
		SwingOffset = SwingRotation.RotateVector(SwingOffset);

		FVector NewLocation = SwingLocation + SwingOffset;
		if (DeltaTime > 0.f)
			SwingVelocity = (NewLocation - GetOwner()->GetActorLocation()) / DeltaTime;
		GetOwner()->SetActorLocation(NewLocation);

		//DrawDebugLine(GetWorld(), SwingLocation, NewLocation, FColor::Red, false, 0.f, SDPG_Foreground, 5.f);

		
		Player->EndOfCable->SetWorldLocation(SwingLocation);
		Player->Cable->SetWorldLocation(Player->GetMesh()->GetSocketLocation("index_02_l"));
	}
	else if (bGravityChange)
	{
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


void URopeHabschRopeComponent::StopSwinging()
{
	if(bSwinging)
	{
		//TurnOnOffMovement(true);
		MovementComponent->AddImpulse(SwingVelocity, true);
		Player->PlayerAnimation(SwingAnimation, true);
	}
	SwingKeyHold = false;
	bSwinging = false;
	bGravityChange = true;
	Player->SetActorEnableCollision(true);
	ChangeAnimationState(CableVisibilityOff);
}

void URopeHabschRopeComponent::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<ARopeHabschCharacter>(GetOwner());
	MovementComponent = Player->GetCharacterMovement();
	CameraComponent = Player->FollowCamera;
	AnimInstance = Player->GetMesh()->GetAnimInstance();
}

// Fast Helpers Functions
void URopeHabschRopeComponent::TurnCableVisibility(const bool Condition) const
{
	Player->EndOfCable->SetVisibility(Condition);
	Player->Cable->SetVisibility(Condition);
}

FRotator URopeHabschRopeComponent::GetPlayerRotationTo(FVector Location) const
{
	FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(Player->GetActorLocation(), Location);
	Rotator.Pitch = 0;
	Rotator.Roll = 0;
	return Rotator;
}

void URopeHabschRopeComponent::TurnOnOffMovement(bool Condition) const
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
