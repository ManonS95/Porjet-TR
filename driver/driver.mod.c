#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xe7cc3b88, "module_layout" },
	{ 0x9a152689, "param_ops_int" },
	{ 0xb7d8a740, "class_unregister" },
	{ 0x1ac09638, "device_destroy" },
	{ 0xfe990052, "gpio_free" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x21ccf193, "class_destroy" },
	{ 0xcd4c837a, "device_create" },
	{ 0x5ba17792, "__class_create" },
	{ 0xf31494e5, "__register_chrdev" },
	{ 0xdbdb0e8b, "request_any_context_irq" },
	{ 0xd3d255e8, "gpiod_to_irq" },
	{ 0x79f06713, "gpio_to_desc" },
	{ 0x403f9529, "gpio_request_one" },
	{ 0xe97c4103, "ioremap" },
	{ 0x2cfde9a2, "warn_slowpath_fmt" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xc5850110, "printk" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "1827EF22968F78DA926CA0D");
