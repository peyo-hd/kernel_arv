/* SPDX-License-Identifier: GPL-2.0 */
/*
 * stf_isp.h
 *
 * StarFive Camera Subsystem - ISP Module
 *
 * Copyright (C) 2021-2023 StarFive Technology Co., Ltd.
 */

#ifndef STF_ISP_H
#define STF_ISP_H

#include <media/media-entity.h>
#include <media/v4l2-subdev.h>

#define STF_ISP_SETFILE     "stf_isp0_fw.bin"

#define ISP_RAW_DATA_BITS       12
#define SCALER_RATIO_MAX        1
#define STF_ISP_REG_OFFSET_MAX  0x0fff
#define STF_ISP_REG_DELAY_MAX   100

/* isp registers */
#define ISP_REG_CSI_INPUT_EN_AND_STATUS	0x000
#define CSI_SCD_ERR	BIT(6)
#define CSI_ITU656_ERR	BIT(4)
#define CSI_ITU656_F	BIT(3)
#define CSI_SCD_DONE	BIT(2)
#define CSI_BUSY_S	BIT(1)
#define CSI_EN_S	BIT(0)

#define ISP_REG_CSIINTS_ADDR	0x008
#define CSI_INTS(n)	((n) << 16)
#define CSI_SHA_M(n)	((n) << 0)
#define CSI_INTS_MASK	GENMASK(17, 16)

#define ISP_REG_CSI_MODULE_CFG	0x010
#define CSI_DUMP_EN	BIT(19)
#define CSI_VS_EN	BIT(18)
#define CSI_SC_EN	BIT(17)
#define CSI_OBA_EN	BIT(16)
#define CSI_AWB_EN	BIT(7)
#define CSI_LCCF_EN	BIT(6)
#define CSI_OECFHM_EN	BIT(5)
#define CSI_OECF_EN	BIT(4)
#define CSI_LCBQ_EN	BIT(3)
#define CSI_OBC_EN	BIT(2)
#define CSI_DEC_EN	BIT(1)
#define CSI_DC_EN	BIT(0)

#define ISP_REG_SENSOR	0x014
#define DVP_SYNC_POL(n)	((n) << 2)
#define ITU656_EN(n)	((n) << 1)
#define IMAGER_SEL(n)	((n) << 0)

#define ISP_REG_RAW_FORMAT_CFG	0x018
#define SMY13(n)	((n) << 14)
#define SMY12(n)	((n) << 12)
#define SMY11(n)	((n) << 10)
#define SMY10(n)	((n) << 8)
#define SMY3(n)	((n) << 6)
#define SMY2(n)	((n) << 4)
#define SMY1(n)	((n) << 2)
#define SMY0(n)	((n) << 0)

#define ISP_REG_PIC_CAPTURE_START_CFG	0x01c
#define VSTART_CAP(n)	((n) << 16)
#define HSTART_CAP(n)	((n) << 0)

#define ISP_REG_PIC_CAPTURE_END_CFG	0x020
#define VEND_CAP(n)	((n) << 16)
#define HEND_CAP(n)	((n) << 0)

#define ISP_REG_DUMP_CFG_0	0x024
#define ISP_REG_DUMP_CFG_1	0x028
#define DUMP_ID(n)	((n) << 24)
#define DUMP_SHT(n)	((n) << 20)
#define DUMP_BURST_LEN(n)	((n) << 16)
#define DUMP_SD(n)	((n) << 0)
#define DUMP_BURST_LEN_MASK	GENMASK(17, 16)
#define DUMP_SD_MASK	GENMASK(15, 0)

#define ISP_REG_DEC_CFG	0x030
#define DEC_V_KEEP(n)	((n) << 24)
#define DEC_V_PERIOD(n)	((n) << 16)
#define DEC_H_KEEP(n)	((n) << 8)
#define DEC_H_PERIOD(n)	((n) << 0)

#define ISP_REG_OBC_CFG	0x034
#define OBC_W_H(y)	((y) << 4)
#define OBC_W_W(x)	((x) << 0)

#define ISP_REG_DC_CFG_1	0x044
#define DC_AXI_ID(n)	((n) << 0)

#define ISP_REG_LCCF_CFG_0	0x050
#define Y_DISTANCE(y)	((y) << 16)
#define X_DISTANCE(x)	((x) << 16)

#define ISP_REG_LCCF_CFG_1	0x058
#define LCCF_MAX_DIS(n)	((n) << 0)

#define ISP_REG_LCBQ_CFG_0	0x074
#define H_LCBQ(y)	((y) << 12)
#define W_LCBQ(x)	((x) << 8)

#define ISP_REG_LCBQ_CFG_1	0x07c
#define Y_COOR(y)	((y) << 16)
#define X_COOR(x)	((x) << 0)

#define ISP_REG_OBA_CFG_0	0x090
#define VSTART(n)	((n) << 16)
#define HSTART(n)	((n) << 0)

#define ISP_REG_OBA_CFG_1	0x094
#define VEND(n)	((n) << 16)
#define HEND(n)	((n) << 0)

#define ISP_REG_SCD_CFG_0	0x098
#define ISP_REG_SCD_CFG_1	0x09c
#define AXI_ID(n)	((n) << 24)

#define ISP_REG_SC_CFG_0	0x0b8
#define VSTART_SC(n)	((n) << 16)
#define HSTART_SC(n)	((n) << 0)

#define ISP_REG_SC_CFG_1	0x0bc
#define SEL_SC(n)	((n) << 30)
#define AWB_PS_GRB_BA(n)	((n) << 16)
#define HEIGHT_SC(n)	((n) << 8)
#define WIDTH_SC(n)	((n) << 0)

#define ISP_REG_SC_AF	0x0c0
#define AF_ES_HTHR(n)	((n) << 16)
#define AF_ES_VTHR(n)	((n) << 8)
#define AF_ES_VE(n)	((n) << 3)
#define AF_ES_HE(n)	((n) << 2)
#define AF_ES_SM(n)	((n) << 1)
#define AF_ES_HM(n)	((n) << 0)

