#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RopeHabschRopeComponent.generated.h"


USTRUCT(BlueprintType)
struct FAttachPointStruct
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	AActor* AttachPointInFocus;
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> AttachPointsInRange;
};

UENUM(BlueprintType)
enum EAnimationStates
{
	CableVisibilityOn UMETA(DisplayName = "Cable Visibility On"),
	LerpToDestination UMETA(DisplayName = "Lerp To Destination"),
	CableVisibilityOff UMETA(DisplayName = "Cable Visibility Off"),
	EndOfAnimation UMETA(DisplayName = "Enf OF Animation"),
	GainGravity UMETA(DisplayName = "Gain Gravity"),
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROPEHABSCH_API URopeHabschRopeComponent : public UActorComponent
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnScanningForAttachPoints, FAttachPointStruct AttachPointStruct);

	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray; // object types to trace
	UPROPERTY()
	class UCharacterMovementComponent* MovementComponent;
	UPROPERTY()
	class UCameraComponent* CameraComponent;
	UPROPERTY()
	class ARopeHabschCharacter* Player;
	/* Allow Player to hook when he is above the grappling point */
	/* Toggle Debug On/Off */
	

	UPROPERTY(EditAnywhere, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsDebug = false;
	
	UPROPERTY()
	class ARopeHabschAttachPoint* ClosestAttachPoint ;
	UPROPERTY()
	ARopeHabschAttachPoint* CurrentInUseAttachPoint;
	EAnimationStates AnimationState;
	UPROPERTY()
	UAnimInstance* AnimInstance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	class UHooksDataAsset* HooksDAsset;
	bool bRopeAttached = false, bPlayerLerp = false, bAddingForceOnPlayer = false, bGravityChange = false, bSwinging = false,bSwingAnim = true;
	float CurrentAngle, TimeAccumulation = 0.f, FullRopeLengthToDestination;
	FVector PlayerToAttachPointDirection,SwingDirection, FinalDestination,SwingVelocity,CurrentAPLocation;
	void TurnOffMovement() const;
	FRotator GetPlayerRotationTo(FVector Location) const;
	
public:
	URopeHabschRopeComponent();

	FOnScanningForAttachPoints OnScanningForAttachPoints;
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings")
	float RayCastRadius = 2000;
	void StartHook();
	ARopeHabschAttachPoint* CheckForAttachPoints() const;

	UFUNCTION(BlueprintCallable)
	void ChangeAnimationState(EAnimationStates State);
	void TurnCableVisibility(bool Condition) const;

	UFUNCTION(BlueprintPure)
	bool IsSwinging() const {return bSwinging;}

	bool SwingKeyHold = false;
	
	void StopSwinging();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
