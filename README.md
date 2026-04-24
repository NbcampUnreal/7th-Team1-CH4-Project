# EternalDreams

<p align="center">
  <img alt="Engine" src="https://img.shields.io/badge/Unreal_Engine-5.5-0E1128?logo=unrealengine&logoColor=white" />
  <img alt="Platform" src="https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white" />
  <img alt="Mode" src="https://img.shields.io/badge/Mode-Multiplayer-2ea44f" />
  <img alt="Gameplay Ability System" src="https://img.shields.io/badge/System-GAS-8250df" />
  <img alt="UI" src="https://img.shields.io/badge/UI-CommonUI%20%2B%20UMG-1f6feb" />
</p>

Unreal Engine 5.5 기반 멀티플레이 액션 프로젝트입니다.  
이 문서는 `Source` 기준으로 프로젝트의 핵심 구조와 시스템 연결을 빠르게 파악할 수 있도록 정리한 **프로젝트 메인 README**입니다.

---

## 📌 프로젝트 요약

- **프로젝트 성격**: 로비 진입 -> 매치 진행(페이즈) -> 전투/파밍/크래프팅 -> 승패 판정
- **기본 시작 맵**: `L_Lobby` (`Config/DefaultEngine.ini`)
- **핵심 기술 축**: Multiplayer, GAS, CommonUI/UMG, AI Behavior Tree, Data-Driven Asset
- **메인 런타임 모듈**: `Source/EternalDreams`
- **에디터 보조 모듈**: `Source/GitStatusBranch`

## 🧱 모듈 구성

### `EternalDreams` (Runtime)

- 위치: `Source/EternalDreams`
- 빌드 설정: `Source/EternalDreams/EternalDreams.Build.cs`
- 주요 의존성:
  - `GameplayAbilities`, `GameplayTags`, `GameplayTasks`
  - `CommonUI`, `UMG`, `EnhancedInput`
  - `AIModule`, `NavigationSystem`, `Niagara`

### `GitStatusBranch` (Editor)

- 위치: `Source/GitStatusBranch`
- 빌드 설정: `Source/GitStatusBranch/GitStatusBranch.Build.cs`
- 목적: 에디터 전용 보조 기능

---

## 🗺️ 코드 도메인 맵

`Source/EternalDreams/Public` 기준:

- `Core/`: 게임 인스턴스, 게임모드/게임스테이트, 로비 흐름, 데이터 서브시스템
- `Characters/`: 플레이어/몬스터 캐릭터, 컨트롤러, 애니메이션 노티파이, GAS 어빌리티/속성
- `AI/`: 몬스터 BT Task/Service/Decorator
- `UI/`: HUD, 패널, 메시지, UI 서브시스템
- `Inventory/`: 인벤토리 컴포넌트, 전송/장착/크래프팅/드랍 로직
- `Item/`: 아이템 타입/핸들, DataAsset, DataTable Row 구조
- `Environment/`: 라이팅 매니저, 제한 구역 등 월드 규칙 요소
- `Interaction/`: 상호작용 대상 컴포넌트
- `Data/`: 게임 공용 타입/태그/데이터 구조
- `Tests/`: 인벤토리/크래프팅 디버그용 BP 라이브러리

---

## 🔄 런타임 흐름

### 1) 로비

- `AEDLobbyGameMode`에서 접속/준비 상태를 관리
- 준비 완료 조건 충족 시 게임 맵으로 이동
- `UEDGameInstance`가 닉네임, 서버 접속, 로딩 화면 흐름을 담당

### 2) 인게임 페이즈

- `AEDGameMode`가 Day/Night 페이즈 사이클을 관리
- 금지구역, 사망/관전/부활, 매치 종료 로직을 서버 권한으로 진행

### 3) 전투

- 플레이어: `AEDPlayerCharacter` + `UEDPlayerAttributeSet`
- 몬스터: `AEDMonsterBase` + AI + Ability
- 데미지 계산: `ExecCalc_Damage`
- 게임플레이 태그: `FEDGameplayTags`

### 4) 데이터 로딩

