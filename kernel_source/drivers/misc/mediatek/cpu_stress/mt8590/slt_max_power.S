.text
//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2013 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2013-07-15 11:45:00 +0100 (Mon, 15 Jul 2013) $
//
//      Revision            : $Revision: 254301 $
//
//      Release Information : CORTEXA53-r0p0-00lac0
//
//-------------------------------------------------------------------------------
// Description:
//
//   Maximum power test
//-------------------------------------------------------------------------------

    //            .section testcode, "ax", %progbits

    //            .global test_start
                //.global fiq_handler

//.include "mcusys.hs"
.equ NUM_ITERS, 100
//.equ NUM_ITERS, 200
//.equ MDM_TM_ENDSIM , 0x10009010
//.equ GPIO , 0x10005104
//-------------------------------------------------------------------------------
// FIQ handler
//
//   This test uses a FIQ to release CPUs from WFI.  The FIQ handler simply
//   clears the FIQ using the trickbox and returns.  Note that all CPUs will
//   end up writing to the trickbox but only the first write will have any
//   effect.
//-------------------------------------------------------------------------------

// Make fiq_handler visible to linker so that it can override the default handler
//.global curr_el_spx_fiq_vector

                // Return using ERET.  This test uses WFI spin code (a WFI
                // instruction followed by a branch back to the WFI) to force
                // the CPUs to remain idle until the test breaks them out of the
                // WFI spin.  For asynchronous exceptions such as FIQ, the ELR
                // points to the first instruction that has not been fully
                // executed, which in the WFI spin loop is the branch back to
                // the WFI instruction.  Therefore we add 4 to the ELR before
                // returning, so that we break out of the loop and execute the
                // first instruction following the loop.
//                mrs     x21, elr_el3
//                add     x21, x21, #4
//                msr     elr_el3, x21
//                eret
//

//-------------------------------------------------------------------------------
// Main test code
//-------------------------------------------------------------------------------

