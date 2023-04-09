`bog` (not the toilet) is a simple program written in C that acts as a client for the BeamNG.drive simulation game's OutGauge feature. It allows users to retrieve telemetry data from the game and display it in real-time on a console.

## Usage
The program accepts the following command-line arguments:
```
Usage: .\bog.exe [-u unit_type] [-p port] [-i ip_address] [-h]
Options:
  -u unit_type  Set unit type (metric or imperial, default: imperial)
  -p port       Set port number (default: 4444)
  -i ip_address Set IP address (default: 127.0.0.1)
  -h            Display this help message
```

## Examples
`.\bog.exe -u metric -p 4312`