# Branch Extractor

This software logs the trace of branche executed during a specific benchmark. This software uses Intel PIN tools for instrumenting the retired instructions.

## Installation
Download Intel PIN tools from [here](https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads) and extract it to your prefered directory (mine is '/home/reza/WorkingDir/Arch/pin-3.7'). Also, clone this project.
```sh
$ export PIN_ROOT="/home/reza/WorkingDir/Arch/pin-3.7"
$ cd ~/WorkingDir/Arch/branchExtractor/
$ make
```
If the compilation finished succefully, a file named `obj-intel64/branchExt.so` should be created inside the working folder.
## How to use
This is how the tool should be called
```sh
$ /home/reza/WorkingDir/Arch/pin-3.7/pin -t ~/WorkingDir/Arch/branchExtractor/obj-intel64/branchExt.so -t branches.out -- ls
```
After execution, `branches.out` is created containing all the information about branched executed by `ls` application. Following is the sample of output:
```
!!! Number of Instructions = 486684
!!! Number of Unconditional branches = 15027
!!! Number of Conditional branches = 87451
------------------------------------
0x00007fb07a2aeea0	T	U	              0x7fb07a2ae093 #BASE E8080E0000               call 0x7fb07a2aeea0
0x00007fb07a2aef90	N	C	              0x7fb07a2aeeef #BASE 0F849B000000             jz 0x7fb07a2aef90
0x00007fb07a2aef5b	T	U	              0x7fb07a2aef34 #BASE EB25                     jmp 0x7fb07a2aef5b
0x00007fb07a2aef4b	T	C	              0x7fb07a2aef5f #BASE 76EA                     jbe 0x7fb07a2aef4b
0x00007fb07a2aef90	N	C	              0x7fb07a2aef59 #BASE 7435                     jz 0x7fb07a2aef90
0x00007fb07a2aef4b	T	C	              0x7fb07a2aef5f #BASE 76EA                     jbe 0x7fb07a2aef4b
0x00007fb07a2aef90	N	C	              0x7fb07a2aef59 #BASE 7435                     jz 0x7fb07a2aef90
```
The first three line is the information about the number of instructions, conditional and unconditional branches. After that, the first colomn is the branch taget IP, second column is if it is taken(T) or untaken(U), third one is about if the branch is conditional or unconditional, forth one is the branch IP, and after the `#` sign is instruction opcode and disassembled verion.