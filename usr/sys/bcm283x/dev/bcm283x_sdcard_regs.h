#ifndef __V7_DEV_BCM283X_SDCARD_REGS_H
#define __V7_DEV_BCM283X_SDCARD_REGS_H

extern u32 _bcm283x_iobase;

#define EMMC_OFFSET                   0x00300000
#define EMMC_BASE                     ((_bcm283x_iobase) + (EMMC_OFFSET))

#define EMMC_ARG2_REG_OFFSET          0x00000000  /* ACMD23 Argument                   */
#define EMMC_ARG2_REG                 ((EMMC_BASE) + (EMMC_ARG2_REG))

/* BLKSIZECNT */
#define EMMC_BLKSIZECNT_REG_OFFSET    0x00000004  /* Block Size and Count              */
#define EMMC_BLKSIZECNT_REG           ((EMMC_BASE) + (EMMC_BLKSIZECNT_REG_OFFSET))
#define BLKSIZECNT_RESERVED_MASK      0x0000fc00U
#define BLKSIZE_SHIFT                 0
#define BLKSIZE_MASK                  0x00000f33U
#define BLKCNT_SHIFT                  16
#define BLKCNT_MASK                   0xffff0000U

#define EMMC_ARG1_REG_OFFSET          0x00000008  /* Argument                          */
#define EMMC_ARG1_REG                 ((EMMC_BASE) + (EMMC_ARG1_REG_OFFSET))

/* CMDTM */
#define EMMC_CMDTM_REG_OFFSET         0x0000000c  /* Command and Transfer Mode         */
#define EMMC_CMDTM_REG                ((EMMC_BASE) + (EMMC_CMDTM_REG_OFFSET))
#define CMDTM_RESERVED_MASK           0xc004ffc1
#define TM_BLKCNT_EN_SHIFT            1
#define TM_BLKCNT_EN_MASK             BIT(TM_BLKCNT_EN_SHIFT)
#define TM_AUTO_CMD_EN_SHIFT          2
#define TM_AUTO_CMD_EN_MASK           0x0000000c
#define TM_DAT_DIR_SHIFT              4
#define TM_DAT_DIR_MASK               BIT(TM_DAT_DIR_SHIFT)
# define TM_DAT_DIR_HOST2CARD         (0x0U << TM_DAT_DIR_SHIFT)
# define TM_DAT_DIR_CARD2HOST         (0x1U << TM_DAT_DIR_SHIFT)
#define TM_MULTI_BLOCK_SHIFT          5
#define TM_MULTI_BLOCK_MASK           BIT(TM_MULTI_BLOCK_SHIFT)
#define CMD_RSPNS_TYPE_SHIFT          16
#define CMD_RSPNS_TYPE_MASK           0x00030000
# define CMD_NO_RESP                  (0x0U << CMD_RSPNS_TYPE_SHIFT)
# define CMD_136BIT_RESP              (0x1U << CMD_RSPNS_TYPE_SHIFT)
# define CMD_48BIT_RESP               (0x2U << CMD_RSPNS_TYPE_SHIFT)
# define CMD_BUSY48BIT_RESP           (0x3U << CMD_RSPNS_TYPE_SHIFT)
#define CMD_CRCCHK_EN_SHIFT           19
#define CMD_CRCCHK_EN_MASK            BIT(CMD_CRCCHK_EN_SHIFT)
#define CMD_IXCHK_EN_SHIFT            20
#define CMD_IXCHK_EN_MASK             BIT(CMD_IXCHK_EN_SHIFT)
#define CMD_ISDATA_SHIFT              21
#define CMD_ISDATA_MASK               BIT(CMD_ISDATA_SHIFT)
#define CMD_TYPE_SHIFT                22
#define CMD_TYPE_MASK                 0x00c00000
#define CMD_INDEX_SHIFT               24
#define CMD_INDEX_MASK                0x3f000000

