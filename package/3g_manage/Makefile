#
# Top level makefile for 3g_manage application
#

include $(TOPDIR)/rules.mk

PKG_NAME:=3g_manage
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/3g_manage
 SECTION:=utils
 CATEGORY:=Utilities
 TITLE:=3g_manage
 DEPENDS:=+libpthread
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

TARGET_CFLAGS += $(FPIC)

define Package/3g_manage/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/3g_manage $(1)/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/power_mode $(1)/bin/
endef

$(eval $(call BuildPackage,3g_manage))
