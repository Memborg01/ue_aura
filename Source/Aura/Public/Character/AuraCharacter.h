// Copyright Jakob Memborg

#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "AuraCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
// ReSharper disable once CppUE4CodingStandardUClassNamingViolationError
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAuraCharacter();

	/** REMOVE? */
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/** end REMOVE? */

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Combat Interface */
	virtual int32 GetPlayerLevel() override;
	virtual void OnDeath() override;
	/** end Combat Interface */

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	virtual void InitAbilityActorInfo() override;
};