#define ISP_REG_SC_AWB_PS_CFG_0	0x0c4
#define AWB_PS_GU(n)	((n) << 24)
#define AWB_PS_GL(n)	((n) << 16)
#define AWB_PS_RU(n)	((n) << 8)
#define AWB_PS_RL(n)	((n) << 0)
#define ISP_REG_SC_AWB_PS_CFG_1	0x0c8
#define AWB_PS_YU(n)	((n) << 24)
#define AWB_PS_YL(n)	((n) << 16)
#define AWB_PS_BU(n)	((n) << 8)
#define AWB_PS_BL(n)	((n) << 0)
#define ISP_REG_SC_AWB_PS_CFG_2	0x0cc
#define AWB_PS_GRU(n)	((n) << 16)
#define AWB_PS_GRL(n)	((n) << 0)
#define ISP_REG_SC_AWB_PS_CFG_3	0x0d0
#define AWB_PS_GBU(n)	((n) << 16)
#define AWB_PS_GBL(n)	((n) << 0)
#define ISP_REG_SC_AWB_PS_CFG_4	0x0d4
#define AWB_PS_GRBU(n)	((n) << 16)
#define AWB_PS_GRBL(n)	((n) << 0)

#define ISP_REG_SC_DEC	0x0d8
#define VKEEP(n)	((n) << 24)
#define VPERIOD(n)	((n) << 16)
#define HKEEP(n)	((n) << 8)
#define HPERIOD(n)	((n) << 0)

#define ISP_REG_LCCF_CFG_2	0x0e0
#define ISP_REG_LCCF_CFG_3	0x0e4
#define ISP_REG_LCCF_CFG_4	0x0e8
#define ISP_REG_LCCF_CFG_5	0x0ec
#define LCCF_F2_PAR(n)	((n) << 16)
#define LCCF_F1_PAR(n)	((n) << 0)

#define ISP_REG_OECF_X0_CFG_0	0x100
#define ISP_REG_OECF_X0_CFG_1	0x104
#define ISP_REG_OECF_X0_CFG_2	0x108
#define ISP_REG_OECF_X0_CFG_3	0x10c
#define ISP_REG_OECF_X0_CFG_4	0x110
#define ISP_REG_OECF_X0_CFG_5	0x114
#define ISP_REG_OECF_X0_CFG_6	0x118
#define ISP_REG_OECF_X0_CFG_7	0x11c
#define ISP_REG_OECF_X1_CFG_0	0x120
#define ISP_REG_OECF_X1_CFG_1	0x124
#define ISP_REG_OECF_X1_CFG_2	0x128
#define ISP_REG_OECF_X1_CFG_3	0x12c
#define ISP_REG_OECF_X1_CFG_4	0x130
#define ISP_REG_OECF_X1_CFG_5	0x134
#define ISP_REG_OECF_X1_CFG_6	0x138
#define ISP_REG_OECF_X1_CFG_7	0x13c
#define ISP_REG_OECF_X2_CFG_0	0x140
#define ISP_REG_OECF_X2_CFG_1	0x144
#define ISP_REG_OECF_X2_CFG_2	0x148
#define ISP_REG_OECF_X2_CFG_3	0x14c
#define ISP_REG_OECF_X2_CFG_4	0x150
#define ISP_REG_OECF_X2_CFG_5	0x154
#define ISP_REG_OECF_X2_CFG_6	0x158
#define ISP_REG_OECF_X2_CFG_7	0x15c
#define ISP_REG_OECF_X3_CFG_0	0x160
#define ISP_REG_OECF_X3_CFG_1	0x164
#define ISP_REG_OECF_X3_CFG_2	0x168
#define ISP_REG_OECF_X3_CFG_3	0x16c
#define ISP_REG_OECF_X3_CFG_4	0x170
#define ISP_REG_OECF_X3_CFG_5	0x174
#define ISP_REG_OECF_X3_CFG_6	0x178
#define ISP_REG_OECF_X3_CFG_7	0x17c
#define OCEF_X_PAR_H(n)	((n) << 16)
#define OCEF_X_PAR_L(n)	((n) << 0)

#define ISP_REG_OECF_Y0_CFG_0	0x180
#define ISP_REG_OECF_Y0_CFG_1	0x184
#define ISP_REG_OECF_Y0_CFG_2	0x188
#define ISP_REG_OECF_Y0_CFG_3	0x18c
#define ISP_REG_OECF_Y0_CFG_4	0x190
#define ISP_REG_OECF_Y0_CFG_5	0x194
#define ISP_REG_OECF_Y0_CFG_6	0x198
#define ISP_REG_OECF_Y0_CFG_7	0x19c
#define ISP_REG_OECF_Y1_CFG_0	0x1a0
#define ISP_REG_OECF_Y1_CFG_1	0x1a4
#define ISP_REG_OECF_Y1_CFG_2	0x1a8
#define ISP_REG_OECF_Y1_CFG_3	0x1ac
#define ISP_REG_OECF_Y1_CFG_4	0x1b0
#define ISP_REG_OECF_Y1_CFG_5	0x1b4
#define ISP_REG_OECF_Y1_CFG_6	0x1b8
#define ISP_REG_OECF_Y1_CFG_7	0x1bc
#define ISP_REG_OECF_Y2_CFG_0	0x1c0
#define ISP_REG_OECF_Y2_CFG_1	0x1c4
#define ISP_REG_OECF_Y2_CFG_2	0x1c8
#define ISP_REG_OECF_Y2_CFG_3	0x1cc
#define ISP_REG_OECF_Y2_CFG_4	0x1d0
#define ISP_REG_OECF_Y2_CFG_5	0x1d4
#define ISP_REG_OECF_Y2_CFG_6	0x1d8
#define ISP_REG_OECF_Y2_CFG_7	0x1dc
#define ISP_REG_OECF_Y3_CFG_0	0x1e0
#define ISP_REG_OECF_Y3_CFG_1	0x1e4
#define ISP_REG_OECF_Y3_CFG_2	0x1e8
#define ISP_REG_OECF_Y3_CFG_3	0x1ec
#define ISP_REG_OECF_Y3_CFG_4	0x1f0
#define ISP_REG_OECF_Y3_CFG_5	0x1f4
#define ISP_REG_OECF_Y3_CFG_6	0x1f8
#define ISP_REG_OECF_Y3_CFG_7	0x1fc
#define OCEF_Y_PAR_H(n)	((n) << 16)
#define OCEF_Y_PAR_L(n)	((n) << 0)

