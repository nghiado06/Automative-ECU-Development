/*******************************************************************************
 * @file    Dio_Cfg.h
 * @author  Do Duc Nghia
 * @brief   Digital I/O Driver Config Header File
 * @details File này cấu hình các port, channel của dòng vi điều khiển STM32F1x
 *
 * @version 1.0
 * @date    2025-06-23
 * *****************************************************************************
 */

#ifndef DIO_CFG_H
#define DIO_CFG_H

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include "Dio.h"
#include <stm32f10x_gpio.h>

/********************************************************************
 * ===================== [ PORT DEFINITIONS ] =======================
 * @brief   Cấu hình lại các port theo thứ tự 0 đến 3
 ********************************************************************/

#define DIO_PORT_A 0
#define DIO_PORT_B 1
#define DIO_PORT_C 2
#define DIO_PORT_D 3

/********************************************************************
 * =================== [ CHANNEL DEFINITIONS ] ======================
 * @brief   Cấu hình các channel theo thứ tự từ 0 trở đi
 ********************************************************************/

#define DIO_CHANNEL(PORTx, Pin) (((PORTx) << 4) + (Pin))

#define DIO_CHANNEL_A0 DIO_CHANNEL(DIO_PORT_A, 0)
#define DIO_CHANNEL_A1 DIO_CHANNEL(DIO_PORT_A, 1)
#define DIO_CHANNEL_A2 DIO_CHANNEL(DIO_PORT_A, 2)
#define DIO_CHANNEL_A3 DIO_CHANNEL(DIO_PORT_A, 3)
#define DIO_CHANNEL_A4 DIO_CHANNEL(DIO_PORT_A, 4)
#define DIO_CHANNEL_A5 DIO_CHANNEL(DIO_PORT_A, 5)
#define DIO_CHANNEL_A6 DIO_CHANNEL(DIO_PORT_A, 6)
#define DIO_CHANNEL_A7 DIO_CHANNEL(DIO_PORT_A, 7)
#define DIO_CHANNEL_A8 DIO_CHANNEL(DIO_PORT_A, 8)
#define DIO_CHANNEL_A9 DIO_CHANNEL(DIO_PORT_A, 9)
#define DIO_CHANNEL_A10 DIO_CHANNEL(DIO_PORT_A, 10)
#define DIO_CHANNEL_A11 DIO_CHANNEL(DIO_PORT_A, 11)
#define DIO_CHANNEL_A12 DIO_CHANNEL(DIO_PORT_A, 12)
#define DIO_CHANNEL_A13 DIO_CHANNEL(DIO_PORT_A, 13)
#define DIO_CHANNEL_A14 DIO_CHANNEL(DIO_PORT_A, 14)
#define DIO_CHANNEL_A15 DIO_CHANNEL(DIO_PORT_A, 15)

#define DIO_CHANNEL_B0 DIO_CHANNEL(DIO_PORT_B, 0)
#define DIO_CHANNEL_B1 DIO_CHANNEL(DIO_PORT_B, 1)
#define DIO_CHANNEL_B2 DIO_CHANNEL(DIO_PORT_B, 2)
#define DIO_CHANNEL_B3 DIO_CHANNEL(DIO_PORT_B, 3)
#define DIO_CHANNEL_B4 DIO_CHANNEL(DIO_PORT_B, 4)
#define DIO_CHANNEL_B5 DIO_CHANNEL(DIO_PORT_B, 5)
#define DIO_CHANNEL_B6 DIO_CHANNEL(DIO_PORT_B, 6)
#define DIO_CHANNEL_B7 DIO_CHANNEL(DIO_PORT_B, 7)
#define DIO_CHANNEL_B8 DIO_CHANNEL(DIO_PORT_B, 8)
#define DIO_CHANNEL_B9 DIO_CHANNEL(DIO_PORT_B, 9)
#define DIO_CHANNEL_B10 DIO_CHANNEL(DIO_PORT_B, 10)
#define DIO_CHANNEL_B11 DIO_CHANNEL(DIO_PORT_B, 11)
#define DIO_CHANNEL_B12 DIO_CHANNEL(DIO_PORT_B, 12)
#define DIO_CHANNEL_B13 DIO_CHANNEL(DIO_PORT_B, 13)
#define DIO_CHANNEL_B14 DIO_CHANNEL(DIO_PORT_B, 14)
#define DIO_CHANNEL_B15 DIO_CHANNEL(DIO_PORT_B, 15)

