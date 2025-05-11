
#include<bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

struct Packet {
    string symbol;
    char buysellindicator;
    int quantity;
    int price;
    int packetSequence;
};

const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 3000;
const int PACKET_SIZE = 17;

int connectToServer() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(1);
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(1);
    }

    return sock;
}

void sendStreamAllRequest(int sock) {
    uint8_t request[2] = {1, 0};
    send(sock, request, 2, 0);
}

void sendResendRequest(int seq, int sock) {
    uint8_t request[2] = {2, static_cast<uint8_t>(seq)};
    send(sock, request, 2, 0);
}

Packet parsePacket(uint8_t* buffer) {
    Packet pkt;
    pkt.symbol = string(reinterpret_cast<char*>(buffer), 4);
    pkt.buysellindicator = buffer[4];
    pkt.quantity = ntohl(*reinterpret_cast<int32_t*>(buffer + 5));
    pkt.price = ntohl(*reinterpret_cast<int32_t*>(buffer + 9));
    pkt.packetSequence = ntohl(*reinterpret_cast<int32_t*>(buffer + 13));
    return pkt;
}

vector<Packet> receivePackets(int sock) {
    vector<Packet> packets;
    uint8_t buffer[PACKET_SIZE];
    ssize_t bytesRead;
    
    while ((bytesRead = recv(sock, buffer, PACKET_SIZE, MSG_WAITALL)) == PACKET_SIZE) {
        packets.push_back(parsePacket(buffer));
    }

    return packets;
}

void writePacketsToJson(const vector<Packet>& packets, const string& filename) {
    json j;
    for (const auto& pkt : packets) {
        j.push_back({
            {"symbol", pkt.symbol},
            {"buysellindicator", string(1, pkt.buysellindicator)},
            {"quantity", pkt.quantity},
            {"price", pkt.price},
            {"packetSequence", pkt.packetSequence}
        });
    }
    ofstream out(filename);
    out << j.dump(4);
}

int main() {
    int sock = connectToServer();
    sendStreamAllRequest(sock);
    vector<Packet> packets = receivePackets(sock);
    close(sock);

    map<int, Packet> packetMap;
    int maxSeq = 0;
    for (const auto& pkt : packets) {
        packetMap[pkt.packetSequence] = pkt;
        if (pkt.packetSequence > maxSeq) maxSeq = pkt.packetSequence;
    }

    for (int seq = 1; seq < maxSeq; ++seq) {
        if (packetMap.find(seq) == packetMap.end()) {
            int resendSock = connectToServer();
            sendResendRequest(seq, resendSock);
            vector<Packet> missed = receivePackets(resendSock);
            if (!missed.empty()) {
                packetMap[seq] = missed[0];
            }
            close(resendSock);
        }
    }

    vector<Packet> sortedPackets;
    for (int i = 1; i <= maxSeq; ++i) {
        sortedPackets.push_back(packetMap[i]);
    }

    writePacketsToJson(sortedPackets, "packets.json");
    cout << "Output written to packets.json" << endl;

    return 0;
}
