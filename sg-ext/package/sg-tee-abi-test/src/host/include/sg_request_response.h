#ifndef SG_REQUEST_RESPONSE_H
#define SG_REQUEST_RESPONSE_H

#define SG_ABI_VERSION		1

#include "sg_drm_metadata.h"

enum sg_decision {
	SG_DECISION_ALLOW = 0,
	SG_DECISION_DENY = 1,
	SG_DECISION_ERROR = 2,
};

enum sg_reason {
	SG_REASON_POLICY_ALLOWED = 0,
	SG_REASON_POLICY_DENIED = 1,
	SG_REASON_INVALID_METADATA = 2,
	SG_REASON_UNSUPPORTED_VERSION = 3,
};

struct sg_request_header {
	uint32_t version;
	uint32_t command;
	uint32_t size;
	uint32_t response_offset;
	uint32_t response_size;
};

struct sg_validate_request {
	struct sg_request_header header;
	struct sg_transaction_metadata metadata;
};

struct sg_validate_response {
	uint32_t version;
	uint32_t decision;
	uint32_t reason;
};

#endif
