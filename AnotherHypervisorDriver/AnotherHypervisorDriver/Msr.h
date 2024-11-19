#pragma once

#define MSR_IA32_FEATURE_CONTROL        0x0000003a

/* Intel VT MSRs */
#define MSR_IA32_VMX_BASIC              0x00000480
#define MSR_IA32_VMX_PINBASED_CTLS      0x00000481
#define MSR_IA32_VMX_PROCBASED_CTLS     0x00000482
#define MSR_IA32_VMX_EXIT_CTLS          0x00000483
#define MSR_IA32_VMX_ENTRY_CTLS         0x00000484
#define MSR_IA32_VMX_MISC               0x00000485
#define MSR_IA32_VMX_CR0_FIXED0         0x00000486
#define MSR_IA32_VMX_CR0_FIXED1         0x00000487
#define MSR_IA32_VMX_CR4_FIXED0         0x00000488
#define MSR_IA32_VMX_CR4_FIXED1         0x00000489
#define MSR_IA32_VMX_VMCS_ENUM          0x0000048a
#define MSR_IA32_VMX_PROCBASED_CTLS2    0x0000048b
#define MSR_IA32_VMX_EPT_VPID_CAP       0x0000048c
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS  0x0000048d
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS 0x0000048e
#define MSR_IA32_VMX_TRUE_EXIT_CTLS      0x0000048f
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS     0x00000490
#define MSR_IA32_VMX_VMFUNC             0x00000491
#define MSR_IA32_VMX_PROCBASED_CTLS3	0x00000492



// Copied from https://tandasat.github.io/HyperPlatform/doxygen/ia32__type_8h_source.html
union Ia32FeatureControlMsr {
    unsigned __int64 all;
    struct {
        unsigned lock : 1;              // Defines whether this MSR is locked
        unsigned enable_smx : 1;        // Safe Mode Extension
        unsigned enable_vmxon : 1;
        unsigned reserved1 : 5;
        unsigned enable_local_senter : 7;
        unsigned enable_global_senter : 1;
        unsigned reserved2 : 16;
        unsigned reserved3 : 32;
    } fields;
};

union Ia32VmxBasicMsr {
    unsigned __int64 all;
    struct {
        unsigned revision_identifier : 31;
        unsigned reserved1 : 1;
        unsigned region_size : 12;
        unsigned region_clear : 1;
        unsigned reserved2 : 3;
        unsigned supported_ia64 : 1;
        unsigned supported_dual_moniter : 1;
        unsigned memory_type : 4;
        unsigned vm_exit_report : 1;
        unsigned vmx_capability_hint : 1;
        unsigned reserved3 : 8;
    } fields;
};