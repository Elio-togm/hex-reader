#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// All binary instructions for MIPS assembly code are 32 bits long.
struct BinaryInstructionR {
    uint8_t opcode = 0;        // 6 bits
    uint8_t source_register1;  // 5 bits
    uint8_t source_register2;  // 5 bits
    uint8_t target_register;   // 5 bits
    uint8_t shift_value;       // 5 bits
    uint8_t function_code;     // 6 bits
};

struct BinaryInstructionI {
    uint8_t opcode;            // 6 bits
    uint8_t source_register;   // 5 bits
    uint8_t target_register;   // 5 bits
    uint16_t immediate_value;  // 16 bits
};

struct BinaryInstructionJ {
    uint8_t opcode;           // 6 bits
    uint32_t pseudo_address;  // 26 bits
};

struct BinaryInstructionUniversal {
    char instruction_type;
    uint32_t instruction;  // The baseline instruction that everything should reference
    BinaryInstructionR register_instruction = {
        static_cast<uint8_t>(instruction >> 26),          // opcode
        static_cast<uint8_t>((instruction << 6) >> 27),   // source register 1
        static_cast<uint8_t>((instruction << 11) >> 27),  // source register 2
        static_cast<uint8_t>((instruction << 16) >> 27),  // target register
        static_cast<uint8_t>((instruction << 21) >> 27),  // shift value
        static_cast<uint8_t>((instruction << 26) >> 26)   // function code
    };

    BinaryInstructionI immediate_instruction = {
        static_cast<uint8_t>(instruction >> 26),            // opcode
        static_cast<uint8_t>((instruction << 6) >> 26),     // source register
        static_cast<uint8_t>((instruction << 11) >> 27),    // target register
        static_cast<uint16_t>((instruction << 16) >> 16)     // immediate value
    };

    BinaryInstructionJ jump_instruction = {
        static_cast<uint8_t>(instruction >> 26),            // opcode
        (instruction << 6) >> 6                             // psuedo address
    };
};

//? THE FOLLOWING SECTION IS INTEGER - OPCODE KEY - VALUE PAIRS

// The Standard Opcodes and their integer representations
const std::map<int, std::string> STANDARD_INSTRUCTIONS{
    {0, "Register"},
    {1, "Register Immediate"},
    {2, "J"},
    {3, "JAL"},
    {4, "BEQ"},
    {5, "BNE"},
    {6, "BLEZ"},
    {7, "BGTZ"},
    {8, "ADDI"},
    {9, "ADDIU"},
    {10, "SLTI"},
    {11, "SLTIU"},
    {12, "ANDI"},
    {13, "ORI"},
    {14, "XORI"},
    {15, "LUI"},
    {16, "System Control Coprocessor"},
    {17, "Floating Point Single Precision Coprocessor"},
    {18, "Vector Point Unit Coprocessor"},
    {20, "BEQL"},
    {21, "BNEL"},
    {22, "BLEZL"},
    {23, "BGTZL"},
    {24, "DADDI"},
    {25, "DADDIU"},
    {26, "LDL"},
    {27, "LDR"},
    {28, "Multimedia Extensions"},
    {30, "LQ"},
    {31, "SQ"},
    {32, "LB"},
    {33, "LH"},
    {34, "LWL"},
    {35, "LW"},
    {36, "LBU"},
    {37, "LHU"},
    {38, "LWR"},
    {39, "LWU"},
    {40, "SB"},
    {41, "SH"},
    {42, "SWL"},
    {43, "SW"},
    {44, "SDL"},
    {45, "SDR"},
    {46, "SWR"},
    {47, "CACHE"},
    {49, "LWC1"},
    {51, "PREF"},
    {54, "LQC2"},
    {55, "LD"},
    {57, "SWC1"},
    {62, "SQC2"},
    {63, "SD"},
};

// The Register-Type Opcodes
const std::map<int, std::string> REGISTER_INSTRUCTIONS{
    {0, "SLL"},     {2, "SRL"},     {3, "SRA"},     {4, "SLLV"},     {6, "SRLV"},   {7, "SRAV"},   {8, "JR"},
    {9, "JALR"},    {10, "MOVZ"},   {11, "MOVN"},   {12, "SYSCALL"}, {13, "BREAK"}, {15, "SNYC"},  {16, "MFHI"},
    {17, "MTHI"},   {18, "MFLO"},   {19, "MTLO"},   {20, "DSLLV"},   {22, "DSRLV"}, {23, "DSRAV"}, {24, "MULT"},
    {25, "MULTU"},  {26, "DIV"},    {27, "DIVU"},   {32, "ADD"},     {33, "ADDU"},  {34, "SUB"},   {35, "SUBU"},
    {36, "AND"},    {37, "OR"},     {38, "XOR"},    {39, "NOR"},     {40, "MFSA"},  {41, "MTSA"},  {42, "SLT"},
    {43, "SLTU"},   {44, "DADD"},   {45, "DADDU"},  {46, "DSUB"},    {47, "DSUBU"}, {48, "TGE"},   {49, "TGEU"},
    {50, "TLT"},    {51, "TLTU"},   {52, "TEQ"},    {54, "TNE"},     {56, "DSLL"},  {58, "DSRL"},  {59, "DSRA"},
    {60, "DSLL32"}, {62, "DSRL32"}, {63, "DSRA32"},
};

// The Register Immediate Opcodes
const std::map<int, std::string> REGIMM_INSTRUCTIONS{
    {0, "BLTZ"},     {1, "BGEZ"},     {2, "BLTZL"},  {3, "BGEZL"},  {8, "TGEI"},    {9, "TEGIU"},
    {10, "TLTI"},    {11, "TLTIU"},   {12, "TEQI"},  {14, "TNEI"},  {16, "BLTZAL"}, {17, "BGEZAL"},
    {18, "BLTZALL"}, {19, "BGEZALL"}, {24, "MTSAB"}, {25, "MTSAH"},
};

// The Multimedia Extension Opcodes
const std::map<int, std::string> MULTIMEDIA_INSTRUCTIONS{
    {0, "MADD"},   {1, "MADDU"},  {4, "PLZCW"},   {8, "MMI0 Group"},  {9, "MMI2 Group"},  {16, "MFHI1"},
    {17, "MTHI1"}, {18, "MFLO1"}, {19, "MTLO1"},  {24, "MULT1"},      {25, "MULTU1"},     {26, "DIV1"},
    {27, "DIVU1"}, {32, "MADD1"}, {33, "MADDU1"}, {40, "MMI1 Group"}, {41, "MMI3 Group"}, {44, "PSLLH"},
    {46, "PSRLH"}, {47, "PSRAH"}, {60, "PSLLW"},  {62, "PSRLW"},      {63, "PSRAW"},
};

// The Multimedia Group 0 Opcodes
const std::map<int, std::string> MULTIMEDIA_GROUP_0{
    {0, "PADDW"},   {1, "PSUBW"},   {2, "PCGTW"},   {3, "PMAXW"},   {4, "PADDH"},   {5, "PSUBH"},   {6, "PCGTH"},
    {7, "PMAXH"},   {8, "PADDB"},   {9, "PSUBB"},   {10, "PCGTB"},  {16, "PADDSW"}, {17, "PSUBSW"}, {18, "PEXTLW"},
    {19, "PPACW"},  {20, "PADDSH"}, {21, "PSUBSH"}, {22, "PEXTLH"}, {23, "PPACH"},  {24, "PADDSB"}, {25, "PSUBSB"},
    {26, "PEXTLB"}, {27, "PPACB"},  {30, "PEXT5"},  {31, "PPAC5"},
};

