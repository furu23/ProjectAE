# ⚔️ Quest System Documentation

## 1. 개요 (Overview)

**QuestSystem**은 게임플레이 로직과 퀘스트 로직 간의 결합도를 낮추기 위해 **Event-Driven(이벤트 주도)** 및 **Data-Driven(데이터 주도)** 방식을 채택했습니다.

### 핵심 철학
* **Loose Coupling (느슨한 결합):** 게임플레이(전투, 아이템 등) 코드는 퀘스트 시스템을 직접 참조하지 않습니다. 대신 `GMS(Gameplay Message Subsystem)`를 통해 상황을 전파(Broadcast)합니다.
* **Tag Based (태그 기반):** 퀘스트 ID, 목표(Objective) ID, 이벤트 채널 등 모든 식별자를 `GameplayTag`로 관리하여 유연성을 확보합니다.
* **UI DTO Support:** UI 팀은 복잡한 내부 객체(`QuestObject`)를 알 필요 없이, UI 표현을 위해 정제된 구조체 `FQuestLogEntry`만 참조하면 됩니다.

---

## 2. 연동 가이드 (Integration)

### 2.1. 모듈 설정
프로젝트의 `.Build.cs` 파일에 `QuestSystem` 모듈을 의존성에 추가해야 API를 사용할 수 있습니다.

```csharp
// Project.Build.cs
PublicDependencyModuleNames.AddRange(
    new string[]
    {
        "Core",
        "Engine",
        "GameplayTags",
        "GameplayMessageRuntime", // GMS 의존성
        "QuestSystem"             // <--- 추가
    }
);
```

---

## 3. 외부 인터페이스 (I/O Specification)

이 시스템의 사용자는 크게 **데이터를 보내는 쪽(Input)**과 **데이터를 받는 쪽(Output)**으로 나뉩니다.

### 3.1. Input: 게임플레이 이벤트 송신 (For Gameplay/AI Programmers)

AI 사망, 아이템 획득, 지역 이동 등 게임 내 사건이 발생했을 때, 퀘스트 시스템에 이를 알리는 방법입니다. `UQuestMessageHelpers` 정적 클래스를 사용합니다.

| 함수명 | 용도 | 주요 파라미터 |
| :--- | :--- | :--- |
| `BroadcastAIKilledEvent` | AI/몬스터 사망 시 | `Instigator`(킬러), `Target`(사망자) |
| `BroadcastInteractEvent` | 상호작용/채집 시 | `Instigator`(플레이어), `Target`(대상), `Tags`(아이템 태그 등) |
| `BroadcastLocationEvent` | 특정 구역 진입 시 | `Instigator`(플레이어), `Target`(트리거), `Tags`(구역 태그) |

#### 사용 예시: 몬스터 처치 (EnemyCharacter.cpp)
```cpp
#include "QuestMessageHelpers.h"

void AEnemyCharacter::Die(AActor* Killer)
{
    // ... 기존 사망 로직 ...

    // 퀘스트 시스템에 "나 죽었음" 이벤트 전송
    // TargetTags는 AI가 가진 GameplayTags 인터페이스에서 자동으로 추출됨
    UQuestMessageHelpers::BroadcastAIKilledEvent(this, Killer, this, FGameplayTagContainer());
}
```

#### 사용 예시: 아이템 획득 (ItemActor.cpp)
```cpp
#include "QuestMessageHelpers.h"

void AItemActor::OnPickedUp(AActor* Player)
{
    // 아이템 정보를 담은 태그 컨테이너 생성
    FGameplayTagContainer ItemTags;
    ItemTags.AddTag(FGameplayTag::RequestGameplayTag("Item.Quest.Letter"));

    // 상호작용 이벤트 전송
    UQuestMessageHelpers::BroadcastInteractEvent(this, Player, this, ItemTags);
}
```

---

### 3.2. Output: UI 데이터 표출 (For UI Programmers)

UI 위젯은 퀘스트 시스템의 내부 로직을 몰라도 됩니다. 오직 **`FQuestLogEntry` (DTO)**와 **서브시스템 API**만 사용하세요.

#### 데이터 구조체: `FQuestLogEntry`
| 필드명 | 타입 | 설명 |
| :--- | :--- | :--- |
| `QuestID` | `FGameplayTag` | 퀘스트 고유 ID (보상 요청 시 사용) |
| `Title` | `FText` | 퀘스트 제목 |
| `Description` | `FText` | 퀘스트 본문 설명 |
| `CurrentState` | `EQuestProgress` | 진행 상태 (진행중, 완료 대기, 완료됨) |
| `FormattedObjectives` | `TArray<FText>` | 포맷팅된 목표 텍스트 목록 (예: "좀비 처치 (3/5)") |

#### UI 갱신 흐름 (Event-Driven)
UI는 `OnQuestEntryUpdated` 델리게이트를 구독하여, 데이터가 변할 때만 화면을 갱신합니다.

