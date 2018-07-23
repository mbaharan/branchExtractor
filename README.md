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
After execution, two log files named `branches.out`(default) and `generalInfo.out` will be created. The first one containing all the information about branched executed by `ls` application. Following is the sample of output:
```
0x00007f10e673cea0	1	0	1	              0x7f10e673c093 #BASE E8080E0000               call 0x7f10e673cea0
0x00007f10e673cf90	0	1	0	              0x7f10e673ceef #BASE 0F849B000000             jz 0x7f10e673cf90
0x00007f10e673cf5b	1	0	0	              0x7f10e673cf34 #BASE EB25                     jmp 0x7f10e673cf5b
0x00007f10e673cf4b	1	1	0	              0x7f10e673cf5f #BASE 76EA                     jbe 0x7f10e673cf4b
0x00007f10e673cf90	0	1	0	              0x7f10e673cf59 #BASE 7435                     jz 0x7f10e673cf90
0x00007f10e673cf4b	1	1	0	              0x7f10e673cf5f #BASE 76EA                     jbe 0x7f10e673cf4b
```

while the latter one contains static information about the executed branches:
```
!!! Number of Instructions = 486684
!!! Number of Unconditional branches = 15027
!!! Number of Conditional branches = 87451
------------------------------------
```

About `branches.out`, the first colomn is the branch taget IP, second column is if it is taken(1) or untaken(0), third one is about if the branch is conditional(1) or unconditional(0), forth is about if it is call(1) instruction or (0) and fifth one is the branch IP, and after the `#` sign is instruction opcode and disassembled verion.