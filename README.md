# Fuse-Filesystem

A custom **FUSE-based filesystem** implemented in **C++** using standard **C libraries**. This project demonstrates low-level file operations by reading from **WAD archive files**, utilizing custom-built data structures, and mounting the filesystem on Linux.

---

## 🚀 Features

- 📦 **WAD File Parsing** – Reads and interprets WAD archive files (used in games like Doom).
- 🧠 **Custom Data Structures** – Stores file metadata and directory structures in memory.
- 🔧 **FUSE Integration** – Mounts a virtual filesystem using FUSE on Linux.
- 📂 **Basic File Operations** – Supports reading files, listing directories, and file metadata queries.

---

## 🛠 Technologies Used

- **C++**, **C**
- **FUSE (Filesystem in Userspace)**
- **Linux System Calls**

---

## 📦 Installation

### Prerequisites

- A Linux-based system
- `libfuse` installed:

```bash
sudo apt update
sudo apt install libfuse-dev

