#ifndef SG_REQUEST_RESPONSE_H
#define SG_REQUEST_RESPONSE_H

#define SG_ABI_VERSION		1

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "sg_drm_metadata.h"

#include <tee_internal_api.h>

/* Request, Response & {De}Serialization Structures - Helper and Core */

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

struct sg_validate_response {
	uint32_t version;
	uint32_t decision;
	uint32_t reason;
};

struct sg_reader {
	const uint8_t *buffer;
	size_t size;
	size_t offset;
};

struct sg_writer {
	uint8_t *buffer;
	size_t size;
	size_t offset;
};

/* Request, Response & {De}Serialization Functions - Helper and Core */

void print_serialized_bytes(uint8_t *buf, uint32_t req_size, uint32_t offset);

void print_sg_validate_request(const struct sg_validate_request *request);

TEE_Result sg_serialize_response(struct sg_writer *writer,
		const struct sg_validate_response *response);

TEE_Result sg_deserialize_header(
		struct sg_reader *reader,
		struct sg_request_header *header);

TEE_Result sg_deserialize_transaction(
		struct sg_reader *reader,
		struct sg_transaction_metadata *metadata);

#endif
