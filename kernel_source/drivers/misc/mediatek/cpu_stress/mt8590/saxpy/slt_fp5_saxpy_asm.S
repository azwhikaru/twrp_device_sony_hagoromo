
                .text
.equ ARRAY_SIZE, 208


@                ALIGN   32
                .align 5

@;*****************************************************************************
@;***    start of main test
@;*****************************************************************************
@
@                EXPORT fp5_saxpy_start
                .global fp5_saxpy_start
fp5_saxpy_start:
                STMDB   sp!, {r1-r12}

                @; read CPU ID register
                MRC p15,0,r6,c0,c0,5
                AND r4,r6,#0xf
                mov r5,r6,lsr #8
                and r5,r5,#0xf
                orr r6,r4,r5, lsl  #2
                CMP r0,r6
                BEQ cpu_id_check_done
    
                MOV r0, #0xFFFFFFFF     @target cpu check fail
                LDMIA   sp!, {r1-r12} 
                MOV     pc , r14        @return         
cpu_id_check_done:   

@;code start here
@
@;-------------------------------------------------------------------------------
@; vfp Test
@;-------------------------------------------------------------------------------

saxpy_test_start:
       @; Enable FPU if present
                MOV     r4,#0xf00000               @Write CPACR (Coprocessor Access Control Register)
                MCR     p15,0,r4,c1,c0,2           @to enable coprocessors cp10 & cp11
                MRC     p15,0,r4,c1,c0,2           @Read CPACR
                UBFX    r4, r4, #20, #1            @if bit 20 is zero then so the bits 23:21 so FPU present
                CMP     r4, #1
                BNE     no_fpu

@ Saxpy loop definition
@  Argument from C:
@      r0: int N, array_size
@      r1: float a[]
@      r2: float b[]
@      s0: float constant
                mov r0, #ARRAY_SIZE
saxpy_asm:
                vld1.32  {d2, d3}, [r1]!
                vld1.32  {d4, d5}, [r2]
                vmla.f32 q2, q1, d0[0]
                subs     r0, r0, #4
                vst1.32  {d4, d5}, [r2]!
                bne      saxpy_asm
                B    test_pass

no_fpu:
                MOV  r0, #0x2
		B    test_error

test_error:      
                MOV  r0, #0x0
                B    test_end

test_pass:       
                MOV  r0, #0x1
                B    test_end

test_end:       LDMIA  sp!, {r1-r12} 
                BX lr


                .end
