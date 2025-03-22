#include <stdio.h>
#include <ssq/a2s.h>
#include <getopt.h>
#include <string.h>

#define DEFAULT_TIMEOUT 1000
#define DEFAULT_RETRY_COUNT 3

#define RETURN_SETTING_ID 1 // Game ID
#define RETURN_SETTING_PLAYERS 2 // Players
#define RETURN_SETTING_MAX_PLAYERS 3 // Max players
#define RETURN_SETTING_BOTS 4 // Bots
#define RETURN_SETTING_PLAYERS_MINUS_BOTS 5 // Players-Bots

static char server_host[64];
static uint16_t server_port;
static int timeout_ms = DEFAULT_TIMEOUT;
static int retry_count = DEFAULT_RETRY_COUNT;
static int return_code_src = 0;
static char output_file_name[256]; // Filename to output server name to. Blank if unused.
static char output_file_map[256]; // Filename to output server map to. Blank if unused.
static char output_file_folder[256]; // Filename to output server folder to. Blank if unused.
static char output_file_game[256]; // Filename to output server game to. Blank if unused.
static char output_file_version[256]; // Filename to output server version to. Blank if unused.
static char output_file_keywords[256]; // Filename to output server keywords to. Blank if unused.

static void help(char* pgm) {
	printf("Usage: %s\n", pgm);
	printf("    [-h Server hostname (required)]\n");
	printf("    [-p Server port (required)]\n");
	printf("    [-t Timeout in milliseconds (default is %i)]\n", DEFAULT_TIMEOUT);
	printf("    [-r Retry count (default is %i)]\n", DEFAULT_RETRY_COUNT);
	printf("    [-c Return code source (always -1 on error) of the following:]\n");
	printf("        0: Nothing (default)\n");
	printf("        1: Game ID\n");
	printf("        2: Players\n");
	printf("        3: Max Players\n");
	printf("        4: Bots\n");
	printf("        5: Players - Bots\n");
	printf("    [--name-file (File to write server name to)]\n");
	printf("    [--map-file (File to write server map to)]\n");
	printf("    [--folder-file (File to write server folder to)]\n");
	printf("    [--game-file (File to write server game to)]\n");
	printf("    [--version-file (File to write server version to)]\n");
	printf("    [--keywords-file (File to write server keywords to)]\n");
}

static int parse_args(int argc, char* argv[]) {
	static const struct option long_opts[] = {
		{ "name-file", required_argument, NULL, 11 },
		{ "map-file", required_argument, NULL, 12 },
		{ "folder-file", required_argument, NULL, 13 },
		{ "game-file", required_argument, NULL, 14 },
		{ "version-file", required_argument, NULL, 15 },
		{ "keywords-file", required_argument, NULL, 16 },
		{ 0 }
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "h:p:t:r:c:", long_opts, NULL)) != -1) {
		switch (opt) {
			case 'h': // HOSTNAME
				strncpy(server_host, optarg, sizeof(server_host) - 1);
				break;
			case 'p': // PORT
				server_port = atoi(optarg);
				break;
			case 't': // TIMEOUT
				timeout_ms = atoi(optarg);
				break;
			case 'r': // RETRIES
				retry_count = atoi(optarg);
				break;
			case 'c': // RETURN CODE
				return_code_src = atoi(optarg);
				break;
			case 11: // NAME FILE
				strncpy(output_file_name, optarg, sizeof(output_file_name) - 1);
				break;
			case 12: // MAP FILE
				strncpy(output_file_map, optarg, sizeof(output_file_map) - 1);
				break;
			case 13: // FOLDER FILE
				strncpy(output_file_folder, optarg, sizeof(output_file_folder) - 1);
				break;
			case 14: // GAME FILE
				strncpy(output_file_game, optarg, sizeof(output_file_game) - 1);
				break;
			case 15: // VERSION FILE
				strncpy(output_file_version, optarg, sizeof(output_file_version) - 1);
				break;
			case 16: // KEYWORDS FILE
				strncpy(output_file_keywords, optarg, sizeof(output_file_keywords) - 1);
				break;
			default:
				help(argv[0]);
				return 1;
		}
	}

	return 0;
}

