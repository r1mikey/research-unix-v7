/*
 * Derived from SDCard.c by Leon de Boer.  Original fetched from:
 *  https://github.com/LdB-ECM/Raspberry-Pi/tree/master/SD_FAT32
 *
  / ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++}
  {                                                                           }                                                                           
  {       Filename: SDCard.c                                                  }                                                   
  {       Copyright(c): Leon de Boer(LdB) 2017                                }                           
  {       Version: 1.01                                                       }                                                   
  {       Original start code from hldswrth use from the Pi Forum             }
  {                                                                           }                                                                           
  {***************[ THIS CODE IS FREEWARE UNDER CC Attribution]***************}
  {                                                                           }                                                               
  {     This sourcecode is released for the purpose to promote programming    }
  {  on the Raspberry Pi. You may redistribute it and/or modify with the      }
  {  following disclaimer and condition.                                      }
  {                                                                           }                                                               
  {      The SOURCE CODE is distributed "AS IS" WITHOUT WARRANTIES AS TO      }
  {   PERFORMANCE OF MERCHANTABILITY WHETHER EXPRESSED OR IMPLIED.            }
  {   Redistributions of source code must retain the copyright notices to     }
  {   maintain the author credit (attribution) .                              }                           
  {                                                                           }                                                                           
  {************************************************************************** /
 */
#include "bcm283x_sdcard.h"

#include "bcm283x_systimer.h"
#include "bcm283x_mbox.h"
#include "bcm283x_irq.h"
#include "bcm283x_io.h"
#include "../arm1176jzfs.h"

#include "../../h/param.h"
#include "../../h/user.h"

/* #include <stdint.h> */                                // Needed for intptr_t
/* #include <ctype.h> */                                // Needed for toupper for wildcard string match
/* #include <wchar.h> */                                // Needed for UTF for long file name support
/* #include <string.h> */                                // Needed for string copy

extern void printf(const char *fmt, ...);

struct volume_t {
  u32 present;
  u32 start;
  u32 count;
};

#define MAX_VOLUMES 4
struct volume_t volumes[MAX_VOLUMES] = {
  { 0, 0, 0, },
  { 0, 0, 0, },
  { 0, 0, 0, },
  { 0, 0, 0, },
};

u32 _bcm283x_sdcard_unix_start_block;
u32 _bcm283x_sdcard_unix_max_blocks;

extern void panic(const char *s) __attribute__((noreturn));

/*--------------------------------------------------------------------------}
{  This controls if debugging code is compiled or removed at compile time   }
{--------------------------------------------------------------------------*/
/* #define DEBUG_INFO 1 */                                  // Compile debugging message code .... set to 1 and other value means no compilation
/* #define DEBUG_SD_READS */

/*--------------------------------------------------------------------------}
{  The working part of the DEBUG_INFO macro to make compilation on or off   }
{--------------------------------------------------------------------------*/
#if DEBUG_INFO == 1
#define LOG_DEBUG(...) printf( __VA_ARGS__ )            // This displays debugging messages to function given
#else
#define LOG_DEBUG(...)                                    // This just swallows debug code, it doesn't even get compiled
#endif

/***************************************************************************}
{    PRIVATE INTERNAL SD HOST REGISTER STRUCTURES AS PER BCM2835 MANUAL     }
****************************************************************************/

/*--------------------------------------------------------------------------}
{      EMMC BLKSIZECNT register - BCM2835.PDF Manual Section 5 page 68      }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regBLKSIZECNT {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned BLKSIZE : 10;                // @0-9        Block size in bytes
            unsigned reserved : 6;                        // @10-15    Write as zero read as don't care
            volatile unsigned BLKCNT : 16;                // @16-31    Number of blocks to be transferred 
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{      EMMC CMDTM register - BCM2835.PDF Manual Section 5 pages 69-70       }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regCMDTM {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            unsigned reserved : 1;                        // @0        Write as zero read as don't care
            volatile unsigned TM_BLKCNT_EN : 1;            // @1        Enable the block counter for multiple block transfers
            volatile enum {    TM_NO_COMMAND = 0,            //            no command 
                            TM_CMD12 = 1,                //            command CMD12 
                            TM_CMD23 = 2,                //            command CMD23
                            TM_RESERVED = 3,
                          } TM_AUTO_CMD_EN : 2;            // @2-3        Select the command to be send after completion of a data transfer
            volatile unsigned TM_DAT_DIR : 1;            // @4        Direction of data transfer (0 = host to card , 1 = card to host )
            volatile unsigned TM_MULTI_BLOCK : 1;        // @5        Type of data transfer (0 = single block, 1 = muli block)
            unsigned reserved1 : 10;                    // @6-15    Write as zero read as don't care
            volatile enum {    CMD_NO_RESP = 0,            //            no response
                            CMD_136BIT_RESP = 1,        //            136 bits response 
                            CMD_48BIT_RESP = 2,            //            48 bits response 
                            CMD_BUSY48BIT_RESP = 3,        //            48 bits response using busy 
                          } CMD_RSPNS_TYPE : 2;            // @16-17
            unsigned reserved2 : 1;                        // @18        Write as zero read as don't care
            volatile unsigned CMD_CRCCHK_EN : 1;        // @19        Check the responses CRC (0=disabled, 1= enabled)
            volatile unsigned CMD_IXCHK_EN : 1;            // @20        Check that response has same index as command (0=disabled, 1= enabled)
            volatile unsigned CMD_ISDATA : 1;            // @21        Command involves data transfer (0=disabled, 1= enabled)
            volatile enum {    CMD_TYPE_NORMAL = 0,        //            normal command
                            CMD_TYPE_SUSPEND = 1,        //            suspend command 
                            CMD_TYPE_RESUME = 2,        //            resume command 
                            CMD_TYPE_ABORT = 3,            //            abort command 
                          } CMD_TYPE : 2;                // @22-23 
            volatile unsigned CMD_INDEX : 6;            // @24-29
            unsigned reserved3 : 2;                        // @30-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{        EMMC STATUS register - BCM2835.PDF Manual Section 5 page 72        }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regSTATUS {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned CMD_INHIBIT : 1;            // @0        Command line still used by previous command
            volatile unsigned DAT_INHIBIT : 1;            // @1        Data lines still used by previous data transfer
            volatile unsigned DAT_ACTIVE : 1;            // @2        At least one data line is active
            unsigned reserved : 5;                        // @3-7        Write as zero read as don't care
            volatile unsigned WRITE_TRANSFER : 1;        // @8        New data can be written to EMMC
            volatile unsigned READ_TRANSFER : 1;        // @9        New data can be read from EMMC
            unsigned reserved1 : 10;                    // @10-19    Write as zero read as don't care
            volatile unsigned DAT_LEVEL0 : 4;            // @20-23    Value of data lines DAT3 to DAT0
            volatile unsigned CMD_LEVEL : 1;            // @24        Value of command line CMD 
            volatile unsigned DAT_LEVEL1 : 4;            // @25-28    Value of data lines DAT7 to DAT4
            unsigned reserved2 : 3;                        // @29-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{      EMMC CONTROL0 register - BCM2835.PDF Manual Section 5 page 73/74     }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regCONTROL0 {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            unsigned reserved : 1;                        // @0        Write as zero read as don't care
            volatile unsigned HCTL_DWIDTH : 1;            // @1        Use 4 data lines (true = enable)
            volatile unsigned HCTL_HS_EN : 1;            // @2        Select high speed mode (true = enable)
            unsigned reserved1 : 2;                        // @3-4        Write as zero read as don't care
            volatile unsigned HCTL_8BIT : 1;            // @5        Use 8 data lines (true = enable)
            unsigned reserved2 : 10;                    // @6-15    Write as zero read as don't care
            volatile unsigned GAP_STOP : 1;                // @16        Stop the current transaction at the next block gap
            volatile unsigned GAP_RESTART : 1;            // @17        Restart a transaction last stopped using the GAP_STOP
            volatile unsigned READWAIT_EN : 1;            // @18        Use DAT2 read-wait protocol for cards supporting this
            volatile unsigned GAP_IEN : 1;                // @19        Enable SDIO interrupt at block gap 
            volatile unsigned SPI_MODE : 1;                // @20        SPI mode enable
            volatile unsigned BOOT_EN : 1;                // @21        Boot mode access
            volatile unsigned ALT_BOOT_EN : 1;            // @22        Enable alternate boot mode access
            unsigned reserved3 : 9;                        // @23-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{      EMMC CONTROL1 register - BCM2835.PDF Manual Section 5 page 74/75     }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regCONTROL1 {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned CLK_INTLEN : 1;            // @0        Clock enable for internal EMMC clocks for power saving
            volatile const unsigned CLK_STABLE : 1;        // @1        SD clock stable  0=No 1=yes   **read only
            volatile unsigned CLK_EN : 1;                // @2        SD clock enable  0=disable 1=enable
            unsigned reserved : 2;                        // @3-4        Write as zero read as don't care
            volatile unsigned CLK_GENSEL : 1;            // @5        Mode of clock generation (0=Divided, 1=Programmable)
            volatile unsigned CLK_FREQ_MS2 : 2;            // @6-7        SD clock base divider MSBs (Version3+ only)
            volatile unsigned CLK_FREQ8 : 8;            // @8-15    SD clock base divider LSBs
            volatile unsigned DATA_TOUNIT : 4;            // @16-19    Data timeout unit exponent
            unsigned reserved1 : 4;                        // @20-23    Write as zero read as don't care
            volatile unsigned SRST_HC : 1;                // @24        Reset the complete host circuit
            volatile unsigned SRST_CMD : 1;                // @25        Reset the command handling circuit
            volatile unsigned SRST_DATA : 1;            // @26        Reset the data handling circuit
            unsigned reserved2 : 5;                        // @27-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{    EMMC CONTROL2 register - BCM2835.PDF Manual Section 5 pages 81-84      }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regCONTROL2 {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile const unsigned ACNOX_ERR : 1;        // @0        Auto command not executed due to an error **read only
            volatile const unsigned ACTO_ERR : 1;        // @1        Timeout occurred during auto command execution **read only
            volatile const unsigned ACCRC_ERR : 1;        // @2        Command CRC error occurred during auto command execution **read only
            volatile const unsigned ACEND_ERR : 1;        // @3        End bit is not 1 during auto command execution **read only
            volatile const unsigned ACBAD_ERR : 1;        // @4        Command index error occurred during auto command execution **read only
            unsigned reserved : 2;                        // @5-6        Write as zero read as don't care
            volatile const unsigned NOTC12_ERR : 1;        // @7        Error occurred during auto command CMD12 execution **read only
            unsigned reserved1 : 8;                        // @8-15    Write as zero read as don't care            
            volatile enum { SDR12 = 0,
                            SDR25 = 1, 
                            SDR50 = 2,
                            SDR104 = 3,
                            DDR50 = 4,
                          } UHSMODE : 3;                // @16-18    Select the speed mode of the SD card (SDR12, SDR25 etc)
            unsigned reserved2 : 3;                        // @19-21    Write as zero read as don't care
            volatile unsigned TUNEON : 1;                // @22        Start tuning the SD clock
            volatile unsigned TUNED : 1;                // @23        Tuned clock is used for sampling data
            unsigned reserved3 : 8;                        // @24-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{     EMMC INTERRUPT register - BCM2835.PDF Manual Section 5 pages 75-77    }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regINTERRUPT {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned CMD_DONE : 1;                // @0        Command has finished
            volatile unsigned DATA_DONE : 1;            // @1        Data transfer has finished
            volatile unsigned BLOCK_GAP : 1;            // @2        Data transfer has stopped at block gap
            unsigned reserved : 1;                        // @3        Write as zero read as don't care
            volatile unsigned WRITE_RDY : 1;            // @4        Data can be written to DATA register
            volatile unsigned READ_RDY : 1;                // @5        DATA register contains data to be read
            unsigned reserved1 : 2;                        // @6-7        Write as zero read as don't care
            volatile unsigned CARD : 1;                    // @8        Card made interrupt request
            unsigned reserved2 : 3;                        // @9-11    Write as zero read as don't care
            volatile unsigned RETUNE : 1;                // @12        Clock retune request was made
            volatile unsigned BOOTACK : 1;                // @13        Boot acknowledge has been received
            volatile unsigned ENDBOOT : 1;                // @14        Boot operation has terminated
            volatile unsigned ERR : 1;                    // @15        An error has occured
            volatile unsigned CTO_ERR : 1;                // @16        Timeout on command line
            volatile unsigned CCRC_ERR : 1;                // @17        Command CRC error
            volatile unsigned CEND_ERR : 1;                // @18        End bit on command line not 1
            volatile unsigned CBAD_ERR : 1;                // @19        Incorrect command index in response
            volatile unsigned DTO_ERR : 1;                // @20        Timeout on data line
            volatile unsigned DCRC_ERR : 1;                // @21        Data CRC error
            volatile unsigned DEND_ERR : 1;                // @22        End bit on data line not 1
            unsigned reserved3 : 1;                        // @23        Write as zero read as don't care
            volatile unsigned ACMD_ERR : 1;                // @24        Auto command error
            unsigned reserved4 : 7;                        // @25-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{     EMMC IRPT_MASK register - BCM2835.PDF Manual Section 5 pages 77-79    }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regIRPT_MASK {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned CMD_DONE : 1;                // @0        Command has finished
            volatile unsigned DATA_DONE : 1;            // @1        Data transfer has finished
            volatile unsigned BLOCK_GAP : 1;            // @2        Data transfer has stopped at block gap
            unsigned reserved : 1;                        // @3        Write as zero read as don't care
            volatile unsigned WRITE_RDY : 1;            // @4        Data can be written to DATA register
            volatile unsigned READ_RDY : 1;                // @5        DATA register contains data to be read
            unsigned reserved1 : 2;                        // @6-7        Write as zero read as don't care
            volatile unsigned CARD : 1;                    // @8        Card made interrupt request
            unsigned reserved2 : 3;                        // @9-11    Write as zero read as don't care
            volatile unsigned RETUNE : 1;                // @12        Clock retune request was made
            volatile unsigned BOOTACK : 1;                // @13        Boot acknowledge has been received
            volatile unsigned ENDBOOT : 1;                // @14        Boot operation has terminated
            volatile unsigned ERR : 1;                    // @15        An error has occured
            volatile unsigned CTO_ERR : 1;                // @16        Timeout on command line
            volatile unsigned CCRC_ERR : 1;                // @17        Command CRC error
            volatile unsigned CEND_ERR : 1;                // @18        End bit on command line not 1
            volatile unsigned CBAD_ERR : 1;                // @19        Incorrect command index in response
            volatile unsigned DTO_ERR : 1;                // @20        Timeout on data line
            volatile unsigned DCRC_ERR : 1;                // @21        Data CRC error
            volatile unsigned DEND_ERR : 1;                // @22        End bit on data line not 1
            unsigned reserved3 : 1;                        // @23        Write as zero read as don't care
            volatile unsigned ACMD_ERR : 1;                // @24        Auto command error
            unsigned reserved4 : 7;                        // @25-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{      EMMC IRPT_EN register - BCM2835.PDF Manual Section 5 pages 79-71     }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regIRPT_EN {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned CMD_DONE : 1;                // @0        Command has finished
            volatile unsigned DATA_DONE : 1;            // @1        Data transfer has finished
            volatile unsigned BLOCK_GAP : 1;            // @2        Data transfer has stopped at block gap
            unsigned reserved : 1;                        // @3        Write as zero read as don't care
            volatile unsigned WRITE_RDY : 1;            // @4        Data can be written to DATA register
            volatile unsigned READ_RDY : 1;                // @5        DATA register contains data to be read
            unsigned reserved1 : 2;                        // @6-7        Write as zero read as don't care
            volatile unsigned CARD : 1;                    // @8        Card made interrupt request
            unsigned reserved2 : 3;                        // @9-11    Write as zero read as don't care
            volatile unsigned RETUNE : 1;                // @12        Clock retune request was made
            volatile unsigned BOOTACK : 1;                // @13        Boot acknowledge has been received
            volatile unsigned ENDBOOT : 1;                // @14        Boot operation has terminated
            volatile unsigned ERR : 1;                    // @15        An error has occured
            volatile unsigned CTO_ERR : 1;                // @16        Timeout on command line
            volatile unsigned CCRC_ERR : 1;                // @17        Command CRC error
            volatile unsigned CEND_ERR : 1;                // @18        End bit on command line not 1
            volatile unsigned CBAD_ERR : 1;                // @19        Incorrect command index in response
            volatile unsigned DTO_ERR : 1;                // @20        Timeout on data line
            volatile unsigned DCRC_ERR : 1;                // @21        Data CRC error
            volatile unsigned DEND_ERR : 1;                // @22        End bit on data line not 1
            unsigned reserved3 : 1;                        // @23        Write as zero read as don't care
            volatile unsigned ACMD_ERR : 1;                // @24        Auto command error
            unsigned reserved4 : 7;                        // @25-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{       EMMC TUNE_STEP  register - BCM2835.PDF Manual Section 5 page 86     }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regTUNE_STEP {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile enum {    TUNE_DELAY_200ps  = 0,
                            TUNE_DELAY_400ps  = 1,
                            TUNE_DELAY_400psA = 2,        // I dont understand the duplicate value???
                            TUNE_DELAY_600ps  = 3,
                            TUNE_DELAY_700ps  = 4,
                            TUNE_DELAY_900ps  = 5,
                            TUNE_DELAY_900psA = 6,        // I dont understand the duplicate value??
                            TUNE_DELAY_1100ps = 7,
                           } DELAY : 3;                    // @0-2        Select the speed mode of the SD card (SDR12, SDR25 etc)
            unsigned reserved : 29;                        // @3-31    Write as zero read as don't care
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};

/*--------------------------------------------------------------------------}
{    EMMC SLOTISR_VER register - BCM2835.PDF Manual Section 5 pages 87-88   }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regSLOTISR_VER {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned SLOT_STATUS : 8;            // @0-7        Logical OR of interrupt and wakeup signal for each slot
            unsigned reserved : 8;                        // @8-15    Write as zero read as don't care
            volatile unsigned SDVERSION : 8;            // @16-23    Host Controller specification version 
            volatile unsigned VENDOR : 8;                // @24-31    Vendor Version Number 
        };
        volatile u32 Raw32;                        // @0-31    Union to access all 32 bits as a u32
    };
};


/***************************************************************************}
{         PRIVATE POINTERS TO ALL THE BCM2835 EMMC HOST REGISTERS           }
****************************************************************************/
extern u32 _bcm283x_iobase;

