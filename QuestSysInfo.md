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

### **상세 UMG 구현 가이드 (Step-by-Step)**

이 가이드는 WBP_Quest_Main (메인 퀘스트 창)과 WBP_QuestList_Entry (목록의 개별 항목) 두 개의 위젯을 사용하여 퀘스트 UI를 만드는 과정을 안내합니다.

#### **1단계: 위젯 블루프린트 준비**

1.  **WBP_QuestList_Entry 생성**:
    *   Content Browser에서 Widget Blueprint를 새로 만들고 이름을 WBP_QuestList_Entry로 지정합니다.
    *   디자이너 탭에서 Canvas Panel 위에 Button을 추가하고, 그 위에 TextBlock을 올립니다. 이 TextBlock은 퀘스트 제목을 표시할 것입니다. 변수 이름은 QuestTitleText로 지정합니다.

2.  **WBP_Quest_Main 생성**:
    *   Widget Blueprint를 새로 만들고 이름을 WBP_Quest_Main으로 지정합니다.
    *   **디자이너 탭 구성**:
        *   Border: 배경 이미지나 색상을 위해 추가합니다.
        *   Button 2개: '진행 중', '완료' 탭 역할을 할 버튼들을 추가합니다.
        *   Widget Switcher: 탭 전환 시 보여줄 위젯을 관리합니다. 여기에 ScrollBox 두 개를 넣습니다.
        *   ScrollBox 2개: Widget Switcher 안에 각각 넣고, 이름을 ScrollBox_InProgress와 ScrollBox_Completed로 지정합니다. 이 두 스크롤박스는 각각 진행 중/완료된 퀘스트 목록을 담게 됩니다.
        *   TextBlock 여러 개: 선택된 퀘스트의 제목(SelectedQuestTitle), 설명(SelectedQuestDescription)을 표시할 텍스트 블록들을 오른쪽에 배치합니다.
        *   ScrollBox 1개: 선택된 퀘스트의 목표 목록(FormattedObjectives)을 담을 ScrollBox_Objectives를 배치합니다.

#### **2단계: WBP_QuestList_Entry 로직 구현**

이 위젯은 퀘스트 데이터 하나를 받아 제목을 표시하고, 클릭 시 자신을 알려주는 역할을 합니다.

1.  **변수 생성**:
    *   Event Graph로 가서 QuestData라는 이름의 변수를 만듭니다. 타입은 FQuestLogEntry로 지정합니다.
    *   변수 디테일 패널에서 **Instance Editable**과 **Expose on Spawn** 체크박스를 모두 활성화합니다. 이렇게 하면 이 위젯이 생성될 때 퀘스트 데이터를 외부에서 넣어줄 수 있습니다.

2.  **이벤트 디스패처 생성**:
    *   My Blueprint 패널 하단의 Event Dispatchers에서 + 버튼을 눌러 OnQuestSelected라는 이름의 디스패처를 추가합니다.
    *   디스패처의 디테일 패널에서 Inputs에 + 버튼을 눌러 FQuestLogEntry 타입의 QuestData 파라미터를 추가합니다.

3.  **초기화 및 클릭 이벤트**:
    *   Event Construct에서 QuestData 변수로부터 Title을 가져와 QuestTitleText의 텍스트를 설정합니다. (Set Text 노드 사용)
    *   디자이너 탭에서 추가했던 Button의 OnClicked 이벤트를 추가합니다.
    *   OnClicked 이벤트에서 Call OnQuestSelected 노드를 호출하고, 이 위젯의 QuestData 변수를 연결해줍니다.

#### **3단계: WBP_Quest_Main 로직 구현 - 목록 채우기**

1.  **서브시스템 참조 가져오기**:
    *   Event Construct에서 Get Owning Local Player -> Get Subsystem 노드를 연결합니다.
    *   Get Subsystem 노드의 Subsystem Class 드롭다운에서 QuestManagerSubSystem을 선택합니다.
    *   반환된 값을 마우스 오른쪽 버튼으로 클릭하여 Promote to Variable을 선택하고, 변수 이름을 QuestManager로 지정합니다.

