/*
 * Copyright (C) 2010-2011 Avionic Design GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>

#include "remote-control-stub.h"
#include "remote-control.h"
#include "rcrpc.h"

int32_t RPC_IMPL(sound_play_async)(void *priv, const char *filename)
{
	int32_t ret = -ENOSYS;
	g_debug("> %s(priv=%p, filename=%s)", __func__, priv, filename);
	g_debug("< %s() = %d", __func__, ret);
	return ret;
}

int32_t RPC_IMPL(sound_play_sync)(void *priv, const char *filename)
{
	int32_t ret = -ENOSYS;
	g_debug("> %s(priv=%p, filename=%s)", __func__, priv, filename);
	g_debug("< %s() = %d", __func__, ret);
	return ret;
}

int32_t RPC_IMPL(sound_play)(void *priv, const char *uri)
{
	struct sound_manager *sound = remote_control_get_sound_manager(RCPTR(priv));
	int32_t ret = -ENOSYS;

	g_debug("> %s(priv=%p, uri=%s)", __func__, priv, uri);
	ret = sound_manager_play(sound, uri);
	g_debug("< %s() = %d", __func__, ret);
	return ret;
}

int32_t RPC_IMPL(sound_stop)(void *priv)
{
	struct sound_manager *sound = remote_control_get_sound_manager(RCPTR(priv));
	int32_t ret = -ENOSYS;

	g_debug("> %s(priv=%p)", __func__, priv);
	ret = sound_manager_stop(sound);
	g_debug("< %s() = %d", __func__, ret);
	return ret;
}
