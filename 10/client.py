import socket
import struct
import time
import os

ICMP_TIMESTAMP = 13
ICMP_TIMESTAMP_REPLY = 14

def checksum(data):
    if len(data) % 2:
        data += b'\x00'
    s = sum(struct.unpack("!%dH" % (len(data) // 2), data))
    s = (s >> 16) + (s & 0xffff)
    s += (s >> 16)
    return ~s & 0xffff

def get_time_ms():
    t = time.time()
    return int((t % 86400) * 1000)

def run_client(dest_ip):
    sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP)

    ident = os.getpid() & 0xFFFF
    seq = 1

    originate = get_time_ms()

    header = struct.pack("!BBHHH",
                         ICMP_TIMESTAMP,
                         0,
                         0,
                         ident,
                         seq)

    body = struct.pack("!III",
                       originate,
                       0,
                       0)

    packet = header + body
    chksum = checksum(packet)

    packet = struct.pack("!BBHHH",
                         ICMP_TIMESTAMP,
                         0,
                         chksum,
                         ident,
                         seq) + body

    sock.sendto(packet, (dest_ip, 0))

    data, _ = sock.recvfrom(1024)

    ip_header_len = (data[0] & 0x0F) * 4
    icmp_packet = data[ip_header_len:]

    icmp_type, code, chksum, ident, seq = struct.unpack("!BBHHH", icmp_packet[:8])

    if icmp_type == ICMP_TIMESTAMP_REPLY:
        originate, receive, transmit = struct.unpack("!III", icmp_packet[8:20])

        print("Originate:", originate, "ms")
        print("Receive  :", receive, "ms")
        print("Transmit :", transmit, "ms")

        now = get_time_ms()
        rtt = now - originate
        print("RTT ≈", rtt, "ms")

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python client.py <ip>")
    else:
        run_client(sys.argv[1])