/*
 * Copyright (c) 2014 Steve Ward
 * Copyright (c) 2018 Jens Kerrinnes
 * LICENCE: MIT License (look at /LICENCE.md)
 */

#ifndef DMA_H
#define DMA_H

/* DMA Configuration for FBC_to_ram_DMA */
#define FBC_to_ram_DMA_BYTES_PER_BURST 2
#define FBC_to_ram_DMA_REQUEST_PER_BURST 1
#define FBC_to_ram_DMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define FBC_to_ram_DMA_DST_BASE (CYDEV_SRAM_BASE)

/* DMA Configuration for ram_to_filter_DMA */
#define ram_to_filter_DMA_BYTES_PER_BURST 2
#define ram_to_filter_DMA_REQUEST_PER_BURST 1
#define ram_to_filter_DMA_SRC_BASE (CYDEV_SRAM_BASE)
#define ram_to_filter_DMA_DST_BASE (CYDEV_PERIPH_BASE)

/* DMA Configuration for filter_to_fram_DMA */
#define filter_to_fram_DMA_BYTES_PER_BURST 2
#define filter_to_fram_DMA_REQUEST_PER_BURST 1
#define filter_to_fram_DMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define filter_to_fram_DMA_DST_BASE (CYDEV_SRAM_BASE)

/* DMA Configuration for fram_to_PWMA_DMA */
#define fram_to_PWMA_DMA_BYTES_PER_BURST 2
#define fram_to_PWMA_DMA_REQUEST_PER_BURST 1
#define fram_to_PWMA_DMA_SRC_BASE (CYDEV_SRAM_BASE)
#define fram_to_PWMA_DMA_DST_BASE (CYDEV_PERIPH_BASE)

/* DMA Configuration for PWMA_init_DMA */
#define PWMA_init_DMA_BYTES_PER_BURST 8
#define PWMA_init_DMA_REQUEST_PER_BURST 1
#define PWMA_init_DMA_SRC_BASE (CYDEV_SRAM_BASE)
#define PWMA_init_DMA_DST_BASE (CYDEV_PERIPH_BASE)

/* DMA Configuration for PSBINIT_DMA */
#define PSBINIT_DMA_BYTES_PER_BURST 4
#define PSBINIT_DMA_REQUEST_PER_BURST 1
#define PSBINIT_DMA_SRC_BASE (CYDEV_SRAM_BASE)
#define PSBINIT_DMA_DST_BASE (CYDEV_PERIPH_BASE)

/* DMA Configuration for PWMB_PSB_DMA */
#define PWMB_PSB_DMA_BYTES_PER_BURST 2
#define PWMB_PSB_DMA_REQUEST_PER_BURST 1
#define PWMB_PSB_DMA_SRC_BASE (CYDEV_SRAM_BASE)
#define PWMB_PSB_DMA_DST_BASE (CYDEV_PERIPH_BASE)

/* DMA Configuration for QCW_CL_DMA */
#define QCW_CL_DMA_BYTES_PER_BURST 2
#define QCW_CL_DMA_REQUEST_PER_BURST 1
#define QCW_CL_DMA_SRC_BASE (CYDEV_SRAM_BASE)
#define QCW_CL_DMA_DST_BASE (CYDEV_PERIPH_BASE)

/* DMA Configuration for TR1_CL_DMA */
#define TR1_CL_DMA_BYTES_PER_BURST 2
#define TR1_CL_DMA_REQUEST_PER_BURST 1
#define TR1_CL_DMA_SRC_BASE (CYDEV_SRAM_BASE)
#define TR1_CL_DMA_DST_BASE (CYDEV_PERIPH_BASE)


void initialize_DMA (void);
#endif 