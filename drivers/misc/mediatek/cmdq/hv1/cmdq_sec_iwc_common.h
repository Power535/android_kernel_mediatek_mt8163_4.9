
/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __CMDQ_SEC_IWC_COMMON_H__
#define __CMDQ_SEC_IWC_COMMON_H__
#include "cmdqSecTl_Api.h"
/* shared DRAM */
/* bit x = 1 means thread x raise IRQ */
#define CMDQ_SEC_SHARED_IRQ_RAISED_OFFSET (0x0)
#define CMDQ_SEC_SHARED_THR_CNT_OFFSET (0x100)
#define CMDQ_SEC_SHARED_TASK_VA_OFFSET (0x200)
#define CMDQ_SEC_SHARED_OP_OFFSET (0x300)

/* commanad buffer & metadata */
#define CMDQ_TZ_CMD_BLOCK_SIZE   (32 * 1024)

#define CMDQ_IWC_MAX_CMD_LENGTH (32 * 1024 / 4)

#define CMDQ_IWC_MAX_ADDR_LIST_LENGTH (20)

#define CMDQ_IWC_CLIENT_NAME (16)

#define CMDQ_SEC_MESSAGE_INST_LEN (8)
#define CMDQ_SEC_DISPATCH_LEN (8)

enum CMDQ_IWC_ADDR_METADATA_TYPE {
	CMDQ_IWC_H_2_PA = 0,	/* sec handle to sec PA */
	CMDQ_IWC_H_2_MVA = 1,	/* sec handle to sec MVA */
	CMDQ_IWC_NMVA_2_MVA = 2,	/* map normal MVA to secure world */
	/* DDP register needs to set opposite value when HDCP fail */
	CMDQ_IWC_DDP_REG_HDCP = 3,
	CMDQ_IWC_NMVA_2_MVA_REVERSE = 4, /* map normal MVA to secure world */
};


/* profile setting enum
 * .default is 0, not enable profile,
 * otherwise we enalbe it depends on profile bit
 */

enum CMDQ_IWC_PROF_ENUM {
	CMDQ_IWC_PROF_LOG,
	CMDQ_IWC_PROF_SYSTRACE,
};

/* IWC systrace tag enum */
enum CMDQ_IWC_SYSTRACE_TAG_ENUM {
	/* profile cmdqSecDR */
	CMDQ_IST_CMDQ_IPC = 0,
	CMDQ_IST_CMDQ_TASK_ACQ,
	CMDQ_IST_CMDQ_TASK_REL,
	CMDQ_IST_CMDQ_HW,
	CMDQ_IST_CMDQ_SIG_WAIT,
	/* dummy tag, just mark end of CMDQ driver profiling tag */
	CMDQ_IST_CMDQ_LAST_TAG,

	/* profile cmdqSecTL */
	CMDQ_IST_CMD_ALLOC,
	CMDQ_IST_H2PAS,
	CMDQ_IST_PORT_ON,
	CMDQ_IST_PORT_OFF,
	CMDQ_IST_DAPC_ON,
	CMDQ_IST_DAPC_OFF,
	CMDQ_IST_SUBMIT,

	/* always at the end */
	CMDQ_IST_MAX_COUNT,
};

/*  */
/* IWC message */
/*  */
struct iwcCmdqAddrMetadata_t {
	/* [IN]_d, index of instruction.
	 * Update its argB value to real PA/MVA in secure world
	 */
	uint32_t instrIndex;

	/*
	 * Note: Buffer and offset
	 *
	 *   -------------
	 *   |     |     |
	 *   -------------
	 *   ^     ^  ^  ^
	 *   A     B  C  D
	 *
	 *	A: baseHandle
	 *	B: baseHandle + blockOffset
	 *  C: baseHandle + blockOffset + offset
	 *	A~B or B~D: size
	 */

