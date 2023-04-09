#include <ws2tcpip.h>
#include <winsock2.h>
#include <stdio.h>

#define BUFLEN 512

// General
//------------------------------------------------------------------------
#define PSI 14.504 // BAR->PSI
#define MPH 2.237  // M/S->MPH
#define KPH 3.6    // M/S->KMH

#define bool char
#define true 1
#define false 0

// Outgauge Flags
//------------------------------------------------------------------------
#define OG_SHIFT       1           // key // N/A
#define OG_CTRL        2           // key // N/A
#define OG_TURBO       8192        // show turbo gauge
#define OG_KM          16384       // if not set - user prefers MILES
#define OG_BAR         32768       // if not set - user prefers PSI

// Light Flags
//------------------------------------------------------------------------
#define DL_SHIFT       1 << 0      // shift light
#define DL_FULLBEAM    1 << 1      // full beam
#define DL_HANDBRAKE   1 << 2      // handbrake
#define DL_PITSPEED    1 << 3      // pit speed limiter // N/A
#define DL_TC          1 << 4      // TC active or switched off
#define DL_SIGNAL_L    1 << 5      // left turn signal
#define DL_SIGNAL_R    1 << 6      // right turn signal
#define DL_SIGNAL_ANY  1 << 7      // shared turn signal // N/A
#define DL_OILWARN     1 << 8      // oil pressure warning
#define DL_BATTERY     1 << 9      // battery warning
#define DL_ABS         1 << 10     // ABS active or switched off
#define DL_SPARE       1 << 11     // N/A

// Outgauge
//------------------------------------------------------------------------
typedef struct Outgauge_t  {
	unsigned       time;            // time in milliseconds (to check order)
	char           car[4];          // Car name
	unsigned short flags;           // Info (see OG_x below)
	char           gear;            // Reverse:0, Neutral:1, First:2...
	char           plid;            // Unique ID of viewed player (0 = none)
	float          speed;           // M/S
	float          rpm;             // RPM
	float          turbo;           // BAR
	float          engTemp;         // C
	float          fuel;            // 0 to 1
	float          oilPressure;     // BAR
	float          oilTemp;         // C
	unsigned       dashLights;      // Dash lights available (see DL_x below)
	unsigned       showLights;      // Dash lights currently switched on
	float          throttle;        // 0 to 1
	float          brake;           // 0 to 1
	float          clutch;          // 0 to 1
	char           display1[16];    // Usually Fuel
	char           display2[16];    // Usually Settings
	int            id;              // optional - only if OutGauge ID is specified
} Outgauge_t;

bool is_metric = false;

void Outgauge_Display(Outgauge_t* outgauge)
{
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(console_handle, (COORD){0, 0});

	// General
	//------------------------------------------------------------------------
	printf("Gear: %d\n", outgauge->gear);
	printf("RPM: %.0f\n", outgauge->rpm);

	if (!is_metric)
	{
		printf("Speed: %.1f mph\n", outgauge->speed * MPH);
		printf("Turbo: %.2f PSI\n", outgauge->turbo * PSI);
		printf("Oil Pressure: %.2f PSI\n", outgauge->oilPressure * PSI);
	}
	else
	{
		printf("Speed: %.1f kph\n", outgauge->speed * KPH);
		printf("Turbo: %.2f BAR\n", outgauge->turbo);
		printf("Oil Pressure: %.2f BAR\n", outgauge->oilPressure);
	}

	printf("Fuel: %f/100\n", outgauge->fuel * 100);
	printf("\n");

	// Temperatures
	//------------------------------------------------------------------------
	if (!is_metric)
	{
		printf("Engine Temperature: %.1f C\n", outgauge->engTemp);
		printf("Oil Temperature: %.1f C\n", outgauge->oilTemp);
	}
	else
	{
		printf("Engine Temperature: %.1f F\n", (outgauge->engTemp * 9/5) + 32);
		printf("Oil Temperature: %.1f C\n", (outgauge->oilTemp * 9/5) + 32);
	}
	printf("\n");

	// Inputs
	//------------------------------------------------------------------------
	printf("Throttle: %.0f/100\n", outgauge->throttle * 100);
	printf("Brake: %.0f/100\n", outgauge->brake * 100);
	printf("Clutch: %.0f/100\n", outgauge->clutch * 100);
	printf("\n");

	// Other
	//------------------------------------------------------------------------
	printf("Shift light: %d\n", outgauge->showLights & DL_SHIFT);
	printf("Fullbeam: %d\n", outgauge->showLights & DL_FULLBEAM);
	printf("Handbrake: %d\n", outgauge->showLights & DL_HANDBRAKE);
	printf("Pitspeed: %d\n", outgauge->showLights & DL_PITSPEED);
	printf("TC: %d\n", outgauge->showLights & DL_TC);

	printf("\n");
	printf("SIGNAL_L: %d\n", outgauge->showLights & DL_SIGNAL_L);
	printf("SIGNAL_R: %d\n", outgauge->showLights & DL_SIGNAL_R);
	printf("SIGNAL_ANY: %d\n", outgauge->showLights & DL_SIGNAL_ANY);
}

int main(int argc, char* argv[])
{
	int port = 4444;
	char* ip = "127.0.0.1";
	char* unit_type;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-u") == 0 && i+1 < argc) {
			unit_type = argv[i+1];
			if (strcmp(unit_type, "imperial") == 0)
			{
				is_metric = false;
			} else if (strcmp(unit_type, "metric") == 0)
			{
				is_metric = true;
			} else
			{
				printf("Invalid unit\n");
				return 1;
			}
		} else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
			port = atoi(argv[i+1]);
			if (port <= 0 || port > 65535) {
				printf("Invalid port number: %s\n", argv[i+1]);
				return 1;
			}
		} else if (strcmp(argv[i], "-i") == 0 && i+1 < argc) {
			struct sockaddr_in sa;
			if (inet_pton(AF_INET, argv[i+1], &(sa.sin_addr)) != 1) {
				printf("Invalid IP address: %s\n", argv[i+1]);
				return 1;
			}
			ip = argv[i+1];
		} else if (strcmp(argv[i], "-h") == 0) {
			printf("Usage: %s [-u unit_type] [-p port] [-i ip_address] [-h]\n", argv[0]);
			printf("Options:\n");
			printf("  -u unit_type  Set unit type (metric or imperial, default: imperial)\n");
			printf("  -p port       Set port number (default: 4444)\n");
			printf("  -i ip_address Set IP address (default: 127.0.0.1)\n");
			printf("  -h            Display this help message\n");
			return 0;
		}
	}

	system("cls");

	// Init wsa
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		fprintf(stderr, "WSAStartup Failed!");
		return 1;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		WSACleanup();
		fprintf(stderr, "Error creating socket\n");
		return 1;
	}

	// Bind socket to local port
	struct sockaddr_in local_addr;
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons((u_short)port);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
	{
		fprintf(stderr, "Error binding socket\n");
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	// Loop to read data
	char buf[BUFLEN];
	struct sockaddr_in remote_addr;
	int remote_addr_len = sizeof(remote_addr);

	while (1)
	{
		int recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr*)&remote_addr, &remote_addr_len);

		if (recv_len < 0)
		{
			fprintf(stderr, "Error receiving data\n");
			break;
		}

		Outgauge_Display((Outgauge_t*)buf);
	}

	// Cleanup
	closesocket(sock);
	WSACleanup();

	return 0;
}