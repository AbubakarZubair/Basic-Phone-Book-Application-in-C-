"""
PhoneBook Pro — Pure Python Backend
====================================
Replacement for phonebook.cpp using plain Python.
Provides all the same functions but without compilation requirements.
"""

# ── Constants ──────────────────────────────────────────────
MAX_CONTACTS = 100
FIELD_LEN = 64
BUF_ENTRY = 130

# ── Internal storage ───────────────────────────────────────
g_contacts = {}  # {name.lower(): (name, phone)}
g_count = 0


# ── Public API ─────────────────────────────────────────────

def pb_init():
    """Initialize the phonebook (reset)."""
    global g_contacts, g_count
    g_contacts = {}
    g_count = 0


def pb_add(name: str, phone: str) -> int:
    """
    Add a contact.
    Returns: 0 = success, 1 = duplicate, 2 = full
    """
    global g_count
    
    if len(g_contacts) >= MAX_CONTACTS:
        return 2  # full
    
    name_key = name.lower()
    if name_key in g_contacts:
        return 1  # duplicate
    
    g_contacts[name_key] = (name, phone)
    g_count += 1
    return 0  # success


def pb_get_all(out_buf: str = "", size: int = 10000) -> str:
    """Get all contacts as a buffer string."""
    result = []
    for _, (name, phone) in g_contacts.items():
        result.append(f"{name}|{phone}\n")
    return "".join(result)


def pb_search_by_name(name: str, out_buf: str = "", size: int = 10000) -> str:
    """Search for contacts by name (case-insensitive partial match)."""
    result = []
    search_lower = name.lower()
    for _, (contact_name, phone) in g_contacts.items():
        if search_lower in contact_name.lower():
            result.append(f"{contact_name}|{phone}\n")
    return "".join(result)


def pb_search_by_phone(phone: str, out_buf: str = "", size: int = 10000) -> str:
    """Search for contacts by phone number."""
    result = []
    for _, (name, contact_phone) in g_contacts.items():
        if phone in contact_phone:
            result.append(f"{name}|{contact_phone}\n")
    return "".join(result)


def pb_update(old_name: str, new_name: str, new_phone: str) -> int:
    """
    Update a contact.
    Returns: 0 = ok, 1 = not found
    """
    old_key = old_name.lower()
    if old_key not in g_contacts:
        return 1  # not found
    
    del g_contacts[old_key]
    new_key = new_name.lower()
    g_contacts[new_key] = (new_name, new_phone)
    return 0  # success


def pb_delete(name: str) -> int:
    """
    Delete a contact.
    Returns: 0 = ok, 1 = not found
    """
    global g_count
    name_key = name.lower()
    if name_key not in g_contacts:
        return 1  # not found
    
    del g_contacts[name_key]
    g_count -= 1
    return 0  # success


def pb_delete_all() -> int:
    """Delete all contacts, return count deleted."""
    global g_contacts, g_count
    count = len(g_contacts)
    g_contacts = {}
    g_count = 0
    return count


def pb_count() -> int:
    """Return count of contacts."""
    return len(g_contacts)
