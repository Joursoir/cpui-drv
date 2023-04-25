#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

struct cpui_info {
	uint32_t cpuid_max;
	char vendor_string[13];
};

static struct cpui_info __cpu;

static void detect_cpu(struct cpui_info *cpu)
{
	/* Get CPU Vendor ID String */
	cpuid(0x00, (unsigned int *)&cpu->cpuid_max,
		(unsigned int *)&cpu->vendor_string[0],
		(unsigned int *)&cpu->vendor_string[8],
		(unsigned int *)&cpu->vendor_string[4]);
	cpu->vendor_string[12] = '\0';
}

static int __init cpui_init(void)
{
	if (!have_cpuid_p()) {
		pr_err("CPUID instruction is NOT supported. Aborting.\n");
		return 1;
	}
	pr_info("CPUID instruction is supported.\n");

	memset(&__cpu, 0, sizeof(__cpu));
	detect_cpu(&__cpu);
	pr_info("Maximum Input Value for Basic CPUID Information = %d\n", __cpu.cpuid_max);
	pr_info("Identity string = %s\n", __cpu.vendor_string);
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