/* Generated by wayland-scanner 1.13.0 */

#ifndef KEYBOARD_CONFIGURATION_UNSTABLE_V1_CLIENT_PROTOCOL_H
#define KEYBOARD_CONFIGURATION_UNSTABLE_V1_CLIENT_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @page page_keyboard_configuration_unstable_v1 The keyboard_configuration_unstable_v1 protocol
 * @section page_ifaces_keyboard_configuration_unstable_v1 Interfaces
 * - @subpage page_iface_zcr_keyboard_configuration_v1 - extends wl_keyboard with events for device configuration change
 * - @subpage page_iface_zcr_keyboard_device_configuration_v1 - extension of wl_keyboard protocol
 * @section page_copyright_keyboard_configuration_unstable_v1 Copyright
 * <pre>
 *
 * Copyright 2016 The Chromium Authors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * </pre>
 */
struct wl_keyboard;
struct zcr_keyboard_configuration_v1;
struct zcr_keyboard_device_configuration_v1;

/**
 * @page page_iface_zcr_keyboard_configuration_v1 zcr_keyboard_configuration_v1
 * @section page_iface_zcr_keyboard_configuration_v1_desc Description
 *
 * Allows a wl_keyboard to notify device configuration change events of
 * the keyboard to the client.
 *
 * Warning! The protocol described in this file is experimental and
 * backward incompatible changes may be made. Backward compatible changes
 * may be added together with the corresponding uinterface version bump.
 * Backward incompatible changes are done by bumping the version number in
 * the protocol and uinterface names and resetting the interface version.
 * Once the protocol is to be declared stable, the 'z' prefix and the
 * version number in the protocol and interface names are removed and the
 * interface version number is reset.
 * @section page_iface_zcr_keyboard_configuration_v1_api API
 * See @ref iface_zcr_keyboard_configuration_v1.
 */
/**
 * @defgroup iface_zcr_keyboard_configuration_v1 The zcr_keyboard_configuration_v1 interface
 *
 * Allows a wl_keyboard to notify device configuration change events of
 * the keyboard to the client.
 *
 * Warning! The protocol described in this file is experimental and
 * backward incompatible changes may be made. Backward compatible changes
 * may be added together with the corresponding uinterface version bump.
 * Backward incompatible changes are done by bumping the version number in
 * the protocol and uinterface names and resetting the interface version.
 * Once the protocol is to be declared stable, the 'z' prefix and the
 * version number in the protocol and interface names are removed and the
 * interface version number is reset.
 */
extern const struct wl_interface zcr_keyboard_configuration_v1_interface;
/**
 * @page page_iface_zcr_keyboard_device_configuration_v1 zcr_keyboard_device_configuration_v1
 * @section page_iface_zcr_keyboard_device_configuration_v1_desc Description
 *
 * The zcr_keyboard_device_configuration_v1 interface extends the wl_keyboard
 * interface with events to describe device configuration of a keyboard.
 * @section page_iface_zcr_keyboard_device_configuration_v1_api API
 * See @ref iface_zcr_keyboard_device_configuration_v1.
 */
/**
 * @defgroup iface_zcr_keyboard_device_configuration_v1 The zcr_keyboard_device_configuration_v1 interface
 *
 * The zcr_keyboard_device_configuration_v1 interface extends the wl_keyboard
 * interface with events to describe device configuration of a keyboard.
 */
extern const struct wl_interface zcr_keyboard_device_configuration_v1_interface;

#ifndef ZCR_KEYBOARD_CONFIGURATION_V1_ERROR_ENUM
#define ZCR_KEYBOARD_CONFIGURATION_V1_ERROR_ENUM
enum zcr_keyboard_configuration_v1_error {
	/**
	 * the keyboard already has a device configuration object associated
	 */
	ZCR_KEYBOARD_CONFIGURATION_V1_ERROR_DEVICE_CONFIGURATION_EXISTS = 0,
};
#endif /* ZCR_KEYBOARD_CONFIGURATION_V1_ERROR_ENUM */

#define ZCR_KEYBOARD_CONFIGURATION_V1_GET_KEYBOARD_DEVICE_CONFIGURATION 0


/**
 * @ingroup iface_zcr_keyboard_configuration_v1
 */
#define ZCR_KEYBOARD_CONFIGURATION_V1_GET_KEYBOARD_DEVICE_CONFIGURATION_SINCE_VERSION 1

/** @ingroup iface_zcr_keyboard_configuration_v1 */
static inline void
zcr_keyboard_configuration_v1_set_user_data(struct zcr_keyboard_configuration_v1 *zcr_keyboard_configuration_v1, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zcr_keyboard_configuration_v1, user_data);
}

/** @ingroup iface_zcr_keyboard_configuration_v1 */
static inline void *
zcr_keyboard_configuration_v1_get_user_data(struct zcr_keyboard_configuration_v1 *zcr_keyboard_configuration_v1)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zcr_keyboard_configuration_v1);
}

