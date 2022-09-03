#include <rvh_test.h>
#include <page_tables.h>

static inline void touchread(uintptr_t addr){
    asm volatile("" ::: "memory");
    volatile uint64_t x = *(volatile uint64_t *)addr;
}

bool two_stage_translation() {
    
    TEST_START();

    uintptr_t addr1 = phys_page_base(SWITCH1);  //   8800_0000 + SWITCH1 * PAGE_SIZE
    uintptr_t addr2 = phys_page_base(SWITCH2);
    uintptr_t vaddr1 = vs_page_base(SWITCH1);   // 1_0000_0000 + SWITCH1 * PAGE_SIZE
    uintptr_t vaddr2 = vs_page_base(SWITCH2);
    write64(addr1, 0x11);
    write64(addr2, 0x22);

    /**
     * Setup hyp page_tables.
     */
    goto_priv(PRIV_HS);
    hspt_init();
    hpt_init();

    /**
     * Setup guest page tables.
     */
    goto_priv(PRIV_VS);
    vspt_init();

    bool check1 = read64(vaddr1) == 0x11;
    bool check2 = read64(vaddr2) == 0x22;
    TEST_ASSERT("vs gets right values", check1 && check2);

    goto_priv(PRIV_HS);
    hpt_switch();
    hfence();
    goto_priv(PRIV_VS);
    check1 = read64(vaddr1) == 0x22;
    check2 = read64(vaddr2) == 0x11;   
    // INFO("0%lx 0x%lx", read64(vaddr1), read64(vaddr2));
    TEST_ASSERT("vs gets right values after changing 2nd stage pt", check1 && check2);

    vspt_switch();
    sfence();
    check1 = read64(vaddr1) == 0x11;
    check2 = read64(vaddr2) == 0x22;   
    TEST_ASSERT("vs gets right values after changing 1st stage pt", check1 && check2);

    goto_priv(PRIV_M); 
    CSRS(medeleg, 1ull << CAUSE_LGPF);
    goto_priv(PRIV_VS);
    TEST_SETUP_EXCEPT();
    read64(vs_page_base(VSRWX_GI));    
    TEST_ASSERT(
        "load guest page fault on unmapped address", 
        excpt.triggered == true && 
        excpt.cause == CAUSE_LGPF &&
        excpt.tval2 == (vs_page_base(VSRWX_GI) >> 2) &&
        excpt.priv == PRIV_HS &&
        excpt.gva == true &&
        excpt.xpv == true
    );

    TEST_SETUP_EXCEPT();
    TEST_EXEC_EXCEPT(vs_page_base(VSRWX_GI)); 
    TEST_ASSERT(
        "instruction guest page fault on unmapped 2-stage address", 
        excpt.triggered == true && 
        excpt.cause == CAUSE_IGPF &&
        excpt.tval2 == (vs_page_base(VSRWX_GI) >> 2) &&
        excpt.priv == PRIV_M  &&
        excpt.gva == true &&
        excpt.xpv == true
    );

    goto_priv(PRIV_M);
    CSRS(medeleg, 1 << CAUSE_LPF | 1 << CAUSE_LGPF);
    goto_priv(PRIV_HS); 
    CSRS(CSR_HEDELEG, 1 << CAUSE_LPF);
    goto_priv(PRIV_VS);
    sfence();
    TEST_SETUP_EXCEPT();
    touchread(vs_page_base(VSI_GI));    
    TEST_ASSERT(
        "invalid pte in both stages leads to s1 page fault", 
        excpt.triggered == true && 
        excpt.cause == CAUSE_LPF &&
        excpt.priv == PRIV_VS && 
        excpt.gva == false
    );

    TEST_END();
}