2.  **퀘스트 목록 새로고침 함수 (RefreshQuestList)**:
    *   My Blueprint 패널에서 + Function을 눌러 RefreshQuestList 함수를 만듭니다.
    *   함수 시작 시, ScrollBox_InProgress와 ScrollBox_Completed를 각각 Clear Children 노드로 비워줍니다.
    *   QuestManager 변수를 가져와 GetQuestLogEntries 함수를 호출합니다.
    *   결과로 나온 배열에 For Each Loop를 연결합니다.
    *   루프의 Array Element에서 Break FQuestLogEntry 노드를 사용하거나, 핀을 직접 끌어 CurrentState에 접근합니다.
    *   Switch on EQuestProgress 노드를 CurrentState에 연결합니다.
    *   **InProgress, Completed_PendingTurnIn 핀에서**:
        *   Create Widget 노드를 사용하여 WBP_QuestList_Entry를 생성합니다. QuestData 핀에 루프의 Array Element를 연결합니다.
        *   생성된 위젯을 ScrollBox_InProgress에 Add Child 합니다.
        *   **[중요]** 생성된 위젯의 Bind Event to OnQuestSelected 노드를 호출합니다. 이 이벤트에 연결될 커스텀 이벤트를 만들어 다음 4단계에서 구현할 상세 정보 표시 로직을 연결합니다.
    *   **Complete_Final 핀에서**: 위와 동일하게 진행하되, ScrollBox_Completed에 Add Child 합니다.

3.  **Event Construct에서 함수 호출**:
    *   Event Construct의 서브시스템 참조를 얻는 로직 뒤에, 위에서 만든 RefreshQuestList 함수를 호출합니다.

#### **4단계: WBP_Quest_Main 로직 구현 - 상세 정보 표시**

1.  **상세 정보 표시 함수 생성**:
    *   DisplayQuestDetails 라는 이름의 새 함수를 만들고, FQuestLogEntry 타입의 QuestData를 입력 파라미터로 받도록 설정합니다.
    *   3단계에서 Bind Event to OnQuestSelected에 이 함수를 연결하거나, 이 함수를 호출하는 커스텀 이벤트를 연결합니다.

2.  **함수 내용 구현**:
    *   입력받은 QuestData를 사용하여 SelectedQuestTitle, SelectedQuestDescription 텍스트 블록의 내용을 채웁니다.
    *   ScrollBox_Objectives를 Clear Children으로 비웁니다.
    *   QuestData의 FormattedObjectives 배열에 For Each Loop를 실행합니다.
    *   루프마다 Create Widget으로 간단한 텍스트 블록 위젯(WBP_Objective_Entry 등)을 만들고, 배열의 텍스트를 설정한 뒤 ScrollBox_Objectives에 Add Child 합니다.

#### **5단계: WBP_Quest_Main 로직 구현 - 실시간 업데이트**

1.  **델리게이트 바인딩**:
    *   Event Construct에서 QuestManager 변수를 가져와 Bind Event to OnQuestEntryUpdated 노드를 호출합니다.
    *   Event 핀에 Create Event를 선택하고, HandleQuestUpdate라는 이름의 새 커스텀 이벤트를 만듭니다.

2.  **업데이트 처리**:
    *   HandleQuestUpdate 이벤트는 FQuestLogEntry 데이터를 파라미터로 받습니다.
    *   가장 간단한 방법은 이 이벤트가 호출될 때마다 **RefreshQuestList 함수를 다시 호출**하는 것입니다. 목록 전체가 새로고침되지만, 구현이 간단하고 직관적입니다.
    *   (고급) 성능 최적화가 필요하다면, 전달받은 QuestID와 일치하는 WBP_QuestList_Entry 위젯을 찾아 그 위젯만 직접 업데이트할 수도 있습니다.
