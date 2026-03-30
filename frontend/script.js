// ── Config ───────────────────────────────────────────────────
// Use relative paths so it works whether served from file:// or http://
const API = (typeof window !== 'undefined' && window.location.protocol === 'file:') 
  ? 'http://127.0.0.1:8000'  // Fallback for file:// protocol
  : '';  // Use relative paths when served from http://

// ── State ────────────────────────────────────────────────────
let searchMode = 'name';

// ── Utility helpers ──────────────────────────────────────────
function log(msg, type = '') {
  const el = document.createElement('div');
  const ts = new Date().toLocaleTimeString('en-GB', { hour12: false });
  el.className = `log-entry ${type}`;
  el.textContent = `[${ts}] ${msg}`;
  const body = document.getElementById('log');
  body.appendChild(el);
  body.scrollTop = body.scrollHeight;
}

function toast(msg, type = 'ok') {
  const el = document.getElementById('toast');
  el.textContent = msg;
  el.className = `toast ${type}`;
  clearTimeout(el._timer);
  el._timer = setTimeout(() => { el.className = 'toast hidden'; }, 3200);
}

async function api(method, path, body = null) {
  const opts = {
    method,
    headers: { 'Content-Type': 'application/json' },
  };
  if (body) opts.body = JSON.stringify(body);
  const res = await fetch(API + path, opts);
  const data = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(data.detail || `HTTP ${res.status}`);
  return data;
}

// ── Health check & count ─────────────────────────────────────
async function checkHealth() {
  try {
    const d = await api('GET', '/health');
    document.getElementById('status-text').textContent = `ONLINE · ${d.engine.toUpperCase()}`;
    document.getElementById('status-text').style.color = 'var(--accent)';
    document.querySelector('.dot').className = 'dot pulse';
    log('API connected. Engine: ' + d.engine, 'ok');
  } catch {
    document.getElementById('status-text').textContent = 'OFFLINE — start the API';
    document.getElementById('status-text').style.color = 'var(--danger)';
    document.querySelector('.dot').className = 'dot';
    document.querySelector('.dot').style.background = 'var(--danger)';
    log('API unreachable. Run: uvicorn backend.api:app --reload', 'error');
  }
}

function updateCount(n) {
  document.getElementById('contact-count').textContent = `${n} CONTACT${n !== 1 ? 'S' : ''}`;
}

// ── Load all contacts ────────────────────────────────────────
async function loadContacts() {
  try {
    const d = await api('GET', '/contacts');
    renderTable(d.contacts);
    updateCount(d.count);
    log(`Loaded ${d.count} contact(s).`, 'info');
  } catch (e) {
    log('Failed to load contacts: ' + e.message, 'error');
  }
}

function renderTable(contacts) {
  const tbody = document.getElementById('contacts-body');
  if (!contacts || contacts.length === 0) {
    tbody.innerHTML = `<tr><td colspan="4" class="empty-row">NO CONTACTS FOUND</td></tr>`;
    return;
  }
  tbody.innerHTML = contacts.map((c, i) => `
    <tr>
      <td>${String(i + 1).padStart(2, '0')}</td>
      <td>${esc(c.name)}</td>
      <td>${esc(c.phone)}</td>
      <td>
        <div class="row-actions">
          <button class="btn-row btn-row-del" onclick="deleteContact('${esc(c.name)}')">✕ DEL</button>
        </div>
      </td>
    </tr>
  `).join('');
}

