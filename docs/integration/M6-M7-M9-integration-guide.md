# M6 / M7 / M9 集成与证据采集指南

> 关联: [主计划](../../.cursor/plans/cod-style_tps_demo_cce8f423.plan.md) · [M6 生命与伤害](../../.cursor/plans/tps-demo-modules/module-06-health.md) · [M7 命中反馈](../../.cursor/plans/tps-demo-modules/module-07-feedback.md) · [M9 HUD/UI](../../.cursor/plans/tps-demo-modules/module-09-hud.md)
>
> 本指南面向**技术美术 / 测试 / 集成**角色，承接已完成、已编译、已审查、已提交的 M6/M7/M9 **C++ 开发产物**。

---

## 0. 当前已完成（C++ + 集成连线）

| 项 | 状态 | 说明 |
|---|---|---|
| M6 伤害结算/死亡/重生 C++ | ✅ 完成·编译·审查·提交 | `UTSAttributeSet`、`ATSCharacterBase::HandleDeath`、`ATSPlayerCharacter/ATSAICharacter::OnDeath` |
| M9 HUD C++ 框架 | ✅ | `UTSUserWidget`、`ATSPlayerController`、`ATSGameMode`、武器 `OnAmmoChanged` 委托 |
| M7 命中反馈 C++ | ✅ | `ATSPlayerController` 订阅 Hit/Kill/TakeDamage 事件，调用 `OnHitMarker`/`OnDamageDirection` BIE |
| `BP_TSPlayerController` | ✅ 已建·已编译 | `/Game/TPS/Blueprints/`，父类 `ATSPlayerController` |
| `BP_TSGameMode` | ✅ 已建·已编译·已配置 | PlayerControllerClass=`BP_TSPlayerController`，DefaultPawnClass=`BP_TSPlayerCharacter` |
| 关卡 GameMode 覆盖 | ✅ 已生效·已保存 | `Lvl_ThirdPerson` World Settings → GameMode Override = `BP_TSGameMode` |

**结论**：现在进入 PIE，玩家会以 `BP_TSPlayerCharacter` 出生、由 `BP_TSPlayerController` 控制；M6 的扣血/死亡/重生、M7 的事件订阅已在运行。**唯一缺口是 HUD 可视控件（WBP）尚未搭建并赋给控制器** —— 这部分必须在编辑器 UMG 设计器内手工完成（MCP 无 UMG 控件树接口）。

---

## 1. UTSUserWidget 提供的接口（搭 WBP 时直接用）

WBP 继承 `TSUserWidget` 后，可直接调用以下成员：

**BlueprintPure 取值（用于 UMG 属性绑定 Binding）**
| 函数 | 返回 | 用途 |
|---|---|---|
| `GetHealthPercent()` | float 0~1 | 血条 ProgressBar 的 Percent |
| `GetCurrentClipAmmo()` | int32 | 弹匣余弹文本 |
| `GetCurrentReserveAmmo()` | int32 | 备弹文本 |
| `GetCurrentWeaponName()` | FName | 武器名文本 |
| `IsADS()` | bool | 准心收紧/隐藏判断 |
| `IsSprinting()` | bool | 准心扩散判断 |
| `GetCrosshairGapPixels()` | float | 准心四线间距（已含 ADS/移动/开火脉冲逻辑） |

**BlueprintImplementableEvent（在 WBP 事件图里覆写实现视觉）**
| 事件 | 触发时机 | 期望视觉 |
|---|---|---|
| `OnHealthChanged(NewHealth, MaxHealth)` | 血量变化（推送） | 可做掉血闪烁动画（基础显示用 Binding 即可） |
| `OnAmmoChanged(ClipAmmo, ReserveAmmo)` | 开火/换弹/切枪 | 弹药刷新动画（基础显示用 Binding 即可） |
| `OnWeaponChanged(WeaponName, ClipAmmo, ReserveAmmo)` | 切枪 | 武器名+弹药整体刷新 |
| `OnHitMarker(bIsKill)` | M7 命中/击杀 | `false`=白色 X 0.15s 淡出；`true`=红色加粗 0.3s |
| `OnDamageDirection(AngleDegrees)` | M7 受伤 | 红色弧形指示器，按角度旋转，1.5s 淡出（0=前 +90=右 180=后 -90=左） |

> 控制器侧：`ATSPlayerController` 已在 `BeginPlay` 创建 `HUDWidgetClass` 实例并 `InitializeForPlayer(this)`，在 `OnPossess`（含重生）后 `RebindToOwner()`。你只需把搭好的 WBP 指给 `HUDWidgetClass`。

---

## 2. WBP 搭建步骤（UMG 设计器）

