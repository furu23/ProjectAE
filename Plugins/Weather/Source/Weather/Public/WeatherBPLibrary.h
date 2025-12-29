// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

#include "WeatherApiHelper.h" // 새로 생성한 헬퍼 클래스 헤더 추가!
#include "WeatherBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/





UCLASS()
class WEATHER_API UWeatherBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
    /**
     * @brief 현재 강수 형태 (PTY)를 가져옵니다.
     * @param WorldContextObject 월드 컨텍스트 오브젝트 (Blueprint Function Library에서 HTTP 요청을 위해 필요)
     * @param APIKey 공공데이터포털에서 발급받은 API 인증키
     * @param Nx 예보 지점 X 좌표 (격자 좌표)
     * @param Ny 예보 지점 Y 좌표 (격자 좌표)
     * @param OnCompleted API 호출 완료 시 호출될 델리게이트
     */
public:
    UFUNCTION(BlueprintCallable, Category = "Weather", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", DisplayName = "Get Current Precipitation Type (PTY)"))
    static void GetPTY(UObject* WorldContextObject, const FString APIKey, int32 Nx, int32 Ny, const FOnWeatherInfoReceivedPTY& OnCompleted, FLatentActionInfo LatentInfo);

    /**
     * @brief 현재 하늘 상태 (SKY)를 가져옵니다.
     * @param WorldContextObject 월드 컨텍스트 오브젝트 (Blueprint Function Library에서 HTTP 요청을 위해 필요)
     * @param APIKey 공공데이터포털에서 발급받은 API 인증키
     * @param Nx 예보 지점 X 좌표 (격자 좌표)
     * @param Ny 예보 지점 Y 좌표 (격자 좌표)
     * @param OnCompleted API 호출 완료 시 호출될 델리게이트
     */
    UFUNCTION(BlueprintCallable, Category = "Weather", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", DisplayName = "Get Current Sky Condition (SKY)"))
    static void GetSKY(UObject* WorldContextObject, const FString APIKey, int32 Nx, int32 Ny, const FOnWeatherInfoReceivedSKY& OnCompleted, FLatentActionInfo LatentInfo);

private:
   
    // 공통 API 요청 함수
    static void SendWeatherRequest(UObject* WorldContextObject, const FString APIKey, int32 Nx, int32 Ny, FString ApiName, FOnWeatherInfoReceivedPTY PTYDelegate, FOnWeatherInfoReceivedSKY SKYDelegate, FLatentActionInfo LatentInfo);
};
