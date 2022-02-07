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

	TArray<FHitResult> Hits;
	UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), GetOwner()->GetActorLocation(), GetOwner()->GetActorLocation(), RayCastRadius,
													 ObjectTypesArray, false, TArray<AActor*>(),
													 bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
													 Hits, true, FLinearColor::Red, FLinearColor::Green, 1);
	float MinDotProduct = 0.7f;

	FAttachPointStruct NewAttachPtStruct = FAttachPointStruct{nullptr,TArray<AActor*>()};

	for (auto Hit : Hits)
	{
		if (Hit.bBlockingHit)
		{

				NewAttachPtStruct.AttachPointsInRange.Add(Hit.GetActor());
				ARopeHabschAttachPoint * TempAttachPoint  = Cast<ARopeHabschAttachPoint>(Hit.GetActor());
				FVector CameraForwardVector = CameraComponent->GetForwardVector();
				FVector DirectionFromPlayerToAttachPoint = (GetOwner()->GetActorLocation() - Hit.GetActor()->GetActorLocation()).GetSafeNormal();
				const float DotValue = FVector::DotProduct(CameraForwardVector,DirectionFromPlayerToAttachPoint);
			
				if(DotValue < MinDotProduct)
				{
					MinDotProduct = DotValue;
					AttachPoint = TempAttachPoint;
				}
			
		}
	}
	if(AttachPoint!=nullptr)
	{
		NewAttachPtStruct.AttachPointInFocus = AttachPoint;
	}

	OnScanningForAttachPoints.Broadcast(NewAttachPtStruct);
	
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