# WIPI C DRAW ENGINE 3.1

> 🗄️ **기록용 백업 (2026-07-30 스냅샷)** — 원본 폴더 `~/Downloads/wipic_draw3_1`에서 4개 파일을 그대로 분리해 둔 기록 보관본입니다. 적극 개발 계획은 없습니다.

**WIPI(SKT/KTF/LGT) 휴대 게임 플랫폼용 C 드로잉 엔진 (Ver 3.1, 2009-04-13)**

---

## 📌 개요

본 리포지토리는 **WIPI**(SKT/KTF/LGT) 표준 그래픽 API(`MC_GrpImage` 등)에 대한 C 드로잉 엔진의 **4개 C/H 소스** 기록 보관본입니다. 출처 표기는 원본 파일 헤더 주석에만 보존하고, 본 문서에는 별도로 기재하지 않습니다.

| 항목 | 값 |
|------|---|
| 엔진명 | WIPI C DRAW ENGINE |
| 버전 | 3.1 |
| 작성일 | 2009. 4. 13 |
| 대상 플랫폼 | WIPI (SKT / KTF / LGT 3종 캐리어 헤더 분기) |
| 언어 | C99 계열 + MSVC `#pragma warning(disable)` 다수 사용 |

---

## 📂 디렉터리 구성

```
wipic-draw3/
├── README.md     # 본 문서
├── draw.c        # 235,175 bytes — 메인 그래픽/블렌드 엔진 구현
├── draw.h        # 11,474 bytes — 그래픽 API 래퍼 선언 + 캐리어 헤더 분기
├── util.c        # 12,517 bytes — sin/cos LUT, 충돌체크, 파일IO, DSP 헬퍼
└── util.h        # 2,650 bytes — 키/포인터 상수 + 유틸 함수 선언
```

총 4개 파일, 약 261 KB.

---

## 🧩 핵심 컴포넌트

### 1. `draw.h` — 그래픽 API 래퍼

WIPI 표준 `MC_GrpImage` / `MC_GrpContext` / `MC_GrpFrameBuffer`를 한 단계 감싸 게임 코드에서 다루기 쉽게 만든 헤더입니다.

| 그룹 | 함수 / 매크로 |
|------|--------------|
| **기본 설정** | `GetGC`, `SetTrans`, `SetDrawEng`, `GetCenterX`, `GetCenterY` |
| **이미지 I/O** | `LoadImage`, `imgres_idget`, `ReleseImage` (오타 보존) |
| **이미지 그리기** | `DrawImage`, `DrawImageEx`, `DrawString` |
| **도형 그리기** | `DFillRect`, `DDrawRect`, `DFillArc`, `DDrawArc`, `DDrawLine` |
| **클리핑** | `DDCliper` (오타 보존) |
| **구조체** | `DClip` (src 좌표 + 출력 좌표 + 폭/높이), `STREX` (스트릭 워킹 상태) |

#### 캐리어 헤더 분기 (3-way)
```c
#if SKT_APP == 1
  #include "WIPIheader.h"
#endif
#if KTF_APP == 1
  #include <WIPIHeader.h>
#endif
#if LGT_APP == 1
  #include "exp_wipic.h"
#endif
```
`AppSet.h`의 매크로로 3캐리어 헤더를 컴파일 타임 분기합니다.

### 2. `draw.c` — 블렌드 모드 매크로 (25종)

드로잉 엔진의 핵심 자산은 **채널 단위 블렌드 매크로** 모음입니다. WIPI 휴대 게임기에 최적화된 25가지 블렌드 모드를 한 헤더 매크로 트리거로 표현합니다.

#### 색상 효과 / 합성 / 비교
`Normal`, `Lighten`, `Darken`, `Multiply`, `Average`, `Add`, `Subtract`, `Difference`, `Negation`, `Screen`, `Exclusion`, `Overlay`, `SoftLight`, `HardLight`, `ColorDodge`, `ColorBurn`, `LinearDodge`, `LinearBurn`, `LinearLight`, `VividLight`, `PinLight`, `HardMix`, `Reflect`, `Glow`, `Phoenix`

#### 알파 합성
- `ChannelBlend_Alpha(B,L,O)` — 채널별 alpha 보간 (O ∈ [0,1])
- `ChannelBlend_AlphaF(B,L,F,O)` — 임의 블렌드 함수 + 알파 결합

### 3. `util.c/h` — 범용 유틸

| 영역 | 내용 |
|------|------|
| **삼각함수 LUT** | `fsinTbl[36]`, `fcosTbl[36]` — 36스텝(10°) 정수 sin/cos (WIPI 휴대 단말에서 부동소수 회전 비용 회피용) |
| **문자열 임시버퍼** | `g_TempStr1[100]`, `g_TempStr2[512]` (네트워크/DSP 모듈 공용) |
| **포인터 이벤트** | `MC_PointerEvent g_pPointerEvt` — 터치/마우스 (캐리어별 매크로 분기) |
| **키 코드 enum** | `WIPI_KEY_UP/DOWN/LEFT/RIGHT/OK/CLR/SOFT1/SOFT2/0~9/STAR/SARP/UP_SCR/DOWN_SCR` |
| **바이트 변환** | `ByteToShort`, `ByteToInt` (1/2바이트 → M_Int16/32) |
| **충돌 체크** | `Collision_Chk(sx, sy, sw, sh, dx, dy, dw, dh)` |
| **난수** | `WSrand`, `WRand` (실수 회피용 정수 난수) |
| **정렬/샘플링** | `NotOverlapRand`, `Bubble_sort` |
| **파일 IO** | `File_Write`, `File_Read`, `dspSaveFile`, `dspLoadFile`, `dspGetFileSize` |
| **메모리** | `exAlloc`, `exFree` (PTR 구조체 기반 핸들) |
| **기하/숫자** | `umin`, `umax`, `umin3`, `umax3`, `NumPosMax`, `PointCheck` |

---

## ⚠️ 알려진 함정 / 노트

1. **오타 보존** — `ReleseImage` (Release의 오타), `DDCliper` (Clipper의 오타)는 **원본 그대로** 보존합니다. 외부 링크 호환성을 위해 임의 수정 금지.
2. **MSVC 전용 경고** — `util.c` / `draw.c` 상단의 `#pragma warning(disable : …)` 13종은 Visual C++ 6.0 빌드 환경에서 함수 시그니처/암묵 변환 경고를 끈 것. GCC/Clang에서는 의미 없는 pragma.
3. **WIPI 헤더 비포함** — 본 리포에는 `WIPIheader.h`, `WIPIHeader.h`, `exp_wipic.h`, `exp_wipic_lgt.h`, `AppSet.h`가 **포함되지 않습니다**. 실 컴파일은 SKT/KTF/LGT WIPI SDK가 별도로 필요합니다.
4. **인코딩** — 원본 헤더 주석이 euc-kr로 작성돼 있습니다. 본 `README.md`만 UTF-8.
5. **신규 통합성 검증 부재** — 채널 블렌드 매크로의 정확한 RGB 채널·알파 처리 일치성은 미검증. 시각 효과 참고용으로만 사용하세요.

---

## 📜 출처 / 라이선스

원본 저작자/저작권 표기는 **소스 파일 헤더 주석**에만 보존되어 있습니다. 본 리포지토리는 외부 공개를 전제로 한 배포본이 아니며, 원본 파일에 명시된 권리 표기를 따릅니다. 별도 라이선스 파일은 포함되어 있지 않습니다.