static inline uint32_t
zcr_keyboard_configuration_v1_get_version(struct zcr_keyboard_configuration_v1 *zcr_keyboard_configuration_v1)
{
	return wl_proxy_get_version((struct wl_proxy *) zcr_keyboard_configuration_v1);
}

/** @ingroup iface_zcr_keyboard_configuration_v1 */
static inline void
zcr_keyboard_configuration_v1_destroy(struct zcr_keyboard_configuration_v1 *zcr_keyboard_configuration_v1)
{
	wl_proxy_destroy((struct wl_proxy *) zcr_keyboard_configuration_v1);
}

/**
 * @ingroup iface_zcr_keyboard_configuration_v1
 *
 * Create keyboard_device_configuration object.
 * See zcr_keyboard_device_configuration interface for details.
 * If the given wl_keyboard object already has a device configuration
 * object associated, the keyboard_device_configuration_exists protocol
 * error is raised.
 */
static inline struct zcr_keyboard_device_configuration_v1 *
zcr_keyboard_configuration_v1_get_keyboard_device_configuration(struct zcr_keyboard_configuration_v1 *zcr_keyboard_configuration_v1, struct wl_keyboard *keyboard)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) zcr_keyboard_configuration_v1,
			 ZCR_KEYBOARD_CONFIGURATION_V1_GET_KEYBOARD_DEVICE_CONFIGURATION, &zcr_keyboard_device_configuration_v1_interface, NULL, keyboard);

	return (struct zcr_keyboard_device_configuration_v1 *) id;
}

#ifndef ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_KEYBOARD_TYPE_ENUM
#define ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_KEYBOARD_TYPE_ENUM
/**
 * @ingroup iface_zcr_keyboard_device_configuration_v1
 * keyboard types
 */
enum zcr_keyboard_device_configuration_v1_keyboard_type {
	/**
	 * Physical keyboard
	 */
	ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_KEYBOARD_TYPE_PHYSICAL = 0,
	/**
	 * Virtual keyboard
	 */
	ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_KEYBOARD_TYPE_VIRTUAL = 1,
};
#endif /* ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_KEYBOARD_TYPE_ENUM */

/**
 * @ingroup iface_zcr_keyboard_device_configuration_v1
 * @struct zcr_keyboard_device_configuration_v1_listener
 */
struct zcr_keyboard_device_configuration_v1_listener {
	/**
	 * keyboard type changed
	 *
	 * Notification that the user is switching to new keyboard type.
	 *
	 * If this event is not received, the cleint has to assume a
	 * physical keyboard is in use.
	 * @param type new device type
	 */
	void (*type_change)(void *data,
			    struct zcr_keyboard_device_configuration_v1 *zcr_keyboard_device_configuration_v1,
			    uint32_t type);
};

/**
 * @ingroup iface_zcr_keyboard_device_configuration_v1
 */
static inline int
zcr_keyboard_device_configuration_v1_add_listener(struct zcr_keyboard_device_configuration_v1 *zcr_keyboard_device_configuration_v1,
						  const struct zcr_keyboard_device_configuration_v1_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) zcr_keyboard_device_configuration_v1,
				     (void (**)(void)) listener, data);
}

#define ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_DESTROY 0

/**
 * @ingroup iface_zcr_keyboard_device_configuration_v1
 */
#define ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_TYPE_CHANGE_SINCE_VERSION 1

/**
 * @ingroup iface_zcr_keyboard_device_configuration_v1
 */
#define ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_DESTROY_SINCE_VERSION 1

/** @ingroup iface_zcr_keyboard_device_configuration_v1 */
static inline void
zcr_keyboard_device_configuration_v1_set_user_data(struct zcr_keyboard_device_configuration_v1 *zcr_keyboard_device_configuration_v1, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) zcr_keyboard_device_configuration_v1, user_data);
}

/** @ingroup iface_zcr_keyboard_device_configuration_v1 */
static inline void *
zcr_keyboard_device_configuration_v1_get_user_data(struct zcr_keyboard_device_configuration_v1 *zcr_keyboard_device_configuration_v1)
{
	return wl_proxy_get_user_data((struct wl_proxy *) zcr_keyboard_device_configuration_v1);
}

static inline uint32_t
zcr_keyboard_device_configuration_v1_get_version(struct zcr_keyboard_device_configuration_v1 *zcr_keyboard_device_configuration_v1)
{
	return wl_proxy_get_version((struct wl_proxy *) zcr_keyboard_device_configuration_v1);
}

/**
 * @ingroup iface_zcr_keyboard_device_configuration_v1
 */
static inline void
zcr_keyboard_device_configuration_v1_destroy(struct zcr_keyboard_device_configuration_v1 *zcr_keyboard_device_configuration_v1)
{
	wl_proxy_marshal((struct wl_proxy *) zcr_keyboard_device_configuration_v1,
			 ZCR_KEYBOARD_DEVICE_CONFIGURATION_V1_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) zcr_keyboard_device_configuration_v1);
}

#ifdef  __cplusplus
}
#endif

#endif