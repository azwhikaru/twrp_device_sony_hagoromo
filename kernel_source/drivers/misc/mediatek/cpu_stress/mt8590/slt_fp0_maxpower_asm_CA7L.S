                @AREA INIT, CODE, ALIGN=5
                .text

@-----------------------------------------------------------------------------
@ The confidential and proprietary information contained in this file may
@ only be used by a person authorised under and to the extent permitted
@ by a subsisting licensing agreement from ARM Limited.
@
@            (C) COPYRIGHT 2013 ARM Limited.
@                ALL RIGHTS RESERVED
@
@ This entire notice must be reproduced on all copies of this file
@ and copies of this file may only be made by a person if such person is
@ permitted to do so under the terms of a subsisting license agreement
@ from ARM Limited.
@
@      SVN Information
@
@      Checked In          : $Date: 2013-04-22 15:23:35 +0100 (Mon, 22 Apr 2013) $
@
@      Revision            : $Revision: 244852 $
@
@      Release Information : CORTEXA53-r0p0-00lac0
@
@-------------------------------------------------------------------------------
@ Description:
@
@   Maximum power test
@-------------------------------------------------------------------------------


@ Enable UAL syntax
@.syntax unified

@                .section testcode, "ax", %progbits

                @.global test_start
                @.global fiq_handler

.equ NUM_ITERS, 100

@-------------------------------------------------------------------------------
@ FIQ handler
@
@   This test uses a FIQ to release CPUs from WFI.  The FIQ handler simply
@   clears the FIQ using the trickbox and returns.  Note that all CPUs will
@   end up writing to the trickbox but only the first write will have any
@   effect.
@-------------------------------------------------------------------------------

