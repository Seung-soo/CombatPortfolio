# CombatPortfolio Development Log

## Project Goal

C++ 중심의 3인칭 액션 전투 포트폴리오를 제작한다.  
빈 Unreal Engine 프로젝트에서 시작하여 이동, 카메라, 락온, 회피, 콤보, 피격, AI, UI, 디버그 기능까지 직접 구현한다.

## Technical Direction

- Blank C++ Project 기반
- Enhanced Input 기반 입력 시스템
- C++로 핵심 로직 구현
- Blueprint는 데이터 설정과 연출 레이어로 제한적으로 사용
- 기능별 폴더 구조와 컴포넌트 기반 설계 적용
- Git + Git LFS를 사용한 버전 관리

## Episode 1

### Completed

- Blank C++ Unreal project created
- Base Content folder structure created
- Enhanced Input plugin checked
- EnhancedInput module dependency added
- Git repository initialized
- Git LFS configured

### Notes

템플릿 프로젝트를 사용하지 않고 직접 구조를 구성함으로써 입력, 캐릭터, 카메라, 게임모드의 동작 원리를 단계적으로 학습할 수 있도록 했다.





## Episode 2

### Goal

Blank C++ 프로젝트에서 직접 플레이어 캐릭터, 플레이어 컨트롤러, 게임모드를 구성하고 Enhanced Input 기반 이동/카메라 입력을 구현한다.

### Completed

- Created `ACombatPlayerCharacter`
- Added SpringArm and Camera components in C++
- Configured basic CharacterMovement settings
- Created `ACombatPlayerController`
- Registered Enhanced Input Mapping Context through PlayerController
- Created `ACombatGameMode`
- Created `IA_Move`, `IA_Look`, and `IMC_Player`
- Created Blueprint subclasses for Character, PlayerController, and GameMode
- Set default pawn and controller through GameMode
- Verified WASD movement and mouse look in `L_CombatSandbox`

### Technical Notes

- Character is used as the player body because it provides capsule collision, mesh support, and built-in movement logic.
- PlayerController owns the input mapping registration because it represents the local player's control layer.
- Core movement logic is implemented in C++, while asset references are assigned in Blueprint.




## Episode 3

### Goal

플레이어 캐릭터에 Skeletal Mesh와 Animation Blueprint를 연결하고, 이동 속도 기반 Locomotion Blend Space를 구성한다.

### Completed

- Added temporary mannequin skeletal mesh for player visualization
- Created `UCombatAnimInstance`
- Calculated `GroundSpeed` and `bIsFalling` in C++
- Created `ABP_CombatPlayer` based on `UCombatAnimInstance`
- Created `BS_Player_Locomotion`
- Connected Idle / Walk / Run animations through Blend Space
- Assigned Animation Blueprint to `BP_CombatPlayerCharacter`
- Verified movement animation in `L_CombatSandbox`

### Technical Notes

- Character capsule is used as the collision and movement root.
- Skeletal Mesh is used only for visual representation.
- Animation state values are calculated in C++ and consumed by Animation Blueprint.
- Basic locomotion uses in-place animation driven by `CharacterMovement`.



## Episode 4

### Goal

플레이어 캐릭터의 이동 상태를 걷기, 기본 달리기, 스프린트로 분리하고, 입력 기반 상태 관리 구조를 구현한다.

### Completed

- Added `IA_Walk` and `IA_Sprint`
- Mapped `Left Ctrl` to walk and `Left Shift` to sprint
- Added `ECombatMovementState`
- Added movement speed variables: `WalkSpeed`, `RunSpeed`, `SprintSpeed`
- Implemented movement state update flow
- Applied movement speed through `CharacterMovementComponent::MaxWalkSpeed`
- Added camera sensitivity variables
- Added movement debug output using `GEngine->AddOnScreenDebugMessage`
- Updated locomotion Blend Space axis from 500 to 700

### Technical Notes

