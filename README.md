# Caesar Cipher decoding using matrix

## Project Overview

The project involves implementing a POSIX-compliant C program that efficiently searches for encoded strings in a given text file using shared memory and message queues.

## Features

- **Shared Memory Integration**: Reads an NxN matrix of strings from shared memory.
- **Caesar Cipher Decryption**: Decodes strings using keys received via a message queue.
- **Multi-Process Communication**: Interacts with a helper process to obtain decryption keys.
- **Efficient Word Search**: Finds occurrences of words in a large text file while avoiding substring matches.
- **POSIX Compliance**: Implements inter-process communication (IPC) using shared memory and message queues.

## How It Works

1. **Read Input Files**: The program takes a single command-line argument to identify the relevant input files:
   - `input[t].txt` (matrix size, string length, shared memory key, message queue key)
   - `words[t].txt` (large text file with words to search)
2. **Access Shared Memory**: Connects to a shared memory segment containing the NxN matrix.
3. **Process Matrix Strings**:
   - The first word is unencoded and counted directly.
   - For other diagonals, the occurrence count of the previous diagonal is sent to the helper process to receive the decryption key.
   - Strings are decrypted using the Caesar Cipher and then searched in `words[t].txt`.
4. **Communicate with Helper Process**:
   - Sends occurrence counts to the helper process via a message queue.
   - Receives decryption keys for subsequent diagonals.
   - Terminates upon receiving the final confirmation from the helper.

## Installation & Execution

### Prerequisites

- Ubuntu 22.04 or later
- GCC compiler with POSIX thread support

### Compilation

```sh
gcc solution.c -o solution

gcc helper_program.c -o helper
```

### Running the Program

```sh
python3 test_case_generator.py
```

This generates `testcase[t].bin`, `answer[t].bin` & `words[t].txt` for 20 testcases in the same folder. Now run:

```sh
./helper <test_case_number>
```

Example:

```sh
./helper 3
```

This will process `input3.txt` and `words3.txt`.