@ Make fiq_handler visible to linker so that it can override the default handler
@.gitcpu0_waitcpu0_waitlobal fiq_handler
@
@fiq_handler:
@                ldr     r8, =0x13000000         @ Trickbox base
@                str     r8, [r8, #0x10]         @ Clear FIQ (nb. write value ignored)
@                dsb
@
@                @ Poll the ISR register until the FIQ has cleared
@fiq_loop:       mrc     p15, 0, r9, c12, c1, 0  @ r9 == ISR
@                ubfx    r9, r9, #6, #1          @ r9 == ISR.F
@                cmp     r9, #0
@                bne     fiq_loop
@
@                @ Return using ERET.  This test uses WFI spin code (a WFI
@                @ instruction followed by a branch back to the WFI) to force
@                @ the CPUs to remain idle until the test breaks them out of the
@                @ WFI spin.  For FIQ exceptions the LR has an offset of +4
@                @ applied, which means that this return will take us to the
@                @ instruction following the WFI spin branch and hence break us
@                @ out of the WFI spin.
@                eret
@

@-------------------------------------------------------------------------------
@ Main test code
@-------------------------------------------------------------------------------

@                @ Set up a FIQ to wake all CPUs from WFI.  When the other CPUs
@                @ wake they will branch to test_start, so the following code
@                @ is only executed by CPU0.
@                mrc     p15, 0, r8, c0, c0, 5   @ Read MPIDR
@                ands    r8, r8, #0xFF           @ r0 == CPU number
@                bne     setup
@                ldr     r9, =0x13000000         @ Trickbox base
@                mov     r10, #0x400             @ FIQ schedule value
@                str     r10, [r9, #0x8]         @ Store to FIQ counter
@                dsb
@
@                @ CPU0 waits here for the interrupt.  Other CPUs are already
@                @ waiting in WFI in the boot code.  After the interrupt all CPUs
@                @ will execute the setup code below (CPUs 1-3 will first branch
@                @ to test_start and then the setup code).
@cpu0_wait:      wfi
@                b       cpu0_wait
@setup:
    .extern pMaxPowerTestMem       
test_mem:
    .word pMaxPowerTestMem
    .equ test_pattern,0x5A5A5A5A
                .align 5
    
.global fp0_maxpower_start
fp0_maxpower_start:
		STMDB   sp!, {r1-r12}
                @ Enable invasive debug by setting the DBGDSCR.HDBGen bit
                @ prior to enabling watchpoints and breakpoints.  Enabling
                @ watchpoints and breakpoints increases power consumption
                @ because the comparators have to compare addresses against
                @ the watchpoints/breakpoint registers.
                mrc     p14, 0, r0, c0, c2, 2   @ r0 == DBGDSCR
                orr     r0, r0, #(0x1 << 14)    @ Set HDBGen
                mcr     p14, 0, r0, c0, c2, 2   @ Write DBGDSCR

                @ Clear the OS lock to enable programming of the breakpoint
                @ and watchpoint registers (writing any value other than
                @ 0xC5ACCE55 clears the lock.)
                mov     r0, #0
                mcr     p14, 0, r0, c1, c0, 4   @ Write DBGOSLAR

                @ Set breakpoint and watchpoint value registers
                ldr     r0, =bkpt0
                mcr     p14, 0, r0, c0, c0, 4   @ Write DBGBVR0
                ldr     r0, =bkpt1
                mcr     p14, 0, r0, c0, c1, 4   @ Write DBGBVR1
                ldr     r0, =bkpt2
                mcr     p14, 0, r0, c0, c2, 4   @ Write DBGBVR2
                ldr     r0, =bkpt3
                mcr     p14, 0, r0, c0, c3, 4   @ Write DBGBVR3
                ldr     r0, =bkpt4
                mcr     p14, 0, r0, c0, c4, 4   @ Write DBGBVR4
                ldr     r0, =bkpt5
                mcr     p14, 0, r0, c0, c5, 4   @ Write DBGBVR5
                ldr     r0, =wpt0
                mcr     p14, 0, r0, c0, c0, 6   @ Write DBGWVR0
                ldr     r0, =wpt1
                mcr     p14, 0, r0, c0, c1, 6   @ Write DBGWVR1
                ldr     r0, =wpt2
                mcr     p14, 0, r0, c0, c2, 6   @ Write DBGWVR2
                ldr     r0, =wpt3
                mcr     p14, 0, r0, c0, c3, 6   @ Write DBGWVR3

                @ Set breakpoint and watchpoint enable registers to enable
                @ in all states.  The value written to the DBGBCRn registers
                @ is:
                @   Bit[13]   (HMC) - 1'b1    - State matching (match all states)
                @   Bits[8:5] (BAS) - 4'b1111 - Match A32/A64 instructions
                @   Bits[2:1] (PMC) - 2'b11   - State matching (match all states)
                @   Bit[0]    (E)   - 1'b1    - Enable this breakpoint
                ldr     r0, =0x21E7             @ DBGBCRn value
                mcr     p14, 0, r0, c0, c0, 5   @ Write DBGBCR0
                mcr     p14, 0, r0, c0, c1, 5   @ Write DBGBCR1
                mcr     p14, 0, r0, c0, c2, 5   @ Write DBGBCR2
                mcr     p14, 0, r0, c0, c3, 5   @ Write DBGBCR3
                mcr     p14, 0, r0, c0, c4, 5   @ Write DBGBCR4
                mcr     p14, 0, r0, c0, c5, 5   @ Write DBGBCR5

                @ The value written to the DBGWCRn registers is:
                @   Bit[13]    (HMC) - 1'b1  - State matching (match all states)
                @   Bits[12:5] (BAS) - 8'hFF - Watch all byte lanes
                @   Bits[2:1]  (PAC) - 2'b11 - State matching (match all states)
                @   Bit[0]     (E)   - 1'b1  - Enable this watchpoint
                ldr     r0, =0x3FE7             @ DBGWCRn value
                mcr     p14, 0, r0, c0, c0, 7   @ Write DBGWCR0
                mcr     p14, 0, r0, c0, c1, 7   @ Write DBGWCR1
                mcr     p14, 0, r0, c0, c2, 7   @ Write DBGWCR2
                mcr     p14, 0, r0, c0, c3, 7   @ Write DBGWCR3

                LDR     r9,test_mem
                LDR     r9,[r9]
                ADD     r9,r9,#0x1000


                @ Set initial register values
                @ldr     r0, =0x00000000
                ldr     r1, =0xFFFFFFFD
                @ldr     r2, =0xFFFFFFFC
                ldr     r3, =0x00000006
                @ldr     r4, =0x00000000
                ldr     r5, =0xFFFFFFFD
                @ldr     r6, =0xFFFFFFFC
                ldr     r7, =0x00000006

                ADD     r0, r9, #0x00000000
                ADD     r10, r9, #0xfffffffd
                ADD     r2, r9, #0xfffffffc
                ADD     r12, r9, #0x00000006
                ADD     r4, r9, #0x00000000
                ADD     r6, r9, #0xfffffffc
                ADD     r9, r9, #0xfffffffd
                
                vmov.i8 q1, #0x66  @ q1 = 0x66666666
                vmov.i8 q2, #0xAA  @ q2 = 0xAAAAAAAA
                vmov.i8 q3, #0x66  @ q3 = 0x66666666
                vmov.i8 q4, #0xAA  @ q4 = 0xAAAAAAAA
                vmov.i8 q5, #0x66  @ q5 = 0x66666666
                vmov.i8 q6, #0xAA  @ q6 = 0xAAAAAAAA

                @ Now CPU0 programs the trickbox to schedule another FIQ and
                @ all CPUs enter WFI.  The CPUs will then all wake and execute
                @ the power loop at the same time.
@                cmp     r8, #0
@                bne     finish_setup
@                str     r10, [r9, #0x8]         @ Setup FIQ counter as before
@                dsb

finish_setup:
                @ Finish setting up registers before entering WFI
@                mov     r9,  #0x007 @remove due to porting
@                mov     r10, #0x005 @remove due to porting
                 mov     r11, #NUM_ITERS
@                mov     r12, #0x001 @remove due to porting
@all_wait:       wfi
@                b       all_wait

                @ On return from the FIQ that releases us from WFI branch
                @ straight to the power loop.  There may be some padding before
                @ the loop in order to align the code in the loop.
                b       power_loop

@-------------------------------------------------------------------------------
@ Main power loop
@
@ The intent is for the add/sub instructions to run along behind and recreate
@ r0, r2 and r4 using the shifter to create the +0x3 or -0xA
@-------------------------------------------------------------------------------

                @ Align the loop to an 8-byte boundary so that two instructions
                @ will be in the same instruction cache data bank.  This allows
                @ us to dual-issue all pairs of instructions in the loop for
                @ maximum power in the pipeline.
                .balign 8
power_loop:
                @ Group 1
                vfma.f32  Q1,Q2,Q3             @ Dual Issue Pair 0, Instruction 0
                ldr       r8, [r0,+r1,LSL #1]! @ Dual Issue Pair 0, Instruction 1

                ldr       r8, [r2,+r3]!        @ Dual Issue Pair 1, Instruction 0
                add       r0, r10, #0x003  @ Dual Issue Pair 1, Instruction 1

                sub       r2, r12, #0x00a @ Dual Issue Pair 2, Instruction 0
                ldr       r8, [r4,+r5,LSL #1]! @ Dual Issue Pair 2, Instruction 1

                ldr       r8, [r6,+r7]         @ Dual Issue Pair 3, Instruction 0
                add       r4, r9, #0x003  @ Dual Issue Pair 3, Instruction 1

                @ Group 2
                vfma.f32  Q4,Q5,Q6             @ Dual Issue Pair 0, Instruction 0
                ldr       r8, [r0,+r1,LSL #1]! @ Dual Issue Pair 0, Instruction 1

                ldr       r8, [r2,+r3]!        @ Dual Issue Pair 1, Instruction 0
                add       r0, r10, #0x003  @ Dual Issue Pair 1, Instruction 1

                sub       r2, r12, #0x00a @ Dual Issue Pair 2, Instruction 0
                ldr       r8, [r4,+r5,LSL #1]! @ Dual Issue Pair 2, Instruction 1

                ldr       r8, [r6,+r7]         @ Dual Issue Pair 3, Instruction 0
                add       r4, r9, #0x003  @ Dual Issue Pair 3, Instruction 1

                @ Group 3
                vfma.f32  Q1,Q2,Q3             @ Dual Issue Pair 0, Instruction 0
                ldr       r8, [r0,+r1,LSL #1]! @ Dual Issue Pair 0, Instruction 1

                ldr       r8, [r2,+r3]!        @ Dual Issue Pair 1, Instruction 0
                add       r0, r10, #0x003  @ Dual Issue Pair 1, Instruction 1

                sub       r2, r12, #0x00a @ Dual Issue Pair 2, Instruction 0
                ldr       r8, [r4,+r5,LSL #1]! @ Dual Issue Pair 2, Instruction 1

                ldr       r8, [r6,+r7]         @ Dual Issue Pair 3, Instruction 0
                add       r4, r9, #0x003  @ Dual Issue Pair 3, Instruction 1

                @ Group 4
                vfma.f32  Q4,Q5,Q6             @ Dual Issue Pair 0, Instruction 0
                ldr       r8, [r0,+r1,LSL #1]! @ Dual Issue Pair 0, Instruction 1

                ldr       r8, [r2,+r3]!        @ Dual Issue Pair 1, Instruction 0
                add       r0, r10, #0x003  @ Dual Issue Pair 1, Instruction 1

                sub       r2, r12, #0x00a @ Dual Issue Pair 2, Instruction 0
                ldr       r8, [r4,+r5,LSL #1]! @ Dual Issue Pair 2, Instruction 1

                ldr       r8, [r6,+r7]         @ Dual Issue Pair 3, Instruction 0
                add       r4, r9, #0x003  @ Dual Issue Pair 3, Instruction 1

                @ End of loop check
                subs    r11, r11, #1
                bne     power_loop
                B       test_pass



test_error:      
                ISB
                MOV  r0, #0x0
                B    test_end

test_pass:
                MOV  r0, #0x1
                B    test_end

test_end:
                LDMIA  sp!, {r1-r12}
                BX lr

@-------------------------------------------------------------------------------
@ End of test
@-------------------------------------------------------------------------------

@                @ All CPUs except CPU0 enter WFI, while CPU0 ends the test
@                mrc     p15, 0, r0, c0, c0, 5   @ Read MPIDR
@                ands    r0, r0, #0xFF           @ r0 == CPU number
@                bne     end_wfi
@
@                ldr     r0, =0x13000000 @ Tube address
@                ldr     r1, =message
@print_loop:     ldrb    r2, [r1], #1
@                cmp     r2, #0
@                beq     end
@                strb    r2, [r0]
@                b       print_loop
@
@end:            mov     r2, #0x04       @ EOT
@                strb    r2, [r0]
@                dsb
@end_wfi:        wfi
@                b       end_wfi

@-------------------------------------------------------------------------------
@ Breakpoint and watchpoint labels
@-------------------------------------------------------------------------------

@ These labels are used to set the breakpoint and watchpoint registers.
@ There is no real code at these labels and they are never executed: the
@ breakpoints and watchpoints are set purely to increase power consumption.
bkpt0:          .word   0
bkpt1:          .word   0
bkpt2:          .word   0
bkpt3:          .word   0
bkpt4:          .word   0
bkpt5:          .word   0

@ Align watchpoints to double-word boundaries so that all byte address select
@ bits can be set in the watchpoint control register
                .balign 8
wpt0:           .word   0
                .word   0
wpt1:           .word   0
                .word   0
wpt2:           .word   0
                .word   0
wpt3:           .word   0
                .word   0

@-------------------------------------------------------------------------------
@ Strings
@-------------------------------------------------------------------------------

message:        .asciz "** TEST PASSED OK **\n"
                .balign 4

                .end