function esc(s) {
  return String(s)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

// ── Add contact ──────────────────────────────────────────────
async function addContact() {
  const name  = document.getElementById('add-name').value.trim();
  const phone = document.getElementById('add-phone').value.trim();
  if (!name || !phone) { toast('Name and phone are required.', 'warn'); return; }

  try {
    await api('POST', '/contacts', { name, phone });
    toast(`✅ '${name}' added.`, 'ok');
    log(`ADD  → ${name} | ${phone}`, 'ok');
    document.getElementById('add-name').value = '';
    document.getElementById('add-phone').value = '';
    loadContacts();
  } catch (e) {
    toast(`❌ ${e.message}`, 'error');
    log('ADD FAILED: ' + e.message, 'error');
  }
}

// ── Search ───────────────────────────────────────────────────
function setSearchMode(mode) {
  searchMode = mode;
  document.getElementById('toggle-name').classList.toggle('active', mode === 'name');
  document.getElementById('toggle-phone').classList.toggle('active', mode === 'phone');
  document.getElementById('search-label').textContent = mode === 'name' ? 'NAME' : 'PHONE';
  document.getElementById('search-input').placeholder = mode === 'name' ? 'Search name…' : 'Search number…';
}

async function searchContact() {
  const val = document.getElementById('search-input').value.trim();
  if (!val) { toast('Enter a search term.', 'warn'); return; }

  try {
    const param = searchMode === 'name' ? `name=${encodeURIComponent(val)}` : `phone=${encodeURIComponent(val)}`;
    const d = await api('GET', `/contacts/search?${param}`);
    renderTable(d.contacts);
    updateCount(d.count);
    toast(`Found ${d.count} result(s).`, d.count ? 'ok' : 'warn');
    log(`SEARCH (${searchMode}="${val}") → ${d.count} result(s).`, d.count ? 'ok' : 'warn');
  } catch (e) {
    toast(`❌ ${e.message}`, 'error');
    log('SEARCH FAILED: ' + e.message, 'error');
  }
}

// ── Update ───────────────────────────────────────────────────
async function updateContact() {
  const old_name  = document.getElementById('upd-old').value.trim();
  const new_name  = document.getElementById('upd-name').value.trim();
  const new_phone = document.getElementById('upd-phone').value.trim();
  if (!old_name || !new_name || !new_phone) { toast('All fields required for update.', 'warn'); return; }

  try {
    await api('PUT', `/contacts/${encodeURIComponent(old_name)}`, { new_name, new_phone });
    toast(`✅ '${old_name}' updated.`, 'ok');
    log(`UPDATE → ${old_name} ⟶ ${new_name} | ${new_phone}`, 'ok');
    ['upd-old','upd-name','upd-phone'].forEach(id => document.getElementById(id).value = '');
    loadContacts();
  } catch (e) {
    toast(`❌ ${e.message}`, 'error');
    log('UPDATE FAILED: ' + e.message, 'error');
  }
}

// ── Delete one ───────────────────────────────────────────────
async function deleteContact(name) {
  if (!confirm(`Delete '${name}'?`)) return;
  try {
    await api('DELETE', `/contacts/${encodeURIComponent(name)}`);
    toast(`🗑 '${name}' deleted.`, 'warn');
    log(`DELETE → ${name}`, 'warn');
    loadContacts();
  } catch (e) {
    toast(`❌ ${e.message}`, 'error');
    log('DELETE FAILED: ' + e.message, 'error');
  }
}

// ── Delete all ───────────────────────────────────────────────
async function deleteAll() {
  if (!confirm('Delete ALL contacts? This cannot be undone.')) return;
  try {
    const d = await api('DELETE', '/contacts');
    toast(`🗑 ${d.message}`, 'warn');
    log('DELETE ALL executed.', 'warn');
    loadContacts();
  } catch (e) {
    toast(`❌ ${e.message}`, 'error');
    log('DELETE ALL FAILED: ' + e.message, 'error');
  }
}

// ── Enter key shortcuts ──────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  ['add-name','add-phone'].forEach(id =>
    document.getElementById(id).addEventListener('keydown', e => {
      if (e.key === 'Enter') addContact();
    })
  );
  document.getElementById('search-input').addEventListener('keydown', e => {
    if (e.key === 'Enter') searchContact();
  });
  ['upd-old','upd-name','upd-phone'].forEach(id =>
    document.getElementById(id).addEventListener('keydown', e => {
      if (e.key === 'Enter') updateContact();
    })
  );

  checkHealth();
  loadContacts();
});
