#include "RopeHabschAttachPoint.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "RopeMechanics/RopeHabschRopeComponent.h"

ARopeHabschAttachPoint::ARopeHabschAttachPoint()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Component"));
	RootComponent = SceneComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh Component"));
	StaticMeshComponent->SetupAttachment(RootComponent);
}

void ARopeHabschAttachPoint::PlayerDistanceToImageScale() 
{
	// Clamping Result from 0 To 100 . For Easy Use 
	const float DistanceFromPlayer = (Player->GetActorLocation() - GetActorLocation() ).Size();
	DistancedFromPlayerClamped = UKismetMathLibrary::MapRangeClamped(DistanceFromPlayer,AttachPointDistance, PlayerRopeComponent->RayCastRadius, 100, 0);
}

void ARopeHabschAttachPoint::UseAttachPoint()
{
	if (AttachPointState!=InUseState)
	{
		AttachPointState = InUseState;
		PlayAnimationOnUse();
	}
	
}

void ARopeHabschAttachPoint::ChangeAttachPointState( FAttachPointStruct AttachPointStruct)
{
	if(AttachPointState == InUseState)
	{
		return;	
	}

	FHitResult HitSingleLine;
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetActorLocation(),
										  Player->GetActorLocation(),
										  UEngineTypes::ConvertToTraceType
										  (ECollisionChannel::ECC_Visibility), false, IgnoreActors,
										  bIsDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
										  HitSingleLine, true, FLinearColor::Red,
										  FLinearColor::Green, 0.02);

	if(HitSingleLine.bBlockingHit )
	{
		AttachPointState = OutRangeState;
		return;
	}


	
	if(this == AttachPointStruct.AttachPointInFocus)
	{
		if(AttachPointState!=InFocusState)
		{
			AttachPointState = InFocusState;
			SetActorTickEnabled(true);
		}
		return;
	}

	if(AttachPointStruct.AttachPointsInRange.Contains(this) && AttachPointState!=InRangeState)
	{
		AttachPointState = InRangeState;
		SetActorTickEnabled(true);
		return;
	}

	if(AttachPointState!=OutRangeState)
	{
		SetActorTickEnabled(false);
		AttachPointState = OutRangeState;
	}
	
	SetActorTickEnabled(false);
}

void ARopeHabschAttachPoint::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);
	Player = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
	IgnoreActors.Add(this);
	IgnoreActors.Add(Player);
	PlayerRopeComponent = Player->FindComponentByClass<URopeHabschRopeComponent>();
	PlayerRopeComponent->OnScanningForAttachPoints.AddUObject(this, &ARopeHabschAttachPoint::ChangeAttachPointState); //see above in wiki

}

void ARopeHabschAttachPoint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PlayerDistanceToImageScale();
}

