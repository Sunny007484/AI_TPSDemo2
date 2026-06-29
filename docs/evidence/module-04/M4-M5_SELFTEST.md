# 模块 4/5 自测说明

> 编译状态：**Development Editor 0 error**（2026-06-29）
> 证据目录：`docs/evidence/module-04/`、`docs/evidence/module-05/`

## 模块 4 (CP4) 验收步骤

1. 打开 `Lvl_ThirdPerson`，PIE 运行（GameMode 使用 `BP_TSPlayerCharacter`）。
2. **CP4-1 持枪**：越肩视角应看到右手 `hand_r` 附着 Cube 占位武器网格 → 截图 `CP4-A_equip_ar.png`。
3. **CP4-2 双槽**：屏幕左上 Combat Debug 显示 `Current Slot: 0 | AssaultRifle`，弹药 `30 / Reserve: 180`。
4. **CP4-3 切枪**：按 `2` 切换到 Pistol，再按 `1` 切回 → `CP4-B_switch_ar.png` / `CP4-B_switch_pistol.png`。
5. **CP4-4 弹药**：Pistol 槽显示 `12 / 60` → `CP4-D_ammo.png`。
6. **CP4-5 Muzzle**：橙色 Debug Sphere 位于枪管前端 → `CP4-C_muzzle.png`。

## 模块 5 (CP5) 验收步骤

1. **CP5-1 射速**：AR 按住左键连发（0.10s/发）；Pistol 单击单发 → `CP5-A_firerate.mp4`。
2. **CP5-2 弹道**：开火时可见红/绿 Debug 射线 → 与准心对齐。
3. **CP5-3 ADS**：右键 0.15s 内 FOV 90→55 → `CP5-B_ads_f1..f4.png`。
4. **CP5-4 散布**：同一墙面腰射 10 发 vs ADS 10 发 → `CP5-C_spread_hip.png` / `CP5-C_spread_ads.png`。
5. **CP5-5 换弹**：R 键 2.3s 后弹匣回满，换弹中无法开火 → `CP5-D_reload.mp4`。
6. **CP5-6 切枪**：换弹中按 1/2 打断换弹。
7. **CP5-7 伤害**：对 `BP_TSEnemy` 开火，`showdebug abilitysystem` Health 100→72（AR Damage=28）→ `CP5-E_damage_before.png` / `CP5-E_damage_after.png`。

## 已创建 Content 资产

- `/Game/TPS/Weapons/DA_Weapon_AR`
- `/Game/TPS/Weapons/DA_Weapon_Pistol`

可在 `BP_TSPlayerCharacter` → DefaultWeapons 中引用上述 DataAsset（可选；未配置时使用 C++ 运行时默认数据）。

## 操作键位

| 输入 | 功能 |
|------|------|
| 鼠标左键 | 开火 |
| 鼠标右键 | ADS |
| R | 换弹 |
| 1 / 2 / 滚轮 | 切枪 |
