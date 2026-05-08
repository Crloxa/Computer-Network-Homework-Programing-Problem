# Project 2 - TCP vs UDP File Transfer (Simulation)

## Build (MinGW g++)

```powershell
g++ tcp_server.c -o tcp_server.exe -lws2_32
g++ tcp_client.c -o tcp_client.exe -lws2_32
g++ udp_server.c -o udp_server.exe -lws2_32
g++ udp_client.c -o udp_client.exe -lws2_32
```

## TCP Run

Terminal A (server):

```powershell
.\tcp_server.exe tcp_recv.bin 9100
```

Terminal B (client):

```powershell
.\tcp_client.exe 127.0.0.1 test_50mb.bin 9100
```

## UDP Run

Terminal A (server):

```powershell
.\udp_server.exe udp_recv.bin 9100
```

Terminal B (client):

```powershell
.\udp_client.exe 127.0.0.1 test_50mb.bin 9100
```

## Notes

- UDP mode is kept simple for simulation (no retransmission).
- Use the same test file and same environment for fair comparison.
- Record file size, total time, and MB/s for each mode.
