![alt text](https://raw.githubusercontent.com/mbaharan/branchExtractor/master/img/TeCSARIcon-1.png)
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
0x00000000004ffcc3	1	0	0	1	              0x47bafd #BASE C3                       ret 
0x00000000004ffcc0	1	1	0	0	              0x4ffccd #BASE 75F1                     jnz 0x4ffcc0
0x000000000047bd10	1	0	1	1	              0x4ffcc0 #BASE FF5010                   call qword ptr [rax+0x10]
0x000000000047be58	0	1	0	0	              0x47bd42 #BASE 0F8510010000             jnz 0x47be58
0x000000000047be80	1	1	0	0	              0x47bd4b #BASE 0F842F010000             jz 0x47be80
0x00000000005128f0	1	0	1	0	              0x47be85 #BASE E8666A0900               call 0x5128f0
0x0000000000512916	1	1	0	0	              0x51290a #BASE 740A                     jz 0x512916
0x0000000000512950	1	1	0	0	              0x512919 #BASE 7435                     jz 0x512950
```

while the latter one contains static information about the executed branches:
```
!!! Number of Instructions = 486684
!!! Number of Unconditional branches = 15027
!!! Number of Conditional branches = 87451
------------------------------------
```

About `branches.out`, the first colomn is the branch taget IP, second column is if it is taken(1) or untaken(0), third one is about if the branch is conditional(1) or unconditional(0), forth is about if it is call(1) instruction or (0), fifth one is if it is direct(0) or indirect branch(1), and sixth one is the branch IP, and after the `#` sign is instruction opcode and disassembled verion.