// The Multimedia Group 1 Opcodes
const std::map<int, std::string> MULTIMEDIA_GROUP_1{
    {1, "PABSW"},   {2, "PCEQW"},   {3, "PMINW"},   {4, "PADSBH"},  {5, "PABSH"},   {6, "PCEQH"},
    {7, "PMINH"},   {10, "PCEQB"},  {16, "PADDUW"}, {17, "PSUBUW"}, {18, "PEXTUW"}, {20, "PADDUH"},
    {21, "PSUBUH"}, {22, "PEXTUH"}, {24, "PADDUB"}, {25, "PSUBUB"}, {26, "PEXTUB"}, {27, "QFSRV"},
};

// The Multimedia Group 2 Opcodes
const std::map<int, std::string> MULTIMEDIA_GROUP_2{
    {0, "PMADDW"},  {2, "PSLLVW"}, {3, "PSRLVW"},  {4, "PMSUBW"},  {5, "PMFLO"},   {6, "PINTH"},  {8, "PMULTW"},
    {9, "PDIVW"},   {10, "CPLYD"}, {12, "PMADDH"}, {13, "PHMADH"}, {14, "PAND"},   {15, "PXOR"},  {16, "PMSUBH"},
    {17, "PHMSBH"}, {26, "PEXEH"}, {27, "PREVH"},  {28, "PMULTH"}, {29, "PDIVBW"}, {30, "PEXEW"}, {31, "PROT3W"},
};

// The Multimedia Group 3 Opcodes
const std::map<int, std::string> MULTIMEDIA_GROUP_3{
    {0, "PMADDUW"}, {3, "PSRAVW"}, {8, "PMTHI"}, {9, "PMTLO"},  {10, "PINTEH"}, {12, "PMULTUW"}, {13, "PDIVUW"},
    {14, "PCPYUD"}, {18, "POR"},   {19, "PNOR"}, {26, "PEXCH"}, {27, "PCPYH"},  {30, "PEXCW"},
};

// The System Control Coprocessor Opcodes
const std::map<int, std::string> COPROCESSOR_0_INSTRUCTIONS{
    {0, "MFC0"},
    {4, "MTC0"},
    {8, "Branch on Coprocessor 0"},
    {16, "Translation Lookaside Buffer/Exceptions"},
};

// The Branchon Coprocessor 0 Opcodes
const std::map<int, std::string> BC0_INSTRUCTIONS{
    {0, "BC0F"},
    {1, "BC0T"},
    {2, "BC0FL"},
    {3, "BC0TL"},
};

// The Translation Lookaside Buffer/Exception Opcodes
const std::map<int, std::string> TLB_EXCEPTION_INSTRUCTIONS{
    {1, "TLBR"}, {2, "TLBWI"}, {6, "TLBWR"}, {8, "TLBP"}, {24, "ERET"}, {56, "EI"}, {57, "DI"},
};

// The Floating Point Unit Opcodes
const std::map<int, std::string> COPROCESSOR_1_INSTRUCTIONS{
    {0, "MFC1"},
    {2, "CFC1"},
    {4, "MTC1"},
    {6, "CTC1"},
    {8, "Branch on Coprocessor 1"},
    {16, "Floating Point Unit Single Precision"},
    {20, "Floating Point Unit Word"},
};

// The Branch on Coprocessor 1 Opcodes
const std::map<int, std::string> BC1_INSTRUCTIONS{
    {0, "BC1F"},
    {1, "BC1T"},
    {2, "BC1FL"},
    {3, "BC1TL"},
};

// The Single-Precision Floating Point Unit Opcodes
const std::map<int, std::string> FPU_S_INSTRUCTIONS{
    {0, "ADD.S"},   {1, "SUB.S"},   {2, "MUL.S"},    {3, "DIV.S"},    {4, "SQRT.S"},  {5, "ABS.S"},
    {6, "MOV.S"},   {7, "NEG.S"},   {22, "RSQRT.S"}, {24, "ADDA.S"},  {25, "SUBA.S"}, {26, "MULA.S"},
    {28, "MADD.S"}, {29, "MSUB.S"}, {30, "MADDA.S"}, {31, "MSUBA.S"}, {40, "MAX.S"},  {41, "MIN.S"},
    {48, "C.F"},    {50, "C.EQ"},   {52, "C.LT"},    {54, "C.LE"},
};

// The Word Fixed-Point Floating Point Unit Opcodes
const std::map<int, std::string> FPU_W_INSTRUCTIONS{
    {32, "CVT.S"},
};

// The Vector Processing Unit Opcodes
const std::map<int, std::string> COPROCESSOR_2_INSTRUCTIONS{
    {1, "QMFC2"}, {2, "CFC2"}, {5, "QMTC2"}, {6, "CTC2"}, {8, "Branch on Coprocessor 2"}, {16, "Special 1"},
};

// The Branch on Coprocessor 2 Opcodes
const std::map<int, std::string> BC2_INSTRUCTIONS{
    {0, "BC2F"},
    {1, "BC2T"},
    {2, "BC2FL"},
    {3, "BC2TL"},
};

// The VPU 1st Extension Opcodes
const std::map<int, std::string> COP2_SPECIAL1_INSTRUCTIONS{
    {0, "VADDx"},   {1, "VADDy"},   {2, "VADDz"},   {3, "VADDw"},   {4, "VSUBx"},    {5, "VSUBy"},    {6, "VSUBz"},
    {7, "VSUBw"},   {8, "VMADDx"},  {9, "VMADDy"},  {10, "VMADDz"}, {11, "VMADDw"},  {12, "VMSUBx"},  {13, "VMSUBy"},
    {14, "VMSUBz"}, {15, "VMSUBw"}, {16, "VMAXx"},  {17, "VMAXy"},  {18, "VMAXz"},   {19, "VMAXw"},   {20, "VMINIx"},
    {21, "VMINIy"}, {22, "VMINIz"}, {23, "VMINIw"}, {24, "VMULx"},  {25, "VMULy"},   {26, "VMULz"},   {27, "VMULw"},
    {28, "VMULq"},  {29, "VMAXi"},  {30, "VMULi"},  {31, "VMINIi"}, {32, "VADDq"},   {33, "VMADDq"},  {34, "VADDi"},
    {35, "VMADDi"}, {36, "VSUBq"},  {37, "VMSUBq"}, {38, "VSUBi"},  {39, "VMSUBi"},  {40, "VADD"},    {41, "VMADD"},
    {42, "VMUL"},   {43, "VMAX"},   {44, "VSUB"},   {45, "VMSUB"},  {46, "VOPMSUB"}, {47, "VMINI"},   {48, "VIADD"},
    {49, "VISUB"},  {50, "VIADDI"}, {52, "VIAND"},  {53, "VIOR"},   {56, "VCALLMS"}, {57, "CALLMSR"}, {60, "Special 2"},
};