> 推荐**单根方案**：只建一个 `WBP_HUDRoot`（父类 `TSUserWidget`），其余视觉作为它内部的子控件。原因：C++ 只对这一个根控件调用 `InitializeForPlayer`，单根最省事且与现有代码完全契合。

### 2.1 新建 WBP_HUDRoot
1. 内容浏览器 `/Game/TPS/UI/`（没有就新建文件夹）→ 右键 → User Interface → Widget Blueprint。
2. 选父类时点 **All Classes**，搜 `TSUserWidget`，选它作为父类，命名 `WBP_HUDRoot`。
3. 打开，Designer 根用 **Canvas Panel**。

> **两种绑定机制（务必分清）**
> - **机制 A：属性绑定（Bind 下拉）** —— 用于 Details 面板属性右侧带 **Bind ▾** 的属性（ProgressBar `Percent`、TextBlock `Text` 等）。点 **Bind ▾ → Create Binding** 生成一个返回该类型的函数图，引擎每帧自动调用；你在图里调用 Pure 取值函数 → 连到 Return Node。
> - **机制 B：Event Graph 连线（Tick）** —— Canvas Slot 位置 / RenderTransform **没有 Bind 下拉**，必须在 Event Graph 的 **Event Tick** 里用代码改位置。准心走这套。

### 2.2 准心（CP9-4）— 机制 B（Event Tick）
1. 放 4 个 `Image`（白，约 2×10 px），命名 `Cross_Up/Down/Left/Right`；每个 **Anchors 居中**、**Alignment=(0.5,0.5)**、Position(0,0)（默认叠在屏幕中心）。
2. 确认每个 Image 顶部勾了 **Is Variable**（默认勾选）。
3. 切到 **Graph** 标签，从 **Event Tick** 拉出：
   - 右键搜 **Get Crosshair Gap Pixels**（Pure，float），记为 `Gap`。
   - 串联 4 个 **Set Render Translation**：Tick → Up → Down → Left → Right。
   - 每个的 `Target` 连对应 `Cross_*` 变量；`In Translation` 用 **Make Vector 2D**：

| 线 | X | Y |
|---|---|---|
| Cross_Up | 0 | `Gap * -1` |
| Cross_Down | 0 | `Gap` |
| Cross_Left | `Gap * -1` | 0 |
| Cross_Right | `Gap` | 0 |

4. `Gap * -1` 用 **float * float**（另一输入填 -1）。Compile。
5. 可选：选准心容器，`Visibility` 用机制 A 绑定 → `IsADS()` 为真时返回 Collapsed（瞄准镜点）。

### 2.3 血条（CP9-3）— 机制 A
1. 选中 `HealthBar`（ProgressBar）。
2. Details → **Progress** → `Percent` 右侧 **Bind ▾ → Create Binding**。
3. 生成的函数图里：右键搜 **Get Health Percent**（`TS|HUD` 分类，Pure）→ 输出连到 Return Node 的 `Return Value`。
4. Compile。

### 2.4 弹药 + 武器名（CP9-2 / CP9-5）— 机制 A
**弹药 `AmmoText`（TextBlock）**
1. 选中 → Details → **Content** → `Text` → **Bind ▾ → Create Binding**。
2. 图里右键搜 **Format Text**，模板填 `{clip} / {reserve}`（自动长出两个输入针）。
3. **Get Current Clip Ammo** → `clip`；**Get Current Reserve Ammo** → `reserve`（int 自动转换）。
4. Format Text 的 `Result` → Return Node。Compile。

**武器名 `WeaponNameText`（TextBlock）**
1. 选中 → `Text` → **Bind ▾ → Create Binding**。
2. 右键搜 **Get Current Weapon Name**（Pure，Name）→ 从输出拖线搜 **To Text (Name)** → 连 Return Node。Compile。

### 2.5 命中标记（CP7-1 / CP7-2）
- `Image` `HitMarker`（X 形贴图或两条交叉线组合），居中，默认 `Visibility=Hidden`。
- 建两个动画：`Anim_HitWhite`（0.15s alpha 1→0）、`Anim_HitKill`（0.3s，颜色红、放大、alpha 1→0）。
- 事件图覆写 `OnHitMarker(bIsKill)`：`Branch(bIsKill)` → True 播 `Anim_HitKill`、False 播 `Anim_HitWhite`（播放前设可见）。

