/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2019-02-22
 * 
 * @copyright (c) 2019 Letter
 * 
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "shell.h"
// #include "serial.h"
#include "stm32h7xx_hal.h"
#include "usart.h"
// #include "cevent.h"
#include "log.h"


Shell shell;
char shellBuffer[512];

static SemaphoreHandle_t shellMutex;

// logs
#ifdef LOG_ENABLE
void uartLogWrite(char *buffer, short len);

Log uartLog = {
.write = uartLogWrite,
.active = LOG_ENABLE,
.level = LOG_DEBUG
};

void uartLogWrite(char *buffer, short len)
{
    if (uartLog.shell)
    {
        shellWriteEndLine(uartLog.shell, buffer, len);
    }
}
#endif



/**
 * @brief 用户shell写
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际写入的数据长度
 */
short userShellWrite(char *data, unsigned short len)
{
    // serialTransmit(&debugSerial, (uint8_t *)data, len, 0x1FF);
    
	HAL_UART_Transmit(&hlpuart1, (uint8_t *)data, len, 0x1FF);
    return len;
}


/**
 * @brief 用户shell读
 * 
 * @param data 数据
 * @param len 数据长度
 * 
 * @return short 实际读取到
 */
short userShellRead(char *data, unsigned short len)
{
	  // return serialReceive(&debugSerial, (uint8_t *)data, len, 0);
        if(HAL_UART_Receive(&hlpuart1, (uint8_t *)data, len, 0xFFFF) != HAL_OK)
		{
				return 0;
		}
        else
        {
				return 1;
		}  
}

/**
 * @brief 用户shell上锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellLock(Shell *shell)
{
     (void)shell;
    xSemaphoreTakeRecursive(shellMutex, portMAX_DELAY);
    return 0;
}

/**
 * @brief 用户shell解锁
 * 
 * @param shell shell
 * 
 * @return int 0
 */
int userShellUnlock(Shell *shell)
{
    (void)shell;
    xSemaphoreGiveRecursive(shellMutex);
    return 0;
}

/**
 * @brief 用户shell初始化
 * 
 */
void userShellInit(void)
{
    shellMutex = xSemaphoreCreateMutex();

    shell.write = userShellWrite;
    shell.read = userShellRead;
    shell.lock = userShellLock;
    shell.unlock = userShellUnlock;
    shellInit(&shell, shellBuffer, 512);
    if (xTaskCreate(shellTask, "shell", 256, &shell, 1, NULL) != pdPASS)
    {
        logError("shell task creat failed");
    }

#ifdef     LOG_ENABLE
    logRegister(&uartLog, &shell);
#endif

}
// CEVENT_EXPORT(EVENT_INIT_STAGE2, userShellInit);

void shellTest(int a, int b, int c)
{
    (void)a;
    (void)b;
    (void)c;
	shellPrint(&shell, "This is test\r\n");
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
					test, shellTest, This is test);

