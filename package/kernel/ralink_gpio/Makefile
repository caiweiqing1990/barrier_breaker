#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ralink_gpio
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ralink_gpio
  SUBMENU:=Other modules
  TITLE:=Driver for ralink_gpio
  AUTOLOAD:=$(call AutoLoad,99,ralink_gpio)
  FILES:=$(PKG_BUILD_DIR)/ralink_gpio.ko
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		modules
endef
define Package/$(PKG_NAME)/postinst
#!/bin/sh
/bin/mkdir -p /dev
/bin/mknod /dev/gpio c 252 0
endef

$(eval $(call KernelPackage,ralink_gpio))
