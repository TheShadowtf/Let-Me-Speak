# 🎙️ LMS - Let Me Speak

**LMS (Let Me Speak)** is a lightweight, open-source voice and message chat application built in C++ using ImGui. It is **not** made to replace Discord or TeamSpeak — instead, it offers a minimal, privacy-friendly, and self-hostable alternative for small groups of friends who just want to talk and chat without all the bloat.

> 📁 Messages are stored **locally** and **encrypted**.  
> 🌐 Servers can be **self-hosted** or run via a lightweight pre-hosted relay — your choice.  
> 🧩 Clean, simple, and modular. Just plug and speak.

---

## ✨ Features

### ✅ Core Chat
- Encrypted **1:1 direct messages** (no cloud storage)
- Lightweight local message storage (JSON or SQLite)
- Friend system with add/remove and presence detection
- Minimal chat UI built with **ImGui**

### 🔊 Voice Communication
- Peer-to-peer or relay-based voice calls
- High-quality audio using **Opus** codec
- One-to-one voice calls
- Group voice channels (small, invite-based)

### 🖥️ Server Options
- Host a server directly from the LMS app (like TeamSpeak)
- Optional pre-hosted relay servers for NAT traversal
- Invite-based access to group channels

### 🔐 Privacy & Security
- **End-to-end encryption** for messages
- Local key storage with password-based access
- No centralized data storage

### ⚙️ Lightweight Tech Stack
- Written in modern **C++**
- UI powered by **ImGui** for speed and portability
- Uses **PortAudio** and **Opus** for sound
- Networking via **Boost.Asio** / **ENet** / custom protocol (WIP)

---

## 🚀 Getting Started

Coming soon! Until then, clone the repo and build the project using your preferred C++ environment with CMake support.

```bash
git clone https://github.com/your-username/LMS.git
cd LMS
mkdir build && cd build
cmake ..
make
./LMS
```

---

## 🛠️ Roadmap

- [x] Initial UI layout (Channels + Chat)
- [ ] Encrypted local messaging
- [ ] One-to-one voice calls
- [ ] Self-hostable server mode
- [ ] Basic friend list & status
- [ ] Group voice channels
- [ ] Push-to-talk & VAD support
- [ ] NAT traversal (STUN/TURN or fallback relay)
- [ ] Custom themes
- [ ] Export/import chats feature

---

## 🤝 Contributing

Contributions are welcome! Whether it’s bug fixes, new features, or just ideas — feel free to open issues or pull requests.

---

## 📄 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---

## ❤️ A Note from the Creator

LMS is built out of a passion for clean, user-first communication tools. It’s not here to compete with the giants — just to offer a faster, simpler, and more private alternative for the people who want it.
