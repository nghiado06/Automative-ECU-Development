/******************************************************************
 * @file    Std_Types.h
 * @author  Do Duc Nghia
 * @brief   File này dùng để cấu hình các kiểu dữ liệu, một số tính
 *          chất đặc thù theo tiêu chuẩn AUTOSAR
 * @version 1.0
 * @date    2025-06-22
 * ****************************************************************
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

/************************************************************
 *
 * ============== [ MACRO QUẢN LÝ PHIÊN BẢN ] ===============
 * ============ ( Software Version Information) =============
 *
 * @brief Quản lý phiên bản giúp các phiên bản match với nhau
 *        khi include giữa các module.
 *
 * STD_TYPES_SW_MAJOR_VERSION Phiên bản chính (Major Version)
 * STD_TYPES_SW_MINOR_VERSION Phiên bản phụ   (Minor Version)
 * STD_TYPES_SW_PATCH_VERSION Bản vá/lỗi      (Patch Version)
 *
 * Ví dụ như với trường hợp bên dưới là v1.0.0
 * **********************************************************
 */
#define STD_TYPES_SW_MAJOR_VERSION (1U)
#define STD_TYPES_SW_MINOR_VERSION (0U)
#define STD_TYPES_SW_PATCH_VERSION (0U)

/************************************************************
 *
 * ============ [ MACRO ĐỊNH NGHĨA MỨC LOGIC ] ===============
 * ============== ( Logic State Definitions ) ================
 *
 * @brief Định nghĩa lại các trạng thái logic high và low để
 *        sử dụng tiêu chuẩn chung
 *
 * STD_HIGH Mức logic high
 * STD_LOW  Mức logic low
 *
 * **********************************************************
 */
#define STD_HIGH 0x01U
#define STD_LOW 0x00U

/************************************************************
 *
 * =========== [ MACRO ĐỊNH NGHĨA CON TRỎ NULL ] ============
 *
 * @brief Định nghĩa lại trạng thái con trỏ NULL, tạo một tiêu
 *        chuẩn chung.
 *
 * **********************************************************
 */
#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif

/*************************************************************
 *
 * ====== [ MACRO ĐỊNH NGHĨA KIỂU DỮ LIỆU BOOLEAN ] =========
 *
 * @brief Định nghĩa lại kiểu dữ liệu boolean theo tiêu chuẩn
 *        chung
 *
 * ***********************************************************
 */
typedef unsigned char boolean;

#ifndef FALSE
#define FALSE ((boolean)0)
#endif
#ifndef TRUE
#define TRUE ((boolean)1)
#endif

/************************************************************
 *
 * ======= [ GENERAL PLATFORM TYPE DEFINITIONS ] ============
 *
 * @brief Định nghĩa lại các kiểu dữ liệu theo quy tắc chung

 *
 * **********************************************************
 */
typedef unsigned char uint8;       /* Số nguyên không dấu 8-bit: [0, 255 ]                */
typedef signed char sint8;         /* Số nguyên có dấu 8-bit: [-128, 127]                 */
typedef unsigned short uint16;     /* Số nguyên không dấu 16-bit: [0, 65535]              */
typedef signed short sint16;       /* Số nguyên có dấu 16-bit: [-32768, 32767]            */
typedef unsigned long uint32;      /* Số nguyên không dấu 32-bit: [0,4294967295]          */
typedef signed long sint32;        /* Số nguyên có dấu 32-bit: [-2147483648, +2147483647] */
typedef unsigned long long uint64; /* Số nguyên không dấu 64-bit                          */
typedef signed long long sint64;   /* Số nguyên không dấu 64-bit                          */
typedef float float32;             /* Số thực 32-bit                                      */
typedef double float64;            /* Số thực 64-bit                                      */

/************************************************************
 *
 * ============= [ VOLATILE TYPE DEFINITIONS ] ==============
 *
 * @brief Định nghĩa lại các kiểu dữ liệu kết hợp với từ khóa
 *        volatile
 *
 * **********************************************************
 */
typedef volatile uint8 vuint8;
typedef volatile sint8 vint8;
typedef volatile uint16 vuint16;
typedef volatile sint16 vsint16;
typedef volatile uint32 vuint32;
typedef volatile sint32 vsint32;
typedef volatile uint64 vuint64;
typedef volatile sint64 vsint64;

/************************************************************
 *
 * =============== [ STANDARD RETURN TYPE ] =================
 *
 * @typedef Std_ReturnType
 * @brief   Định nghĩa lại các error codes chuẩn theo AUTOSAR
 * @details Hai giá trị E_OK và E_NOT_OK dùng để báo kết quả
 *          thực thi của một hàm.
 *
 * **********************************************************
 */
typedef uint8 Std_ReturnType;

#define E_OK 0x00U
#define E_NOT_OK 0x01U

/************************************************************
 *
 * =========== [ VERSION INFORMATION STRUCTURE ] ============
 *
 * @typedef Std_VersionInfoType
 * @brief   Định nghĩa lại các error codes chuẩn theo AUTOSAR
 * @details Hai giá trị E_OK và E_NOT_OK dùng để báo kết quả
 *          thực thi của một hàm.
 *
 * **********************************************************
 */
typedef struct
{
    uint16 vendorID;        /* Mã định danh của nhà cung cấp (ST, NXP,..) của module đó                 */
    uint16 moduleID;        /* Mã định danh của module AUTOSAR đang sử dụng (Dio: 120, Port: 124,.. )   */
    uint8 sw_major_version; /* Phiên bản chính của phần mềm                                             */
    uint8 sw_minor_version; /* Phiên bản phụ của phần mềm                                               */
    uint8 sw_patch_version; /* Bản vá/lỗi của phần mềm                                                  */
} Std_VersionInfoType;

#endif