/*
 * ============================================================
 *  PhoneBook Pro — Core C++ Backend
 *  Author  : Your Name
 *  Version : 2.0.0
 *  License : MIT
 * ============================================================
 *
 *  This file contains ALL core phonebook logic.
 *  It is compiled into a shared library (.so / .dll) that the
 *  Python FastAPI wrapper loads via ctypes.
 *
 *  Functions exposed (extern "C"):
 *    pb_init()
 *    pb_add(name, phone)        -> 0 = success, 1 = duplicate, 2 = full
 *    pb_get_all(out_buf, size)  -> count
 *    pb_search_by_name(name, out_buf, size) -> found count
 *    pb_search_by_phone(phone, out_buf, size) -> found count
 *    pb_update(old_name, new_name, new_phone) -> 0 = ok, 1 = not found
 *    pb_delete(name)            -> 0 = ok, 1 = not found
 *    pb_delete_all()            -> count deleted
 *    pb_count()                 -> int
 * ============================================================
 */

#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>

// ── Constants ──────────────────────────────────────────────
static const int MAX_CONTACTS = 100;
static const int FIELD_LEN    = 64;   // max chars per name/phone
static const int BUF_ENTRY    = 130;  // "name|phone\n" per record

// ── Internal storage ───────────────────────────────────────
static char g_names [MAX_CONTACTS][FIELD_LEN];
static char g_phones[MAX_CONTACTS][FIELD_LEN];
static int  g_count = 0;

// ── Helpers ────────────────────────────────────────────────

/** Case-insensitive string comparison */
static bool iequal(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;
        ++a; ++b;
    }
    return *a == '\0' && *b == '\0';
}

/** Find index of contact by name (-1 if not found) */
static int find_by_name(const char* name) {
    for (int i = 0; i < g_count; i++)
        if (iequal(g_names[i], name))
            return i;
    return -1;
}

/** Find index of contact by phone (-1 if not found) */
static int find_by_phone(const char* phone) {
    for (int i = 0; i < g_count; i++)
        if (strcmp(g_phones[i], phone) == 0)
            return i;
    return -1;
}

/** Remove element at index and shift array left */
static void remove_at(int idx) {
    for (int i = idx; i < g_count - 1; i++) {
        strncpy(g_names [i], g_names [i+1], FIELD_LEN);
        strncpy(g_phones[i], g_phones[i+1], FIELD_LEN);
    }
    g_names [g_count-1][0] = '\0';
    g_phones[g_count-1][0] = '\0';
    g_count--;
}

// ── Public API (extern "C" for ctypes / FFI) ───────────────
extern "C" {

/**
 * Initialize / reset the phonebook (call once at startup).
 */
void pb_init() {
    g_count = 0;
    for (int i = 0; i < MAX_CONTACTS; i++) {
        g_names [i][0] = '\0';
        g_phones[i][0] = '\0';
    }
}

/**
 * Add a new contact.
 * Returns: 0 = success | 1 = duplicate name | 2 = phonebook full
 */
int pb_add(const char* name, const char* phone) {
    if (g_count >= MAX_CONTACTS) return 2;
    if (find_by_name(name) != -1) return 1;     // duplicate

    strncpy(g_names [g_count], name,  FIELD_LEN - 1);
    strncpy(g_phones[g_count], phone, FIELD_LEN - 1);
    g_names [g_count][FIELD_LEN-1] = '\0';
    g_phones[g_count][FIELD_LEN-1] = '\0';
    g_count++;
    return 0;
}

/**
 * Serialize all contacts into out_buf as "name|phone\n" lines.
 * Returns number of contacts written.
 */
int pb_get_all(char* out_buf, int buf_size) {
    out_buf[0] = '\0';
    int written = 0;
    for (int i = 0; i < g_count; i++) {
        char entry[BUF_ENTRY];
        snprintf(entry, sizeof(entry), "%s|%s\n", g_names[i], g_phones[i]);
        if ((int)(strlen(out_buf) + strlen(entry)) >= buf_size) break;
        strcat(out_buf, entry);
        written++;
    }
    return written;
}

/**
 * Search by name (case-insensitive substring match).
 * Returns count of matches; results written to out_buf.
 */
int pb_search_by_name(const char* name, char* out_buf, int buf_size) {
    out_buf[0] = '\0';
    int found = 0;
    std::string needle(name);
    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);

    for (int i = 0; i < g_count; i++) {
        std::string hay(g_names[i]);
        std::transform(hay.begin(), hay.end(), hay.begin(), ::tolower);

        if (hay.find(needle) != std::string::npos) {
            char entry[BUF_ENTRY];
            snprintf(entry, sizeof(entry), "%s|%s\n", g_names[i], g_phones[i]);
            if ((int)(strlen(out_buf) + strlen(entry)) >= buf_size) break;
            strcat(out_buf, entry);
            found++;
        }
    }
    return found;
}

