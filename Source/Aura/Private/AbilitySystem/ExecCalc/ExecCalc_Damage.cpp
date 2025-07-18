// Copyright Jakob Memborg


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"

struct AuraDamageStatics
{
	/* Target */
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalResistance);
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistanceFire);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistanceLightning);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistanceArcane);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ResistancePhysical);

	/* Source*/
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);

	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	AuraDamageStatics()
	{
		/* Target */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalResistance, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistanceFire, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistanceLightning, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistanceArcane, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ResistancePhysical, Target, false);

		/* Source */
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalDamage, Source, false);

		/* Tags to Attributes mapping */
		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalResistance, CriticalResistanceDef);
		
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, ResistanceFireDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, ResistanceLightningDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, ResistanceArcaneDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, ResistancePhysicalDef);

		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalChance, CriticalChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalDamage, CriticalDamageDef);
	}
};

static const AuraDamageStatics DamageStatics()
{
	static AuraDamageStatics Statics;

	return Statics;
};

UExecCalc_Damage::UExecCalc_Damage()
{
	/* Target */
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalResistanceDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().ResistanceFireDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResistanceLightningDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResistanceArcaneDef);
	RelevantAttributesToCapture.Add(DamageStatics().ResistancePhysicalDef);

	/* Source */
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalDamageDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceAsc = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetAsc = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvater = SourceAsc ? SourceAsc->GetAvatarActor() : nullptr;
	AActor* TargetAvater = TargetAsc ? TargetAsc->GetAvatarActor() : nullptr;
	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvater);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvater);

	const FGameplayEffectSpec Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = SourceTags;
	EvaluationParams.TargetTags = TargetTags;

	/* Get Damage Set by Caller Magnitude */
	float Damage = 0.f;

	for (const TPair<FGameplayTag, FGameplayTag>& DamageTypeTag : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageType = DamageTypeTag.Key;
		const FGameplayTag ResistanceType = DamageTypeTag.Value;

		checkf(AuraDamageStatics().TagsToCaptureDefs.Contains(ResistanceType), TEXT("TagsToCaptureDefs does not contain Tag: [%s] in ExecCalc_Damage"), *ResistanceType.ToString());
		const FGameplayEffectAttributeCaptureDefinition& CaptureDef = AuraDamageStatics().TagsToCaptureDefs.FindRef(ResistanceType);

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag.Key);
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParams, Resistance);
		Resistance = FMath::Clamp<float>(Resistance, 0.f, 100.f);

		DamageTypeValue *= (100.f - Resistance) * 0.01f;
		
		Damage += DamageTypeValue;
	}

	/*Capture blockchance on target*/
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParams,
	                                                           TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	const bool bBlocked = FMath::RandRange(0.f, 100.f) < TargetBlockChance;

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

	/* If Block, halve the damage */
	Damage = bBlocked ? Damage * 0.5f : Damage;

	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParams, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParams,
	                                                           SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);

	UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvater);

	/* ArmorPenetration, ignores a percentage of the target armor */
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(
		FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) * 0.01f;

	/* Armor ignores a percentage of incoming Damage */
	const float EffectiveArmorCoefficient = CharacterClassInfo->DamageCalculationCoefficients->
	                                                   FindCurve(FName("EffectiveArmor"), FString())->Eval(
		                                                   TargetCombatInterface->GetPlayerLevel());
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) * 0.01f;

	/* Critical Hit */
	float SourceCriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalChanceDef, EvaluationParams, SourceCriticalChance);
	SourceCriticalChance = FMath::Max<float>(SourceCriticalChance, 0.f);

	float TargetCriticalResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalResistanceDef, EvaluationParams, TargetCriticalResistance);
	TargetCriticalResistance = FMath::Max<float>(TargetCriticalResistance, 0.f);

	const float CriticalResistCoefficient = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalResistance"), FString())->Eval(TargetCombatInterface->GetPlayerLevel());
	const float EffectiveCriticalHitChance = SourceCriticalChance - TargetCriticalResistance * CriticalResistCoefficient;
	
	/* If Critical Hit, Calculate Crit Damage */
	if (const bool bCriticalHit = FMath::RandRange(0.f, 100.f) < EffectiveCriticalHitChance)
	{
		UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);
		float SourceCriticalDamage = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalDamageDef, EvaluationParams, SourceCriticalDamage);
		SourceCriticalDamage = FMath::Max<float>(SourceCriticalDamage, 0.f);

		Damage = Damage * 2.f + SourceCriticalDamage;
	}

	/* Set new damage */
	FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(),
	                                             EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
