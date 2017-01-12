# floorplan
This project makes use of the HotSpot thermal tool version 5.02.


Environment required: 

	linux.

Input:

	.txt file as described in the project description pdf.

Output:

	1. Maximum chip temperature, displayed on the terminal.
	
	2. corresponding floor plan file 'xxx.txt'.


To compile and execute:

	1. Put all source files and input file in the same directory with HotSpot-5.02 folder so that the hotpost lib file path is './HotSpot-5.02'.
	
	2. 'cd HotSpot-5.02' and 'make' to get the lib file we need if you haven't done so before.
	
	3. return to the source files' directory. A Makefile is provided, so just type 'make' to compile.
	
	4. Type './floorplan' to execute.

