#ifndef __V7_DEV_SD_CMD_H
#define __V7_DEV_SD_CMD_H

#include "../../h/types.h"

#define SD_APP_CMD_MASK                 (0x20000000)
#define SD_CMD_RSP_TYPE_MASK            (0x1e000000)
#define SD_CMD_RSP_TYPE_SHIFT           (25)
#define SD_CMD_META_MASK                ((SD_APP_CMD_MASK)|(SD_CMD_RSP_TYPE_MASK))
#define SD_CMD_CMD_INDEX_MASK           (0x3f)

#define SD_CMD(n, r)                    ((n) | (r))
#define SD_ACMD(n, r)                   ((SD_APP_CMD_MASK)|(SD_CMD((n),(r))))

#define IS_SD_ACMD(n)                   (((n) & (SD_APP_CMD_MASK)) == (SD_APP_CMD_MASK))
#define SD_CMD_GET_RSP_TYPE(n)          (((n) & (SD_CMD_RSP_TYPE_MASK)) >> (SD_CMD_RSP_TYPE_SHIFT))
#define SD_CMD_GET_CMD_INDEX(n)         ((n) & (SD_CMD_CMD_INDEX_MASK))
#define SD_CMD_IS_VALID(n)              (!((n) & ~((SD_CMD_META_MASK)|(SD_CMD_CMD_INDEX_MASK))))

#define SD_RESP_TYPE_NONE               (0x0 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R1                 (0x1 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R1B                (0x2 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R2                 (0x3 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R3                 (0x4 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R4                 (0x5 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R5                 (0x6 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R5B                (0x7 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R6                 (0x8 << (SD_CMD_RSP_TYPE_SHIFT))
#define SD_RESP_TYPE_R7                 (0x9 << (SD_CMD_RSP_TYPE_SHIFT))

#define SD_CMD_NUM_GO_IDLE_STATE 0
/* 1 is reserved */
#define SD_CMD_NUM_ALL_SEND_CID 2
#define SD_CMD_NUM_SEND_RELATIVE_ADDR 3
#define SD_CMD_NUM_SET_DSR 4
/* 5 is reserved for I/O cards (refer to "SDIO Card Specification") */
#define SD_CMD_NUM_SWITCH_FUNC 6
#define SD_CMD_NUM_SELECT_CARD 7
#define SD_CMD_NUM_SEND_IF_COND 8
#define SD_CMD_NUM_SEND_CSD 9
#define SD_CMD_NUM_SEND_CID 10
#define SD_CMD_NUM_VOLTAGE_SWITCH 11
#define SD_CMD_NUM_STOP_TRANSMISSION 12
#define SD_CMD_NUM_SEND_STATUS 13
/* 14 is Reserved */
#define SD_CMD_NUM_GO_INACTIVE_STATE 15
#define SD_CMD_NUM_SET_BLOCKLEN 16
#define SD_CMD_NUM_READ_SINGLE_BLOCK 17
#define SD_CMD_NUM_READ_MULTIPLE_BLOCK 18
#define SD_CMD_NUM_SEND_TUNING_BLOCK 19
#define SD_CMD_NUM_SPEED_CLASS_CONTROL 20
/* 21 is Reserved for DPS Specification */
#define SD_CMD_NUM_ADDRESS_EXTENSION 22
#define SD_CMD_NUM_SET_BLOCK_COUNT 23
#define SD_CMD_NUM_WRITE_BLOCK 24
#define SD_CMD_NUM_WRITE_MULTIPLE_BLOCK 25
/* 26 is Reserved For Manufacturer */
#define SD_CMD_NUM_PROGRAM_CSD 27
#define SD_CMD_NUM_SET_WRITE_PROT 28
#define SD_CMD_NUM_CLR_WRITE_PROT 29
#define SD_CMD_NUM_SEND_WRITE_PROT 30
/* 31 is Reserved */
#define SD_CMD_NUM_ERASE_WR_BLK_START 32
#define SD_CMD_NUM_ERASE_WR_BLK_END 33
/* 34-37 are reserved for command systems set by switch function
         which is in each command system specification */