- Input intent and final movement state are separated.
- `bWantsToWalk` and `bWantsToSprint` represent player input intent.
- `MovementState` represents the resolved movement state after priority rules.
- Movement speed is updated through a single function to keep the logic centralized.
- Debug output is currently Tick-based and will later be moved behind a dedicated debug system.



## Episode 5

### Goal

전투 게임에 필요한 캐릭터 회전 기준을 정리하고, 일반 이동 모드와 Strafe 모드를 전환할 수 있는 구조를 구현한다.

### Completed

- Added `IA_ToggleRotationMode`
- Mapped `Tab` to rotation mode toggle
- Added `ECombatRotationMode`
- Implemented `OrientToMovement` rotation mode
- Implemented `Strafe` rotation mode
- Added `SetRotationMode()` and `ApplyRotationMode()`
- Added camera lag to SpringArm
- Added camera pitch limits in `ACombatPlayerController`
- Extended debug output to show current rotation mode and control yaw

### Technical Notes

- `OrientToMovement` mode rotates the character toward movement direction.
- `Strafe` mode rotates the character using Controller Yaw.
- Rotation mode is separated from movement state to prepare for lock-on combat.
- Camera pitch limits are controlled by PlayerController through PlayerCameraManager.


## Episode 6

### Goal

전투 로직을 Character에서 분리하기 위해 `UCombatComponent`를 만들고, 공격 입력과 공격 상태 흐름을 구현한다.

### Completed

- Created `UCombatComponent`
- Added `ECombatActionState`
- Implemented `RequestAttack()`, `CanStartAttack()`, `StartAttack()`, and `FinishAttack()`
- Added temporary attack duration timer
- Added `IA_Attack`
- Mapped `Left Mouse Button` to attack input
- Added `CombatComponent` to `ACombatPlayerCharacter`
- Connected attack input from Character to CombatComponent
- Reduced movement speed while attacking
- Extended debug output to show combat state

### Technical Notes

- Character receives input and forwards combat requests to CombatComponent.
- CombatComponent owns combat state and attack timing.
- Attack is requested through `RequestAttack()` instead of directly changing combat state.
- Attack state currently uses a timer and will later be connected to animation montage timing.


## Episode 7

### Goal

공격 상태를 실제 Animation Montage 재생 흐름과 연결하고, CombatComponent의 상태 변경을 Character가 이벤트 기반으로 받도록 개선한다.

### Completed

- Created `AM_Player_Attack_01`
- Added `DefaultSlot` to `ABP_CombatPlayer`
- Replaced temporary attack timer with montage end handling
- Added `AttackMontage` and `AttackPlayRate` to `UCombatComponent`
- Played attack montage through `UAnimInstance::Montage_Play`
- Added montage end delegate to finish attack state
- Added combat state changed delegate
- Connected Character movement speed update to CombatComponent state changes
- Reduced Character Tick responsibility to debug output only

### Technical Notes

- CombatComponent owns attack state and montage playback.
- Character forwards attack input to CombatComponent.
- Animation Blueprint receives montage playback through `DefaultSlot`.
- Combat state changes are broadcast through a delegate instead of being polled every frame.


## Episode 8

### Goal

공격 애니메이션의 실제 타격 가능 구간을 Anim Notify State로 정의하고, CombatComponent에서 Hit Window 상태를 관리한다.

### Completed

- Added `bHitWindowOpen` to `UCombatComponent`
- Added `BeginHitWindow()` and `EndHitWindow()`
- Added `OnHitWindowChanged` delegate
- Created `UCombatAnimNotifyState_HitWindow`
- Connected NotifyBegin to `CombatComponent::BeginHitWindow()`
- Connected NotifyEnd to `CombatComponent::EndHitWindow()`
- Added Hit Window debug output to player character
- Placed Hit Window Notify State inside `AM_Player_Attack_01`

### Technical Notes

