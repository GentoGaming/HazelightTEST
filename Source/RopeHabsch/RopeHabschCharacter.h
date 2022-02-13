// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RopeHabschCharacter.generated.h"

UENUM(BlueprintType)
enum EHookAnimation
{
	GrapplingAnimation UMETA(DisplayName = "Grappling Anim"),
	SwingAnimation UMETA(DisplayName = "Swing Anim"),
	LerpAnimation UMETA(DisplayName = "Lerp Anim"),
	RollAnimation UMETA(DisplayName = "Roll Anim")
};

UCLASS(config=Game)
class ARopeHabschCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;



public:
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swing Component", meta = (AllowPrivateAccess = "true"))
	class URopeHabschRopeComponent *RopeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Swing Component", meta = (AllowPrivateAccess = "true"))
	class UCableComponent *Cable;
	
	UPROPERTY(EditDefaultsOnly, Category = "Swing Component", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class URopeHabschRopeComponent> CableComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing Component", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* EndOfCable;
	
	UFUNCTION(BlueprintImplementableEvent, Category = MainMenu)
	void PlayerAnimation( const EHookAnimation Anim, const bool bInAir);
	
	ARopeHabschCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	
	bool KeyBoardEnabled = true;
	
protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom sub Object **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera sub Object **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};