- `UEDGameDataSubsystem`이 로비/UI/아이템/몬스터/플레이어 데이터를 단계적으로 로드
- Primary Asset 캐시로 런타임 조회 성능/일관성 확보

---

## ⚔️ 주요 시스템 상세

### 🎮 캐릭터 & 전투

- 플레이어/몬스터 모두 Ability System 기반 전투 파이프라인 사용
- AttributeSet을 통해 체력/이동/전투 수치 관리
- 애니메이션 노티파이와 능력 발동이 전투 판정과 연결됨

### 🤖 AI & 몬스터

- Behavior Tree Task/Service/Decorator로 전투 행동 분리
- 몬스터 스폰 서브시스템(`UEDMonsterSpawnSubsystem`)으로 등급별 스폰 제어
- 몬스터 데이터는 데이터 에셋 + 서브시스템 조회 구조로 관리

### 🧩 UI & HUD

- `UEDUIManageSubsystem`이 레이어 기반 패널 오픈/토글/ESC 처리 담당
- HUD, 상태창, 인벤토리, 크래프팅, 토스트 메시지를 컴포넌트/위젯 단위로 분리
- CommonUI + UMG 혼합 구조로 UI 흐름을 구성

### 🎒 인벤토리 & 아이템

- 핵심: `UEDInventoryComponent`
- 일반 슬롯 + 장비 슬롯 + 스킬 슬롯 지원
- 드랍 액터(`AEDDroppedItemActor`) 생성/병합/픽업 처리
- 크래프팅 테이블, 랜덤 루팅, 분배 로직을 데이터 중심으로 구성
- 아이템 정의: `UEDInventoryItemDataAsset`, `FEDCraftingRecipeRow`, `FEDItemSpawnRow`

### 🌍 환경 & 상호작용

- 제한 구역/라이팅 시스템이 페이즈 진행과 연동
- 루팅 타겟 컴포넌트를 통해 월드 오브젝트와 플레이어 상호작용 연결

---

## 🌐 멀티플레이 원칙

- 상태 변경 로직은 **서버 권한 경로**를 기준으로 처리
- 클라이언트는 사전 검증 후 요청하는 패턴 사용 권장
- UI 표시의 최종 신뢰값은 서버 복제 결과 기준으로 반영

---

## 📁 주요 폴더 구조

```text
Source/
  EternalDreams/
    Public/
      AI/
      Characters/
      Core/
      Data/
      Environment/
      Interaction/
      Inventory/
      Item/
      Tests/
      UI/
    Private/
      ...
  GitStatusBranch/
Config/
Content/
```

---

## 🚀 시작하기

### 1) 프로젝트 열기

```powershell
# Rider/Visual Studio에서 솔루션 열기
EternalDreams.sln
```

또는

```powershell
# Unreal Editor에서 프로젝트 열기
EternalDreams.uproject
```

### 2) 빌드/실행 (에디터 환경)

- 에디터에서 C++ 컴파일 후 PIE로 로비/네트워크 시나리오를 검증합니다.
- 기본 시작 맵 및 게임 인스턴스는 `Config/DefaultEngine.ini` 기준입니다.

---

## ✅ 협업 체크리스트

- 기능 수정 시 C++ 로직 + 데이터 에셋/테이블 + UI 바인딩을 함께 점검
- 멀티플레이 기능은 서버/클라 권한 경계와 복제 타이밍을 반드시 확인
- 신규 시스템 추가 시 도메인 분리(`Public`/`Private`)와 책임 경계를 명확히 유지

## 📸 스크린샷

> 아래 영역은 스크린샷 교체용 자리입니다. 파일 업로드 후 경로만 교체해서 사용하세요.

### 로비

![로비 스크린샷 자리](./docs/images/placeholder-lobby.png)

### 전투

![전투 스크린샷 자리](./docs/images/placeholder-combat.png)

### 인벤토리 / 크래프팅

![인벤토리 스크린샷 자리](./docs/images/placeholder-inventory.png)

### UI / HUD

![HUD 스크린샷 자리](./docs/images/placeholder-ui.png)

---