/**
 * Search by phone number (exact match).
 * Returns count of matches; results written to out_buf.
 */
int pb_search_by_phone(const char* phone, char* out_buf, int buf_size) {
    out_buf[0] = '\0';
    int idx = find_by_phone(phone);
    if (idx == -1) return 0;

    snprintf(out_buf, buf_size, "%s|%s\n", g_names[idx], g_phones[idx]);
    return 1;
}

/**
 * Update an existing contact by old name.
 * Returns: 0 = updated | 1 = not found | 2 = new name already exists
 */
int pb_update(const char* old_name, const char* new_name, const char* new_phone) {
    int idx = find_by_name(old_name);
    if (idx == -1) return 1;

    // Check duplicate only if name is actually changing
    if (!iequal(old_name, new_name) && find_by_name(new_name) != -1)
        return 2;

    strncpy(g_names [idx], new_name,  FIELD_LEN - 1);
    strncpy(g_phones[idx], new_phone, FIELD_LEN - 1);
    return 0;
}

/**
 * Delete a contact by name.
 * Returns: 0 = deleted | 1 = not found
 */
int pb_delete(const char* name) {
    int idx = find_by_name(name);
    if (idx == -1) return 1;
    remove_at(idx);
    return 0;
}

/**
 * Delete ALL contacts.
 * Returns: number of contacts that were deleted.
 */
int pb_delete_all() {
    int deleted = g_count;
    pb_init();
    return deleted;
}

/**
 * Return current total contact count.
 */
int pb_count() {
    return g_count;
}

} // extern "C"


// ── Optional standalone CLI (compile without -DLIB flag) ───
#ifndef LIB_BUILD
int menu();
void start();