#define ISP_REG_OECF_S0_CFG_0	0x200
#define ISP_REG_OECF_S0_CFG_1	0x204
#define ISP_REG_OECF_S0_CFG_2	0x208
#define ISP_REG_OECF_S0_CFG_3	0x20c
#define ISP_REG_OECF_S0_CFG_4	0x210
#define ISP_REG_OECF_S0_CFG_5	0x214
#define ISP_REG_OECF_S0_CFG_6	0x218
#define ISP_REG_OECF_S0_CFG_7	0x21c
#define ISP_REG_OECF_S1_CFG_0	0x220
#define ISP_REG_OECF_S1_CFG_1	0x224
#define ISP_REG_OECF_S1_CFG_2	0x228
#define ISP_REG_OECF_S1_CFG_3	0x22c
#define ISP_REG_OECF_S1_CFG_4	0x230
#define ISP_REG_OECF_S1_CFG_5	0x234
#define ISP_REG_OECF_S1_CFG_6	0x238
#define ISP_REG_OECF_S1_CFG_7	0x23c
#define ISP_REG_OECF_S2_CFG_0	0x240
#define ISP_REG_OECF_S2_CFG_1	0x244
#define ISP_REG_OECF_S2_CFG_2	0x248
#define ISP_REG_OECF_S2_CFG_3	0x24c
#define ISP_REG_OECF_S2_CFG_4	0x250
#define ISP_REG_OECF_S2_CFG_5	0x254
#define ISP_REG_OECF_S2_CFG_6	0x258
#define ISP_REG_OECF_S2_CFG_7	0x25c
#define ISP_REG_OECF_S3_CFG_0	0x260
#define ISP_REG_OECF_S3_CFG_1	0x264
#define ISP_REG_OECF_S3_CFG_2	0x268
#define ISP_REG_OECF_S3_CFG_3	0x26c
#define ISP_REG_OECF_S3_CFG_4	0x270
#define ISP_REG_OECF_S3_CFG_5	0x274
#define ISP_REG_OECF_S3_CFG_6	0x278
#define ISP_REG_OECF_S3_CFG_7	0x27c
#define OCEF_S_PAR_H(n)	((n) << 16)
#define OCEF_S_PAR_L(n)	((n) << 0)

#define ISP_REG_AWB_X0_CFG_0	0x280
#define ISP_REG_AWB_X0_CFG_1	0x284
#define ISP_REG_AWB_X1_CFG_0	0x288
#define ISP_REG_AWB_X1_CFG_1	0x28c
#define ISP_REG_AWB_X2_CFG_0	0x290
#define ISP_REG_AWB_X2_CFG_1	0x294
#define ISP_REG_AWB_X3_CFG_0	0x298
#define ISP_REG_AWB_X3_CFG_1	0x29c
#define AWB_X_SYMBOL1(n)	((n) << 16)
#define AWB_X_SYMBOL0(n)	((n) << 0)

#define ISP_REG_AWB_Y0_CFG_0	0x2a0
#define ISP_REG_AWB_Y0_CFG_1	0x2a4
#define ISP_REG_AWB_Y1_CFG_0	0x2a8
#define ISP_REG_AWB_Y1_CFG_1	0x2ac
#define ISP_REG_AWB_Y2_CFG_0	0x2b0
#define ISP_REG_AWB_Y2_CFG_1	0x2b4
#define ISP_REG_AWB_Y3_CFG_0	0x2b8
#define ISP_REG_AWB_Y3_CFG_1	0x2bc
#define AWB_Y_SYMBOL1(n)	((n) << 16)
#define AWB_Y_SYMBOL0(n)	((n) << 0)

#define ISP_REG_AWB_S0_CFG_0	0x2c0
#define ISP_REG_AWB_S0_CFG_1	0x2c4
#define ISP_REG_AWB_S1_CFG_0	0x2c8
#define ISP_REG_AWB_S1_CFG_1	0x2cc
#define ISP_REG_AWB_S2_CFG_0	0x2d0
#define ISP_REG_AWB_S2_CFG_1	0x2d4
#define ISP_REG_AWB_S3_CFG_0	0x2d8
#define ISP_REG_AWB_S3_CFG_1	0x2dc
#define AWB_S_SYMBOL1(n)	((n) << 16)
#define AWB_S_SYMBOL0(n)	((n) << 0)

#define ISP_REG_OBCG_CFG_0	0x2e0
#define ISP_REG_OBCG_CFG_1	0x2e4
#define ISP_REG_OBCG_CFG_2	0x2e8
#define ISP_REG_OBCG_CFG_3	0x2ec
#define ISP_REG_OBCO_CFG_0	0x2f0
#define ISP_REG_OBCO_CFG_1	0x2f4
#define ISP_REG_OBCO_CFG_2	0x2f8
#define ISP_REG_OBCO_CFG_3	0x2fc
#define D_S0(x)	((x) << 24)
#define C_S0(x)	((x) << 16)
#define B_S0(x)	((x) << 8)
#define A_S0(x)	((x) << 0)

#define ISP_REG_LCBQ_GAIN0_CFG_0	0x300
#define ISP_REG_LCBQ_GAIN0_CFG_1	0x304
#define ISP_REG_LCBQ_GAIN0_CFG_2	0x308
#define ISP_REG_LCBQ_GAIN0_CFG_3	0x30c
#define ISP_REG_LCBQ_GAIN0_CFG_4	0x310
#define ISP_REG_LCBQ_GAIN0_CFG_5	0x314
#define ISP_REG_LCBQ_GAIN0_CFG_6	0x318
#define ISP_REG_LCBQ_GAIN0_CFG_7	0x31c
#define ISP_REG_LCBQ_GAIN0_CFG_8	0x320
#define ISP_REG_LCBQ_GAIN0_CFG_9	0x324
#define ISP_REG_LCBQ_GAIN0_CFG_10	0x328
#define ISP_REG_LCBQ_GAIN0_CFG_11	0x32c
#define ISP_REG_LCBQ_GAIN0_CFG_12	0x330
#define ISP_REG_LCBQ_GAIN1_CFG_0	0x334
#define ISP_REG_LCBQ_GAIN1_CFG_1	0x338
#define ISP_REG_LCBQ_GAIN1_CFG_2	0x33c
#define ISP_REG_LCBQ_GAIN1_CFG_3	0x340
#define ISP_REG_LCBQ_GAIN1_CFG_4	0x344
#define ISP_REG_LCBQ_GAIN1_CFG_5	0x348
#define ISP_REG_LCBQ_GAIN1_CFG_6	0x34c
#define ISP_REG_LCBQ_GAIN1_CFG_7	0x350
#define ISP_REG_LCBQ_GAIN1_CFG_8	0x354
#define ISP_REG_LCBQ_GAIN1_CFG_9	0x358
#define ISP_REG_LCBQ_GAIN1_CFG_10	0x35c
#define ISP_REG_LCBQ_GAIN1_CFG_11	0x360
#define ISP_REG_LCBQ_GAIN1_CFG_12	0x364
#define ISP_REG_LCBQ_GAIN2_CFG_0	0x368
#define ISP_REG_LCBQ_GAIN2_CFG_1	0x36c
#define ISP_REG_LCBQ_GAIN2_CFG_2	0x370
#define ISP_REG_LCBQ_GAIN2_CFG_3	0x374
#define ISP_REG_LCBQ_GAIN2_CFG_4	0x378
#define ISP_REG_LCBQ_GAIN2_CFG_5	0x37c
#define ISP_REG_LCBQ_GAIN2_CFG_6	0x380
#define ISP_REG_LCBQ_GAIN2_CFG_7	0x384
#define ISP_REG_LCBQ_GAIN2_CFG_8	0x388
#define ISP_REG_LCBQ_GAIN2_CFG_9	0x38c
#define ISP_REG_LCBQ_GAIN2_CFG_10	0x390
#define ISP_REG_LCBQ_GAIN2_CFG_11	0x394
#define ISP_REG_LCBQ_GAIN2_CFG_12	0x398
#define ISP_REG_LCBQ_GAIN3_CFG_0	0x39c
#define ISP_REG_LCBQ_GAIN3_CFG_1	0x3a0
#define ISP_REG_LCBQ_GAIN3_CFG_2	0x3a4
#define ISP_REG_LCBQ_GAIN3_CFG_3	0x3a8
#define ISP_REG_LCBQ_GAIN3_CFG_4	0x3ac
#define ISP_REG_LCBQ_GAIN3_CFG_5	0x3b0
#define ISP_REG_LCBQ_GAIN3_CFG_6	0x3b4
#define ISP_REG_LCBQ_GAIN3_CFG_7	0x3b8
#define ISP_REG_LCBQ_GAIN3_CFG_8	0x3bc
#define ISP_REG_LCBQ_GAIN3_CFG_9	0x3c0
#define ISP_REG_LCBQ_GAIN3_CFG_10	0x3c4
#define ISP_REG_LCBQ_GAIN3_CFG_11	0x3c8
#define ISP_REG_LCBQ_GAIN3_CFG_12	0x3cc
#define GAIN_H(n)	((n) << 16)
#define GAIN_L(n)	((n) << 0)

