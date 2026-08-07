################################################################################
#
# optee_os_artifacts_ext
#
################################################################################

OPTEE_OS_ARTIFACTS_EXT_VERSION = 1.0
OPTEE_OS_ARTIFACTS_EXT_SOURCE = local
OPTEE_OS_ARTIFACTS_EXT_SITE = $(BR2_PACKAGE_OPTEE_OS_ARTIFACTS_EXT_SITE)
OPTEE_OS_ARTIFACTS_EXT_SITE_METHOD = local

define OPTEE_OS_ARTIFACTS_EXT_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/optee_armtz
	$(INSTALL) -D -m 0444 $(@D)/*.ta $(TARGET_DIR)/lib/optee_armtz/
endef

$(eval $(generic-package))