#if 0
/* TODO: enable these when we fix the struct weirdness */
/* values for TM_AUTO_CMD_EN */
#define TM_NO_COMMAND                 (0U << TM_AUTO_CMD_EN_SHIFT)
#define TM_CMD12                      (1U << TM_AUTO_CMD_EN_SHIFT)
#define TM_CMD23                      (2U << TM_AUTO_CMD_EN_SHIFT)
#define TM_RESERVED                   (3U << TM_AUTO_CMD_EN_SHIFT)

/* values for CMD_RSPNS_TYPE */
#define CMD_NO_RESP                   (0U << CMD_RSPNS_TYPE_SHIFT)
#define CMD_136BIT_RESP               (1U << CMD_RSPNS_TYPE_SHIFT)
#define CMD_48BIT_RESP                (2U << CMD_RSPNS_TYPE_SHIFT)
#define CMD_BUSY48BIT_RESP            (3U << CMD_RSPNS_TYPE_SHIFT)

/* values for CMD_TYPE */
#define CMD_TYPE_NORMAL               (0U << CMD_TYPE_NORMAL)
#define CMD_TYPE_SUSPEND              (1U << CMD_TYPE_NORMAL)
#define CMD_TYPE_RESUME               (2U << CMD_TYPE_NORMAL)
#define CMD_TYPE_ABORT                (3U << CMD_TYPE_NORMAL)
#endif

/* RESP0 */
#define EMMC_RESP0_REG_OFFSET         0x00000010  /* Response bits 31 : 0              */
#define EMMC_RESP0_REG                ((EMMC_BASE) + (EMMC_RESP0_REG_OFFSET))

/* RESP1 */
#define EMMC_RESP1_REG_OFFSET         0x00000014  /* Response bits 63 : 32             */
#define EMMC_RESP1_REG                ((EMMC_BASE) + (EMMC_RESP1_REG_OFFSET))

/* RESP2 */
#define EMMC_RESP2_REG_OFFSET         0x00000018  /* Response bits 95 : 64             */
#define EMMC_RESP2_REG                ((EMMC_BASE) + (EMMC_RESP2_REG_OFFSET))

/* RESP3 */
#define EMMC_RESP3_REG_OFFSET         0x0000001c  /* Response bits 127 : 96            */
#define EMMC_RESP3_REG                ((EMMC_BASE) + (EMMC_RESP3_REG_OFFSET))

/* DATA */
#define EMMC_DATA_REG_OFFSET          0x00000020  /* Data                              */
#define EMMC_DATA_REG                 ((EMMC_BASE) + (EMMC_DATA_REG_OFFSET))

/* STATUS */
#define EMMC_STATUS_REG_OFFSET        0x00000024  /* Status                            */
#define EMMC_STATUS_REG               ((EMMC_BASE) + (EMMC_STATUS_REG_OFFSET))
#define STATUS_RESERVED_MASK          0xe00ffcf8
#define CMD_INHIBIT_SHIFT             0
#define CMD_INHIBIT_MASK              BIT(CMD_INHIBIT_SHIFT)
#define DAT_INHIBIT_SHIFT             1
#define DAT_INHIBIT_MASK              BIT(DAT_INHIBIT_SHIFT)
#define DAT_ACTIVE_SHIFT              2
#define DAT_ACTIVE_MASK               BIT(DAT_ACTIVE_SHIFT)
#define WRITE_TRANSFER_SHIFT          8
#define WRITE_TRANSFER_MASK           BIT(WRITE_TRANSFER_SHIFT)
#define READ_TRANSFER_SHIFT           9
#define READ_TRANSFER_MASK            BIT(READ_TRANSFER_SHIFT)
#define DAT_LEVEL0_SHIFT              20
#define DAT_LEVEL0_MASK               0x00f00000
#define CMD_LEVEL_SHIFT               24
#define CMD_LEVEL_MASK                BIT(CMD_LEVEL_SHIFT)
#define DAT_LEVEL1_SHIFT              25
#define DAT_LEVEL1_MASK               0x1e000000

