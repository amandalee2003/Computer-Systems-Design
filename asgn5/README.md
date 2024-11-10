#Assignment 5 directory

This directory contains source code and other files for Assignment 5.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.


Amanda Lee
ammolee@ucsc.edu
cruzid - 1868079
Assignment 5

<h2>Before you Compile</h2><br>
You should include any files it may need corresponding to the cacher.c file before trying to run cacher.c. This is because we want test functions for this file to be able to be called within cacher.c and created from stdin.

<h2>How to run your Program</h2><br>
In order to run the cacher.c file, we must be in the correct directory and first remove and recompile the files by calling make clean and make all from the Makefile before cacher.c (this allows the files to be cleared out and reset). After calling from Makefile, we will call ./cacher -N <size> with the corresponding cache eviction type (-C, -F, -L or none, which is default FIFO) and attached printf in stdin.

<h2>Files and Purpose</h2><br>
cacher.c: The source and main file for the cache eviction program.<br>
Makefile: This file makes/recompiles and clears the files before being ran after new changes.<br>
README.md: This file contains notes about design, testing, and questions.<br>

<h2>Sources</h2><br>
I refered the discord for advice on the .c files with errors I had or advice if my code wasn't running how it should.<br>
I attended office hours as needed with questions that referred to testing my code and errors I couldn't work past.<br>
I used Chat GPT by pasting in a few lines of code and the given error for those lines asking for ideas of how to work past the errors.<br>
I used the given httpserver.c code heavily from Professor Quinn in the helper function file while coding my httpserver.<br>


