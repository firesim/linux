/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_JUMP_LABEL_PATCH_LOG_H
#define _LINUX_JUMP_LABEL_PATCH_LOG_H

#include <linux/types.h>
#include <linux/jump_label.h>

struct jl_entry {
	u64 addr;            // site address
	u32 new_insn;
};

#ifdef CONFIG_JUMP_LABEL_PATCH_LOG_SNAPSHOT
void jl_snap_append(const struct jl_entry *e);
void jl_snap_publish_debugfs(void);   // call later (late init)
#else
static inline void jl_snap_append(const struct jl_entry *e) { }
static inline void jl_snap_publish_debugfs(void) { }
#endif

#endif