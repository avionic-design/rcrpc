/*
 * Copyright (C) 2010-2012 Avionic Design GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "remote-control-client.h"

#define LLDP_MAX_FRAME_SIZE 1500

/*
 * "help" command
 */
static int exec_help(struct cli *cli, int argc, char *argv[])
{
	if (argc < 2) {
		const struct cli_command_info *cmd;

		printf("Commands:\n");

		command_foreach (cmd)
			printf("  %-25s %s\n", cmd->name, cmd->summary);

		printf("\n");
		printf("(specify help <command> for details about the command)\n");
		printf("\n");
	} else {
		const struct cli_command_info *cmd;

		command_foreach (cmd) {
			if (strcmp(argv[1], cmd->name) == 0) {
				printf("%s - %s\n\n", cmd->name,
						cmd->summary);

				if (cmd->help)
					printf("%s\n\n", cmd->help);
				else if (cmd->helpcmd)
					cmd->helpcmd();

				break;
			}
		}
	}

	return 0;
}

const struct cli_command_info cmd_help _command_ = {
	.name = "help",
	.summary = "print global or command-specific help",
	.help = NULL,
	.options = NULL,
	.exec = exec_help,
};

/*
 * "audio-volume" command
 */
static int exec_audio_volume(struct cli *cli, int argc, char *argv[])
{
	int err;

	if (argc == 1) {
		uint8_t volume = 0;

		err = remote_audio_get_volume(cli->client, &volume);
		if (err < 0) {
			printf("Current volume: Unknown\n");
		} else {
			printf("Current volume: %d\n", volume);
		}
	}
	else if (argc == 2) {
		int32_t volume;

		volume = atol(argv[1]);
		if (volume < 0 || volume > 255)
			return -EINVAL;
		err = remote_audio_set_volume(cli->client, (uint8_t)volume);
	} else {
		printf("Invalid argument count, try help\n");
		err = -EINVAL;
	}

	return err;
}

const struct cli_command_info cmd_audio_volume _command_ = {
	.name = "audio-volume",
	.summary = "get or set the current audio volume",
	.help = NULL,
	.options = NULL,
	.exec = exec_audio_volume
};

/*
 * "audio-state" command
 */
static int exec_audio_state(struct cli *cli, int argc, char *argv[])
{
	enum remote_audio_state state = REMOTE_AUDIO_STATE_UNKNOWN;
	char name[32];
	int err;
	int i;

	switch (argc) {
	case 1:
		err = remote_audio_get_state(cli->client, &state);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
		err = audio_state_name(state, name, sizeof(name));
		if (err < 0)
			printf("current state: unknown (id=%d)\n", state);
		else
			printf("current state: %s\n", name);
		break;
	case 2:
		state = parse_audio_state(argv[1]);
		err = remote_audio_set_state(cli->client, state, true);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
		break;
	default:
		printf("ERROR: Invalid argument, valid states are:\n");
		for (i = 0; i < REMOTE_AUDIO_STATE_MAX; i++) {
			err = audio_state_name(state, name, sizeof(name));
			if (err < 0)
				break;
			printf("   %s\n", name);
		}
		return -EINVAL;
	}

	return 0;
}

static void help_audio_state()
{
	printf("Possible audio-states are:\n");
	audio_state_print_names();
}

const struct cli_command_info cmd_audio_state _command_ = {
	.name = "audio-state",
	.summary = "get or set the current audio state",
	.help = NULL,
	.options = NULL,
	.exec = exec_audio_state,
	.helpcmd = help_audio_state
};

/*
 * "mixer-volume" command
 */
static int exec_mixer_volume(struct cli *cli, int argc, char *argv[])
{
	enum remote_mixer_control control = REMOTE_MIXER_CONTROL_UNKNOWN;
	int err;

	if (argc < 2)
		return -EINVAL;

	control = parse_mixer_control(argv[1]);

	if (argc < 3) {
		uint8_t volume = 0;

		err = remote_mixer_get_volume(cli->client, control, &volume);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}

		printf("volume: %u\n", volume);
	} else {
		unsigned long volume = 0;
		char *end = NULL;

		volume = strtoul(argv[2], &end, 0);
		if ((end == argv[2]) || (volume > 255)) {
			printf("invalid volume: %s\n", argv[2]);
			return -EINVAL;
		}

		err = remote_mixer_set_volume(cli->client, control, volume);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	}

	return 0;
}

