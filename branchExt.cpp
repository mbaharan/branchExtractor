/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2018 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */

/*
    Following code has been modified by "Mohammadreza Baharani"
    UNC, Charlotte, NC, 28223 USA

    This code extracts the number of instruction executed by the processors and logs
    the branches. It produces two log files name `generalInfo.out` consisting
    information about the total number of instuctions and `branches.out`(default)
    consisting following information:
    BRANCH_TARGET `S` `K` `C` BRANCH_ADDR # BASE ?????? 
    where `S` can be `1`(Taken) or `0`(Not taken) and `K` can be `1`(Conditional) or
    `0`(Unconditional), and `C` can be `1`(Call ints.) or `0`(Not call instruction)
    and ?????? is the disassembled instruction and its opcode.
*/


#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>
#include "pin.H"
#include "instlib.H"

#define axuliryFileName "generalInfo"
std::map<ADDRINT, std::string> disAssemblyMap;


static ADDRINT dl_debug_state_Addr = 0;
static ADDRINT dl_debug_state_AddrEnd = 0;
static BOOL justFoundDlDebugState = FALSE;

ofstream OutFile;
ofstream axuFile;
// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;
static UINT64 cbcount = 0;
static UINT64 ubcount = 0;
static UINT64 callcount = 0;
static UINT64 retcount = 0;
static int64_t howManyBranch = 0;
static UINT64 howManySet = 0;
static UINT64 fileCounter = 0;
static UINT64 offset_inst = 0;
static UINT64 first_inst_count_after_offset = 0;
static bool first_record = true;
static bool record = false;
static ostringstream filePrefix;


KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "branches", "specifies the output file name prefix.");

KNOB<string> KnobHowManySet(KNOB_MODE_WRITEONCE, "pintool", "b", "1", "Specifies how many set should be created.");

KNOB<string> KnobHowManyBranch(KNOB_MODE_WRITEONCE, "pintool", "m", "-1", "Specifies how many instructions should be probed. -1 for probing whole program.");

KNOB<string> KnobOffset(KNOB_MODE_WRITEONCE, "pintool", "f", "1000000", "Starts saving instructions after seeing the first `f` instruction.");

VOID write_on_axu(){
    axuFile << "!!! Number of Instructions = " << (icount - offset_inst - ((fileCounter-1)*howManyBranch) + 1) << endl;
    axuFile << "!!! Number of Unconditional branches = " << ubcount << endl;
    axuFile << "!!! Number of Conditional branches = " << cbcount << endl;
    axuFile << "!!! Number of Call branches = " << callcount << endl;
    axuFile << "!!! Number of Ret branches = " << retcount << endl;

    axuFile.close();

}

VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    cout << "Logging data..." << endl;
    write_on_axu();
    OutFile.close();
}

VOID reset_var(){
    cbcount = 0;
    ubcount = 0;
    callcount = 0;
    retcount = 0;
    first_inst_count_after_offset = 0;
}

UINT32 file_init(){
    cout << "Writing " << fileCounter-1 << endl;

    write_on_axu();
    
    OutFile.close();
    filePrefix.str("");
    filePrefix.clear();
    filePrefix << KnobOutputFile.Value() << "_" << fileCounter <<  ".out";
    OutFile.open(filePrefix.str().c_str());
    OutFile.setf(ios::showbase);

    filePrefix.str("");
    filePrefix.clear();
    filePrefix << axuliryFileName << "_" << fileCounter <<  ".out";
    axuFile.open(filePrefix.str().c_str());
    axuFile.setf(ios::showbase);

    reset_var();

    return 0;
}


// This function is called before every instruction is executed
VOID docount() { 
    //cerr<< "I:" << icount << "V:" << (howManyBranch+ offset_inst - 1) << (!((icount) % (howManyBranch+ offset_inst - 1))? "Tr":"Fa") << endl;
    if(howManyBranch > 0){
        if (!((icount) % ((howManyBranch*(fileCounter+1)) + offset_inst - 1)) && icount > 0)
        {
            fileCounter++;
            if (fileCounter > howManySet-1){
                cout << "Exiting because of user conditions"<<endl;
                Fini(0, 0);
                exit(0);
            }else{
                file_init();
            }
        }
    }

    icount++; 
    if (icount >= offset_inst && fileCounter==0){
        first_inst_count_after_offset++; //Although here is going to be incerased by one, it will be set to 1 whenever it reaches to fist branch;
        record = true;
    }else if(fileCounter>0){
        first_inst_count_after_offset++;
    }
}


static char nibble_to_ascii_hex(UINT8 i) {
    if (i<10) return i+'0';
    if (i<16) return i-10+'A';
    return '?';
}

static void print_hex_line(char* buf, const UINT8* array, const int length) {
  int n = length;
  int i=0;
  if (length == 0)
      n = XED_MAX_INSTRUCTION_BYTES;
  for( i=0 ; i< n; i++)     {
      buf[2*i+0] = nibble_to_ascii_hex(array[i]>>4);
      buf[2*i+1] = nibble_to_ascii_hex(array[i]&0xF);
  }
  buf[2*i]=0;
}

