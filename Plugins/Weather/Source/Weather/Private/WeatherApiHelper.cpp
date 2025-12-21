// Fill out your copyright notice in the Description page of Project Settings.


#include "WeatherApiHelper.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h" // GetWorld() 대신 WorldContextObject 사용
#include "Engine/GameInstance.h" // For GetWorld() on some contexts
#include "LatentActions.h" // Latent actions for Blueprint Callable Latent functions


// ----- UWeatherApiHelper 구현 (이전 UMyWeatherFunctionLibrary::UWeatherApiHelper 내부 구현 코드와 동일) -----

// Helper: 현재 날짜 가져오기 (YYYYMMDD)
FString UWeatherApiHelper::GetCurrentBaseDate()
{
    FDateTime Now = FDateTime::Now();
    return Now.ToString(TEXT("%Y%m%d"));
}

// Helper: 현재 시간에 가장 가까운 base_time 가져오기 (매시 30분 발표)
FString UWeatherApiHelper::GetCurrentBaseTime()
{
    FDateTime Now = FDateTime::Now();
    int32 Hour = Now.GetHour();
    int32 Minute = Now.GetMinute();

    if (Minute < 30)
    {
        Hour = (Hour - 1 + 24) % 24;
        Minute = 30;
    }
    else
    {
        Minute = 30;
    }

    return FString::Printf(TEXT("%02d%02d"), Hour, Minute);
}
FCurrentWeatherResult UWeatherApiHelper::ParseWeatherResponse(const FString& JsonString)
{
    FCurrentWeatherResult Result;

    TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonString);
    TSharedPtr<FJsonObject> JsonObject;

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        TSharedPtr<FJsonObject> ResponseObject = JsonObject->GetObjectField(TEXT("response"));
        if (ResponseObject.IsValid())
        {
            TSharedPtr<FJsonObject> BodyObject = ResponseObject->GetObjectField(TEXT("body"));
            if (BodyObject.IsValid())
            {
                TSharedPtr<FJsonObject> ItemsObject = BodyObject->GetObjectField(TEXT("items"));
                if (ItemsObject.IsValid())
                {
                    TArray<TSharedPtr<FJsonValue>> ItemArray = ItemsObject->GetArrayField(TEXT("item"));
                    for (TSharedPtr<FJsonValue> ItemValue : ItemArray)
                    {
                        TSharedPtr<FJsonObject> ItemObject = ItemValue->AsObject();
                        if (ItemObject.IsValid())
                        {
                            FWeatherCategoryData Data;
                            Data.Category = ItemObject->GetStringField(TEXT("category"));

                            if (ItemObject->HasField(TEXT("obsrValue")))
                            {
                                Data.Value = ItemObject->GetStringField(TEXT("obsrValue"));
                            }
                            else if (ItemObject->HasField(TEXT("fcstValue")))
                            {
                                Data.Value = ItemObject->GetStringField(TEXT("fcstValue"));
                                Data.FcstTime = ItemObject->GetStringField(TEXT("fcstTime"));
                            }

                            Result.WeatherDataItems.Add(Data);
                        }
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Weather API: 'items' field not found in response body."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Weather API: 'body' field not found in response."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Weather API: 'response' field not found."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON response: %s"), *JsonString);
    }
    return Result;
}


// HTTP 요청 완료 시 호출될 콜백 함수
void UWeatherApiHelper::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    int32 ResultValue = -1;
    bool bSuccess = false;
    FString ErrorMsg = TEXT("Unknown Error");
    FCurrentWeatherResult ParsedResult;

    if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
    {
        FString ResponseString = Response->GetContentAsString();
        ParsedResult = ParseWeatherResponse(ResponseString);

        if (ApiType == TEXT("PTY"))
        {
            for (const FWeatherCategoryData& Data : ParsedResult.WeatherDataItems)
            {
                if (Data.Category == TEXT("PTY"))
                {
                    ResultValue = FCString::Atoi(*Data.Value);
                    bSuccess = true;
                    ErrorMsg = TEXT("Success");
                    UE_LOG(LogTemp, Log, TEXT("PTY Value received: %d"), ResultValue);
                    break;
                }
            }
            if (!bSuccess)
            {
                ErrorMsg = TEXT("PTY category not found in response.");
                UE_LOG(LogTemp, Warning, TEXT("Weather API: %s"), *ErrorMsg);
            }
        }
        else if (ApiType == TEXT("SKY"))
        {
            FString TargetFcstTime = FString::Printf(TEXT("%02d00"), (FDateTime::Now().GetHour() + 1) % 24);
            if (FDateTime::Now().GetMinute() >= 30)
            {
                TargetFcstTime = FString::Printf(TEXT("%02d00"), (FDateTime::Now().GetHour() + 1) % 24);
            }
            else {
                TargetFcstTime = FString::Printf(TEXT("%02d00"), FDateTime::Now().GetHour() % 24);
            }


            // int32 FoundSKYValue = -1;
            for (const FWeatherCategoryData& Data : ParsedResult.WeatherDataItems)
            {
                if (Data.Category == TEXT("SKY"))
                {
                    if (Data.FcstTime == TargetFcstTime)
                    {
                        ResultValue = FCString::Atoi(*Data.Value);
                        bSuccess = true;
                        ErrorMsg = TEXT("Success");
                        UE_LOG(LogTemp, Log, TEXT("SKY Value received (for %s): %d"), *TargetFcstTime, ResultValue);
                        break;
                    }
                }
            }
            if (!bSuccess)
            {
                ErrorMsg = FString::Printf(TEXT("SKY category for fcstTime %s not found in response."), *TargetFcstTime);
                UE_LOG(LogTemp, Warning, TEXT("Weather API: %s"), *ErrorMsg);
            }
        }
    }
    else
    {
        ErrorMsg = FString::Printf(TEXT("HTTP Request failed or invalid response. Code: %d"), Response.IsValid() ? Response->GetResponseCode() : -1);
        UE_LOG(LogTemp, Error, TEXT("Weather API: %s"), *ErrorMsg);
    }

    if (ApiType == TEXT("PTY"))
    {
        if (PTYOnCompleted.IsBound())
        {
            PTYOnCompleted.ExecuteIfBound(ResultValue, bSuccess, ErrorMsg, ParsedResult);
        }
    }
    else if (ApiType == TEXT("SKY"))
    {
        if (SKYOnCompleted.IsBound())
        {
            SKYOnCompleted.ExecuteIfBound(ResultValue, bSuccess, ErrorMsg, ParsedResult);
        }
    }

    this->MarkAsGarbage();
}