	/* [IN] addr handle type*/
	uint32_t type;
	/* [IN]_h, secure address handle */
	uint32_t baseHandle;
	/* [IN]_b, block offset from handle(PA) to current block(plane) */
	uint32_t blockOffset;
	/* [IN]_b, buffser offset to secure handle */
	uint32_t offset;
	/* buffer size */
	uint32_t size;
	/* hw port id (i.e. M4U port id)*/
	uint32_t port;
};

/* tablet use */

enum CMDQ_DISP_MODE {
	CMDQ_DISP_NON_SUPPORTED_MODE = 0,
	CMDQ_DISP_SINGLE_MODE = 1,
	CMDQ_DISP_VIDEO_MODE = 2,
	CMDQ_MDP_USER_MODE = 3,
};
struct iwcCmdqDebugConfig_t {
	int32_t logLevel;
	int32_t enableProfile;
};

struct iwcCmdqSecStatus_t {
	uint32_t step;
	int32_t status;
	uint32_t args[4];
	uint32_t sec_inst[CMDQ_SEC_MESSAGE_INST_LEN];
	uint32_t inst_index;
	char dispatch[CMDQ_SEC_DISPATCH_LEN];
};

struct iwcCmdqSystraceLog_t {
	uint64_t startTime; /* start timestamp */
	uint64_t endTime;   /* end timestamp */
};


struct iwcCmdqMetadata_t {
	uint32_t addrListLength;
	struct iwcCmdqAddrMetadata_t addrList[CMDQ_IWC_MAX_ADDR_LIST_LENGTH];

	uint64_t enginesNeedDAPC;
	uint64_t enginesNeedPortSecurity;
	enum CMDQ_DISP_MODE secMode;
	/* for MDP to copy HDCP version from srcHandle to dstHandle */
	/* will remove later */
	uint32_t srcHandle;
	uint32_t dstHandle;

};

struct iwcCmdqSectraceBuffer_t {
	uint32_t addr; /* pass VA for TCI cases, and pass PA for DCI case */
	uint32_t size;
};

struct iwcCmdqPathResource_t {
	/* use long long for 64 bit compatible support */
	long long shareMemoryPA;
	uint32_t size;
	bool useNormalIRQ;      /* use normal IRQ in SWd */
};

struct iwcCmdqCancelTask_t {
	/* [IN] */
	int32_t thread;
	uint32_t waitCookie;

	/* [OUT] */
	bool throwAEE;
	bool hasReset;
	int32_t irqStatus; /* global secure IRQ flag */
	int32_t irqFlag; /* thread IRQ flag */
	uint32_t errInstr[2]; /* errInstr[0] = instB, errInstr[1] = instA */
	uint32_t regValue;
	uint32_t pc;
};

struct iwcCmdqCommand_t {
	/* share memory with NWd */
	/* startPA and size must be 4K aligned for drApiMapPhys */
	/*   [IN]PA start address of THR cookie */
	/* uint32_t sharedThrExecCntPA; */
	/* uint32_t sharedThrExecCntSize; */

	/* basic execution data */
	uint32_t thread;
	uint32_t scenario;
	uint32_t priority;
	uint32_t commandSize;
	uint64_t engineFlag;
	uint32_t pVABase[CMDQ_IWC_MAX_CMD_LENGTH];

	/* exec order data */
	/* [IN] index in thread's task list, it should be (nextCookie - 1) */
	uint32_t waitCookie;
	/* [IN] reset HW thread */
	bool resetExecCnt;

	/* client info */
	int32_t callerPid;
	char callerName[CMDQ_IWC_CLIENT_NAME];

	/* metadata */
	struct iwcCmdqMetadata_t metadata;

	/* debug */
	uint64_t hNormalTask;	/* handle to reference task in normal world */
};

/* linex kernel and mobicore has their own MMU tables,
 * the latter's is used to map world shared memory and physical address
 * so mobicore dose not understand linux virtual address mapping.
 * if we want to transact a large buffer in TCI/DCI,
 * there are 2 method (both need 1 copy):
 * 1. use mc_map, to map normal world buffer to WSM,
 * and pass secure_virt_addr in TCI/DCI buffer
 * note mc_map implies a memcopy to copy content from normal world to WSM
 * 2. declare a fixed length array in TCI/DCI struct, and its size must be < 1M
 */