- Hit Window represents the time range where attack collision will be valid.
- Attack timing is authored on the animation timeline through Anim Notify State.
- CombatComponent owns hit window state.
- Notify State finds CombatComponent from the animation owner instead of depending on the player character class directly.


## Episode 9

### Goal

Hit Window 동안 Sphere Trace 기반 공격 판정을 수행하고, 같은 공격에서 동일 대상이 중복으로 맞지 않도록 처리한다.

### Completed

- Enabled `UCombatComponent` Tick only during Hit Window
- Added attack trace settings:
    - `AttackTraceRadius`
    - `AttackTraceForwardOffset`
    - `AttackTraceHalfHeight`
- Implemented `PerformAttackTrace()`
- Used sphere sweep to detect actors in front of the character
- Ignored the owner actor in collision query params
- Added debug capsule visualization
- Added `HitActorsThisAttack` to prevent duplicate hits
- Added hit count debug output
- Created temporary `BP_DummyTarget` for trace testing

### Technical Notes

- Attack tracing is active only while Hit Window is open.
- CombatComponent Tick is disabled outside attack hit windows.
- Detected actors are stored per attack to prevent repeated hits from the same attack.
- Damage is not applied yet; this episode focuses only on hit detection.


## Episode 10

### Goal

공격 판정으로 감지된 Actor에게 데미지를 적용하고, 체력과 사망 상태를 관리하는 HealthComponent를 구현한다.

### Completed

- Created `UHealthComponent`
- Added `MaxHealth`, `CurrentHealth`, and `bDead`
- Implemented `ApplyDamage()`
- Added health clamping through `FMath::Clamp`
- Added `OnHealthChanged` and `OnDeath` delegates
- Added `AttackDamage` to `UCombatComponent`
- Applied damage to hit actors with `HealthComponent`
- Added `HealthComponent` to `BP_DummyTarget`
- Verified HP reduction through Output Log

### Technical Notes

- CombatComponent detects hit actors and requests damage.
- HealthComponent owns health state, death state, and health change events.
- Damage is ignored when the target is already dead.
- This episode uses direct HealthComponent lookup; Interface-based damage handling may be introduced later.


## Episode 11

### Goal

HealthComponent의 체력 변경/사망 이벤트를 기반으로 DummyTarget의 피격 반응과 사망 반응을 구현한다.

### Completed

- Updated `OnHealthChanged` delegate to pass current health, max health, and delta
- Created `ACombatDummyTarget`
- Added Capsule, StaticMesh, and HealthComponent to DummyTarget
- Bound DummyTarget reaction functions to HealthComponent events
- Implemented temporary hit reaction using mesh scale feedback
- Added debug sphere feedback on hit
- Implemented temporary death state
- Disabled collision on death
- Created `BP_CombatDummyTarget` for test placement

### Technical Notes

- HealthComponent owns health state and broadcasts health/death events.
- DummyTarget listens to HealthComponent events and performs visual reactions.
- Hit reaction uses Timer instead of Tick.
- Death handling currently uses temporary visual feedback and collision disabling.


## Episode 12

### Goal

공격 중 입력을 무시하지 않고, 특정 Combo Input Window 안에서 다음 공격으로 예약하는 기본 콤보 시스템을 구현한다.

### Completed

- Added combo state variables to `UCombatComponent`
  - `CurrentComboIndex`
  - `bComboInputWindowOpen`
  - `bComboInputBuffered`
- Added `ComboSectionNames`
- Updated `RequestAttack()` to handle attack input during attacking state
- Implemented combo input buffering
- Implemented combo commit using montage section jump
- Created `UCombatAnimNotifyState_ComboInputWindow`
- Added ComboInputWindow Notify State to attack montage
- Added combo debug output to player character
- Configured montage sections:
  - `Attack_1`
  - `Attack_2`
  - `Attack_3`

### Technical Notes

