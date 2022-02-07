#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RopeHabschAttachPoint.generated.h"


UENUM(BlueprintType)
enum EAttachPointKind
{
	SwingAttachPoint UMETA(DisplayName = "Swing Attach Point"),
	GrapplingAttachPoint UMETA(DisplayName = "Grappling Attach Point"),
	LerpAttachPoint UMETA(DisplayName = "Lerp Attach Point"),
};

UENUM(BlueprintType)
enum EAttachPointState
{
	OutRangeState UMETA(DisplayName = "Out Range"),
	InRangeState UMETA(DisplayName = "In Range"),
	InFocusState UMETA(DisplayName = "In Focus"),
	InUseState UMETA(DisplayName = "In Use"),
};




UCLASS()
class ARopeHabschAttachPoint : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scene Component", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scene Component", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY()
	class URopeHabschRopeComponent* PlayerRopeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	float DistancedFromPlayerClamped = 0.f;

	void ChangeAttachPointState(  struct FAttachPointStruct AttachPointStruct);
	
	UPROPERTY()
	AActor* Player;

	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsDebug = false;

	UPROPERTY()
	TArray<AActor*>IgnoreActors;
public:
	
	ARopeHabschAttachPoint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EAttachPointKind> AttachPointKind;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float RopeLength = 1000.f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Settings")
	float AttachPointDistance = 300.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Settings")
	TEnumAsByte<EAttachPointState> AttachPointState;

	void PlayerDistanceToImageScale() ;
	void UseAttachPoint();
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnimationOnUse();
	
protected:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};
