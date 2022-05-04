/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: start for riscv64
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-01     JasonHu           Init
 */

#include <sbi.h>

NX_IMPORT NX_Addr __NX_BssStart;
NX_IMPORT NX_Addr __NX_BssEnd;

NX_IMPORT int NX_Main(NX_UArch coreId);
NX_Size __firstBootMagic = 0x5a5a;

int __GetBootHartid(int a0)
{
#if defined(CONFIG_NX_PLATFORM_HIFIVE_UNMACHED)
    int i;
    for (i = 0; i < 5; i++)
    {
        if (sbi_hsm_hart_status(i) == SBI_HSM_STATUS_STARTED)
        {
            return i;
        }
    }
#endif
    return a0;
}

void NX_HalClearBSS(void)
{
    NX_UArch *dst;

    dst = &__NX_BssStart;
    while (dst < &__NX_BssEnd)
    {
        *dst++ = 0x00UL;
    }
}

void __NX_EarlyMain(NX_UArch coreId)
{
    if (__firstBootMagic == 0x5a5a)
    {
        NX_HalClearBSS();
        __firstBootMagic = 0;
    }
    NX_Main(coreId);
}