#define EMMC_OFFSET                   0x00300000
#define EMMC_BASE                     ((_bcm283x_iobase) + (EMMC_OFFSET))

#define EMMC_ARG2_REG_OFFSET          0x00000000  /* ACMD23 Argument                   */
#define EMMC_BLKSIZECNT_REG_OFFSET    0x00000004  /* Block Size and Count              */
#define EMMC_ARG1_REG_OFFSET          0x00000008  /* Argument                          */
#define EMMC_CMDTM_REG_OFFSET         0x0000000c  /* Command and Transfer Mode         */
#define EMMC_RESP0_REG_OFFSET         0x00000010  /* Response bits 31 : 0              */
#define EMMC_RESP1_REG_OFFSET         0x00000014  /* Response bits 63 : 32             */
#define EMMC_RESP2_REG_OFFSET         0x00000018  /* Response bits 95 : 64             */
#define EMMC_RESP3_REG_OFFSET         0x0000001c  /* Response bits 127 : 96            */
#define EMMC_DATA_REG_OFFSET          0x00000020  /* Data                              */
#define EMMC_STATUS_REG_OFFSET        0x00000024  /* Status                            */
#define EMMC_CONTROL0_REG_OFFSET      0x00000028  /* Host Configuration bits           */
#define EMMC_CONTROL1_REG_OFFSET      0x0000002c  /* Host Configuration bits           */
#define EMMC_INTERRUPT_REG_OFFSET     0x00000030  /* Interrupt Flags                   */
#define EMMC_IRPT_MASK_REG_OFFSET     0x00000034  /* Interrupt Flag Enable             */
#define EMMC_IRPT_EN_REG_OFFSET       0x00000038  /* Interrupt Generation Enable       */
#define EMMC_CONTROL2_REG_OFFSET      0x0000003c  /* Host Configuration bits           */
                                                  /* Force Interrupt Event             */
                                                  /* Timeout in boot mode              */
                                                  /* Debug Bus Configuration           */
                                                  /* Extension FIFO Configuration      */
                                                  /* Extension FIFO Enable             */
#define EMMC_TUNE_STEP_REG_OFFSET     0x00000088  /* Delay per card clock tuning step  */
                                                  /* Card clock tuning steps for SDR   */
                                                  /* Card clock tuning steps for DDR   */
                                                  /* SPI Interrupt Support             */
#define EMMC_SLOTISR_VER_REG_OFFSET   0x000000fc  /* Slot Interrupt Status and Version */

#define EMMC_ARG2_REG                 ((EMMC_BASE) + (EMMC_ARG2_REG))
#define EMMC_BLKSIZECNT_REG           ((EMMC_BASE) + (EMMC_BLKSIZECNT_REG_OFFSET))
#define EMMC_ARG1_REG                 ((EMMC_BASE) + (EMMC_ARG1_REG_OFFSET))
#define EMMC_CMDTM_REG                ((EMMC_BASE) + (EMMC_CMDTM_REG_OFFSET))
#define EMMC_RESP0_REG                ((EMMC_BASE) + (EMMC_RESP0_REG_OFFSET))
#define EMMC_RESP1_REG                ((EMMC_BASE) + (EMMC_RESP1_REG_OFFSET))
#define EMMC_RESP2_REG                ((EMMC_BASE) + (EMMC_RESP2_REG_OFFSET))
#define EMMC_RESP3_REG                ((EMMC_BASE) + (EMMC_RESP3_REG_OFFSET))
#define EMMC_DATA_REG                 ((EMMC_BASE) + (EMMC_DATA_REG_OFFSET))
#define EMMC_STATUS_REG               ((EMMC_BASE) + (EMMC_STATUS_REG_OFFSET))
#define EMMC_CONTROL0_REG             ((EMMC_BASE) + (EMMC_CONTROL0_REG_OFFSET))
#define EMMC_CONTROL1_REG             ((EMMC_BASE) + (EMMC_CONTROL1_REG_OFFSET))
#define EMMC_INTERRUPT_REG            ((EMMC_BASE) + (EMMC_INTERRUPT_REG_OFFSET))
#define EMMC_IRPT_MASK_REG            ((EMMC_BASE) + (EMMC_IRPT_MASK_REG_OFFSET))
#define EMMC_IRPT_EN_REG              ((EMMC_BASE) + (EMMC_IRPT_EN_REG_OFFSET))
#define EMMC_CONTROL2_REG             ((EMMC_BASE) + (EMMC_CONTROL2_REG_OFFSET))
#define EMMC_TUNE_STEP_REG            ((EMMC_BASE) + (EMMC_TUNE_STEP_REG_OFFSET))
#define EMMC_SLOTISR_VER_REG          ((EMMC_BASE) + (EMMC_SLOTISR_VER_REG_OFFSET))

/* EMMC_STATUS_REG */
#define CMD_INHIBIT_BIT               0
#define DAT_INHIBIT_BIT               1
#define CMD_INHIBIT_MASK              BIT(CMD_INHIBIT_BIT)
#define DAT_INHIBIT_MASK              BIT(DAT_INHIBIT_BIT)

#define EMMC_ARG2            ((volatile __attribute__((aligned(4))) u32*)(uptr_t)(_bcm283x_iobase + 0x300000))
#define EMMC_BLKSIZECNT        ((volatile struct __attribute__((aligned(4))) regBLKSIZECNT*)(uptr_t)(_bcm283x_iobase + 0x300004))
#define EMMC_ARG1            ((volatile __attribute__((aligned(4))) u32*)(uptr_t)(_bcm283x_iobase + 0x300008))
#define EMMC_CMDTM            ((volatile struct __attribute__((aligned(4))) regCMDTM*)(uptr_t)(_bcm283x_iobase + 0x30000c))
#define EMMC_RESP0            ((volatile __attribute__((aligned(4))) u32*)(uptr_t)(_bcm283x_iobase + 0x300010))
#define EMMC_RESP1            ((volatile __attribute__((aligned(4))) u32*)(uptr_t)(_bcm283x_iobase + 0x300014))
#define EMMC_RESP2            ((volatile __attribute__((aligned(4))) u32*)(uptr_t)(_bcm283x_iobase + 0x300018))
#define EMMC_RESP3            ((volatile __attribute__((aligned(4))) u32*)(uptr_t)(_bcm283x_iobase + 0x30001C))
#define EMMC_DATA            ((volatile __attribute__((aligned(4))) u32*)(uptr_t)(_bcm283x_iobase + 0x300020))
#define EMMC_STATUS            ((volatile struct __attribute__((aligned(4))) regSTATUS*)(uptr_t)(_bcm283x_iobase + 0x300024))
#define EMMC_CONTROL0        ((volatile struct __attribute__((aligned(4))) regCONTROL0*)(uptr_t)(_bcm283x_iobase + 0x300028))
#define EMMC_CONTROL1        ((volatile struct __attribute__((aligned(4))) regCONTROL1*)(uptr_t)(_bcm283x_iobase + 0x30002C))
#define EMMC_INTERRUPT        ((volatile struct __attribute__((aligned(4))) regINTERRUPT*)(uptr_t)(_bcm283x_iobase + 0x300030))
#define EMMC_IRPT_MASK        ((volatile struct __attribute__((aligned(4))) regIRPT_MASK*)(uptr_t)(_bcm283x_iobase + 0x300034))
#define EMMC_IRPT_EN        ((volatile struct __attribute__((aligned(4))) regIRPT_EN*)(uptr_t)(_bcm283x_iobase + 0x300038))
#define EMMC_CONTROL2        ((volatile struct __attribute__((aligned(4))) regCONTROL2*)(uptr_t)(_bcm283x_iobase + 0x30003C))
#define EMMC_TUNE_STEP         ((volatile struct __attribute__((aligned(4))) regTUNE_STEP*)(uptr_t)(_bcm283x_iobase + 0x300088))
#define EMMC_SLOTISR_VER    ((volatile struct __attribute__((aligned(4))) regSLOTISR_VER*)(uptr_t)(_bcm283x_iobase + 0x3000fC))


