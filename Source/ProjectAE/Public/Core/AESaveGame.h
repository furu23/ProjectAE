// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AESaveGame.generated.h"

/**
 * @brief 세이브게임 클래스입니다. 데이터 컨테이너 스타일의 저장 객체를 목표로 합니다.
 * @note UPROPERTY의 SaveGame 지정자를 이용해 직렬화 한 데이터를 받는 것을 전제로 합니다.
 * 
 * 만약 SaveGame 지정자를 사용하지 않았다면 추가적인 객체를 이 클래스 위에 선언해,
 * 그 객체의 형식으로 저장하고 불러와주세요.
 * 추가적인 헬퍼 함수를 만드는 것도 좋습니다.
 */
UCLASS()
class PROJECTAE_API UAESaveGame : public USaveGame
{
	GENERATED_BODY()

public:
    // 퀘스트 모듈 데이터
    UPROPERTY(VisibleAnywhere, Category = "Save")
    TArray<uint8> QuestSystemData;

    // 인벤토리 데이터
    UPROPERTY(VisibleAnywhere, Category = "Save")
    TArray<uint8> PlayerInventoryData;

    // 창고 데이터
    UPROPERTY(VisibleAnywhere, Category = "Save")
    TArray<uint8> WarehouseInventoryData;
};
