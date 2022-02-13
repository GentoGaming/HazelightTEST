#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RopeHabschAttachPoint.generated.h"



UENUM(BlueprintType)
enum EAttachPointState
{
	OutRangeState UMETA(DisplayName = "Out Range"),
	InRangeState UMETA(DisplayName = "In Range"),
	InFocusState UMETA(DisplayName = "In Focus"),
	InUseState UMETA(DisplayName = "In Use"),
	InBlockViewState UMETA(DisplayName = "In Block View")
};

UENUM(BlueprintType)
enum EAttachPointKind
{
	SwingAttachPoint UMETA(DisplayName = "Swing Attach Point"),
	GrapplingAttachPoint UMETA(DisplayName = "Grappling Attach Point"),
	LerpAttachPoint UMETA(DisplayName = "Lerp Attach Point"),
};

UCLASS()
class ARopeHabschAttachPoint : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scene Component", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scene Component", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* StaticMeshComponent;
	

	UPROPERTY()
	class URopeHabschRopeComponent* PlayerRopeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visible Data", meta = (AllowPrivateAccess = "true"))
	float DistancedFromPlayerClamped = 0.f;
	
	UPROPERTY()
	class ARopeHabschCharacter* Player;

	UPROPERTY(EditDefaultsOnly, Category="General Settings", meta = (AllowPrivateAccess = "true"))
	bool bIsDebug = false;

	UPROPERTY()
	TArray<AActor*>IgnoreActors;

	FVector Destination;
	/*Decide Which State the Attach Point is In .*/
	void SetAttachPointCloseToPlayer(struct FAttachPointStruct AttachPointStruct);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
public:
	
	ARopeHabschAttachPoint();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene Component", meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* ArrowComponent;
	
	/*Distance of Attach point that allows it to be focused*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "General Settings")
	float AttachPointDistance = 1200.f;

	/*Disable Attach Point at this distance from the player*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "General Settings")
	float DisableAtMinDistanceOf = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	float RopeLength = 1000.f;
	
	/*
	Out Range | Attach Point Out Of Range, Update is Disabled 
	In Range  | Attach Point In Range , in Range Enabled 
	In Focus" | Attach Point Focused , Player can only hook towards it .
	In Use    | Attach Point In Use , Player Still Using it .
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visible Data")
	TEnumAsByte<EAttachPointState> AttachPointState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attach Point Type")
	TEnumAsByte<EAttachPointKind> AttachPointKind;


	/*Player To Attach Point Ratio Clamped to change UI Image Scale and help enable , Disable Update*/
	void PlayerDistanceToImageScale() ;
	
	/*Hook This Attach point*/
	bool UseAttachPoint();

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeAttachPointState(EAttachPointState State);
	void ReleasePoint();
};