/***************************************************************************}
{   PRIVATE INTERNAL SD CARD REGISTER STRUCTURES AS PER SD CARD STANDARD    }
****************************************************************************/

/*--------------------------------------------------------------------------}
{                           SD CARD OCR register                                }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regOCR {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            unsigned reserved : 15;                        // @0-14    Write as zero read as don't care
            unsigned voltage2v7to2v8 : 1;                // @15        Voltage window 2.7v to 2.8v
            unsigned voltage2v8to2v9 : 1;                // @16        Voltage window 2.8v to 2.9v
            unsigned voltage2v9to3v0 : 1;                // @17        Voltage window 2.9v to 3.0v
            unsigned voltage3v0to3v1 : 1;                // @18        Voltage window 3.0v to 3.1v
            unsigned voltage3v1to3v2 : 1;                // @19        Voltage window 3.1v to 3.2v
            unsigned voltage3v2to3v3 : 1;                // @20        Voltage window 3.2v to 3.3v
            unsigned voltage3v3to3v4 : 1;                // @21        Voltage window 3.3v to 3.4v
            unsigned voltage3v4to3v5 : 1;                // @22        Voltage window 3.4v to 3.5v
            unsigned voltage3v5to3v6 : 1;                // @23        Voltage window 3.5v to 3.6v
            unsigned reserved1 : 6;                        // @24-29    Write as zero read as don't care
            unsigned card_capacity : 1;                    // @30        Card Capacity status
            unsigned card_power_up_busy : 1;            // @31        Card power up status (busy)
        };
        volatile u32 Raw32;                        // @0-31    Union to access 32 bits as a u32        
    };
};

/*--------------------------------------------------------------------------}
{                           SD CARD SCR register                                }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) regSCR {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile enum { SD_SPEC_1_101 = 0,            // ..enum..    Version 1.0-1.01 
                            SD_SPEC_11 = 1,                // ..enum..    Version 1.10 
                            SD_SPEC_2_3 = 2,            // ..enum..    Version 2.00 or Version 3.00 (check bit SD_SPEC3)
                          } SD_SPEC : 4;                // @0-3        SD Memory Card Physical Layer Specification version
            volatile enum {    SCR_VER_1 = 0,                // ..enum..    SCR version 1.0
                          } SCR_STRUCT : 4;                // @4-7        SCR structure version
            volatile enum { BUS_WIDTH_1 = 1,            // ..enum..    Card supports bus width 1
                            BUS_WIDTH_4 = 4,            // ..enum.. Card supports bus width 4
                           } BUS_WIDTH : 4;                // @8-11    SD Bus width
            volatile enum { SD_SEC_NONE = 0,            // ..enum..    No Security
                            SD_SEC_NOT_USED = 1,        // ..enum..    Security Not Used
                            SD_SEC_101 = 2,                // ..enum..    SDSC Card (Security Version 1.01)
                            SD_SEC_2 = 3,                // ..enum..    SDHC Card (Security Version 2.00)
                            SD_SEC_3 = 4,                // ..enum..    SDXC Card (Security Version 3.xx)
                          } SD_SECURITY : 3;            // @12-14    Card security in use
            volatile unsigned DATA_AFTER_ERASE : 1;        // @15        Defines the data status after erase, whether it is 0 or 1
            unsigned reserved : 3;                        // @16-18    Write as zero read as don't care
            volatile enum {    EX_SEC_NONE = 0,            // ..enum..    No extended Security
                          } EX_SECURITY : 4;            // @19-22    Extended security
            volatile unsigned SD_SPEC3 : 1;                // @23        Spec. Version 3.00 or higher
            volatile enum { CMD_SUPP_SPEED_CLASS = 1,
                            CMD_SUPP_SET_BLKCNT = 2,
                           } CMD_SUPPORT : 2;            // @24-25    CMD support
            unsigned reserved1 : 6;                        // @26-63    Write as zero read as don't care
        };
        volatile u32 Raw32_Lo;                        // @0-31    Union to access low 32 bits as a u32        
    };
    volatile u32 Raw32_Hi;                            // @32-63    Access upper 32 bits as a u32
};

/*--------------------------------------------------------------------------}
{                          PI SD CARD CID register                            }
{--------------------------------------------------------------------------*/
/* The CID is Big Endian and secondly the Pi butchers it by not having CRC */
/*  So the CID appears shifted 8 bits right with first 8 bits reading zero */
struct __attribute__((__packed__, aligned(4))) regCID {
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile u8 OID_Lo;                    
            volatile u8 OID_Hi;                    // @0-15    Identifies the card OEM. The OID is assigned by the SD-3C, LLC
            volatile u8 MID;                        // @16-23    Manufacturer ID, assigned by the SD-3C, LLC
            unsigned reserved : 8;                        // @24-31    PI butcher with CRC removed these bits end up empty
        };
        volatile u32 Raw32_0;                        // @0-31    Union to access 32 bits as a u32        
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile char ProdName4 : 8;                // @0-7        Product name character four
            volatile char ProdName3 : 8;                // @8-15    Product name character three
            volatile char ProdName2 : 8;                // @16-23    Product name character two
            volatile char ProdName1 : 8;                // @24-31    Product name character one    
        };
        volatile u32 Raw32_1;                        // @0-31    Union to access 32 bits as a u32        
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned SerialNumHi : 16;            // @0-15    Serial number upper 16 bits
            volatile unsigned ProdRevLo : 4;            // @16-19    Product revision low value in BCD
            volatile unsigned ProdRevHi : 4;            // @20-23    Product revision high value in BCD
            volatile char ProdName5 : 8;                // @24-31    Product name character five
        };
        volatile u32 Raw32_2;                        // @0-31    Union to access 32 bits as a u32        
    };
    union {
        struct __attribute__((__packed__, aligned(1))) {
            volatile unsigned ManufactureMonth : 4;        // @0-3        Manufacturing date month (1=Jan, 2=Feb, 3=Mar etc)
            volatile unsigned ManufactureYear : 8;        // @4-11    Manufacturing dateyear (offset from 2000 .. 1=2001,2=2002,3=2003 etc)
            unsigned reserved1 : 4;                        // @12-15     Write as zero read as don't care
            volatile unsigned SerialNumLo : 16;            // @16-23    Serial number lower 16 bits
        };
        volatile u32 Raw32_3;                        // @0-31    Union to access 32 bits as a u32        
    };
};

/***************************************************************************}
{             PRIVATE INTERNAL SD CARD REGISTER STRUCTURE CHECKS             }
****************************************************************************/
/*--------------------------------------------------------------------------}
{                     INTERNAL STRUCTURE COMPILE TIME CHECKS                    }
{--------------------------------------------------------------------------*/
/* GIVEN THE AMOUNT OF PRECISE PACKING OF THESE STRUCTURES .. IT'S PRUDENT */
/* TO CHECK THEM AT COMPILE TIME. USE IS POINTLESS IF THE SIZES ARE WRONG. */
/*-------------------------------------------------------------------------*/
/* If you have never seen compile time assertions it's worth google search */
/* on "Compile Time Assertions". It is part of the C11++ specification and */
/* all compilers that support the standard will have them (GCC, MSC inc)   */
/* This actually produces no code it only executes as a compile time check */
/*-------------------------------------------------------------------------*/
#if 0
#include <assert.h>                                // Need for compile time static_assert

/* Check the main register section group sizes */
static_assert(sizeof(struct regBLKSIZECNT) == 0x04, "eMMC register BLKSIZECNT should be 0x04 bytes in size");
static_assert(sizeof(struct regCMDTM) == 0x04, "eMMC register CMDTM should be 0x04 bytes in size");
static_assert(sizeof(struct regSTATUS) == 0x04, "eMMC register STATUS should be 0x04 bytes in size");
static_assert(sizeof(struct regCONTROL0) == 0x04, "eMMC register CONTROL0 should be 0x04 bytes in size");
static_assert(sizeof(struct regCONTROL1) == 0x04, "eMMC register CONTROL1 should be 0x04 bytes in size");
static_assert(sizeof(struct regCONTROL2) == 0x04, "eMMC register CONTROL2 should be 0x04 bytes in size");
static_assert(sizeof(struct regINTERRUPT) == 0x04, "eMMC register INTERRUPT should be 0x04 bytes in size");
static_assert(sizeof(struct regIRPT_MASK) == 0x04, "eMMC register IRPT_MASK should be 0x04 bytes in size");
static_assert(sizeof(struct regIRPT_EN) == 0x04, "eMMC register IRPT_EN should be 0x04 bytes in size");
static_assert(sizeof(struct regTUNE_STEP) == 0x04, "eMMC register TUNE_STEP should be 0x04 bytes in size");
static_assert(sizeof(struct regSLOTISR_VER) == 0x04, "eMMC register SLOTISR_VER should be 0x04 bytes in size");

static_assert(sizeof(struct regOCR) == 0x04, "eMMC register OCR should be 0x04 bytes in size");
static_assert(sizeof(struct regSCR) == 0x08, "eMMC register SCR should be 0x08 bytes in size");
static_assert(sizeof(struct regCID) == 0x10, "eMMC register CID should be 0x10 bytes in size");
#endif

/*--------------------------------------------------------------------------}
{              INTERRUPT REGISTER TURN TO MASK BIT DEFINITIONS                }
{--------------------------------------------------------------------------*/
#define INT_AUTO_ERROR   0x01000000                                    // ACMD_ERR bit in register
#define INT_DATA_END_ERR 0x00400000                                    // DEND_ERR bit in register
#define INT_DATA_CRC_ERR 0x00200000                                    // DCRC_ERR bit in register
#define INT_DATA_TIMEOUT 0x00100000                                    // DTO_ERR bit in register
#define INT_INDEX_ERROR  0x00080000                                    // CBAD_ERR bit in register
#define INT_END_ERROR    0x00040000                                    // CEND_ERR bit in register
#define INT_CRC_ERROR    0x00020000                                    // CCRC_ERR bit in register
#define INT_CMD_TIMEOUT  0x00010000                                    // CTO_ERR bit in register
#define INT_ERR          0x00008000                                    // ERR bit in register
#define INT_ENDBOOT      0x00004000                                    // ENDBOOT bit in register
#define INT_BOOTACK      0x00002000                                    // BOOTACK bit in register
#define INT_RETUNE       0x00001000                                    // RETUNE bit in register
#define INT_CARD         0x00000100                                    // CARD bit in register
#define INT_READ_RDY     0x00000020                                    // READ_RDY bit in register
#define INT_WRITE_RDY    0x00000010                                    // WRITE_RDY bit in register
#define INT_BLOCK_GAP    0x00000004                                    // BLOCK_GAP bit in register
#define INT_DATA_DONE    0x00000002                                    // DATA_DONE bit in register
#define INT_CMD_DONE     0x00000001                                    // CMD_DONE bit in register
#define INT_ERROR_MASK   (INT_CRC_ERROR|INT_END_ERROR|INT_INDEX_ERROR| \
                          INT_DATA_TIMEOUT|INT_DATA_CRC_ERR|INT_DATA_END_ERR| \
                          INT_ERR|INT_AUTO_ERROR)
#define INT_ALL_MASK     (INT_CMD_DONE|INT_DATA_DONE|INT_READ_RDY|INT_WRITE_RDY|INT_ERROR_MASK)


/*--------------------------------------------------------------------------}
{                          SD CARD FREQUENCIES                                }
{--------------------------------------------------------------------------*/
#define FREQ_SETUP                400000  // 400 Khz
#define FREQ_NORMAL              25000000  // 25 Mhz

/*--------------------------------------------------------------------------}
{                          CMD 41 BIT SELECTIONS                                }
{--------------------------------------------------------------------------*/
#define ACMD41_HCS           0x40000000
#define ACMD41_SDXC_POWER    0x10000000
#define ACMD41_S18R          0x04000000
#define ACMD41_VOLTAGE       0x00ff8000
/* PI DOES NOT SUPPORT VOLTAGE SWITCH */
#define ACMD41_ARG_HC        (ACMD41_HCS|ACMD41_SDXC_POWER|ACMD41_VOLTAGE)//(ACMD41_HCS|ACMD41_SDXC_POWER|ACMD41_VOLTAGE|ACMD41_S18R)
#define ACMD41_ARG_SC        (ACMD41_VOLTAGE)   //(ACMD41_VOLTAGE|ACMD41_S18R)

/*--------------------------------------------------------------------------}
{                           SD CARD COMMAND RECORD                            }
{--------------------------------------------------------------------------*/
typedef struct EMMCCommand
{
    const char cmd_name[16];
    struct regCMDTM code;
    struct __attribute__((__packed__)) {
        unsigned use_rca : 1;                                        // @0        Command uses rca                                        
        unsigned reserved : 15;                                        // @1-15    Write as zero read as don't care
        u16 delay;                                                // @16-31    Delay to apply after command
    };
} EMMCCommand;

