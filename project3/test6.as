        lw      0   1   one
        lw      0   5   size
        lw      0   2   search
loop    beq     5   3   end
        lw      3   4   arr
        beq     2   4   end
        add     3   1   3
        beq     0   0   loop
end     halt
one     .fill   1
arr     .fill   15
        .fill   -4
        .fill   5
        .fill   24
        .fill   13
size    .fill   5
search  .fill   24
