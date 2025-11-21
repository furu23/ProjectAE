# QuestSystem 분석 및 UI 연동 가이드

이 문서는 QuestSystem 플러그인의 구조를 분석하고, UMG 위젯과 연동하여 퀘스트 UI를 구현하기 위한 기술적인 가이드를 제공합니다.

---

## 1. 개요 (Overview)

퀘스트 시스템은 ULocalPlayerSubsystem을 상속받는 UQuestManagerSubSystem에 의해 관리됩니다. 이는 각 플레이어별로 퀘스트 상태가 독립적으로 추적됨을 의미하며, 블루프린트에서 비교적 쉽게 접근할 수 있는 구조입니다.

시스템은 이벤트 기반으로 설계되어, 퀘스트 상태가 변경될 때마다 델리게이트(이벤트)를 발생시켜 UI가 실시간으로 반응할 수 있게 합니다.

---

## 2. 핵심 API (Core API for UI)

UI 구현에 필요한 핵심 기능은 UQuestManagerSubSystem에 모두 BlueprintCallable 또는 BlueprintAssignable로 노출되어 있습니다.

| 종류 | 이름 | 설명 |
| --- | --- | --- |
| **함수** | GetQuestLogEntries() | 현재 플레이어의 모든 퀘스트 정보를 FQuestLogEntry 구조체 배열로 반환합니다. 퀘스트 UI를 처음 열 때 목록을 채우는 데 사용합니다. |
| **이벤트** | OnQuestEntryUpdated | 퀘스트의 상태나 목표 진행도가 변경될 때, 변경된 퀘스트의 FQuestLogEntry 데이터를 담아 호출됩니다. UI의 특정 항목을 동적으로 업데이트하는 데 사용합니다. |

---

## 3. 주요 데이터 구조체 (Key Data Structures)

UI는 아래의 구조체와 열거형을 통해 퀘스트 정보를 얻습니다. 모든 속성은 BlueprintReadOnly로 안전하게 읽을 수 있습니다.

### 3.1. FQuestLogEntry (UI용 퀘스트 데이터)

GetQuestLogEntries() 함수와 OnQuestEntryUpdated 이벤트가 사용하는 핵심 데이터 구조체입니다.

| 속성명 | 타입 | 설명 |
| --- | --- | --- |
| QuestID | FGameplayTag | 퀘스트의 고유 식별자입니다. UI에서 '보상 받기' 등 특정 퀘스트에 대한 상호작용을 요청할 때 이 ID를 시스템에 다시 전달해야 합니다. |
| Title | FText | 퀘스트의 제목입니다. 퀘스트 목록에 표시됩니다. |
| Description | FText | 퀘스트의 상세 설명입니다. 퀘스트를 클릭했을 때 표시됩니다. |
| CurrentState | EQuestProgress | 퀘스트의 현재 상태입니다. (아래 EQuestProgress 참고) |
| FormattedObjectives | TArray<FText> | **[중요]** "슬라임 5/10마리 처치" 와 같이, 현재 진행도가 포함된 가공된 목표 텍스트의 배열입니다. UI에서는 이 배열의 내용을 그대로 한 줄씩 표시하기만 하면 됩니다. |

### 3.2. EQuestProgress (퀘스트 상태 열거형)

FQuestLogEntry의 CurrentState 속성의 타입입니다. UI는 이 상태를 기준으로 퀘스트를 '진행 중' 또는 '완료' 탭으로 분류할 수 있습니다.

| 상태 | 설명 | UI 탭 분류 |
| --- | --- | --- |
| InProgress | 진행 중인 퀘스트입니다. | **진행 중 (In-Progress)** |
| Completed_PendingTurnIn | 목표를 모두 달성했으나 아직 보상을 받지 않은 상태입니다. | **진행 중 (In-Progress)** 또는 **완료 대기** |
| Complete_Final | 모든 과정이 끝나고 보상까지 받은 최종 완료 상태입니다. | **완료 (Completed)** |
| CanAccept | 수락 가능한 상태입니다. | (퀘스트 저널보다는 NPC 상호작용 UI에 사용될 가능성이 높음) |
| NotStarted | 아직 시작하지 않은 퀘스트입니다. | (UI에 표시되지 않음) |

---