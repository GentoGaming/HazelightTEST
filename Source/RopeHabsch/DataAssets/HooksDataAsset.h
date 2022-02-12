// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HooksDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ROPEHABSCH_API UHooksDataAsset : public UDataAsset
{
	
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* CableLengthFloatCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* CableEndMovementFloatCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* PlayerGrapplingFloatCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* PlayerLandingFloatCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* PlayerForceMultiplierFloatCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* PlayerGravityGainFloatCurve;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* PlayerLerpFloatCurve;
	
};
