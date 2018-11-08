#!/bin/sh

#  build.sh
#  Project1
#
#  Created by BigVic on 1/31/18.
#  Copyright © 2018 mac. All rights reserved.
gcc -o hello hello.c
if [diff hello.txt <(./hello) == ""]; then
	echo "All tests passed."
else
	echo "Test failed. Expected output >>Hello World!<<,got output >> " ./hello "<<"
fi