// The VPU 2nd Extension Opcodes
const std::map<int, std::string> COP2_SPECIAL2_INSTRUCTIONS{
    {0, "VADDAx"},   {1, "VADDAy"},   {2, "VADDAz"},   {3, "VADDAw"},   {4, "VSUBAx"},   {5, "VSUBAy"},
    {6, "VSUBAz"},   {7, "VSUBAw"},   {8, "VMADDAx"},  {9, "VMADDAy"},  {10, "VMADDAz"}, {11, "VMADDAw"},
    {12, "VMSUBAx"}, {13, "VMSUBAy"}, {14, "VMSUBAz"}, {15, "VMSUBAw"}, {16, "VITOF0"},  {17, "VITOF4"},
    {18, "VITOF12"}, {19, "VITOF15"}, {20, "VFTOI0"},  {21, "VFTOI4"},  {22, "VFTOI12"}, {23, "VFTOI15"},
    {24, "VMULAx"},  {25, "VMULAy"},  {26, "VMULAz"},  {27, "VMULAw"},  {28, "VMULAq"},  {29, "VABS"},
    {30, "VMULAi"},  {31, "VCLIPw"},  {32, "VADDAq"},  {33, "VMADDAq"}, {34, "VADDAi"},  {35, "VMADDAi"},
    {36, "VSUBAq"},  {37, "VMSUBAq"}, {38, "VSUBAi"},  {39, "VMSUBAi"}, {40, "VADDA"},   {41, "VMADDA"},
    {42, "VMULA"},   {44, "VSUBA"},   {45, "VMSUBA"},  {46, "VOPMULA"}, {47, "VNOP"},    {48, "VMOVE"},
    {49, "VMR32"},   {52, "VLQI"},    {53, "VSQI"},    {54, "VLQD"},    {55, "VSQD"},    {56, "VDIV"},
    {57, "VSQRT"},   {58, "VRSQRT"},  {59, "VWAITQ"},  {60, "VMTIR"},   {61, "VMFIR"},   {62, "VILWR"},
    {63, "VISWR"},   {64, "VRNEXT"},  {65, "VRGET"},   {66, "VRINIT"},  {67, "VRXOR"},
};

//? Enumeration keyword for all above instructions

// The Standard Opcodes and their integer representations
enum class STANDARD_INSTRUCTIONS : uint32_t {
    REGISTER = 0,
    REGISTER_IMMEDIATE = 1,
    J = 2,
    JAL = 3,
    BEQ = 4,
    BNE = 5,
    BLEZ = 6,
    BGTZ = 7,
    ADDI = 8,
    ADDIU = 9,
    SLTI = 10,
    SLTIU = 11,
    ANDI = 12,
    ORI = 13,
    XORI = 14,
    LUI = 15,
    SYSTEM_CONTROL_COPROCESSOR = 16,
    FLOATING_POINT_SINGLE_PRECISION_COPROCESSOR = 17,
    VECTOR_POINT_UNIT_COPROCESSOR = 18,
    BEQL = 20,
    BNEL = 21,
    BLEZL = 22,
    BGTZL = 23,
    DADDI = 24,
    DADDIU = 25,
    LDL = 26,
    LDR = 27,
    MULTIMEDIA_EXTENSIONS = 28,
    LQ = 30,
    SQ = 31,
    LB = 32,
    LH = 33,
    LWL = 34,
    LW = 35,
    LBU = 36,
    LHU = 37,
    LWR = 38,
    LWU = 39,
    SB = 40,
    SH = 41,
    SWL = 42,
    SW = 43,
    SDL = 44,
    SDR = 45,
    SWR = 46,
    CACHE = 47,
    LWC1 = 49,
    PREF = 51,
    LQC2 = 54,
    LD = 55,
    SWC1 = 57,
    SQC2 = 62,
    SD = 63
};

// The Register-Type Opcodes
enum class REGISTER_INSTRUCTIONS : uint32_t {
    SLL = 0,
    SRL = 2,
    SRA = 3,
    SLLV = 4,
    SRLV = 6,
    SRAV = 7,
    JR = 8,
    JALR = 9,
    MOVZ = 10,
    MOVN = 11,
    SYSCALL = 12,
    BREAK = 13,
    SNYC = 15,
    MFHI = 16,
    MTHI = 17,
    MFLO = 18,
    MTLO = 19,
    DSLLV = 20,
    DSRLV = 22,
    DSRAV = 23,
    MULT = 24,
    MULTU = 25,
    DIV = 26,
    DIVU = 27,
    ADD = 32,
    ADDU = 33,
    SUB = 34,
    SUBU = 35,
    AND = 36,
    OR = 37,
    XOR = 38,
    NOR = 39,
    MFSA = 40,
    MTSA = 41,
    SLT = 42,
    SLTU = 43,
    DADD = 44,
    DADDU = 45,
    DSUB = 46,
    DSUBU = 47,
    TGE = 48,
    TGEU = 49,
    TLT = 50,
    TLTU = 51,
    TEQ = 52,
    TNE = 54,
    DSLL = 56,
    DSRL = 58,
    DSRA = 59,
    DSLL32 = 60,
    DSRL32 = 62,
    DSRA32 = 63
};

// The Register Immediate Opcodes
enum class REGIMM_INSTRUCTIONS : uint32_t {
    BLTZ = 0,
    BGEZ = 1,
    BLTZL = 2,
    BGEZL = 3,
    TGEI = 8,
    TGEIU = 9,
    TLTI = 10,
    TLTIU = 11,
    TEQI = 12,
    TNEI = 14,
    BLTZAL = 16,
    BGEZAL = 17,
    BLTZALL = 18,
    BGEZALL = 19,
    MTSAB = 24,
    MTSAH = 25
};

// The Multimedia Extension Opcodes
enum class MULTIMEDIA_INSTRUCTIONS : uint32_t {
    MADD = 0,
    MADDU = 1,
    PLZCW = 4,
    MMI0_GROUP = 8,
    MMI2_GROUP = 9,
    MFHI1 = 16,
    MTHI1 = 17,
    MFLO1 = 18,
    MTLO1 = 19,
    MULT1 = 24,
    MULTU1 = 25,
    DIV1 = 26,
    DIVU1 = 27,
    MADD1 = 32,
    MADDU1 = 33,
    MMI1_GROUP = 40,
    MMI3_GROUP = 41,
    PSLLH = 44,
    PSRLH = 46,
    PSRAH = 47,
    PSLLW = 60,
    PSRLW = 62,
    PSRAW = 63
};

// The Multimedia Group 0 Opcodes
enum class MULTIMEDIA_GROUP_0 : uint32_t {
    PADDW = 0,
    PSUBW = 1,
    PCGTW = 2,
    PMAXW = 3,
    PADDH = 4,
    PSUBH = 5,
    PCGTH = 6,
    PMAXH = 7,
    PADDB = 8,
    PSUBB = 9,
    PCGTB = 10,
    PADDSW = 16,
    PSUBSW = 17,
    PEXTLW = 18,
    PPACW = 19,
    PADDSH = 20,
    PSUBSH = 21,
    PEXTLH = 22,
    PPACH = 23,
    PADDSB = 24,
    PSUBSB = 25,
    PEXTLB = 26,
    PPACB = 27,
    PEXT5 = 30,
    PPAC5 = 31
};

