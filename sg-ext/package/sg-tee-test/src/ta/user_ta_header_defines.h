#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

// to get the TA UUID definition
#include <sg_tee_test_ta.h>

#define TA_UUID			TA_SG_TEE_TEST_UUID

#define TA_FLAGS		(TA_FLAG_DEVICE_ENUM)

#define TA_STACK_SIZE		(2 * 1024)

#define TA_DATA_SIZE		(32 * 1024)

#define TA_VERSION	"1.0"

#define TA_DESCRIPTION	"Secure Graphics TEE Test Application"

#endif

