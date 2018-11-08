        lw      0       1       answer      load reg1 with result
        lw      0       3       mcand       used to add
        lw      0       2       mplier      load reg3
        lw      0       6       lit       load reg4
        lw      0       7       neg1        reg5 value is -1
        lw      0       4       border      load reg6
        lw      0       5       temp         load reg7
loop    beq     5       4       end         branches to next
        nor     3       6       5           nor reg2 and reg4 values to reg7
        beq     0       5       odd         branches to odd
even    add     2       2       2
        add     7       6       6
        add     7       7       7
        beq     0       0       loop
odd     add     2       1       1
        beq     0       0       even
end     halt
mcand .fill 15
mplier .fill 10
neg1 .fill -1
border .fill -32768
lit .fill -2
answer .fill 0
temp .fill 1
