#include <linux/jump_label_patch_log.h>
#include <linux/atomic.h>

#define JL_MAX 8192  // tune or make it cmdline-configurable
static struct jl_entry jl_buf[JL_MAX];
static atomic_t jl_widx = ATOMIC_INIT(0);

#ifdef CONFIG_JUMP_LABEL_PATCH_LOG_SNAPSHOT
void jl_snap_append(const struct jl_entry *e)
{
	int i = atomic_fetch_add_unless(&jl_widx, 1, JL_MAX);
	if (i >= JL_MAX) return; // drop if full
	jl_buf[i] = *e;
}
#endif

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int jl_dbg_show(struct seq_file *m, void *v)
{
	int i, n = min(atomic_read(&jl_widx), JL_MAX);
	for (i = 0; i < n; i++) {
		const struct jl_entry *e = &jl_buf[i];
		seq_printf(m, "%llx,%08x\n",
			(unsigned long long)e->addr,
			e->new_insn);
	}
	return 0;
}
static int jl_dbg_open(struct inode *inode, struct file *file)
{
	return single_open(file, jl_dbg_show, NULL);
}
static const struct file_operations jl_dbg_fops = {
	.owner = THIS_MODULE,
	.open  = jl_dbg_open,
	.read  = seq_read,
	.llseek= seq_lseek,
	.release= single_release,
};

#ifdef CONFIG_JUMP_LABEL_PATCH_LOG_SNAPSHOT
static struct dentry *jl_dent;
void jl_snap_publish_debugfs(void)
{
	if (!jl_dent)
		jl_dent = debugfs_create_file("jump_label_snapshot", 0444,
		                              NULL, NULL, &jl_dbg_fops);
}

static int __init jl_snap_publish_debugfs_init(void)
{
	jl_snap_publish_debugfs();
	return 0;
}
late_initcall(jl_snap_publish_debugfs_init);

#endif // CONFIG_JUMP_LABEL_PATCH_LOG_SNAPSHOT
#else // CONFIG_DEBUG_FS
void jl_snap_publish_debugfs(void) { }
#endif // CONFIG_DEBUG_FS