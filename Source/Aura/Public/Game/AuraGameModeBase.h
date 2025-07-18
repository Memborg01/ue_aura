// Copyright Jakob Memborg

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class UCharacterClassInfo;
/**
 * 
 */
UCLASS()
// ReSharper disable once CppUE4CodingStandardUClassNamingViolationError
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Charcter Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
};