#define ISP_REG_OECFHM_Y_CFG_0	0x3d0
#define ISP_REG_OECFHM_Y_CFG_1	0x3d4
#define ISP_REG_OECFHM_Y_CFG_2	0x3d8
#define ISP_REG_OECFHM_S_CFG_0	0x3dc
#define ISP_REG_OECFHM_S_CFG_1	0x3e0
#define ISP_REG_OECFHM_S_CFG_2	0x3e4
#define OECFHM_H(n)	((n) << 16)
#define OECFHM_L(n)	((n) << 0)

#define ISP_REG_LCBQ_OFFSET0_CFG_0	0x400
#define ISP_REG_LCBQ_OFFSET0_CFG_1	0x404
#define ISP_REG_LCBQ_OFFSET0_CFG_2	0x408
#define ISP_REG_LCBQ_OFFSET0_CFG_3	0x40c
#define ISP_REG_LCBQ_OFFSET0_CFG_4	0x410
#define ISP_REG_LCBQ_OFFSET0_CFG_5	0x414
#define ISP_REG_LCBQ_OFFSET0_CFG_6	0x418
#define ISP_REG_LCBQ_OFFSET0_CFG_7	0x41c
#define ISP_REG_LCBQ_OFFSET0_CFG_8	0x420
#define ISP_REG_LCBQ_OFFSET0_CFG_9	0x424
#define ISP_REG_LCBQ_OFFSET0_CFG_10	0x428
#define ISP_REG_LCBQ_OFFSET0_CFG_11	0x42c
#define ISP_REG_LCBQ_OFFSET0_CFG_12	0x430
#define ISP_REG_LCBQ_OFFSET1_CFG_0	0x434
#define ISP_REG_LCBQ_OFFSET1_CFG_1	0x438
#define ISP_REG_LCBQ_OFFSET1_CFG_2	0x43c
#define ISP_REG_LCBQ_OFFSET1_CFG_3	0x440
#define ISP_REG_LCBQ_OFFSET1_CFG_4	0x444
#define ISP_REG_LCBQ_OFFSET1_CFG_5	0x448
#define ISP_REG_LCBQ_OFFSET1_CFG_6	0x44c
#define ISP_REG_LCBQ_OFFSET1_CFG_7	0x450
#define ISP_REG_LCBQ_OFFSET1_CFG_8	0x454
#define ISP_REG_LCBQ_OFFSET1_CFG_9	0x458
#define ISP_REG_LCBQ_OFFSET1_CFG_10	0x45c
#define ISP_REG_LCBQ_OFFSET1_CFG_11	0x460
#define ISP_REG_LCBQ_OFFSET1_CFG_12	0x464
#define ISP_REG_LCBQ_OFFSET2_CFG_0	0x468
#define ISP_REG_LCBQ_OFFSET2_CFG_1	0x46c
#define ISP_REG_LCBQ_OFFSET2_CFG_2	0x470
#define ISP_REG_LCBQ_OFFSET2_CFG_3	0x474
#define ISP_REG_LCBQ_OFFSET2_CFG_4	0x478
#define ISP_REG_LCBQ_OFFSET2_CFG_5	0x47c
#define ISP_REG_LCBQ_OFFSET2_CFG_6	0x480
#define ISP_REG_LCBQ_OFFSET2_CFG_7	0x484
#define ISP_REG_LCBQ_OFFSET2_CFG_8	0x488
#define ISP_REG_LCBQ_OFFSET2_CFG_9	0x48c
#define ISP_REG_LCBQ_OFFSET2_CFG_10	0x490
#define ISP_REG_LCBQ_OFFSET2_CFG_11	0x494
#define ISP_REG_LCBQ_OFFSET2_CFG_12	0x498
#define ISP_REG_LCBQ_OFFSET3_CFG_0	0x49c
#define ISP_REG_LCBQ_OFFSET3_CFG_1	0x4a0
#define ISP_REG_LCBQ_OFFSET3_CFG_2	0x4a4
#define ISP_REG_LCBQ_OFFSET3_CFG_3	0x4a8
#define ISP_REG_LCBQ_OFFSET3_CFG_4	0x4ac
#define ISP_REG_LCBQ_OFFSET3_CFG_5	0x4b0
#define ISP_REG_LCBQ_OFFSET3_CFG_6	0x4b4
#define ISP_REG_LCBQ_OFFSET3_CFG_7	0x4b8
#define ISP_REG_LCBQ_OFFSET3_CFG_8	0x4bc
#define ISP_REG_LCBQ_OFFSET3_CFG_9	0x4c0
#define ISP_REG_LCBQ_OFFSET3_CFG_10	0x4c4
#define ISP_REG_LCBQ_OFFSET3_CFG_11	0x4c8
#define ISP_REG_LCBQ_OFFSET3_CFG_12	0x4cc

