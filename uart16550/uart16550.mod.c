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
	{ 0x897778bc, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x51eafc8e, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x188c027c, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x9bce482f, __VMLINUX_SYMBOL_STR(__release_region) },
	{ 0x39ed7d58, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x4993e7aa, __VMLINUX_SYMBOL_STR(class_unregister) },
	{ 0xa475c139, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x8ad9a685, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xadf42bd5, __VMLINUX_SYMBOL_STR(__request_region) },
	{ 0x59d8223a, __VMLINUX_SYMBOL_STR(ioport_resource) },
	{ 0x22f068d1, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xfcec0987, __VMLINUX_SYMBOL_STR(enable_irq) },
	{ 0x2a4b3dfe, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0x3ce4ca6f, __VMLINUX_SYMBOL_STR(disable_irq) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x52afc890, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0x51a5ae61, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0x4de561e3, __VMLINUX_SYMBOL_STR(cdev_alloc) },
	{ 0xc3aaf0a9, __VMLINUX_SYMBOL_STR(__put_user_1) },
	{ 0x4292364c, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xd115b743, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x8063aca5, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0xd16b0bfd, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