```cpp
// MyQuestLogWidget.cpp

void UMyQuestLogWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 1. 서브시스템 가져오기
    auto* QuestSys = GetGameInstance()->GetSubsystem<UQuestManagerSubSystem>();
    
    // 2. 이벤트 구독 (퀘스트 정보가 갱신될 때마다 호출됨)
    if (QuestSys)
    {
        QuestSys->OnQuestEntryUpdated.AddDynamic(this, &UMyQuestLogWidget::UpdateQuestSlot);
    }
}

// 델리게이트 콜백 함수
void UMyQuestLogWidget::UpdateQuestSlot(const FQuestLogEntry& UpdatedEntry)
{
    // UpdatedEntry 안에 UI 표시에 필요한 모든 텍스트/상태값이 들어있음
    // 예: QuestList->FindRow(UpdatedEntry.QuestID)->SetData(UpdatedEntry);
}
```

#### 전체 목록 가져오기 (Polling)
초기화 시점에 현재 보유한 모든 퀘스트 목록이 필요할 때 사용합니다.
```cpp
TArray<FQuestLogEntry> AllQuests = QuestSys->GetQuestLogEntries();
```

---

## 4. World Interaction: 월드 태스크 (For Level Designers)

퀘스트 수락 시점이나 특정 단계에서 월드에 변화(아이템 스폰, 문 개방, 네비게이션 핑 등)를 주어야 할 때 **Quest Task**를 사용합니다.

### 동작 원리
1.  **데이터 설정:** 기획자가 `ObjectiveConfig`에 수행할 `Task`를 추가합니다.
2.  **이벤트 버블링:** 해당 목표가 활성화되면 `QuestManager`가 `OnQuestTaskBubbleUp` 델리게이트를 통해 Task 목록을 방송합니다.
3.  **실행:** `GameMode`나 `WorldDirector`가 이를 수신하여 실제 월드 로직을 수행합니다.

### 월드 디렉터 처리 예시
```cpp
// MyGameMode.cpp

void AMyGameMode::BeginPlay()
{
    // Task 요청 델리게이트 구독
    auto* QuestSys = GetGameInstance()->GetSubsystem<UQuestManagerSubSystem>();
    QuestSys->OnQuestTaskBubbleUp.BindUObject(this, &AMyGameMode::ExecuteQuestTasks);
}

void AMyGameMode::ExecuteQuestTasks(const TArray<TObjectPtr<UQuestTask>>& Tasks)
{
    for (UQuestTask* Task : Tasks)
    {
        // 각 Task 객체의 Execute 함수 호출 (실제 스폰/연출 로직 수행)
        Task->Execute(this);
    }
}
```

---

## 5. Data Authoring (데이터 제작 가이드)

에디터에서 `DA_QuestBase`를 상속받은 데이터 에셋을 생성하여 퀘스트를 정의합니다.

### 5.1. 기본 설정 (Quest Base)
* **Quest ID:** 퀘스트를 구분하는 고유 태그 (예: `Quest.Ep1.Intro`)
* **Prerequisite Quests:** 이 퀘스트를 시작하기 위해 완료해야 하는 선행 퀘스트 ID 목록.
* **Objectives:** 수행해야 할 목표들의 목록.

### 5.2. 목표 설정 (Objective Config)
`Objectives` 배열에 `QuestObjectiveConfig`를 상속받은 구체적인 설정(KillConfig, InteractConfig 등)을 추가합니다.

* **Objective ID:** 해당 목표의 고유 ID.
* **bNotifyWorldSystemOnActivation:** 체크 시, 이 목표가 시작될 때 `TaskOnActivation`에 등록된 태스크들을 실행합니다.
* **TaskOnActivation:** (위 옵션 체크 시) 실행할 월드 태스크 목록 (예: `Task_SpawnItem`).

---

## 6. 시스템 구조 (Architecture)

### 데이터 흐름 요약
1.  **Event (Game -> System):** 게임플레이 코드에서 `Helper` 함수 호출 -> `GMS` 메시지 방송.
2.  **Process (System):** `QuestObjective`가 메시지 수신 -> 조건 검사 -> 진행도 업데이트.
3.  **Notify (System -> UI):** 진행도 변경 시 `QuestManager`가 `OnQuestEntryUpdated` 방송 -> UI 갱신.
4.  **Task (System -> World):** 목표 활성화 시 `QuestManager`가 `OnQuestTaskBubbleUp` 방송 -> `GameMode`가 태스크 실행.

### 저장/로드 (Serialization)
* `UQuestManagerSubSystem::GetSaveData` 호출 시 `PlayerQuestHistory` (퀘스트 ID 및 상태 맵)가 바이너리로 직렬화됩니다.
* 런타임 객체(`UQuestObject`)는 저장되지 않으며, 로드 시 히스토리 데이터를 기반으로 재구성됩니다.