/*--------------------------------------------------------------------------}
{                        SD CARD COMMAND INDEX DEFINITIONS                    }
{--------------------------------------------------------------------------*/
#define IX_GO_IDLE_STATE    0
#define IX_ALL_SEND_CID     1
#define IX_SEND_REL_ADDR    2
#define IX_SET_DSR          3
#define IX_SWITCH_FUNC      4
#define IX_CARD_SELECT      5
#define IX_SEND_IF_COND     6
#define IX_SEND_CSD         7
#define IX_SEND_CID         8
#define IX_VOLTAGE_SWITCH   9
#define IX_STOP_TRANS       10
#define IX_SEND_STATUS      11
#define IX_GO_INACTIVE      12
#define IX_SET_BLOCKLEN     13
#define IX_READ_SINGLE      14
#define IX_READ_MULTI       15
#define IX_SEND_TUNING      16
#define IX_SPEED_CLASS      17
#define IX_SET_BLOCKCNT     18
#define IX_WRITE_SINGLE     19
#define IX_WRITE_MULTI      20
#define IX_PROGRAM_CSD      21
#define IX_SET_WRITE_PR     22
#define IX_CLR_WRITE_PR     23
#define IX_SND_WRITE_PR     24
#define IX_ERASE_WR_ST      25
#define IX_ERASE_WR_END     26
#define IX_ERASE            27
#define IX_LOCK_UNLOCK      28
#define IX_APP_CMD          29
#define IX_APP_CMD_RCA      30
#define IX_GEN_CMD          31

// Commands hereafter require APP_CMD.
#define IX_APP_CMD_START    32
#define IX_SET_BUS_WIDTH    32
#define IX_SD_STATUS        33
#define IX_SEND_NUM_WRBL    34
#define IX_SEND_NUM_ERS     35
#define IX_APP_SEND_OP_COND 36
#define IX_SET_CLR_DET      37
#define IX_SEND_SCR         38

/*--------------------------------------------------------------------------}
{                              SD CARD COMMAND TABLE                            }
{--------------------------------------------------------------------------*/
static EMMCCommand sdCommandTable[IX_SEND_SCR + 1] =  {
    [IX_GO_IDLE_STATE] =    { "GO_IDLE_STATE", .code.CMD_INDEX = 0x00, .code.CMD_RSPNS_TYPE = CMD_NO_RESP        , .use_rca = 0 , .delay = 0},
    [IX_ALL_SEND_CID] =        { "ALL_SEND_CID" , .code.CMD_INDEX = 0x02, .code.CMD_RSPNS_TYPE = CMD_136BIT_RESP    , .use_rca = 0 , .delay = 0},
    [IX_SEND_REL_ADDR] =    { "SEND_REL_ADDR", .code.CMD_INDEX = 0x03, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_SET_DSR] =            { "SET_DSR"      , .code.CMD_INDEX = 0x04, .code.CMD_RSPNS_TYPE = CMD_NO_RESP        , .use_rca = 0 , .delay = 0},
    [IX_SWITCH_FUNC] =        { "SWITCH_FUNC"  , .code.CMD_INDEX = 0x06, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_CARD_SELECT] =        { "CARD_SELECT"  , .code.CMD_INDEX = 0x07, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP , .use_rca = 1 , .delay = 0},
    [IX_SEND_IF_COND] =     { "SEND_IF_COND" , .code.CMD_INDEX = 0x08, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 100},
    [IX_SEND_CSD] =            { "SEND_CSD"     , .code.CMD_INDEX = 0x09, .code.CMD_RSPNS_TYPE = CMD_136BIT_RESP    , .use_rca = 1 , .delay = 0},
    [IX_SEND_CID] =            { "SEND_CID"     , .code.CMD_INDEX = 0x0A, .code.CMD_RSPNS_TYPE = CMD_136BIT_RESP    , .use_rca = 1 , .delay = 0},
    [IX_VOLTAGE_SWITCH] =    { "VOLT_SWITCH"  , .code.CMD_INDEX = 0x0B, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_STOP_TRANS] =        { "STOP_TRANS"   , .code.CMD_INDEX = 0x0C, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP , .use_rca = 0 , .delay = 0},
    [IX_SEND_STATUS] =        { "SEND_STATUS"  , .code.CMD_INDEX = 0x0D, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 1 , .delay = 0},
    [IX_GO_INACTIVE] =        { "GO_INACTIVE"  , .code.CMD_INDEX = 0x0F, .code.CMD_RSPNS_TYPE = CMD_NO_RESP        , .use_rca = 1 , .delay = 0},
    [IX_SET_BLOCKLEN] =        { "SET_BLOCKLEN" , .code.CMD_INDEX = 0x10, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_READ_SINGLE] =        { "READ_SINGLE"  , .code.CMD_INDEX = 0x11, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , 
                                               .code.CMD_ISDATA = 1  , .code.TM_DAT_DIR = 1,                       .use_rca = 0 , .delay = 0},
    [IX_READ_MULTI] =        { "READ_MULTI"   , .code.CMD_INDEX = 0x12, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , 
                                               .code.CMD_ISDATA = 1 ,  .code.TM_DAT_DIR = 1,
                                               .code.TM_BLKCNT_EN =1 , .code.TM_MULTI_BLOCK = 1,                   .use_rca = 0 , .delay = 0},
    [IX_SEND_TUNING] =        { "SEND_TUNING"  , .code.CMD_INDEX = 0x13, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_SPEED_CLASS] =        { "SPEED_CLASS"  , .code.CMD_INDEX = 0x14, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP , .use_rca = 0 , .delay = 0},
    [IX_SET_BLOCKCNT] =        { "SET_BLOCKCNT" , .code.CMD_INDEX = 0x17, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_WRITE_SINGLE] =        { "WRITE_SINGLE" , .code.CMD_INDEX = 0x18, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , 
                                               .code.CMD_ISDATA = 1  ,                                               .use_rca = 0 , .delay = 0},
    [IX_WRITE_MULTI] =        { "WRITE_MULTI"  , .code.CMD_INDEX = 0x19, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , 
                                               .code.CMD_ISDATA = 1  ,  
                                               .code.TM_BLKCNT_EN = 1, .code.TM_MULTI_BLOCK = 1,                   .use_rca = 0 , .delay = 0},
    [IX_PROGRAM_CSD] =        { "PROGRAM_CSD"  , .code.CMD_INDEX = 0x1B, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_SET_WRITE_PR] =        { "SET_WRITE_PR" , .code.CMD_INDEX = 0x1C, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP , .use_rca = 0 , .delay = 0},
    [IX_CLR_WRITE_PR] =        { "CLR_WRITE_PR" , .code.CMD_INDEX = 0x1D, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP , .use_rca = 0 , .delay = 0},
    [IX_SND_WRITE_PR] =        { "SND_WRITE_PR" , .code.CMD_INDEX = 0x1E, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_ERASE_WR_ST] =        { "ERASE_WR_ST"  , .code.CMD_INDEX = 0x20, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_ERASE_WR_END] =        { "ERASE_WR_END" , .code.CMD_INDEX = 0x21, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_ERASE] =            { "ERASE"        , .code.CMD_INDEX = 0x26, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP , .use_rca = 0 , .delay = 0},
    [IX_LOCK_UNLOCK] =        { "LOCK_UNLOCK"  , .code.CMD_INDEX = 0x2A, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_APP_CMD] =            { "APP_CMD"      , .code.CMD_INDEX = 0x37, .code.CMD_RSPNS_TYPE = CMD_NO_RESP        , .use_rca = 0 , .delay = 100},
    [IX_APP_CMD_RCA] =        { "APP_CMD"      , .code.CMD_INDEX = 0x37, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 1 , .delay = 0},
    [IX_GEN_CMD] =            { "GEN_CMD"      , .code.CMD_INDEX = 0x38, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},

    // APP commands must be prefixed by an APP_CMD.
    [IX_SET_BUS_WIDTH] =    { "SET_BUS_WIDTH", .code.CMD_INDEX = 0x06, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_SD_STATUS] =        { "SD_STATUS"    , .code.CMD_INDEX = 0x0D, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 1 , .delay = 0},
    [IX_SEND_NUM_WRBL] =    { "SEND_NUM_WRBL", .code.CMD_INDEX = 0x16, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_SEND_NUM_ERS] =        { "SEND_NUM_ERS" , .code.CMD_INDEX = 0x17, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_APP_SEND_OP_COND] =    { "SD_SENDOPCOND", .code.CMD_INDEX = 0x29, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 1000},
    [IX_SET_CLR_DET] =        { "SET_CLR_DET"  , .code.CMD_INDEX = 0x2A, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , .use_rca = 0 , .delay = 0},
    [IX_SEND_SCR] =            { "SEND_SCR"     , .code.CMD_INDEX = 0x33, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP     , 
                                               .code.CMD_ISDATA = 1  , .code.TM_DAT_DIR = 1,                       .use_rca = 0 , .delay = 0},
};

static const char* SD_TYPE_NAME[] = { "Unknown", "MMC", "Type 1", "Type 2 SC", "Type 2 HC" };

/*--------------------------------------------------------------------------}
{                          SD CARD DESCRIPTION RECORD                        }
{--------------------------------------------------------------------------*/
typedef struct SDDescriptor {
    struct regCID cid;                            // Card cid
    struct CSD csd;                                // Card csd
    struct regSCR scr;                            // Card scr
    u64 CardCapacity;                        // Card capacity expanded .. calculated from card details
    SDCARD_TYPE type;                            // Card type
    u32 rca;                                // Card rca
    struct regOCR ocr;                            // Card ocr
    u32 status;                            // Card last status

    EMMCCommand* lastCmd;
} SDDescriptor;

/*--------------------------------------------------------------------------}
{                        CURRENT SD CARD DATA STORAGE                        }
{--------------------------------------------------------------------------*/
static SDDescriptor sdCard = { 0 };


//**************************************************************************
// SD Card PUBLIC functions.
//**************************************************************************

static int pisd_fastclock = 0;


static void pisd_coherence(void)
{
  udelay(pisd_fastclock ? 2 : 20);
  do_arm1176jzfs_dsb();
  do_arm1176jzfs_isb();
}


static u32 pisd_readreg(u32 a)
{
  return ioread32((caddr_t)a);
}


static void pisd_writereg(u32 a, u32 v)
{
  iowrite32((caddr_t)a, v);
  pisd_coherence();
}


static int sdDebugResponse(int resp)
{
  LOG_DEBUG("eMMC: Status: %x, control1: %x, interrupt: %x\n",
    (unsigned int)EMMC_STATUS->Raw32, (unsigned int)EMMC_CONTROL1->Raw32,
    (unsigned int)EMMC_INTERRUPT->Raw32);
  LOG_DEBUG("eMMC: Command %s resp %x: %x %x %x %x\n",
    sdCard.lastCmd->cmd_name, (unsigned int)resp,(unsigned int)*EMMC_RESP3,
    (unsigned int)*EMMC_RESP2, (unsigned int)*EMMC_RESP1,
    (unsigned int)*EMMC_RESP0);
  return resp;
}

#define PISD_WFI_TIMEOUT_US 1000000
static int pisd_wfi(u64 max_us, u32 intr_mask)
{ 
  /* why are we not using CMD_DONE and ERR bits to signal completion? */
  u64 elapsed_us;
  u64 start;
  u32 mask;
  u32 sta;
  u32 intr;
  
  elapsed_us = 0;
  start = ticks(NULL);
  mask = intr_mask | INT_ERROR_MASK;
  intr = 0;

  intr = pisd_readreg(EMMC_INTERRUPT_REG);

  while (!(intr & mask) && elapsed_us < max_us) {
    elapsed_us = tick_diff_us(start, ticks(NULL));
    intr = pisd_readreg(EMMC_INTERRUPT_REG);
  }

  intr = pisd_readreg(EMMC_INTERRUPT_REG);

  if (elapsed_us >= max_us ||
      intr & (INT_CMD_TIMEOUT|INT_DATA_TIMEOUT) ||
      intr & INT_ERROR_MASK) {
    pisd_writereg(EMMC_INTERRUPT_REG, intr);
    return (intr & (INT_CMD_TIMEOUT|INT_DATA_TIMEOUT) || elapsed_us >= max_us)
      ? -EBUSY
      : -ENXIO;
  }

  pisd_writereg(EMMC_INTERRUPT_REG, intr_mask);
  return 0;
}


#define PISD_CMD_TIMEOUT_US 1000000
#define PISD_DAT_TIMEOUT_US  500000
static int pisd_waitfor(u64 max_us, u32 sta_mask)
{
  /* why are we not using CMD_DONE and ERR bits to signal completion? */
  u64 elapsed_us;
  u64 start;
  u32 sta;
  u32 intr;
 
  elapsed_us = 0;
  start = ticks(NULL);
  intr = 0;
 
  while (elapsed_us < max_us) {
    sta = pisd_readreg(EMMC_STATUS_REG);
    if (!(sta & sta_mask)) {
      intr = pisd_readreg(EMMC_INTERRUPT_REG);
      if (!(intr & INT_ERROR_MASK)) {
        return 0;
      } else {
        return -ENXIO;
      }
    }
 
    elapsed_us = tick_diff_us(start, ticks(NULL));
  }

  return -EBUSY;
}


