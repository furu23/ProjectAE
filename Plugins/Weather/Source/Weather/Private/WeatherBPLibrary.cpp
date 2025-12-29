// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeatherBPLibrary.h"
// 기존 include 유지
#include "GenericPlatform/GenericPlatformHttp.h"

UWeatherBPLibrary::UWeatherBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}


// ----- UMyWeatherFunctionLibrary 구현 -----

// 공통 API 요청 함수 (static)
void UWeatherBPLibrary::SendWeatherRequest(UObject* WorldContextObject, const FString APIKey, int32 Nx, int32 Ny, FString ApiName, FOnWeatherInfoReceivedPTY PTYDelegate, FOnWeatherInfoReceivedSKY SKYDelegate, FLatentActionInfo LatentInfo)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("Weather API: WorldContextObject is null."));
        if (ApiName == TEXT("PTY") && PTYDelegate.IsBound()) PTYDelegate.ExecuteIfBound(-1, false, TEXT("WorldContextObject is null"), FCurrentWeatherResult());
        if (ApiName == TEXT("SKY") && SKYDelegate.IsBound()) SKYDelegate.ExecuteIfBound(-1, false, TEXT("WorldContextObject is null"), FCurrentWeatherResult());
        return;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Weather API: Could not get World from WorldContextObject."));
        if (ApiName == TEXT("PTY") && PTYDelegate.IsBound()) PTYDelegate.ExecuteIfBound(-1, false, TEXT("Could not get World"), FCurrentWeatherResult());
        if (ApiName == TEXT("SKY") && SKYDelegate.IsBound()) SKYDelegate.ExecuteIfBound(-1, false, TEXT("Could not get World"), FCurrentWeatherResult());
        return;
    }

    if (APIKey.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Weather API: API Key is not set!"));
        if (ApiName == TEXT("PTY") && PTYDelegate.IsBound()) PTYDelegate.ExecuteIfBound(-1, false, TEXT("API Key is empty"), FCurrentWeatherResult());
        if (ApiName == TEXT("SKY") && SKYDelegate.IsBound()) SKYDelegate.ExecuteIfBound(-1, false, TEXT("API Key is empty"), FCurrentWeatherResult());
        return;
    }

    // 헬퍼 객체 생성 (UMyWeatherFunctionLibrary::UWeatherApiHelper -> UWeatherApiHelper로 변경)
    UWeatherApiHelper* Helper = NewObject<UWeatherApiHelper>(World); // 이제 전역 클래스이므로 직접 생성
    Helper->ApiKey = APIKey;
    Helper->CurrentNx = Nx;
    Helper->CurrentNy = Ny;
    Helper->ApiType = ApiName;

    if (ApiName == TEXT("PTY"))
    {
        Helper->PTYOnCompleted = PTYDelegate;
        Helper->PTYLatentInfo = LatentInfo;
    }
    else if (ApiName == TEXT("SKY"))
    {
        Helper->SKYOnCompleted = SKYDelegate;
        Helper->SKYLatentInfo = LatentInfo;
    }

    FString BaseDate = Helper->GetCurrentBaseDate();
    FString BaseTime = Helper->GetCurrentBaseTime();

    FString ApiUrl;
    if (ApiName == TEXT("PTY"))
    {
        ApiUrl = TEXT("http://apis.data.go.kr/1360000/VilageFcstInfoService_2.0/getUltraSrtNcst");
    }
    else if (ApiName == TEXT("SKY"))
    {
        ApiUrl = TEXT("http://apis.data.go.kr/1360000/VilageFcstInfoService_2.0/getUltraSrtFcst");
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Weather API: Invalid ApiName provided: %s"), *ApiName);
        if (ApiName == TEXT("PTY") && PTYDelegate.IsBound()) PTYDelegate.ExecuteIfBound(-1, false, TEXT("Invalid ApiName"), FCurrentWeatherResult());
        if (ApiName == TEXT("SKY") && SKYDelegate.IsBound()) SKYDelegate.ExecuteIfBound(-1, false, TEXT("Invalid ApiName"), FCurrentWeatherResult());
        return;
    }

    FString EncodedServiceKey = APIKey; // FGenericPlatformHttp::UrlEncode(APIKey);

    FString FullUrl = FString::Printf(TEXT("%s?serviceKey=%s&pageNo=1&numOfRows=100&dataType=JSON&base_date=%s&base_time=%s&nx=%d&ny=%d"),
        *ApiUrl, *EncodedServiceKey, *BaseDate, *BaseTime, Helper->CurrentNx, Helper->CurrentNy);

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("GET");
    Request->SetURL(FullUrl);
    Request->OnProcessRequestComplete().BindUObject(Helper, &UWeatherApiHelper::OnResponseReceived); // 헬퍼 클래스 변경
    Request->ProcessRequest();

    UE_LOG(LogTemp, Log, TEXT("Weather API Requesting %s: %s"), *ApiName, *FullUrl);
}

void UWeatherBPLibrary::GetPTY(UObject* WorldContextObject, const FString APIKey, int32 Nx, int32 Ny, const FOnWeatherInfoReceivedPTY& OnCompleted, FLatentActionInfo LatentInfo)
{
    SendWeatherRequest(WorldContextObject, APIKey, Nx, Ny, TEXT("PTY"), OnCompleted, FOnWeatherInfoReceivedSKY(), LatentInfo);
}

void UWeatherBPLibrary::GetSKY(UObject* WorldContextObject, const FString APIKey, int32 Nx, int32 Ny, const FOnWeatherInfoReceivedSKY& OnCompleted, FLatentActionInfo LatentInfo)
{
    SendWeatherRequest(WorldContextObject, APIKey, Nx, Ny, TEXT("SKY"), FOnWeatherInfoReceivedPTY(), OnCompleted, LatentInfo);
}