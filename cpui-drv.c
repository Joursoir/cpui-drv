#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <asm/msr.h>

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

#define FEATURE_SSE3		(0 + FEATURES_01_ECX*32)
#define FEATURE_PCLMUL		(1 + FEATURES_01_ECX*32)
#define FEATURE_DTES64		(2 + FEATURES_01_ECX*32)
#define FEATURE_MONITOR		(3 + FEATURES_01_ECX*32)
#define FEATURE_DS_CPL		(4 + FEATURES_01_ECX*32)
#define FEATURE_VMX		(5 + FEATURES_01_ECX*32)
#define FEATURE_SMX		(6 + FEATURES_01_ECX*32)
#define FEATURE_EIST		(7 + FEATURES_01_ECX*32)
#define FEATURE_TM2		(8 + FEATURES_01_ECX*32)
#define FEATURE_SSSE3		(9 + FEATURES_01_ECX*32)
#define FEATURE_CNXT_ID		(10 + FEATURES_01_ECX*32)
#define FEATURE_SDBG		(11 + FEATURES_01_ECX*32)
#define FEATURE_FMA		(12 + FEATURES_01_ECX*32)
#define FEATURE_CMPXCHG16B	(13 + FEATURES_01_ECX*32)
#define FEATURE_XTPR		(14 + FEATURES_01_ECX*32)
#define FEATURE_PDCM		(15 + FEATURES_01_ECX*32)
#define FEATURE_PCID		(17 + FEATURES_01_ECX*32)
#define FEATURE_DCA		(18 + FEATURES_01_ECX*32)
#define FEATURE_SSE4_1		(19 + FEATURES_01_ECX*32)
#define FEATURE_SSE4_2		(20 + FEATURES_01_ECX*32)
#define FEATURE_X2APIC		(21 + FEATURES_01_ECX*32)
#define FEATURE_MOVBE		(22 + FEATURES_01_ECX*32)
#define FEATURE_POPCNT		(23 + FEATURES_01_ECX*32)
#define FEATURE_TSCD		(24 + FEATURES_01_ECX*32) /* TSC-Deadline */
#define FEATURE_AES		(25 + FEATURES_01_ECX*32)
#define FEATURE_XSAVE		(26 + FEATURES_01_ECX*32)
#define FEATURE_OSXSAVE		(27 + FEATURES_01_ECX*32)
#define FEATURE_AVX		(28 + FEATURES_01_ECX*32)
#define FEATURE_F16C		(29 + FEATURES_01_ECX*32)
#define FEATURE_RDRAND		(30 + FEATURES_01_ECX*32)

#define FEATURE_FPU		(0 + FEATURES_01_EDX*32)
#define FEATURE_VME		(1 + FEATURES_01_EDX*32)
#define FEATURE_DE		(2 + FEATURES_01_EDX*32)
#define FEATURE_PSE		(3 + FEATURES_01_EDX*32)
#define FEATURE_TSC		(4 + FEATURES_01_EDX*32) /* Time Stamp Counter */
#define FEATURE_MSR		(5 + FEATURES_01_EDX*32)
#define FEATURE_PAE		(6 + FEATURES_01_EDX*32)
#define FEATURE_MCE		(7 + FEATURES_01_EDX*32)
#define FEATURE_CX8		(8 + FEATURES_01_EDX*32)
#define FEATURE_APIC		(9 + FEATURES_01_EDX*32)
#define FEATURE_SEP		(11 + FEATURES_01_EDX*32)
#define FEATURE_MTRR		(12 + FEATURES_01_EDX*32)
#define FEATURE_PGE		(13 + FEATURES_01_EDX*32)
#define FEATURE_MCA		(14 + FEATURES_01_EDX*32)
#define FEATURE_CMOV		(15 + FEATURES_01_EDX*32)
#define FEATURE_PAT		(16 + FEATURES_01_EDX*32)
#define FEATURE_PSE36		(17 + FEATURES_01_EDX*32)
#define FEATURE_PSN		(18 + FEATURES_01_EDX*32)
#define FEATURE_CLFLUSH		(19 + FEATURES_01_EDX*32)
#define FEATURE_DS		(21 + FEATURES_01_EDX*32)
#define FEATURE_ACPI		(22 + FEATURES_01_EDX*32)
#define FEATURE_MMX		(23 + FEATURES_01_EDX*32)
#define FEATURE_FXSR		(24 + FEATURES_01_EDX*32)
#define FEATURE_SSE		(25 + FEATURES_01_EDX*32)
#define FEATURE_SSE2		(26 + FEATURES_01_EDX*32)
#define FEATURE_SS		(27 + FEATURES_01_EDX*32)
#define FEATURE_HTT		(28 + FEATURES_01_EDX*32)
#define FEATURE_TM		(29 + FEATURES_01_EDX*32)
#define FEATURE_PBE		(31 + FEATURES_01_EDX*32)

#define FEATURE_SYSCALL		(11 + FEATURES_8000_0001_EDX*32)
#define FEATURE_EXECD		(20 + FEATURES_8000_0001_EDX*32)
#define FEATURE_GBPAGES		(26 + FEATURES_8000_0001_EDX*32)
#define FEATURE_RDTSCP		(27 + FEATURES_8000_0001_EDX*32)
#define FEATURE_LM		(29 + FEATURES_8000_0001_EDX*32)

#define test_cpu_feat(c, bit) \
	 arch_test_bit(bit, (unsigned long *)((c)->features))

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
	uint64_t value;
	struct cpui_info *cpu;

	if (!have_cpuid_p()) {
		pr_err("CPUID instruction is NOT supported. Aborting.\n");
		return 1;
	}
	pr_info("CPUID instruction is supported.\n");

	memset(&__cpu, 0, sizeof(__cpu));
	cpu = &__cpu;
	detect_cpu(cpu);
	pr_info("Maximum Input Value for Basic CPUID Information = %d\n", cpu->cpuid_max);
	pr_info("Identity string = %s\n", cpu->vendor_string);

	get_cpu_features(cpu);

	if (!test_cpu_feat(cpu, FEATURE_MSR)) {
		pr_err("The RDMSR and WRMSR instructions are NOT supported. Aborting.\n");
		return 1;
	}

	/* Check if IA32_EFER is present */
	if (test_cpu_feat(cpu, FEATURE_EXECD) || test_cpu_feat(cpu, FEATURE_LM)) {
		rdmsrl(MSR_EFER, value);
		pr_info("IA32_EFER = 0x%016llx\n", value);
		pr_info("\tIA32_EFER.SCE (System Call Extensions) = %llu\n", (value & EFER_SCE) >> _EFER_SCE);
		pr_info("\tIA32_EFER.LME (Long Mode Enable) = %llu\n", (value & EFER_LME) >> _EFER_LME);
		pr_info("\tIA32_EFER.LMA (Long Mode Active) = %llu\n", (value & EFER_LMA) >> _EFER_LMA);
		pr_info("\tIA32_EFER.NXE (No-Execute Enable) = %llu\n", (value & EFER_NX) >> _EFER_NX);
		pr_info("\tIA32_EFER.SVME (Secure Virtual Machine Enable) = %llu\n", (value & EFER_SVME) >> _EFER_SVME);
		pr_info("\tIA32_EFER.LMSLE (Long Mode Segment Limit Enable) = %llu\n", (value & EFER_LMSLE) >> _EFER_LMSLE);
	}
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