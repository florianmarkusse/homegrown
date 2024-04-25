#ifndef EFI_C_EFI_PROTOCOL_MP_SERVICES_H
#define EFI_C_EFI_PROTOCOL_MP_SERVICES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "c-efi-base.h"

#define C_EFI_MP_SERVICES_PROTOCOL_GUID                                        \
    C_EFI_GUID(0x3fdda605, 0xa76e, 0x4f46, 0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, \
               0x3d, 0x08)

typedef struct {
    CEfiU32 package;
    CEfiU32 core;
    CEfiU32 thread;
} CEfiPhysicalLocation;

typedef struct {
    CEfiU32 package;
    CEfiU32 module;
    CEfiU32 tile;
    CEfiU32 die;
    CEfiU32 core;
    CEfiU32 thread;
} CEfiCPUPhysicalLocation;

typedef union {
    CEfiPhysicalLocation location2;
} CEfiExtendedProcessorInformation;

typedef struct {
    CEfiU64 processorId;
    CEfiU32 statusFlag;
    CEfiPhysicalLocation location;
    CEfiExtendedProcessorInformation extendedInformation;
} CEfiProcessorInformation;

typedef void(CEFICALL *CEfiAPProcedure)(void *procedureArgument);

typedef struct CEfiMPServicesProtocol {
    CEfiUSize(CEFICALL *getNumberOfProcessors)(
        CEfiMPServicesProtocol *this_, CEfiUSize *numberOfProcessors,
        CEfiUSize *numberOfEnabledProcessors);
    CEfiUSize(CEFICALL *getProcessorInfo)(
        CEfiMPServicesProtocol *this_, CEfiUSize processorNumber,
        CEfiProcessorInformation *processorInfoBuffer);
    CEfiUSize(CEFICALL *startupAllAPs)(CEfiMPServicesProtocol *this_,
                                       CEfiAPProcedure procedure,
                                       CEfiBool singleThread,
                                       CEfiEvent waitEvent,
                                       CEfiUSize timeoutInMicroSeconds,
                                       void *procedureArgument,
                                       CEfiUSize **failedCPUList);
    CEfiUSize(CEFICALL *startupThisAP)(CEfiMPServicesProtocol *this_,
                                       CEfiAPProcedure procedure,
                                       CEfiUSize processorNumber,
                                       CEfiEvent waitEvent,
                                       CEfiUSize timeoutInMicroSeconds,
                                       void *procedureArgument,
                                       CEfiBool *finished);
    CEfiUSize(CEFICALL *switchBSP)(CEfiMPServicesProtocol *this_,
                                   CEfiUSize processorNumber,
                                   CEfiBool enableOldBSP);
    CEfiUSize(CEFICALL *enableDisableAP)(CEfiMPServicesProtocol *this_,
                                         CEfiUSize processorNumber,
                                         CEfiBool enableAP,
                                         CEfiU32 *healthFlag);
    CEfiUSize(CEFICALL *whoAmI)(CEfiMPServicesProtocol *this_,
                                CEfiUSize processorNumber);

} CEfiMPServicesProtocol;

#ifdef __cplusplus
}
#endif

#endif
