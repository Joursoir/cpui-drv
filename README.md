# CPU Internals Driver (cpui-drv)

A driver that allows users to interact with various internal components of the CPU. Implemented and tested with Linux kernel v6.2

## CPU IDentification (CPUID)

The software needs a way to find out if particular features are available (for example, 64-bit mode, Hardware Virtualization, HyperThreading, etc). The CPUID instruction is a mechanism that can be used to retrieve such information to understand whether or not CPU supports these features.

CPUID does NOT have operands. Rather it takes input as a value preloaded into _eax_ (it's _eax_ even on 64-bit systems, not _rax_) and possibly _ecx_. After the instruction is executed, the outputs are stored to _eax_, _ebx_, _ecx_, and _edx_.

### How to Check for CPUID Support

CPUID didn't exist on the first systems so we actually have to check if the hardware supports CPUID (added in late i486 models):

```
The ID flag (bit 21) in the EFLAGS register indicates support for the CPUID instruction. If a software procedure can set and clear this flag, the processor executing the procedure supports the CPUID instruction.
```

To manipulate with the EFLAGS register values, we need `PUSHF` / `POPF` instructions, they push / pop lower 16 bits of EFLAGS to the stack.

That's how it's done in the Linux kernel ([arch/x86/kernel/cpu/common.c](https://github.com/torvalds/linux/blob/c9c3395d5e3dcc6daee66c6908354d47bf98cb0c/arch/x86/kernel/cpu/common.c#L304)):

``` c
/* Standard macro to see if a specific flag is changeable */
static inline int flag_is_changeable_p(u32 flag)
{
  u32 f1, f2;
  asm volatile (
    "pushfl      \n\t" // Push the current value of the EFLAGS
    "pushfl      \n\t" // register onto the stack twice.
    "popl %0     \n\t" // Pop the original EFLAGS value into the var `f1`
    "movl %0, %1 \n\t" // Move the value of `f1` into `f2`
    "xorl %2, %0 \n\t" // XOR op. between `flag` and `f1`
    "pushl %0    \n\t" // Push the modified value of `f1` onto the stack and..
    "popfl       \n\t" // Pop it into the EFLAGS register
    "pushfl      \n\t" // Push the current value of the EFLAGS
                       // register onto the stack
    "popl %0     \n\t" // Pop it into `f2`
    "popfl       \n\t" // Restore the original value of the EFLAGS

    : "=&r" (f1), "=&r" (f2)
    : "ir" (flag));

  return ((f1^f2) & flag) != 0;
}

/* Probe for the CPUID instruction */
int have_cpuid_p(void)
{
  return flag_is_changeable_p(X86_EFLAGS_ID);
}
```

`X86_EFLAGS_ID` is defined in [arch/x86/include/uapi/asm/processor-flags.h](https://github.com/torvalds/linux/blob/c9c3395d5e3dcc6daee66c6908354d47bf98cb0c/arch/x86/include/uapi/asm/processor-flags.h#L46) :

``` c
#define X86_EFLAGS_ID_BIT	21 /* CPUID detection */
#define X86_EFLAGS_ID		_BITUL(X86_EFLAGS_ID_BIT)
```

To obtain the table with possible inputs and outputs, use _Intel 64 and IA-32 Architectures Software Developer's Manual,  Volume 2, Chapter 3.3 Instructions, CPUID - CPU Identification_.

## Model Specific Registers (MSRs)

After the fact, when you find out that some features you want to use are supported, you want to have a mechanism for configuring them. Over time, this list of MSRs has grown so large that it has become a separate volume of the Intel manuals - _Volume 4: Model-Specific Registers_.

Many MSRs have carried over from one generation of IA-32 processors to the next and to Intel 64 processors. A subset of MSRs and associated bit fields, which do **NOT** change on future processor generations, are now considered **architectural MSRs**. For historical reasons (beginning with the Pentium 4 processor), these “**architectural MSRs**” were given the prefix “**IA32_**” (it doesn't mean it's restricted to 32-bit execution).

`RDMSR` / `WRMSR` are privileged instructions, so they cannot be used in user-space, only in kernel-mode.

Take a look how the Linux kernel uses these instructions in C code ([arch/x86/include/asm/msr.h](https://github.com/torvalds/linux/blob/c9c3395d5e3dcc6daee66c6908354d47bf98cb0c/arch/x86/include/asm/msr.h)):

``` c
/*
 * both i386 and x86_64 returns 64-bit value in edx:eax, but gcc's "A"
 * constraint has different meanings. For i386, "A" means exactly
 * edx:eax, while for x86_64 it doesn't mean rdx:rax or edx:eax. Instead,
 * it means rax *or* rdx.
 */
#ifdef CONFIG_X86_64
  /* Using 64-bit values saves one instruction clearing the high half of low */
  #define DECLARE_ARGS(val, low, high)  unsigned long low, high
  #define EAX_EDX_VAL(val, low, high) ((low) | (high) << 32)
  #define EAX_EDX_RET(val, low, high) "=a" (low), "=d" (high)
#else
  #define DECLARE_ARGS(val, low, high)  unsigned long long val
  #define EAX_EDX_VAL(val, low, high) (val)
  #define EAX_EDX_RET(val, low, high) "=A" (val)
#endif

...

static __always_inline unsigned long long __rdmsr(unsigned int msr)
{
  DECLARE_ARGS(val, low, high);

  asm volatile("1: rdmsr\n"
         "2:\n"
         _ASM_EXTABLE_TYPE(1b, 2b, EX_TYPE_RDMSR)
         : EAX_EDX_RET(val, low, high) : "c" (msr));

  return EAX_EDX_VAL(val, low, high);
}

static __always_inline void __wrmsr(unsigned int msr, u32 low, u32 high)
{
  asm volatile("1: wrmsr\n"
         "2:\n"
         _ASM_EXTABLE_TYPE(1b, 2b, EX_TYPE_WRMSR)
         : : "c" (msr), "a"(low), "d" (high) : "memory");
}
```

## Resources

* [Intel 64 and IA-32 Architectures Software Developer's Manual, Combined Volumes: 1, 2A, 2B, 2C, 2D, 3A, 3B, 3C, 3D and 4](https://cdrdv2.intel.com/v1/dl/getContent/671200)
* [AMD CPUID Specification](https://www.amd.com/system/files/TechDocs/25481.pdf)