// The Multimedia Group 1 Opcodes
enum class MULTIMEDIA_GROUP_1 : uint32_t {
    PABSW = 1,
    PCEQW = 2,
    PMINW = 3,
    PADSBH = 4,
    PABSH = 5,
    PCEQH = 6,
    PMINH = 7,
    PCEQB = 10,
    PADDUW = 16,
    PSUBUW = 17,
    PEXTUW = 18,
    PADDUH = 20,
    PSUBUH = 21,
    PEXTUH = 22,
    PADDUB = 24,
    PSUBUB = 25,
    PEXTUB = 26,
    QFSRV = 27
};

// The Multimedia Group 2 Opcodes
enum class MULTIMEDIA_GROUP_2 : uint32_t {
    PMADDW = 0,
    PSLLVW = 2,
    PSRLVW = 3,
    PMSUBW = 4,
    PMFLO = 5,
    PINTH = 6,
    PMULTW = 8,
    PDIVW = 9,
    CPLYD = 10,
    PMADDH = 12,
    PHMADH = 13,
    PAND = 14,
    PXOR = 15,
    PMSUBH = 16,
    PHMSBH = 17,
    PEXEH = 26,
    PREVH = 27,
    PMULTH = 28,
    PDIVBW = 29,
    PEXEW = 30,
    PROT3W = 31
};

// The Multimedia Group 3 Opcodes
enum class MULTIMEDIA_GROUP_3 : uint32_t {
    PMADDUW = 0,
    PSRAVW = 3,
    PMTHI = 8,
    PMTLO = 9,
    PINTEH = 10,
    PMULTUW = 12,
    PDIVUW = 13,
    PCPYUD = 14,
    POR = 18,
    PNOR = 19,
    PEXCH = 26,
    PCPYH = 27,
    PEXCW = 30
};

// The System Control Coprocessor Opcodes
enum class COPROCESSOR_0_INSTRUCTIONS : uint32_t {
    MFC0 = 0,
    MTC0 = 4,
    BRANCH_ON_COPROCESSOR_0 = 8,
    TRANSLATION_LOOKASIDE_BUFFER_EXCEPTIONS = 16
};

// The Branch on Coprocessor 0 Opcodes
enum class BC0_INSTRUCTIONS : uint32_t { BC0F = 0, BC0T = 1, BC0FL = 2, BC0TL = 3 };

// The Translation Lookaside Buffer/Exception Opcodes
enum class TLB_EXCEPTION_INSTRUCTIONS : uint32_t {
    TLBR = 1,
    TLBWI = 2,
    TLBWR = 6,
    TLBP = 8,
    ERET = 24,
    EI = 56,
    DI = 57
};

// The Floating Point Unit Opcodes
enum class COPROCESSOR_1_INSTRUCTIONS : uint32_t {
    MFC1 = 0,
    CFC1 = 2,
    MTC1 = 4,
    CTC1 = 6,
    BRANCH_ON_COPROCESSOR_1 = 8,
    FLOATING_POINT_UNIT_SINGLE_PRECISION = 16,
    FLOATING_POINT_UNIT_WORD = 20
};

// The Branch on Coprocessor 1 Opcodes
enum class BC1_INSTRUCTIONS : uint32_t { BC1F = 0, BC1T = 1, BC1FL = 2, BC1TL = 3 };

// The Single-Precision Floating Point Unit Opcodes
enum class FPU_S_INSTRUCTIONS : uint32_t {
    ADD_S = 0,
    SUB_S = 1,
    MUL_S = 2,
    DIV_S = 3,
    SQRT_S = 4,
    ABS_S = 5,
    MOV_S = 6,
    NEG_S = 7,
    RSQRT_S = 22,
    ADDA_S = 24,
    SUBA_S = 25,
    MULA_S = 26,
    MADD_S = 28,
    MSUB_S = 29,
    MADDA_S = 30,
    MSUBA_S = 31,
    MAX_S = 40,
    MIN_S = 41,
    C_F = 48,
    C_EQ = 50,
    C_LT = 52,
    C_LE = 54
};

// The Word Fixed-Point Floating Point Unit Opcodes
enum class FPU_W_INSTRUCTIONS : uint32_t { CVT_S = 32 };

// The Vector Processing Unit Opcodes
enum class COPROCESSOR_2_INSTRUCTIONS : uint32_t {
    QMFC2 = 1,
    CFC2 = 2,
    QMTC2 = 5,
    CTC2 = 6,
    BRANCH_ON_COPROCESSOR_2 = 8,
    SPECIAL_1 = 16
};

// The Branch on Coprocessor 2 Opcodes
enum class BC2_INSTRUCTIONS : uint32_t { BC2F = 0, BC2T = 1, BC2FL = 2, BC2TL = 3 };

// The VPU 1st Extension Opcodes
enum class COP2_SPECIAL1_INSTRUCTIONS : uint32_t {
    VADDx = 0,
    VADDy = 1,
    VADDz = 2,
    VADDw = 3,
    VSUBx = 4,
    VSUBy = 5,
    VSUBz = 6,
    VSUBw = 7,
    VMADDx = 8,
    VMADDy = 9,
    VMADDz = 10,
    VMADDw = 11,
    VMSUBx = 12,
    VMSUBy = 13,
    VMSUBz = 14,
    VMSUBw = 15,
    VMAXx = 16,
    VMAXy = 17,
    VMAXz = 18,
    VMAXw = 19,
    VMINIx = 20,
    VMINIy = 21,
    VMINIz = 22,
    VMINIw = 23,
    VMULx = 24,
    VMULy = 25,
    VMULz = 26,
    VMULw = 27,
    VMULq = 28,
    VMAXi = 29,
    VMULi = 30,
    VMINIi = 31,
    VADDq = 32,
    VMADDq = 33,
    VADDi = 34,
    VMADDi = 35,
    VSUBq = 36,
    VMSUBq = 37,
    VSUBi = 38,
    VMSUBi = 39,
    VADD = 40,
    VMADD = 41,
    VMUL = 42,
    VMAX = 43,
    VSUB = 44,
    VMSUB = 45,
    VOPMSUB = 46,
    VMINI = 47,
    VIADD = 48,
    VISUB = 49,
    VIADDI = 50,
    VIAND = 52,
    VIOR = 53,
    VCALLMS = 56,
    CALLMSR = 57,
    SPECIAL_2 = 60
};

// The VPU 2nd Extension Opcodes
enum class COP2_SPECIAL2_INSTRUCTIONS : uint32_t {
    VADDAx = 0,
    VADDAy = 1,
    VADDAz = 2,
    VADDAw = 3,
    VSUBAx = 4,
    VSUBAy = 5,
    VSUBAz = 6,
    VSUBAw = 7,
    VMADDAx = 8,
    VMADDAy = 9,
    VMADDAz = 10,
    VMADDAw = 11,
    VMSUBAx = 12,
    VMSUBAy = 13,
    VMSUBAz = 14,
    VMSUBAw = 15,
    VITOF0 = 16,
    VITOF4 = 17,
    VITOF12 = 18,
    VITOF15 = 19,
    VFTOI0 = 20,
    VFTOI4 = 21,
    VFTOI12 = 22,
    VFTOI15 = 23,
    VMULAx = 24,
    VMULAy = 25,
    VMULAz = 26,
    VMULAw = 27,
    VMULAq = 28,
    VABS = 29,
    VMULAi = 30,
    VCLIPw = 31,
    VADDAq = 32,
    VMADDAq = 33,
    VADDAi = 34,
    VMADDAi = 35,
    VSUBAq = 36,
    VMSUBAq = 37,
    VSUBAi = 38,
    VMSUBAi = 39,
    VADDA = 40,
    VMADDA = 41,
    VMULA = 42,
    VSUBA = 44,
    VMSUBA = 45,
    VOPMULA = 46,
    VNOP = 47,
    VMOVE = 48,
    VMR32 = 49,
    VLQI = 52,
    VSQI = 53,
    VLQD = 54,
    VSQD = 55,
    VDIV = 56,
    VSQRT = 57,
    VRSQRT = 58,
    VWAITQ = 59,
    VMTIR = 60,
    VMFIR = 61,
    VILWR = 62,
    VISWR = 63,
    VRNEXT = 64,
    VRGET = 65,
    VRINIT = 66,
    VRXOR = 67
};

