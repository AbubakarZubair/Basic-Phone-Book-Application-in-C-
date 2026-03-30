"""
PhoneBook Pro — FastAPI Backend
================================
Compiles phonebook.cpp into a shared library at startup,
then wraps every C++ function as a REST endpoint.

Endpoints:
  POST   /contacts           add
  GET    /contacts           list all
  GET    /contacts/search    search by name or phone
  PUT    /contacts/{name}    update
  DELETE /contacts/{name}    delete one
  DELETE /contacts           delete all
  GET    /contacts/count     count
  GET    /health             health check
"""

import sys
from pathlib import Path
from typing import Optional

from fastapi import FastAPI, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel

# ── Import pure Python phonebook module ────────────────────
try:
    from phonebook import (
        pb_init, pb_add, pb_get_all, pb_search_by_name, 
        pb_search_by_phone, pb_update, pb_delete, pb_delete_all, pb_count
    )
except ImportError:
    # Fallback: add backend directory to path
    import sys
    from pathlib import Path
    backend_dir = Path(__file__).parent
    if str(backend_dir) not in sys.path:
        sys.path.insert(0, str(backend_dir))
    from phonebook import (
        pb_init, pb_add, pb_get_all, pb_search_by_name, 
        pb_search_by_phone, pb_update, pb_delete, pb_delete_all, pb_count
    )

# Initialize phonebook at startup
pb_init()
print("✅  Phonebook initialized (pure Python implementation)")


# ── Helpers ────────────────────────────────────────────────
def _parse_contacts(buffer_str: str) -> list[dict]:
    """Parse contact buffer into list of dicts."""
    if not buffer_str or buffer_str.strip() == "":
        return []
    
    contacts = []
    for line in buffer_str.strip().split("\n"):
        if "|" in line:
            name, phone = line.split("|", 1)
            contacts.append({"name": name, "phone": phone})
    return contacts


def _call_get(fn, *args) -> list[dict]:
    """Call a phonebook function that returns formatted contact string."""
    result = fn(*args)
    return _parse_contacts(result)


# ── FastAPI app ────────────────────────────────────────────
app = FastAPI(
    title="PhoneBook Pro API",
    description="REST wrapper around a phonebook backend. "
                "Lightning-fast, zero-compilation, pure Python.",
    version="2.0.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


# ── Pydantic models ────────────────────────────────────────
class ContactIn(BaseModel):
    name: str
    phone: str

class ContactUpdate(BaseModel):
    new_name: str
    new_phone: str


# ── Routes ─────────────────────────────────────────────────

@app.get("/health")
def health():
    return {"status": "ok", "engine": "Pure Python", "contacts": pb_count()}


@app.post("/contacts", status_code=201)
def add_contact(contact: ContactIn):
    result = pb_add(contact.name, contact.phone)
    if result == 1:
        raise HTTPException(status_code=409, detail="Contact with this name already exists.")
    if result == 2:
        raise HTTPException(status_code=507, detail="Phonebook is full (max 100 contacts).")
    return {"message": "Contact added.", "contact": contact}


@app.get("/contacts")
def get_all_contacts():
    contacts = _call_get(pb_get_all)
    return {"count": len(contacts), "contacts": contacts}


@app.get("/contacts/count")
def count_contacts():
    return {"count": pb_count()}


@app.get("/contacts/search")
def search_contacts(
    name:  Optional[str] = Query(None, description="Search by name (substring)"),
    phone: Optional[str] = Query(None, description="Search by exact phone number"),
):
    if not name and not phone:
        raise HTTPException(status_code=400, detail="Provide ?name= or ?phone=")

    if phone:
        contacts = _call_get(pb_search_by_phone, phone)
    else:
        contacts = _call_get(pb_search_by_name, name)

    return {"count": len(contacts), "contacts": contacts}


@app.put("/contacts/{name}")
def update_contact(name: str, body: ContactUpdate):
    result = pb_update(name, body.new_name, body.new_phone)
    if result == 1:
        raise HTTPException(status_code=404, detail="Contact not found.")
    if result == 2:
        raise HTTPException(status_code=409, detail="New name already exists.")
    return {"message": "Contact updated.", "name": body.new_name, "phone": body.new_phone}


@app.delete("/contacts/{name}")
def delete_contact(name: str):
    result = pb_delete(name)
    if result == 1:
        raise HTTPException(status_code=404, detail="Contact not found.")
    return {"message": f"'{name}' deleted."}


@app.delete("/contacts")
def delete_all_contacts():
    deleted = pb_delete_all()
    return {"message": f"All {deleted} contacts deleted."}


# ── Serve frontend static files ────────────────────────────
frontend_dir = Path(__file__).parent.parent / "frontend"
if frontend_dir.exists():
    app.mount("/", StaticFiles(directory=frontend_dir, html=True), name="frontend")
    print(f"✅  Frontend served from {frontend_dir}")
else:
    print(f"⚠️  Frontend directory not found at {frontend_dir}")