static string
disassemble(UINT64 start, UINT64 stop) {
    UINT64 pc = start;
    xed_state_t dstate;
    xed_syntax_enum_t syntax = XED_SYNTAX_INTEL;
    xed_error_enum_t xed_error;
    xed_decoded_inst_t xedd;
    ostringstream os;
    if (sizeof(ADDRINT) == 4)
        xed_state_init(&dstate,
                       XED_MACHINE_MODE_LEGACY_32,
                       XED_ADDRESS_WIDTH_32b,
                       XED_ADDRESS_WIDTH_32b);
    else
        xed_state_init(&dstate,
                       XED_MACHINE_MODE_LONG_64,
                       XED_ADDRESS_WIDTH_64b,
                       XED_ADDRESS_WIDTH_64b);

    /*while( pc < stop )*/ {
        xed_decoded_inst_zero_set_mode(&xedd, &dstate);
        UINT32 len = 15;
        if (stop - pc < 15)
            len = stop-pc;

        xed_error = xed_decode(&xedd, reinterpret_cast<const UINT8*>(pc), len);
        bool okay = (xed_error == XED_ERROR_NONE);
        iostream::fmtflags fmt = os.flags();
        /*os << std::setfill('0')
           << "XDIS "*/
        os << std::hex
           << std::setw(sizeof(ADDRINT)*2)
           << "0x"
           << pc
           << std::dec
           << " #"
           << std::setfill(' ')
           << std::setw(4);

        if (okay) {
            char buffer[200];
            unsigned int dec_len, sp;

            os << xed_extension_enum_t2str(xed_decoded_inst_get_extension(&xedd));
            dec_len = xed_decoded_inst_get_length(&xedd);
            print_hex_line(buffer, reinterpret_cast<UINT8*>(pc), dec_len);
            os << " " << buffer;
            for ( sp=dec_len; sp < 12; sp++)     // pad out the instruction bytes
                os << "  ";
            os << " ";
            memset(buffer,0,200);
            int dis_okay = xed_format_context(syntax, &xedd, buffer, 200, pc, 0, 0);
            if (dis_okay)
                os << buffer << endl;
            else
                os << "Error disasassembling pc 0x" << std::hex << pc << std::dec << endl;
            pc += dec_len;
        }
        else { // print the byte and keep going.
            UINT8 memval = *reinterpret_cast<UINT8*>(pc);
            os << "???? " // no extension
               << std::hex
               << std::setw(2)
               << std::setfill('0')
               << static_cast<UINT32>(memval)
               << std::endl;
            pc += 1;
        }
        os.flags(fmt);
    }
    return os.str();
}

VOID ImageLoad(IMG img, VOID *v)
{

    printf ("ImageLoad %s\n", IMG_Name(img).c_str());
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
             if (strcmp(RTN_Name(rtn).c_str(), "_dl_debug_state") == 0)
             {
                 printf ("  RTN %s at %p\n", RTN_Name(rtn).c_str(),  reinterpret_cast<void *>(RTN_Address(rtn)));
                 printf ("    ** found _dl_debug_state\n");
                 dl_debug_state_Addr = RTN_Address(rtn);
                 justFoundDlDebugState = TRUE;
             }
             else if (justFoundDlDebugState)
             {
                 printf ("  RTN %s at %p\n", RTN_Name(rtn).c_str(),  reinterpret_cast<void *>(RTN_Address(rtn)));
                 dl_debug_state_AddrEnd =  RTN_Address(rtn);
                 justFoundDlDebugState = FALSE;
                 printf ("    ** _dl_debug_state from %p to %p\n", reinterpret_cast<void *>(dl_debug_state_Addr), reinterpret_cast<void *>(dl_debug_state_AddrEnd));
             }
        }
    }
}

/************
 * 
 * JMP segment
 * 
 */ 

static VOID UnconDirectJMP(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    Unc      Call    NotRet     Direct
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t0" << "\t0" << "\t0" << "\t1"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;

        ubcount++;
        
    
}

static VOID UnconUnDirectJMP(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    Unc      Call    NotRet     UnDirect
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t0" << "\t0" << "\t0" << "\t0"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        ubcount++;
    
}

static VOID ConDirectJMP(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    C      Call    NotRet     Direct
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t1" << "\t0" << "\t0" << "\t1"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        cbcount++;
    
}
static VOID ConUnDirectJMP(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    C        Call     NotRet     UNDirect
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t1" << "\t0" << "\t0" << "\t0"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        cbcount++;
    
}
//****************************************************************

/************
 * 
 * Ret segment
 * 
 */ 

static VOID UnconDirectRet(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    Unc      Call    NotRet     Direct
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t0" << "\t0" << "\t1" << "\t1"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        ubcount++;
        retcount++;
    
}

