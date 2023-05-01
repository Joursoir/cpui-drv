#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

enum feature_information {
	FEATURES_01_ECX		= 0,
	FEATURES_01_EDX,
	FEATURES_6_EAX,
	FEATURES_7_0_EBX,
	FEATURES_7_ECX,
	FEATURES_7_EDX,
	FEATURES_7_1_EAX,
	FEATURES_8000_0001_ECX,
	FEATURES_8000_0001_EDX,
	FEATURES_LAST,
};

struct cpui_info {
	uint32_t cpuid_max;
	uint32_t ext_cpuid_max;
	char vendor_string[13];

	uint8_t family;
	uint8_t model;
	uint8_t stepping;

	uint32_t features[FEATURES_LAST];
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

static void get_cpu_features(struct cpui_info *cpu)
{
	uint32_t eax, ebx, ecx, edx;

	if (cpu->cpuid_max >= 0x01) {
		cpuid(0x01, &eax, &ebx, &ecx, &edx);
		cpu->features[FEATURES_01_ECX] = ecx;
		cpu->features[FEATURES_01_EDX] = edx;
	}

	/* Thermal and Power Management Leaf */
	if (cpu->cpuid_max >= 0x06) {
		cpuid(0x01, &eax, &ebx, &ecx, &edx);
		cpu->features[FEATURES_6_EAX] = eax;
	}

	/* Structured Extended Feature Flags Enumeration Leaf */
	if (cpu->cpuid_max >= 0x07) {
		cpuid_count(0x07, 0, &eax, &ebx, &ecx, &edx);
		cpu->features[FEATURES_7_0_EBX] = ebx;
		cpu->features[FEATURES_7_ECX] = ecx;
		cpu->features[FEATURES_7_EDX] = edx;

		if (eax >= 1) {
			cpuid_count(0x07, 1, &eax, &ebx, &ecx, &edx);
			cpu->features[FEATURES_7_1_EAX] = eax;
		}
	}

	/* Get Maximum Input Value for Extended Function CPUID Information */
	cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
	cpu->ext_cpuid_max = eax;

	if (cpu->ext_cpuid_max >= 0x80000001) {
		cpuid(0x80000001, &eax, &ebx, &ecx, &edx);

		cpu->features[FEATURES_8000_0001_ECX] = ecx;
		cpu->features[FEATURES_8000_0001_EDX] = edx;
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

	get_cpu_features(&__cpu);
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