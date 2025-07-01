Fuse-Filesystem
A custom FUSE-based filesystem implemented in C++ using standard C libraries. This project demonstrates low-level file operations by reading from WAD archive files, utilizing custom-built data structures, and mounting the filesystem on Linux.

Features
📦 WAD File Parsing: Reads and interprets WAD archive files (commonly used in games like Doom).

🧠 Custom Data Structures: Efficient in-memory representations of file metadata and directory trees.

🔧 FUSE Integration: Mount the archive contents as a virtual filesystem on Linux.

📂 File and Directory Operations: Supports file reading, directory listing, and stat operations.

Technologies Used
C++, C

FUSE (Filesystem in Userspace)

Linux system calls

Installation
Prerequisites
Linux OS

libfuse installed (sudo apt install libfuse-dev)

A valid .wad file

Build
bash
Copy
Edit
make
Run
bash
Copy
Edit
./fusefs <mount_directory> <wad_file>
Example:

bash
Copy
Edit
mkdir /tmp/mount
./fusefs /tmp/mount doom1.wad
Project Structure
graphql
Copy
Edit
├── src/
│   ├── main.cpp         # Entry point
│   ├── fuse_ops.cpp     # FUSE operation handlers
│   ├── wad_parser.cpp   # Parses WAD file into memory
│   └── datastructures/  # Custom file/directory structures
├── include/
├── Makefile
└── README.md
Future Improvements
Write support (currently read-only)

Support for other archive formats

Performance profiling and caching strategies

References
FUSE documentation

WAD File Format
