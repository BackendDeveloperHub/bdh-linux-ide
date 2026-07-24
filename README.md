<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>BDH Linux IDE - Documentation</title>
    <style>
        :root {
            --bg-color: #f6f8fa;
            --text-color: #24292f;
            --link-color: #0969da;
            --code-bg: #1f2428;
            --code-text: #e1e4e8;
            --border-color: #d0d7de;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
            line-height: 1.6;
            color: var(--text-color);
            background-color: var(--bg-color);
            margin: 0;
            padding: 20px;
        }
        .container {
            max-width: 850px;
            margin: 0 auto;
            background: #ffffff;
            padding: 40px;
            border-radius: 8px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.1);
            border: 1px solid var(--border-color);
        }
        h1, h2, h3 {
            border-bottom: 1px solid var(--border-color);
            padding-bottom: 0.3em;
            margin-top: 1.5em;
        }
        h1 { font-size: 2em; margin-top: 0; }
        h2 { font-size: 1.5em; }
        h3 { font-size: 1.25em; border-bottom: none; }
        p { margin-top: 0; margin-bottom: 16px; }
        ul { padding-left: 2em; margin-bottom: 16px; }
        li { margin-bottom: 0.5em; }
        pre {
            background-color: var(--code-bg);
            color: var(--code-text);
            padding: 16px;
            overflow: auto;
            border-radius: 6px;
            font-family: ui-monospace, SFMono-Regular, "SF Mono", Menlo, Consolas, monospace;
            font-size: 85%;
            line-height: 1.45;
        }
        code {
            font-family: inherit;
        }
        p code, li code {
            background-color: rgba(175, 184, 193, 0.2);
            padding: 0.2em 0.4em;
            border-radius: 6px;
            font-size: 85%;
            color: #24292f;
        }
        a {
            color: var(--link-color);
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        hr {
            height: 1px;
            padding: 0;
            margin: 24px 0;
            background-color: var(--border-color);
            border: 0;
        }
        footer {
            text-align: center;
            margin-top: 40px;
            color: #57606a;
            font-size: 0.9em;
        }
    </style>
</head>
<body>

<div class="container">
    <h1>🚀 BDH Linux IDE</h1>
    <p><strong>A Lightweight, Terminal-Based Hardcore IDE for Arch & Manjaro Linux</strong></p>

    <p>BDH Linux IDE is a powerful, minimal, and terminal-exclusive Integrated Development Environment built using <strong>Tmux, Ranger, and Nano</strong>. It is designed specifically for developers who want to master programming from the ground up without the distractions of modern AI auto-completions.</p>

    <h2>🤔 Why BDH IDE? (The Core Philosophy)</h2>
    <p>Modern IDEs come with AI assistants and auto-complete features. While great for production, they often hinder beginners from learning the actual core syntax.</p>
    <p><strong>BDH IDE is built differently:</strong></p>
    <ul>
        <li>💪 <strong>Muscle Memory:</strong> No auto-complete. You type every bracket and semicolon, which deeply embeds the language syntax into your muscle memory.</li>
        <li>🧠 <strong>Real Problem Solving:</strong> You debug your own errors without AI holding your hand, building true problem-solving skills.</li>
        <li>🔥 <strong>Hardcore Experience:</strong> Mastering a terminal-based workflow makes you a true power user. Once you master this, you can master any GUI-based IDE effortlessly.</li>
    </ul>

    <h2>⚙️ Features</h2>
    <ul>
        <li><strong>Fully Terminal Based:</strong> Runs completely inside your terminal.</li>
        <li><strong>Custom Layout:</strong> Split screens perfectly organized with Ranger (File Manager) and Nano (Editor).</li>
        <li><strong>Lightweight:</strong> Consumes almost zero system resources compared to Electron-based IDEs.</li>
        <li><strong>One-Command Setup:</strong> Easily installable globally across your system.</li>
    </ul>

    <h2>📦 Installation</h2>
    
    <h3>Method 1: Install via AUR (Recommended for Arch/Manjaro users)</h3>
    <p>You can easily install BDH Linux IDE directly from the Arch User Repository using your favorite helper (like <code>yay</code> or <code>paru</code>):</p>
    <pre><code>yay -S bdh-linux-ide</code></pre>

    <h3>Method 2: Manual Installation (Git Clone)</h3>
    <pre><code>git clone https://github.com/your-username/bdh-linux-ide.git
cd bdh-linux-ide
sudo ./install.sh</code></pre>

    <h2>🚀 Usage</h2>
    <p>To launch the IDE, simply type:</p>
    <pre><code>bdh-ide</code></pre>
    <p>To close/kill the IDE sessions safely:</p>
    <pre><code>bdh-ide-kill</code></pre>

    <h2>🗑️ Uninstallation</h2>
    <p><strong>If installed manually:</strong></p>
    <pre><code>sudo ./uninstall.sh</code></pre>
    <p><strong>If installed via AUR:</strong></p>
    <pre><code>yay -Rns bdh-linux-ide</code></pre>

    <hr>
    
    <footer>
        Developed and maintained with ❤️ by <strong>Prabakaran | Developer Labs</strong>
    </footer>
</div>

</body>
</html>