#define ISP_REG_SC_AWB_WS_CW0_CFG_0	0x4d0
#define ISP_REG_SC_AWB_WS_CW0_CFG_1	0x4d4
#define ISP_REG_SC_AWB_WS_CW1_CFG_0	0x4d8
#define ISP_REG_SC_AWB_WS_CW1_CFG_1	0x4dc
#define ISP_REG_SC_AWB_WS_CW2_CFG_0	0x4e0
#define ISP_REG_SC_AWB_WS_CW2_CFG_1	0x4e4
#define ISP_REG_SC_AWB_WS_CW3_CFG_0	0x4e8
#define ISP_REG_SC_AWB_WS_CW3_CFG_1	0x4ec
#define ISP_REG_SC_AWB_WS_CW4_CFG_0	0x4f0
#define ISP_REG_SC_AWB_WS_CW4_CFG_1	0x4f4
#define ISP_REG_SC_AWB_WS_CW5_CFG_0	0x4f8
#define ISP_REG_SC_AWB_WS_CW5_CFG_1	0x4fc
#define ISP_REG_SC_AWB_WS_CW6_CFG_0	0x500
#define ISP_REG_SC_AWB_WS_CW6_CFG_1	0x504
#define ISP_REG_SC_AWB_WS_CW7_CFG_0	0x508
#define ISP_REG_SC_AWB_WS_CW7_CFG_1	0x50c
#define ISP_REG_SC_AWB_WS_CW8_CFG_0	0x510
#define ISP_REG_SC_AWB_WS_CW8_CFG_1	0x514
#define ISP_REG_SC_AWB_WS_CW9_CFG_0	0x518
#define ISP_REG_SC_AWB_WS_CW9_CFG_1	0x51c
#define ISP_REG_SC_AWB_WS_CW10_CFG_0	0x520
#define ISP_REG_SC_AWB_WS_CW10_CFG_1	0x524
#define ISP_REG_SC_AWB_WS_CW11_CFG_0	0x528
#define ISP_REG_SC_AWB_WS_CW11_CFG_1	0x52c
#define ISP_REG_SC_AWB_WS_CW12_CFG_0	0x530
#define ISP_REG_SC_AWB_WS_CW12_CFG_1	0x534
#define AWB_WS_CW_W_7(n)	((n) << 28)
#define AWB_WS_CW_W_6(n)	((n) << 24)
#define AWB_WS_CW_W_5(n)	((n) << 20)
#define AWB_WS_CW_W_4(n)	((n) << 16)
#define AWB_WS_CW_W_3(n)	((n) << 12)
#define AWB_WS_CW_W_2(n)	((n) << 8)
#define AWB_WS_CW_W_1(n)	((n) << 4)
#define AWB_WS_CW_W_0(n)	((n) << 0)

#define ISP_REG_SC_AWB_WS_IWV_CFG_0	0x538
#define ISP_REG_SC_AWB_WS_IWV_CFG_1	0x53c
#define AWB_WS_IW_V_7(n)	((n) << 28)
#define AWB_WS_IW_V_6(n)	((n) << 24)
#define AWB_WS_IW_V_5(n)	((n) << 20)
#define AWB_WS_IW_V_4(n)	((n) << 16)
#define AWB_WS_IW_V_3(n)	((n) << 12)
#define AWB_WS_IW_V_2(n)	((n) << 8)
#define AWB_WS_IW_V_1(n)	((n) << 4)
#define AWB_WS_IW_V_0(n)	((n) << 0)

#define ISP_REG_SC_AWB_WS_IWS_CFG_0	0x540
#define ISP_REG_SC_AWB_WS_IWS_CFG_1	0x544
#define ISP_REG_SC_AWB_WS_IWS_CFG_2	0x548
#define ISP_REG_SC_AWB_WS_IWS_CFG_3	0x54c
#define AWB_WS_IW_S3(n)	((n) << 24)
#define AWB_WS_IW_S2(n)	((n) << 16)
#define AWB_WS_IW_S1(n)	((n) << 8)
#define AWB_WS_IW_S0(n)	((n) << 0)

#define ISP_REG_SC_AWB_WS_CFG_0	0x5d0
#define AWB_WS_GRU(n)	((n) << 24)
#define AWB_WS_GRL(n)	((n) << 16)
#define AWB_WS_RU(n)	((n) << 8)
#define AWB_WS_RL(n)	((n) << 0)
#define ISP_REG_SC_AWB_WS_CFG_1	0x5d4
#define AWB_WS_BU(n)	((n) << 24)
#define AWB_WS_BL(n)	((n) << 16)
#define AWB_WS_GBU(n)	((n) << 8)
#define AWB_WS_GBL(n)	((n) << 0)

#define ISP_REG_CBAR0	0x600
#define ISP_REG_CBAR1	0x604
#define ISP_REG_CBAR2	0x608
#define ISP_REG_CBAR3	0x60c
#define ISP_REG_CBAR4	0x610
#define ISP_REG_CBAR5	0x614
#define ISP_REG_CBAR6	0x618
#define ISP_REG_CBAR7	0x61c
#define ISP_REG_CBAR8	0x620
#define ISP_REG_CBAR9	0x624
#define ISP_REG_CBAR10	0x628
#define ISP_REG_CBAR11	0x62c
#define ISP_REG_CBAR12	0x630
#define ISP_REG_CBAR13	0x634
#define ISP_REG_CBAR14	0x638
#define ISP_REG_CBAR15	0x63c
#define ISP_REG_CBAR16	0x640
#define ISP_REG_CBAR17	0x644
#define ISP_REG_CBAR18	0x648
#define ISP_REG_CBAR19	0x64c
#define ISP_REG_CBAR20	0x650
#define CBAR_PAR_H(n)	((n) << 16)
#define CBAR_PAR_L(n)	((n) << 0)

#define ISP_REG_ISP_CTRL_0	0xa00
#define ISPC_VSFWINT	BIT(26)
#define ISPC_VSINT	BIT(25)
#define ISPC_INTS	BIT(24)
#define ISPC_ENUO	BIT(20)
#define ISPC_ENLS	BIT(17)
#define ISPC_ENSS1	BIT(12)
#define ISPC_ENSS0	BIT(11)
#define ISPC_RST	BIT(1)
#define ISPC_EN	BIT(0)
#define ISPC_RST_MASK	BIT(1)