/*-[INTERNAL: unpack_csd]---------------------------------------------------}
. Unpacks a CSD in Resp0,1,2,3 into the proper CSD structure we defined.
. 16Aug17 LdB
.--------------------------------------------------------------------------*/
static void unpack_csd(struct CSD* csd)
{
    u8 buf[16] = { 0 };

    /* Fill buffer CSD comes IN MSB so I will invert so its sort of right way up so I can debug it */
    __attribute__((aligned(4))) u32* p;
    p = (u32*)&buf[12];
    *p = *EMMC_RESP0;
    p = (u32*)&buf[8];
    *p = *EMMC_RESP1;
    p = (u32*)&buf[4];
    *p = *EMMC_RESP2;
    p = (u32*)&buf[0];
    *p = *EMMC_RESP3;

    /* Display raw CSD - values of my SANDISK ultra 16GB shown under each */
    LOG_DEBUG("CSD Contents : %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n",
         buf[2],  buf[1],  buf[0],  buf[7],  buf[6],  buf[5],  buf[4],
        /*  40       e0       00       32       5b       59       00          */
        buf[11], buf[10],  buf[9],  buf[8], buf[15], buf[14], buf[13], buf[12]);
        /*  00       73       a7       7f       80       0a       40       00 */

    /* Populate CSD structure */
    csd->csd_structure = (buf[2] & 0xc0) >> 6;                                // @126-127 ** correct 
    csd->spec_vers = buf[2] & 0x3F;                                            // @120-125 ** correct
    csd->taac = buf[1];                                                        // @112-119 ** correct
    csd->nsac = buf[0];                                                        // @104-111 ** correct
    csd->tran_speed = buf[7];                                                // @96-103  ** correct
    csd->ccc = (((u16)buf[6]) << 4) | ((buf[5] & 0xf0) >> 4);            // @84-95   ** correct
    csd->read_bl_len = buf[5] & 0x0f;                                        // @80-83   ** correct
    csd->read_bl_partial = (buf[4] & 0x80) ? 1 : 0;                            // @79        ** correct
    csd->write_blk_misalign = (buf[4] & 0x40) ? 1 : 0;                        // @78      ** correct
    csd->read_blk_misalign = (buf[4] & 0x20) ? 1 : 0;                        // @77        ** correct
    csd->dsr_imp = (buf[4] & 0x10) ? 1 : 0;                                    // @76        ** correct

    if (csd->csd_structure == 0x1) {                                        // CSD VERSION 2.0 
        /* Basically absorbs bottom of buf[4] to align to next byte */        // @@75-70  ** Correct
        csd->ver2_c_size = (u32)(buf[11] & 0x3F) << 16;                // @69-64
        csd->ver2_c_size += (u32)buf[10] << 8;                            // @63-56
        csd->ver2_c_size += (u32)buf[9];                                // @55-48
        sdCard.CardCapacity = csd->ver2_c_size;
        sdCard.CardCapacity *= (512 * 1024);                                // Calculate Card capacity
    }
    else {                                                                    // CSD VERSION 1.0
        csd->c_size = (u32)(buf[4] & 0x03) << 8;
        csd->c_size += (u32)buf[11];
        csd->c_size <<= 2;
        csd->c_size += (buf[10] & 0xc0) >> 6;                                // @62-73
        csd->vdd_r_curr_min = (buf[10] & 0x38) >> 3;                        // @59-61
        csd->vdd_r_curr_max = buf[10] & 0x07;                                // @56-58
        csd->vdd_w_curr_min = (buf[9] & 0xe0) >> 5;                            // @53-55
        csd->vdd_w_curr_max = (buf[9] & 0x1c) >> 2;                            // @50-52    
        csd->c_size_mult = ((buf[9] & 0x03) << 1) | ((buf[8] & 0x80) >> 7);    // @47-49
        sdCard.CardCapacity = (csd->c_size + 1) * (1 << (csd->c_size_mult + 2)) * (1 << csd->read_bl_len);
    }

    csd->erase_blk_en = (buf[8] & 0x40) >> 6;                                // @46
    csd->sector_size = ((buf[15] & 0x80) >> 1) | (buf[8] & 0x3F);            // @39-45
    csd->wp_grp_size = buf[15] & 0x7f;                                        // @32-38
    csd->wp_grp_enable = (buf[14] & 0x80) ? 1 : 0;                            // @31  
    csd->default_ecc = (buf[14] & 0x60) >> 5;                                // @29-30
    csd->r2w_factor = (buf[14] & 0x1c) >> 2;                                // @26-28   ** correct
    csd->write_bl_len = ((buf[14] & 0x03) << 2) | ((buf[13] & 0xc0) >> 6);  // @22-25   **correct
    csd->write_bl_partial = (buf[13] & 0x20) ? 1 : 0;                        // @21 
                                                                            // @16-20 are reserved
    csd->file_format_grp = (buf[12] & 0x80) ? 1 : 0;                        // @15
    csd->copy = (buf[12] & 0x40) ? 1 : 0;                                    // @14
    csd->perm_write_protect = (buf[12] & 0x20) ? 1 : 0;                        // @13
    csd->tmp_write_protect = (buf[12] & 0x10) ? 1 : 0;                        // @12
    csd->file_format = (buf[12] & 0x0c) >> 2;                                // @10-11    **correct
    csd->ecc = buf[12] & 0x03;                                                // @8-9      **corrrect   

    LOG_DEBUG("  csd_structure=%d\t  spec_vers=%d\t  taac=%x\t nsac=%x\t  tran_speed=%x\t  ccc=%x\n"
        "  read_bl_len=%d\t  read_bl_partial=%d\t  write_blk_misalign=%d\t  read_blk_misalign=%d\n"
        "  dsr_imp=%d\t  sector_size =%d\t  erase_blk_en=%d\n",
        csd->csd_structure, csd->spec_vers, csd->taac, csd->nsac, csd->tran_speed, csd->ccc,
        csd->read_bl_len, csd->read_bl_partial, csd->write_blk_misalign, csd->read_blk_misalign,
        csd->dsr_imp, csd->sector_size, csd->erase_blk_en);

    if (csd->csd_structure == 0x1) {
        LOG_DEBUG("CSD 2.0: ver2_c_size = %d\t  card capacity: %llu\n",
            csd->ver2_c_size, sdCard.CardCapacity);
    }
    else {
        LOG_DEBUG("CSD 1.0: c_size = %d\t  c_size_mult=%d\t card capacity: %lu\n"
            "  vdd_r_curr_min = %d\t  vdd_r_curr_max=%d\t  vdd_w_curr_min = %d\t  vdd_w_curr_max=%d\n",
            csd->c_size, csd->c_size_mult, sdCard.CardCapacity,
            csd->vdd_r_curr_min, csd->vdd_r_curr_max, csd->vdd_w_curr_min, csd->vdd_w_curr_max);
    }

    LOG_DEBUG("  wp_grp_size=%d\t  wp_grp_enable=%d\t  default_ecc=%d\t  r2w_factor=%d\n"
        "  write_bl_len=%d\t  write_bl_partial=%d\t  file_format_grp=%d\t  copy=%d\n"
        "  perm_write_protect=%d\t  tmp_write_protect=%d\t  file_format=%d\t  ecc=%d\n",
        csd->wp_grp_size, csd->wp_grp_enable, csd->default_ecc, csd->r2w_factor,
        csd->write_bl_len, csd->write_bl_partial, csd->file_format_grp, csd->copy,
        csd->perm_write_protect, csd->tmp_write_protect, csd->file_format, csd->ecc);
}

/*-[INTERNAL: sdSendCommandP]-----------------------------------------------}
. Send command and handle response.
. 10Aug17 LdB
.--------------------------------------------------------------------------*/
#define R1_ERRORS_MASK       0xfff9c004
static int sdSendCommandP( EMMCCommand* cmd, u32 arg )
{
    SDRESULT res;

    /* Check for command in progress */
    if (pisd_waitfor(PISD_CMD_TIMEOUT_US, CMD_INHIBIT_MASK) != 0) return SD_BUSY; // Check command wait

    LOG_DEBUG("eMMC: Sending command %s code %x arg %x\n",
        cmd->cmd_name, (unsigned int)cmd->code.CMD_INDEX, (unsigned int)arg);
    sdCard.lastCmd = cmd;

    /* Clear interrupt flags.  This is done by setting the ones that are currently set */
    EMMC_INTERRUPT->Raw32 = EMMC_INTERRUPT->Raw32;                    // Clear interrupts
    pisd_coherence();

    /* Set the argument and the command code, Some commands require a delay before reading the response */
    *EMMC_ARG1 = arg;                                                // Set argument to SD card
    pisd_coherence();
    *EMMC_CMDTM = cmd->code;                                        // Send command to SD card                                
    pisd_coherence();
    if ( cmd->delay ) udelay(cmd->delay);                        // Wait for required delay

    /* Wait until command complete interrupt */
    if (pisd_wfi(PISD_WFI_TIMEOUT_US, INT_CMD_DONE)) return SD_TIMEOUT;

    /* Get response from RESP0 */
    u32 resp0 = *EMMC_RESP0;                                    // Fetch SD card response 0 to command

    /* Handle response types for command */
    switch ( cmd->code.CMD_RSPNS_TYPE) {
        // no response
        case CMD_NO_RESP:
            return SD_OK;                                            // Return okay then

        case CMD_BUSY48BIT_RESP:
            sdCard.status = resp0;
            // Store the card state.  Note that this is the state the card was in before the
            // command was accepted, not the new state.
            //sdCard.cardState = (resp0 & ST_CARD_STATE) >> R1_CARD_STATE_SHIFT;
            return resp0 & R1_ERRORS_MASK;

        // RESP0 contains card status, no other data from the RESP* registers.
        // Return value non-zero if any error flag in the status value.
        case CMD_48BIT_RESP:
            switch (cmd->code.CMD_INDEX) {
                case 0x03:                                            // SEND_REL_ADDR command
                    // RESP0 contains RCA and status bits 23,22,19,12:0
                    sdCard.rca = resp0 & 0xffff0000;                // RCA[31:16] of response
                    sdCard.status = ((resp0 & 0x00001fff)) |        // 12:0 map directly to status 12:0
                        ((resp0 & 0x00002000) << 6) |                // 13 maps to status 19 ERROR
                        ((resp0 & 0x00004000) << 8) |                // 14 maps to status 22 ILLEGAL_COMMAND
                        ((resp0 & 0x00008000) << 8);                // 15 maps to status 23 COM_CRC_ERROR
                    // Store the card state.  Note that this is the state the card was in before the
                    // command was accepted, not the new state.
                    // sdCard.cardState = (resp0 & ST_CARD_STATE) >> R1_CARD_STATE_SHIFT;
                    return sdCard.status & R1_ERRORS_MASK;
                case 0x08:                                            // SEND_IF_COND command
                    // RESP0 contains voltage acceptance and check pattern, which should match
                    // the argument.
                    sdCard.status = 0;
                    return resp0 == arg ? SD_OK : SD_ERROR;
                    // RESP0 contains OCR register
                    // TODO: What is the correct time to wait for this?
                case 0x29:                                            // SD_SENDOPCOND command
                    sdCard.status = 0;
                    sdCard.ocr.Raw32 = resp0;
                    return SD_OK;
                default:
                    sdCard.status = resp0;
                    // Store the card state.  Note that this is the state the card was in before the
                    // command was accepted, not the new state.
                    //sdCard.cardState = (resp0 & ST_CARD_STATE) >> R1_CARD_STATE_SHIFT;
                    return resp0 & R1_ERRORS_MASK;
            }
        // RESP0..3 contains 128 bit CID or CSD shifted down by 8 bits as no CRC
        // Note: highest bits are in RESP3.
        case CMD_136BIT_RESP:        
            sdCard.status = 0;
            if (cmd->code.CMD_INDEX == 0x09) {
                unpack_csd(&sdCard.csd);
            } else {
                u32* data = (u32*)(uptr_t)&sdCard.cid;
                data[3] = resp0;
                data[2] = *EMMC_RESP1;
                data[1] = *EMMC_RESP2;
                data[0] = *EMMC_RESP3;
            }
            return SD_OK;
    }

    return SD_ERROR;
}


/*-[INTERNAL: sdSendAppCommand]---------------------------------------------}
. Send APP command and handle response.
. 10Aug17 LdB
.--------------------------------------------------------------------------*/
#define ST_APP_CMD           0x00000020
static SDRESULT sdSendAppCommand (void)
{
    SDRESULT resp;
    // If no RCA, send the APP_CMD and don't look for a response.
    if ( !sdCard.rca )
        sdSendCommandP(&sdCommandTable[IX_APP_CMD], 0x00000000);
    // If there is an RCA, include that in APP_CMD and check card accepted it.
    else {
        if( (resp = sdSendCommandP(&sdCommandTable[IX_APP_CMD_RCA], sdCard.rca)) ) 
            return sdDebugResponse(resp);
        // Debug - check that status indicates APP_CMD accepted.
        if( !(sdCard.status & ST_APP_CMD) ) return SD_ERROR;
    }
    return SD_OK;
}

 /*-[INTERNAL: sdSendCommand]------------------------------------------------}
 . Send a command with no argument. RCA automatically added if required.
 . APP_CMD sent automatically if required.
 . 10Aug17 LdB
 .--------------------------------------------------------------------------*/
