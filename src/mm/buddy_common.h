/**
 * Copyright (c) 2018-2022, BookOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: Buddy system common
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2021-10-17     JasonHu           Update code style
 */

#define NX_LOG_LEVEL NX_LOG_INFO
#define NX_LOG_NAME "Buddy"
#include <utils/log.h>
#include <xbook/debug.h>
#include <utils/bitops.h>
#include <xbook.h>

#define BUDDY_ASSERT(x, msg) NX_ASSERT((x) && (msg))

/**
 * find first bit set
 */
NX_INLINE int BuddyFFS(NX_U32 word)
{
    return NX_FFS(word) - 1;
}

/**
 * find last bit set
 */
NX_INLINE int BuddyFLS(NX_U32 word)
{
    return NX_FLS(word) - 1;
}

/* Possibly 64-bit version of BuddyFLS. */
#if defined(CONFIG_NX_CPU_64BITS)
NX_PRIVATE int BuddyFlsSizet(NX_Size size)
{
    int high = (int)(size >> 32);
    int bits = 0;
    if (high)
    {
        bits = 32 + BuddyFLS(high);
    }
    else
    {
        bits = BuddyFLS((int)size & 0xffffffff);
    }
    return bits;
}

NX_PRIVATE int BuddyFfsSizet(NX_Size size)
{
    int low = (int)(size);
    int bits = 0;
    if (low)
    {
        bits = BuddyFFS(low);
    }
    else
    {
        bits = 32 + BuddyFFS((int)(size >> 32));
    }
    return bits;
}
#else
#define BuddyFlsSizet BuddyFLS
#define BuddyFfsSizet BuddyFFS
#endif

NX_PRIVATE NX_Size BuddyAlignUp(NX_Size x, NX_Size align)
{
    BUDDY_ASSERT(0 == (align & (align - 1)), "must align to a power of two");
    return (x + (align - 1)) & ~(align - 1);
}

NX_USED NX_PRIVATE NX_Size BuddyAlignDown(NX_Size x, NX_Size align)
{
    BUDDY_ASSERT(0 == (align & (align - 1)), "must align to a power of two");
    return x - (x & (align - 1));
}

NX_PRIVATE void *BuddyAlignPtr(const void *ptr, NX_Size align)
{
    NX_ASSERT(ptr && align);

    const NX_PtrDiff aligned =
        (NX_TYPE_CAST(NX_PtrDiff, ptr) + (align - 1)) & ~(align - 1);
    BUDDY_ASSERT(0 == (align & (align - 1)), "must align to a power of two");
    return NX_TYPE_CAST(void*, aligned);
}
