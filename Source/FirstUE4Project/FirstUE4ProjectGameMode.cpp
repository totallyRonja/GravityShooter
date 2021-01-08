// Copyright Epic Games, Inc. All Rights Reserved.

#include "FirstUE4ProjectGameMode.h"
#include "FirstUE4ProjectHUD.h"
#include "FirstUE4ProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFirstUE4ProjectGameMode::AFirstUE4ProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFirstUE4ProjectHUD::StaticClass();
}