#define SD_CMD_NUM_ERASE 38
/* 39 is Reserved */
/* 40 is Defined by DPS Spec. */
/* 41 is Reserved */
#define SD_CMD_NUM_LOCK_UNLOCK 42
#define SD_CMD_NUM_Q_MANAGEMENT 43
#define SD_CMD_NUM_Q_TASK_INFO_A 44
#define SD_CMD_NUM_Q_TASK_INFO_B 45
#define SD_CMD_NUM_Q_RD_TASK 46
#define SD_CMD_NUM_Q_WR_TASK 47
#define SD_CMD_NUM_READ_EXTR_SINGLE 48
#define SD_CMD_NUM_WRITE_EXTR_SINGLE 49
/* 50    are reserved for command systems set by switch function
         which is in each command system specification */
/* 51 is Reserved */
/* 52-54 Commands for SDIO (refer to "SDIO Card Specification") */
#define SD_CMD_NUM_APP_CMD 55
#define SD_CMD_NUM_GEN_CMD 56
/* 57    are reserved for command systems set by switch function
         which is in each command system specification */
#define SD_CMD_NUM_READ_EXTR_MULTI 58
#define SD_CMD_NUM_WRITE_EXTR_MULTI 59
/* 60-63 are reserved for manufacturer */

/* application commands */
/* 0 is not specified */
/* 1-5 are Reserved */
#define SD_ACMD_NUM_SET_BUS_WIDTH 6
/* 7-12 are Reserved */
#define SD_ACMD_NUM_SD_STATUS 13
/* 14-16 are Reserved for DPS Specification */
/* 17 is Reserved */
/* 18 is Reserved for SD security applications */
/* 19-21 are Reserved */
#define SD_ACMD_NUM_SEND_NUM_WR_BLOCKS 22
#define SD_ACMD_NUM_SET_WR_BLK_ERASE_COUNT 23
/* 24 is Reserved */
/* 25 is Reserved for SD security applications */
/* 26 is Reserved for SD security applications */
/* 27 is Shall not use this command */
/* 28 is Reserved for DPS Specification */
/* 29 is Reserved */
/* 30-35 are Reserved for Security Specification */
/* 36-37 are Reserved */
/* 38 is Reserved for SD security applications */
/* 39-40 are Reserved */
#define SD_ACMD_NUM_SD_SEND_OP_COND 41
#define SD_ACMD_NUM_SET_CLR_CARD_DETECT 42
/* 43-49 are Reserved for SD security applications */
/* 50 is not described */
#define SD_ACMD_NUM_SEND_SCR 51
/* 52-54 are Reserved for Security Specification */
/* ACMD55 does not exist (same as CMD55) */
/* 56-59 Reserved for Security Specification */
/* 60-63 are not specified */

