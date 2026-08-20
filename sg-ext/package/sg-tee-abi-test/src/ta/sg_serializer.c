#include <sg_request_response.h>

static uint32_t sg_cpu_to_le32(uint32_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        return value;
#else
        return __builtin_bswap32(value);
#endif
}

static TEE_Result sg_write_u32(struct sg_writer *writer, uint32_t value)
{
	uint32_t v = sg_cpu_to_le32(value);

	if (writer->offset > writer->size ||
            writer->size - writer->offset < sizeof(v))
                return TEE_ERROR_SHORT_BUFFER;

	memcpy(writer->buffer + writer->offset, &v, sizeof(v));

	writer->offset += sizeof(v);

	return TEE_SUCCESS;
}

static TEE_Result sg_read_u32(struct sg_reader *reader, uint32_t *value)
{
	uint32_t v;

	if (reader->offset > reader->size ||
			reader->size - reader->offset < sizeof(v))
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(&v, reader->buffer + reader->offset, sizeof(v));

	reader->offset += sizeof(v);

	*value = v;

	return TEE_SUCCESS;
}

static TEE_Result sg_read_u64(struct sg_reader *reader, uint64_t *value)
{
	uint64_t v;

	if (reader->offset > reader->size ||
			reader->size - reader->offset < sizeof(v))
		return TEE_ERROR_BAD_PARAMETERS;

	memcpy(&v, reader->buffer + reader->offset, sizeof(v));

	reader->offset += sizeof(v);

	*value = v;

	return TEE_SUCCESS;
}

static TEE_Result sg_read_s32(struct sg_reader *reader, int32_t *value)
{
        uint32_t v;
        TEE_Result res;

        res = sg_read_u32(reader, &v);
        if (res != TEE_SUCCESS)
                return res;

        *value = (int32_t)v;

        return TEE_SUCCESS;
}

TEE_Result sg_serialize_response(struct sg_writer *writer,
		const struct sg_validate_response *response)
{
	
	TEE_Result res;

	res = sg_write_u32(writer, response->version);
	if (res != TEE_SUCCESS)
		return res;
	
	res = sg_write_u32(writer, response->decision);
	if (res != TEE_SUCCESS)
		return res;
	
	res = sg_write_u32(writer, response->reason);
	if (res != TEE_SUCCESS)
		return res;

	return TEE_SUCCESS;
}

static TEE_Result sg_deserialize_connector(
		struct sg_reader *reader,
		struct sg_connector_metadata *connector)
{
	TEE_Result res;

	res = sg_read_u32(reader, &connector->connector_id);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &connector->crtc_id);
	if (res != TEE_SUCCESS)
		return res;

	return TEE_SUCCESS;
}

TEE_Result sg_deserialize_header(struct sg_reader *reader,
		struct sg_request_header *header)
{
	TEE_Result res;
	
	res = sg_read_u32(reader, &header->version);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &header->command);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &header->size);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &header->response_offset);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &header->response_size);
	if (res != TEE_SUCCESS)
		return res;


	return TEE_SUCCESS;
}



static TEE_Result sg_deserialize_crtc(
		struct sg_reader *reader,
		struct sg_crtc_metadata *crtc)
{
	TEE_Result res;

	res = sg_read_u32(reader, &crtc->crtc_id);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &crtc->enable);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &crtc->active);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &crtc->mode_width);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &crtc->mode_height);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &crtc->refresh_millihz);
	if (res != TEE_SUCCESS)
		return res;

	return TEE_SUCCESS;
}

static TEE_Result sg_deserialize_plane(
		struct sg_reader *reader,
		struct sg_plane_metadata *plane)
{
	TEE_Result res;

	res = sg_read_u32(reader, &plane->plane_id);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->crtc_id);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->fb_id);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->fb_width);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->fb_height);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->format);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u64(reader, &plane->modifier);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->src_x);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->src_y);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->src_width);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->src_height);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_s32(reader, &plane->dst_x);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_s32(reader, &plane->dst_y);
	if (res != TEE_SUCCESS)
		return res;
	
	res = sg_read_u32(reader, &plane->dst_width);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->dst_height);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &plane->num_fb_planes);
	if (res != TEE_SUCCESS)
		return res;

	for (uint32_t i = 0; i < plane->num_fb_planes; i++){
		res = sg_read_u32(reader, &plane->buffers[i].security_class);
		if (res != TEE_SUCCESS)
			return res;
	}

	return TEE_SUCCESS;
}

TEE_Result sg_deserialize_transaction(
		struct sg_reader *reader,
		struct sg_transaction_metadata *metadata)
{
	TEE_Result res;