#define ISP_REG_ISP_CTRL_1	0xa08
#define CTRL_SAT(n)	((n) << 28)
#define CTRL_DBC	BIT(22)
#define CTRL_CTC	BIT(21)
#define CTRL_YHIST	BIT(20)
#define CTRL_YCURVE	BIT(19)
#define CTRL_CTM	BIT(18)
#define CTRL_BIYUV	BIT(17)
#define CTRL_SCE	BIT(8)
#define CTRL_EE	BIT(7)
#define CTRL_CCE	BIT(5)
#define CTRL_RGE	BIT(4)
#define CTRL_CME	BIT(3)
#define CTRL_AE	BIT(2)
#define CTRL_CE	BIT(1)
#define CTRL_SAT_MASK	GENMASK(31, 28)

#define ISP_REG_PIPELINE_XY_SIZE	0xa0c
#define H_ACT_CAP(n)	((n) << 16)
#define W_ACT_CAP(n)	((n) << 0)

#define ISP_REG_ICTC	0xa10
#define MAXGT(n)	((n) << 16)
#define MINGT(n)	((n) << 0)

#define ISP_REG_IDBC	0xa14
#define BADGT(n)	((n) << 16)
#define BADXT(n)	((n) << 0)

#define ISP_REG_ICFAM	0xa1c
#define CROSS_COV(n)	((n) << 4)
#define HV_W(n)	((n) << 0)

#define ISP_REG_ISAT0	0xa30
#define CMMD(n)	((n) << 16)
#define CMAB(n)	((n) << 0)

#define ISP_REG_ISAT1	0xa34
#define CMD(n)	((n) << 16)
#define CMB(n)	((n) << 0)

#define ISP_REG_ISAT2	0xa38
#define VOFF(n)	((n) << 16)
#define UOFF(n)	((n) << 0)

#define ISP_REG_ISAT3	0xa3c
#define SIN(n)	((n) << 16)
#define COS(n)	((n) << 0)

#define ISP_REG_ISAT4	0xa40
#define CMSF(n)	((n) << 0)

#define ISP_REG_IESHD_ADDR	0xa50
#define SHAD_UP_M	BIT(1)
#define SHAD_UP_EN	BIT(0)

#define ISP_REG_IYADJ0	0xa54
#define YOIR(n)	((n) << 16)
#define YIMIN(n)	((n) << 0)

#define ISP_REG_IYADJ1	0xa58
#define YOMAX(n)	((n) << 16)
#define YOMIN(n)	((n) << 0)

#define ISP_REG_Y_PLANE_START_ADDR	0xa80
#define ISP_REG_UV_PLANE_START_ADDR	0xa84

#define ISP_REG_STRIDE	0xa88
#define IMG_STR(n)	((n) << 0)

#define ISP_REG_PIXEL_COORDINATE_GEN	0xa8c
#define OUT_SCANH(n)	((n) << 4)

#define ISP_REG_UOAXI	0xa90
#define OUT_AXI_W_ID(n)	((n) << 8)

#define ISP_REG_SS0AY	0xa94
#define ISP_REG_SS0AUV	0xa98
#define ISP_REG_SS0S	0xa9c
#define SD_IMG(n)	((n) << 0)

#define ISP_REG_SS0HF	0xaa0
#define H_SF_SCAL(n)	((n) << 16)
#define H_SM_SCAL(n)	((n) << 0)

#define ISP_REG_SS0VF	0xaa4
#define V_SF_SCAL(n)	((n) << 16)
#define V_SM_SCAL(n)	((n) << 0)

#define ISP_REG_SS0IW	0xaa8
#define W_OUT(n)	((n) << 16)
#define H_OUT(n)	((n) << 0)

#define ISP_REG_SS1AY	0xaac
#define Y_PLANE_SAD(n)	((n) << 0)

#define ISP_REG_SS1AUV	0xab0
#define UV_PLANE_SAD(n)	((n) << 0)

#define ISP_REG_SS1S	0xab4
#define OUT_IMG_STR(n)	((n) << 0)

#define ISP_REG_SS1HF	0xab8
#define H_SCAL_FACTOR(n)	((n) << 16)
#define H_SCAL_MODE(n)	((n) << 0)

#define ISP_REG_SS1VF	0xabc
#define V_SCAL_FACTOR(n)	((n) << 16)
#define V_SCAL_MODE(n)	((n) << 0)

#define ISP_REG_SS1IW	0xac0
#define W_OUT_IMG(n)	((n) << 16)
#define H_OUT_IMG(n)	((n) << 0)

#define ISP_REG_SSAXI	0xac4
#define SS1WID(n)	((n) << 8)
#define SS0WID(n)	((n) << 0)

#define ISP_REG_YHIST_CFG_4	0xcd8

#define ISP_REG_ITIIWSR	0xb20
#define ITI_HSIZE(n)	((n) << 16)
#define ITI_WSIZE(n)	((n) << 0)

#define ISP_REG_ITIDWLSR	0xb24
#define ITI_WSTRIDE(n)	((n) << 0)

#define ISP_REG_ITIDWYSAR	0xb28
#define ISP_REG_ITIDWUSAR	0xb2C
#define ISP_REG_ITIDRYSAR	0xb30
#define ISP_REG_ITIDRUSAR	0xb34

#define ISP_REG_ITIPDFR	0xb38
#define ITI_PACKAGE_FMT(n)	((n) << 0)

#define ISP_REG_ITIDRLSR	0xb3C
#define ITI_STRIDE_L(n)	((n) << 0)

#define ISP_REG_ITIBSR	0xb40

#define ISP_REG_ITIAIR	0xb44
#define ITI_UVRID(n)	((n) << 24)
#define ITI_YRID(n)	((n) << 16)
#define ITI_UVWID(n)	((n) << 8)
#define ITI_YWID(n)	((n) << 0)

#define ISP_REG_ITIDPSR	0xb48
#define ITI_W_INDEX(n)	((n) << 8)
#define ITI_R_INDEX(n)	((n) << 0)

#define ISP_REG_DNYUV_YSWR0	0xc00
#define ISP_REG_DNYUV_YSWR1	0xc04
#define ISP_REG_DNYUV_CSWR0	0xc08
#define ISP_REG_DNYUV_CSWR1	0xc0c
#define YUVSW5(n)	((n) << 20)
#define YUVSW4(n)	((n) << 16)
#define YUVSW3(n)	((n) << 12)
#define YUVSW2(n)	((n) << 8)
#define YUVSW1(n)	((n) << 4)
#define YUVSW0(n)	((n) << 0)

