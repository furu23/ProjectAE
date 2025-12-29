// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "WeatherApiHelper.generated.h"

// 날씨 정보 데이터를 저장할 구조체
USTRUCT(BlueprintType)
struct WEATHER_API FWeatherCategoryData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Weather")
    FString Category; // PTY, SKY 등 카테고리 코드

    UPROPERTY(BlueprintReadWrite, Category = "Weather")
    FString Value;    // 해당 카테고리의 값 (예: "0", "1", "3", "28.9" 등)

    UPROPERTY(BlueprintReadWrite, Category = "Weather")
    FString FcstTime; // 예보 시간 (UltraSrtFcst 전용)
};

// 모든 날씨 카테고리 데이터를 담을 컨테이너
USTRUCT(BlueprintType)
struct WEATHER_API FCurrentWeatherResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Weather")
    TArray<FWeatherCategoryData> WeatherDataItems;
};

// 날씨 정보 API 응답 콜백 델리게이트
// PTY/SKY 값을 직접 전달하도록 수정 (블루프린트에서 쉽게 사용)
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnWeatherInfoReceivedPTY, int32, PTYValue, bool, bIsSuccess, FString, ErrorMessage, const FCurrentWeatherResult&, RawResult);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnWeatherInfoReceivedSKY, int32, SKYValue, bool, bIsSuccess, FString, ErrorMessage, const FCurrentWeatherResult&, RawResult);

/**
 * 
 */
UCLASS()
class WEATHER_API UWeatherApiHelper : public UObject
{
	GENERATED_BODY()
public:
    // PTY 호출 관련 멤버
    FOnWeatherInfoReceivedPTY PTYOnCompleted;
    FLatentActionInfo PTYLatentInfo;

    // SKY 호출 관련 멤버
    FOnWeatherInfoReceivedSKY SKYOnCompleted;
    FLatentActionInfo SKYLatentInfo;

    FString ApiKey;
    int32 CurrentNx;
    int32 CurrentNy;
    FString ApiType; // "PTY" 또는 "SKY"

    // HTTP 요청 완료 시 호출될 콜백 함수
    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);


    // JSON 데이터 파싱 함수
    FCurrentWeatherResult ParseWeatherResponse(const FString& JsonString);
    FString GetCurrentBaseTime();
    FString GetCurrentBaseDate();
	
};