//? ELF FILE STRUCTURES

// Elf Header
struct Elf32_Ehdr {
    unsigned char ident[16];             // Magic numbers + metadata
    uint16_t type;                       // Object file type
    uint16_t machine;                    // Required architecture
    uint32_t version;                    // File version
    uint32_t entry;                      // Entry point of program (VERY important)
    uint32_t program_header_offset;      // Program Header offset
    uint32_t section_header_offset;      // Section Header offset
    uint32_t flags;                      // Processor-specific flags
    uint16_t elf_header_size;            // ELF header size
    uint16_t program_header_entry_size;  // Program header size
    uint16_t program_header_number;      // Number of program headers
    uint16_t section_header_entry_size;  // Section header size
    uint16_t section_header_number;      // Number of section headers
    uint16_t section_header_str_index;   // Section name string table index
};

// Program Header
struct Elf32_Phdr {
    uint32_t type;            // Type of data to expect e.g., 1 == Loadable segment
    uint32_t offset;          // Offset in file
    uint32_t virtual_addr;    // Virtual address in memory
    uint32_t physical_addr;   // Physical address
    uint32_t file_size;       // Size in file
    uint32_t memory_size;     // Size in memory  | memory > file | memory !< file
    uint32_t flags;           // Bit Mask of flags for segment (Read, Write, Executable)
    uint32_t byte_alignment;  // Page size (bytes)
};

// Section Header
struct Elf32_Shdr {
    uint32_t name;        // Index into the section header string table section
    uint32_t type;        // Determines section's contents
    uint32_t flags;       // Attribute determinates
    uint32_t addr;        // Address of first byte
    uint32_t offset;      // Offset from beginning of file to first byte
    uint32_t size;        // Section's size in bytes
    uint32_t link;        // Section Header Index Table link
    uint32_t info;        // Extra info
    uint32_t addralign;   // Size of address alignment (except when 0 or 1)
    uint32_t entry_size;  // Size of entries
};

// Endian Swap Helpers
uint16_t swap16(uint16_t value) { return (value >> 8) | (value << 8); }

uint32_t swap32(uint32_t value) {
    return (value >> 24) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | (value << 24);
}

std::string int_to_hex(int num);
// unsigned int hex_to_int(HexByte hex_value);
std::string int_to_binary(int num);
bool valid_arg(char* argument, std::string& arg, std::string& value);
std::vector<int> bin_to_hex();
int hex_to_asm(int argc, char* argv[]);
// uint32_t parseBinary(std::vector<char>& buffer);
// void assemble_r_instruction(uint32_t instruction);
// void assemble_i_instruction(uint32_t instruction);
// void assemble_j_instruction(uint32_t instruction);

class ELFParser {
   public:
    bool load(const std::string& filename) {
        // Opening the file in binary mode doesn't allow you to use read/write
        // functions involving formatting (e.g., "cout <<", "cin >>")
        file.open(filename, std::ios::binary);

        // Tell the user if the file fails to open (return false to exit early)
        if (!file) {
            std::cerr << "Failed to open file\n";
            return false;
        }

        // Read ELF header
        file.read(reinterpret_cast<char*>(&elf_header), sizeof(elf_header));

        // Validate magic
        if (!(elf_header.ident[0] == 0x7F && elf_header.ident[1] == 'E' && elf_header.ident[2] == 'L' &&
              elf_header.ident[3] == 'F')) {
            std::cerr << "Not a valid ELF file\n";
            return false;
        }

        // Detect Endianess
        isLittleEndian = (elf_header.ident[5] == 1);

        if (!isLittleEndian) {
            swapElfHeader();
        }

        readProgramHeaders();
        readSectionHeaders();

        parseProgram(program_headers[0]);

        // If we've gotten to this point, we've succesfully loaded the binary file into our program/memory
        return true;
    }

    void printInfo() {
        std::cout << "Entry point: 0x" << std::hex << elf_header.entry << "\n";
        std::cout << "Program headers: " << std::dec << elf_header.program_header_number << "\n";

        for (size_t i = 0; i < program_headers.size(); i++) {
            const auto& ph = program_headers[i];
            std::cout << "\nSegment " << i << ":\n";
            std::cout << "  Type: 0x" << std::hex << ph.type << "\n";
            std::cout << "  Offset: 0x" << ph.offset << "\n";
            std::cout << "  VAddr: 0x" << ph.virtual_addr << "\n";
            std::cout << "  Filesize: 0x" << ph.file_size << "\n";
            std::cout << "  Memorysize: 0x" << ph.memory_size << "\n";
            std::cout << "  Byte Alignment: 0x" << ph.byte_alignment << "\n";
        }
    }

    void printSectionInfo() {
        std::cout << "Section headers: " << elf_header.section_header_number << "\n";

        for (size_t i = 0; i < section_headers.size(); i++) {
            const auto& sh = section_headers[i];
            std::cout << "\nSection " << i << ":\n";
            std::cout << "  Name: 0x" << std::hex << sh.name << "\n";
            std::cout << "  type: 0x" << sh.type << "\n";
            std::cout << "  Addr: 0x" << sh.addr << "\n";
            std::cout << "  Offset: 0x" << sh.offset << "\n";
            std::cout << "  Size: 0x" << sh.size << "\n";
            std::cout << "  Entry Size: 0x" << sh.entry_size << "\n";
        }
    }

    // Core mapping function
    uint32_t fileOffsetToVaddr(uint32_t offset) {
        for (const auto& ph : program_headers) {
            if (offset >= ph.offset && offset < ph.offset + ph.file_size) {
                return ph.virtual_addr + (offset - ph.offset);
            }
        }
        return 0;  // Not found
    }

   private:
    std::ifstream file;
    Elf32_Ehdr elf_header{};
    std::vector<Elf32_Phdr> program_headers;
    std::vector<Elf32_Shdr> section_headers;
    bool isLittleEndian = true;

    // Swap the endianness so values are stored the way we want to read them
    void swapElfHeader() {
        elf_header.type = swap16(elf_header.type);
        elf_header.machine = swap16(elf_header.machine);
        elf_header.version = swap32(elf_header.version);
        elf_header.entry = swap32(elf_header.entry);
        elf_header.program_header_offset = swap32(elf_header.program_header_offset);
        elf_header.section_header_offset = swap32(elf_header.section_header_offset);
        elf_header.flags = swap32(elf_header.flags);
        elf_header.elf_header_size = swap16(elf_header.elf_header_size);
        elf_header.program_header_entry_size = swap16(elf_header.program_header_entry_size);
        elf_header.program_header_number = swap16(elf_header.program_header_number);
        elf_header.section_header_entry_size = swap16(elf_header.section_header_entry_size);
        elf_header.section_header_number = swap16(elf_header.section_header_number);
        elf_header.section_header_str_index = swap16(elf_header.section_header_str_index);
    }

