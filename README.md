# 📞 PhoneBook Pro

> **A C++-powered contact manager — reimagined as a full-stack web product.**  
> Same core logic. REST API wrapper. Beautiful browser UI.

---

## 🧩 Problem Statement

Managing contacts on the command line is powerful but inaccessible to most users.  
**PhoneBook Pro** takes a C++ console application and transforms it into a real product:

- The **C++ core** stays — it's compiled at runtime into a shared library (`.so`)  
- A **Python FastAPI** layer wraps every C++ function as a REST endpoint  
- A **browser UI** lets anyone use it without touching a terminal  

No database needed. No cloud dependency. Pure C++ memory management, exposed to the world.

---

## 🛠️ Technology Stack

| Layer      | Technology                         | Purpose                         |
|------------|------------------------------------|---------------------------------|
| Backend    | **C++17** (GCC / G++)              | Core contact logic, array ops   |
| Bridge     | **Python ctypes**                  | Load `.so`, call C++ from Python |
| API        | **FastAPI** + **Uvicorn**          | REST endpoints, CORS, Pydantic  |
| Frontend   | **HTML / CSS / Vanilla JS**        | Browser UI, fetch API calls     |
| Runtime    | **Python venv**                    | Isolated, reproducible environment |

---

## 📁 Project Structure

```
phonebook-pro/
├── backend/
│   ├── phonebook.cpp        ← All C++ core functions (add, search, update, delete…)
│   ├── api.py               ← FastAPI wrapper: compiles C++ & exposes REST routes
│   ├── requirements.txt     ← Python dependencies (fastapi, uvicorn)
│   └── phonebook.so         ← Auto-generated at first run (do not commit)
│
├── frontend/
│   ├── index.html           ← Single-page app
│   ├── style.css            ← Retro terminal design system
│   └── script.js            ← API calls, UI rendering, keyboard shortcuts
│
├── screenshots/             ← UI screenshots for README / portfolio
├── demo-video/              ← Screen recording demo
│   └── demo.mp4
│
├── README.md                ← This file
└── HOW_TO_RUN.md            ← Step-by-step VSCode setup guide
```

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| ➕ Add Contact | Add name + phone, duplicate detection |
| 📋 View All | Paginated live table with row count |
| 🔍 Search by Name | Case-insensitive substring match |
| 🔍 Search by Phone | Exact phone number lookup |
| ✏️ Update Contact | Rename or re-number any contact |
| 🗑️ Delete One | Remove a single contact |
| 🗑️ Delete All | Wipe entire phonebook |
| 📊 Count | Live contact count in header |
| 🖥️ System Log | Real-time terminal log of all API calls |

---

## 🚀 Quick Start

```bash
# 1. Clone
git clone https://github.com/YOUR_USERNAME/phonebook-pro.git
cd phonebook-pro

# 2. Create virtual environment & install deps
python -m venv venv
source venv/bin/activate          # Windows: venv\Scripts\activate

pip install -r backend/requirements.txt

# 3. Run API (compiles C++ automatically)
uvicorn backend.api:app --reload --port 8000

# 4. Open frontend
#    Open frontend/index.html in your browser
#    OR: python -m http.server 3000 --directory frontend
```

See **HOW_TO_RUN.md** for the full VSCode step-by-step guide.

---

## 📡 API Reference

Base URL: `http://127.0.0.1:8000`

| Method   | Endpoint                    | Description              |
|----------|-----------------------------|--------------------------|
| `GET`    | `/health`                   | Health check + count     |
| `POST`   | `/contacts`                 | Add contact              |
| `GET`    | `/contacts`                 | List all contacts        |
| `GET`    | `/contacts/count`           | Count only               |
| `GET`    | `/contacts/search?name=...` | Search by name           |
| `GET`    | `/contacts/search?phone=..` | Search by phone          |
| `PUT`    | `/contacts/{name}`          | Update contact           |
| `DELETE` | `/contacts/{name}`          | Delete one contact       |
| `DELETE` | `/contacts`                 | Delete all contacts      |

Interactive docs: **http://127.0.0.1:8000/docs**

---

## 🖥️ Demo

> _Add your screen recording here_

[![Demo Video](screenshots/ui-preview.png)](demo-video/demo.mp4)

---

## 📸 Screenshots

<table>
  <tr>
    <td><img src="screenshots/ui-preview.png" alt="Main UI"/></td>
    <td><img src="screenshots/api-docs.png" alt="API Docs"/></td>
  </tr>
</table>

---

## 🔗 GitHub

> **https://github.com/YOUR_USERNAME/phonebook-pro**

---

## 📄 License

MIT License — free to use, modify, and distribute.

---

## 👤 Author

**Your Name**  
BS Computer Science | C++ & Python Developer  
[LinkedIn](https://linkedin.com/in/YOUR_HANDLE) · [GitHub](https://github.com/YOUR_USERNAME)
