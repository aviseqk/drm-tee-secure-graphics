/* 
########################################################
#		sg-drm-info
#
## Authored By: Abhishek
## Version: 1.0
## Description:
## This is a basic utility program written using libdrm
## which prints the DRM device and driver information.
## 
### This program is installed into the root filesystem 
### using the Buildroot external tree
########################################################
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

int main(void)
{

	// Open the primary DRM device node
	int drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (drm_fd < 0) {
		perror("Failed to open /dev/dri/card0");
		return 1;
	}

	// Get the DRM Version
	drmVersion* version = drmGetVersion(drm_fd);
	if (!version) {
		perror("failed to retrieve DRM Driver version information");
		close(drm_fd);
		return 1;
	}
	printf("DRM Driver Information\n");
	printf("----------------------\n");
	printf("Driver:		%s\n", version->name);
	printf("Version:	%d.%d\n", version->version_major, version->version_minor);
	printf("Description:	%s\n", version->desc);
	printf("\n");

	// Retrieve the DRM Resources
	drmModeRes *resources = drmModeGetResources(drm_fd);
	if (!resources) {
		perror("failed to retrieve DRM resources");
		drmFreeVersion(version);
		close(drm_fd);
		return 1;
	}

	printf("DRM Resources:\n");
	printf("--------------\n");
	printf("  Count of Connectors:	%d\n", resources->count_connectors);
	printf("  Count of CRTCs:	%d\n", resources->count_crtcs);
	printf("  Count of Encoders:	%d\n", resources->count_encoders);
	printf("\n");

	// Enumerate Connectors to check connectivity
	for (int i = 0; i < resources->count_connectors; i++) {
		drmModeConnector *conn = drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (!conn) continue;

		if (conn->connection == DRM_MODE_CONNECTED) {
			printf("Connector %d\n", conn->connector_id);
			printf("------------\n");
			printf("Status			: %s\n", "CONNECTED");
			printf("Modes			: %d\n", conn->count_modes);
		
                        if (conn->count_modes > 0) {
			    // printing the preferred resolution
                            printf("Preferred Resolution: %dx%d @%dHz\n",
                                    conn->modes[0].hdisplay,
				    conn->modes[0].vdisplay,
				    conn->modes[0].vrefresh);
                        }
			printf("Available Modes		:\n");
			for (int j=0; j < conn->count_modes; j++) {
				printf("%dx%d @%dHz\t", conn->modes[j].hdisplay, conn->modes[j].vdisplay, conn->modes[j].vrefresh);
			}
			printf("\n");
                } else {
			printf("Connector %d NOT CONNECTED\n", conn->connector_id);
		}
                drmModeFreeConnector(conn);
        }

	// Cleanup
        drmFreeVersion(version);
	drmModeFreeResources(resources);
	close(drm_fd);
	return 0;
}
