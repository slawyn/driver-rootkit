#pragma once
/* sidt returns idt in this format */
#include "stdint.h"

typedef struct
{
    uint16_t    ui16IdtLimit;
    uint64_t    ui64IdtBase;
} IDTINFO;


// entry in the IDT: this is sometimes called

// an "interrupt gate" for 32-bit


#pragma pack(1)

/*
#define MAX_IDT_ENTRIES 0xFF
typedef struct{
    unsigned short LowOffset;
    unsigned short selector;
    unsigned char unused_lo;
    unsigned char segment_type : 4; //0x0E is interrupt gate
    unsigned char system_segment_flag : 1;
    unsigned char DPL : 2;     // descriptor privilege level
    unsigned char P : 1;       // present
    unsigned short HiOffset;
} IDTENTRY32_t;*/

typedef struct {
    uint16_t ui16LowOffset; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type_attr; // type and attributes
    uint16_t ui16MiddeOffset; // offset bits 16..31
    uint32_t ui32HighOffset; // offset bits 32..63
    uint32_t zero;     // reserved
} IDTENTRY;
#pragma pack()



#define MAKELONG(a, b) ((unsigned long)(((unsigned short)(a)) | ((unsigned long)((unsigned short)(b)))<< 16))

