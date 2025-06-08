// Copyright Jakob Memborg

#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "AuraEnemy.generated.h"

UCLASS()
// ReSharper disable once CppUE4CodingStandardUClassNamingViolationError
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AAuraEnemy();

	/** Enemy Interface */
#pragma region Enemy Interface
	virtual void HighlightActor() override;
	virtual void UnhighlightActor() override;
#pragma endregion
	/** end Enemy Interface */

	/** Combat Interface */
#pragma region Combat Interface
	virtual int32 GetPlayerLevel() override;
#pragma endregion 
	/** end Combat Interface */

	UPROPERTY(BlueprintReadOnly)
	bool bIsHighlighted = false;

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Default")
	int32 Level = 1;
};
