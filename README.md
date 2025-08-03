<p align="center"><img src="https://github.com/ViewTechOrg/Server/blob/main/ratC2/photo_2025-08-03_20-54-40.jpg" width=600></p>
<p><h1 align="center">0x Team C2 - Web-Based Golang RAT</h1>
  <p align="center">
    <img src="https://img.shields.io/github/stars/ViewTechOrg/0xTeam-C2-WebBased---Golang?style=flat-square&logo=github">
    <img src="https://img.shields.io/github/forks/ViewTechOrg/0xTeam-C2-WebBased---Golang?style=flat-square&logo=gojek">
    <img src="https://img.shields.io/github/issues/ViewTechOrg/0xTeam-C2-WebBased---Golang?style=flat-square&logo=googledocs">
    <img src="https://img.shields.io/github/last-commit/ViewTechOrg/0xTeam-C2-WebBased---Golang?style=flat-square&logo=clockify"><br>
    <img src="https://img.shields.io/github/languages/top/ViewTechOrg/0xTeam-C2-WebBased---Golang?style=flat-square&logo=go" alt="Top Language">
    <img src="https://img.shields.io/github/license/ViewTechOrg/0xTeam-C2-WebBased---Golang?style=flat-square&logo=open-source-initiative" alt="License">
    <img src="https://img.shields.io/badge/Made%20With-Golang-blue?style=flat-square&logo=go" alt="Made with Golang">
    <img src="https://img.shields.io/badge/Client-C++-informational?style=flat-square&logo=c%2B%2B" alt="Client C++"><br>
    <img src="https://img.shields.io/badge/Type-RAT-critical?style=flat-square&logo=virustotal" alt="RAT Type">
    <img src="https://img.shields.io/badge/Control-Web%20Panel-success?style=flat-square&logo=googlechrome" alt="Web Control Panel">
    <img src="https://img.shields.io/badge/Platform-Windows-lightgrey?style=flat-square&logo=windows" alt="Platform Windows">
    <img src="https://img.shields.io/badge/UI-HTML5-orange?style=flat-square&logo=html5" alt="HTML5 UI">
  </p><br>

---

## 📖 Penjelasan

**0x Team C2** adalah alat **Remote Access Trojan (RAT)** yang dapat dikontrol melalui website lokal.  
- Backend listener dibuat dengan **Golang**  
- Payload (client) dibuat dengan **C++**  
- Sistem kontrol dijalankan langsung dari browser (`localhost:8080`)

Uniknya, kontrol RAT ini dilakukan sepenuhnya melalui tampilan website, bukan GUI native.

---

## 🎯 Tujuan
> Remote Access Trojan (RAT) yang dikontrol melalui Website berbasis Golang (Server) dan C++ (Payload)
>    Memberikan kendali jarak jauh terhadap komputer target melalui sistem C2 (Command & Control) berbasis web.

---

## 🚀 Fitur Utama

- 🔐 Sistem login sebelum akses panel
- 🎵 Pemutar musik saat korban terhubung
- 🖥️ Remote Desktop Viewer
- 💻 Remote CMD / Shell Execution
- 📂 File Manager
- 🧠 Process Manager
- 🔁 Persistence (auto startup)
- 🧩 Modul-modul tambahan

---

## 🛠️ Instalasi

1. **Download tools yang dibutuhkan:**

   - [Golang (Windows)](https://go.dev/dl/)
   - [Visual Studio (BUKAN VS Code)](https://visualstudio.microsoft.com/downloads/)

---

## ⚙️ Cara Menjalankan

### 💡 Jalankan Listener Golang

```bash
go run server.go
```

Akan muncul output:
```
TCP start at 9000
HTTP start at 8080
```

### 🌐 Akses Panel Web

Buka di browser:
```
http://localhost:8080
```

Login dengan default `id: razer` (bisa diubah di source).

### 💣 Compile Client (Payload)

1. Buka folder `GoAheadClient`
2. Jalankan file `.sln` menggunakan Visual Studio
3. Tekan `CTRL + SHIFT + B` untuk Build Payload
4. Hasil compile siap disebar ke target

---

## 🧪 Screenshot

> *Tambahkan screenshot panel & client jika ada*

---

## ❗ Legal Disclaimer

This tool is intended for **educational purposes only**.  
Author is **not responsible** for any misuse of this project.  
**Use at your own risk.**

---

## 🙋 Kontak Developer

🌐 sosial media : <a href="https://www.tiktok.com/@jack.kd84?_t=ZS-8yZRU7tE9FX&_r=1"><img src="https://img.shields.io/badge/tiktok%20-jackKD%20-blue?logo=tiktok"></a><br>
📂 Repository: <a href="https://github.com/ViewTechOrg/0xTeam-C2-WebBased---Golang"><img src="https://img.shields.io/badge/Golang-C2WebBased-blue?logo=goland"></a>

---

## 📄 License

MIT License — Free to use with proper credit.
