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
	TArray<AActor*>AttachPointsInRange;
};



UCLASS(BlueprintType, Meta=(BlueprintSpawnableComponent))
class ROPEHABSCH_API URopeHabschRopeComponent : public UActorComponent
{
	
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnScanningForAttachPoints, FAttachPointStruct AttachPointStruct);

	GENERATED_BODY()
	/*  Will Raycast to get the attach points available
	 *  array of filters the ray cast hit for scalability. "Attach Points was added to object channel"
	 */
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray; // object types to trace

	UPROPERTY()
	class UCharacterMovementComponent * MovementComponent;
	UPROPERTY()
	class UCameraComponent * CameraComponent;

	/* Allow Player to hook when he is above the grappling point */
	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsHookingDown = false;
	/* Toggle Debug On/Off */
	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsDebug = false;

	UPROPERTY()
	class ARopeHabschAttachPoint* ClosestAttach;
	
	void DetectAvailablePoint();
	
public:	

	URopeHabschRopeComponent();

	FOnScanningForAttachPoints OnScanningForAttachPoints;

	/*Ray cast every __ Second, 0.x for Milli Seconds, no need to cast it every frame ( OnTick )*/
	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings")
	float RayCastTimeRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="RayCast Settings")
	float RayCastRadius = 2000;
	
	void StartHook();
	void StopHook();
	class ARopeHabschAttachPoint* CheckForAttachPoints() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
