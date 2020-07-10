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
#include "bcm283x_sdcard_regs.h"

#include "bcm283x_systimer.h"
#include "bcm283x_mbox.h"
#include "bcm283x_irq.h"
#include "bcm283x_io.h"
#include "../arm1176jzfs.h"

#include "../../h/param.h"
#include "../../h/user.h"
#include "../../h/prf.h"

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

/*--------------------------------------------------------------------------}
{  This controls if debugging code is compiled or removed at compile time   }
{--------------------------------------------------------------------------*/
#define DEBUG_INFO 0                                  // Compile debugging message code .... set to 1 and other value means no compilation

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

/* needs to stay for now because of the EMMCCommand struct */
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
            volatile enum {    CMD_NO_RESP_OLD = 0,            //            no response
                            CMD_136BIT_RESP_OLD = 1,        //            136 bits response 
                            CMD_48BIT_RESP_OLD = 2,            //            48 bits response 
                            CMD_BUSY48BIT_RESP_OLD = 3,        //            48 bits response using busy 
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

/*--------------------------------------------------------------------------}
{              INTERRUPT REGISTER TURN TO MASK BIT DEFINITIONS                }
{--------------------------------------------------------------------------*/
#define INT_ERROR_MASK   (CCRC_ERR_MASK|CEND_ERR_MASK|CBAD_ERR_MASK| \
                          DTO_ERR_MASK|DCRC_ERR_MASK|DEND_ERR_MASK| \
                          ERR_MASK|ACMD_ERR_MASK)
#define INT_ALL_MASK     (CMD_DONE_MASK|DATA_DONE_MASK|READ_RDY_MASK|WRITE_RDY_MASK|INT_ERROR_MASK)


/*--------------------------------------------------------------------------}
{                          SD CARD FREQUENCIES                                }
{--------------------------------------------------------------------------*/
#define FREQ_SETUP                400000  // 400 Khz
#define FREQ_NORMAL              25000000  // 25 Mhz
#define FREQ_HS                  50000000  // 50 Mhz

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
#define IX_GO_IDLE_STATE    0       /* used */                      /* 9p */
#define IX_ALL_SEND_CID     1       /* used */                      /* 9p */
#define IX_SEND_REL_ADDR    2       /* used */                      /* 9p */
#define IX_SET_DSR          3
#define IX_SWITCH_FUNC      4                                       /* 9p */
#define IX_CARD_SELECT      5       /* used */                      /* 9p */
#define IX_SEND_IF_COND     6       /* used */                      /* 9p */
#define IX_SEND_CSD         7       /* used */                      /* 9p */
#define IX_SEND_CID         8
#define IX_VOLTAGE_SWITCH   9
#define IX_STOP_TRANS       10      /* used */                      /* 9p */
#define IX_SEND_STATUS      11      /* was used, not really */      /* 9p */
#define IX_GO_INACTIVE      12
#define IX_SET_BLOCKLEN     13      /* used */                      /* 9p */
#define IX_READ_SINGLE      14      /* used */                      /* 9p */
#define IX_READ_MULTI       15      /* used, could delete */        /* 9p */
#define IX_SEND_TUNING      16
#define IX_SPEED_CLASS      17
#define IX_SET_BLOCKCNT     18      /* used */
#define IX_WRITE_SINGLE     19      /* used */                      /* 9p */
#define IX_WRITE_MULTI      20      /* used */                      /* 9p */
#define IX_PROGRAM_CSD      21
#define IX_SET_WRITE_PR     22
#define IX_CLR_WRITE_PR     23
#define IX_SND_WRITE_PR     24
#define IX_ERASE_WR_ST      25      /* was used, can delete */
#define IX_ERASE_WR_END     26      /* was used, can delete */
#define IX_ERASE            27      /* was used, can delete */
#define IX_LOCK_UNLOCK      28
#define IX_APP_CMD          29      /* used */                      /* 9p */
#define IX_APP_CMD_RCA      30      /* used */
#define IX_GEN_CMD          31

// Commands hereafter require APP_CMD.
#define IX_APP_CMD_START    32      /* used */
#define IX_SET_BUS_WIDTH    32      /* used */                      /* 9p */
#define IX_SD_STATUS        33
#define IX_SEND_NUM_WRBL    34
#define IX_SEND_NUM_ERS     35
#define IX_APP_SEND_OP_COND 36      /* used */                      /* 9p */
#define IX_SET_CLR_DET      37
#define IX_SEND_SCR         38      /* used */

