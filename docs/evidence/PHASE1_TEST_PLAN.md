# 阶段 1 (CP-Phase1) 测试与证据计划

> 本文件由“开发”子代理在编辑器内用 Unreal MCP 搭建阶段1剩余资产后生成。
> 覆盖模块 M2(角色)/M10a(输入)/M3(移动) 的验收证据状态。
> 限制：MCP **无法注入玩家按键**，因此所有“行为/手感类”证据（strafe、冲刺、滑铲、蹲伏、按住语义、互斥）必须由人工在 PIE 实操采集；本计划逐条给出操作步骤、预期现象、CP 编号与建议文件名。

---

## A. 已自动采集的可视化证据（已落盘）

| 文件 | 路径 | 采集方式 | 对应验收点 | 说明 |
|---|---|---|---|---|
| `CP2-A_over_shoulder.png` | `docs/evidence/module-02/` | PIE(InViewPort, warmup 2s) + CaptureEditorImage | CP2-1 / CP2-A | 玩家角色以越肩第三人称视角生成，角色偏屏幕中线左侧（越肩构图），相机臂长≈250、肩部偏移生效；画面中可见远处放置的 BP_TSEnemy。 |
| `CP2-C_enemy_render.png` | `docs/evidence/module-02/` | 编辑器视口 CaptureViewport（无 PIE，俯瞰敌人） | CP2-5（部分） | 场景中放置的 `BP_TSEnemy_1`（SKM_Quinn_Simple + ABP_Unarmed）正确直立、贴地，胶囊与网格变换对齐（RelativeLocation Z=-89, Yaw=-90），证明敌人蓝图网格/变换配置正确。 |

> 说明：CaptureEditorImage 捕获“整个编辑器应用画面”，在 PIE 停止后偶发 “Failed to capture any editor windows”（窗口焦点问题）；此时改用 CaptureViewport（直接渲染视口，数据在 `returnValue.image.data`）可稳定取图。

---

## B. 需人工在 PIE 实操采集的行为类证据（MCP 无法自动）

> 进入方式：编辑器顶部 Play（默认关卡 `Lvl_ThirdPerson`，GameMode 默认 Pawn 已设为 `BP_TSPlayerCharacter_C`）。建议先在控制台输入 `showdebug abilitysystem` 以便同屏观察 GameplayTag。

### B1. 越肩 strafe（CP2-3 / 证据 CP2-B）
- 操作：仅移动鼠标转动视角（不按 WASD），再按住 A/D 横向移动。
- 预期：角色身体 Yaw 跟随相机方向旋转（始终面向相机朝向），横移时播放左右 strafe，不自动转向移动方向。
- 文件名：`docs/evidence/module-02/CP2-B_strafe.mp4`（或 `CP2-B_strafe_f1..f4.png`）。

### B2. 玩家/敌人 ASC 属性（CP2-2 / CP2-5 / 证据 CP2-C）
- 操作：PIE 中 `showdebug abilitysystem`，分别对玩家与敌人（可用 `nextdebugtarget` 或点选）截图。
- 预期：Owner/Avatar 非空，Health=100 / MaxHealth=100。
- 文件名：`docs/evidence/module-02/CP2-C_player_asc.png`、`docs/evidence/module-02/CP2-C_enemy_asc.png`。

### B3. 全键位输入屏幕反馈（CP10a-1/2/4 / 证据 CP10a-A、CP10a-B）
- 前置：模块10a 文档要求各 handler 用 `AddOnScreenDebugMessage` 打印对应 tag。若 C++ 中尚未加屏显日志，需开发补上后再采集。
- 操作：依次触发 9 个键（见下表“按键映射”），按住 Shift/RMB 验证 Pressed/Released 各一次，滚轮上下各一次。
- 文件名：`docs/evidence/module-10a/CP10a-A_onscreen_inputs.png`、`CP10a-B_press.png`、`CP10a-B_release.png`。

### B4. 冲刺提速/退出（CP3-1/2 / 证据 CP3-A）
- 操作：前进按住 LeftShift；松开或改后退。
- 预期：速度 450→750（±10）；松开/后退 0.2s 内回落到 ≤450。
- 文件名：`docs/evidence/module-03/CP3-A_sprint_vs_walk.mp4`。

