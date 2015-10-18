#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x1e94b2a0, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x51eafc8e, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x73c96062, __VMLINUX_SYMBOL_STR(blk_cleanup_queue) },
	{ 0xf07c91d, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0xd4f72046, __VMLINUX_SYMBOL_STR(put_disk) },
	{ 0xee0d0891, __VMLINUX_SYMBOL_STR(del_gendisk) },
	{ 0xb5a459dc, __VMLINUX_SYMBOL_STR(unregister_blkdev) },
	{ 0xb071d227, __VMLINUX_SYMBOL_STR(add_disk) },
	{ 0xeecc088b, __VMLINUX_SYMBOL_STR(alloc_disk) },
	{ 0xe5d54a00, __VMLINUX_SYMBOL_STR(blk_queue_max_hw_sectors) },
	{ 0xa839614f, __VMLINUX_SYMBOL_STR(blk_queue_io_min) },
	{ 0xd2178458, __VMLINUX_SYMBOL_STR(blk_queue_physical_block_size) },
	{ 0xcdf0e316, __VMLINUX_SYMBOL_STR(blk_queue_logical_block_size) },
	{ 0x70cb5d65, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0x71a50dbc, __VMLINUX_SYMBOL_STR(register_blkdev) },
	{ 0x3414fd01, __VMLINUX_SYMBOL_STR(blk_init_queue) },
	{ 0xff8aecb8, __VMLINUX_SYMBOL_STR(blk_end_request_cur) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x6614a29, __VMLINUX_SYMBOL_STR(__blk_end_request_cur) },
	{ 0x6e0a5efd, __VMLINUX_SYMBOL_STR(__blk_end_request_all) },
	{ 0xffcf931e, __VMLINUX_SYMBOL_STR(blk_fetch_request) },
	{ 0x4f1eff, __VMLINUX_SYMBOL_STR(try_module_get) },
	{ 0x87e897fa, __VMLINUX_SYMBOL_STR(module_put) },
	{ 0xc0c6a0f4, __VMLINUX_SYMBOL_STR(call_usermodehelper_exec) },
	{ 0x370b6690, __VMLINUX_SYMBOL_STR(call_usermodehelper_setup) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x3d1f7a21, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xda22cdde, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "35A53D2CEBD5E6BB16304CE");