/*--------------------------------------------------------------------------}
{                              SD CARD COMMAND TABLE                            }
{--------------------------------------------------------------------------*/
static EMMCCommand sdCommandTable[IX_SEND_SCR + 1] =  {
    [IX_GO_IDLE_STATE] =    {"GO_IDLE_STATE", .code.CMD_INDEX = 0x00, .code.CMD_RSPNS_TYPE = CMD_NO_RESP_OLD       , .use_rca = 0 , .delay = 0},
    [IX_ALL_SEND_CID] =     {"ALL_SEND_CID" , .code.CMD_INDEX = 0x02, .code.CMD_RSPNS_TYPE = CMD_136BIT_RESP_OLD   , .use_rca = 0 , .delay = 0},
    [IX_SEND_REL_ADDR] =    {"SEND_REL_ADDR", .code.CMD_INDEX = 0x03, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_SET_DSR] =          {"SET_DSR"      , .code.CMD_INDEX = 0x04, .code.CMD_RSPNS_TYPE = CMD_NO_RESP_OLD       , .use_rca = 0 , .delay = 0},
    [IX_SWITCH_FUNC] =      {"SWITCH_FUNC"  , .code.CMD_INDEX = 0x06, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_CARD_SELECT] =      {"CARD_SELECT"  , .code.CMD_INDEX = 0x07, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP_OLD, .use_rca = 1 , .delay = 0},
    [IX_SEND_IF_COND] =     {"SEND_IF_COND" , .code.CMD_INDEX = 0x08, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 100},
    [IX_SEND_CSD] =         {"SEND_CSD"     , .code.CMD_INDEX = 0x09, .code.CMD_RSPNS_TYPE = CMD_136BIT_RESP_OLD   , .use_rca = 1 , .delay = 0},
    [IX_SEND_CID] =         {"SEND_CID"     , .code.CMD_INDEX = 0x0A, .code.CMD_RSPNS_TYPE = CMD_136BIT_RESP_OLD   , .use_rca = 1 , .delay = 0},
    [IX_VOLTAGE_SWITCH] =   {"VOLT_SWITCH"  , .code.CMD_INDEX = 0x0B, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_STOP_TRANS] =       {"STOP_TRANS"   , .code.CMD_INDEX = 0x0C, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP_OLD, .use_rca = 0 , .delay = 0},
    [IX_SEND_STATUS] =      {"SEND_STATUS"  , .code.CMD_INDEX = 0x0D, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 1 , .delay = 0},
    [IX_GO_INACTIVE] =      {"GO_INACTIVE"  , .code.CMD_INDEX = 0x0F, .code.CMD_RSPNS_TYPE = CMD_NO_RESP_OLD       , .use_rca = 1 , .delay = 0},
    [IX_SET_BLOCKLEN] =     {"SET_BLOCKLEN" , .code.CMD_INDEX = 0x10, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_READ_SINGLE] =      {"READ_SINGLE"  , .code.CMD_INDEX = 0x11, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , 
                                              .code.CMD_ISDATA = 1  , .code.TM_DAT_DIR = 1,                      .use_rca = 0 , .delay = 0},
    [IX_READ_MULTI] =       {"READ_MULTI"   , .code.CMD_INDEX = 0x12, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , 
                                              .code.CMD_ISDATA = 1 ,  .code.TM_DAT_DIR = 1,
                                              .code.TM_BLKCNT_EN =1 , .code.TM_MULTI_BLOCK = 1,                  .use_rca = 0 , .delay = 0},
    [IX_SEND_TUNING] =      {"SEND_TUNING"  , .code.CMD_INDEX = 0x13, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_SPEED_CLASS] =      {"SPEED_CLASS"  , .code.CMD_INDEX = 0x14, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP_OLD, .use_rca = 0 , .delay = 0},
    [IX_SET_BLOCKCNT] =     {"SET_BLOCKCNT" , .code.CMD_INDEX = 0x17, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_WRITE_SINGLE] =     {"WRITE_SINGLE" , .code.CMD_INDEX = 0x18, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , 
                                              .code.CMD_ISDATA = 1  ,                                               .use_rca = 0 , .delay = 0},
    [IX_WRITE_MULTI] =      {"WRITE_MULTI"  , .code.CMD_INDEX = 0x19, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , 
                                              .code.CMD_ISDATA = 1  ,  
                                              .code.TM_BLKCNT_EN = 1, .code.TM_MULTI_BLOCK = 1                 , .use_rca = 0 , .delay = 0},
    [IX_PROGRAM_CSD] =      {"PROGRAM_CSD"  , .code.CMD_INDEX = 0x1B, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_SET_WRITE_PR] =     {"SET_WRITE_PR" , .code.CMD_INDEX = 0x1C, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP_OLD, .use_rca = 0 , .delay = 0},
    [IX_CLR_WRITE_PR] =     {"CLR_WRITE_PR" , .code.CMD_INDEX = 0x1D, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP_OLD, .use_rca = 0 , .delay = 0},
    [IX_SND_WRITE_PR] =     {"SND_WRITE_PR" , .code.CMD_INDEX = 0x1E, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_ERASE_WR_ST] =      {"ERASE_WR_ST"  , .code.CMD_INDEX = 0x20, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_ERASE_WR_END] =     {"ERASE_WR_END" , .code.CMD_INDEX = 0x21, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_ERASE] =            {"ERASE"        , .code.CMD_INDEX = 0x26, .code.CMD_RSPNS_TYPE = CMD_BUSY48BIT_RESP_OLD, .use_rca = 0 , .delay = 0},
    [IX_LOCK_UNLOCK] =      {"LOCK_UNLOCK"  , .code.CMD_INDEX = 0x2A, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_APP_CMD] =          {"APP_CMD"      , .code.CMD_INDEX = 0x37, .code.CMD_RSPNS_TYPE = CMD_NO_RESP_OLD       , .use_rca = 0 , .delay = 100},
    [IX_APP_CMD_RCA] =      {"APP_CMD"      , .code.CMD_INDEX = 0x37, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 1 , .delay = 0},
    [IX_GEN_CMD] =          {"GEN_CMD"      , .code.CMD_INDEX = 0x38, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},

    // APP commands must be prefixed by an APP_CMD.
    [IX_SET_BUS_WIDTH] =    {"SET_BUS_WIDTH", .code.CMD_INDEX = 0x06, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_SD_STATUS] =        {"SD_STATUS"    , .code.CMD_INDEX = 0x0D, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 1 , .delay = 0},
    [IX_SEND_NUM_WRBL] =    {"SEND_NUM_WRBL", .code.CMD_INDEX = 0x16, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_SEND_NUM_ERS] =     {"SEND_NUM_ERS" , .code.CMD_INDEX = 0x17, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_APP_SEND_OP_COND] = {"SD_SENDOPCOND", .code.CMD_INDEX = 0x29, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 1000},
    [IX_SET_CLR_DET] =      {"SET_CLR_DET"  , .code.CMD_INDEX = 0x2A, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , .use_rca = 0 , .delay = 0},
    [IX_SEND_SCR] =         {"SEND_SCR"     , .code.CMD_INDEX = 0x33, .code.CMD_RSPNS_TYPE = CMD_48BIT_RESP_OLD    , 
                                              .code.CMD_ISDATA = 1  , .code.TM_DAT_DIR = 1                     , .use_rca = 0 , .delay = 0},
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


#if 0
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
#endif

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
      intr & (CTO_ERR_MASK|DTO_ERR_MASK) ||
      intr & INT_ERROR_MASK) {
    pisd_writereg(EMMC_INTERRUPT_REG, intr);
    return (intr & (CTO_ERR_MASK|DTO_ERR_MASK) || elapsed_us >= max_us)
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
    *p = pisd_readreg(EMMC_RESP0_REG);
    p = (u32*)&buf[8];
    *p = pisd_readreg(EMMC_RESP1_REG);
    p = (u32*)&buf[4];
    *p = pisd_readreg(EMMC_RESP2_REG);
    p = (u32*)&buf[0];
    *p = pisd_readreg(EMMC_RESP3_REG);

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


#define R1_ERRORS_MASK       0xfff9c004
static int do_send_cmd_p(EMMCCommand* cmd, u32 arg)
{
  int ret;
  u32 resp0;

  if (0 != (ret = pisd_waitfor(PISD_CMD_TIMEOUT_US, CMD_INHIBIT_MASK)))
    return ret;

  sdCard.lastCmd = cmd;

  pisd_writereg(EMMC_INTERRUPT_REG, pisd_readreg(EMMC_INTERRUPT_REG));
  pisd_writereg(EMMC_ARG1_REG, arg);
  pisd_writereg(EMMC_CMDTM_REG, cmd->code.Raw32);
  if (cmd->delay)
    udelay(cmd->delay);

  if (0 != (ret = pisd_wfi(PISD_WFI_TIMEOUT_US, CMD_DONE_MASK)))
    return ret;

  resp0 = pisd_readreg(EMMC_RESP0_REG);

  switch (cmd->code.CMD_RSPNS_TYPE) {
    case CMD_NO_RESP_OLD:
      return 0;
    case CMD_BUSY48BIT_RESP_OLD:
      sdCard.status = resp0;
      return resp0 & R1_ERRORS_MASK;
    case CMD_136BIT_RESP_OLD:
      sdCard.status = 0;
      if (cmd->code.CMD_INDEX == 0x09) {
        unpack_csd(&sdCard.csd);
      } else {
        u32* data = (u32*)(uptr_t)&sdCard.cid;
        data[3] = resp0;
        data[2] = pisd_readreg(EMMC_RESP1_REG);
        data[1] = pisd_readreg(EMMC_RESP2_REG);
        data[0] = pisd_readreg(EMMC_RESP3_REG);
      }
      return 0;
    case CMD_48BIT_RESP_OLD:
      switch (cmd->code.CMD_INDEX) {
        case 0x03:
          sdCard.rca = resp0 & 0xffff0000;
          sdCard.status = ((resp0 & 0x00001fff)) | ((resp0 & 0x00002000) << 6) | ((resp0 & 0x00004000) << 8) | ((resp0 & 0x00008000) << 8);
          return sdCard.status & R1_ERRORS_MASK;
        case 0x08:
          sdCard.status = 0;
          return resp0 == arg ? SD_OK : SD_ERROR;
        case 0x29:
          sdCard.status = 0;
          sdCard.ocr.Raw32 = resp0;
          return 0;
        default:
          sdCard.status = resp0;
          return resp0 & R1_ERRORS_MASK;
      }
    default:
      break;
  }

  return -ENXIO;
}


#define ST_APP_CMD           0x00000020
static int do_send_app_cmd(void)
{
  int ret;

  if (!sdCard.rca)
    return do_send_cmd_p(&sdCommandTable[IX_APP_CMD], 0x00000000);

  if (0 != (ret = do_send_cmd_p(&sdCommandTable[IX_APP_CMD_RCA], sdCard.rca)))
    return ret;

  if (!(sdCard.status & ST_APP_CMD))
    ret = -ENXIO;

  return ret;
}


static int do_send_cmd(int index)
{
  int ret;
  u32 arg;

  if (index >= IX_APP_CMD_START && (ret = do_send_app_cmd()))
    return ret;

  EMMCCommand* cmd = &sdCommandTable[index];
  arg = cmd->use_rca == 1 ? sdCard.rca : 0;

  if (0 != (ret = do_send_cmd_p(cmd, arg)))
    return ret;

  if (index >= IX_APP_CMD_START && sdCard.rca && !(sdCard.status & ST_APP_CMD))
    ret = -ENXIO;

  return ret;
}


static int do_send_cmd_a(int index, u32 arg)
{
  int ret;

  if (index >= IX_APP_CMD_START && (ret = do_send_app_cmd()))
    return ret;

  if (0 != (ret = do_send_cmd_p(&sdCommandTable[index], arg)))
    return ret;

  if (index >= IX_APP_CMD_START && sdCard.rca && !(sdCard.status & ST_APP_CMD))
    ret = -ENXIO;

  return ret;
}


static int do_read_scr(void)
{
  int ret;
  int nr;
  int c;

  nr = 0;
  c = 100000;

  if (0 != (ret = pisd_waitfor(PISD_DAT_TIMEOUT_US, DAT_INHIBIT_MASK)))
    return ret;

  pisd_writereg(EMMC_BLKSIZECNT_REG, (1 << BLKCNT_SHIFT) | (8 << BLKSIZE_SHIFT));
  if (0 != (ret = do_send_cmd(IX_SEND_SCR)))
    return ret;

  if (0 != (ret = pisd_wfi(PISD_WFI_TIMEOUT_US, READ_RDY_MASK)))
    return ret;

  while (nr < 2) {
    if (pisd_readreg(EMMC_STATUS_REG) & READ_TRANSFER_MASK) {
      if (nr == 0)
        sdCard.scr.Raw32_Lo = pisd_readreg(EMMC_DATA_REG);
      else
        sdCard.scr.Raw32_Hi = pisd_readreg(EMMC_DATA_REG);
      nr++;
    } else {
      udelay(1);
      if (--c == 0)
        break;
    }
  }

  if (nr != 2) {
    return -EBUSY;
  }

  return 0;
}


static u32 do_fls(u32 x)
{
  u32 b;
  u32 msk;

  if (!x)
    return 0;

  msk = 0x80000000;

  for (b = 32; b; --b) {
    if (x & msk)
      break;
    msk >>= 1;
  }

  return b;
}


static int do_sd_get_clkdiv(u32 freq)
{
  u32 base_hz;
  u32 sdver;
  u32 divisor;

  base_hz = 0;
  (void)bcm283x_mbox_get_sdcard_clock(&base_hz);
  if (!base_hz) {
    base_hz = 41666667;
  }
  base_hz = 41666667;  /* clock logic in thise code is pure crap */
  LOG_DEBUG("eMMC: Base clock: %uHz\n", base_hz);

  divisor = (base_hz + freq - 1) / freq;
  if (divisor > 0x3ff)
    divisor = 0x3ff;

  sdver = (pisd_readreg(EMMC_SLOTISR_VER_REG) & SDVERSION_MASK) >> SDVERSION_SHIFT;
  LOG_DEBUG("eMMC: SD version: %u\n", sdver);
  /* is this even necessary? this is not the standard interface */
  if (sdver < 2) {  /* applies to HOST SPECIFICATION < 3 */
    u32 sc;  /* shift count */
    sc = do_fls(divisor);
    if (sc)
      sc--;
    if (sc > 7)
      sc = 7;
    divisor = (1U << sc);
  } else if (divisor < 3)
    divisor = 4; /* que? */

  LOG_DEBUG("eMMC: Divisor = %d, Freq Set = %dHz\n", (int)divisor, (int)(base_hz/divisor));
  return divisor;
}


static int do_sd_set_clk(u32 freq)
{
  int ret;
  u32 ctrl1;
  u32 cdiv;
  u32 divlo;
  u32 divhi;
  u64 max_us;
  u64 elapsed_us;
  u64 start;

  if (0 != (ret = pisd_waitfor(PISD_CMD_TIMEOUT_US, CMD_INHIBIT_MASK|DAT_INHIBIT_MASK)))
    return ret;

  ctrl1 = pisd_readreg(EMMC_CONTROL1_REG) & ~CLK_EN_MASK;
  pisd_writereg(EMMC_CONTROL1_REG, ctrl1);  /* disable the clock */
  cdiv = do_sd_get_clkdiv(freq);            /* get the new divisor value */
  ctrl1 &= 0xffff001f;                      /* clear out divisor bits */
  ctrl1 |= ((cdiv & 0xff) << 8);            /* set divisor low bits */
  ctrl1 |= ((cdiv & 0x300) >> 2);           /* set divisor high bits */
  pisd_writereg(EMMC_CONTROL1_REG, ctrl1);  /* set the new divisor */
  ctrl1 |= CLK_EN_MASK;
  pisd_writereg(EMMC_CONTROL1_REG, ctrl1);  /* enable the clock */

  max_us = 100000;  /* 1/10th of a second */
  elapsed_us = 0;
  start = ticks(NULL);

  while (elapsed_us < max_us) {
    if (pisd_readreg(EMMC_CONTROL1_REG) & CLK_STABLE_MASK) {
      return 0;
    }

    elapsed_us = tick_diff_us(start, ticks(NULL));
  }

  return -EBUSY;
}


static int do_sd_reset_card(void)
{
  int ret;
  u64 max_us;
  u64 elapsed_us;
  u64 start;

  pisd_writereg(EMMC_CONTROL0_REG, 0);                   /* clear control reg 0 */
  pisd_writereg(EMMC_CONTROL1_REG, 0);                   /* clear control reg 1 */
  pisd_writereg(EMMC_CONTROL1_REG, SRST_HC_MASK);        /* reset the host circuit */
  /* EMMC_CONTROL2->UHSMODE = SDR12; // what is this all about? */

  max_us = 100000;  /* 1/10th of a second */
  elapsed_us = 0;
  start = ticks(NULL);

  while (elapsed_us < max_us) {
    if (pisd_readreg(EMMC_CONTROL1_REG) & SRST_HC_MASK) {
      elapsed_us = tick_diff_us(start, ticks(NULL));
      continue;
    }

    break;
  }

  if (elapsed_us >= max_us) {
    return -EBUSY;
  }

  pisd_writereg(EMMC_CONTROL1_REG,
    (pisd_readreg(EMMC_CONTROL1_REG) & ~DATA_TOUNIT_MASK) |
      (0xeU << DATA_TOUNIT_SHIFT));

  pisd_writereg(EMMC_CONTROL1_REG,
    (pisd_readreg(EMMC_CONTROL1_REG) & ~CLK_INTLEN_MASK) |
      (0x1U << CLK_INTLEN_SHIFT));

  if (0 != (ret = do_sd_set_clk(FREQ_SETUP))) {
    return ret;
  }

  pisd_fastclock = 0;

  /* enable interrupts for command completion values */
  pisd_writereg(EMMC_IRPT_EN_REG, 0xffffffff);
  pisd_writereg(EMMC_IRPT_MASK_REG, 0xffffffff);

  sdCard.rca = 0;
  sdCard.ocr.Raw32 = 0;
  sdCard.lastCmd = 0;
  sdCard.status = 0;
  sdCard.type = SD_TYPE_UNKNOWN;

  return do_send_cmd(IX_GO_IDLE_STATE);
}


static int do_sd_app_send_ap_cond(u32 arg)
{
  int ret;
  u32 c;

  if (0 != (ret = do_send_cmd_a(IX_APP_SEND_OP_COND, arg)) && ret != -EBUSY)
    return ret;

  c = 6;
  while (sdCard.ocr.card_power_up_busy == 0 && c--) {
    udelay(400000);
    if (0 != (ret = do_send_cmd_a(IX_APP_SEND_OP_COND, arg)) && ret != -EBUSY)
      return ret;
  }

  if (sdCard.ocr.card_power_up_busy == 0)
    return -EBUSY;

  /*
   * Pi is 3.3v SD only, so check that a voltage value around 3.3v was returned.
   */
  if (sdCard.ocr.voltage3v2to3v3 == 0 && sdCard.ocr.voltage3v3to3v4 == 0)
    return -ENXIO;

  return 0;
}


static int do_sd_transfer_blocks_sync(u32 sb, u32 nb, u8 *buf, int w)
{
  int ret;
  u32 intr_pin;
  u32 xfer_cmd;
  u32 ba;
  u32 bd;
  u32 i;
  u32 datum;
  u32 *data;

  if (sdCard.type == SD_TYPE_UNKNOWN)
    return -ENODEV;

  if (0 != (ret = pisd_waitfor(PISD_DAT_TIMEOUT_US, DAT_INHIBIT_MASK)))
    return ret;

  intr_pin = w ? WRITE_RDY_MASK : READ_RDY_MASK;
  xfer_cmd = w ? (nb == 1 ? IX_WRITE_SINGLE : IX_WRITE_MULTI) :
    (nb == 1 ? IX_READ_SINGLE : IX_READ_MULTI);

  if (nb > 1 && sdCard.scr.CMD_SUPPORT == CMD_SUPP_SET_BLKCNT &&
      0 != (ret = do_send_cmd_a(IX_SET_BLOCKCNT, nb)))
    return ret;

  /*
   * The address differs depending on the card type.
   *   For HC the address is a block number, so just pass it through.
   *   For SC the address is a byte offset, so multiply by 512.
   */
  ba = sdCard.type == SD_TYPE_2_SC ? (sb << 9) : sb;

  /* set up transfer parameters */
  pisd_writereg(EMMC_BLKSIZECNT_REG,
    (512 << BLKSIZE_SHIFT) | (nb << BLKCNT_SHIFT));

  if (0 != (ret = do_send_cmd_a(xfer_cmd, ba)))
    return ret;

  bd = 0;
  while (bd < nb) {
    if (0 != (ret = pisd_wfi(PISD_WFI_TIMEOUT_US, intr_pin))) {
      return ret;
    }

    /*
     * Handle non-word-aligned buffers byte-by-byte.
     *
     * The entire block is sent without looking at status registers, which is
     * something that could be improved.
     */
    if ((uptr_t)buf & 0x3) {
      for (i = 0; i < 512; ++i) {
        if (w) {
          datum =     (buf[i]        );
          datum |=    (buf[i+1] << 8 );
          datum |=    (buf[i+2] << 16);
          datum |=    (buf[i+3] << 24);
          pisd_writereg(EMMC_DATA_REG, datum);
        } else {
          datum = pisd_readreg(EMMC_DATA_REG);
          buf[i] =   (datum      ) & 0xff;
          buf[i+1] = (datum >> 8 ) & 0xff;
          buf[i+2] = (datum >> 16) & 0xff;
          buf[i+3] = (datum >> 24) & 0xff;
        }
      }
    } else {
      /*
       * Word-aligned buffers can be hanlded more efficiently.
       */
      data = (u32 *)buf;
      for (i = 0; i < 128; ++i) {
        if (w)
          pisd_writereg(EMMC_DATA_REG, data[i]);
        else
          data[i] = pisd_readreg(EMMC_DATA_REG);
      }
    }

    bd++;
    buf += 512;
  }

  /*
   * If not all bytes were read, the operation timed out
   *
   * NOTE: this is LdB logic, but I don't see how this is possible?!
   */
  if (bd != nb) {
    if (!w && nb > 1 && 0 != (ret = do_send_cmd(IX_STOP_TRANS)))
      return ret;
    return -EBUSY;
  }

  if (w && 0 != (ret = pisd_wfi(PISD_WFI_TIMEOUT_US, DATA_DONE_MASK))) {
    return ret;
  }

  if (nb > 1 && sdCard.scr.CMD_SUPPORT != CMD_SUPP_SET_BLKCNT &&
      0 != (ret = do_send_cmd(IX_STOP_TRANS)))
    return ret;

  return 0;
}

/*--------------------------------------------------------------------------}
{                    PARTITION DECRIPTION BLOCK STRUCTURE                    }
{--------------------------------------------------------------------------*/
//Structure to access info of a partition on the disk
struct __attribute__((packed, aligned(2))) partition_info {
    u8   status;        // 0x80 - active partition
    u8   headStart;     // starting head
    u16  cylSectStart;  // starting cylinder and sector
    u8   type;          // partition type (01h = 12bit FAT, 04h = 16bit FAT, 05h = Ex MSDOS, 06h = 16bit FAT (>32Mb), 0Bh = 32bit FAT (<2048GB))
    u8   headEnd;       // ending head of the partition
    u16  cylSectEnd;    // ending cylinder and sector
    u32  firstSector;   // total sectors between MBR & the first sector of the partition
    u32  sectorsTotal;  // size of this partition in sectors
};

/*--------------------------------------------------------------------------}
{                     MASTER BOOT RECORD BLOCK STRUCTURE                        }
{--------------------------------------------------------------------------*/
// Structure to access Master Boot Record for getting info about partitions
struct __attribute__((packed, aligned(4))) MBR_info {
    u8                    nothing[446];      // Filler the gap in the structure
    struct partition_info partitionData[4];  // partition records (16x4)
    u16                   signature;         // 0xaa55
};


int do_sd_init_card(void)
{
  int ret;

  if (0 != (ret = do_sd_reset_card()))
    return ret;

  /*
   * Send SEND_IF_COND, 0x000001aa (CMD8) voltage range 0x1 check pattern 0xaa.
   * If voltage range and check pattern don't match, look for older card.
   */
  ret = do_send_cmd_a(IX_SEND_IF_COND, 0x000001aa);
  if (ret == -EBUSY)
    return ret;

  if (ret == 0) {
    /*
     * Card responded with voltage and check pattern.  Resolve voltage and
     * check for high capacity card.
     */
    if (0 != (ret = do_sd_app_send_ap_cond(ACMD41_ARG_HC)))
      return ret;
    /* check for card capacity */
    if (sdCard.ocr.card_capacity)
      sdCard.type = SD_TYPE_2_HC;
    else
      sdCard.type = SD_TYPE_2_SC;
  } else {
    /*
     * If there is no response to the SEND_IF_COND command we treat this as an
     * older card.
     */
    /* if there's a command in progress we reset the card */
    if ((pisd_readreg(EMMC_STATUS_REG) & CMD_INHIBIT_MASK) &&
        0 != (ret = do_sd_reset_card()))
      return ret;
    udelay(50);  /* this is unnecessary when no command was in progress */
    /* resolve voltage */
    if (0 != (ret = do_sd_app_send_ap_cond(ACMD41_ARG_SC)))
      return ret;
    sdCard.type = SD_TYPE_1;
  }

  /* send ALL_SEND_CID */
  if (0 != (ret = do_send_cmd(IX_ALL_SEND_CID)))
    return ret;

  /* Send SEND_REL_ADDR (CMD3) */
  /* TODO: In theory, loop back to SEND_IF_COND to find additional cards. */
  if (0 != (ret = do_send_cmd(IX_SEND_REL_ADDR)))
    return ret;

#if 0
  /*
   * From now on the card should be in standby state.
   * Actually cards seem to respond in identify state at this point.
   * Check this with a SEND_STATUS (CMD13)
   */
  if (0 != (ret = do_send_cmd(IX_SEND_STATUS)))
    return ret;
  printf("eMMC: Card current state: %x %s\n",
    sdCard.status, STATUS_NAME[sdCard.cardState]);
#endif

  /* send SEND_CSD (CMD9) and parse the result */
  if (0 != (ret = do_send_cmd(IX_SEND_CSD)))
    return ret;

  /* At this point, set the clock to full speed. */
  if (0 != (ret = do_sd_set_clk(FREQ_NORMAL)))  /* full speed - 25Mhz */
    return SD_ERROR;
  pisd_fastclock = 1;

  // Send CARD_SELECT (CMD7)
  // TODO: Check card_is_locked status in the R1 response from CMD7 [bit 25], if so, use CMD42 to unlock
  // CMD42 structure [4.3.7] same as a single block write; data block includes
  // PWD setting mode, PWD len, PWD data.
  if (0 != (ret = do_send_cmd(IX_CARD_SELECT)))
    return ret;

  // Get the SCR as well.
  // Need to do this before sending ACMD6 so that allowed bus widths are known.
  if (0 != (ret = do_read_scr()))
    return ret;

  // Send APP_SET_BUS_WIDTH (ACMD6)
  // If supported, set 4 bit bus width and update the CONTROL0 register.
  if (sdCard.scr.BUS_WIDTH == BUS_WIDTH_4) {
    if (0 != (ret = do_send_cmd_a(IX_SET_BUS_WIDTH, sdCard.rca | 2)))
      return ret;
    pisd_writereg(EMMC_CONTROL0_REG,
      pisd_readreg(EMMC_CONTROL0_REG) | HCTL_DWIDTH_MASK);
    LOG_DEBUG("eMMC: Bus width set to 4\n");
  }

  /* send SET_BLOCKLEN (CMD16) */
  if (0 != (ret = do_send_cmd_a(IX_SET_BLOCKLEN, 512)))
    return ret;

#if 0
  /* print out the CID having got this far */
  u32 serial = sdCard.cid.SerialNumHi << 16 | sdCard.cid.SerialNumLo;
  printf("eMMC: SD Card %s %dMb mfr %d '%c%c:%c%c%c%c%c' r%d.%d %d/%d, #%x RCA %x\n",
    SD_TYPE_NAME[sdCard.type], (int)(sdCard.CardCapacity >> 20),
    sdCard.cid.MID, sdCard.cid.OID_Hi, sdCard.cid.OID_Lo,
    sdCard.cid.ProdName1, sdCard.cid.ProdName2, sdCard.cid.ProdName3, sdCard.cid.ProdName4, sdCard.cid.ProdName5,
    sdCard.cid.ProdRevHi, sdCard.cid.ProdRevLo, sdCard.cid.ManufactureMonth, 2000+sdCard.cid.ManufactureYear, serial,
    sdCard.rca >> 16);
#endif
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
  int ret;

  if (0 != (ret = do_sd_init_card())) {
    panic("eMMC: failed to initialize SD card");
  }

  if (0 != (ret = do_sd_transfer_blocks_sync(0, 1, (u8*)&buffer[0], 0))) {
    panic("eMMC: failed to transfer block 0");
  }

  pisd_writereg(EMMC_INTERRUPT_REG, pisd_readreg(EMMC_INTERRUPT_REG));

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
  u32 data;
  u32 i;
  int ret;

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
  /* pisd_writereg(EMMC_IRPT_EN_REG, 0xffffffff); */
  pisd_writereg(EMMC_IRPT_MASK_REG, 0xffffffff);
  pisd_writereg(EMMC_INTERRUPT_REG, pisd_readreg(EMMC_INTERRUPT_REG));

  /* ensure any data operation has completed before doing the transfer */
  if (0 != (ret = pisd_waitfor(PISD_DAT_TIMEOUT_US, DAT_INHIBIT_MASK))) {
    /* sdDebugResponse(SD_TIMEOUT); */
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }

  startBlock = bp->b_blkno + volumes[minor(bp->b_dev)].start;

  /* work out the status, interrupt and command values for the transfer */
  readyInt = (bp->b_flags & B_READ) ? READ_RDY_MASK : WRITE_RDY_MASK;
  transferCmd = (bp->b_flags & B_READ) ? IX_READ_SINGLE : IX_WRITE_SINGLE;
  blockAddress = sdCard.type == SD_TYPE_2_SC ? (startBlock << 9) : startBlock;

  pisd_writereg(EMMC_BLKSIZECNT_REG,
    (512 << BLKSIZE_SHIFT) | (1U << BLKCNT_SHIFT));

  if (do_send_cmd_a(transferCmd, blockAddress)) {
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }

  if (bp->b_flags & B_READ) {
    pisd_writereg(EMMC_IRPT_EN_REG, 0x00008022);  /* read-ready, data-done, err */
  } else {
    for (i = 0; i < 512; i += 4) {
      data =
        ((bp->b_un.b_addr[i + 0] <<  0) & 0x000000ff) |
        ((bp->b_un.b_addr[i + 1] <<  8) & 0x0000ff00) |
        ((bp->b_un.b_addr[i + 2] << 16) & 0x00ff0000) |
        ((bp->b_un.b_addr[i + 3] << 24) & 0xff000000);
      pisd_writereg(EMMC_DATA_REG, data);
    }
    LOG_DEBUG("eMMC: wrote %u bytes in %u steps - waiting for DATA_DONE\n", i, i / 4);
    pisd_writereg(EMMC_IRPT_EN_REG, 0x00008002);  /* data-done, err */
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

  if (!pisdtab.b_active) {
    return;
  }

  do_arm1176jzfs_dmb();
  do_arm1176jzfs_dsb();
  do_arm1176jzfs_isb();

  bp = pisdtab.b_actf;

  if (bp->b_flags & B_READ) {
    if (pisd_readreg(EMMC_INTERRUPT_REG) & ERR_MASK) {
      deverror(bp, pisd_readreg(EMMC_INTERRUPT_REG), pisd_readreg(EMMC_STATUS_REG));
      bp->b_flags |= B_ERROR;
    } else {
      /* pisd_readreg(EMMC_INTERRUPT_REG) & READ_READY_MASK is expected to be non-zero */
      idata = (u32 *)&bp->b_un.b_addr[0];
      for (i = 0; i < 128; ++i) {
        data = pisd_readreg(EMMC_DATA_REG);
        *idata++ = data;
      }
    }

    pisd_writereg(EMMC_INTERRUPT_REG, pisd_readreg(EMMC_INTERRUPT_REG));
    LOG_DEBUG("eMMC: read %u bytes in %u steps\n", i * 4, i);
  } else {  /* we're either read or write, nothing inbetween */
    if (pisd_readreg(EMMC_INTERRUPT_REG) & ERR_MASK) {
      deverror(bp, pisd_readreg(EMMC_INTERRUPT_REG), pisd_readreg(EMMC_STATUS_REG));
      bp->b_flags |= B_ERROR;
    }

    /* pisd_readreg(EMMC_INTERRUPT_REG) & DATA_DONE_MASK is expected to be non-zero */
    pisd_writereg(EMMC_INTERRUPT_REG, pisd_readreg(EMMC_INTERRUPT_REG));
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
