# Cache Simulator

This repository contains a C program that simulates the behavior of a cache memory in a computer system. It demonstrates how cache memory handles read and write operations, and it supports various cache replacement policies like FIFO (First-In, First-Out) and LRU (Least Recently Used).

## Features

- Simulate basic cache operations (read/write)
- Support for different cache replacement policies (FIFO and LRU)
- Calculation of cache statistics (hits, misses, memory reads, memory writes)
- Prefetching capability
- Dynamic creation and destruction of cache structure

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

You need to have a C compiler like GCC installed on your system to compile and run the program.

### Installation

1. Clone the repository
   ```bash
   git clone https://github.com/your-username/cache-simulator.git
2. Navigate to the cloned directory
   ```bash
   cd cache-simulator
4. Compilation: To compile the program, run the following command in the terminal:
   ```bash
   gcc -o cache_simulator main.c