static VOID UnconUnDirectRet(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    Unc      Call    NotRet     UnDirect
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t0" << "\t0" << "\t1" << "\t0"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        ubcount++;
        retcount++;
    
}

static VOID ConDirectRet(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    C      Call    NotRet     Direct
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t1" << "\t0" << "\t1" << "\t1"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        cbcount++;
        retcount++;
    
}
static VOID ConUnDirectRet(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    C        Call     NotRet     UNDirect
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t1" << "\t0" << "\t1" << "\t0"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        cbcount++;
        retcount++;
    
}
//****************************************************************



/************
 * 
 * Call segment
 * 
 */ 

static VOID UnconDirectCall(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    Unc      Call    NotRet     Direct
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t0" << "\t1" << "\t0" << "\t1"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        ubcount++;
        callcount++;
    
}

static VOID UnconUnDirectCall(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    Unc      Call    NotRet     UnDirect
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t0" << "\t1" << "\t0" << "\t0"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        ubcount++;
        callcount++;
    
}

static VOID ConDirectCall(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    C      Call    NotRet     Direct
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t1" << "\t1" << "\t0" << "\t1"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        cbcount++;
        callcount++;
    
}
static VOID ConUnDirectCall(ADDRINT ip, ADDRINT target, BOOL taken)
{//(T-N), (Con-Uncon), (Call-NotCall), (Ret-NotRet), (Direct-NotDirect)

        string s = disassemble ((ip),(ip)+15);
                                                        //T                    C        Call     NotRet     UNDirect
        OutFile <<  reinterpret_cast<void *>(target) << (taken?"\t1":"\t0") << "\t1" << "\t1" << "\t0" << "\t0"<< "\t" <<first_inst_count_after_offset << "\t" << s << flush;
        cbcount++;
        callcount++;
    
}
//****************************************************************


static VOID Instruction(INS ins, VOID *v)
{
    // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);

    if(record){
        if (INS_IsBranchOrCall(ins)){
            if(first_record){ //Detected the first branch
                first_inst_count_after_offset = 1;
                first_record = false;
            }
            if (INS_HasFallThrough(ins) == false){ // It is unconditional branch
                if (INS_IsCall(ins)){ // It is call
                    if(INS_IsDirectBranchOrCall(ins) == true){ //direct
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UnconDirectCall, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }else{
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UnconUnDirectCall, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }
                }
                else if(INS_IsRet(ins)){ // It is RET
                    if(INS_IsDirectBranchOrCall(ins) == true){
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UnconDirectRet, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    } else{
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UnconUnDirectRet, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }
                }else{ // It is JMP 
                    if(INS_IsDirectBranchOrCall(ins) == true){
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UnconDirectJMP, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    } else{
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)UnconUnDirectJMP, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }
                }
            } else {// It is conditional branch
                if (INS_IsCall(ins)){ // It is call
                    if(INS_IsDirectBranchOrCall(ins) == true){ //direct
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ConDirectCall, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }else{
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ConUnDirectCall, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }
                }
                else if(INS_IsRet(ins)){ // It is RET
                    if(INS_IsDirectBranchOrCall(ins) == true){
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ConDirectRet, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    } else{
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ConUnDirectRet, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }
                }else{ // It is JMP 
                    if(INS_IsDirectBranchOrCall(ins) == true){
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ConDirectJMP, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    } else{
                        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ConUnDirectJMP, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
                    }
                }
            }
        }
    }    
    // We do not care about instrunctions that are not branches.
    //else
    //    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AtNonBranch, IARG_INST_PTR, IARG_END);
}


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed and log program branches" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

INT32 InitFile()
{
    filePrefix.str("");
    filePrefix.clear();
    filePrefix << KnobOutputFile.Value() << "_" << fileCounter <<  ".out";
    OutFile.open(filePrefix.str().c_str());
    OutFile.setf(ios::showbase);

    filePrefix.str("");
    filePrefix.clear();
    filePrefix << axuliryFileName << "_" << fileCounter <<  ".out";
    axuFile.open(filePrefix.str().c_str());
    axuFile.setf(ios::showbase);

    howManyBranch = atoi(KnobHowManyBranch.Value().c_str());
    howManySet = atoi(KnobHowManySet.Value().c_str());
    offset_inst = atoi(KnobOffset.Value().c_str());

    /*
    cout << "---------------------" <<KnobHowManyBranch.Value() << endl;
    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile.setf(ios::showbase);
    axuFile.setf(ios::showbase);
    howManyBranch = stoi(KnobHowManyBranch.Value().c_str());
    howManySet = stoi(KnobHowManySet.Value().c_str());
    */
    return 0;
}

int main(INT32 argc, CHAR **argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    InitFile();

    INS_AddInstrumentFunction(Instruction, 0);
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);


    PIN_StartProgram();
    return 0;
}
