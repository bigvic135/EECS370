        lw      3       4       done
        lw      2       3       6
        jalr    2       2
        add     4       0       1
Jump    beq     0       0       Loop
Loop    noop
        add     0       3       0
        beq     1       5       Jump
        nor     3       4       7
        nor     3       7       1
done    halt
        noop