#define SD_CMD0   SD_CMD(SD_CMD_NUM_GO_IDLE_STATE,            SD_RESP_TYPE_NONE)
#define SD_CMD2   SD_CMD(SD_CMD_NUM_ALL_SEND_CID,             SD_RESP_TYPE_R2)
#define SD_CMD3   SD_CMD(SD_CMD_NUM_SEND_RELATIVE_ADDR,       SD_RESP_TYPE_R6)
#define SD_CMD4   SD_CMD(SD_CMD_NUM_SET_DSR,                  SD_RESP_TYPE_NONE)
#define SD_CMD6   SD_CMD(SD_CMD_NUM_SWITCH_FUNC,              SD_RESP_TYPE_R1)
#define SD_CMD7   SD_CMD(SD_CMD_NUM_SELECT_CARD,              SD_RESP_TYPE_R1B)
#define SD_CMD8   SD_CMD(SD_CMD_NUM_SEND_IF_COND,             SD_RESP_TYPE_R7)
#define SD_CMD9   SD_CMD(SD_CMD_NUM_SEND_CSD,                 SD_RESP_TYPE_R2)
#define SD_CMD10  SD_CMD(SD_CMD_NUM_SEND_CID,                 SD_RESP_TYPE_R2)
#define SD_CMD11  SD_CMD(SD_CMD_NUM_VOLTAGE_SWITCH,           SD_RESP_TYPE_R1)
#define SD_CMD12  SD_CMD(SD_CMD_NUM_STOP_TRANSMISSION,        SD_RESP_TYPE_R1B)
#define SD_CMD13  SD_CMD(SD_CMD_NUM_SEND_STATUS,              SD_RESP_TYPE_R1)
#define SD_CMD15  SD_CMD(SD_CMD_NUM_GO_INACTIVE_STATE,        SD_RESP_TYPE_NONE)
#define SD_CMD16  SD_CMD(SD_CMD_NUM_SET_BLOCKLEN,             SD_RESP_TYPE_R1)
#define SD_CMD17  SD_CMD(SD_CMD_NUM_READ_SINGLE_BLOCK,        SD_RESP_TYPE_R1)
#define SD_CMD18  SD_CMD(SD_CMD_NUM_READ_MULTIPLE_BLOCK,      SD_RESP_TYPE_R1)
#define SD_CMD19  SD_CMD(SD_CMD_NUM_SEND_TUNING_BLOCK,        SD_RESP_TYPE_R1)
#define SD_CMD20  SD_CMD(SD_CMD_NUM_SPEED_CLASS_CONTROL,      SD_RESP_TYPE_R1B)
#define SD_CMD22  SD_CMD(SD_CMD_NUM_ADDRESS_EXTENSION,        SD_RESP_TYPE_R1)
#define SD_CMD23  SD_CMD(SD_CMD_NUM_SET_BLOCK_COUNT,          SD_RESP_TYPE_R1)
#define SD_CMD24  SD_CMD(SD_CMD_NUM_WRITE_BLOCK,              SD_RESP_TYPE_R1)
#define SD_CMD25  SD_CMD(SD_CMD_NUM_WRITE_MULTIPLE_BLOCK,     SD_RESP_TYPE_R1)
#define SD_CMD27  SD_CMD(SD_CMD_NUM_PROGRAM_CSD,              SD_RESP_TYPE_R1)
#define SD_CMD28  SD_CMD(SD_CMD_NUM_SET_WRITE_PROT,           SD_RESP_TYPE_R1B)
#define SD_CMD29  SD_CMD(SD_CMD_NUM_CLR_WRITE_PROT,           SD_RESP_TYPE_R1B)
#define SD_CMD30  SD_CMD(SD_CMD_NUM_SEND_WRITE_PROT,          SD_RESP_TYPE_R1)
#define SD_CMD32  SD_CMD(SD_CMD_NUM_ERASE_WR_BLK_START,       SD_RESP_TYPE_R1)
#define SD_CMD33  SD_CMD(SD_CMD_NUM_ERASE_WR_BLK_END,         SD_RESP_TYPE_R1)
#define SD_CMD38  SD_CMD(SD_CMD_NUM_ERASE,                    SD_RESP_TYPE_R1B)
#define SD_CMD42  SD_CMD(SD_CMD_NUM_LOCK_UNLOCK,              SD_RESP_TYPE_R1)
#define SD_CMD43  SD_CMD(SD_CMD_NUM_Q_MANAGEMENT,             SD_RESP_TYPE_R1B)
#define SD_CMD44  SD_CMD(SD_CMD_NUM_Q_TASK_INFO_A,            SD_RESP_TYPE_R1)
#define SD_CMD45  SD_CMD(SD_CMD_NUM_Q_TASK_INFO_B,            SD_RESP_TYPE_R1)
#define SD_CMD46  SD_CMD(SD_CMD_NUM_Q_RD_TASK,                SD_RESP_TYPE_R1)
#define SD_CMD47  SD_CMD(SD_CMD_NUM_Q_WR_TASK,                SD_RESP_TYPE_R1)
#define SD_CMD48  SD_CMD(SD_CMD_NUM_READ_EXTR_SINGLE,         SD_RESP_TYPE_R1)
#define SD_CMD49  SD_CMD(SD_CMD_NUM_WRITE_EXTR_SINGLE,        SD_RESP_TYPE_R1)
#define SD_CMD55  SD_CMD(SD_CMD_NUM_APP_CMD,                  SD_RESP_TYPE_R1)
#define SD_CMD56  SD_CMD(SD_CMD_NUM_GEN_CMD,                  SD_RESP_TYPE_R1)
#define SD_CMD58  SD_CMD(SD_CMD_NUM_READ_EXTR_MULTI,          SD_RESP_TYPE_R1)
#define SD_CMD59  SD_CMD(SD_CMD_NUM_WRITE_EXTR_MULTI,         SD_RESP_TYPE_R1)

