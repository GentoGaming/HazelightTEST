#include "RopeHabschAttachPoint.h"

#include "RopeHabschCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "RopeMechanics/RopeHabschRopeComponent.h"

ARopeHabschAttachPoint::ARopeHabschAttachPoint()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	RootComponent = SceneComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh Component"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow Component"));
	ArrowComponent->SetupAttachment(RootComponent);

}

void ARopeHabschAttachPoint::PlayerDistanceToImageScale() 
{
	// Clamping Result from 0 To 100 . For Easy Use 
	const float DistanceFromPlayer = (Player->GetActorLocation() - GetActorLocation() ).Size();
	DistancedFromPlayerClamped = UKismetMathLibrary::MapRangeClamped(DistanceFromPlayer,AttachPointDistance, PlayerRopeComponent->RayCastRadius, 100, 0);
}

bool ARopeHabschAttachPoint::UseAttachPoint()
{

	if (AttachPointState!=InUseState && AttachPointState == InFocusState && DistancedFromPlayerClamped == 100)
	{
		SetActorTickEnabled(false);
		AttachPointState = InUseState;
		ChangeAttachPointState(InUseState);
		return true;
	}
	return false;
}

void ARopeHabschAttachPoint::ReleasePoint()
{
	AttachPointState = InRangeState;
	ChangeAttachPointState(AttachPointState);
}


void ARopeHabschAttachPoint::SetAttachPointCloseToPlayer( FAttachPointStruct AttachPointStruct)
{

	// This Attach Point in Use , Return
	if (AttachPointState == InUseState) return;

	SetActorTickEnabled(false);
	
	// Too Close Disable Attach Point
	if ((GetActorLocation() - Player->GetActorLocation()).Size() < DisableAtMinDistanceOf) {
		if(AttachPointState != InBlockViewState )
		{
			AttachPointState = InBlockViewState;
			ChangeAttachPointState(InBlockViewState);
			return;
		}
		return;
	}
	
	if (AttachPointStruct.AttachPointInFocus != this && !AttachPointStruct.AttachPointsInRange.Contains(this))
	{
		if(AttachPointState != OutRangeState)
		{
			AttachPointState = OutRangeState;
			ChangeAttachPointState(OutRangeState);
			return;
		}

		return;
	}


	if (AttachPointStruct.AttachPointInFocus == this)
	{

		FHitResult HitSingleLine;
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetActorLocation(),
			Player->GetActorLocation(),
			UEngineTypes::ConvertToTraceType
			(ECollisionChannel::ECC_Visibility), false, IgnoreActors,
			bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
			HitSingleLine, true, FLinearColor::Red,
			FLinearColor::Green, 0.02);


		if (!HitSingleLine.bBlockingHit)
		{
			SetActorTickEnabled(true);
			if (AttachPointState != InFocusState)
			{
				AttachPointState = InFocusState;
				ChangeAttachPointState(InFocusState);
				return;

			}
			return;

		}
		else
		{
			if (AttachPointState == InFocusState)
			{
				AttachPointState = OutRangeState;
				ChangeAttachPointState(OutRangeState);
				return;
			}
		}

	}


	if(AttachPointStruct.AttachPointInFocus != this && AttachPointStruct.AttachPointsInRange.Contains(this))
	{
		if(AttachPointState != InRangeState)
		{
			AttachPointState = InRangeState;
			ChangeAttachPointState(InRangeState);
			return;
		}
		return;
	}

	
	if(AttachPointState != OutRangeState)
	{
		ChangeAttachPointState(OutRangeState);
	}

}

void ARopeHabschAttachPoint::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true);
	Player = Cast<ARopeHabschCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
	IgnoreActors.Add(this);
	IgnoreActors.Add(Player);
	PlayerRopeComponent = Player->RopeComponent;
	PlayerRopeComponent->OnScanningForAttachPoints.AddUObject(this, &ARopeHabschAttachPoint::SetAttachPointCloseToPlayer); //see above in wiki
	Destination = GetActorLocation() + FVector(0,0,Player->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
}

void ARopeHabschAttachPoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PlayerDistanceToImageScale();
}

