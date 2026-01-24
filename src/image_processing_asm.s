.global _ImageSingleThreshold_NEON
.align 4


_ImageSingleThreshold_NEON:
    stp x19, x20, [sp, #-32]!

    mov x19, x0     
    mov x20, x1               

    
    dup v30.16b, w2           

    ld1r {v16.16b}, [x3], #1
    ld1r {v17.16b}, [x3], #1
    ld1r {v18.16b}, [x3], #1 
    ld1r {v19.16b}, [x3], #1  

    ld1r {v20.16b}, [x3], #1  
    ld1r {v21.16b}, [x3], #1  
    ld1r {v22.16b}, [x3], #1  
    ld1r {v23.16b}, [x3], #1  

.Lloop_start:
    cmp x20, #16
    blt .Lend

    ld4 {v0.16b, v1.16b, v2.16b, v3.16b}, [x19]

    cmhs v24.16b, v0.16b, v30.16b

    mov v0.16b, v24.16b
    bsl v0.16b, v20.16b, v16.16b

    mov v1.16b, v24.16b
    bsl v1.16b, v21.16b, v17.16b

    mov v2.16b, v24.16b
    bsl v2.16b, v22.16b, v18.16b

    mov v3.16b, v24.16b
    bsl v3.16b, v23.16b, v19.16b

    st4 {v0.16b, v1.16b, v2.16b, v3.16b}, [x19], #64

    sub x20, x20, #16
    b .Lloop_start

.Lend:
    ldp x19, x20, [sp], #32
    ret