#define SD_ACMD6  SD_ACMD(SD_ACMD_NUM_SET_BUS_WIDTH,          SD_RESP_TYPE_R1)
#define SD_ACMD13 SD_ACMD(SD_ACMD_NUM_SD_STATUS,              SD_RESP_TYPE_R1)
#define SD_ACMD22 SD_ACMD(SD_ACMD_NUM_SEND_NUM_WR_BLOCKS,     SD_RESP_TYPE_R1)
#define SD_ACMD23 SD_ACMD(SD_ACMD_NUM_SET_WR_BLK_ERASE_COUNT, SD_RESP_TYPE_R1)
#define SD_ACMD41 SD_ACMD(SD_ACMD_NUM_SD_SEND_OP_COND,        SD_RESP_TYPE_R3)
#define SD_ACMD42 SD_ACMD(SD_ACMD_NUM_SET_CLR_CARD_DETECT,    SD_RESP_TYPE_R1)
#define SD_ACMD51 SD_ACMD(SD_ACMD_NUM_SEND_SCR,               SD_RESP_TYPE_R1)
#define SD_ACMD55 (SD_CMD55)

#define SD_CMD_GO_IDLE_STATE (SD_CMD0)
#define SD_CMD_ALL_SEND_CID (SD_CMD2)
#define SD_CMD_SEND_RELATIVE_ADDR (SD_CMD3)
#define SD_CMD_SET_DSR (SD_CMD4)
#define SD_CMD_SWITCH_FUNC (SD_CMD6)
#define SD_CMD_SELECT_CARD (SD_CMD7)
#define SD_CMD_SEND_IF_COND (SD_CMD8)
#define SD_CMD_SEND_CSD (SD_CMD9)
#define SD_CMD_SEND_CID (SD_CMD10)
#define SD_CMD_VOLTAGE_SWITCH (SD_CMD11)
#define SD_CMD_STOP_TRANSMISSION (SD_CMD12)
#define SD_CMD_SEND_STATUS (SD_CMD13)
#define SD_CMD_GO_INACTIVE_STATE (SD_CMD15)
#define SD_CMD_SET_BLOCKLEN (SD_CMD16)
#define SD_CMD_READ_SINGLE_BLOCK (SD_CMD17)
#define SD_CMD_READ_MULTIPLE_BLOCK (SD_CMD18)
#define SD_CMD_SEND_TUNING_BLOCK (SD_CMD19)
#define SD_CMD_SPEED_CLASS_CONTROL (SD_CMD20)
#define SD_CMD_ADDRESS_EXTENSION (SD_CMD22)
#define SD_CMD_SET_BLOCK_COUNT (SD_CMD23)
#define SD_CMD_WRITE_BLOCK (SD_CMD24)
#define SD_CMD_WRITE_MULTIPLE_BLOCK (SD_CMD25)
#define SD_CMD_PROGRAM_CSD (SD_CMD27)
#define SD_CMD_SET_WRITE_PROT (SD_CMD28)
#define SD_CMD_CLR_WRITE_PROT (SD_CMD29)
#define SD_CMD_SEND_WRITE_PROT (SD_CMD30)
#define SD_CMD_ERASE_WR_BLK_START (SD_CMD32)
#define SD_CMD_ERASE_WR_BLK_END (SD_CMD33)
#define SD_CMD_ERASE (SD_CMD38)
#define SD_CMD_LOCK_UNLOCK (SD_CMD42)
#define SD_CMD_Q_MANAGEMENT (SD_CMD43)
#define SD_CMD_Q_TASK_INFO_A (SD_CMD44)
#define SD_CMD_Q_TASK_INFO_B (SD_CMD45)
#define SD_CMD_Q_RD_TASK (SD_CMD46)
#define SD_CMD_Q_WR_TASK (SD_CMD47)
#define SD_CMD_READ_EXTR_SINGLE (SD_CMD48)
#define SD_CMD_WRITE_EXTR_SINGLE (SD_CMD49)
#define SD_CMD_APP_CMD (SD_CMD55)
#define SD_CMD_GEN_CMD (SD_CMD56)
#define SD_CMD_READ_EXTR_MULTI (SD_CMD58)
#define SD_CMD_WRITE_EXTR_MULTI (SD_CMD59)