	res = sg_read_u32(reader, &metadata->version);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &metadata->num_crtcs);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &metadata->num_connectors);
	if (res != TEE_SUCCESS)
		return res;

	res = sg_read_u32(reader, &metadata->num_planes);
	if (res != TEE_SUCCESS)
		return res;

	if (metadata->num_crtcs > SG_MAX_CRTCS || 
	    metadata->num_connectors > SG_MAX_CONNECTORS ||
	    metadata->num_planes > SG_MAX_PLANES)
		return TEE_ERROR_BAD_PARAMETERS;


	
	for (uint32_t i = 0; i < metadata->num_crtcs; i++) {
		res = sg_deserialize_crtc(reader, &metadata->crtcs[i]);
		if (res != TEE_SUCCESS)
			return res;
	}

	for (uint32_t i = 0; i < metadata->num_connectors; i++) {
		res = sg_deserialize_connector(reader, &metadata->connectors[i]);
		if (res != TEE_SUCCESS)
			return res;
	}

	for (uint32_t i = 0; i < metadata->num_planes; i++) {
		res = sg_deserialize_plane(reader, &metadata->planes[i]);
		if (res != TEE_SUCCESS)
			return res;
	}

	return TEE_SUCCESS;

}


void print_serialized_bytes(uint8_t *buf, uint32_t req_size, uint32_t offset)
{
    printf("serialized request size: %u\n", req_size);
    printf("response offset: %u\n", offset);

    printf("\n\nhex dump:\n");

    for (size_t i = 0; i < req_size; i++) {
	printf("%02X ", buf[i]);

	if ( (i + 1) % 8 == 0)
	    printf("\n");
    }
}


void print_sg_validate_request(const struct sg_validate_request *request)
{
	const struct sg_request_header *h = &request->header;
	const struct sg_transaction_metadata *m = &request->metadata;

	IMSG("\n========== SG VALIDATE REQUEST ==========\n");

	/* Request header */
	IMSG("---- Request Header ----\n");
	IMSG("version:        %u\n", h->version);
	IMSG("command:        %u\n", h->command);
	IMSG("size:           %u\n", h->size);
	IMSG("response_offset:%u\n", h->response_offset);
	IMSG("response_size:  %u\n", h->response_size);

	/* Transaction metadata */
	IMSG("\n---- Transaction Metadata ----\n");
	IMSG("version:        %u\n", m->version);
	IMSG("num_crtcs:      %u\n", m->num_crtcs);
	IMSG("num_connectors: %u\n", m->num_connectors);
	IMSG("num_planes:     %u\n", m->num_planes);

	/* CRTCs */
	IMSG("\n---- CRTCs ----\n");

	for (uint32_t i = 0; i < m->num_crtcs && i < SG_MAX_CRTCS; i++) {
		const struct sg_crtc_metadata *c = &m->crtcs[i];

		IMSG("CRTC[%u]\n", i);
		IMSG("  crtc_id:       %u\n", c->crtc_id);
		IMSG("  enable:        %u\n", c->enable);
		IMSG("  active:        %u\n", c->active);
		IMSG("  mode_width:    %u\n", c->mode_width);
		IMSG("  mode_height:   %u\n", c->mode_height);
		IMSG("  refresh_millihz:%u\n", c->refresh_millihz);
	}

	/* Connectors */
	IMSG("\n---- Connectors ----\n");

	for (uint32_t i = 0;
	     i < m->num_connectors && i < SG_MAX_CONNECTORS;
	     i++) {
		const struct sg_connector_metadata *c = &m->connectors[i];

		IMSG("CONNECTOR[%u]\n", i);
		IMSG("  connector_id:  %u\n", c->connector_id);
		IMSG("  crtc_id:       %u\n", c->crtc_id);
	}

	/* Planes */
	IMSG("\n---- Planes ----\n");

	for (uint32_t i = 0;
	     i < m->num_planes && i < SG_MAX_PLANES;
	     i++) {
		const struct sg_plane_metadata *p = &m->planes[i];

		IMSG("PLANE[%u]\n", i);
		IMSG("  plane_id:      %u\n", p->plane_id);
		IMSG("  crtc_id:       %u\n", p->crtc_id);
		IMSG("  fb_id:         %u\n", p->fb_id);
		IMSG("  fb_width:      %u\n", p->fb_width);
		IMSG("  fb_height:     %u\n", p->fb_height);
		IMSG("  format:        %u\n", p->format);
		IMSG("  modifier:      0x%016llx\n",
		     (unsigned long long)p->modifier);

		IMSG("  src_x:         %u\n", p->src_x);
		IMSG("  src_y:         %u\n", p->src_y);
		IMSG("  src_width:     %u\n", p->src_width);
		IMSG("  src_height:    %u\n", p->src_height);

		IMSG("  dst_x:         %d\n", p->dst_x);
		IMSG("  dst_y:         %d\n", p->dst_y);
		IMSG("  dst_width:     %u\n", p->dst_width);
		IMSG("  dst_height:    %u\n", p->dst_height);

		IMSG("  num_fb_planes: %u\n", p->num_fb_planes);

		/* Buffer metadata */
		for (uint32_t j = 0;
		     j < p->num_fb_planes && j < SG_MAX_FB_PLANES;
		     j++) {
			const struct sg_buffer_metadata *b = &p->buffers[j];

			IMSG("  BUFFER[%u]\n", j);
			IMSG("    security_class: %u\n",
			     b->security_class);
		}
	}

	IMSG("\n========== END SG VALIDATE REQUEST ==========\n");
}
