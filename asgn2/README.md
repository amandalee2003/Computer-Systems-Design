#Assignment 2 directory

This directory contains source code and other files for Assignment 2.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

Amanda Lee
ammolee@ucsc.edu
1868079
Assignment 2

<h2>Before you Compile</h2><br>
You should include any files it may need corresponding to the httpserver.c file before trying to run httpserver.c. This is because we want functions from this file to be able to be called within httpserver.c and other needed files being called or created from stdin.

<h2>How to run your Program</h2><br>
In order to run the httpserver.c file, we must be in the correct directory and first remove and recompile the files by calling make clean and make all from the Makefile before calling httpserver.c (this allows the files to be cleared out and reset). After calling from Makefile, we will call ./httpserver <port> with the corresponding desired file and attached string using get or put.

<h2>Command Lines and Purposes</h2><br>
**httpserver.c** <br>
- GET and PUT - formatted as Request-Line\r\n (Header-Field\r\n) \r\n Message-Body is an stdin input from the user.<br>


<h2>Files and Purpose</h2><br>
httpserver.c: The source and main file for the httpserver program.<br>
Makefile: This file makes/recompiles and clears the files before being ran after new changes.<br>
README.md: This file contains notes about design, testing, and questions.<br>

<h2>Sources</h2><br>
I refered the discord for advice on the .c files with errors I had or advice if my code wasn't running how it should.<br>
I attended office hours as needed with questions that referred to testing my code and errors I couldn't work past.<br>
I used Chat GPT by pasting in a few lines of code and the given error for those lines asking for ideas of how to work past the errors.<br>


	