#define ISP_REG_DNYUV_YDR0	0xc10
#define ISP_REG_DNYUV_YDR1	0xc14
#define ISP_REG_DNYUV_YDR2	0xc18
#define ISP_REG_DNYUV_CDR0	0xc1c
#define ISP_REG_DNYUV_CDR1	0xc20
#define ISP_REG_DNYUV_CDR2	0xc24
#define CURVE_D_H(n)	((n) << 16)
#define CURVE_D_L(n)	((n) << 0)

#define ISP_REG_ICAMD_0	0xc40
#define DNRM(n)	((n) << 16)
#define ISP_REG_ICAMD_1	0xc44
#define ISP_REG_ICAMD_2	0xc48
#define ISP_REG_ICAMD_3	0xc4c
#define ISP_REG_ICAMD_4	0xc50
#define ISP_REG_ICAMD_5	0xc54
#define ISP_REG_ICAMD_6	0xc58
#define ISP_REG_ICAMD_7	0xc5c
#define ISP_REG_ICAMD_8	0xc60
#define ISP_REG_ICAMD_9	0xc64
#define ISP_REG_ICAMD_10	0xc68
#define ISP_REG_ICAMD_11	0xc6c
#define ISP_REG_ICAMD_12	0xc70
#define ISP_REG_ICAMD_13	0xc74
#define ISP_REG_ICAMD_14	0xc78
#define ISP_REG_ICAMD_15	0xc7c
#define ISP_REG_ICAMD_16	0xc80
#define ISP_REG_ICAMD_17	0xc84
#define ISP_REG_ICAMD_18	0xc88
#define ISP_REG_ICAMD_19	0xc8c
#define ISP_REG_ICAMD_20	0xc90
#define ISP_REG_ICAMD_21	0xc94
#define ISP_REG_ICAMD_22	0xc98
#define ISP_REG_ICAMD_23	0xc9c
#define ISP_REG_ICAMD_24	0xca0
#define ISP_REG_ICAMD_25	0xca4
#define CCM_M_DAT(n)	((n) << 0)

#define ISP_REG_YHIST_CFG_0	0xcc8
#define YH_VSTART(n)	((n) << 16)
#define YH_HSTART(n)	((n) << 0)
#define ISP_REG_YHIST_CFG_1	0xccc
#define YH_HEIGHT(n)	((n) << 16)
#define YH_WIDTH(n)	((n) << 0)
#define ISP_REG_YHIST_CFG_2	0xcd0
#define YH_DEC_ETH(n)	((n) << 16)
#define YH_DEC_ETW(n)	((n) << 0)
#define ISP_REG_YHIST_CFG_3	0xcd4
#define YH_MUX(n)	((n) << 0)

#define ISP_REG_IGRVAL_0	0xe00
#define ISP_REG_IGRVAL_1	0xe04
#define ISP_REG_IGRVAL_2	0xe08
#define ISP_REG_IGRVAL_3	0xe0c
#define ISP_REG_IGRVAL_4	0xe10
#define ISP_REG_IGRVAL_5	0xe14
#define ISP_REG_IGRVAL_6	0xe18
#define ISP_REG_IGRVAL_7	0xe1c
#define ISP_REG_IGRVAL_8	0xe20
#define ISP_REG_IGRVAL_9	0xe24
#define ISP_REG_IGRVAL_10	0xe28
#define ISP_REG_IGRVAL_11	0xe2c
#define ISP_REG_IGRVAL_12	0xe30
#define ISP_REG_IGRVAL_13	0xe34
#define ISP_REG_IGRVAL_14	0xe38
#define SGVAL(n)	((n) << 16)
#define GVAL(n)	((n) << 0)

#define ISP_REG_ICCONV_0	0xe40
#define ISP_REG_ICCONV_1	0xe44
#define ISP_REG_ICCONV_2	0xe48
#define ISP_REG_ICCONV_3	0xe4c
#define ISP_REG_ICCONV_4	0xe50
#define ISP_REG_ICCONV_5	0xe54
#define ISP_REG_ICCONV_6	0xe58
#define ISP_REG_ICCONV_7	0xe5c
#define ISP_REG_ICCONV_8	0xe60
#define CSC_M(n)	((n) << 0)

#define ISP_REG_ISHRP1_0	0xe80
#define ISP_REG_ISHRP1_1	0xe84
#define ISP_REG_ISHRP1_2	0xe88
#define ISP_REG_ISHRP1_3	0xe8c
#define ISP_REG_ISHRP1_4	0xe90
#define ISP_REG_ISHRP1_5	0xe94
#define ISP_REG_ISHRP1_6	0xe98
#define ISP_REG_ISHRP1_7	0xe9c
#define ISP_REG_ISHRP1_8	0xea0
#define ISP_REG_ISHRP1_9	0xea4
#define ISP_REG_ISHRP1_10	0xea8
#define ISP_REG_ISHRP1_11	0xeac
#define ISP_REG_ISHRP1_12	0xeb0
#define ISP_REG_ISHRP1_13	0xeb4
#define ISP_REG_ISHRP1_14	0xeb8
#define DIFF(n)	((n) << 16)
#define SHRP_W(n)	((n) << 8)

#define ISP_REG_ISHRP2_0	0xebc
#define ISP_REG_ISHRP2_1	0xec0
#define ISP_REG_ISHRP2_2	0xec4
#define ISP_REG_ISHRP2_3	0xec8
#define ISP_REG_ISHRP2_4	0xecc
#define ISP_REG_ISHRP2_5	0xed0
#define F_AMP(n)	((n) << 24)
#define S_AMP(n)	((n) << 0)

#define ISP_REG_ISHRP3	0xed4
#define PDIRF(n)	((n) << 28)
#define NDIRF(n)	((n) << 24)
#define WSUM(n)	((n) << 0)

#define ISP_REG_IUVS1	0xed8
#define UVDIFF2(n)	((n) << 16)
#define UVDIFF1(n)	((n) << 0)

#define ISP_REG_IUVS2	0xedc
#define UVF(n)	((n) << 24)
#define UVSLOPE(n)	((n) << 0)

#define ISP_REG_IUVCKS1	0xee0
#define UVCKDIFF2(n)	((n) << 16)
#define UVCKDIFF1(n)	((n) << 0)

#define ISP_REG_IUVCKS2	0xee4
#define UVCKSLOPE(n)	((n) << 0)

#define ISP_REG_ISHRPET	0xee8
#define TH(n)	((n) << 8)
#define EN(n)	((n) << 0)