- HitWindow controls damage timing.
- ComboInputWindow controls input buffering timing.
- Combo input is accepted only while ComboInputWindow is open.
- Current implementation uses `Montage_JumpToSection`; later it may be improved with section transition control.


## Episode 13

### Goal

콤보 단계별로 Section, Damage, Trace 범위를 분리해 공격 데이터를 구조화한다.

### Completed

- Added `FComboAttackData`
- Replaced `ComboSectionNames` with `ComboAttackDataList`
- Removed shared `AttackDamage`
- Removed shared attack trace radius/offset values
- Added per-combo attack damage
- Added per-combo trace radius
- Added per-combo trace forward offset and half height
- Updated attack trace to use current combo data
- Updated damage application to use current combo data
- Added current attack damage to debug output

### Technical Notes

- Each combo step is represented by a single `FComboAttackData`.
- `CurrentComboIndex` is used to select the active combo data.
- Combo data is currently stored in CombatComponent and can later be moved to DataAsset.
- This structure allows each combo attack to have different damage and hit detection settings.


## Episode 14

### Goal

플레이어 회피 입력을 추가하고, 회피 상태, 회피 이동, 회피 몽타주, 짧은 무적 상태를 구현한다.

### Completed

- Added `IA_Dodge`
- Mapped `Space Bar` to dodge input
- Added `Dodging` to `ECombatActionState`
- Added dodge settings to `UCombatComponent`
  - `DodgeMontage`
  - `DodgeStrength`
  - `DodgeDuration`
  - `DodgeInvincibleDuration`
- Implemented `RequestDodge()`
- Implemented dodge direction calculation in `ACombatPlayerCharacter`
- Applied dodge movement using `LaunchCharacter`
- Added temporary invincibility state during dodge
- Blocked attack and sprint during dodge
- Added dodge and invincibility debug output
- Created `AM_Player_Dodge`

### Technical Notes

- Character calculates dodge direction from camera-relative movement input.
- CombatComponent owns dodge state and invincibility state.
- Dodge currently uses `LaunchCharacter` as a prototype movement method.
- Future improvement may replace launch-based dodge with Root Motion or curve-driven movement.


## Episode 15

### Goal

스프린트와 회피에 스태미나 비용을 연결하고, 스태미나 소모/회복/고갈 이벤트를 관리하는 StaminaComponent를 구현한다.

### Completed

- Created `UStaminaComponent`
- Added `MaxStamina` and `CurrentStamina`
- Implemented instant stamina spending with `TrySpendStamina()`
- Implemented continuous stamina drain with `StartStaminaDrain()` and `StopStaminaDrain()`
- Added stamina regeneration delay using Timer
- Enabled StaminaComponent Tick only during drain or regeneration
- Added `OnStaminaChanged` and `OnStaminaDepleted` delegates
- Added StaminaComponent to player character
- Connected sprint stamina drain
- Connected dodge stamina cost
- Stopped sprint automatically when stamina is depleted
- Added stamina debug output

### Technical Notes

- Sprint uses continuous stamina drain.
- Dodge uses instant stamina spending.
- Stamina regeneration starts after a delay.
- Component Tick is enabled only while stamina is draining or regenerating.
- StaminaComponent owns stamina state; Character decides which actions consume stamina.



## Episode 16

### Goal

플레이어에게 HealthComponent를 추가하고, 회피 무적 상태를 실제 데미지 처리 흐름에 연결한다.

### Completed

- Added HealthComponent to `ACombatPlayerCharacter`
- Bound player health and death event handlers
- Added player health debug output
- Added temporary `DebugApplyDamageToPlayer()` function
- Added invincibility check before applying damage
- Blocked damage when target CombatComponent is invincible
- Connected dodge IFrame state to actual damage prevention
- Added temporary player death handling

### Technical Notes

- CombatComponent checks target invincibility before applying damage.
- HealthComponent remains responsible only for health and death state.
- Player can now receive damage through HealthComponent.
- Debug damage function is temporary until enemy attacks are implemented.