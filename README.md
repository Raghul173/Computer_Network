# Network Protocols & Algorithms Laboratory

This repository contains implementations of fundamental **Computer Networking protocols, error detection and correction techniques, routing algorithms, and socket programming concepts**. It serves as a practical guide and reference for understanding and implementing key concepts in computer networks.

## Contents

### 1. Implementation of Framing Protocol

This experiment demonstrates **framing techniques** used in the Data Link Layer to divide a continuous stream of data into manageable frames.

- **Byte Stuffing** – A character-oriented framing technique used to provide data transparency by adding special characters when required.

### 2. Implementation of Error Detection Techniques

This experiment focuses on techniques used to detect errors that may occur during data transmission.

- **Checksum** – An error detection method used to verify the integrity of transmitted data.
- **2D Parity Bit** – A two-dimensional parity checking technique that uses row and column parity bits to detect transmission errors.

### 3. Implementation of Error Correction Technique

This experiment demonstrates techniques used to **detect and correct errors** in transmitted data.

- **2D Parity Check** – Uses row and column parity information to identify and correct certain single-bit errors.

### 4. Implementation of Routing Algorithms

This experiment focuses on algorithms used to determine efficient paths between nodes in a network.

- **Bellman-Ford Algorithm** – A distance-vector routing algorithm used to find the shortest paths from a source node to all other nodes in a weighted graph.

### 5. Implementation of Client-Server Application Using TCP Socket

This experiment demonstrates **client-server communication using TCP sockets**.

- Establishes a reliable connection between the client and server.
- Uses **TCP (Transmission Control Protocol)** for connection-oriented communication.
- Demonstrates sending and receiving data between the client and server.

### 6. Implementation of Iterative Server Using Socket Programming – UDP Socket

This experiment demonstrates an **iterative server using UDP socket programming**.

- Uses **UDP (User Datagram Protocol)** for connectionless communication.
- The server processes one client request at a time.
- Demonstrates communication using UDP sockets.

### 7. Implementation of Concurrent Server Using Socket Programming – TCP Socket

This experiment demonstrates a **concurrent server using TCP socket programming**.

- Uses **TCP** for reliable, connection-oriented communication.
- Allows the server to handle multiple clients concurrently.
- Demonstrates concurrent client-server communication using socket programming.

## Technologies Used

- **C / C++**
- **Socket Programming**
- **TCP (Transmission Control Protocol)**
- **UDP (User Datagram Protocol)**
- **Computer Networking Algorithms**

## Repository Structure

```text
Network-Protocols-Algorithms-Lab/
│
├── EXP1/
│   └── Framing Protocol
│
├── EXP2/
│   └── Error Detection Techniques
│
├── EXP3/
│   └── Error Correction Technique
│
├── EXP4/
│   └── Routing Algorithms
│
├── EXP5/
│   └── TCP Client-Server Application
│
├── EXP6/
│   └── Iterative Server - UDP
│
└── EXP7/
    └── Concurrent Server - TCP