#define ISP_REG_YCURVE_0	0xf00
#define ISP_REG_YCURVE_1	0xf04
#define ISP_REG_YCURVE_2	0xf08
#define ISP_REG_YCURVE_3	0xf0c
#define ISP_REG_YCURVE_4	0xf10
#define ISP_REG_YCURVE_5	0xf14
#define ISP_REG_YCURVE_6	0xf18
#define ISP_REG_YCURVE_7	0xf1c
#define ISP_REG_YCURVE_8	0xf20
#define ISP_REG_YCURVE_9	0xf24
#define ISP_REG_YCURVE_10	0xf28
#define ISP_REG_YCURVE_11	0xf2c
#define ISP_REG_YCURVE_12	0xf30
#define ISP_REG_YCURVE_13	0xf34
#define ISP_REG_YCURVE_14	0xf38
#define ISP_REG_YCURVE_15	0xf3c
#define ISP_REG_YCURVE_16	0xf40
#define ISP_REG_YCURVE_17	0xf44
#define ISP_REG_YCURVE_18	0xf48
#define ISP_REG_YCURVE_19	0xf4c
#define ISP_REG_YCURVE_20	0xf50
#define ISP_REG_YCURVE_21	0xf54
#define ISP_REG_YCURVE_22	0xf58
#define ISP_REG_YCURVE_23	0xf5c
#define ISP_REG_YCURVE_24	0xf60
#define ISP_REG_YCURVE_25	0xf64
#define ISP_REG_YCURVE_26	0xf68
#define ISP_REG_YCURVE_27	0xf6c
#define ISP_REG_YCURVE_28	0xf70
#define ISP_REG_YCURVE_29	0xf74
#define ISP_REG_YCURVE_30	0xf78
#define ISP_REG_YCURVE_31	0xf7c
#define ISP_REG_YCURVE_32	0xf80
#define ISP_REG_YCURVE_33	0xf84
#define ISP_REG_YCURVE_34	0xf88
#define ISP_REG_YCURVE_35	0xf8c
#define ISP_REG_YCURVE_36	0xf90
#define ISP_REG_YCURVE_37	0xf94
#define ISP_REG_YCURVE_38	0xf98
#define ISP_REG_YCURVE_39	0xf9c
#define ISP_REG_YCURVE_40	0xfa0
#define ISP_REG_YCURVE_41	0xfa4
#define ISP_REG_YCURVE_42	0xfa8
#define ISP_REG_YCURVE_43	0xfac
#define ISP_REG_YCURVE_44	0xfb0
#define ISP_REG_YCURVE_45	0xfb4
#define ISP_REG_YCURVE_46	0xfb8
#define ISP_REG_YCURVE_47	0xfbc
#define ISP_REG_YCURVE_48	0xfc0
#define ISP_REG_YCURVE_49	0xfc4
#define ISP_REG_YCURVE_50	0xfc8
#define ISP_REG_YCURVE_51	0xfcc
#define ISP_REG_YCURVE_52	0xfd0
#define ISP_REG_YCURVE_53	0xfd4
#define ISP_REG_YCURVE_54	0xfd8
#define ISP_REG_YCURVE_55	0xfdc
#define ISP_REG_YCURVE_56	0xfe0
#define ISP_REG_YCURVE_57	0xfe4
#define ISP_REG_YCURVE_58	0xfe8
#define ISP_REG_YCURVE_59	0xfec
#define ISP_REG_YCURVE_60	0xff0
#define ISP_REG_YCURVE_61	0xff4
#define ISP_REG_YCURVE_62	0xff8
#define ISP_REG_YCURVE_63	0xffc
#define L_PARAM(n)	((n) << 0)

/* The output line of ISP */
enum isp_line_id {
	STF_ISP_LINE_INVALID = -1,
	STF_ISP_LINE_SRC = 1,
	STF_ISP_LINE_MAX = STF_ISP_LINE_SRC
};

/* pad id for media framework */
enum isp_pad_id {
	STF_ISP_PAD_SINK = 0,
	STF_ISP_PAD_SRC,
	STF_ISP_PAD_MAX
};

enum {
	EN_INT_NONE                 = 0,
	EN_INT_ISP_DONE             = (0x1 << 24),
	EN_INT_CSI_DONE             = (0x1 << 25),
	EN_INT_SC_DONE              = (0x1 << 26),
	EN_INT_LINE_INT             = (0x1 << 27),
	EN_INT_ALL                  = (0xF << 24),
};

enum {
	INTERFACE_DVP = 0,
	INTERFACE_CSI,
};

struct isp_format {
	u32 code;
	u8 bpp;
};

struct isp_format_table {
	const struct isp_format *fmts;
	int nfmts;
};

struct regval_t {
	u32 addr;
	u32 val;
	u32 delay_ms;
};

struct reg_table {
	const struct regval_t *regval;
	int regval_num;
};

struct isp_stream_format {
	struct v4l2_rect rect;
	u32 bpp;
};

struct isp_setfile {
	struct reg_table settings;
	const u8 *data;
	unsigned int size;
	unsigned int state;
};

enum {
	ISP_CROP = 0,
	ISP_COMPOSE,
	ISP_RECT_MAX
};

struct stf_isp_dev {
	enum stf_subdev_type sdev_type;  /* This member must be first */
	struct stfcamss *stfcamss;
	struct v4l2_subdev subdev;
	struct media_pad pads[STF_ISP_PAD_MAX];
	struct v4l2_mbus_framefmt fmt[STF_ISP_PAD_MAX];
	struct isp_stream_format rect[ISP_RECT_MAX];
	const struct isp_format_table *formats;
	unsigned int nformats;
	struct mutex power_lock;	/* serialize power control*/
	int power_count;
	struct mutex stream_lock;	/* serialize stream control */
	int stream_count;
	atomic_t shadow_count;

	struct mutex setfile_lock;	/* protects setting files */
	struct isp_setfile setfile;
};

int stf_isp_clk_enable(struct stf_isp_dev *isp_dev);
int stf_isp_clk_disable(struct stf_isp_dev *isp_dev);
int stf_isp_reset(struct stf_isp_dev *isp_dev);
int stf_isp_config_set(struct stf_isp_dev *isp_dev);
int stf_isp_set_format(struct stf_isp_dev *isp_dev,
		       struct isp_stream_format *crop_array,
		       u32 mcode, int type);
int stf_isp_stream_set(struct stf_isp_dev *isp_dev, int on);
int stf_isp_subdev_init(struct stfcamss *stfcamss);
int stf_isp_register(struct stf_isp_dev *isp_dev, struct v4l2_device *v4l2_dev);
int stf_isp_unregister(struct stf_isp_dev *isp_dev);

#endif /* STF_ISP_H */
