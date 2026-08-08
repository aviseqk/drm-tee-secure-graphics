SG_DRM_INFO_VERSION=1.0
SG_DRM_INFO_SITE=$(BR2_EXTERNAL_SG_EXT_PATH)/package/sg-drm-info
SG_DRM_INFO_SITE_METHOD=local
SG_DRM_INFO_DEPENDENCIES = libdrm

define SG_DRM_INFO_BUILD_CMDS
	$(TARGET_MAKE_ENV) \
	$(MAKE) \
		CC="$(TARGET_CC)" \
		PKG_CONFIG="$(PKG_CONFIG_HOST_BINARY)" \
		STAGING_DIR="$(STAGING_DIR)" \
		-C $(@D)
endef

define SG_DRM_INFO_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 \
	$(@D)/sg-drm-info \
	$(TARGET_DIR)/usr/bin/sg-drm-info
endef

$(eval $(generic-package))
