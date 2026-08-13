# 🚀 BDH Linux IDE
### A 100% Custom, Terminal-Based Code Editor Environment for Arch & Manjaro Linux
![AUR version](https://img.shields.io/aur/version/bdh-linux-ide)
![GitHub stars](https://img.shields.io/github/stars/BackendDeveloperHub/bdh-linux-ide)
![GitHub last commit](https://img.shields.io/github/last-commit/BackendDeveloperHub/bdh-linux-ide)
![License](https://img.shields.io/github/license/BackendDeveloperHub/bdh-linux-ide)
BDH Linux IDE is a powerful, minimal, terminal-exclusive code workspace. Moving entirely away from third-party wrappers like Tmux, Ranger, or Nano, this ecosystem is **built entirely from scratch in pure C**. It features its own custom file manager (`bdh-tree`) and a native text editor (`bdh-edit`), seamlessly integrated into a master split-pane environment. 
Built for developers who want to master programming from the ground up — writing every line by hand in a raw, high-performance Linux environment.
---
## 🤔 Why BDH IDE? (The Core Philosophy)
Most modern editors ship with auto-complete, GUI bloat, and AI assistance baked in. BDH IDE takes a different, hardcore approach:
- ⚡ **100% Custom Native C Architecture** — No Tmux, no Ranger, no Nano. We built the tree navigator and the editor from the ground up for maximum speed and zero dependencies.
- 💪 **Muscle Memory** — No auto-complete. You type every bracket and semicolon yourself, embedding syntax deeply into your muscle memory.
- 🧠 **Real Problem Solving** — Debug your own errors without hand-holding, building true problem-solving skills.
- 🔥 **Zero Bloat** — Consumes almost zero system resources, running perfectly on anything from a heavy desktop to a headless server or Android Termux.
---
## ⚙️ The BDH Custom Engine (Core Features)
Instead of stitching together existing tools, BDH IDE utilizes its own modular C binaries:
- **`bdh-ide` (Master UI Controller)** — The master executable that renders the split-screen layout, clipping text accurately to prevent overlap.
- **`bdh-tree` (File Explorer)** — Our custom-built, lightweight directory navigator for the left pane.
- **`bdh-edit` (Modal Editor)** — A fast, VT100-based native text editor built strictly for typing raw code.
- **`bdh-db`** — Custom database console integration built directly into the UI.
---
## 🧰 The Extended Tool Suite
BDH IDE isn't just an editor; it comes packed with a powerful suite of terminal utilities:

| Tool | Description |
| :--- | :--- |
| **`bdh-browser`** | A seamless, lightweight minimal GUI browser session (utilizing `cage` and `firefox`) without leaving your workflow. |
| **`bdh-record`** | Built-in terminal session recorder. Captures your terminal via `asciinema`, converts it to GIF using `agg`, and serves it instantly via a local Python HTTP server & QR Code (`qrencode`). |
| **`install.sh`** | The BDH Smart Installer. Automatically detects your environment (Arch/Termux), installs base tools, compiles the C engine, and sets up the ecosystem. |

---
## 📋 Prerequisites
- **Arch Linux** or **Manjaro Linux** (or any Arch-based distro) / **Termux (Android)**
- `gcc`, `make`, `git` (Core build tools)
- Base packages (auto-installed): `zsh`, `fzf`, `bat`, `eza`, `cage`, `postgresql`, `qrencode`
---
## 📦 Installation
### Method 1: Install via AUR (Recommended for Arch)
```bash
yay -S bdh-linux-ide
# or
paru -S bdh-linux-ide
```
Method 2: Manual Installation (Git Clone & Smart Installer)
```bash
git clone [https://github.com/BackendDeveloperHub/bdh-linux-ide.git](https://github.com/BackendDeveloperHub/bdh-linux-ide.git)
cd bdh-linux-ide
chmod +x install.sh
./install.sh
```
Usage
To launch the split-screen editor environment, simply type:
```bash
bdh-ide
```
Keyboard Controls (IDE)
TAB → Switch focus between BDH-TREE (Left) and BDH-EDIT (Right)
Up / Down Arrows → Navigate files in the tree
Enter → Open a directory or load a file into the editor
Ctrl + S → Save the active file
Q → Quit the IDE (From the Tree pane)


Repository Structure
<pre>
bdh-linux-ide/
├── src/               # Pure C Source Code (main.c, bdh-tree.c, bdh-edit.c, bdh-db.c)
├── include/           # C Header files
├── bdh-ide            # Smart installer script for the ecosystem
├── bdh-browser        # Kiosk Wayland browser launcher script
├── bdh-record         # Screen recording & local QR server script
├── packages.txt       # Dependency list
├── install.sh         # Master compile & install script
├── uninstall.sh       # Automated cleanup script
├── PKGBUILD           # Arch Linux AUR package build configuration
├── LICENSE            # GPL-3.0 License
└── README.md          # Project documentation 
</pre>

🤝 Contributing
Contributions, issues, and feature requests are always welcome!
This project thrives on the open-source community. Feel free to check the issues page if you want to contribute.
📜 License
Distributed under the GNU General Public License v3.0. See the LICENSE file for more information.
👨‍💻 Author & Architecture
Prabakaran P (Backend Developer Hub)
Systems Architecture & Low-Level C Implementation
"Master the terminal, master the code." 🚀