### B5. 滑铲位移与冷却（CP3-3/4 / 证据 CP3-B）
- 操作：冲刺中按 LeftControl 触发滑铲；连续再触发验证冷却。
- 预期：胶囊半高 88→44，前冲位移 ≥3m，持续≈0.8s；两次成功滑铲间隔 ≥1.5s（`Cooldown.Slide`）。
- 文件名：`docs/evidence/module-03/CP3-B_slide.mp4`（或 `CP3-B_slide_f1..f6.png`）。

### B6. 蹲伏（CP3-5 / 证据 CP3-C）
- 操作：按 C 切换蹲/起；头顶有遮挡时尝试起身。
- 预期：胶囊半高 44、移速 ≤250；遮挡时无法起身。
- 文件名：`docs/evidence/module-03/CP3-C_stand.png`、`CP3-C_crouch.png`。

### B7. 互斥：ADS 阻止冲刺（CP3-6 / 证据 CP3-D）
- 操作：按住 RMB(ADS) 同时尝试 LeftShift 冲刺。
- 预期：`showdebug` tag 区无 `State.Movement.Sprinting`（GA_Sprint 的 ActivationBlockedTags 含 State.Combat.ADS）。
- 文件名：`docs/evidence/module-03/CP3-D_mutex_ads_sprint.png`。

> 注意：B4–B7 的能力逻辑依赖对应 GameplayAbility 的实际激活。GA_Fire/GA_ADS/GA_Reload/GA_WeaponSwitch 在阶段2才实现，故 Fire/ADS/Reload/切枪按键现阶段虽已映射，但行为为空（仅 Sprint/Slide/Crouch 有逻辑）。ADS 互斥(B7)需要 GA_ADS 能加 `State.Combat.ADS` tag 才能完整验证；阶段1可先验证 Sprint/Slide/Crouch。

---

## C. 通过 set_properties 已成功写入的项（无需人工）

- IMC 按键映射：**已成功写入 `IMC_Default.defaultKeyMappings.mappings`**（追加 9 条战斗/移动能力映射，保留原 Jump/Move/Look 12 条及其 Modifier 子对象）。无需人工配置。
- IC_TPSInput 能力映射数组、BP InputConfig/CrouchAction、敌人网格/变换：均已写入并验证读回。

---

## D. 需人工在编辑器复核/配置的项（受限或建议人工确认）

1. **IMC 写入机制的限制（已绕过，但需复核）**：UE5.6 的 `UInputMappingContext` 真实映射存放在 `defaultKeyMappings.mappings`（顶层 `mappings` 为空/弃用）。MCP `set_properties` 对该嵌套数组只能做“纯追加”——若同时修改已有元素会报 `ArrayAdd: insertion points are ambiguous` 并整体失败。本次通过“原 12 条逐字节保持不变 + 末尾追加 9 条”成功写入。**建议人工在 IMC_Default 编辑器中肉眼复核 21 条映射齐全、Move 的 Swizzle/Negate 修饰符未丢失。**
2. **输入屏显日志**：CP10a 证据要求 `AddOnScreenDebugMessage` 打印 tag；若 C++ handler 未实现屏显，需开发补充。
3. **showdebug abilitysystem 截图**：需 PIE 内控制台命令，MCP 无控制台注入工具，故由人工采集（B2、B7）。
4. **行为/手感证据**：B1、B4–B7 全部需人工 PIE 实操（MCP 不能注入按键）。

---

## E. 本阶段“资产搭建”完成度小结

- 已自动完成：9 个 InputAction、IMC_Default 21 条映射、IC_TPSInput 8 条能力映射、BP_TSPlayerCharacter(InputConfig+CrouchAction，已编译)、BP_TSEnemy(网格/动画/变换，已编译)、场景放置 1 个敌人、全部保存。
- 待人工：按 B 节采集行为类可视化证据，按 D 节复核 IMC 与补充屏显日志，方可由“集成”角色判定 CP-Phase1 放行。