    void swapProgramHeader(Elf32_Phdr& program_header) {
        program_header.type = swap32(program_header.type);
        program_header.offset = swap32(program_header.offset);
        program_header.virtual_addr = swap32(program_header.virtual_addr);
        program_header.physical_addr = swap32(program_header.physical_addr);
        program_header.file_size = swap32(program_header.file_size);
        program_header.memory_size = swap32(program_header.memory_size);
        program_header.flags = swap32(program_header.flags);
        program_header.byte_alignment = swap32(program_header.byte_alignment);
    }

    void swapSectionHeader(Elf32_Shdr& section_header) {
        section_header.name = swap32(section_header.name);
        section_header.type = swap32(section_header.type);
        section_header.flags = swap32(section_header.flags);
        section_header.addr = swap32(section_header.addr);
        section_header.offset = swap32(section_header.offset);
        section_header.size = swap32(section_header.size);
        section_header.link = swap32(section_header.link);
        section_header.info = swap32(section_header.info);
        section_header.addralign = swap32(section_header.addralign);
        section_header.entry_size = swap32(section_header.entry_size);
    }

    void readProgramHeaders() {
        file.seekg(elf_header.program_header_offset, std::ios::beg);  // go to first program header offset

        // For each program header, read all the associated info and store it in our vector of program headers
        for (int i = 0; i < elf_header.program_header_number; i++) {
            Elf32_Phdr ph;
            file.read(reinterpret_cast<char*>(&ph), sizeof(ph));

            if (!isLittleEndian) {
                swapProgramHeader(ph);
            }

            program_headers.push_back(ph);
        }
    }

    void readSectionHeaders() {
        file.seekg(elf_header.section_header_offset, std::ios::beg);

        // Obtain all section header info store each entry in a vector
        for (int i = 0; i < elf_header.section_header_number; i++) {
            Elf32_Shdr sh;
            file.read(reinterpret_cast<char*>(&sh), sizeof(sh));

            if (!isLittleEndian) {
                swapSectionHeader(sh);
            }

            section_headers.push_back(sh);
        }
    }

    uint32_t parseBinary(char* buffer) {
        uint32_t instruction = 0;                  // Holds enough for 1 MIPS instruction
        instruction += uint32_t(buffer[3]) << 24;  // Move the first byte into position
        instruction += uint32_t(buffer[2]) << 16;  // Second byte
        instruction += uint32_t(buffer[1]) << 8;   // Third byte
        instruction += uint32_t(buffer[0]);        // Fourth byte
        // std::cout << instruction << "\n";

        return instruction;

        // // Check opcode == 0 | true: R-type instruction | false: I or J instruction
        // if (instruction >> 26 == 0) {
        //     // Assemble R-type instruction
        //     assemble_r_instruction(instruction);
        // } else if (instruction >> 26 == 2 || instruction >> 26 == 3) {  // There are only 2 J-type instructions
        //     // Assemble J-type instruction
        //     assemble_j_instruction(instruction);
        // } else {
        //     // Assemble I-type instruction
        //     assemble_i_instruction(instruction);
        // }
    }

    void parseProgram(Elf32_Phdr& program_header) {
        file.seekg(program_header.offset, std::ios::beg);  // Go to the offset of the actual program
        char char_array[4];                                // to read in 4 byte chunks

        // std::cout << program_header.file_size + program_header.offset << " " << file.tellg() << "\n";
        while (program_header.file_size + program_header.offset >= file.tellg()) {
            // R | 6 opcode, 5 source1, 5 source2, 5 target, 5 shift, 6 function_code
            // I | 6 opcode, 5 source,  5 target, ___________ 16 immediate __________
            // J | 6 opcode, __________________ 26 pseudo_address ___________________
            // std::cout << "Position Before Read: " << file.tellg();
            file.read(reinterpret_cast<char*>(char_array), sizeof(char_array));  // Read 32 bits

            // std::cout << "\nPosition After Read: " << file.tellg() << "\n";
            // Now we have the whole instruction
            uint32_t raw_instruction = parseBinary(char_array);  // Store as a general instruction

            if (!isLittleEndian) {
                raw_instruction = swap32(raw_instruction);
            }

            BinaryInstructionUniversal formatted_instruction = opcodeChecker(raw_instruction);
        }
    }

    BinaryInstructionUniversal opcodeChecker(uint32_t instruction) {
        // Time to figure out what the instruction is supposed to do
        // std::cout << std::hex << instruction << "\n";
        int opcode = instruction >> 26;  // in binary

        //? Use Enum Class and Switch Statement for opcode lookup
        switch (opcode) {
            // R-Type Instruction
            case static_cast<int>(STANDARD_INSTRUCTIONS::REGISTER):
                //* Should call a function to determine the specific Register-Type Instruction

                // Removes all bits except those in function_code(last 6) section
                // int function_code = (instruction << 26) >> 26;  //! Only exists within the scope of the swtich statement
                // std::cout << function_code << std::endl;
                break;
            case static_cast<int>(STANDARD_INSTRUCTIONS::REGISTER_IMMEDIATE):

                break;
        }

        return BinaryInstructionUniversal{'R', instruction};
    }
};

/**
 * @brief The main function :) We start executing code here.
 *
 * @param argc The number of command-line arguments provided when running the executable.
 * @param argv Command-line arguments using C-style strings which pertain to certain
 * properties of changable variables the user may  want control over (e.g., input-file, output-file, etc.)
 * @return int The return code given from running the program (e.g., 0 for clear, 1 for error).
 */
int main() {
    ELFParser parser;

    // Attempts to load the binary of the ps2 "executable"
    if (!parser.load("SLUS_205.91")) {
        return 1;
    }

    parser.printInfo();
    parser.printSectionInfo();

    // Example mapping
    uint32_t testOffset = 0x1000;
    uint32_t addr = parser.fileOffsetToVaddr(testOffset);

    std::cout << "\nOffset 0x" << std::hex << testOffset << " maps to VAddr 0x" << addr << "\n";
    // std::vector<int> buffer = bin_to_hex();
    // parseBinary(buffer);

    return 0;
}

// void assemble_r_instruction(uint32_t instruction) {
//     int source_register1 = (instruction << 6) >> 27;
//     int source_register2 = (instruction << 11) >> 27;
//     int target_register = (instruction << 16) >> 27;
//     int shift_value = (instruction << 21) >> 27;
//     int function_code = (instruction << 26) >> 26;
// }

// void assemble_i_instruction(uint32_t instruction) {}

// void assemble_j_instruction(uint32_t instruction) {}

/**
 * @brief Takes hexadecimal and translates it into MIPS assembly instructions
 *
 * This function takes hexadecimal input from a .txt file, in the format of the
 * bin-to-hex function and outputs MIPS assembly instructions in a new .txt file.
 *
 * @param argc The number of arguments held inside argv.
 * @param argv The arguments that could be used to modify the input file or output
 * file of this function.
 * @return int The return code given to the main function (e.g., 0 for clear, 1 for error).
 */