### 2.6 受伤方向指示器（CP7-3 / CP7-4）
- `Image` `DamageArc`（红色弧/箭头贴图），锚点居中，轴心在中心，默认 Hidden。
- 动画 `Anim_DamageFade`（1.5s alpha 1→0）。
- 事件图覆写 `OnDamageDirection(AngleDegrees)`：`SetRenderTransformAngle(AngleDegrees)` → 设可见 → 播 `Anim_DamageFade`。
  - 要"贴屏幕边缘"：可把 DamageArc 放进一个居中的容器并按角度做半径偏移；最简版只旋转一个居中红弧即可满足"右/后/左"方向判读。

### 2.7 编译保存
- 编译 `WBP_HUDRoot`（0 error）。基础显示用属性绑定即可通过 CP9；如要 CP9-2/3 的动画刷新再实现 `OnAmmoChanged`/`OnHealthChanged`。

---

## 3. 把 WBP 赋给控制器 + 击杀音

1. 打开 `/Game/TPS/Blueprints/BP_TSPlayerController`，Class Defaults：
   - **HUD Widget Class** = `WBP_HUDRoot`。
   - **Kill Sound** = 任一击杀音 `USoundBase`（无资源可先留空，CP7-2 的音效项延后）。
2. 编译保存 `BP_TSPlayerController`。

> 完成后，告诉我即可——我可以用 MCP 校验 `BP_TSPlayerController` 的 `HUDWidgetClass`/`KillSound` 是否正确写入，再开始取证。

---

## 4. 测试关卡布置

- `Lvl_ThirdPerson` 已指向 `BP_TSGameMode`。
- 在玩家出生点前方放 1~2 个 `BP_TSEnemy`（`/Game/TPS/Blueprints/BP_TSEnemy`）。我可用 MCP 的 `add_to_scene_from_asset` 帮你放置（给我坐标或让我放在出生点前方约 600~1000 cm）。
- 确认敌人持有可造成伤害的武器配置（M4/M5 的 WeaponDataAsset）。

---

## 5. 证据采集清单（按模块文档命名，存 `docs/evidence/module-0X/`）

> 凡"手感/动画/UI/方向"必须用帧序列或录屏；属性数值可用 `showdebug abilitysystem` 截图。开火/移动需你实际游玩（MCP 不能注入输入）；我可在你游玩时用 MCP 抓视口截图、读日志辅助取证。

### M6 → `docs/evidence/module-06/`
| 证据 | 文件名 | 采集方式 |
|---|---|---|
| A 扣血序列 | `CP6-A_hp_100.png` / `CP6-A_hp_72.png` / `CP6-A_hp_0.png` | PIE 中 `showdebug abilitysystem`，命中前/1 发后/致命后各截一张（验证 100→72→…→0 不为负） |
| B AI 死亡 | `CP6-B_enemy_death.mp4` | 录 AI 中弹→Ragdoll 倒地（≥6 帧/连续录屏） |
| C 玩家重生 | `CP6-C_player_respawn.mp4` | 录玩家死亡→3.0s→出生点满血重生 |
| D AI 清理 | `CP6-D_cleanup_before.png` / `CP6-D_cleanup_after.png` | AI 死亡瞬间 vs 5.0s 后已消失对比 |

### M7 → `docs/evidence/module-07/`
| 证据 | 文件名 | 采集方式 |
|---|---|---|
| A 命中标记 | `CP7-A_hitmarker.png` | 命中敌人瞬间，准心处白色 X 可见 |
| B 击杀标记 | `CP7-B_killmarker.mp4` | 击杀瞬间 Hitmarker 变红 + 含音轨录屏 |
| C 受伤方向 | `CP7-C_dir_right.png` / `CP7-C_dir_back.png` / `CP7-C_dir_left.png` | 从右/后/左被击各一张，标注来源与指示器方向一致（误差<30°） |

### M9 → `docs/evidence/module-09/`
| 证据 | 文件名 | 采集方式 |
|---|---|---|
| A HUD 全貌 | `CP9-A_hud_overview.png` | PIE 画面，准心+弹药+血条+武器名清晰 |
| B 弹药递减 | `CP9-B_ammo_f1.png … f4.png` | 连开 3 发帧序列（30→29→28→27） |
| C 血条变化 | `CP9-C_health_before.png` / `CP9-C_health_after.png` | 受伤前后血条长度明显变化 |
| D 准心动态 | `CP9-D_crosshair.mp4` | 静止→行走→开火→ADS 准心变化录屏 |

---

## 6. 推进顺序建议

1. 搭 `WBP_HUDRoot`（§2）→ 赋给 `BP_TSPlayerController`（§3）→ 告诉我，我用 MCP 校验。
2. 我帮你在关卡放敌人（§4）。
3. 你进 PIE 实际游玩，我用 MCP 抓视口截图/读日志，按 §5 清单逐项产出证据。
4. 证据齐全后，由集成角色判定 CP6/CP7/CP9 通过并标记模块完成。
