# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

基于 STM32F411VETx（Cortex-M4，512KB Flash，128KB SRAM）的嵌入式音频项目，目标板为 STM32F411VE Discovery。核心功能是通过 I2S3 + DMA 驱动板载 CS43L22 音频编解码器播放音频（当前为 1kHz 正弦波测试）。项目同时集成了 USB Host（CDC 类）和 minimp3 解码支持。

## 编译与烧录

所有命令在项目根目录下通过 **Git Bash** 执行（不支持 PowerShell/cmd）。

```bash
# 仅编译
./keil_build_flash.bash -b

# 仅烧录（需通过 J-Link/ST-Link 连接开发板）
./keil_build_flash.bash -f

# 编译后烧录（无参数时的默认行为）
./keil_build_flash.bash
# 或
./keil_build_flash.bash -a
```

编译产物：`MDK-ARM/F411VE_DISCO/F411VE_DISCO.axf` 和 `.hex`  
日志文件：项目根目录下的 `build_log.txt` 和 `flash_log.txt`

**路径依赖：** `keil_build_flash.bash` 中的 `UV` 变量指向 `UV4.exe`，换机器时需要更新（脚本中已注释了家用和公司两套路径）。

## VS Code / clangd 配置

安装 Keil 后或换新机器时，重新生成 `compile_commands.json`：

```bash
./uv2clangd.bash
```

该脚本运行内置的 `uv2clangd.exe`，然后将 ARMCLANG 工具链路径和目标三元组注入 `.vscode/compile_commands.json`。若 Keil 安装路径不同，需更新脚本中的 `ARMCLANG_BIN` 和 `ARMCLANG_INC`。

## 工具链

- **IDE：** Keil MDK-ARM v6.22
- **编译器：** ARM ARMCLANG 6.22（基于 LLVM，`-std=c99 -O2 -W3`）
- **宏定义：** `USE_HAL_DRIVER`、`STM32F411xE`
- **外设配置工具：** STM32CubeMX（`.ioc` 文件）

## 代码架构

```
Core/Src/           STM32CubeMX 生成的外设初始化代码 + 用户应用（main.c）
Bsp/cs43l22/        CS43L22 BSP 驱动（编解码器初始化、音量、静音、播放）
Platform/HAL_ST/    HAL 操作表（cs43lxxx_hal_ops_t），将 BSP 与 STM32 HAL 对接
Drivers/            STM32 HAL + CMSIS（不要手动修改）
Middlewares/
  EasyLog/          轻量级日志库（elog_* API，基于 tag，输出到 USART1）
  MP3/              minimp3（纯头文件；只在一个 .c 文件中定义 MINIMP3_IMPLEMENTATION）
  ST/STM32_USB_Host_Library/  USB Host 协议栈
MDK-ARM/            Keil 工程 + startup_stm32f411xe.s（栈 26.5KB，堆 512B）
```

### 音频子系统

控制路径：`main.c` → `bsp_cs43lxxx_drv.c` → `cs43lxxx_hal.c` → STM32 HAL I2C1  
数据路径：I2S3（PA4/PC10/PC12/PC7-MCK）→ DMA1 Stream5（SPI3_TX）→ CS43L22  
I2C 地址：`0x94`（8 位）。编解码器复位引脚：PD4。

`cs43lxxx_hal_ops_t` 结构体（`Platform/HAL_ST/` 中的 `g_cs43lxxx_hal_ops`）是可移植 BSP 驱动与 STM32 HAL 之间的接缝，需在调用 `bsp_cs43lxxx_init()` 时传入。

### CubeMX 生成文件

`Core/Src/` 中带有 `MX_` 前缀函数和 `/* USER CODE BEGIN/END */` 注释块的文件由 CubeMX 自动生成。只能在 `USER CODE` 块内编写代码，块外的内容在重新生成时会被覆盖。

### 调试日志

使用 EasyLog 库。在 `main.c` 中通过 `elog_init_handler()` 初始化。每个文件定义自己的 tag 宏：
```c
#define MAIN_TAG "main"
log_d(MAIN_TAG, "message %d", val);
```
日志输出至 USART1（PA15 TX，PB7 RX）。

## 代码风格

由 `.clang-format` 强制约束（基于 LLVM 风格），关键规则：
- 4 空格缩进，不使用 Tab
- Allman 风格大括号（`{` 独占一行）
- 80 列限制
- 指针符号靠右：`int *p`
- `SortIncludes: Never`——嵌入式项目头文件顺序敏感，不自动排序
- 禁止将短函数/if/循环合并为单行
- 全局变量命名方式如：uint8_t g_xxxx; 需在变量名面前加g
- 指针类型需要加p开头，函数指针需要加pf开头
- 例如：uint8_t *p_xxx; 
- 例如：cs43lxxx_status_t (*pf_i2s_transmit_with_dma)(uint16_t *p_buffer,
                                                 uint16_t  size);

