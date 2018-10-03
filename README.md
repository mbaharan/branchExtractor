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
After execution, two log files named `branches_0.out`(default) and `generalInfo_0.out` will be created. The first one containing all the information about branched executed by `ls` application. Following is the sample of output:
```
0x000000000045dda0	1	0	1	0	1	29999829	              0x44f70b #BASE E890E60000               call 0x45dda0
0x000000000045ddd4	1	1	0	0	1	29999841	              0x45ddca #BASE 7408                     jz 0x45ddd4
0x000000000045debb	1	1	0	0	1	29999845	              0x45dddf #BASE 0F8ED6000000             jle 0x45debb
0x000000000045de2b	1	0	0	0	1	29999847	              0x45debe #BASE E968FFFFFF               jmp 0x45de2b
0x000000000044f710	1	0	0	1	0	29999859	              0x45de4c #BASE C3                       ret 
0x000000000044f858	1	1	0	0	1	29999862	              0x44f716 #BASE 0F843C010000             jz 0x44f858
0x000000000044f8c7	1	1	0	0	1	29999865	              0x44f862 #BASE 7463                     jz 0x44f8c7
0x000000000044fa77	0	1	0	0	1	29999869	              0x44f8da #BASE 0F8597010000             jnz 0x44fa77
0x000000000047bde1	1	0	0	1	0	29999877	              0x44f8ee #BASE C3                       ret 
0x0000000000479bc0	1	0	0	0	1	29999880	              0x47bde8 #BASE E9D3DDFFFF               jmp 0x479bc0
```

while the latter one contains static information about the executed branches:
```
!!! Number of Instructions = 486684
!!! Number of Unconditional branches = 15027
!!! Number of Conditional branches = 87451
------------------------------------
```

About `branches_0.out`, the first column is the branch target IP, the second column is `1` if it is taken, the third one is `1` if the branch is conditional, the fourth one is `1` if it is a call instruction, the fifth one is `1` if it is a RET instruction, the sixth one is `1` if it is direct branch, and the seventh one is the total number of instructions seen until the current branch after the defined offset, the eighth on is the address of PC, and after the `#` sign is instruction opcode and disassembled version.

Please have look at following lines in branchExt.cpp to underestand the tools options:

```c++
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "branches", "specifies the output file name prefix.");

KNOB<string> KnobHowManySet(KNOB_MODE_WRITEONCE, "pintool", "b", "1", "Specifies how many set should be created.");

KNOB<string> KnobHowManyBranch(KNOB_MODE_WRITEONCE, "pintool", "m", "30000000", "Specifies how many instructions should be probed.");

KNOB<string> KnobOffset(KNOB_MODE_WRITEONCE, "pintool", "f", "1000000", "Starts saving instructions after seeing the first `f` instruction.");
```