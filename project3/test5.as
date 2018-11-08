        lw      0   1   poop
        lw      0   2   vape
        lw      0   3   one
outer   noop
        beq     0   1   exit
        noop
        lw      0   2   vape
inner   beq     0   2   done
        add     4   3   4
        add     2   3   2
        beq     0   0   inner
done    add     1   3   1
        beq     0   0   outer
exit    halt
poop    .fill   30
vape    .fill   2
one     .fill   -1