// int hex_to_asm(int argc, char* argv[]) {  //! Add buffer object input as a reference
//     // Properties
//     std::string valid_in_file = "";
//     std::string valid_out_file = "";
//     bool valid_format = false;

//     // Configures which propertries to change
//     for (int i = 0; i < argc; i++) {
//         // std::cout << argv[i] << "\n";
//         std::string command_value;
//         std::string arg_key;
//         if (valid_arg(argv[i], arg_key, command_value)) {
//             // std::cout << arg_key << ' ' << argc << "\n";
//             if (arg_key == "input-file") {
//                 valid_in_file = command_value;
//             } else if (arg_key == "output-file") {
//                 valid_out_file = command_value;
//             } else if (arg_key == "format-input") {
//                 valid_format = true;
//             }
//         }
//     }

//     // std::cout << valid_in_file << " " << valid_out_file << " " << valid_bpr << " " << valid_interpreter << "\n";

//     // Creates variables holding changed values for properties
//     std::string hexfile;
//     std::string outfile;
//     bool format_input;
//     if (valid_in_file != "") {
//         hexfile = valid_in_file;
//     } else {
//         hexfile = "SLUS_custom.txt";  // File to read the hex data from
//     }
//     if (valid_out_file != "") {
//         outfile = valid_out_file;
//     } else {
//         outfile = "SLUS_205_asm.txt";  // File to write the asm data to
//     }
//     if (!valid_format) {  // Is the
//         format_input = false;
//     } else {
//         format_input = true;
//     }

//     std::fstream readfile{hexfile, std::ios::in | std::ios::ate};  // Start at the end of the file

//     // If can't open file then exit
//     if (!readfile.is_open()) {
//         std::cerr << "Uh oh, " << hexfile << " could not be opened for reading!\n";
//         return 1;
//     }

//     std::streamsize size = readfile.tellg();

//     // Move the read cursor to the beginning of the file
//     readfile.seekg(0, std::fstream::beg);

//     std::vector<HexInstruction> buffer(size / 2);
//     std::string line;

//     // Read the bulk stream into the vector memory block
//     if (format_input) {  // input is space separated and has new-line characters
//         while (std::getline(readfile, line)) {
//             for (int i = 0; i < 8; i++) {
//                 HexInstruction inst{};
//                 inst.a = {line[0 + 8 * i], line[1 + 8 * i]};
//                 inst.b = {line[3 + 8 * i], line[4 + 8 * i]};
//                 inst.c = {line[6 + 8 * i], line[7 + 8 * i]};
//                 inst.d = {line[9 + 8 * i], line[10 + 8 * i]};
//                 buffer.push_back(inst);
//             }

//             // std::cout << "Length: " << line.length() << "\n";
//             // std::cout << "Line: " << line << "\n";
//             // std::cout << "Chars: " << line[0] << line[1] << ' ' << line[3] << line[4] << ' ' << line[6] << line[7]
//             //           << ' ' << line[9] << line[10] << "\n";
//             // std::cout << "Ints: " << (int)line[0] << (int)line[1] << ' ' << (int)line[3] << (int)line[4] << ' '
//             //           << (int)line[6] << (int)line[7] << ' ' << (int)line[9] << (int)line[10] << "\n";
//         }
//     } else {  // input is one continuous stream without any terminators or delimiters
//         while (std::getline(readfile, line)) {
//             for (int i = 0; i < 8; i++) {
//                 HexInstruction inst{};
//                 inst.a = {line[0 + 8 * i], line[1 + 8 * i]};
//                 inst.b = {line[2 + 8 * i], line[3 + 8 * i]};
//                 inst.c = {line[4 + 8 * i], line[5 + 8 * i]};
//                 inst.d = {line[6 + 8 * i], line[7 + 8 * i]};
//                 buffer.push_back(inst);
//             }

//             // std::cout << "Length: " << line.length() << "\n";
//             // std::cout << "Line: " << line << "\n";
//             // std::cout << "Chars: " << line[0] << line[1] << ' ' << line[2] << line[3] << ' ' << line[4] << line[5]
//             //           << ' ' << line[6] << line[7] << "\n";
//             // std::cout << "Ints: " << (int)line[0] << ' ' << (int)line[1] << ' ' << (int)line[2] << ' ' <<
//             (int)line[3]
//             //           << ' ' << (int)line[4] << ' ' << (int)line[5] << ' ' << (int)line[6] << ' ' << (int)line[7]
//             //           << "\n";
//         }
//     }
//     readfile.close();

//     // Covert first Hex to Binary to check the opcode (first 6 bits)
//     int opcode_int = hex_to_int(buffer[0].a);
//     std::cout << opcode_int << ' ' << buffer[0].a << "\n";

//     // for (int i = 0; i < size; i++) {
//     //     std::cout << buffer[i].a << ' ' << buffer[i].b << ' ' << buffer[i].c << ' ' << buffer[i].d << "  ";
//     //     std::cout << hex_to_int(buffer[i].a) << ' ' << hex_to_int(buffer[i].b) << ' ' << hex_to_int(buffer[i].c)
//     << '
//     //     '
//     //               << hex_to_int(buffer[i].d) << "\n";
//     // }

//     return 0;
// }

/**
 * @brief Converts a Hexadecimal value to a base-10 integer.
 *
 * @param hex_value The hexadecimal (base-16) value to convert.
 * @return int Returns a base-10 integer value.
 */
// unsigned int hex_to_int(HexByte hex_value) {
//     int num1 = 16;
//     int num2 = 1;

//     if ('A' >= hex_value.a && hex_value.a <= 'F') {
//         num1 *= hex_value.a - 'A' + 10;
//     } else if ('a' >= hex_value.a && hex_value.a <= 'f') {
//         num1 *= hex_value.a - 'a' + 10;
//     } else if ('0' >= hex_value.a && hex_value.a <= '9') {
//         num1 *= hex_value.a - '0';
//     }

//     if ('A' >= hex_value.b && hex_value.b <= 'F') {
//         num2 *= hex_value.b - 'A' + 10;
//     } else if ('a' >= hex_value.b && hex_value.b <= 'f') {
//         num2 *= hex_value.b - 'a' + 10;
//     } else if ('0' >= hex_value.b && hex_value.b <= '9') {
//         num2 *= hex_value.b - '0';
//     }

//     return num1 + num2;
// }

/**
 * @brief Translates an integer value into its corresponding hexadecimal value
 *
 * @param num The integer to translate into hexadecimal format (0-9, A-F).
 * @return std::string A string containing two hexadecimal characters, which match an integer 0-255.
 */
std::string int_to_hex(int num) {
    std::string result = "";
    int num1 = num / 16;
    int num2 = num % 16;

    // std::cout << num1 << ' ' << num2 << '\n';

    if (num1 < 10) {
        result.push_back(num1 + '0');
    } else {
        switch (num1) {
            case 10: {
                result.push_back('A');
                break;
            }
            case 11: {
                result.push_back('B');
                break;
            }
            case 12: {
                result.push_back('C');
                break;
            }
            case 13: {
                result.push_back('D');
                break;
            }
            case 14: {
                result.push_back('E');
                break;
            }
            case 15: {
                result.push_back('F');
                break;
            }
        }
    }

    if (num2 < 10) {
        result.push_back(num2 + '0');
    } else {
        switch (num2) {
            case 10: {
                result.push_back('A');
                break;
            }
            case 11: {
                result.push_back('B');
                break;
            }
            case 12: {
                result.push_back('C');
                break;
            }
            case 13: {
                result.push_back('D');
                break;
            }
            case 14: {
                result.push_back('E');
                break;
            }
            case 15: {
                result.push_back('F');
                break;
            }
        }
    }

    return result;
}