int main() {
    pb_init();
    std::string name[MAX_CONTACTS], no[MAX_CONTACTS];

    start();

    int choice;
    do {
        choice = menu();

        if (choice == 1) {
            std::string n, p;
            std::cout << "\n\t\tEnter Name  : "; std::cin >> n;
            std::cout << "\t\tEnter Phone : "; std::cin >> p;
            int r = pb_add(n.c_str(), p.c_str());
            if (r == 0) std::cout << "\n\t\t✅ Contact added!\n";
            else if (r == 1) std::cout << "\n\t\t⚠️  Name already exists.\n";
            else std::cout << "\n\t\t❌ Phonebook is full.\n";
        }
        else if (choice == 2) {
            char buf[MAX_CONTACTS * BUF_ENTRY];
            int n = pb_get_all(buf, sizeof(buf));
            if (n == 0) { std::cout << "\n\t\tNo contacts found.\n"; }
            else {
                std::cout << "\n\t\t--- All Contacts ---\n";
                std::string s(buf);
                for (auto& line : {s}) {
                    size_t pos = 0, found;
                    while ((found = line.find('\n', pos)) != std::string::npos) {
                        std::string entry = line.substr(pos, found - pos);
                        if (!entry.empty()) {
                            auto bar = entry.find('|');
                            std::cout << "\t\tName: " << entry.substr(0, bar)
                                      << "  Phone: " << entry.substr(bar+1) << "\n";
                        }
                        pos = found + 1;
                    }
                }
            }
        }
        else if (choice == 3) {
            std::string p;
            std::cout << "\n\t\tEnter Phone: "; std::cin >> p;
            char buf[512];
            int n = pb_search_by_phone(p.c_str(), buf, sizeof(buf));
            if (n == 0) std::cout << "\n\t\tNot found.\n";
            else std::cout << "\n\t\tFound: " << buf;
        }
        else if (choice == 4) {
            std::string n;
            std::cout << "\n\t\tEnter Name: "; std::cin >> n;
            char buf[MAX_CONTACTS * BUF_ENTRY];
            int cnt = pb_search_by_name(n.c_str(), buf, sizeof(buf));
            if (cnt == 0) std::cout << "\n\t\tNot found.\n";
            else std::cout << "\n\t\tFound " << cnt << " match(es):\n" << buf;
        }
        else if (choice == 5) {
            std::string on, nn, np;
            std::cout << "\n\t\tCurrent Name  : "; std::cin >> on;
            std::cout << "\t\tNew Name      : "; std::cin >> nn;
            std::cout << "\t\tNew Phone     : "; std::cin >> np;
            int r = pb_update(on.c_str(), nn.c_str(), np.c_str());
            if (r == 0) std::cout << "\n\t\t✅ Updated!\n";
            else if (r == 1) std::cout << "\n\t\t❌ Name not found.\n";
            else std::cout << "\n\t\t⚠️  New name already exists.\n";
        }
        else if (choice == 6) {
            std::string n;
            std::cout << "\n\t\tEnter Name to Delete: "; std::cin >> n;
            int r = pb_delete(n.c_str());
            if (r == 0) std::cout << "\n\t\t✅ Deleted.\n";
            else std::cout << "\n\t\t❌ Not found.\n";
        }
        else if (choice == 7) {
            int d = pb_delete_all();
            std::cout << "\n\t\t✅ All " << d << " contacts deleted.\n";
        }
        else if (choice == 8) {
            std::cout << "\n\t\tTotal Contacts: " << pb_count() << "\n";
        }

        if (choice != 9) {
            std::cout << "\n\tPress Enter to continue...";
            std::cin.ignore(); std::cin.get();
        }

    } while (choice != 9);

    std::cout << "\n\t\tGoodbye!\n";
    return 0;
}

int menu() {
    std::cout << "\n\t╔══════════════════════════════╗\n";
    std::cout << "\t║   PHONE BOOK PRO  v2.0       ║\n";
    std::cout << "\t╠══════════════════════════════╣\n";
    std::cout << "\t║  1. Add Contact              ║\n";
    std::cout << "\t║  2. Display All              ║\n";
    std::cout << "\t║  3. Search by Phone          ║\n";
    std::cout << "\t║  4. Search by Name           ║\n";
    std::cout << "\t║  5. Update Contact           ║\n";
    std::cout << "\t║  6. Delete Contact           ║\n";
    std::cout << "\t║  7. Delete All               ║\n";
    std::cout << "\t║  8. Total Contacts           ║\n";
    std::cout << "\t║  9. Exit                     ║\n";
    std::cout << "\t╚══════════════════════════════╝\n";
    std::cout << "\tChoice: ";
    int a; std::cin >> a;
    return a;
}

void start() {
    std::cout << "\n\n\t╔════════════════════════════╗\n";
    std::cout << "\t║    PHONE BOOK PRO  v2.0    ║\n";
    std::cout << "\t║  C++ Powered Contact Mgr   ║\n";
    std::cout << "\t╚════════════════════════════╝\n\n";
}
#endif