/* CONTROL0 */
#define EMMC_CONTROL0_REG_OFFSET      0x00000028  /* Host Configuration bits           */
#define EMMC_CONTROL0_REG             ((EMMC_BASE) + (EMMC_CONTROL0_REG_OFFSET))
#define CONTROL0_RESERVED_MASK        0xff80ffd9
#define HCTL_DWIDTH_SHIFT             1
#define HCTL_DWIDTH_MASK              BIT(HCTL_DWIDTH_SHIFT)
#define HCTL_HS_EN_SHIFT              2
#define HCTL_HS_EN_MASK               BIT(HCTL_HS_EN_SHIFT)
#define HCTL_8BIT_SHIFT               5
#define HCTL_8BIT_MASK                BIT(HCTL_8BIT_SHIFT)
#define GAP_STOP_SHIFT                16
#define GAP_STOP_MASK                 BIT(GAP_STOP_SHIFT)
#define GAP_RESTART_SHIFT             17
#define GAP_RESTART_MASK              BIT(GAP_RESTART_SHIFT)
#define READWAIT_EN_SHIFT             18
#define READWAIT_EN_MASK              BIT(READWAIT_EN_SHIFT)
#define GAP_IEN_SHIFT                 19
#define GAP_IEN_MASK                  BIT(GAP_IEN_SHIFT)
#define SPI_MODE_SHIFT                20
#define SPI_MODE_MASK                 BIT(SPI_MODE_SHIFT)
#define BOOT_EN_SHIFT                 21
#define BOOT_EN_MASK                  BIT(BOOT_EN_SHIFT)
#define ALT_BOOT_EN_SHIFT             22
#define ALT_BOOT_EN_MASK              BIT(ALT_BOOT_EN_SHIFT)

/* CONTROL1 */
#define EMMC_CONTROL1_REG_OFFSET      0x0000002c  /* Host Configuration bits           */
#define EMMC_CONTROL1_REG             ((EMMC_BASE) + (EMMC_CONTROL1_REG_OFFSET))
#define CONTROL1_RESERVED_MASK        0xf8f00018
#define CLK_INTLEN_SHIFT              0
#define CLK_INTLEN_MASK               BIT(CLK_INTLEN_SHIFT)
#define CLK_STABLE_SHIFT              1
#define CLK_STABLE_MASK               BIT(CLK_STABLE_SHIFT)
#define CLK_EN_SHIFT                  2
#define CLK_EN_MASK                   BIT(CLK_EN_SHIFT)
#define CLK_GENSEL_SHIFT              5
#define CLK_GENSEL_MASK               BIT(CLK_GENSEL_SHIFT)
#define CLK_FREQ_MS2_SHIFT            6
#define CLK_FREQ_MS2_MASK             0x000000c0
#define CLK_FREQ8_SHIFT               8
#define CLK_FREQ8_MASK                0x0000ff00
#define DATA_TOUNIT_SHIFT             16
#define DATA_TOUNIT_MASK              0x000f0000
#define SRST_HC_SHIFT                 24
#define SRST_HC_MASK                  BIT(SRST_HC_SHIFT)
#define SRST_CMD_SHIFT                25
#define SRST_CMD_MASK                 BIT(SRST_CMD_SHIFT)
#define SRST_DATA_SHIFT               26
#define SRST_DATA_MASK                BIT(SRST_DATA_SHIFT)

