/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Modifications Copyright 2022 NXP
 */
#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

/*
 * include/linux/spinlock.h - generic spinlock/rwlock declarations
 *
 * here's the role of the various spinlock/rwlock related include files:
 *
 * on SMP builds:
 *
 *  asm/spinlock_types.h: contains the arch_spinlock_t/arch_rwlock_t and the
 *                        initializers
 *
 *  linux/spinlock_types.h:
 *                        defines the generic type and initializers
 *
 *  asm/spinlock.h:       contains the arch_spin_*()/etc. lowlevel
 *                        implementations, mostly inline assembly code
 *
 *   (also included on UP-debug builds:)
 *
 *  linux/spinlock_api_smp.h:
 *                        contains the prototypes for the _spin_*() APIs.
 *
 *  linux/spinlock.h:     builds the final spin_*() APIs.
 *
 * on UP builds:
 *
 *  linux/spinlock_type_up.h:
 *                        contains the generic, simplified UP spinlock type.
 *                        (which is an empty structure on non-debug builds)
 *
 *  linux/spinlock_types.h:
 *                        defines the generic type and initializers
 *
 *  linux/spinlock_up.h:
 *                        contains the arch_spin_*()/etc. version of UP
 *                        builds. (which are NOPs on non-debug, non-preempt
 *                        builds)
 *
 *   (included on UP-non-debug builds:)
 *
 *  linux/spinlock_api_up.h:
 *                        builds the _spin_*() APIs.
 *
 *  linux/spinlock.h:     builds the final spin_*() APIs.
 */

#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/lockdep.h>

/*
 * Pull the arch_spinlock_t and arch_rwlock_t definitions:
 */
#include <linux/spinlock_types.h>

/*
* We implement spinlock API as NOP because there is no straightforward
* Windows spinlock API available. Windows uses different mechanism to synchronize
* driver thread with the interrupt thread.
* On Windows side this should be safe, because the next page flipping/mode setting
* request will not come in until the current one is completed.
* This is not meant to be a generic emulation, but rather for our limited scenario.
*/

# define raw_spin_lock_init(lock)

inline void raw_spin_lock_irqsave(raw_spinlock_t *lock,
	unsigned char *flags)
{
}

inline void raw_spin_unlock_irqrestore(raw_spinlock_t *lock,
	unsigned char flags)
{
}

# define spin_lock_init(lock)

static __always_inline void spin_lock(spinlock_t *lock)
{

}

inline void spin_lock_irq(spinlock_t *lock)
{
}

#define spin_lock_irqsave(lock, flags)	\
	flags = 0

static __always_inline void spin_unlock(spinlock_t *lock)
{
}

inline void spin_unlock_irq(spinlock_t *lock)
{
}

inline void spin_unlock_irqrestore(spinlock_t *lock,
	unsigned long flags)
{
}

#endif /* __LINUX_SPINLOCK_H */
