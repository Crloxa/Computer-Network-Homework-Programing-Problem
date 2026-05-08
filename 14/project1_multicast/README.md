# Project 1 - Multicast File Distribution (Simulation)

## Build (MinGW g++)

```powershell
g++ multicast_sender.c -o multicast_sender.exe -lws2_32
g++ multicast_receiver.c -o multicast_receiver.exe -lws2_32
```

## Run

Terminal A (receiver 1):

```powershell
.\multicast_receiver.exe recv1.bin 9000
```

Terminal B (receiver 2, optional):

```powershell
.\multicast_receiver.exe recv2.bin 9000
```

Terminal C (sender):

```powershell
.\multicast_sender.exe test_50mb.bin 9000
```

## Notes

- This is a teaching simulation using UDP multicast + simple packet header.
- No retransmission is implemented.
- Compare output file sizes to check basic integrity.
