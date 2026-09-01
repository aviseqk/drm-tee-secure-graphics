/*
########################################################
#		sg-drm-dump
#
## Authored By: Abhishek
## Version: 1.0
##
## Description:
## This is a utility program written using libdrm
## which shows how the DRM/KMS Object Model looks
## like, just dump everything interesting
## It navigates through the whole KMS Object Model
## connector -> encoder -> CRTC -> plane -> FB
## and prints its properties sequentially.

## Eventually, when Secure World will need to validate
## these same properties, this utility will help. 
########################################################
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

const char* connection_name(uint32_t connection);
const char* subpixel_name(uint32_t subpixel);
void print_mode_details(drmModeModeInfo *modes, int count);
void print_binary32(uint32_t value);
static void print_fourcc(uint32_t format);

int main(void)
{

	// Open the primary DRM device node
	int drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (drm_fd < 0) {
		perror("Failed to open /dev/dri/card0");
		return 1;
	}

	// Retrieve the DRM Resources
	drmModeRes *resources = drmModeGetResources(drm_fd);
	if (!resources) {
		perror("failed to retrieve DRM resources");
		close(drm_fd);
		return 1;
	}

	// Connector
	for (int i = 0; i < resources->count_connectors; i++) {
		drmModeConnector *conn = drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (!conn) continue;

		printf("Connector %d\n", conn->connector_id);
		printf("------------\n");
		printf("ID			: %d\n", conn->connector_id);
		printf("Status			: %s\n", connection_name(conn->connection));
		printf("Physical Size(HxW, mm)	: %dx%d\n", conn->mmHeight, conn->mmWidth);
		printf("Number of modes		: %d\n", conn->count_modes);
		printf("Encoder ID		: %d\n", conn->encoder_id);
		printf("Connector Type		: %s\n", drmModeGetConnectorTypeName(conn->connector_type));
		printf("Subpixel Order		: %s\n", subpixel_name(conn->subpixel));
		
                if (conn->count_modes > 0) {
		    // printing the preferred resolution
			printf("Preferred Resolution: %dx%d @%dHz\n",
				conn->modes[0].hdisplay,
				conn->modes[0].vdisplay,
				conn->modes[0].vrefresh);
		}
		
		printf("Available Modes		:\n");
		print_mode_details(conn->modes, conn->count_modes);

		drmModeObjectProperties *props = drmModeObjectGetProperties(drm_fd, conn->connector_id, DRM_MODE_OBJECT_CONNECTOR);

		if (!props) {
			perror("drmModeObjectGetProperties");
		} else {
			printf("Connector-%" PRIu32 "\n\t#properties: %d\n", conn->connector_id, props->count_props);

			for (uint32_t j = 0; j < props->count_props; j++) {
				
				uint32_t prop_id = props->props[j];
				uint64_t prop_value = props->prop_values[j];

				drmModePropertyRes *property = drmModeGetProperty(drm_fd, prop_id);
				
				if (!property)
					continue;

				printf("Property (ID: %u): %s\n", prop_id, property->name);
				printf("       Value: %" PRIu64 "\n", prop_value);
				printf("       flags: ");
				print_binary32(property->flags);

				drmModeFreeProperty(property);

			}
		}
		drmModeFreeObjectProperties(props);
		drmModeFreeConnector(conn);

        }


	// CRTC
	for (int i = 0; i < resources->count_crtcs; i++) {
		 drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, resources->crtcs[i]);

		 if (!crtc)
			 continue;

		 printf("CRTC %d\n", crtc->crtc_id);
		 printf("id \t fb \t active \t pos \t size\n");
		 printf("%d \t %d \t %d \t (%d,%d) \t (%dx%d)\n",
				 crtc->crtc_id, crtc->buffer_id, crtc->mode_valid,
				 crtc->x, crtc->y,
				 crtc->width, crtc->height);

		print_mode_details(&crtc->mode, 1);

		drmModeObjectProperties *props = drmModeObjectGetProperties(drm_fd, crtc->crtc_id, DRM_MODE_OBJECT_CRTC);

		if (!props) {
			perror("drmModeObjectGetProperties");
		} else {
			printf("Crtc-%" PRIu32 "\n\t#properties: %d\n", crtc->crtc_id, props->count_props);

			for (uint32_t j = 0; j < props->count_props; j++) {
				
				uint32_t prop_id = props->props[j];
				uint64_t prop_value = props->prop_values[j];

				drmModePropertyRes *property = drmModeGetProperty(drm_fd, prop_id);
				
				if (!property)
					continue;

				printf("Property (ID: %u): %s\n", prop_id, property->name);
				printf("       Value: %" PRIu64 "\n", prop_value);
				printf("       flags: ");
				print_binary32(property->flags);

				drmModeFreeProperty(property);

			}
		}
		drmModeFreeObjectProperties(props);
		drmModeFreeCrtc(crtc);

	}

	// Plane
	// Enable primary + cursor + overlay planes
	if (drmSetClientCap(drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
		perror("drmSetClientCap(UNIVERSAL_PLANES)");
		goto cleanup;
	}


	drmModePlaneRes *planes = drmModeGetPlaneResources(drm_fd);

	if (!planes) {
		perror("failed to retrieve plane resources");
		goto cleanup;
	}

	printf("Planes:\n");
	printf("id\t crtc \t fb \t CRTC x,y \t x,y \t gamma size \t possible crtcs\n");

	for (uint32_t i = 0; i < planes->count_planes; i++) {
		drmModePlane *plane = drmModeGetPlane(drm_fd, planes->planes[i]);

		if (!plane)
			continue;
		
		printf("%d\t%d\t%d\t%d,%d\t\t%d,%d\t\t%d\t0x%08" PRIx32 "\n", plane->plane_id, plane->crtc_id, plane->fb_id,
				plane->crtc_x, plane->crtc_y, plane->x, plane->y,
				plane->gamma_size, plane->possible_crtcs);
		
		printf("\tformats:\n");
		for (int j = 0; j < plane->count_formats; j++) {
			printf(" \t\t");
			print_fourcc(plane->formats[j]);
			printf("\n");
		}

		drmModeObjectProperties *props = drmModeObjectGetProperties(drm_fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);

		if (!props) {
			perror("drmModeObjectGetProperties");
		} else {
			printf(" #properties: %d\n", props->count_props);

			for (uint32_t j = 0; j < props->count_props; j++) {
				
				uint32_t prop_id = props->props[j];
				uint64_t prop_value = props->prop_values[j];

				drmModePropertyRes *property = drmModeGetProperty(drm_fd, prop_id);
				
				if (!property)
					continue;

				printf("Property (ID: %u): %s\n", prop_id, property->name);
				printf("       Value: %" PRIu64 "\n", prop_value);
				printf("       flags: ");
				print_binary32(property->flags);

				drmModeFreeProperty(property);

			}
		}
		drmModeFreeObjectProperties(props);
		drmModeFreePlane(plane);
		printf("\n");
	}

	drmModeFreePlaneResources(planes);

	// Cleanup
cleanup:
	drmModeFreeResources(resources);
	close(drm_fd);
	return 0;
}

void print_mode_details(drmModeModeInfo *modes, int count)
{
	printf("modes:\n\n");
	printf("index\t name \t refresh(Hz) \t hdisp \t htot \t vdisp \t vtot\n");

	for (int i = 0; i < count; i++) {
		printf("#%d \t %s \t %d \t %d \t %d \t %d \t %d\n",
				i, modes[i].name, modes[i].vrefresh, modes[i].hdisplay, 
				modes[i].htotal, modes[i].vdisplay, modes[i].vtotal);
	}
}

const char* connection_name(uint32_t connection)
{
	switch (connection) {
	case DRM_MODE_CONNECTED:
		return "connected";
	case DRM_MODE_DISCONNECTED:
		return "disconnected";
	case DRM_MODE_UNKNOWNCONNECTION:
		return "unknownconnection";
	default:
		return "unknown";
	}
}

const char* subpixel_name(uint32_t subpixel)
{
	switch (subpixel) {
		case DRM_MODE_SUBPIXEL_UNKNOWN:
			return "DRM_MODE_SUBPIXEL_UNKNOWN";
		case DRM_MODE_SUBPIXEL_HORIZONTAL_RGB:
			return "DRM_MODE_SUBPIXEL_HORIZONTAL_RGB";
		case DRM_MODE_SUBPIXEL_HORIZONTAL_BGR:
			return "DRM_MODE_SUBPIXEL_HORIZONTAL_BGR";
		case DRM_MODE_SUBPIXEL_VERTICAL_RGB:
			return "DRM_MODE_SUBPIXEL_VERTICAL_RGB";
		case DRM_MODE_SUBPIXEL_VERTICAL_BGR:
			return "DRM_MODE_SUBPIXEL_VERTICAL_BGR";
		case DRM_MODE_SUBPIXEL_NONE:
			return "DRM_MODE_SUBPIXEL_NONE";
		default:
			return "unknown";
	}
}

void print_binary32(uint32_t value)
{
    for (int i = 31; i >= 0; i--) {
        printf("%u", (value >> i) & 1U);

        if (i % 4 == 0)
            printf(" ");
    }

    printf("\n");
}

static void print_fourcc(uint32_t format)
{
    printf("%c%c%c%c",
           format & 0xff,
           (format >> 8) & 0xff,
           (format >> 16) & 0xff,
           (format >> 24) & 0xff);
}