/* INTERRUPT, IRPT_MASK, IRPT_EN */
#define EMMC_INTERRUPT_REG_OFFSET     0x00000030  /* Interrupt Flags                   */
#define EMMC_INTERRUPT_REG            ((EMMC_BASE) + (EMMC_INTERRUPT_REG_OFFSET))
#define EMMC_IRPT_MASK_REG_OFFSET     0x00000034  /* Interrupt Flag Enable             */
#define EMMC_IRPT_MASK_REG            ((EMMC_BASE) + (EMMC_IRPT_MASK_REG_OFFSET))
#define EMMC_IRPT_EN_REG_OFFSET       0x00000038  /* Interrupt Generation Enable       */
#define EMMC_IRPT_EN_REG              ((EMMC_BASE) + (EMMC_IRPT_EN_REG_OFFSET))
#define INTERRUPT_RESERVED_MASK       0xfe800ec8
#define CMD_DONE_SHIFT                0
#define CMD_DONE_MASK                 BIT(CMD_DONE_SHIFT)
#define DATA_DONE_SHIFT               1
#define DATA_DONE_MASK                BIT(DATA_DONE_SHIFT)
#define BLOCK_GAP_SHIFT               2
#define BLOCK_GAP_MASK                BIT(BLOCK_GAP_SHIFT)
#define WRITE_RDY_SHIFT               4
#define WRITE_RDY_MASK                BIT(WRITE_RDY_SHIFT)
#define READ_RDY_SHIFT                5
#define READ_RDY_MASK                 BIT(READ_RDY_SHIFT)
#define CARD_SHIFT                    8
#define CARD_MASK                     BIT(CARD_SHIFT)
#define RETUNE_SHIFT                  12
#define RETUNE_MASK                   BIT(RETUNE_SHIFT)
#define BOOTACK_SHIFT                 13
#define BOOTACK_MASK                  BIT(BOOTACK_SHIFT)
#define ENDBOOT_SHIFT                 14
#define ENDBOOT_MASK                  BIT(ENDBOOT_SHIFT)
#define ERR_SHIFT                     15
#define ERR_MASK                      BIT(ERR_SHIFT)
#define CTO_ERR_SHIFT                 16
#define CTO_ERR_MASK                  BIT(CTO_ERR_SHIFT)
#define CCRC_ERR_SHIFT                17
#define CCRC_ERR_MASK                 BIT(CCRC_ERR_SHIFT)
#define CEND_ERR_SHIFT                18
#define CEND_ERR_MASK                 BIT(CEND_ERR_SHIFT)
#define CBAD_ERR_SHIFT                19
#define CBAD_ERR_MASK                 BIT(CBAD_ERR_SHIFT)
#define DTO_ERR_SHIFT                 20
#define DTO_ERR_MASK                  BIT(DTO_ERR_SHIFT)
#define DCRC_ERR_SHIFT                21
#define DCRC_ERR_MASK                 BIT(DCRC_ERR_SHIFT)
#define DEND_ERR_SHIFT                22
#define DEND_ERR_MASK                 BIT(DEND_ERR_SHIFT)
#define ACMD_ERR_SHIFT                24
#define ACMD_ERR_MASK                 BIT(ACMD_ERR_SHIFT)

/* CONTROL2 */
#define EMMC_CONTROL2_REG_OFFSET      0x0000003c  /* Host Configuration bits           */
                                                  /* Force Interrupt Event             */
                                                  /* Timeout in boot mode              */
                                                  /* Debug Bus Configuration           */
                                                  /* Extension FIFO Configuration      */
                                                  /* Extension FIFO Enable             */
#define EMMC_CONTROL2_REG             ((EMMC_BASE) + (EMMC_CONTROL2_REG_OFFSET))
#define CONTROL2_RESERVED_MASK        0xff38ff60
#define ACNOX_ERR_SHIFT               0
#define ACNOX_ERR_MASK                BIT(ACNOX_ERR_MASK)
#define ACTO_ERR_SHIFT                1
#define ACTO_ERR_MASK                 BIT(ACTO_ERR_MASK)
#define ACCRC_ERR_SHIFT               2
#define ACCRC_ERR_MASK                BIT(ACCRC_ERR_SHIFT)
#define ACEND_ERR_SHIFT               3
#define ACEND_ERR_MASK                BIT(ACEND_ERR_MASK)
#define ACBAD_ERR_SHIFT               4
#define ACBAD_ERR_MASK                BIT(ACBAD_ERR_MASK)
#define NOTC12_ERR_SHIFT              7
#define NOTC12_ERR_MASK               BIT(NOTC12_ERR_SHIFT)
#define UHSMODE_SHIFT                 16
#define UHSMODE_MASK                  0x00070000
#define UHSMODE_SDR12                 0
#define UHSMODE_SDR25                 1
#define UHSMODE_SDR50                 2
#define UHSMODE_SDR104                3
#define UHSMODE_DDR50                 4
#define TUNEON_SHIFT                  22
#define TUNEON_MASK                   BIT(TUNEON_SHIFT)
#define TUNED_SHIFT                   23
#define TUNED_MASK                    BIT(TUNED_SHIFT)

