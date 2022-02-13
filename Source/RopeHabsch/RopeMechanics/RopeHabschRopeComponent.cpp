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
		
		TurnOffMovement();
		CurrentInUseAttachPoint = ClosestAttachPoint;
		CurrentAPLocation = CurrentInUseAttachPoint->ArrowComponent->GetComponentLocation();
		Player->SetActorRotation(GetPlayerRotationTo(CurrentAPLocation));
		const bool bPlayerInAir = MovementComponent->IsFalling();
		PlayerToAttachPointDirection = (Player->GetActorLocation() - CurrentAPLocation).
			GetSafeNormal();
		FinalDestination = CurrentAPLocation;
		CurrentInUseAttachPoint->AttachPointKind == GrapplingAttachPoint
			? Player->PlayerAnimation(GrapplingAnimation, bPlayerInAir)
			: Player->PlayerAnimation(LerpAnimation, bPlayerInAir);
		if (CurrentInUseAttachPoint->AttachPointKind == SwingAttachPoint)
		{
			SwingKeyHold = true;
			bSwingAnim = true;
		}
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
		FullRopeLengthToDestination = (StartLocation - CurrentAPLocation).Size();
		bRopeAttached = true;
		return;
	}
	else if (State == CableVisibilityOff)
	{
		TurnCableVisibility(false);
		bRopeAttached = false;
		return;
	}

	if (CurrentInUseAttachPoint->AttachPointKind == SwingAttachPoint)
	{
		if(State == LerpToDestination)bPlayerLerp = true;
		return;
	}
	
	if (CurrentInUseAttachPoint->AttachPointKind == LerpAttachPoint)
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
				Player->SetActorEnableCollision(true);
				Player->KeyBoardEnabled = true;
				Player->PlayerAnimation(RollAnimation, true);
				MovementComponent->AddImpulse(SwingVelocity * CurrentInUseAttachPoint->VelocityMultiplier, true);
				bPlayerLerp = false;
				bRopeAttached = false;
				bGravityChange = true;
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
				MovementComponent->GravityScale = 1;
				Player->SetActorEnableCollision(true);
				Player->KeyBoardEnabled = true;
				CurrentInUseAttachPoint->ReleasePoint();
				CurrentInUseAttachPoint = nullptr;
				bPlayerLerp = false;
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
		float CableSegmentValue = FullRopeLengthToDestination * AlphaTime1;
		Player->Cable->CableLength = CableSegmentValue;

		// End Of Cable Location
		const FVector NewLocation = UKismetMathLibrary::VLerp(HandLocation, CurrentInUseAttachPoint->GetActorLocation(),
		AlphaTime2);
		Player->EndOfCable->SetWorldLocation(NewLocation);
	}


	// Lerp Variations
	if (bPlayerLerp)
	{
		AlphaTime1 = HooksDAsset->PlayerGrapplingFloatCurve->GetFloatValue(AnimationCurrentTime);

		const FVector NewLocation = UKismetMathLibrary::VLerp(Player->GetActorLocation(), FinalDestination,
		                                                      0.05);
		SwingVelocity = (NewLocation - Player->GetActorLocation()) / DeltaTime;
		Player->SetActorLocation(NewLocation);
		bool StopCondition = false;


		
		// If Not a grappling hook , Rotate Player forward to direction with bit of angle down
		if (CurrentInUseAttachPoint->AttachPointKind != GrapplingAttachPoint)
		{
			FRotator Rotation = GetOwner()->GetActorRotation();
			const FQuat CurrentRot = FQuat::FindBetweenVectors(-1.f * PlayerToAttachPointDirection,
			                                                   -1.f * FVector::UpVector);
			CurrentAngle = CurrentRot.GetAngle();
			Rotation.Pitch = CurrentAngle - 60;
			GetOwner()->SetActorRotation(Rotation);
			if ((CurrentInUseAttachPoint->AttachPointKind == LerpAttachPoint && (CurrentAPLocation
					- Player->GetActorLocation()).Size() <= 200)
				||
				CurrentInUseAttachPoint->AttachPointKind == SwingAttachPoint && (CurrentAPLocation
					- Player->GetActorLocation()).Size() <= CurrentInUseAttachPoint->RopeLength)
			{
				StopCondition = true;
			}
		}


		if (StopCondition)
		{
			switch (CurrentInUseAttachPoint->AttachPointKind)
			{
			case LerpAttachPoint:
				{
					ChangeAnimationState(EndOfAnimation);
					break;
				}
			case SwingAttachPoint:
				{
					bPlayerLerp = false;
					float RopeLength = FMath::Min(FVector::Dist(CurrentAPLocation, Player->GetActorLocation()) - 100.f, CurrentInUseAttachPoint->RopeLength);
					SwingDirection = (CurrentAPLocation - Player->GetActorLocation()).GetSafeNormal();
					const FQuat CurrentRot = FQuat::FindBetweenVectors(-1.f * SwingDirection, -1.f * FVector::UpVector);
					CurrentAngle = CurrentRot.GetAngle();
					Player->SetActorRotation(UKismetMathLibrary::MakeRotFromX(SwingDirection));
					bSwinging = true;
					Player->Cable->CableLength = (Player->GetActorLocation() - CurrentAPLocation).Size();
					break;
				}
			default: ;
			}
		}
	}
	else if (bSwinging)
	{
		CurrentAngle -= FMath::DegreesToRadians(CurrentInUseAttachPoint->SwingSpeed) * DeltaTime;

		// Turn around if we've reached out maximum angle
		if (CurrentAngle <= -1.f * FMath::DegreesToRadians(CurrentInUseAttachPoint->MaxAngle))
		{
			CurrentAngle *= -1.f;
			SwingDirection *= -1.f;
			GetOwner()->SetActorRotation(UKismetMathLibrary::MakeRotFromX(SwingDirection));
		}

		// Update the position based on the angle
		const FVector SwingLocation = CurrentAPLocation;

		const FVector SwingRightVector = FVector::CrossProduct(SwingDirection, -1.f * FVector::UpVector);
		const FQuat SwingRotation = FQuat(SwingRightVector.GetSafeNormal(), CurrentAngle);

		FVector SwingOffset = FVector(0.f, 0.f, -1.f * CurrentInUseAttachPoint->RopeLength);
		SwingOffset = SwingRotation.RotateVector(SwingOffset);

		const FVector NewLocation = SwingLocation + SwingOffset;
		if (DeltaTime > 0.f)
			SwingVelocity = (NewLocation - GetOwner()->GetActorLocation()) / DeltaTime;
		Player->SetActorLocation(NewLocation);

		Player->EndOfCable->SetWorldLocation(SwingLocation);
		Player->Cable->SetWorldLocation(Player->GetMesh()->GetSocketLocation("index_02_l"));
		
		if(!SwingKeyHold) {
			SwingVelocity = FVector::ZeroVector;
			bSwingAnim = false;
			StopSwinging();
		}
		
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
			MovementComponent->GravityScale = 1;
			TempRotator.Pitch = 0;
			Player->SetActorRotation(TempRotator);
			bGravityChange = false;
		}
	}
}


void URopeHabschRopeComponent::StopSwinging()
{
	if (bSwinging)
	{
		MovementComponent->AddImpulse(SwingVelocity * CurrentInUseAttachPoint->VelocityMultiplier, true);

		if(CurrentInUseAttachPoint!=nullptr)
	{
		CurrentInUseAttachPoint->ReleasePoint();
		CurrentInUseAttachPoint = nullptr;
	}
		if(bSwingAnim)Player->PlayerAnimation(SwingAnimation, true);
		Player->SetActorEnableCollision(true);
		ChangeAnimationState(CableVisibilityOff);
		Player->KeyBoardEnabled = true;
		bSwinging = false;
		bGravityChange = true;
	}
	SwingKeyHold = false;
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

void URopeHabschRopeComponent::TurnOffMovement() const
{

		MovementComponent->GravityScale = 0;
		MovementComponent->StopMovementImmediately();
		//MovementComponent->SetMovementMode(MOVE_Falling);
		Player->SetActorEnableCollision(false);
		Player->KeyBoardEnabled = false;
}