/**
 * @brief Translates an integer value into its corresponding binary value
 *
 * @param num The integer to translate into binary format (0-1).
 * @return std::string A string containing eight binary characters, which match an integer 0-255.
 */
std::string int_to_binary(int num) {
    std::string result = "";
    int num1 = num % 2;
    int num2 = (num / 2) % 2;
    int num3 = (num / 4) % 2;
    int num4 = (num / 8) % 2;
    int num5 = (num / 16) % 2;
    int num6 = (num / 32) % 2;
    int num7 = (num / 64) % 2;
    int num8 = (num / 128) % 2;

    // std::cout << num1 << ' ' << num2 << ' ' << num3 << ' ' << num4 << ' ' << num5;
    // std::cout << ' ' << num6 << ' ' << num7 << ' ' << num8 << '\n';

    result.push_back(num8 + '0');
    result.push_back(num7 + '0');
    result.push_back(num6 + '0');
    result.push_back(num5 + '0');
    result.push_back(num4 + '0');
    result.push_back(num3 + '0');
    result.push_back(num2 + '0');
    result.push_back(num1 + '0');

    return result;
}

/**
 * @brief Checks to see if a given command-line argument is valid
 *
 * @param argument A C-style string containing the argument to check the validity of.
 * @param arg Set to the key that corresponds to a certain property for the bin-to-hex
 * function. Used to check against known keys for determining if argument is valid.
 * @param value Set to the value that a given property should be changed to. Used by
 * caller function.
 * @return true The given argument was valid, proceed with changing propertie(s) for bin-to-hex.
 * @return false The given argument was NOT valid, DO NOT change propertie(s) for bin-to-hex.
 */
bool valid_arg(char* argument, std::string& arg, std::string& value) {
    int i = 2;
    do {
        arg.push_back(argument[i]);
        i++;
    } while (argument[i] != '=' && argument[i] != '\0');

    std::vector<std::string> possible_args{"input-file",    "output-file",   "bytes-per-row",
                                           "binary-output", "format-output", "format-input"};

    // Tries to find the current argument in the list of possible_args
    if (std::find(possible_args.begin(), possible_args.end(), arg) != possible_args.end()) {
        i++;
        do {
            value.push_back(argument[i]);
            i++;
        } while (argument[i] != '\0');
        return 1;
    }

    return 0;
}

/**
 * @brief Translates pre-compiled binary (normally unreadable unless using a hex editor) into hexadecimal.
 *
 * @param argc The number of C-style strings provided by argv.
 * @param argv Contains different properties that may be changed within the function
 * (e.g., input-file, output-file, bytes-per-row, binary-output).
 * @return int The return code given to the main function (e.g., 0 for clear, 1 for error).
 */
std::vector<int> bin_to_hex() {
    // std::string valid_in_file = "";
    // std::string valid_out_file = "";
    // int valid_bpr = 0;
    // bool valid_interpreter = false;
    // bool valid_format = false;

    // for (int i = 0; i < argc; i++) {
    //     // std::cout << argv[i] << "\n";
    //     std::string command_value;
    //     std::string arg_key;
    //     if (valid_arg(argv[i], arg_key, command_value)) {
    //         // std::cout << arg_key << ' ' << argc << "\n";
    //         if (arg_key == "input-file") {
    //             valid_in_file = command_value;
    //         } else if (arg_key == "output-file") {
    //             valid_out_file = command_value;
    //         } else if (arg_key == "binary-output") {
    //             valid_interpreter = std::stoi(command_value);  // Should be 1 for true, 0 for false
    //         } else if (arg_key == "bytes-per-row") {
    //             valid_bpr = std::stoi(command_value);
    //         } else if (arg_key == "format-output") {  // Should be just a key tag without a value
    //             valid_format = true;
    //         }
    //     }
    // }

    // std::cout << valid_in_file << " " << valid_out_file << " " << valid_bpr << " " << valid_interpreter << "\n";

    std::string hexfile = "SLUS_205.91";
    // std::string outfile;
    // int bytes_per_row = 32;
    // bool binary_file = false;
    // bool format_output = false;
    // if (valid_in_file != "") {
    //     hexfile = valid_in_file;
    // } else {
    //     hexfile = "SLUS_205.91";
    // }  // File to read the hex data from
    // if (valid_out_file != "") {
    //     outfile = valid_out_file;
    // } else {
    //     outfile = "SLUS_custom.txt";
    // }  // File to write the hex data to
    // if (valid_bpr != 0) {
    //     bytes_per_row = valid_bpr;
    // } else {
    //     bytes_per_row = 32;
    // }
    // if (!valid_interpreter) {
    //     binary_file = false;
    // } else {
    //     binary_file = true;
    // }
    // if (!valid_format) {
    //     format_output = false;
    // } else {
    //     format_output = true;
    // }

    // Access point to read file as binary, starting at the end of the file
    std::fstream readfile{hexfile, std::ios::in | std::ios::binary | std::ios::ate};
    // Check for open error
    if (!readfile.is_open()) {
        std::cerr << "Failed to open the file: " << hexfile << ".\n";
    }

    std::streamsize file_size = readfile.tellg();  // Tells the total length of the file we're reading
    readfile.seekg(0);                             // Moves the read file pointer to begin reading the file

    // Create a buffer the same size as the file
    std::vector<int> buffer;
    buffer.reserve(file_size);
    char line[5];
    line[4] = '\0';

    // Read the bulk stream into the vector memory block
    while (readfile.read(line, 4)) {
        // std::cout << line[0] << line[1] << line[2] << line[3] << "\n";
        buffer.push_back(int(uint8_t(line[0])));
        buffer.push_back(int(uint8_t(line[1])));
        buffer.push_back(int(uint8_t(line[2])));
        buffer.push_back(int(uint8_t(line[3])));
    }
    std::cout << "Successfully loaded " << file_size << " bytes into memory.\n";
    readfile.close();

    return buffer;  //! RETURN BUFFER OBJECT HERE

    // // Access point to write to file, which will be in hex (0-9, A-F)
    // std::ofstream writefile{outfile, std::ios::out};
    // // Check for open error
    // if (!writefile) {
    //     std::cerr << "Uh oh, " << outfile << " could not be opened for writing!\n";
    //     return 1;
    // }

    // // Goes through entire buffer and translates each unit to hex or binary
    // for (int i = 0; i <= file_size; i++) {
    //     if (i % bytes_per_row == 0 && i != 0 && format_output) {
    //         writefile << "\n";
    //     }
    //     if (binary_file) {
    //         writefile << int_to_binary(int(uint8_t(buffer[i])));
    //     } else {
    //         writefile << int_to_hex(int(uint8_t(buffer[i])));
    //     }
    //     if (format_output) {
    //         writefile << ' ';
    //     }
    // }
    // writefile.close();

    // if (binary_file) {
    //     std::cout << "Binary Writing Completed Successfully!\n";
    // } else {
    //     std::cout << "Hex Writing Completed Successfully!\n";
    // }
}
