import socket
import struct
import time

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

def run_server():
    sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP)

    while True:
        data, addr = sock.recvfrom(1024)

        ip_header_len = (data[0] & 0x0F) * 4
        icmp_packet = data[ip_header_len:]

        icmp_type, code, chksum, ident, seq = struct.unpack("!BBHHH", icmp_packet[:8])

        if icmp_type == ICMP_TIMESTAMP:
            originate, receive, transmit = struct.unpack("!III", icmp_packet[8:20])

            receive_time = get_time_ms()
            transmit_time = get_time_ms()

            header = struct.pack("!BBHHH",
                                 ICMP_TIMESTAMP_REPLY,
                                 0,
                                 0,
                                 ident,
                                 seq)

            body = struct.pack("!III",
                               originate,
                               receive_time,
                               transmit_time)

            packet = header + body
            chksum = checksum(packet)

            packet = struct.pack("!BBHHH",
                                 ICMP_TIMESTAMP_REPLY,
                                 0,
                                 chksum,
                                 ident,
                                 seq) + body

            sock.sendto(packet, addr)

if __name__ == "__main__":
    run_server()