#define DIO_CHANNEL_C0 DIO_CHANNEL(DIO_PORT_C, 0)
#define DIO_CHANNEL_C1 DIO_CHANNEL(DIO_PORT_C, 1)
#define DIO_CHANNEL_C2 DIO_CHANNEL(DIO_PORT_C, 2)
#define DIO_CHANNEL_C3 DIO_CHANNEL(DIO_PORT_C, 3)
#define DIO_CHANNEL_C4 DIO_CHANNEL(DIO_PORT_C, 4)
#define DIO_CHANNEL_C5 DIO_CHANNEL(DIO_PORT_C, 5)
#define DIO_CHANNEL_C6 DIO_CHANNEL(DIO_PORT_C, 6)
#define DIO_CHANNEL_C7 DIO_CHANNEL(DIO_PORT_C, 7)
#define DIO_CHANNEL_C8 DIO_CHANNEL(DIO_PORT_C, 8)
#define DIO_CHANNEL_C9 DIO_CHANNEL(DIO_PORT_C, 9)
#define DIO_CHANNEL_C10 DIO_CHANNEL(DIO_PORT_C, 10)
#define DIO_CHANNEL_C11 DIO_CHANNEL(DIO_PORT_C, 11)
#define DIO_CHANNEL_C12 DIO_CHANNEL(DIO_PORT_C, 12)
#define DIO_CHANNEL_C13 DIO_CHANNEL(DIO_PORT_C, 13)
#define DIO_CHANNEL_C14 DIO_CHANNEL(DIO_PORT_C, 14)
#define DIO_CHANNEL_C15 DIO_CHANNEL(DIO_PORT_C, 15)

#define DIO_CHANNEL_D0 DIO_CHANNEL(DIO_PORT_D, 0)
#define DIO_CHANNEL_D1 DIO_CHANNEL(DIO_PORT_D, 1)
#define DIO_CHANNEL_D2 DIO_CHANNEL(DIO_PORT_D, 2)
#define DIO_CHANNEL_D3 DIO_CHANNEL(DIO_PORT_D, 3)
#define DIO_CHANNEL_D4 DIO_CHANNEL(DIO_PORT_D, 4)
#define DIO_CHANNEL_D5 DIO_CHANNEL(DIO_PORT_D, 5)
#define DIO_CHANNEL_D6 DIO_CHANNEL(DIO_PORT_D, 6)
#define DIO_CHANNEL_D7 DIO_CHANNEL(DIO_PORT_D, 7)
#define DIO_CHANNEL_D8 DIO_CHANNEL(DIO_PORT_D, 8)
#define DIO_CHANNEL_D9 DIO_CHANNEL(DIO_PORT_D, 9)
#define DIO_CHANNEL_D10 DIO_CHANNEL(DIO_PORT_D, 10)
#define DIO_CHANNEL_D11 DIO_CHANNEL(DIO_PORT_D, 11)
#define DIO_CHANNEL_D12 DIO_CHANNEL(DIO_PORT_D, 12)
#define DIO_CHANNEL_D13 DIO_CHANNEL(DIO_PORT_D, 13)
#define DIO_CHANNEL_D14 DIO_CHANNEL(DIO_PORT_D, 14)
#define DIO_CHANNEL_D15 DIO_CHANNEL(DIO_PORT_D, 15)

/********************************************************************
 * @brief   Định nghĩa macro dùng để xác định port dựa trên channel
 ********************************************************************/
#define DIO_GET_PORT(ChannelId) ((ChannelId) < 16 ? GPIOA : (ChannelId) < 32 ? GPIOB \
                                                        : (ChannelId) < 48   ? GPIOC \
                                                        : (ChannelId) < 54   ? GPIOD \
                                                                             : NULL_PTR)

/********************************************************************
 * @brief   Định nghĩa macro dùng để xác định mask của pin dựa trên
 *          channel
 ********************************************************************/
#define DIO_GET_PIN(ChannelId) (1 << ((ChannelId) % 16))

/********************************************************************
 * @brief   Định nghĩa macro dùng để xác định port dựa trên PortId
 ********************************************************************/
#define DIO_PORT(PortId) ((PortId == DIO_PORT_A) ? GPIOA : (PortId == DIO_PORT_B) ? GPIOB \
                                                       : (PortId == DIO_PORT_C)   ? GPIOC \
                                                                                  : GPIOD)

#endif