struct iwcCmdqMessage_t {
	union {
	    uint32_t cmd;   /* [IN] command id */
	    int32_t rsp;    /* [OUT] 0 for success, < 0 for error */
	};

	union {
	    struct iwcCmdqCommand_t command;
	    struct iwcCmdqCancelTask_t cancelTask;
	    struct iwcCmdqPathResource_t pathResource;
	    struct iwcCmdqSectraceBuffer_t sectracBuffer;
	};

	struct iwcCmdqDebugConfig_t debug;
	struct iwcCmdqSecStatus_t secStatus;
};
#define iwcCmdqMessage_ptr struct iwcCmdqMessage_t *

/*  */
/* ERROR code number (ERRNO) */
/* note the error result returns negative value, i.e, -(ERRNO) */
/*  */
enum ERROR_STATUS {
	CMDQ_ERR_INVALID_PARAM                =    50,
	CMDQ_ERR_INVALID_SECURE_THREAD_ID    =    51,
	CMDQ_ERR_INVALID_THREAD                =    52,
	CMDQ_ERR_INVALID_IRQ_FLAG			=	53,

	/*alloc secure memeory fail*/
	CMDQ_ERR_NOMEM						=	12,
	CMDQ_ERR_FAULT						=	13,
	CMDQ_ERR_NO_PREV_TASK				=	14,

	CMDQ_ERR_SECURE_PATH_NOT_READY		=	15,
	/*acqire secure task error*/
	CMDQ_ERR_ACQUIRE_TASK_FAILED		=	17,
	CMDQ_ERR_TASK_BUFFER_SIZE_WRONG		=	18,
	CMDQ_ERR_METADATA_COUNT_MISMATCH	=	19,
	CMDQ_ERR_METADATA_INDEX_TOO_LARGE	=	20,

	CMDQ_ERR_RESET_THREAD_WITH_TASK		=	21,
	CMDQ_ERR_INSET_WRONG_COOKIE			=	22,


	CMDQ_ERR_RESET_HW_THREAD_FAIL		=	23,
	CMDQ_ERR_SUSPEND_HW_THREAD_FAIL		=	24,
	CMDQ_ERR_ERR_TASK_IS_NULL			=	25,

	CMDQ_ERR_RELEASE_TASK_IS_NULL		=	26,
	CMDQ_ERR_REMOVE_TASK_IS_NOT_BUSY	=	27,
	CMDQ_ERR_NON_SUPPORT_DISP_MODE		=	28,

	/*m4u convert handle to secure addr error*/
	CMDQ_ERR_ADDR_CONVERT_HANDLE_2_PA	=	1000,
	CMDQ_ERR_ADDR_CONVERT_HANDLE_2_VA	=	1001,
	CMDQ_ERR_M4U_MAP_NONSEC_BUFFER_FAILED	=	1100,
	CMDQ_ERR_M4U_ALLOC_SEC_MVA_FAILED		=	1101,

	/* param check error*/
	CMDQ_ERR_UNKNOWN_ADDR_METADATA_TYPE	=	1400,
	/*Too many secure handle address in one secure task*/
	CMDQ_ERR_TOO_MANY_SEC_HANDLE		=	1401,
		/* security check error*/
	CMDQ_ERR_SECURITY_INVALID_SUBSYS_ID	=	1500,
	CMDQ_ERR_SECURITY_INVALID_SEC_HANDLE		=	1501,
	CMDQ_ERR_SECURITY_INVALID_DAPC_FALG			=	1502,
	CMDQ_ERR_NO_M4U_PORT_CONFIGED				=	1503,
	CMDQ_ERR_NO_DAPC_ENGINE_CONFIGED			=	1504,

	ERROR_STATUS_MAX_COUNT
};


#endif				/* __CMDQ_SEC_TLAPI_H__ */
