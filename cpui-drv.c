#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

static int __init cpui_init(void)
{
	char id_str[13];
	uint32_t eax = 0, ebx, ecx, edx;

	if (!have_cpuid_p()) {
		pr_err("CPUID instruction is NOT supported. Aborting.\n");
		return 1;
	}

	cpuid(0, &eax, &ebx, &ecx, &edx);
	pr_info("CPUID instruction is supported.\n");
	pr_info("Maximum Input Value for Basic CPUID Information = %d\n", eax);
	memcpy(&(id_str[0]), &ebx, 4);
	memcpy(&(id_str[4]), &edx, 4);
	memcpy(&(id_str[8]), &ecx, 4);
	id_str[12] = '\0';
	pr_info("Identity string = %s\n", id_str);
	return 0;
}

static void __exit cpui_exit(void)
{
	pr_info("Exit complete\n");
}

module_init(cpui_init);
module_exit(cpui_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joursoir");
MODULE_DESCRIPTION("A driver that allows users to interact with various internal components of the CPU");