const struct cli_command_info cmd_mixer_volume _command_ = {
	.name = "mixer-volume",
	.summary = "access audio volume",
	.help = NULL,
	.options = NULL,
	.exec = exec_mixer_volume,
};

/*
 * "mixer-mute" command
 */
static int exec_mixer_mute(struct cli *cli, int argc, char *argv[])
{
	enum remote_mixer_control control = REMOTE_MIXER_CONTROL_UNKNOWN;
	int err;

	if (argc < 2)
		return -EINVAL;

	control = parse_mixer_control(argv[1]);

	if (argc < 3) {
		bool mute = false;

		err = remote_mixer_get_mute(cli->client, control, &mute);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}

		printf("muted: %s\n", mute ? "yes" : "no");
	} else {
		bool mute = false;

		err = parse_bool(argv[2], &mute);
		if (err < 0) {
			printf("Invalid boolean value '%s'.\n", argv[2]);
			return -EINVAL;
		}

		err = remote_mixer_set_mute(cli->client, control, mute);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	}

	return 0;
}

const struct cli_command_info cmd_mixer_mute _command_ = {
	.name = "mixer-mute",
	.summary = "mute or unmute audio control",
	.help = NULL,
	.options = NULL,
	.exec = exec_mixer_mute,
};

/*
 * "mixer-input" command
 */
static int exec_mixer_input(struct cli *cli, int argc, char *argv[])
{
	int err;

	if (argc < 2) {
		enum remote_mixer_input_source input = REMOTE_MIXER_INPUT_SOURCE_UNKNOWN;
		char buffer[16];

		err = remote_mixer_get_input_source(cli->client, &input);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}

		err = mixer_input_source_name(input, buffer, sizeof(buffer));
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}

		printf("input source: %s\n", buffer);
	} else {
		enum remote_mixer_input_source input;

		input = parse_mixer_input_source(argv[1]);

		err = remote_mixer_set_input_source(cli->client, input);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	}

	return 0;
}

const struct cli_command_info cmd_mixer_input _command_ = {
	.name = "mixer-input",
	.summary = "access audio input source",
	.help = NULL,
	.options = NULL,
	.exec = exec_mixer_input,
};

/*
 * "mixer-loopback" command
 */
static int exec_mixer_loopback(struct cli *cli, int argc, char *argv[])
{
	bool enable = false;
	int err;

	if (argc < 2) {
		err = remote_mixer_loopback_is_enabled(cli->client, &enable);
		if (err < 0) {
			printf("ERROR: %s\n", strerror(-err));
			return err;
		}
	} else {
		err = parse_bool(argv[1], &enable);
		if (err < 0) {
			printf("Invalid boolean value '%s'.\n", argv[1]);
			return -EINVAL;
		}

		err = remote_mixer_loopback_enable(cli->client, enable);
		if ((err < 0) && (err != -EBUSY) && (err != -ESRCH)) {
			printf("ERROR: %s\n", strerror(-err));
			return err;
		}
	}

	printf("loopback %sabled\n", enable ? "en" : "dis");
	return 0;
}

const struct cli_command_info cmd_mixer_loopback _command_ = {
	.name = "mixer-loopback",
	.summary = "control audio loopback mode",
	.help = NULL,
	.options = NULL,
	.exec = exec_mixer_loopback,
};

/*
 * "backlight-power" command
 */
