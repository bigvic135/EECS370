		lw		0	1 zero
		lw		0	2 ten
		lw 		0	3 neg1
start	add		3	2 2
        nor     2   1   4
		beq	    1	2	start
		halt
zero	.fill	0
ten		.fill	10
neg1	.fill 	-1