static SDRESULT sdSendCommand ( int index )
{
    // Issue APP_CMD if needed.
    SDRESULT resp;
    if ( index >= IX_APP_CMD_START && (resp = sdSendAppCommand()) )
        return sdDebugResponse(resp);

    // Get the command and set RCA if required.
    EMMCCommand* cmd = &sdCommandTable[index];
    u32 arg = 0;
    if( cmd->use_rca == 1 ) arg = sdCard.rca;

    if( (resp = sdSendCommandP(cmd, arg)) ) return resp;

    // Check that APP_CMD was correctly interpreted.
    if( index >= IX_APP_CMD_START && sdCard.rca && !(sdCard.status & ST_APP_CMD) )
        return SD_ERROR_APP_CMD;

    return resp;
}

 /*-[INTERNAL: sdSendCommandA]-----------------------------------------------}
 . Send a command with argument. APP_CMD sent automatically if required.
 . APP_CMD sent automatically if required.
 . 10Aug17 LdB
 .--------------------------------------------------------------------------*/
static SDRESULT sdSendCommandA ( int index, u32 arg )
{
    // Issue APP_CMD if needed.
    SDRESULT resp;
    if( index >= IX_APP_CMD_START && (resp = sdSendAppCommand()) )
        return sdDebugResponse(resp);

    // Get the command and pass the argument through.
    if( (resp = sdSendCommandP(&sdCommandTable[index],arg)) ) return resp;

    // Check that APP_CMD was correctly interpreted.
    if( index >= IX_APP_CMD_START && sdCard.rca && !(sdCard.status & ST_APP_CMD) )
        return SD_ERROR_APP_CMD;

    return resp;
}


/*-[INTERNAL: sdReadSCR]----------------------------------------------------}
. Read card's SCR
. APP_CMD sent automatically if required.
. 10Aug17 LdB
.--------------------------------------------------------------------------*/
static SDRESULT sdReadSCR (void)
{
    // SEND_SCR command is like a READ_SINGLE but for a block of 8 bytes.
    // Ensure that any data operation has completed before reading the block.
    if (pisd_waitfor(PISD_DAT_TIMEOUT_US, DAT_INHIBIT_MASK)) return SD_TIMEOUT;

    // Set BLKSIZECNT to 1 block of 8 bytes, send SEND_SCR command
    EMMC_BLKSIZECNT->BLKCNT = 1;
    EMMC_BLKSIZECNT->BLKSIZE = 8;
    int resp;
    if( (resp = sdSendCommand(IX_SEND_SCR)) ) return sdDebugResponse(resp);

    // Wait for READ_RDY interrupt.
    if (pisd_wfi(PISD_WFI_TIMEOUT_US, INT_READ_RDY)) {
        printf("eMMC: Timeout waiting for ready to read\n");
        return sdDebugResponse(SD_TIMEOUT);
    }

    // Allow maximum of 100ms for the read operation.
    int numRead = 0, count = 100000;
    while( numRead < 2 )  {
        if (EMMC_STATUS->READ_TRANSFER) {
            //sdCard.scr[numRead++] = *EMMC_DATA;
            if (numRead == 0) sdCard.scr.Raw32_Lo = *EMMC_DATA;
                else sdCard.scr.Raw32_Hi = *EMMC_DATA;
            numRead++;
        } else {
            udelay(1);
            if( --count == 0 ) break;
        }
    }

    // If SCR not fully read, the operation timed out.
    if( numRead != 2 )
    {
        printf("eMMC: SEND_SCR ERR: %x %x %x\n", 
                (unsigned int)EMMC_STATUS->Raw32, 
                (unsigned int)EMMC_INTERRUPT->Raw32, 
                (unsigned int)*EMMC_RESP0);
        printf("eMMC: Reading SCR, only read %d words\n", numRead);
        return SD_TIMEOUT;
    }

    return SD_OK;
}


/*-[INTERNAL: fls_u32]-------------------------------------------------}
. Find Last Set bit in given u32 value. That is find the bit index of 
. the MSB that is set in the value. 
. RETURN: 0 - 32 are only possible answers given u32 is 32 bits.
. 10Aug17 LdB
.--------------------------------------------------------------------------*/
static u32 fls_u32 (u32 x) 
{
    u32 r = 32;                                            // Start at 32
    if (!x)  return 0;                                                // If x is zero answer must be zero
    if (!(x & 0xffff0000u)) {                                        // If none of the upper word bits are set
        x <<= 16;                                                    // We can roll it up 16 bits
        r -= 16;                                                    // Reduce r by 16
    }
    if (!(x & 0xff000000u)) {                                        // If none of uppermost byte bits are set
        x <<= 8;                                                    // We can roll it up by 8 bits
        r -= 8;                                                        // Reduce r by 8
    }
    if (!(x & 0xf0000000u)) {                                        // If none of the uppermost 4 bits are set
        x <<= 4;                                                    // We can roll it up by 4 bits
        r -= 4;                                                        // Reduce r by 4
    }
    if (!(x & 0xc0000000u)) {                                        // If none of the uppermost 2 bits are set
        x <<= 2;                                                    // We can roll it up by 2 bits
        r -= 2;                                                        // Reduce r by 2
    }
    if (!(x & 0x80000000u)) {                                        // If the uppermost bit is not set
        x <<= 1;                                                    // We can roll it up by 1 bit
        r -= 1;                                                        // Reduce r by 1
    }
    return r;                                                        // Return the number of the uppermost set bit
}


/*-[INTERNAL: sdGetClockDivider]--------------------------------------------}
. Get clock divider for the given requested frequency. This is calculated 
. relative to the SD base clock of 41.66667Mhz
. RETURN: 3 - 0x3FF are only possible answers for the divisor
. 10Aug17 LdB
.--------------------------------------------------------------------------*/
static u32 sdGetClockDivider (u32 freq) 
{
    u32 base_hz = 41666667;                                    // If not set, default to 41.66667Mhz
    (void)bcm283x_mbox_get_sdcard_clock(&base_hz);
    if (!base_hz) {
        base_hz = 41666667;
    }
    LOG_DEBUG("eMMC: Base clock: %uHz\n", base_hz);
    u32 divisor = (base_hz + freq - 1) / freq;
    if (divisor > 0x3FF) divisor = 0x3FF;                            // Constrain divisor to max 0x3FF
    if (EMMC_SLOTISR_VER->SDVERSION < 2) {                            // Any version less than HOST SPECIFICATION 3 (Aka numeric 2)                        
        u32 shiftcount = fls_u32(divisor);            // Only 8 bits and set pwr2 div on Hosts specs 1 & 2
        if (shiftcount > 0) shiftcount--;                            // Note the offset of shift by 1 (look at the spec)
        if (shiftcount > 7) shiftcount = 7;                            // It's only 8 bits maximum on HOST_SPEC_V2
        divisor = ((u32)1 << shiftcount);                        // Version 1,2 take power 2
    } else if (divisor < 3) divisor = 4;                            // Set minimum divisor limit
    LOG_DEBUG("eMMC: Divisor = %d, Freq Set = %d\n", (int)divisor, (int)(41666667/divisor));
    return divisor;                                                    // Return divisor that would be required
}


static u32 get_clock_divider(u32 freq)
{
  return sdGetClockDivider(freq);
}



/*-[INTERNAL: sdGetClockDivider]--------------------------------------------}
. Set the SD clock to the given frequency from the 41.66667Mhz base clock
. RETURN: SD_ERROR_CLOCK - A fatal error occurred setting the clock
.          SD_OK - the clock change to given frequency
. 10Aug17 LdB
.--------------------------------------------------------------------------*/
static SDRESULT sdSetClock (u32 freq)
{
    u64 td = 0;                                                // Zero time difference
    u64 start_time = 0;                                        // Zero start time
    LOG_DEBUG("eMMC: setting clock speed to %u.\n", freq);
    while ((EMMC_STATUS->CMD_INHIBIT || EMMC_STATUS->DAT_INHIBIT)    // Data inhibit signal
             && (td < 100000))                                        // Timeout not reached
    {
        if (!start_time) start_time = ticks(NULL);                    // If start time not set the set start time
            else td = tick_diff_us(start_time, ticks(NULL));            // Time difference between start time and now
    }
    if (td >= 100000) {                                                // Timeout waiting for inghibit flags
        printf("eMMC: Set clock: timeout waiting for inhibit flags. Status %x.\n",
            (unsigned int)EMMC_STATUS->Raw32);
        return SD_ERROR_CLOCK;                                        // Return clock error
    }

    /* Switch clock off */
    EMMC_CONTROL1->CLK_EN = 0;                                        // Disable clock
    pisd_coherence();

    /* Request the divisor for new clock setting */
    u32 cdiv = get_clock_divider(freq);                    // Fetch divisor for new frequency
    u32 divlo = (cdiv & 0xff) << 8;                        // Create divisor low bits value
    u32 divhi = ((cdiv & 0x300) >> 2);                    // Create divisor high bits value

    /* Set new clock frequency by setting new divisor */
    EMMC_CONTROL1->Raw32 = (EMMC_CONTROL1->Raw32 & 0xffff001f) | divlo | divhi;
    pisd_coherence();

    /* Enable the clock. */
    EMMC_CONTROL1->CLK_EN = 1;                                        // Enable the clock
    pisd_coherence();

    /* Wait for clock to be stablize */
    td = 0;                                                            // Zero time difference
    start_time = 0;                                                    // Zero start time
    while (!(EMMC_CONTROL1->CLK_STABLE)                                // Clock not stable yet
        && (td < 100000))                                            // Timeout not reached
    {
        if (!start_time) start_time = ticks(NULL);                    // If start time not set the set start time
            else td = tick_diff_us(start_time, ticks(NULL));            // Time difference between start time and now
    }
    if (td >= 100000) {                                                // Timeout waiting for stability flag
        printf("eMMC: ERROR: failed to get stable clock.\n");
        return SD_ERROR_CLOCK;                                        // Return clock error
    }
    return SD_OK;                                                    // Clock frequency set worked
}

/*-[INTERNAL: sdResetCard]--------------------------------------------------}
. Reset the SD Card
. RETURN: SD_ERROR_RESET - A fatal error occurred resetting the SD Card
.          SD_OK - SD Card reset correctly
. 10Aug17 LdB
.--------------------------------------------------------------------------*/
static SDRESULT sdResetCard (void)
{
    SDRESULT resp;
    u64 td = 0;                                                // Zero time difference
    u64 start_time = 0;                                        // Zero start time


    /* Send reset host controller and wait for complete */
    EMMC_CONTROL0->Raw32 = 0;                                        // Zero control0 register
    pisd_coherence();
    EMMC_CONTROL1->Raw32 = 0;                                        // Zero control1 register
    pisd_coherence();
    EMMC_CONTROL1->SRST_HC = 1;                                        // Reset the complete host circuit
    pisd_coherence();
      //EMMC_CONTROL2->UHSMODE = SDR12;
    LOG_DEBUG("eMMC: reset card.\n");
    while ((EMMC_CONTROL1->SRST_HC)                                    // Host circuit reset not clear
            && (td < 100000))                                        // Timeout not reached
    {
        if (!start_time) start_time = ticks(NULL);                    // If start time not set the set start time
            else td = tick_diff_us(start_time, ticks(NULL));            // Time difference between start time and now
    }
    if (td >= 100000) {                                                // Timeout waiting for reset flag
        printf("eMMC: ERROR: failed to reset.\n");
        return SD_ERROR_RESET;                                        // Return reset SD Card error
    }

    /* Enable internal clock and set data timeout */
    EMMC_CONTROL1->DATA_TOUNIT = 0xE;                                // Maximum timeout value
    pisd_coherence();
    EMMC_CONTROL1->CLK_INTLEN = 1;                                    // Enable internal clock
    pisd_coherence();

    /* Set clock to setup frequency */
    if ( (resp = sdSetClock(FREQ_SETUP)) ) return resp;                // Set low speed setup frequency (400Khz)
    pisd_fastclock = 0;

    /* Enable interrupts for command completion values */
    EMMC_IRPT_EN->Raw32   = 0xffffffff;
    pisd_coherence();
    EMMC_IRPT_MASK->Raw32 = 0xffffffff;
    pisd_coherence();

    /* Reset our card structure entries */
    sdCard.rca = 0;                                                    // Zero rca
    sdCard.ocr.Raw32 = 0;                                            // Zero ocr
    sdCard.lastCmd = 0;                                                // Zero lastCmd
    sdCard.status = 0;                                                // Zero status
    sdCard.type = SD_TYPE_UNKNOWN;                                    // Set card type unknown

    /* Send GO_IDLE_STATE to card */
    resp = sdSendCommand(IX_GO_IDLE_STATE);                            // Send GO idle state

    return resp;                                                    // Return response
}

 /*-[INTERNAL: sdAppSendOpCond]---------------------------------------------}
 . Common routine for APP_SEND_OP_COND
 . This is used for both SC and HC cards based on the parameter
 . 10Aug17 LdB
 .-------------------------------------------------------------------------*/