static int exec_backlight_power(struct cli *cli, int argc, char *argv[])
{
	bool enable = false;
	int err;

	if (argc < 2)
		return -EINVAL;

	err = parse_bool(argv[1], &enable);
	if (err < 0) {
		printf("Invalid boolean value '%s'.\n", argv[1]);
		return -EINVAL;
	}

	err = remote_backlight_enable(cli->client, enable);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_backlight_power _command_ = {
	.name = "backlight-power",
	.summary = "control backlight power",
	.help = NULL,
	.options = NULL,
	.exec = exec_backlight_power,
};

/*
 * "backlight-brightness" command
 */
static int exec_backlight_brightness(struct cli *cli, int argc, char *argv[])
{
	int err;

	if (argc < 2) {
		uint8_t brightness = 0;

		err = remote_backlight_get(cli->client, &brightness);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}

		printf("brightness: %u\n", brightness);
	} else {
		unsigned long value = 0;
		char *end = NULL;

		value = strtoul(argv[1], &end, 0);
		if (end == argv[1]) {
			printf("Invalid numerical value '%s'.\n", argv[1]);
			return -EINVAL;
		}

		if (value > 255) {
			printf("Invalid backlight brightness %lu. Must be "
					"less than or equal to 255.\n",
					value);
			return -EINVAL;
		}

		err = remote_backlight_set(cli->client, value);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	}

	return 0;
}

const struct cli_command_info cmd_backlight_brightness _command_ = {
	.name = "backlight-brightness",
	.summary = "access backlight brightness",
	.help = NULL,
	.options = NULL,
	.exec = exec_backlight_brightness,
};

/*
 * "lldp-dump" command
 */
static int exec_lldp_dump(struct cli *cli, int argc, char *argv[])
{
	void *frame;
	int err;

	frame = malloc(LLDP_MAX_FRAME_SIZE);
	if (!frame)
		return -ENOMEM;

	err = remote_lldp_read(cli->client, frame, LLDP_MAX_FRAME_SIZE);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		free(frame);
		return err;
	}

	printf("LLDP frame:\n");
	print_hex_dump(stdout, "  ", DUMP_PREFIX_OFFSET, 16, 1, frame, err, true);

	free(frame);
	return 0;
}

const struct cli_command_info cmd_lldp_dump _command_ = {
	.name = "lldp-dump",
	.summary = "dump LLDP data",
	.help = NULL,
	.options = NULL,
	.exec = exec_lldp_dump,
};

/*
 * "media-player-uri" command
 */
static int exec_media_player_uri(struct cli *cli, int argc, char *argv[])
{
	int err;

	if (argc < 2) {
		char *uri = NULL;

		err = remote_media_player_get_stream(cli->client, &uri);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}

		printf("URI: %s\n", uri);
	} else {
		err = remote_media_player_set_stream(cli->client, argv[1]);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	}

	return 0;
}

const struct cli_command_info cmd_media_player_uri _command_ = {
	.name = "media-player-uri",
	.summary = "access media stream URI",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_uri,
};

/*
 * "media-player-output" command
 */
static int exec_media_player_output(struct cli *cli, int argc, char *argv[])
{
	uint16_t x = UINT16_MAX;
	uint16_t y = UINT16_MAX;
	uint16_t width = UINT16_MAX;
	uint16_t height = UINT16_MAX;
	int err;

	if (argc > 1) {
		unsigned long value;
		char *end = NULL;

		value = strtoul(argv[1], &end, 0);
		if (end != argv[1])
			x = value;
	}

	if (argc > 2) {
		unsigned long value;
		char *end = NULL;

		value = strtoul(argv[2], &end, 0);
		if (end != argv[2])
			y = value;
	}

	if (argc > 3) {
		unsigned long value;
		char *end = NULL;

		value = strtoul(argv[3], &end, 0);
		if (end != argv[3])
			width = value;
	}

	if (argc > 4) {
		unsigned long value;
		char *end = NULL;

		value = strtoul(argv[4], &end, 0);
		if (end != argv[4])
			height = value;
	}

	if ((x == UINT16_MAX) && (y == UINT16_MAX) &&
	    (width == UINT16_MAX) && (height == UINT16_MAX)) {
		/* nothing to see here, move along */
	} else {
		err = remote_media_player_set_output_window(cli->client,
				x, y, width, height);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	}

	return 0;
}

