/* @header@ */
#include <stdint.h>


struct emu_env_w32_dll
{
	char 		*dllname;

	char 		*image;
	uint32_t	imagesize;

	uint32_t	baseaddr;

	struct emu_env_w32_dll_export **exports;
};

struct emu_env_w32_dll *emu_env_w32_dll_new();
void emu_env_w32_dll_free(struct emu_env_w32_dll *dll);
struct emu_env_w32_dll_export *emu_env_w32_dll_export_add(struct emu_env_w32_dll *dll, const char *fnname, uint32_t addr);

