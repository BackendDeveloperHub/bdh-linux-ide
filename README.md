# 🚀 BDH Linux IDE
### A Lightweight, Terminal-Based Code Editor Environment for Arch & Manjaro Linux

BDH Linux IDE is a powerful, minimal, terminal-exclusive code editor environment built using **Tmux**, **Ranger**, and **Nano**. It is built for developers who want to master programming from the ground up — writing every line by hand, with zero auto-completion getting in the way.

## 🤔 Why BDH IDE? (The Core Philosophy)

Most modern editors ship with auto-complete and AI assistance baked in. Great for shipping fast, but it quietly robs beginners of the syntax practice that builds real fluency.

BDH IDE is built differently:

- 💪 **Muscle Memory** — No auto-complete. You type every bracket and semicolon yourself, which deeply embeds the language syntax into your muscle memory.
- 🧠 **Real Problem Solving** — You debug your own errors without anything holding your hand, building true problem-solving skills.
- 🔥 **Hardcore Experience** — Mastering a terminal-based workflow makes you a true power user. Once you master this, you can master any GUI-based editor effortlessly.

## ⚙️ Features

- **Fully Terminal Based** — Runs completely inside your terminal.
- **Custom Layout** — Split screens perfectly organized with Ranger (File Manager) and Nano (Editor).
- **Lightweight** — Consumes almost zero system resources compared to Electron-based editors.
- **One-Command Setup** — Easily installable globally across your system.

## 🧰 The BDH Tool Suite (Included Utilities)

BDH IDE isn't just an editor; it comes packed with a powerful suite of terminal utilities to supercharge your workflow:

- **`bdh-bd`** — The core background daemon/utility tool. Installs PostgreSQL and drops you straight into the `postgres#` shell so you can create a DB right away.
- **`bdh-browser`** — A seamless, lightweight terminal-integrated browser experience (built on `cage`) without leaving your editor.
- **`bdh-rec`** — Built-in terminal session recorder. Capture your coding workflow, create demos, and share your terminal sessions effortlessly.

## 📦 Installation

### Method 1: Install via AUR (Recommended for Arch/Manjaro users)

You can easily install BDH Linux IDE directly from the Arch User Repository using your favorite AUR helper (like `yay` or `paru`):

\`\`\`bash
yay -S bdh-linux-ide
\`\`\`

### Method 2: Manual Installation (Git Clone)

\`\`\`bash
git clone https://github.com/BackendDeveloperHub/bdh-linux-ide.git
cd bdh-linux-ide
chmod +x install.sh
./install.sh
\`\`\`

## 🚀 Usage

To launch the editor environment, simply type:

\`\`\`bash
bdh-ide
\`\`\`

To close/kill the sessions safely:

\`\`\`bash
bdh-ide-kill
\`\`\`

## 🗑️ Uninstallation

If installed manually:

\`\`\`bash
cd bdh-linux-ide
chmod +x uninstall.sh
./uninstall.sh
\`\`\`

If installed via AUR:

\`\`\`bash
yay -R bdh-linux-ide
\`\`\`

---

Developed and maintained with ❤️ by **Prabakaran** | **BDH**
