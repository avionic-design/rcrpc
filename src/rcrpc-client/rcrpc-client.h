/*
 * Copyright (C) 2010-2017 Avionic Design GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef RCRPC_CLIENT_H
#define RCRPC_CLIENT_H 1

#include <stdbool.h>
#include <stdint.h>

#if __GNUC__ >= 4
#  define remote_public __attribute__((visibility("default")))
#  define remote_private __attribute__((visibility("hidden")))
#else
#  define remote_public
#  define remote_private
#endif

enum remote_audio_state {
	REMOTE_AUDIO_STATE_UNKNOWN = -1,
	REMOTE_AUDIO_STATE_INACTIVE = 0,
	REMOTE_AUDIO_STATE_HIFI_PLAYBACK_SPEAKER = 10,
	REMOTE_AUDIO_STATE_HIFI_PLAYBACK_HEADSET,
	REMOTE_AUDIO_STATE_VOICECALL_HANDSET = 20,
	REMOTE_AUDIO_STATE_VOICECALL_HEADSET,
	REMOTE_AUDIO_STATE_VOICECALL_SPEAKER,
	REMOTE_AUDIO_STATE_VOICECALL_IP_HANDSET = 30,
	REMOTE_AUDIO_STATE_VOICECALL_IP_HEADSET,
	REMOTE_AUDIO_STATE_VOICECALL_IP_SPEAKER,
	REMOTE_AUDIO_STATE_LINEIN_SPEAKER = 40,
	REMOTE_AUDIO_STATE_LINEIN_HEADSET,
	REMOTE_AUDIO_STATE_MAX
};

enum remote_mixer_control {
	REMOTE_MIXER_CONTROL_UNKNOWN,
	REMOTE_MIXER_CONTROL_PLAYBACK_MASTER,
	REMOTE_MIXER_CONTROL_PLAYBACK_PCM,
	REMOTE_MIXER_CONTROL_PLAYBACK_HEADSET,
	REMOTE_MIXER_CONTROL_PLAYBACK_SPEAKER,
	REMOTE_MIXER_CONTROL_PLAYBACK_HANDSET,
	REMOTE_MIXER_CONTROL_CAPTURE_MASTER,
	REMOTE_MIXER_CONTROL_CAPTURE_HEADSET,
	REMOTE_MIXER_CONTROL_CAPTURE_HANDSET,
	REMOTE_MIXER_CONTROL_CAPTURE_LINE,
	REMOTE_MIXER_CONTROL_MAX
};

enum remote_mixer_input_source {
	REMOTE_MIXER_INPUT_SOURCE_UNKNOWN,
	REMOTE_MIXER_INPUT_SOURCE_HEADSET,
	REMOTE_MIXER_INPUT_SOURCE_HANDSET,
	REMOTE_MIXER_INPUT_SOURCE_LINE,
	REMOTE_MIXER_INPUT_SOURCE_MAX
};

enum remote_card_type {
	REMOTE_CARD_TYPE_UNKNOWN,
	REMOTE_CARD_TYPE_I2C,
	REMOTE_CARD_TYPE_MAX
};

enum remote_handset_icon {
	REMOTE_HANDSET_ICON_PHONE,
	REMOTE_HANDSET_ICON_RADIO,
	REMOTE_HANDSET_ICON_TV,
	REMOTE_HANDSET_ICON_LOGO,
	REMOTE_HANDSET_ICON_MAX
};

enum remote_irq_source {
	REMOTE_IRQ_SOURCE_UNKNOWN,
	REMOTE_IRQ_SOURCE_HOOK,
	REMOTE_IRQ_SOURCE_SMARTCARD,
	REMOTE_IRQ_SOURCE_VOIP,
	REMOTE_IRQ_SOURCE_IO,
	REMOTE_IRQ_SOURCE_MODEM,
	REMOTE_IRQ_SOURCE_HANDSET,
	REMOTE_IRQ_SOURCE_MAX
};

struct remote_voip_account {
	char *proxy;
	uint16_t port;
	char *username;
	char *password;
};

enum remote_event {
	REMOTE_EVENT_CARD,
	REMOTE_EVENT_MODEM,
	REMOTE_EVENT_VOIP,
	REMOTE_EVENT_MAX,
};

typedef void (*remote_event_handler_t)(uint32_t type, void *data);

struct remote_client;

remote_public int remote_init(struct remote_client **clientp,
		const char *hostname, const char *service);
remote_public int remote_exit(struct remote_client *client);

remote_public int remote_register_event_handler(struct remote_client *client,
		enum remote_event queue, remote_event_handler_t handler,
		void *data);
remote_public int remote_unregister_event_handler(struct remote_client *client,
		enum remote_event queue, remote_event_handler_t handler);

remote_public int remote_audio_set_state(void *priv,
		enum remote_audio_state state, bool force);
remote_public int remote_audio_get_state(void *priv,
		enum remote_audio_state *statep);
remote_public int remote_audio_get_volume(void *priv, uint8_t *volume);
remote_public int remote_audio_set_volume(void *priv, uint8_t volume);

remote_public int remote_mixer_set_volume(void *priv,
		enum remote_mixer_control control, uint8_t volume);
remote_public int remote_mixer_get_volume(void *priv,
		enum remote_mixer_control control, uint8_t *volumep);
remote_public int remote_mixer_set_mute(void *priv,
		enum remote_mixer_control control, bool mute);
remote_public int remote_mixer_get_mute(void *priv,
		enum remote_mixer_control control, bool *mutep);
remote_public int remote_mixer_set_input_source(void *priv,
		enum remote_mixer_input_source source);
remote_public int remote_mixer_get_input_source(void *priv,
		enum remote_mixer_input_source *sourcep);
remote_public int remote_mixer_loopback_enable(void *priv, bool enable);
remote_public int remote_mixer_loopback_is_enabled(void *priv, bool *enabled);

remote_public int remote_backlight_enable(void *priv, uint32_t flags);
remote_public int remote_backlight_get(void *priv, uint8_t *brightness);
remote_public int remote_backlight_set(void *priv, uint8_t brightness);

remote_public int remote_lldp_read(void *priv, void *data, size_t size);

remote_public int remote_media_player_start(void *priv);
remote_public int remote_media_player_stop(void *priv);
remote_public int remote_media_player_is_running(void *priv, bool *running);
remote_public int remote_media_player_get_stream(void *priv, char **url);
remote_public int remote_media_player_set_stream(void *priv, const char *url);
remote_public int remote_media_player_set_crop(void *priv, uint16_t left, uint16_t right,
		uint16_t top, uint16_t bottom);
remote_public int remote_media_player_set_output_window(void *priv,
		uint16_t x, uint16_t y, uint16_t width, uint16_t height);
remote_public int remote_media_player_pause(void *priv);
remote_public int remote_media_player_resume(void *priv);
remote_public int remote_media_player_get_duration(void *priv,
		unsigned long *duration);
remote_public int remote_media_player_get_position(void *priv,
		unsigned long *position);
remote_public int remote_media_player_set_position(void *priv,
		unsigned long position);

remote_public int remote_modem_init(void *priv);
remote_public int remote_modem_deinit(void *priv);
remote_public int remote_modem_pick_up(void *priv);
remote_public int remote_modem_hang_up(void *priv);
remote_public int remote_modem_dial(void *priv, const char *number);
remote_public int remote_modem_is_calling(void *priv, bool *status);
remote_public int remote_modem_is_connected(void *priv, bool *status);

remote_public int remote_voip_login(void *priv,
		struct remote_voip_account *account);
remote_public int remote_voip_logout(void *priv);
remote_public int remote_voip_connect_to(void *priv, const char *uri);
remote_public int remote_voip_accept_incoming(void *priv, char **uri);
remote_public int remote_voip_disconnect(void *priv);
remote_public int remote_voip_still_logged_in(void *priv, bool *status);
remote_public int remote_voip_dial(void *priv, uint8_t dtmf);

remote_public int remote_card_get_type(void *priv,
		enum remote_card_type *type);
remote_public int remote_card_read(void *priv, off_t offset, void *buffer,
		size_t size);
remote_public int remote_card_write(void *priv, off_t offset,
		const void *buffer, size_t size);

remote_public int remote_handset_display_clear(void *priv);
remote_public int remote_handset_display_sync(void *priv);
remote_public int remote_handset_display_set_brightness(void *priv,
		unsigned int brightness);
remote_public int remote_handset_keypad_set_brightness(void *priv,
		unsigned int brightness);
remote_public int remote_handset_show_icon(void *priv,
		enum remote_handset_icon id);
remote_public int remote_handset_hide_icon(void *priv,
		enum remote_handset_icon id);
remote_public int remote_handset_show_text(void *priv, unsigned int x,
		unsigned int y, const char *text);
remote_public int remote_handset_hide_text(void *priv, unsigned int x,
		unsigned int y, const char *text);

/*
 * Note: The interrupt status returned by remote_irq_get_mask() is a bitmask
 *       where a set bit means a request is pending for the corresponding
 *       interrupt. *However*, the source parameter that is passed to the
 *       remote_irq_get_info() function does *not* correspond to the bit
 *       position in the mask. Instead, 1 needs to be added to the bit
 *       position in order to obtain the source parameter.
 *
 *       Note that I am not responsible for this blunder. This should never
 *       have happened, but as it is we will likely have to live with it
 *       forever because it is actively in use already.
 *
 *       Also not that remote_irq_get_mask() should really have been called
 *       remote_irq_get_status().
 */
remote_public int remote_irq_enable(void *priv, uint8_t virtkey);
remote_public int remote_irq_get_mask(void *priv, uint32_t *mask);
remote_public int remote_irq_get_info(void *priv,
		enum remote_irq_source source, uint32_t *info);

remote_public int remote_gpio_set(void *priv, unsigned int gpio, int value);
remote_public int remote_gpio_get(void *priv, unsigned int gpio);

remote_public int remote_task_manager_exec(void *priv, const char *command);
remote_public int remote_task_manager_kill(void *priv, int pid, int sig);

remote_public int remote_tuner_set_frequency(void *priv, unsigned long frequency);

remote_public int remote_sound_play(void *priv, const char *url);
remote_public int remote_sound_stop(void *priv);

#endif /* RCRPC_CLIENT_H */
