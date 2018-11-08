        lw      0   1   one
        lw      0   2   size
loop1   beq     2   3   done
        add     0   0   4
loop2   beq     3   4   incl
        lw      3   6   data
        lw      4   7   data
        beq     6   7   skip
        add     1   5   5
skip    add     1   4   4
        beq     0   0   loop2
incl    add     1   3   3
        beq     0   0   loop1
done    halt
one     .fill   1
size    .fill   3
data    .fill   0
        .fill   1
        .fill   0