#define SD_ACMD_SET_BUS_WIDTH (SD_ACMD6)
#define SD_ACMD_SD_STATUS (SD_ACMD13)
#define SD_ACMD_SEND_NUM_WR_BLOCKS (SD_ACMD22)
#define SD_ACMD_SET_WR_BLK_ERASE_COUNT (SD_ACMD23)
#define SD_ACMD_SD_SEND_OP_COND (SD_ACMD41)
#define SD_ACMD_SET_CLR_CARD_DETECT (SD_ACMD42)
#define SD_ACMD_SEND_SCR (SD_ACMD51)

#define SD_CARD_STATUS_AKE_SEQ_ERROR        (0x1 <<  3)
#define SD_CARD_STATUS_APP_CMD              (0x1 <<  5)
#define SD_CARD_STATUS_FX_EVENT             (0x1 <<  6)
#define SD_CARD_STATUS_READY_FOR_DATA       (0x1 <<  8)
#define SD_CARD_STATUS_CURRENT_STATE_MASK   (0xf <<  9)
#define  SD_CARD_STATUS_STATE_IDLE          (0x0 <<  9)
#define  SD_CARD_STATUS_STATE_READY         (0x1 <<  9)
#define  SD_CARD_STATUS_STATE_IDENT         (0x2 <<  9)
#define  SD_CARD_STATUS_STATE_STBY          (0x3 <<  9)
#define  SD_CARD_STATUS_STATE_TRAN          (0x4 <<  9)
#define  SD_CARD_STATUS_STATE_DATA          (0x5 <<  9)
#define  SD_CARD_STATUS_STATE_RCV           (0x6 <<  9)
#define  SD_CARD_STATUS_STATE_PRG           (0x7 <<  9)
#define  SD_CARD_STATUS_STATE_DIS           (0x8 <<  9)
#define SD_CARD_STATUS_STATE_IS_VALID(s)    (((((s) & SD_CARD_STATUS_CURRENT_STATE_MASK) >> 9) & 0xf) < 0x9)
#define SD_CARD_STATUS_ERASE_RESET          (0x1 << 13)
#define SD_CARD_STATUS_CARD_ECC_DISABLED    (0x1 << 14)
#define SD_CARD_STATUS_WP_ERASE_SKIP        (0x1 << 15)
#define SD_CARD_STATUS_CSD_OVERWRITE        (0x1 << 16)
#define SD_CARD_STATUS_ERROR                (0x1 << 19)
#define SD_CARD_STATUS_CC_ERROR             (0x1 << 20)
#define SD_CARD_STATUS_CARD_ECC_FAILED      (0x1 << 21)
#define SD_CARD_STATUS_ILLEGAL_COMMAND      (0x1 << 22)
#define SD_CARD_STATUS_COM_CRC_ERROR        (0x1 << 23)
#define SD_CARD_STATUS_LOCK_UNLOCK_FAILED   (0x1 << 24)
#define SD_CARD_STATUS_CARD_IS_LOCKED       (0x1 << 25)
#define SD_CARD_STATUS_WP_VIOLATION         (0x1 << 26)
#define SD_CARD_STATUS_ERASE_PARAM          (0x1 << 27)
#define SD_CARD_STATUS_ERASE_SEQ_ERROR      (0x1 << 28)
#define SD_CARD_STATUS_BLOCK_LEN_ERROR      (0x1 << 29)
#define SD_CARD_STATUS_ADDRESS_ERROR        (0x1 << 30)
#define SD_CARD_STATUS_OUT_OF_RANGE         (0x1 << 31)

