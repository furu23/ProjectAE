// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AEDamageExecution.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AS_HealthSet.h"

// 1. 캡처 정의: 값을 바꿀 대상인 Health만 캡처합니다.
struct FMyDamageStatics
{
    // 공격력/방어력 어트리뷰트가 없으니 캡처할 필요 없음.
    // 타겟의 Health만 캡처 (값을 깎아야 하니까)
    DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

    FMyDamageStatics()
    {
        // Snapshot: false (데미지 적용 시점의 체력을 대상으로 함)
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAS_HealthSet, Health, Target, false);
    }
};

static const FMyDamageStatics& DamageStatics()
{
    static FMyDamageStatics DStatics;
    return DStatics;
}

UAEDamageExecution::UAEDamageExecution()
{
    // 캡처할 속성 등록
    RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
}

void UAEDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    // -------------------------------------------------------------------------
    // 1. 기본 데미지 가져오기 (SetByCaller)
    // -------------------------------------------------------------------------
    // BP에서 SetByCallerMagnitude로 넘겨준 그 태그 이름을 정확히 써야 합니다.
    // 예: "Data.BaseDamage"
    const FGameplayTag DamageDataTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));

    // Spec에서 값을 꺼냅니다. 기본값은 0.0f
    float BaseDamage = Spec.GetSetByCallerMagnitude(DamageDataTag, false, 0.0f);

    // -------------------------------------------------------------------------
    // 2. 태그 기반 보정 (과열 체크)
    // -------------------------------------------------------------------------

    // 태그 준비
    const FGameplayTag OverheatTag = FGameplayTag::RequestGameplayTag(FName("State.Condition.Overheating"));

    // 태그 수집
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    // 변수 초기화
    float DamageMultiplier = 1.0f; // 공격력 배율
    float DamageReduction = 0.0f;  // 피해 감소율

    // A. 공격자(Source) 과열 체크 -> 공격력 증가
    if (EvaluationParameters.SourceTags && EvaluationParameters.SourceTags->HasTag(OverheatTag))
    {
         DamageMultiplier = 1.5f; // 50% 증가 (원하는 수치로 변경)
    }

    // B. 피격자(Target) 과열 체크 -> 피해 감소 (방어력 역할)
    if (EvaluationParameters.TargetTags && EvaluationParameters.TargetTags->HasTag(OverheatTag))
    {
        DamageReduction = 0.3f; // 30% 감소 (원하는 수치로 변경)
    }

    // -------------------------------------------------------------------------
    // 3. 최종 계산
    // -------------------------------------------------------------------------
    float FinalDamage = BaseDamage * DamageMultiplier * (1.0f - DamageReduction);

    if (FinalDamage < 0.f)
    {
        FinalDamage = 0.f;
    }

    // -------------------------------------------------------------------------
    // 4. 결과 적용 (Health 깎기)
    // -------------------------------------------------------------------------
    // Enemy가 Meta Attribute(IncomingDamage)를 안 쓴다면, Health에 직접 음수를 더합니다.
    if (FinalDamage > 0.f)
    {
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(
                DamageStatics().HealthProperty,
                EGameplayModOp::Additive,  // 더하기 연산
                -FinalDamage               // 음수 값을 더해서 체력 감소
            )
        );
    }
}