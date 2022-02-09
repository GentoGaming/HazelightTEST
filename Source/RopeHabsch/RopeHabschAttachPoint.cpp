#include "RopeHabschAttachPoint.h"

#include "Components/ArrowComponent.h"
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

void ARopeHabschAttachPoint::UseAttachPoint()
{

	if (AttachPointState!=InUseState && AttachPointState == InFocusState && DistancedFromPlayerClamped == 100)
	{
		SetActorTickEnabled(false);
		AttachPointState = InUseState;
		ChangeAttachPointState(InUseState);

	}
	
}

void ARopeHabschAttachPoint::ChangeAttachPointState( FAttachPointStruct AttachPointStruct)
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
	Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
	IgnoreActors.Add(this);
	IgnoreActors.Add(Player);
	PlayerRopeComponent = Player->FindComponentByClass<URopeHabschRopeComponent>();
	PlayerRopeComponent->OnScanningForAttachPoints.AddUObject(this, &ARopeHabschAttachPoint::ChangeAttachPointState); //see above in wiki
	UE_LOG(LogTemp,Warning,TEXT("Hello %s"), *GetClass()->GetName());
}

void ARopeHabschAttachPoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PlayerDistanceToImageScale();
}

