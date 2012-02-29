# Makefile for encode app

APP_NAME = kfw_cap

ifeq ($(PLATFORM),ti816x-evm)
PLATDIR = dm816x
else
PLATDIR = dm814x
endif

SRCDIR = src   src/$(PLATDIR)  ../common
INCDIR = . src src/$(PLATDIR)  ../common

# List all the external components/interfaces, whose interface header files 
#  need to be included for this component
INCLUDE_EXERNAL_INTERFACES = bios xdc omx timmosal ipc fc xdais osal syslink \
                             ce h264enc uia slog

# List of all external libraries that need to be linked
EXTLIBS_m3video =
EXTLIBS_m3vpss =
EXTLIBS_a8host = $(omx_LIBPATH)/../lib/omxcore.av5T \
                 $(omx_LIBPATH)/../lib/memcfg.av5T                     \
                 $(omx_LIBPATH)/../lib/domx.av5T           \
                 $(omx_LIBPATH)/../lib/domx_delegates_shmem.av5T       \
                 $(omx_LIBPATH)/../lib/timmosal.av5T \
                 $(omx_LIBPATH)/../lib/omxcfg.av5T         \
                 $(osal_PATH)/packages/linuxdist/build/lib/osal.a \
                 $(osal_PATH)/packages/linuxdist/cstubs/lib/cstubs.a \
                 $(fc_PATH)/packages/ti/sdo/rcm/lib/debug/rcm_syslink.av5T \
                 $(fc_PATH)/packages/ti/sdo/fc/memutils/lib/release/memutils.av5T \
                 $(osal_PATH)/packages/ti/sdo/xdcruntime/linux/lib/debug/osal_linux_470.av5T \
                 $(fc_PATH)/packages/ti/sdo/fc/global/lib/debug/fcsettings.av5T \
                 $(syslink_PATH)/packages/ti/syslink/lib/syslink.a_release \
                 $(linuxutils_PATH)/packages/ti/sdo/linuxutils/cmem/lib/cmem.a470MV \
                 $(uia_PATH)/packages/ti/uia/linux/lib/servicemgr.a
#                 $(omx_LIBPATH)/../lib/ldrmemcfg.av5T
#                 $(slog_PATH)/lib/slog.a 

# List all the components required by the application
COMP_LIST_m3video = omxbase venc server scheduler domx omxcore 
COMP_LIST_m3vpss = omxbase vfcc vfpc vfdc ctrl domx omxcore hdvpss
COMP_LIST_a8host = 


# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON =
CFLAGS_LOCAL_COMMON = -DGlobal_appTerminate=0 



SRCS_a8host = main.c msgq.c platform_utils.c \
              ilclient.c  ilclient_utils.c semp.c serv.c sync_sem.c

CFLAGS_LOCAL_a8host = -DGlobal_GrpxDssMsgHeapId=6 -DGlobal_TilerHeapId=7 -D_VIDEO_M3_DYNAMIC_CONFIG

ifeq ($(CORE),a8host)
  CFLAGS_LOCAL_a8host += -D_LOCAL_CORE_a8host_
endif

# Include common make files
include $(ROOTDIR)/makerules/common.mk


# Nothing beyond this point
