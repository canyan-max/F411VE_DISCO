# bsp_cs43lxxx_drv.c 改动说明

**日期**: 2026-07-11

---

## 总览

这个文件从原来只有一个 `read_id` 函数 + 空壳 `instruct`，补全为完整的 CS43L22 驱动层。改动对齐 ST 官方 BSP（`cs43l22.c`）。

---

## 1. 新增宏定义

```c
#define VOLUME_CONVERT(Volume)  (((Volume) > 100) ? 255 : ((uint8_t)(((Volume) * 255) / 100)))
#define MUTE_DISABLE  (0x00U)
#define MUTE_ENABLE   (0x01U)
```

---

## 2. 新增 self-operation 函数（6个）

### 2.1 `cs43lxxx_init` — 13步完整初始化序列

原先只有 step1（写 `POWER_CTL1=0x01`）。现在完整流程：

| Step | 寄存器 | 值 | 说明 |
|------|--------|-----|------|
| 0 | — | — | Power-on reset（RST 脚拉低→延时→拉高） |
| 1 | POWER_CTL1 (0x02) | `0x01` | 退出掉电 |
| 2 | POWER_CTL2 (0x04) | 见下表 | **设备相关**，之前固定写错 |
| 3 | CLOCKING_CTL (0x05) | `0x81` | Auto-detect + Double-speed，之前 `0x80` |
| 4 | INTERFACE_CTL1 (0x06) | `0x04` | I2S, 16-bit, slave |
| 4a | INTERFACE_CTL2 (0x07) | `0x00` | SCLK 正常极性，**新增** |
| 4b | PASSTHR_A/B (0x08/0x09) | `0x00` | 关闭模拟直通，走 I2S→DAC，**新增** |
| 5 | MASTER_A/B_VOL | volume | 设置音量 |
| 5a | PASSTHR_GANG_CTL (0x0A) | `0x00` | 独立 L/R，**新增** |
| 5b | PLAYBACK_CTL1 (0x0D) | `0x00` | 取消静音，**新增** |
| 6 | PLAYBACK_CTL2 (0x0F) | `0x06` | 播放使能 |
| 7 | SPEAKER_A/B_VOL | `0x00` | 0dB |
| 8 | ANALOG_ZC_SR_SETT (0x1B) | `0x00` | 零交叉设置 |
| 9 | MISC_CTL (0x0E) | `0x04` | 初始值 |
| 10 | LIMIT_CTL1 (0x1E) | `0x00` | 限幅关 |
| 11 | TONE_CTL (0x1F) | `0x0F` | 高低音 0dB |
| 12 | PCMA_VOL / PCMB_VOL | `0x00` | PCM 0dB，**新增** |
| 13 | CHARGE_PUMP_FREQ (0x34) | `0x05` | **使能电荷泵，耳机输出必需！** **新增** |

**POWER_CTL2 设备映射**（之前固定写错值）：

| 设备 | 值 | 含义 |
|------|-----|------|
| SPEAKER | `0xFA` | SPK always ON, HP always OFF |
| HEADPHONE | `0xAF` | SPK always OFF, HP always ON |
| BOTH | `0xAA` | Both always ON |
| AUTO | `0x05` | Auto-detect |

### 2.2 `cs43lxxx_read_id`

原来就有，加了失败时 `*p_id = 0xa5` 标记。

### 2.3 `cs43lxxx_set_volume`

音量百分比→寄存器值映射，写 MASTER_A_VOL + MASTER_B_VOL。

### 2.4 `cs43lxxx_set_mute`

静音：POWER_CTL2=`0xFF`（全关），HP_VOL=`0x01`（衰减）。

### 2.5 `cs43lxxx_set_out` 🔴 关键

取消静音，对齐 ST BSP `SetMute(OFF)`：
1. HP_A_VOL / HP_B_VOL = `0x00`（取消静音，0dB）
2. **按设备恢复 POWER_CTL2 值**（之前漏掉了这一步！）

### 2.6 `cs43lxxx_play` 🔴 关键

播放路径激活，对齐 ST BSP：
1. MISC_CTL = `0x06`（使能数字软斜坡）— 之前误写为 `0x02`
2. 调用 `cs43lxxx_set_out`（取消静音+恢复 POWER_CTL2）
3. POWER_CTL1 = `0x9E`（启动 DAC + 耳机输出）

---

## 3. `cs43lxxx_instruct` 重构

### 参数变化
```c
// 之前
cs43lxxx_instruct(p_drv, p_hal_ops, i2c_id)

// 现在
cs43lxxx_instruct(p_drv, p_hal_ops, i2c_id, out_put_dev)  // 多了 out_put_dev
```

### 函数指针挂载
```c
p_drv->pf_init       = cs43lxxx_init;
p_drv->pf_read_id    = cs43lxxx_read_id;
p_drv->pf_set_volume = cs43lxxx_set_volume;
p_drv->pf_set_mute   = cs43lxxx_set_mute;
p_drv->pf_set_out    = cs43lxxx_set_out;
p_drv->pf_play       = cs43lxxx_play;
```

### 初始化 + 失败处理
```c
cs43lxxx_status_t ret = p_drv->pf_init(p_drv, 70);  // 默认音量 70%
if (ret != OK) {
    // 所有函数指针置 NULL
    p_drv->pf_init = p_drv->pf_read_id = ... = NULL;
    p_drv->is_init = CS43XXX_NOT_INIT;
    return ret;
}
p_drv->is_init = CS43XXX_IS_INIT;
```

---

## 4. 与 ST 官方 BSP 对齐的寄存器值变更

| 寄存器 | 地址 | 旧值 | 新值 | 原因 |
|--------|------|------|------|------|
| POWER_CTL2 | 0x04 | 固定 `0x01` | 设备相关 | 不同输出设备需要不同上电配置 |
| CLOCKING_CTL | 0x05 | `0x80` | `0x81` | ST 用 auto-detect + double-speed |
| MISC_CTL | 0x0E | `0x02`(play时) | `0x06` | bit1=数字软斜坡使能（ST 原值） |
| CHARGE_PUMP_FREQ | 0x34 | 未配置 | `0x05` | 耳机输出依赖电荷泵供电 |
| PCM_VOL | 0x1A/0x1C | 未配置 | `0x00` | 0dB 不过衰减 |
| INTERFACE_CTL2 | 0x07 | 未配置 | `0x00` | SCLK 极性 |
| PASSTHR_A/B | 0x08/0x09 | 未配置 | `0x00` | 选 I2S→DAC 路径而非模拟直通 |
| PLAYBACK_CTL1 | 0x0D | 未配置 | `0x00` | 取消播放静音 |
