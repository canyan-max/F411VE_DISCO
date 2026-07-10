# I2S 音频输出修复总结

**日期**: 2026-07-11  
**问题**: CS43L22 寄存器配置全部正确，I2C 通信正常，但完全无声。  
**根因**: GPIO 硬件层两个问题 + DMA 播放策略问题，导致 I2S 时钟/数据信号无法到达 CS43L22。

---

## 有效改动

### 1. GPIO MODER：PC12 (I2S3_SD) 卡在 Input 模式 🔴 关键

**现象**: `GPIOC_MODER` bit25:24 = `00` (Input)，AFR 已正确配置为 AF6。  
**后果**: I2S3 数据线 (SD) 无法输出，CS43L22 收到时钟但收不到音频数据。  
**修复**: 用 `HAL_GPIO_Init()` API 重新配置 PC7/PC10/PC12，确保 MODER 切换到 AF 模式。

```c
GPIO_InitTypeDef g = {0};
g.Pin       = I2S3_MCK_Pin | I2S3_SCK_Pin | I2S3_SD_Pin;  // PC7|PC10|PC12
g.Mode      = GPIO_MODE_AF_PP;
g.Pull      = GPIO_NOPULL;
g.Speed     = GPIO_SPEED_FREQ_HIGH;
g.Alternate = GPIO_AF6_SPI3;
HAL_GPIO_Init(GPIOC, &g);
```

> 裸写 `GPIOC->MODER |= GPIO_MODER_MODER12_1` 对 PC12 不生效（bit25 始终为 0），改用 HAL API 后正常。

### 2. GPIO Speed：LOW → HIGH 🔴 关键

**现象**: `HAL_I2S_MspInit` 将 I2S3 引脚速度设为 `GPIO_SPEED_FREQ_LOW` (~2MHz)。  
**后果**: 
- MCK = 24.576 MHz (96k × 256) 远超 LOW 上限 → MCLK 信号严重衰减
- SCK = 6.144 MHz (96k × 64) 也超 LOW 上限 → 位时钟不可靠
- CS43L22 PLL 无法锁定 → `CLK_STATUS = 0x00`

**修复**: 将 PC7(MCK)、PC10(SCK)、PC12(SD) 速度提升至 `GPIO_SPEED_FREQ_HIGH` (~50MHz)。

### 3. DMA CIRCULAR 模式 🟡 重要

**现象**: DMA 配置为 `NORMAL` 模式，buffer 仅 32 samples。  
**后果**: 96kHz 下 32 samples = 0.33ms，人耳无法感知（听上去是静音）。  
**修复**: 切换为 `CIRCULAR` 模式，DMA 循环搬运，I2S 持续输出。

```c
hdma_spi3_tx.Init.Mode = DMA_CIRCULAR;
HAL_DMA_Init(&hdma_spi3_tx);
```

### 4. 测试音：3kHz → 1kHz，buffer 扩大 🟢 辅助

- 1kHz 比 3kHz 在人耳敏感区，小耳机更容易听到
- 96 samples @ 96kHz = 整整一个周期，循环播放

---

## 关键寄存器验证

| 寄存器 | 地址 | 修复前 | 修复后 |
|--------|------|--------|--------|
| GPIOC_MODER (PC12) | - | `00` (Input) | `10` (AF) |
| GPIOC_OSPEEDR (PC7/10/12) | - | `00` (LOW) | `10` (HIGH) |
| CS43L22 CLK_STATUS | 0x2E | `0x00` | `0x70` (PLL lock + SCLK + LRCK) |
| CS43L22 ID | 0x01 | `0xE3` ✓ | `0xE3` ✓ |

---

## 教训

1. **GPIO MODER ≠ AFR**：AFR 选对但 MODER 没切到 AF，外设信号出不去。这是最隐蔽的硬件层问题。
2. **GPIO Speed 不是摆设**：高频信号 (≥6MHz) 必须匹配足够的输出驱动速度，否则信号物理上无法正确翻转。
3. **DMA buffer 不能太短**：人耳时间分辨率 ~10ms，96kHz 下至少需要 960 samples 才能被感知为"声音"而非"咔嗒"。
4. **CLK_STATUS 要在播放中读**：DMA 停止后 I2S 时钟也停，此时读到的 `0x00` 是正常现象，不能据此判断硬件故障。