static SDRESULT sdAppSendOpCond (u32 arg )
{
    // Send APP_SEND_OP_COND with the given argument (for SC or HC cards).
    // Note: The host shall set ACMD41 timeout more than 1 second to abort repeat of issuing ACMD41
    SDRESULT  resp;
    if( (resp = sdSendCommandA(IX_APP_SEND_OP_COND,arg)) && resp != SD_TIMEOUT )
    {
        printf("eMMC: ACMD41 returned non-timeout error %d\n",resp);
            return resp;
    }
    int count = 6;
    while( (sdCard.ocr.card_power_up_busy == 0) && count-- )
    {
        //scPrintf("eMMC: Retrying ACMD SEND_OP_COND status %x\n",*EMMC_STATUS);
        udelay(400000);
        if( (resp = sdSendCommandA(IX_APP_SEND_OP_COND,arg)) && resp != SD_TIMEOUT )
        {
            printf("eMMC: ACMD41 returned non-timeout error %d\n",resp);
            return resp;
        }
    }

    // Return timeout error if still not busy.
    if( sdCard.ocr.card_power_up_busy == 0 )
    return SD_TIMEOUT;

    // Pi is 3.3v SD only so check that one voltage values around 3.3v was returned.
    if(  sdCard.ocr.voltage3v2to3v3 == 0  && sdCard.ocr.voltage3v3to3v4 == 0)
    return SD_ERROR_VOLTAGE;

    return SD_OK;
}

/*-[sdTransferBlocks]-------------------------------------------------------}
. Transfer the count blocks starting at given block to/from SD Card.
. 21Aug17 LdB
.--------------------------------------------------------------------------*/
SDRESULT sdTransferBlocks (u32 startBlock, u32 numBlocks, u8* buffer, int write )
{
    if ( sdCard.type == SD_TYPE_UNKNOWN ) return SD_NO_RESP;        // If card not known return error
    if (pisd_waitfor(PISD_DAT_TIMEOUT_US, DAT_INHIBIT_MASK)) return SD_TIMEOUT; // Ensure any data operation has completed before doing the transfer.

    // Work out the status, interrupt and command values for the transfer.
    int readyInt = write ? INT_WRITE_RDY : INT_READ_RDY;

    int transferCmd = write ? ( numBlocks == 1 ? IX_WRITE_SINGLE : IX_WRITE_MULTI) :
                            ( numBlocks == 1 ? IX_READ_SINGLE : IX_READ_MULTI);

    // If more than one block to transfer, and the card supports it,
    // send SET_BLOCK_COUNT command to indicate the number of blocks to transfer.
    SDRESULT resp;
    if ( numBlocks > 1 &&
        (sdCard.scr.CMD_SUPPORT == CMD_SUPP_SET_BLKCNT) &&
        (resp = sdSendCommandA(IX_SET_BLOCKCNT, numBlocks)) ) return sdDebugResponse(resp);

    // Address is different depending on the card type.
    // HC pass address as block # so just pass it thru.
    // SC pass address so need to multiply by 512 which is shift left 9.
    u32 blockAddress = sdCard.type == SD_TYPE_2_SC ? (u32)(startBlock << 9) : (u32)startBlock;

    // Set BLKSIZECNT to number of blocks * 512 bytes, send the read or write command.
    // Once the data transfer has started and the TM_BLKCNT_EN bit in the CMDTM register is
    // set the EMMC module automatically decreases the BLKCNT value as the data blocks
    // are transferred and stops the transfer once BLKCNT reaches 0.
    // TODO: TM_AUTO_CMD12 - is this needed?  What effect does it have?
    EMMC_BLKSIZECNT->BLKCNT = numBlocks;
    pisd_coherence();
    EMMC_BLKSIZECNT->BLKSIZE = 512;
    pisd_coherence();
    if ((resp = sdSendCommandA(transferCmd, blockAddress))) {
        return sdDebugResponse(resp);
    }

    // Transfer all blocks.
    u32 blocksDone = 0;
    while ( blocksDone < numBlocks )
    {
        // Wait for ready interrupt for the next block.
        if (pisd_wfi(PISD_WFI_TIMEOUT_US, readyInt)) {
            printf("eMMC: Timeout waiting for ready to read\n");
            return sdDebugResponse(SD_TIMEOUT);
        }

        // Handle non-word-aligned buffers byte-by-byte.
        // Note: the entire block is sent without looking at status registers.
        if ((uptr_t)buffer & 0x03) {
            for (u32 i = 0; i < 512; i++ ) {
                if ( write ) {
                    u32 data = (buffer[i]      );
                    data |=    (buffer[i+1] << 8 );
                    data |=    (buffer[i+2] << 16);
                    data |=    (buffer[i+3] << 24);
                    *EMMC_DATA = data;
                } else {
                    u32 data = *EMMC_DATA;
                    buffer[i] =   (data      ) & 0xff;
                    buffer[i+1] = (data >> 8 ) & 0xff;
                    buffer[i+2] = (data >> 16) & 0xff;
                    buffer[i+3] = (data >> 24) & 0xff;
                }
            }
        }
        // Handle word-aligned buffers more efficiently.
        // Hopefully people smart enough to privide aligned data buffer
        else {
            u32* intbuff = (u32*)buffer;
            for (u32 i = 0; i < 128; i++ ) {
                if ( write ) *EMMC_DATA = intbuff[i];
                    else intbuff[i] = *EMMC_DATA;
            }
        }

        blocksDone++;
        buffer += 512;
    }

    // If not all bytes were read, the operation timed out.
    if( blocksDone != numBlocks ) {
        printf("eMMC: Transfer error only done %d/%d blocks\n",blocksDone,numBlocks);
        LOG_DEBUG("eMMC: Transfer: %x %x %x %x\n", (unsigned int)EMMC_STATUS->Raw32,
            (unsigned int)EMMC_INTERRUPT->Raw32, (unsigned int)*EMMC_RESP0, 
            (unsigned int)EMMC_BLKSIZECNT->Raw32);
        if( !write && numBlocks > 1 && (resp = sdSendCommand(IX_STOP_TRANS)) )
            LOG_DEBUG("eMMC: Error response from stop transmission: %d\n",resp);

        return SD_TIMEOUT;
    }

    // For a write operation, ensure DATA_DONE interrupt before we stop transmission.
    if (write && pisd_wfi(PISD_WFI_TIMEOUT_US, INT_DATA_DONE)) {
        printf("eMMC: Timeout waiting for data done\n");
        return sdDebugResponse(SD_TIMEOUT);
    }

    // For a multi-block operation, if SET_BLOCKCNT is not supported, we need to indicate
    // that there are no more blocks to be transferred.
    if( (numBlocks > 1) && (sdCard.scr.CMD_SUPPORT != CMD_SUPP_SET_BLKCNT) &&
        (resp = sdSendCommand(IX_STOP_TRANS)) ) return sdDebugResponse(resp);

    return SD_OK;
}

/*-[sdClearBlocks]----------------------------------------------------------}
. Clears the count blocks starting at given block from SD Card.
. 21Aug17 LdB
.--------------------------------------------------------------------------*/
SDRESULT sdClearBlocks(u32 startBlock , u32 numBlocks)
{
    if (sdCard.type == SD_TYPE_UNKNOWN) return SD_NO_RESP;

    // Ensure that any data operation has completed before doing the transfer.
    if (pisd_waitfor(PISD_DAT_TIMEOUT_US, DAT_INHIBIT_MASK)) return SD_TIMEOUT;

    // Address is different depending on the card type.
    // HC pass address as block # which is just address/512.
    // SC pass address straight through.
    u32 startAddress = sdCard.type == SD_TYPE_2_SC ? (u32)(startBlock << 9) : (u32)startBlock;
    u32 endAddress = sdCard.type == SD_TYPE_2_SC ? (u32)( (startBlock +numBlocks) << 9) : (u32)(startBlock + numBlocks);
    SDRESULT resp;
    LOG_DEBUG("eMMC: erasing blocks from %d to %d\n", startAddress, endAddress);
    if ( (resp = sdSendCommandA(IX_ERASE_WR_ST,startAddress)) ) return sdDebugResponse(resp);
    if ( (resp = sdSendCommandA(IX_ERASE_WR_END,endAddress)) ) return sdDebugResponse(resp);
    if ( (resp = sdSendCommand(IX_ERASE)) ) return sdDebugResponse(resp);

    // Wait for data inhibit status to drop.
    int count = 1000000;
    while( EMMC_STATUS->DAT_INHIBIT )
    {
    if ( --count == 0 )
        {
        printf("eMMC: Timeout waiting for erase: %x %x\n",
            (unsigned int)EMMC_STATUS->Raw32, (unsigned int)EMMC_INTERRUPT->Raw32);
        return SD_TIMEOUT;
        }

        udelay(10);
    }

    //scPrintf("eMMC: completed erase command int %x\n",*EMMC_INTERRUPT);

    return SD_OK;
}

/*--------------------------------------------------------------------------}
{                    PARTITION DECRIPTION BLOCK STRUCTURE                    }
{--------------------------------------------------------------------------*/
//Structure to access info of a partition on the disk
struct __attribute__((packed, aligned(2))) partition_info {
    u8        status;                            // 0x80 - active partition
    u8        headStart;                        // starting head
    u16    cylSectStart;                    // starting cylinder and sector
    u8        type;                            // partition type (01h = 12bit FAT, 04h = 16bit FAT, 05h = Ex MSDOS, 06h = 16bit FAT (>32Mb), 0Bh = 32bit FAT (<2048GB))
    u8        headEnd;                        // ending head of the partition
    u16    cylSectEnd;                        // ending cylinder and sector
    u32    firstSector;                    // total sectors between MBR & the first sector of the partition
    u32    sectorsTotal;                    // size of this partition in sectors
};

/*--------------------------------------------------------------------------}
{                     MASTER BOOT RECORD BLOCK STRUCTURE                        }
{--------------------------------------------------------------------------*/
// Structure to access Master Boot Record for getting info about partitions
struct __attribute__((packed, aligned(4))) MBR_info {
    u8                    nothing[446];        // Filler the gap in the structure
    struct partition_info partitionData[4];        // partition records (16x4)
    u16                signature;            // 0xaa55
};


/*-[sdInitCard]-------------------------------------------------------------}
. Attempts to initializes current SD Card and returns success/error status.
. This call should be done before any attempt to do anything with a SD card.
. RETURN: SD_OK indicates the current card successfully initialized.
.         !SD_OK if card initialize failed with code identifying error.
. 21Aug17 LdB
.--------------------------------------------------------------------------*/
SDRESULT sdInitCard()
{
  SDRESULT resp;

  /* Reset the card */
  if ((resp = sdResetCard())) {
    return resp;
  }

  /*
   * Send SEND_IF_COND, 0x000001aa (CMD8) voltage range 0x1 check pattern 0xaa.
   * If voltage range and check pattern don't match, look for older card.
   */
  resp = sdSendCommandA(IX_SEND_IF_COND, 0x000001aa);
  if (resp == SD_OK) {
    /*
     * Card responded with voltage and check pattern.
     *
     * Resolve voltage and check for high capacity card.
     */
    if ((resp = sdAppSendOpCond(ACMD41_ARG_HC))) {
      return sdDebugResponse(resp);
    }

    /*
     * Figure out of we have a standard or high capacity card.
     */
    if (sdCard.ocr.card_capacity) {
        sdCard.type = SD_TYPE_2_HC;
    } else {
        sdCard.type = SD_TYPE_2_SC;
    }
  } else if (resp == SD_BUSY) {
    return resp;
  } else {
    /* If no response to the SEND_IF_COND, treat this as an old card */

    /* If there appears to be a command in progress, reset the card. */
    if ((EMMC_STATUS->CMD_INHIBIT) && (resp = sdResetCard())) {
      return resp;
    }

    udelay(50);
    /* Resolve voltage... */
    if ((resp = sdAppSendOpCond(ACMD41_ARG_SC))) {
      return sdDebugResponse(resp);
    }

    sdCard.type = SD_TYPE_1;
  }

  /* Send the ALL_SEND_CID (CMD2) */
  if ((resp = sdSendCommand(IX_ALL_SEND_CID))) {
    return sdDebugResponse(resp);
  }

  /* Send SEND_REL_ADDR (CMD3) */
  /* TODO: In theory, loop back to SEND_IF_COND to find additional cards. */
  if ((resp = sdSendCommand(IX_SEND_REL_ADDR))) {
    return sdDebugResponse(resp);
  }

  // From now on the card should be in standby state.
  // Actually cards seem to respond in identify state at this point.
  // Check this with a SEND_STATUS (CMD13)
  //if( (resp = sdSendCommand(IX_SEND_STATUS)) ) return sdDebugResponse(resp);
  //printf("Card current state: %x %s\n",sdCard.status,STATUS_NAME[sdCard.cardState]);

  /* Send SEND_CSD (CMD9) and parse the result. */
  if ((resp = sdSendCommand(IX_SEND_CSD))) {
    return sdDebugResponse(resp);
  }

  /* At this point, set the clock to full speed. */
  if ((resp = sdSetClock(FREQ_NORMAL))) {
    return sdDebugResponse(resp);
  }
  pisd_fastclock = 1;

  // Send CARD_SELECT  (CMD7)
  // TODO: Check card_is_locked status in the R1 response from CMD7 [bit 25], if so, use CMD42 to unlock
  // CMD42 structure [4.3.7] same as a single block write; data block includes
  // PWD setting mode, PWD len, PWD data.
  if( (resp = sdSendCommand(IX_CARD_SELECT)) ) return sdDebugResponse(resp);

  // Get the SCR as well.
  // Need to do this before sending ACMD6 so that allowed bus widths are known.
  if( (resp = sdReadSCR()) ) return sdDebugResponse(resp);

  // Send APP_SET_BUS_WIDTH (ACMD6)
  // If supported, set 4 bit bus width and update the CONTROL0 register.
  if (sdCard.scr.BUS_WIDTH == BUS_WIDTH_4)
  {
      if( (resp = sdSendCommandA(IX_SET_BUS_WIDTH, sdCard.rca | 2)) ) 
          return sdDebugResponse(resp);
      EMMC_CONTROL0->HCTL_DWIDTH = 1;
      LOG_DEBUG("eMMC: Bus width set to 4\n");
  }

  // Send SET_BLOCKLEN (CMD16)
  if( (resp = sdSendCommandA(IX_SET_BLOCKLEN,512)) ) return sdDebugResponse(resp);

  // Print out the CID having got this far.
  unsigned int serial = sdCard.cid.SerialNumHi;
  serial <<= 16;
  serial |= sdCard.cid.SerialNumLo;
#if 0
    printf("eMMC: SD Card %s %dMb mfr %d '%c%c:%c%c%c%c%c' r%d.%d %d/%d, #%x RCA %x\n",
        SD_TYPE_NAME[sdCard.type], (int)(sdCard.CardCapacity >> 20),
        sdCard.cid.MID, sdCard.cid.OID_Hi, sdCard.cid.OID_Lo,
        sdCard.cid.ProdName1, sdCard.cid.ProdName2, sdCard.cid.ProdName3, sdCard.cid.ProdName4, sdCard.cid.ProdName5,
        sdCard.cid.ProdRevHi, sdCard.cid.ProdRevLo, sdCard.cid.ManufactureMonth, 2000+sdCard.cid.ManufactureYear, serial,
        sdCard.rca >> 16);
#endif
  return SD_OK;
}