/* FORCE_IRPT */
#define EMMC_FORCE_IRPT_REG_OFFSET    0x00000050
#define EMMC_FORCE_IRPT_REG           ((EMMC_BASE) + (EMMC_FORCE_IRPT_REG_OFFSET))

/* BOOT_TIMEOUT */
#define EMMC_BOOT_TIMEOUT_REG_OFFSET  0x00000070
#define EMMC_BOOT_TIMEOUT_REG         ((EMMC_BASE) + (EMMC_BOOT_TIMEOUT_REG_OFFSET))

/* DBG_SEL */
#define EMMC_DBG_SEL_REG_OFFSET       0x00000074
#define EMMC_DBG_SEL_REG              ((EMMC_BASE) + (EMMC_DBG_SEL_REG_OFFSET))

/* EXRDFIFO_CFG */
#define EMMC_EXRDFIFO_CFG_REG_OFFSET  0x00000080
#define EMMC_EXRDFIFO_CFG_REG         ((EMMC_BASE) + (EMMC_EXRDFIFO_CFG_REG_OFFSET))

/* EXRDFIFO_EN */
#define EMMC_EXRDFIFO_EN_REG_OFFSET   0x00000084
#define EMMC_EXRDFIFO_EN_REG          ((EMMC_BASE) + (EMMC_EXRDFIFO_EN_REG_OFFSET))

/* TUNE_STEP */
#define EMMC_TUNE_STEP_REG_OFFSET     0x00000088  /* Delay per card clock tuning step  */
                                                  /* Card clock tuning steps for SDR   */
                                                  /* Card clock tuning steps for DDR   */
                                                  /* SPI Interrupt Support             */
#define EMMC_TUNE_STEP_REG            ((EMMC_BASE) + (EMMC_TUNE_STEP_REG_OFFSET))
#define TUNE_STEP_RESERVED_MASK       0xfffffff8
#define TUNE_DELAY_SHIFT              0
#define TUNE_DELAY_MASK               0x00000007
#define TUNE_DELAY_200PS              0
#define TUNE_DELAY_400PS              1
#define TUNE_DELAY_400PSA             2
#define TUNE_DELAY_600PS              3
#define TUNE_DELAY_700PS              4
#define TUNE_DELAY_900PS              5
#define TUNE_DELAY_900PSA             6
#define TUNE_DELAY_1100PS             7

/* TUNE_STEPS_STD */
#define EMMC_TUNE_STEPS_STD_REG_OFFSET 0x0000008c
#define EMMC_TUNE_STEPS_STD_REG       ((EMMC_BASE) + (EMMC_TUNE_STEPS_STD_REG_OFFSET))

/* TUNE_STEPS_DDR */
#define EMMC_TUNE_STEPS_DDR_REG_OFFSET 0x00000090
#define EMMC_TUNE_STEPS_DDR_REG       ((EMMC_BASE) + (EMMC_TUNE_STEPS_DDR_REG_OFFSET))

/* SPI_INT_SPT */
#define EMMC_SPI_INT_SPT_REG_OFFSET   0x000000f0
#define EMMC_SPI_INT_SPT_REG          ((EMMC_BASE) + (EMMC_SPI_INT_SPT_REG_OFFSET))

/* SLOTISR_VER */
#define EMMC_SLOTISR_VER_REG_OFFSET   0x000000fc  /* Slot Interrupt Status and Version */
#define EMMC_SLOTISR_VER_REG          ((EMMC_BASE) + (EMMC_SLOTISR_VER_REG_OFFSET))
#define SLOTISR_RESERVED_MASK         0x0000ff00
#define SLOT_STATUS_SHIFT             0
#define SLOT_STATUS_MASK              0x000000ff
#define SDVERSION_SHIFT               16
#define SDVERSION_MASK                0x00ff0000
#define VENDOR_SHIFT                  24
#define VENDOR_MASK                   0xff000000

#endif