/* card status bits marked as indicating errors */
#define SD_CARD_STATUS_ERROR_MASK           (SD_CARD_STATUS_AKE_SEQ_ERROR       \
                                            |SD_CARD_STATUS_WP_ERASE_SKIP       \
                                            |SD_CARD_STATUS_CSD_OVERWRITE       \
                                            |SD_CARD_STATUS_ERROR               \
                                            |SD_CARD_STATUS_CC_ERROR            \
                                            |SD_CARD_STATUS_CARD_ECC_FAILED     \
                                            |SD_CARD_STATUS_ILLEGAL_COMMAND     \
                                            |SD_CARD_STATUS_COM_CRC_ERROR       \
                                            |SD_CARD_STATUS_LOCK_UNLOCK_FAILED  \
                                            |SD_CARD_STATUS_WP_VIOLATION        \
                                            |SD_CARD_STATUS_ERASE_PARAM         \
                                            |SD_CARD_STATUS_ERASE_SEQ_ERROR     \
                                            |SD_CARD_STATUS_BLOCK_LEN_ERROR     \
                                            |SD_CARD_STATUS_ADDRESS_ERROR       \
                                            |SD_CARD_STATUS_OUT_OF_RANGE)
/* all valid card bits */
#define SD_CARD_STATUS_READ_MASK            (SD_CARD_STATUS_ERROR_MASK          \
                                            |SD_CARD_STATUS_APP_CMD             \
                                            |SD_CARD_STATUS_FX_EVENT            \
                                            |SD_CARD_STATUS_READY_FOR_DATA      \
                                            |SD_CARD_STATUS_CURRENT_STATE_MASK  \
                                            |SD_CARD_STATUS_ERASE_RESET         \
                                            |SD_CARD_STATUS_CARD_ECC_DISABLED   \
                                            |SD_CARD_STATUS_CARD_IS_LOCKED)

/* convert an R6 response to a card status */
#define SD_CARD_STATUS_R6_TO_CARD_STATUS(s) (((s) & 0xffff)                     \
                                            |((((s) & 0x2000) >> 13) << 19)     \
                                            |((((s) & 0x4000) >> 14) << 22)     \
                                            |((((s) & 0x8000) >> 15) << 23))

#define SD_CARD_STATUS_IS_ERROR(s)          (((s) & (SD_CARD_STATUS_ERROR_MASK)) != 0)

#define SD_SEND_OP_COND_OCR_MASK            (0x00ffff00)
/* Switching to 1.8V Request */
#define SD_SEND_OP_COND_S18R                (0x1 << 24)
/* Switching to 1.8V Accepted */
#define SD_SEND_OP_COND_S18A                (SD_SEND_OP_COND_S18R)
/* SDXC Power Control */
#define SD_SEND_OP_COND_XPC                 (0x1 << 28)
#define SD_SEND_OP_COND_FB                  (0x1 << 29)
/* UHS-II Card Status */
#define SD_SEND_OP_COND_UHSII               (SD_SEND_OP_COND_FB)
/* Host Capacity Support */
#define SD_SEND_OP_COND_HCS                 (0x1 << 30)
/* Card Capacity Status */
#define SD_SEND_OP_COND_CCS                 (SD_SEND_OP_COND_HCS)
#define SD_SEND_OP_COND_BUSY                (0x1 << 31)

#define OCR_2V7TO2V8            (1U << 15)
#define OCR_2V8TO2V9            (1U << 16)
#define OCR_2V9TO3V0            (1U << 17)
#define OCR_3V0TO3V1            (1U << 18)
#define OCR_3V1TO3V2            (1U << 19)
#define OCR_3V2TO3V3            (1U << 20)
#define OCR_3V3TO3V4            (1U << 21)
#define OCR_3V4TO3V5            (1U << 22)
#define OCR_3V5TO3V6            (1U << 23)
#define OCR_CARD_CAPACITY       (1U << 30)
#define OCR_CARD_POWER_UP_BUSY  (1U << 31)

#endif
