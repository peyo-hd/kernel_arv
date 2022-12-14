/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright (C) 2021 Emil Renner Berthing <kernel@esmil.dk>
 */

#ifndef __DT_BINDINGS_CLOCK_STARFIVE_JH7100_AUDIO_H__
#define __DT_BINDINGS_CLOCK_STARFIVE_JH7100_AUDIO_H__

#define JH7100_AUDCLK_ADC_MCLK		0
#define JH7100_AUDCLK_I2S1_MCLK		1
#define JH7100_AUDCLK_I2SADC_APB	2
#define JH7100_AUDCLK_I2SADC_BCLK	3
#define JH7100_AUDCLK_I2SADC_BCLK_N	4
#define JH7100_AUDCLK_I2SADC_LRCLK	5
#define JH7100_AUDCLK_PDM_APB		6
#define JH7100_AUDCLK_PDM_MCLK		7
#define JH7100_AUDCLK_I2SVAD_APB	8
#define JH7100_AUDCLK_SPDIF		9
#define JH7100_AUDCLK_SPDIF_APB		10
#define JH7100_AUDCLK_PWMDAC_APB	11
#define JH7100_AUDCLK_DAC_MCLK		12
#define JH7100_AUDCLK_I2SDAC_APB	13
#define JH7100_AUDCLK_I2SDAC_BCLK	14
#define JH7100_AUDCLK_I2SDAC_BCLK_N	15
#define JH7100_AUDCLK_I2SDAC_LRCLK	16
#define JH7100_AUDCLK_I2S1_APB		17
#define JH7100_AUDCLK_I2S1_BCLK		18
#define JH7100_AUDCLK_I2S1_BCLK_N	19
#define JH7100_AUDCLK_I2S1_LRCLK	20
#define JH7100_AUDCLK_I2SDAC16K_APB	21
#define JH7100_AUDCLK_APB0_BUS		22
#define JH7100_AUDCLK_DMA1P_AHB		23
#define JH7100_AUDCLK_USB_APB		24
#define JH7100_AUDCLK_USB_LPM		25
#define JH7100_AUDCLK_USB_STB		26
#define JH7100_AUDCLK_APB_EN		27
#define JH7100_AUDCLK_VAD_MEM		28

#define JH7100_AUDCLK_I2SADC_BCLK_IOPAD 	29
#define JH7100_AUDCLK_I2SADC_LRCLK_IOPAD 	30
#define JH7100_AUDCLK_I2SDAC_BCLK_IOPAD 	31
#define JH7100_AUDCLK_I2SDAC_LRCLK_IOPAD 	32
#define JH7100_AUDCLK_EXT_CODEC 	33

#define JH7100_AUDCLK_END		34

#endif /* __DT_BINDINGS_CLOCK_STARFIVE_JH7100_AUDIO_H__ */
