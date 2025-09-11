# bank.project
Banking Transaction Simulator (C++) – Developed a program to process deposits, withdrawals, and queries for multiple users using efficient data structures (maps, priority queues).

# Banking Transaction Simulator – 281Bank

A **C++ banking system simulator** created for EECS 281 (Data Structures & Algorithms). The program processes deposits, withdrawals, transfers, and queries for multiple users using efficient **priority queues**, **maps**, and other STL containers.

## ✨ Features

* Simulates a multi-user banking system with transaction timestamps and ordering.
* Supports operations like deposits, withdrawals, balance checks, and median transaction queries.
* Implements efficient data structures to ensure fast lookups and updates.
* Includes multiple output modes (verbose, watcher, general evaluation) for testing and debugging.

## 🛠 Tech Stack

* **Language:** C++17
* **Data Structures:** Maps, Priority Queues, Vectors
* **Tools:** G++, Make (optional), Valgrind for memory analysis.

## 🚀 Getting Started

### Prerequisites

* A C++17-compatible compiler (`g++`, `clang++`, or MSVC).
* Command line access to build and run.

### Build and Run

```bash
# Clone the repo
git clone https://github.com/p-rodr/bank.project.git
cd bank.project

# Compile
g++ -std=c++17 -O2 -o bank main.cpp

# Run with sample commands
./bank -vf spec-reg.txt < spec-commands.txt 
```

## 🧠 What I Learned

* Designing and implementing a robust simulation system using STL containers.
* Managing transaction order and median calculations with priority queues.
* Debugging and optimizing for performance with large input sizes.
* Practicing good coding practices like modularity and clean output formatting.
