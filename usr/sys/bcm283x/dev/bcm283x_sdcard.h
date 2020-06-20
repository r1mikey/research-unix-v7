#ifndef BCM283X_SDCARD_H
#define BCM283X_SDCARD_H

#include <stdint.h>
#include <stdbool.h>

#include "../../h/types.h"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++}
{																			}
{       Filename: SDCard.h													}
{       Copyright(c): Leon de Boer(LdB) 2017								}
{       Version: 1.01														}
{       Original start code from Holdswrthy use from the Pi Forum           }
{																			}
{***************[ THIS CODE IS FREEWARE UNDER CC Attribution]***************}
{																            }
{     This sourcecode is released for the purpose to promote programming    }
{  on the Raspberry Pi. You may redistribute it and/or modify with the      }
{  following disclaimer and condition.                                      }
{																            }
{      The SOURCE CODE is distributed "AS IS" WITHOUT WARRANTIES AS TO      }
{   PERFORMANCE OF MERCHANTABILITY WHETHER EXPRESSED OR IMPLIED.            }
{   Redistributions of source code must retain the copyright notices to     }
{   maintain the author credit (attribution) .								}
{																			}
{***************************************************************************/

/***************************************************************************}
{                    PUBLIC ENUMERATIONS FOR THIS UNIT					    }
****************************************************************************/

/*--------------------------------------------------------------------------}
{					     PUBLIC SD RESULT CODES							    }
{--------------------------------------------------------------------------*/
typedef enum {
	SD_OK				= 0,							// NO error
	SD_ERROR			= 1,							// General non specific SD error
	SD_TIMEOUT			= 2,							// SD Timeout error
	SD_BUSY				= 3,							// SD Card is busy
	SD_NO_RESP			= 5,							// SD Card did not respond
	SD_ERROR_RESET		= 6,							// SD Card did not reset
	SD_ERROR_CLOCK		= 7,							// SD Card clock change failed
	SD_ERROR_VOLTAGE	= 8,							// SD Card does not support requested voltage
	SD_ERROR_APP_CMD	= 9,							// SD Card app command failed						
	SD_CARD_ABSENT		= 10,							// SD Card not present
	SD_READ_ERROR		= 11,
	SD_MOUNT_FAIL		= 12,
} SDRESULT;

// SD card types

/*--------------------------------------------------------------------------}
{				    PUBLIC ENUMERATION OF SD CARD TYPE					    }
{--------------------------------------------------------------------------*/
typedef enum {
	SD_TYPE_UNKNOWN = 0,
	SD_TYPE_MMC = 1,
	SD_TYPE_1 = 2,
	SD_TYPE_2_SC = 3,
	SD_TYPE_2_HC = 4,
} SDCARD_TYPE;


/***************************************************************************}
{                    PUBLIC STRUCTURES FOR THIS UNIT					    }
****************************************************************************/

/*--------------------------------------------------------------------------}
{					   PUBLIC CSD DECODED STRUCTURE						    }
{---------------------------------------------------------------------------}
{  You usually do not create this structure you simply ask for the pointer  }
{  to the one automatically created when an SD Card is initialized. It has  }
{  many of the entries of the standard SD Card CSD. However as it allows    }
{  for many SD Card variants the field positions can be very different.     }
{--------------------------------------------------------------------------*/
struct __attribute__((__packed__, aligned(4))) CSD {
	union {
		struct __attribute__((__packed__, aligned(1))) {
			enum { 
				CSD_VERSION_1 = 0,						// enum CSD version 1.0 - 1.1, Version 2.00/Standard Capacity
				CSD_VERSION_2 = 1,						// enum CSD cersion 2.0, Version 2.00/High Capacity and Extended Capacity
			} csd_structure : 2;						// @127-126	CSD Structure Version as on SD CSD bits 
			unsigned spec_vers : 6;						// @125-120 CSD version as on SD CSD bits
			u8 taac;								// @119-112	taac as on SD CSD bits
			u8 nsac;								// @111-104	nsac as on SD CSD bits
			u8 tran_speed;							// @103-96	trans_speed as on SD CSD bits
		};
		u32 Raw32_0;								// @127-96	Union to access 32 bits as a u32		
	};
	union {
		struct __attribute__((__packed__, aligned(1))) {
			unsigned ccc : 12;							// @95-84	ccc as on SD CSD bits
			unsigned read_bl_len : 4;					// @83-80	read_bl_len on SD CSD bits
			unsigned read_bl_partial : 1;				// @79		read_bl_partial as on SD CSD bits
			unsigned write_blk_misalign : 1;			// @78		write_blk_misalign as on SD CSD bits
			unsigned read_blk_misalign : 1;				// @77		read_blk_misalign as on SD CSD bits
			unsigned dsr_imp : 1;						// @76		dsr_imp as on SD CSD bits
			unsigned c_size : 12;						// @75-64   Version 1 C_Size as on SD CSD bits
		};
		u32 Raw32_1;								// @0-31	Union to access 32 bits as a u32		
	};
	union {
		struct __attribute__((__packed__, aligned(1))) {
			union {
				struct __attribute__((__packed__, aligned(1))) {
					unsigned vdd_r_curr_min : 3;		// @61-59	vdd_r_curr_min as on SD CSD bits
					unsigned vdd_r_curr_max : 3;		// @58-56	vdd_r_curr_max as on SD CSD bits
					unsigned vdd_w_curr_min : 3;		// @55-53	vdd_w_curr_min as on SD CSD bits
					unsigned vdd_w_curr_max : 3;		// @52-50	vdd_w_curr_max as on SD CSD bits
					unsigned c_size_mult : 3;			// @49-47	c_size_mult as on SD CSD bits
					unsigned reserved0 : 7;				// reserved for CSD ver 2.0 size match
				};
				unsigned ver2_c_size : 22;				// Version 2 C_Size
			};
			unsigned erase_blk_en : 1;					// @46		erase_blk_en as on SD CSD bits
			unsigned sector_size : 7;					// @45-39	sector_size as on SD CSD bits
			unsigned reserved1 : 2;						// 2 Spares bit unused
		};
		u32 Raw32_2;								// @0-31	Union to access 32 bits as a u32		
	};
	union {
		struct __attribute__((__packed__, aligned(1))) {
			unsigned wp_grp_size : 7;					// @38-32	wp_grp_size as on SD CSD bits
			unsigned wp_grp_enable : 1;					// @31		wp_grp_enable as on SD CSD bits
			unsigned reserved2 : 2;						// @30-29 	Write as zero read as don't care
			unsigned r2w_factor : 3;					// @28-26	r2w_factor as on SD CSD bits
			unsigned write_bl_len : 4;					// @25-22	write_bl_len as on SD CSD bits
			unsigned write_bl_partial : 1;				// @21		write_bl_partial as on SD CSD bits
			unsigned default_ecc : 5;					// @20-16	default_ecc as on SD CSD bits
			unsigned file_format_grp : 1;				// @15		file_format_grp as on SD CSD bits
			unsigned copy : 1;							// @14		copy as on SD CSD bits
			unsigned perm_write_protect : 1;			// @13		perm_write_protect as on SD CSD bits
			unsigned tmp_write_protect : 1;				// @12		tmp_write_protect as on SD CSD bits
			enum {
				FAT_PARTITION_TABLE = 0,				// enum SD card is FAT with partition table
				FAT_NO_PARTITION_TABLE = 1,				// enum SD card is FAT with no partition table
				FS_UNIVERSAL = 2,						// enum	SD card file system is universal
				FS_OTHER = 3,							// enum	SD card file system is other
			} file_format : 2;							// @11-10	File format as on SD CSD bits
			unsigned ecc : 2;							// @9-8		ecc as on SD CSD bits
			unsigned reserved3 : 1;						// 1 spare bit unused
		};
		u32 Raw32_3;								// @0-31	Union to access 32 bits as a u32		
	};
};


/***************************************************************************}
{					      PUBLIC INTERFACE ROUTINES			                }
****************************************************************************/

/*==========================================================================}
{						 PUBLIC RAW SD CARD ROUTINES						}
{==========================================================================*/

extern void bcm283x_sdcard_init(void);

SDRESULT sdInitCard();
struct CSD* sdCardCSD(void);
SDRESULT sdTransferBlocks(u32 startBlock, u32 numBlocks, u8* buffer, bool write);
SDRESULT sdClearBlocks(u32 startBlock, u32 numBlocks);

#endif