const struct cli_command_info cmd_media_player_output _command_ = {
	.name = "media-player-output",
	.summary = "control media player output window",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_output,
};

/*
 * "media-player-play" command
 */
static int exec_media_player_play(struct cli *cli, int argc, char *argv[])
{
	int err;

	if (argc > 1) {
		err = remote_media_player_set_stream(cli->client, argv[1]);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	}

	err = remote_media_player_start(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_media_player_play _command_ = {
	.name = "media-player-play",
	.summary = "start media playback",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_play,
};

/*
 * "media-player-stop" command
 */
static int exec_media_player_stop(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_media_player_stop(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_media_player_stop _command_ = {
	.name = "media-player-stop",
	.summary = "stop media playback",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_stop,
};

/*
 * "media-player-pause" command
 */
static int exec_media_player_pause(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_media_player_pause(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_media_player_pause _command_ = {
	.name = "media-player-pause",
	.summary = "pause media stream",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_pause,
};

/*
 * "media-player-resume" command
 */
static int exec_media_player_resume(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_media_player_resume(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_media_player_resume _command_ = {
	.name = "media-player-resume",
	.summary = "resume media stream",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_resume,
};


/*
 * "media-player-duration" command
 */
static int exec_media_player_duration(struct cli *cli, int argc, char *argv[])
{
	unsigned long duration;
	int err;

	err = remote_media_player_get_duration(cli->client, &duration);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	printf("Duration: %lu\n", duration);

	return 0;
}

const struct cli_command_info cmd_media_player_duration _command_ = {
	.name = "media-player-duration",
	.summary = "obtain media stream duration",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_duration,
};


/*
 * "media-player-position" command
 */
static int exec_media_player_position(struct cli *cli, int argc, char *argv[])
{
	unsigned long position;
	int err;

	if (argc > 1) {
		char *end = NULL;

		position = strtoul(argv[1], &end, 10);
		if (end == argv[1]) {
			printf("invalid position: %s\n", argv[1]);
			return -EINVAL;
		}

		err = remote_media_player_set_position(cli->client, position);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}
	} else {
		err = remote_media_player_get_position(cli->client, &position);
		if (err < 0) {
			printf("%s\n", strerror(-err));
			return err;
		}

		printf("Position: %lu\n", position);
	}

	return 0;
}

const struct cli_command_info cmd_media_player_position _command_ = {
	.name = "media-player-position",
	.summary = "access media stream position",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_position,
};

/*
 * "media-player-state" command
 */
static int exec_media_player_state(struct cli *cli, int argc, char *argv[])
{
	const char *state = "unknown";
	bool running = false;
	int32_t err;

	err = remote_media_player_is_running(cli->client, &running);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	if (running)
		state = "playing";
	else
		state = "stopped";

	printf("state: %s\n", state);
	return 0;
}

const struct cli_command_info cmd_media_player_state _command_ = {
	.name = "media-player-state",
	.summary = "retrieve media player state",
	.help = NULL,
	.options = NULL,
	.exec = exec_media_player_state,
};

/*
 * "sound-play" command
 */
static int exec_sound_play(struct cli *cli, int argc, char *argv[])
{
	int err;

	if (argc < 2)
		return -EINVAL;

	err = remote_sound_play(cli->client, argv[1]);
	if (err < 0)
		printf("%s\n", strerror(-err));

	return err;
}

const struct cli_command_info cmd_sound_play _command_ = {
	.name = "sound-play",
	.summary = "play sound file",
	.help = NULL,
	.options = NULL,
	.exec = exec_sound_play,
};

/*
 * "sound-stop" command
 */
static int exec_sound_stop(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_sound_stop(cli->client);
	if (err < 0)
		printf("%s\n", strerror(-err));

	return err;
}

const struct cli_command_info cmd_sound_stop _command_ = {
	.name = "sound-stop",
	.summary = "stop sound playback",
	.help = NULL,
	.options = NULL,
	.exec = exec_sound_stop,
};

/*
 * "modem-call" command
 */
static int exec_modem_call(struct cli *cli, int argc, char *argv[])
{
	int32_t err;

	err = remote_modem_dial(cli->client, argv[1]);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_modem_call _command_ = {
	.name = "modem-call",
	.summary = "place a phone call",
	.help = NULL,
	.options = NULL,
	.exec = exec_modem_call,
};

/*
 * "modem-accept" command
 */
static int exec_modem_accept(struct cli *cli, int argc, char *argv[])
{
	int32_t err;

	err = remote_modem_pick_up(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_modem_accept _command_ = {
	.name = "modem-accept",
	.summary = "accept an incoming phone call",
	.help = NULL,
	.options = NULL,
	.exec = exec_modem_accept,
};

/*
 * "modem-terminate" command
 */
static int exec_modem_terminate(struct cli *cli, int argc, char *argv[])
{
	int32_t err;

	err = remote_modem_hang_up(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_modem_terminate _command_ = {
	.name = "modem-terminate",
	.summary = "terminate a phone call",
	.help = NULL,
	.options = NULL,
	.exec = exec_modem_terminate,
};

/*
 * "modem-reset" command
 */
static int exec_modem_reset(struct cli *cli, int argc, char *argv[])
{
	int32_t err;

	err = remote_modem_deinit(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	err = remote_modem_init(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_modem_reset _command_ = {
	.name = "modem-reset",
	.summary = "reset the modem",
	.help = "No options",
	.options = NULL,
	.exec = exec_modem_reset,
};

/*
 * "smartcard-read" command
 */
static int exec_smartcard_read(struct cli *cli, int argc, char *argv[])
{
	uint8_t buffer[256];
	ssize_t err;

	err = remote_card_read(cli->client, 0, buffer, sizeof(buffer));
	if (err < 0)
		return err;

	print_hex_dump(stdout, "", DUMP_PREFIX_OFFSET, 16, 1, buffer, err,
			true);

	return 0;
}

const struct cli_command_info cmd_smartcard_read _command_ = {
	.name = "smartcard-read",
	.summary = "read from a smart card",
	.help = NULL,
	.options = NULL,
	.exec = exec_smartcard_read,
};

/*
 * "task-exec" command
 */
static int exec_task_exec(struct cli *cli, int argc, char *argv[])
{
	size_t length = 0;
	char *cmdline;
	int err;
	int i;

	if (argc < 2)
		return -1;

	for (i = 1; i < argc; i++) {
		length += strlen(argv[i]) + 1;
		if (shell_str_needs_escape(argv[i]))
			length += 2;
	}

	cmdline = calloc(1, length);
	if (!cmdline)
		return -ENOMEM;

	for (i = 1; i < argc; i++) {
		bool escape = shell_str_needs_escape(argv[i]);

		if (i > 1)
			strcat(cmdline, " ");

		if (escape)
			strcat(cmdline, "'");

		strcat(cmdline, argv[i]);

		if (escape)
			strcat(cmdline, "'");
	}

	err = remote_task_manager_exec(cli->client, cmdline);
	if (err < 0)
		printf("%s\n", strerror(-err));
	else
		printf("PID: %d\n", err);

	free(cmdline);
	return err;
}

const struct cli_command_info cmd_task_exec _command_ = {
	.name = "task-exec",
	.summary = "execute a task",
	.help = NULL,
	.options = NULL,
	.exec = exec_task_exec,
};

/*
 * "task-kill" command
 */
static int exec_task_kill(struct cli *cli, int argc, char *argv[])
{
	int sig = SIGTERM;
	char *end = NULL;
	int pid;
	int err;

	if (argc < 2)
		return -1;

	pid = strtoul(argv[1], &end, 0);
	if (end == argv[1])
		return -1;

	if (argc > 2) {
		unsigned long signum = strtoul(argv[2], &end, 0);
		if (end != argv[2])
			sig = signum;
	}

	err = remote_task_manager_kill(cli->client, pid, sig);
	if (err < 0)
		printf("%s\n", strerror(-err));

	return err;
}

const struct cli_command_info cmd_task_kill _command_ = {
	.name = "task-kill",
	.summary = "kill a running task",
	.help = NULL,
	.options = NULL,
	.exec = exec_task_kill,
};

/*
 * "voip-login" command
 */
static int exec_voip_login(struct cli *cli, int argc, char *argv[])
{
	struct remote_voip_account account;
	int err;

	if (argc > 1)
		account.proxy = argv[1];
	else
		account.proxy = NULL;

	if (argc > 2) {
		unsigned long port;
		char *end = NULL;

		port = strtoul(argv[2], &end, 0);
		if (end == argv[2])
			return -EINVAL;

		account.port = port;
	} else {
		account.port = 0;
	}

	if (argc > 3)
		account.username = argv[3];
	else
		account.username = NULL;

	if (argc > 4)
		account.password = argv[4];
	else
		account.password = NULL;

	err = remote_voip_login(cli->client, &account);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_voip_login _command_ = {
	.name = "voip-login",
	.summary = "register with VoIP proxy",
	.help = "<registrar> <port> <user> <passwd>",
	.options = NULL,
	.exec = exec_voip_login,
};

/*
 * "voip-logout" command
 */
static int exec_voip_logout(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_voip_logout(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_voip_logout _command_ = {
	.name = "voip-logout",
	.summary = "unregister from VoIP proxy",
	.help = NULL,
	.options = NULL,
	.exec = exec_voip_logout,
};

/*
 * "voip-call" command
 */
static int exec_voip_call(struct cli *cli, int argc, char *argv[])
{
	int err;

	if (argc < 2)
		return -EINVAL;

	err = remote_voip_connect_to(cli->client, argv[1]);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_voip_call _command_ = {
	.name = "voip-call",
	.summary = "place outgoing VoIP call",
	.help = NULL,
	.options = NULL,
	.exec = exec_voip_call,
};

/*
 * "voip-accept" command
 */
static int exec_voip_accept(struct cli *cli, int argc, char *argv[])
{
	char *caller = NULL;
	int err;

	err = remote_voip_accept_incoming(cli->client, &caller);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	printf("caller: %s\n", caller);
	free(caller);
	return 0;
}

const struct cli_command_info cmd_voip_accept _command_ = {
	.name = "voip-accept",
	.summary = "accept incoming VoIP call",
	.help = NULL,
	.options = NULL,
	.exec = exec_voip_accept,
};

/*
 * "voip-terminate" command
 */
static int exec_voip_terminate(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_voip_disconnect(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_voip_terminate _command_ = {
	.name = "voip-terminate",
	.summary = "terminate VoIP call",
	.help = NULL,
	.options = NULL,
	.exec = exec_voip_terminate,
};

/*
 * "voip-dtmf" command
 */
static int exec_voip_dtmf(struct cli *cli, int argc, char *argv[])
{
	uint8_t dtmf;
	int err;

	if (argc < 2)
		return -EINVAL;

	dtmf = (uint8_t)argv[1][0];

	err = remote_voip_dial(cli->client, dtmf);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_voip_dtmf _command_ = {
	.name = "voip-dtmf",
	.summary = "dial a dtmf tone",
	.help = NULL,
	.options = NULL,
	.exec = exec_voip_dtmf,
};

/*
 * "tuner-frequency" command
 */
static int exec_tuner_frequency(struct cli *cli, int argc, char *argv[])
{
	unsigned long frequency;
	char *end = NULL;
	int err;

	if (argc < 2)
		return -EINVAL;

	frequency = strtoul(argv[1], &end, 0);
	if (end == argv[1]) {
		printf("%s\n", strerror(errno));
		return -EINVAL;
	}

	err = remote_tuner_set_frequency(cli->client, frequency);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_tuner_frequency _command_ = {
	.name = "tuner-frequency",
	.summary = "set the tuner frequency",
	.help = NULL,
	.options = NULL,
	.exec = exec_tuner_frequency,
};

/*
 * "handset-display-clear" command
 */
static int exec_handset_display_clear(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_handset_display_clear(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_handset_display_clear _command_ = {
	.name = "handset-display-clear",
	.summary = "clear the handset display",
	.help = NULL,
	.options = NULL,
	.exec = exec_handset_display_clear,
};

/*
 * "handset-display-sync" command
 */
static int exec_handset_display_sync(struct cli *cli, int argc, char *argv[])
{
	int err;

	err = remote_handset_display_sync(cli->client);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_handset_display_sync _command_ = {
	.name = "handset-display-sync",
	.summary = "update the handset display",
	.help = NULL,
	.options = NULL,
	.exec = exec_handset_display_sync,
};

/*
 * "handset-backlight" command
 */
static int exec_handset_backlight(struct cli *cli, int argc, char *argv[])
{
	unsigned long brightness = 0;
	char *end = NULL;
	int err;

	if (argc < 3)
		return -EINVAL;

	brightness = strtoul(argv[2], &end, 10);
	if (end == argv[2])
		return -EINVAL;

	if (strcmp(argv[1], "display") == 0) {
		err = remote_handset_display_set_brightness(cli->client,
				brightness);
		if (err < 0)
			return err;
	} else if (strcmp(argv[1], "keypad") == 0) {
		err = remote_handset_keypad_set_brightness(cli->client,
				brightness);
		if (err < 0)
			return err;
	} else
		return -EINVAL;

	return 0;
}

const struct cli_command_info cmd_handset_backlight _command_ = {
	.name = "handset-backlight",
	.summary = "control the display or keypad backlight",
	.help = NULL,
	.options = NULL,
	.exec = exec_handset_backlight,
};

/*
 * "handset-show-icon" command
 */
static int exec_handset_show_icon(struct cli *cli, int argc, char *argv[])
{
	enum remote_handset_icon id;
	int err;

	if (argc < 2)
		return -EINVAL;

	if (strcmp(argv[1], "phone") == 0)
		id = REMOTE_HANDSET_ICON_PHONE;
	else if (strcmp(argv[1], "radio") == 0)
		id = REMOTE_HANDSET_ICON_RADIO;
	else if (strcmp(argv[1], "tv") == 0)
		id = REMOTE_HANDSET_ICON_TV;
	else if (strcmp(argv[1], "logo") == 0)
		id = REMOTE_HANDSET_ICON_LOGO;
	else
		return -EINVAL;

	err = remote_handset_show_icon(cli->client, id);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_handset_show_icon _command_ = {
	.name = "handset-show-icon",
	.summary = "show a given icon on the handset display",
	.help = NULL,
	.options = NULL,
	.exec = exec_handset_show_icon,
};

/*
 * "handset-show-text" command
 */
static int exec_handset_show_text(struct cli *cli, int argc, char *argv[])
{
	const char *text = argv[1];
	unsigned long x = 8;
	unsigned long y = 48;
	int err;

	if (argc < 2)
		return -EINVAL;

	if (argc > 2) {
		char *end = NULL;

		x = strtoul(argv[1], &end, 10);
		if (end == argv[1]) {
			printf("invalid X-coordinate: %s\n", argv[1]);
			return -EINVAL;
		}

		text = argv[2];
	}

	if (argc > 3) {
		char *end = NULL;

		y = strtoul(argv[2], &end, 10);
		if (end == argv[2]) {
			printf("invalid Y-coordinate: %s\n", argv[2]);
			return -EINVAL;
		}

		text = argv[3];
	}

	err = remote_handset_show_text(cli->client, x, y, text);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_handset_show_text _command_ = {
	.name = "handset-show-text",
	.summary = "show a given string on the handset display",
	.help = NULL,
	.options = NULL,
	.exec = exec_handset_show_text,
};

/*
 * "gpio-get" command
 */
static int exec_gpio_get(struct cli *cli, int argc, char *argv[])
{
	unsigned int gpio;
	char *end = NULL;
	int err;

	if (argc < 2) {
		printf("no GPIO number specified\n");
		return -EINVAL;
	}

	gpio = strtoul(argv[1], &end, 0);
	if (end == argv[1]) {
		printf("invalid GPIO: %s\n", argv[1]);
		return -EINVAL;
	}

	err = remote_gpio_get(cli->client, gpio);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	printf("GPIO#%u: %u\n", gpio, !!err);

	return 0;
}

const struct cli_command_info cmd_gpio_get _command_ = {
	.name = "gpio-get",
	.summary = "obtain the pin level of a given GPIO",
	.help = NULL,
	.options = NULL,
	.exec = exec_gpio_get,
};

/*
 * "gpio-set" command
 */
static int exec_gpio_set(struct cli *cli, int argc, char *argv[])
{
	unsigned int value;
	unsigned int gpio;
	char *end = NULL;
	int err;

	if (argc < 3)
		return -EINVAL;

	gpio = strtoul(argv[1], &end, 0);
	if (end == argv[1]) {
		printf("invalid GPIO: %s\n", argv[1]);
		return -EINVAL;
	}

	value = strtoul(argv[2], &end, 0);
	if ((end == argv[2]) || ((value != 0) && (value != 1))) {
		printf("invalid pin level: %s\n", argv[2]);
		return -EINVAL;
	}

	err = remote_gpio_set(cli->client, gpio, value);
	if (err < 0) {
		printf("%s\n", strerror(-err));
		return err;
	}

	return 0;
}

const struct cli_command_info cmd_gpio_set _command_ = {
	.name = "gpio-set",
	.summary = "set the pin level of a given GPIO",
	.help = NULL,
	.options = NULL,
	.exec = exec_gpio_set,
};

/*
 * "irq-poll" command
 */
static int exec_irq_poll(struct cli *cli, int argc, char *argv[])
{
	static enum remote_irq_source sources[] = {
		REMOTE_IRQ_SOURCE_HOOK,
		REMOTE_IRQ_SOURCE_SMARTCARD,
		REMOTE_IRQ_SOURCE_VOIP,
		REMOTE_IRQ_SOURCE_IO,
		REMOTE_IRQ_SOURCE_MODEM,
		REMOTE_IRQ_SOURCE_HANDSET,
	};
	uint32_t mask = 0;
	unsigned int i;
	int err;

	err = remote_irq_get_mask(cli->client, &mask);
	if (err < 0) {
		fprintf(stderr, "remote_irq_get_mask(): %s", strerror(-err));
		return err;
	}

	for (i = 0; i < ARRAY_SIZE(sources); i++) {
		unsigned int source = sources[i];

		if (mask & (1 << (source - 1))) {
			uint32_t status = 0;

			switch (source) {
			case REMOTE_IRQ_SOURCE_UNKNOWN:
			case REMOTE_IRQ_SOURCE_MAX:
				fprintf(stderr, "UNKNOWN\n");
				break;

			case REMOTE_IRQ_SOURCE_HOOK:
				fprintf(stderr, "HOOK\n");
				break;

			case REMOTE_IRQ_SOURCE_SMARTCARD:
				fprintf(stderr, "SMARTCARD\n");
				break;

			case REMOTE_IRQ_SOURCE_VOIP:
				fprintf(stderr, "VOIP\n");
				break;

			case REMOTE_IRQ_SOURCE_IO:
				fprintf(stderr, "IO\n");
				break;

			case REMOTE_IRQ_SOURCE_MODEM:
				fprintf(stderr, "MODEM\n");
				break;

			case REMOTE_IRQ_SOURCE_HANDSET:
				fprintf(stderr, "HANDSET\n");
				break;
			}

			err = remote_irq_get_info(cli->client, source, &status);
			if (err < 0) {
				fprintf(stderr, "remote_irq_get_info(): %s\n",
						strerror(-err));
				continue;
			}

			fprintf(stderr, "  %08x\n", status);
		}
	}

	return 0;
}

const struct cli_command_info cmd_irq_poll _command_ = {
	.name = "irq-poll",
	.summary = "poll interrupts",
	.help = NULL,
	.options = NULL,
	.exec = exec_irq_poll,
};
