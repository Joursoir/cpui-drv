#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

struct cpui_info {
	uint32_t cpuid_max;
	char vendor_string[13];

	uint8_t family;
	uint8_t model;
	uint8_t stepping;
};

static struct cpui_info __cpu;

static uint8_t get_family(uint32_t verinfo)
{
	uint8_t display_family;

	display_family = (verinfo >> 8) & 0xf;

	/* Examine Extended Family ID */
	if (display_family == 0x0f)
		display_family += (verinfo >> 20) & 0xff;

	return display_family;
}

static uint8_t get_model(uint32_t verinfo)
{
	uint8_t family, display_model;

	family = get_family(verinfo);
	display_model = (verinfo >> 4) & 0xf;

	/* Examine Extended Model ID */
	if (family == 0x06 || family == 0x0f)
		display_model += ((verinfo >> 16) & 0xf) << 4;

	return display_model;
}

static uint8_t get_stepping(uint32_t verinfo)
{
	return verinfo & 0xf;
}

static void detect_cpu(struct cpui_info *cpu)
{
	/* Get CPU Vendor ID String */
	cpuid(0x00, (unsigned int *)&cpu->cpuid_max,
		(unsigned int *)&cpu->vendor_string[0],
		(unsigned int *)&cpu->vendor_string[8],
		(unsigned int *)&cpu->vendor_string[4]);
	cpu->vendor_string[12] = '\0';

	if (cpu->cpuid_max >= 0x01) {
		uint32_t eax, ebx, ecx, edx;

		cpuid(0x01, &eax, &ebx, &ecx, &edx);
		cpu->family	= get_family(eax);
		cpu->model	= get_model(eax);
		cpu->stepping	= get_stepping(eax);
	}
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