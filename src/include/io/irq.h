/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: IO Interrupt ReQuest 
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-11-28     JasonHu           Init
 */

#ifndef __IO_IRQ__
#define __IO_IRQ__

#include <xbook.h>
#include <utils/list.h>
#include <xbook/atomic.h>

#ifdef CONFIG_NX_IRQ_NAME_LEN
#define NX_IRQ_NAME_LEN CONFIG_NX_IRQ_NAME_LEN
#else
#define NX_IRQ_NAME_LEN 48
#endif

#ifndef CONFIG_NX_NR_IRQS
#error "Please config NX_NR_IRQS"
#else
#define NX_NR_IRQS CONFIG_NX_NR_IRQS
#endif

#define NX_IRQ_FLAG_REENTER    0x01    /* handle irq allow reenter */
#define NX_IRQ_FLAG_SHARED     0x02    /* irq was shared by more device */

typedef NX_U32 NX_IRQ_Number;
typedef NX_Error (*NX_IRQ_Handler)(NX_IRQ_Number, void *);

struct NX_IRQ_Controller
{
    NX_Error (*unmask)(NX_IRQ_Number irqno);
    NX_Error (*mask)(NX_IRQ_Number irqno);
    NX_Error (*ack)(NX_IRQ_Number irqno);
    
    void (*enable)(void);
    void (*disable)(void);
    NX_UArch (*saveLevel)(void);
    void (*restoreLevel)(NX_UArch level);
};
typedef struct NX_IRQ_Controller NX_IRQ_Controller;

struct NX_IRQ_Action
{
    NX_List list;
    NX_IRQ_Handler handler;
    void *data;
    NX_U32 flags;
    char name[NX_IRQ_NAME_LEN];
};
typedef struct NX_IRQ_Action NX_IRQ_Action;

struct NX_IRQ_Node
{
    NX_IRQ_Controller *controller;
    NX_List actionList;
    NX_U32 flags;
    NX_Atomic reference;   /* irq reference */
};
typedef struct NX_IRQ_Node NX_IRQ_Node;

NX_INTERFACE NX_IMPORT NX_IRQ_Controller NX_IRQ_ControllerInterface;

NX_Error NX_IRQ_Bind(NX_IRQ_Number irqno,
                         NX_IRQ_Handler handler,
                         void *data,
                         char *name,
                         NX_U32 flags);
                         
NX_Error NX_IRQ_Unbind(NX_IRQ_Number irqno, void *data);

NX_Error NX_IRQ_Unmask(NX_IRQ_Number irqno);
NX_Error NX_IRQ_Mask(NX_IRQ_Number irqno);

NX_Error NX_IRQ_Handle(NX_IRQ_Number irqno);

#define NX_IRQ_Enable()            NX_IRQ_ControllerInterface.enable()
#define NX_IRQ_Disable()           NX_IRQ_ControllerInterface.disable()
#define NX_IRQ_SaveLevel()         NX_IRQ_ControllerInterface.saveLevel()
#define NX_IRQ_RestoreLevel(level) NX_IRQ_ControllerInterface.restoreLevel(level)

void NX_IRQ_Init(void);

#endif  /* __IO_IRQ__ */
