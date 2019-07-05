.set noreorder

#include "pspimport.s"

IMPORT_START "scePower",0x40010000
IMPORT_FUNC	 "scePower",0x737486F2,scePowerSetClockFrequency
IMPORT_FUNC	 "scePower",0x843FBF43,scePowerSetCpuClockFrequency
IMPORT_FUNC	 "scePower",0xB8D7B3FB,scePowerSetBusClockFrequency

IMPORT_START "sceReg",0x40010000
IMPORT_FUNC	 "sceReg",0x92E41280,sceRegOpenRegistry
IMPORT_FUNC	 "sceReg",0x1D8A762E,sceRegOpenCategory
IMPORT_FUNC	 "sceReg",0x39461B4D,sceRegFlushRegistry
IMPORT_FUNC	 "sceReg",0xFA8A5739,sceRegCloseRegistry
IMPORT_FUNC	 "sceReg",0xD4475AA8,sceRegGetKeyInfo
IMPORT_FUNC	 "sceReg",0x0CAE832B,sceRegCloseCategory
IMPORT_FUNC	 "sceReg",0x17768E14,sceRegSetKeyValue
IMPORT_FUNC	 "sceReg",0x0D69BF40,sceRegFlushCategory
IMPORT_FUNC	 "sceReg",0x28A8E98A,sceRegGetKeyValue
