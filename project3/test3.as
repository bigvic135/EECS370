        lw  0   1   pos1
        lw  0   2   neg1
        lw  0   3   count
        nor 1   1   4
loop    beq 0   3   done
        nor 3   3   5
        nor 4   5   5
        beq 0   5   even
        add 6   1   6
        beq 0   0   else
even    add 7   1   7
else    add 3   2   3
        beq 0   0   loop
done    halt
count   .fill 4
pos1    .fill 1
neg1    .fill -1