//test_start:
//                //For FT pattern, assert the GPIO
//		mov  x1, #0x1
//		ldr  x0, =GPIO
//	        ldr  w2, [x0]
//		lsl  x1, x1, #21
//		orr  x2, x2, x1
//                str  w2, [x0]
//		dsb  sy
//
//                //read SOC register for loop count
//iters_setup:
//                ldr  x1, =MCUSYS_RSVD_1
//		ldr  w0, [x1]
//		and  x0, x0, #0x7
//		cmp  x0, #0
//		beq  iters0
//		cmp  x0, #1
//		beq  iters1
//		cmp  x0, #2
//		beq  iters2
//		cmp  x0, #3
//		beq  iters3
//		cmp  x0, #4
//		beq  iters4
//		cmp  x0, #5
//		beq  iters5
//		cmp  x0, #6
//		beq  iters6
//		cmp  x0, #7
//		beq  iters7
//
//iters0:        mov  x11, #50
//                b    setup
//iters1:        mov  x11, #100
//                b    setup
//iters2:        mov  x11, #200
//                b    setup
//iters3:        mov  x11, #500
//                b    setup
//iters4:        mov  x11, #800
//                b    setup
//iters5:        mov  x11, #1000
//                b    setup
//iters6:        mov  x11, #1536
//                b    setup
//iters7:        mov  x11, #2560

        .macro  push, xreg1, xreg2
        stp     \xreg1, \xreg2, [sp, #-16]!
        .endm

        .macro  pop, xreg1, xreg2
        ldp     \xreg1, \xreg2, [sp], #16
        .endm

.extern pMaxPowerTestMem
test_mem:
        .quad pMaxPowerTestMem

.global fp0_maxpower_start
fp0_maxpower_start:
setup:
		push    x28, x29
		push    x26, x27
		push    x24, x25
		push    x22, x23
		push    x20, x21
		push    x18, x19
		push    x16, x17
		push    x14, x15
		push    x12, x13
		push    x10, x11
		push    x8, x9
		push    x6, x7
		push    x4, x5
		push    x2, x3
		//push    x0, x1


                // Enable debug then enable all breakpoint and watchpoint
                // comparators.  Enabling breakpoints and watchpoints
                // increases power consumption for the test because the
                // comparators have to compare addresses against the breakpoint
                // and watchpoint registers.

                // Enable halting debug by setting the HDE bit of the MSDCR
                // and then clear the OS lock to enable programming of the
                // breakpoint and watchpoint registers.
                mrs     x0, mdscr_el1
                orr     x0, x0, #(0x1 << 14)    // HDE bit
                msr     mdscr_el1, x0
                mov     x0, #0
                msr     oslar_el1, x0           // Clear OS lock
                isb

                // Set breakpoint and watchpoint value registers
                adr     x0, bkpt0
                msr     dbgbvr0_el1, x0   // Write DBGBVR0
                adr     x0, bkpt1
                msr     dbgbvr1_el1, x0   // Write DBGBVR1
                adr     x0, bkpt2
                msr     dbgbvr2_el1, x0   // Write DBGBVR2
                adr     x0, bkpt3
                msr     dbgbvr3_el1, x0   // Write DBGBVR3
                adr     x0, bkpt4
                msr     dbgbvr4_el1, x0   // Write DBGBVR4
                adr     x0, bkpt5
                msr     dbgbvr5_el1, x0   // Write DBGBVR5
                adr     x0, wpt0
                msr     dbgwvr0_el1, x0   // Write DBGWVR0
                adr     x0, wpt1
                msr     dbgwvr0_el1, x0   // Write DBGWVR1
                adr     x0, wpt2
                msr     dbgwvr0_el1, x0   // Write DBGWVR2
                adr     x0, wpt3
                msr     dbgwvr0_el1, x0   // Write DBGWVR3

                // Set breakpoint and watchpoint enable registers to enable
                // in all states.  The value written to the DBGBCRn registers
                // is:
                //   Bit[13]   (HMC) - 1'b1    - State matching (match all states)
                //   Bits[8:5] (BAS) - 4'b1111 - Match A32/A64 instructions
                //   Bits[2:1] (PMC) - 2'b11   - State matching (match all states)
                //   Bit[0]    (E)   - 1'b1    - Enable this breakpoint
                mov     x0, #0x21E7       // DBGBCRn value
                msr     dbgbcr0_el1, x0   // Write DBGBCR0
                msr     dbgbcr1_el1, x0   // Write DBGBCR1
                msr     dbgbcr2_el1, x0   // Write DBGBCR2
                msr     dbgbcr3_el1, x0   // Write DBGBCR3
                msr     dbgbcr4_el1, x0   // Write DBGBCR4
                msr     dbgbcr5_el1, x0   // Write DBGBCR5

                // The value written to the DBGWCRn registers is:
                //   Bit[13]    (HMC) - 1'b1  - State matching (match all states)
                //   Bits[12:5] (BAS) - 8'hFF - Watch all byte lanes
                //   Bits[2:1]  (PAC) - 2'b11 - State matching (match all states)
                //   Bit[0]     (E)   - 1'b1  - Enable this watchpoint
                mov     x0, #0x3FE7       // DBGWCRn value
                msr     dbgwcr0_el1, x0   // Write DBGWCR0
                msr     dbgwcr1_el1, x0   // Write DBGWCR1
                msr     dbgwcr2_el1, x0   // Write DBGWCR2
                msr     dbgwcr3_el1, x0   // Write DBGWCR3

                LDR     x9,test_mem
                LDR     x9,[x9]
                ADD     x9,x9,#0x1000

                // Set initial register values
                //ldr     x0, =0xFFFFFFFF55555555
                //ldr     x1, =0x0000000055555555
                //ldr     x2, =0xFFFFFFFDAAAAAAAA
                //ldr     x3, =0x00000000AAAAAAAA
                //ldr     x4, =0xFFFFFFFF55555555
                //ldr     x5, =0x0000000055555555
                //ldr     x6, =0xFFFFFFFDAAAAAAAA
                //ldr     x7, =0x00000000AAAAAAAA
                ldr     x0, test_mem
                //ldr     x0, [x0]
                ldr     x0, [x0]
                add     x1, x0, #0x8
                add     x2, x0, #0x88
                add     x3, x0, #0x108
                add     x4, x0, #0x188
                add     x5, x0, #0x208
                add     x6, x0, #0x288
                add     x7, x0, #0x308
                add     x9, x0, #0x388
                add     x10,x0, #0x408

                mov     x12, #0x500
                mov     x13, #0x600
                mov     x14, #0x700
                mov     x15, #0x800
                mov     x16, #0x900
                mov     x17, #0xa00
                mov     x18, #0xb00
                mov     x19, #0xc00

                movi v1.16b, #0x66  // q1 = 0x66666666
                movi v2.16b, #0xAA  // q2 = 0xAAAAAAAA
                movi v3.16b, #0x66  // q3 = 0x66666666
                movi v4.16b, #0xAA  // q4 = 0xAAAAAAAA
                movi v5.16b, #0x66  // q5 = 0x66666666
                movi v6.16b, #0xAA  // q6 = 0xAAAAAAAA

                // Now CPU0 programs the trickbox to schedule another FIQ and
                // all CPUs enter WFI.  The CPUs will then all wake and execute
                // the power loop at the same time.
                // others CPU enter in WFE after setup
//              MOV     x24, #0x0
//              MOV     x22, #0x2
//              mrs     x28, mpidr_el1           // Read MPIDR
//              UBFX    x28, x28, #8, #8 // Cluster ID
//              mov     x21, x28
//              LSL     x21, x21 , x22
//              mrs     x28, mpidr_el1           // Read MPIDR 
//		        UBFX    x28, x28, #0, #8
//		        ORR     x20, x28, x21
//		        CBNZ    x20, cpu_wait

//wait_all_core:
//              //ldr    x20, =0x10207000
//              //ldr    w23, [x20]
//              //mov    x23, #0xfe
//	      // efuse info
//              MOV     x15, #0x1
//              LDR     x20, =SOC_FUSED
//              LDR     w23, [x20]
//              UBFX    x23, x23, #4, #8
//              EOR     x23, x23, #0xff
//              SUB     x23, x23, x15
//
//              ldr    x20, =MP0_MISC_CONFIG
//              ldr    w21, [x20]
//              lsr    x21, x21, #20
//              ldr    x20, =MP1_MISC_CONFIG
//              ldr    w22, [x20]
//              lsr    x22, x22, #16
//              orr    x21, x21 ,x22
//              and    x21, x21, #0xff
//              cmp    w21, w23
//              bne    wait_all_core
//              sev   
//              b finish_setup
//cpu_wait:
//              wfe
//              wfe

finish_setup:
                // Finish setting up registers before entering WFI
                //ldr     x9,  =0xFFFFFFFD55555555
                //ldr     x10, =0x0000000155555555
                mov     x11, #NUM_ITERS
           
                // On return from the FIQ that releases us from WFI branch
                // straight to the power loop.  There may be some padding before
                // the loop in order to align the code in the loop.
	       b power_loop
//-------------------------------------------------------------------------------
// Main power loop
//
// The intent is for the add/sub instructions to cause all input bits to change,
// yet create a result that can be used as a pointer within a 4GB address space.
// These instructions, along with the load register-offset shifted, run along
// behind the FMLA.
//-------------------------------------------------------------------------------

                // Align the loop to an 8-byte boundary so that two instructions
                // will be in the same instruction cache data bank.  This allows
                // us to dual-issue all pairs of instructions in the loop for
                // maximum power in the pipeline.
power_loop:
                .balign 8
                // Group 1
                fmla      v1.4s, v2.4s, v3.4s  // Dual Issue Pair 0, Instruction 0
                ldr       w8, [x0, x12, lsl #2] // Dual Issue Pair 0, Instruction 1

                ldr       w8, [x2, x13, lsl #2] // Dual Issue Pair 1, Instruction 0
                add       x0, x1, x12           // Dual Issue Pair 1, Instruction 1

                sub       x2, x3, x13          // Dual Issue Pair 2, Instruction 0
                ldr       w8, [x4, x14, lsl #2] // Dual Issue Pair 2, Instruction 1

                ldr       w8, [x6, x15, lsl #2] // Dual Issue Pair 3, Instruction 0
                add       x4, x9, x14           // Dual Issue Pair 3, Instruction 1

                // Group 2
                fmla      v4.4s, v5.4s, v6.4s  // Dual Issue Pair 0, Instruction 0
                ldr       w8, [x0, x16, lsl #2] // Dual Issue Pair 0, Instruction 1

                ldr       w8, [x2, x17, lsl #2] // Dual Issue Pair 1, Instruction 0
                sub       x0, x3, x15          // Dual Issue Pair 1, Instruction 1

                add       x2, x1, x16           // Dual Issue Pair 2, Instruction 0
                ldr       w8, [x4, x18, lsl #2] // Dual Issue Pair 2, Instruction 1

                ldr       w8, [x6, x19, lsl #2] // Dual Issue Pair 3, Instruction 0
                sub       x4, x7, x17         // Dual Issue Pair 3, Instruction 1

                // Group 3
                fmla      v1.4s, v2.4s, v3.4s  // Dual Issue Pair 0, Instruction 0
                ldr       w8, [x0, x12, lsl #2] // Dual Issue Pair 0, Instruction 1

                ldr       w8, [x2, x13, lsl #2] // Dual Issue Pair 1, Instruction 0
                add       x0, x1, x12           // Dual Issue Pair 1, Instruction 1

                sub       x2, x3, x13          // Dual Issue Pair 2, Instruction 0
                ldr       w8, [x4, x14, lsl #2] // Dual Issue Pair 2, Instruction 1

                ldr       w8, [x6, x15, lsl #2] // Dual Issue Pair 3, Instruction 0
                add       x4, x9, x14          // Dual Issue Pair 3, Instruction 1

                // Group 4
                fmla      v4.4s, v5.4s, v6.4s  // Dual Issue Pair 0, Instruction 0
                ldr       w8, [x0, x16, lsl #2] // Dual Issue Pair 0, Instruction 1

                ldr       w8, [x2, x17, lsl #2] // Dual Issue Pair 1, Instruction 0
                sub       x0, x3, x15         // Dual Issue Pair 1, Instruction 1

                add       x2, x1, x16           // Dual Issue Pair 2, Instruction 0
                ldr       w8, [x4, x18, lsl #2] // Dual Issue Pair 2, Instruction 1

                ldr       w8, [x6, x19, lsl #2] // Dual Issue Pair 3, Instruction 0
                sub       x4, x7, x17          // Dual Issue Pair 3, Instruction 1

                // End of loop check
                subs    x11, x11, #1
                bne     power_loop
		B       test_pass
test_pass:
                MOV  x0, #0x1
                B    test_end
test_end:
        //pop     x0, x1
        pop     x2, x3                          // load the rest of the registers
        pop     x4, x5
        pop     x6, x7
        pop     x8, x9
	pop     x10, x11
        pop     x12, x13
        pop     x14, x15
        pop     x16, x17
        pop     x18, x19
        pop     x20, x21
        pop     x22, x23
        pop     x24, x25
        pop     x26, x27
        pop     x28, x29
        ret 
	
	

//-------------------------------------------------------------------------------
// End of test
//-------------------------------------------------------------------------------

//                // All CPUs except CPU0 enter WFI, while CPU0 ends the test
//	      MOV     x24, #0x0
//              MOV     x22, #0x2
//              mrs     x28, mpidr_el1           // Read MPIDR
//              UBFX    x28, x28, #8, #8 // Cluster ID
//              mov     x21, x28
//              LSL     x21, x21 , x22
//              mrs     x28, mpidr_el1           // Read MPIDR 
//	      UBFX    x28, x28, #0, #8
//	      ORR     x20, x28, x21
//              cbnz    x20, end_wfi
//
//
//              //FT pattern, program the MDM_TM_ENDSIM register for end
//              //simulation
//	      //finish For FT pattern, assert the GPIO
//	        mov  x1, #0x1
//	        ldr  w0, =GPIO
//	        ldr  w2, [x0]
//	        lsl  x1, x1, 23
//	        orr  x2, x2, x1
//                str  w2, [x0]
//	        dsb  sy
//wait_all_wfi:
//		// efuse info
//                MOV     x15, #0x1
//                LDR     x0, =SOC_FUSED
//                LDR     w3, [x0]
//                UBFX    x3, x3, #4, #8
//                EOR     x3, x3, #0xff
//                SUB     x3, x3, x15
//                LDR     x0, =MP0_MISC_CONFIG
//                LDR     w1, [x0]
//                LSR     x1, x1, #24
//                LDR     x0, =MP1_MISC_CONFIG
//                LDR     w2, [x0]
//                LSR     x2, x2, #20
//                ORR     x1, x1, x2
//                AND     x1, x1, #0xff
//                CMP     x1, x3
//                BNE     wait_all_wfi
//
//
//end:           
//              //  mov     w2, #0x04       // EOT
//              //  strb    w2, [x0]
//              //  dsb     sy
//                mov     w2, 0x0
//                ldr     w0, =MDM_TM_ENDSIM
//                str     w2, [x0]
//                dsb     sy
//
//end_wfi:        wfi
//                b       end_wfi

//-------------------------------------------------------------------------------
// Breakpoint and watchpoint labels
//-------------------------------------------------------------------------------

// These labels are used to set the breakpoint and watchpoint registers.
// There is no real code at these labels and they are never executed: the
// breakpoints and watchpoints are set purely to increase power consumption.
bkpt0:          .word   0
bkpt1:          .word   0
bkpt2:          .word   0
bkpt3:          .word   0
bkpt4:          .word   0
bkpt5:          .word   0

// Align watchpoints to double-word boundaries so that all byte address select
// bits can be set in the watchpoint control register
                .balign 8
wpt0:           .word   0
                .word   0
wpt1:           .word   0
                .word   0
wpt2:           .word   0
                .word   0
wpt3:           .word   0
                .word   0

//-------------------------------------------------------------------------------
// Strings
//-------------------------------------------------------------------------------

message:        .asciz "** TEST PASSED OK **\n"
                .balign 4

                .end