/*-[sdCardCSD]--------------------------------------------------------------}
. Returns the pointer to the CSD structure for the current SD Card.
. RETURN: Valid CSD pointer if the current card successfully initialized
.         NULL if current card initialize not called or failed.
. 21Aug17 LdB
.--------------------------------------------------------------------------*/
struct CSD* sdCardCSD (void) {
    if (sdCard.type != SD_TYPE_UNKNOWN) return (&sdCard.csd);        // Card success so return structure pointer
        else return NULL;                                            // Return fail result of null
}


void pisdintr(void);

static void bcm283x_sdcard_irq(void *arg)
{
  pisdintr();
}


void bcm283x_sdcard_init(void)
{
  u8 buffer[512] __attribute__((aligned(4)));
  struct MBR_info* mbr;
  struct partition_info* pd;
  u32 i;
  u32 nvol;

  if (sdInitCard() != SD_OK) {
    panic("eMMC: failed to initialize SD card");
  }

  if (sdTransferBlocks(0, 1, (u8*)&buffer[0], 0) != SD_OK) {
    panic("eMMC: failed to transfer block 0");
  }

  EMMC_INTERRUPT->Raw32 = EMMC_INTERRUPT->Raw32;
  pisd_coherence();

  mbr = (struct MBR_info*)&buffer[0];

  if (mbr->signature != 0xaa55) {
    panic("eMMC: block 0 is not a master boot record");
  }

  LOG_DEBUG("eMMC: SD Card Capacity: %llu\n", sdCard.CardCapacity);

  nvol = 0;

  for (i = 0; i < 4; ++i) {
    pd = &mbr->partitionData[i];

    if (pd->type == 0x7f) {
      volumes[nvol].present = 1;
      volumes[nvol].start = pd->firstSector;
      volumes[nvol].count = pd->sectorsTotal;

      LOG_DEBUG("eMMC: UNIX volume %u, start block: %u, max size %u block%s\n",
        nvol, pd->firstSector, pd->sectorsTotal, pd->sectorsTotal ? "s" : "");

      nvol++;
    }
  }

  LOG_DEBUG("eMMC: Found %u volume(s)\n", nvol);

  if (nvol) {
    if (0 != bcm283x_register_irq_handler(GPU_IRQ_EMMC_INT, bcm283x_sdcard_irq, NULL)) {
      panic("bcm283x_sdcard_init: failed to register interrupt handler");
    }
  } else {
    panic("eMMC: no UNIX partition (MBR type 7f) found");
  }
}


void bcm283x_sdcard_transfer(u32 bno, u8* buffer, u32 w)
{
  if (sdTransferBlocks(bno, 1, buffer, !!w) != SD_OK) {
    panic("eMMC: failed to start block transfer");
  }
}


/* *****************************************************************************
 * *****************************************************************************
 * *** UNIX v7 Device Interface                                              ***
 * *****************************************************************************
 * ****************************************************************************/


/*
 * Raspberry Pi SD Card disk driver
 */

#include "../../h/param.h"
#include "../../h/systm.h"
#include "../../h/buf.h"
#include "../../h/dir.h"
#include "../../h/conf.h"
#include "../../h/user.h"

#include <stdint.h>

extern void iodone(struct buf *bp);
extern void physio(void (*strat)(struct buf *), struct buf *bp, int dev, int rw);
extern void deverror(struct buf *bp, int o1, int o2);

struct buf pisdtab;
struct buf rpisdbuf;

#define IRPT_MASK_RESERVED 0xfe808ec8
#define IRPT_EN_RESERVED 0xfe808ec8
#define INTERRUPT_RESERVED 0xfe800ec8

/*
 * Monitoring device number
 */
#define	DK_N	0

/*
 * Start I/O operations - runs at spl5 (set by the caller)
 */
void pisdstart(void)
{
  struct buf *bp;
  int readyInt;
  int transferCmd;
  u32 blockAddress;
  u32 startBlock;
  SDRESULT resp;
  u32 data;
  u32 i;

  if (!(bp = pisdtab.b_actf)) {
    LOG_DEBUG("eMMC: pisdstart called with nothing to do\n");
    return;  /* nothing to do */
  }

  LOG_DEBUG("eMMC: pisdstart called with bp->b_blkno %d\n", bp->b_blkno);

  if (sdCard.type == SD_TYPE_UNKNOWN) {
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;  /* nothing to do: no card */
  }

  /* sync settings - will set up masks for async part below */
  EMMC_IRPT_EN->Raw32   = 0xffffffff;
  EMMC_IRPT_MASK->Raw32 = 0xffffffff;
  EMMC_INTERRUPT->Raw32 = EMMC_INTERRUPT->Raw32;

  /* ensure any data operation has completed before doing the transfer */
  if (pisd_waitfor(PISD_DAT_TIMEOUT_US, DAT_INHIBIT_MASK)) {
    sdDebugResponse(SD_TIMEOUT);
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }

  startBlock = bp->b_blkno + volumes[minor(bp->b_dev)].start;

  /* work out the status, interrupt and command values for the transfer */
  readyInt = (bp->b_flags & B_READ) ? INT_READ_RDY : INT_WRITE_RDY;
  transferCmd = (bp->b_flags & B_READ) ? IX_READ_SINGLE : IX_WRITE_SINGLE;
  blockAddress = sdCard.type == SD_TYPE_2_SC ? (startBlock << 9) : startBlock;

  EMMC_BLKSIZECNT->BLKCNT = 1;
  EMMC_BLKSIZECNT->BLKSIZE = 512;

  if ((resp = sdSendCommandA(transferCmd, blockAddress))) {
    sdDebugResponse(resp);
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }

  if (bp->b_flags & B_READ) {
    EMMC_IRPT_EN->Raw32 = 0x8022;  /* read-ready, data-done, err */
  } else {
    for (i = 0; i < 512; i += 4) {
      data =
        ((bp->b_un.b_addr[i + 0] <<  0) & 0x000000ff) |
        ((bp->b_un.b_addr[i + 1] <<  8) & 0x0000ff00) |
        ((bp->b_un.b_addr[i + 2] << 16) & 0x00ff0000) |
        ((bp->b_un.b_addr[i + 3] << 24) & 0xff000000);
      *EMMC_DATA = data;
    }
    LOG_DEBUG("eMMC: wrote %u bytes in %u steps - waiting for DATA_DONE\n", i, i / 4);
    EMMC_IRPT_EN->Raw32 = 0x8002;  /* data-done, err */
  }

  pisdtab.b_active++;
  dk_busy |= (1 << DK_N);
  dk_numb[DK_N] += 1;
  dk_wds[DK_N] += (bp->b_bcount >> 6);  /* tracks clicks written/read to/from disk */
}


/*
 * Maintains the buffer ops list for the SD card, kicking off processing when
 * needed.
 */
void pisdstrategy(struct buf *bp)
{
  struct buf *dp;
  long sz;
  u32 unit;
  u32 s;

  sz = bp->b_bcount;
  sz = (sz + 511) >> 9;  /* round up to a number of blocks */

  unit = minor(bp->b_dev);

  if (unit >= MAX_VOLUMES || !volumes[unit].present) {
    printf("eMMC: bad unit %u\n", unit);
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;  /* nothing to do: bad unit (partition) */
  }

  if (bp->b_blkno + sz >= volumes[unit].count) {
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }

  LOG_DEBUG("eMMC: accessing device unit %u\n", unit);
  LOG_DEBUG("eMMC: pisdstrategy called with bp->b_blkno %d and sz %ld\n", bp->b_blkno, sz);

  bp->av_forw = NULL;
  s = spl5();

  dp = &pisdtab;

  if (dp->b_actf) {
    dp->b_actl->av_forw = bp;
  } else {
    dp->b_actf = bp;
  }

  dp->b_actl = bp;

  if (!dp->b_active) {
    pisdstart();
  }

  splx(s);
}


void pisdintr(void)
{
  struct buf *bp;
  u32 i;
  u32 data;
  u32 *idata;
#if defined(DEBUG_SD_READS)
  u32 dbuf[8];
  u32 ndbuf;

  ndbuf = 0;
#endif

  if (!pisdtab.b_active) {
    return;
  }

  do_arm1176jzfs_dmb();
  do_arm1176jzfs_dsb();
  do_arm1176jzfs_isb();

  bp = pisdtab.b_actf;

  if (bp->b_flags & B_READ) {
    if (EMMC_INTERRUPT->ERR) {
      deverror(bp, EMMC_INTERRUPT->Raw32, EMMC_STATUS->Raw32);
      bp->b_flags |= B_ERROR;
    } else {
      /* EMMC_INTERRUPT->READ_READY is expected to be 1 */
      idata = (u32 *)&bp->b_un.b_addr[0];
      for (i = 0; i < 128; ++i) {
        data = *EMMC_DATA;
        *idata++ = data;
#if defined(DEBUG_SD_READS)
        dbuf[ndbuf++] = data;
        if (ndbuf >= 8) {
          printf("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
            dbuf[0], dbuf[1], dbuf[2], dbuf[3], dbuf[4], dbuf[5], dbuf[6], dbuf[7]);
          ndbuf = 0;
        }
#endif
      }
    }

    EMMC_INTERRUPT->Raw32 = EMMC_INTERRUPT->Raw32;
    LOG_DEBUG("eMMC: read %u bytes in %u steps\n", i * 4, i);
  } else {  /* we're either read or write, nothing inbetween */
    if (EMMC_INTERRUPT->ERR) {
      deverror(bp, EMMC_INTERRUPT->Raw32, EMMC_STATUS->Raw32);
      bp->b_flags |= B_ERROR;
    }

    /* EMMC_INTERRUPT->DATA_DONE is expected to be 1 */
    EMMC_INTERRUPT->Raw32 = EMMC_INTERRUPT->Raw32;
  }

  dk_busy &= ~(1<<DK_N);
  bp = pisdtab.b_actf;
  pisdtab.b_active = 0;

#if 0
  /* XXX: check for errors */
  {
     // deverror(bp, RPADDR->rper, RPADDR->rpds);
     // clear the error, reset the device etc. etc.
     // while error count is not too high, retry (see the rp driver and rk driver)
     // finally, bp->b_flags |= B_ERROR;
  }
#endif

  pisdtab.b_errcnt = 0;
  pisdtab.b_actf = bp->av_forw;
  bp->b_resid = 0;
  LOG_DEBUG("eMMC: pisdintr done, calling iodone\n");
  iodone(bp);
  LOG_DEBUG("eMMC: iodone complete, calling pisdstart\n");
  pisdstart();

  do_arm1176jzfs_dmb();
  do_arm1176jzfs_dsb();
  do_arm1176jzfs_isb();
}


/*
 * Raw device read (character interface)
 */
void pisdread(int dev)
{
  physio(pisdstrategy, &rpisdbuf, dev, B_READ);
}


/*
 * Raw device write (character interface)
 */
void pisdwrite(int dev)
{
  physio(pisdstrategy, &rpisdbuf, dev, B_WRITE);
}
