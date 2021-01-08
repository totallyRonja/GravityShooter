// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FirstUE4ProjectHUD.generated.h"

UCLASS()
class AFirstUE4ProjectHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFirstUE4ProjectHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