static int validate_args() {
	//Check that the hostname is set
	if (server_host[0] == 0) {
		printf("Server hostname is not set. Specify it with -h.\n");
		return 1;
	}

	//Check that the port is specified
	if (server_port == 0) {
		printf("Server port is not set. Specify it with -p.\n");
		return 1;
	}

	return 0;
}

static void dump_to_output(const char* displayName, const char* filename, const char* value, size_t valueLen) {
	//If the filename is empty, skip
	if (filename[0] == 0)
		return;

	//Open the output file
	FILE* file = fopen(filename, "w");
	if (file == NULL) {
		printf("ERROR: Failed to open %s file (at %s).\n", displayName, filename);
		return;
	}

	//Write text
	if (fwrite(value, 1, valueLen, file) != valueLen) {
		printf("ERROR: Failed to write %s file entirely. Out of disk space?\n", displayName);
	}

	//Close the file
	fclose(file);
}

int main(int argc, char* argv[])
{
	//Clear out all settings
	memset(server_host, 0, sizeof(server_host));
	memset(output_file_name, 0, sizeof(output_file_name));
	memset(output_file_map, 0, sizeof(output_file_map));
	memset(output_file_folder, 0, sizeof(output_file_folder));
	memset(output_file_game, 0, sizeof(output_file_game));
	memset(output_file_version, 0, sizeof(output_file_version));
	memset(output_file_keywords, 0, sizeof(output_file_keywords));

	//If no arguments (other than the program name) were specified, show help
	if (argc <= 1) {
		help(argv[0]);
		return -1;
	}

	//Parse args
	if (parse_args(argc, argv))
		return -1;

	//Validate args
	if (validate_args())
		return -1;

	//Fetch server info with retries
	A2S_INFO* info;
	int attemptNum = 0;
	int returnResult = -1;
	SSQ_SERVER* server;
	while (1) {
		//Initialize client
		server = ssq_server_new(server_host, server_port);
		if (server == NULL) {
			printf("Memory exhausted.\n");
			return -1;
		}
		if (!ssq_server_eok(server)) {
			printf("Failed to initialize server connection: %s: %s\n", server_host, ssq_server_emsg(server));
			ssq_server_free(server);
			return -1;
		}

		//Set timeout
		ssq_server_timeout(server, SSQ_TIMEOUT_RECV | SSQ_TIMEOUT_SEND, timeout_ms);

		//Fetch info
		info = ssq_info(server);

		//Check if OK
		if (ssq_server_eok(server))
			break;

		//Failed, log error and then reset it
		printf("Failed to connect (attempt %i of %i): %s\n", attemptNum + 1, retry_count + 1, ssq_server_emsg(server));
		ssq_server_eclr(server);

		//Increment attempt count and check it
		if (attemptNum++ >= retry_count) {
			printf("Retries exhausted. Aborting...\n");
			return -1;
		}

		//Cleanup
		ssq_server_free(server);
	}

	//Dump outputs
	dump_to_output("server name", output_file_name, info->name, info->name_len);
	dump_to_output("server map", output_file_map, info->map, info->map_len);
	dump_to_output("game folder", output_file_folder, info->folder, info->folder_len);
	dump_to_output("game name", output_file_game, info->game, info->game_len);
	dump_to_output("server version", output_file_version, info->version, info->version_len);
	dump_to_output("game keywords", output_file_keywords, info->keywords, info->keywords_len);

	//Set the return code
	switch (return_code_src) {
	case RETURN_SETTING_ID:					returnResult = info->id; break;
	case RETURN_SETTING_PLAYERS:			returnResult = info->players; break;
	case RETURN_SETTING_MAX_PLAYERS:		returnResult = info->max_players; break;
	case RETURN_SETTING_BOTS:				returnResult = info->bots; break;
	case RETURN_SETTING_PLAYERS_MINUS_BOTS: returnResult = info->players - (int)info->bots; break;
	default: returnResult = 0;
	}

	//Constrain the result code to minimum of 0
	if (returnResult < 0)
		returnResult = 0;

	//Log result code
	printf("(returned %i)\n", returnResult);

	//Finally, release it
	ssq_info_free(info);

	//Cleanup
	ssq_server_free(server);

	return returnResult;
}