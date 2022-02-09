#include "RopeHabschRopeComponent.h"
#include "RopeHabschAttachPoint.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
URopeHabschRopeComponent::URopeHabschRopeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URopeHabschRopeComponent::StartHook()
{
	if(ClosestAttach!=nullptr)
	{
		ClosestAttach->UseAttachPoint();
	}
}

void URopeHabschRopeComponent::StopHook()
{
//	if(ClosestAttach!=nullptr)
//	{
//	}
//	ClosestAttach = nullptr;
}



ARopeHabschAttachPoint* URopeHabschRopeComponent::CheckForAttachPoints() const
{

	ARopeHabschAttachPoint * AttachPoint = nullptr;

	// Ray Casting Sphere of specific radius , Activate Attach Points
	TArray<FHitResult> Hits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation(), RayCastRadius,
													 ObjectTypesArray, false, TArray<AActor*>(),
													 bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
													 Hits, true, FLinearColor::Red, FLinearColor::Green, 1);
	
	float MinDotProduct = 1.0f; // Max 1

	// Prepare to send Event in case found at least one attach point

	FAttachPointStruct NewAttachPtStruct = FAttachPointStruct{nullptr,TArray<AActor*>()};

	for (auto Hit : Hits)
	{
		if (Hit.bBlockingHit)
		{
			UE_LOG(LogTemp,Warning,TEXT("Hit %s"), *Hit.GetActor()->GetName());

				ARopeHabschAttachPoint * TempAttachPoint  = Cast<ARopeHabschAttachPoint>(Hit.GetActor());
				NewAttachPtStruct.AttachPointsInRange.Add(Hit.GetActor());
				FVector CameraForwardVector = CameraComponent->GetForwardVector();
				FVector DirectionFromPlayerToAttachPoint = (GetOwner()->GetActorLocation() - Hit.GetActor()->GetActorLocation()).GetSafeNormal();
				const float DotValue = FVector::DotProduct(DirectionFromPlayerToAttachPoint, CameraForwardVector);
								
				if( (DotValue <= MinDotProduct ) && !(TempAttachPoint->AttachPointState == InBlockViewState))
				{
					MinDotProduct = DotValue;
					AttachPoint = TempAttachPoint;
				}
			
		}
	}

	NewAttachPtStruct.AttachPointInFocus = AttachPoint;

		// Send Event Only If 1 Valid Attach Point Found 
		if (NewAttachPtStruct.AttachPointsInRange.Num() != 0) {
		OnScanningForAttachPoints.Broadcast(NewAttachPtStruct);
	}
	
	
	return  AttachPoint;
}

void URopeHabschRopeComponent::BeginPlay()
{
	Super::BeginPlay();
	MovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	CameraComponent = GetOwner()->FindComponentByClass<UCameraComponent>();
}

void URopeHabschRopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ClosestAttach = CheckForAttachPoints();
}