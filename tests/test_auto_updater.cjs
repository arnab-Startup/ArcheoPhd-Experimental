/**
 * ArchaeoPhD — Streamlined Auto-Updater Test Suite
 * 
 * Verifies:
 * 1. Clean, minimal top-right "Update" button injection
 * 2. Direct 1-click update on button click (zero modals, zero text walls)
 * 3. Handoff to native bridge apply_update
 * 4. Stored version persistence and "Up to date" transition
 * 5. Strict Zero-Emoji compliance
 */

const fs = require('fs');
const path = require('path');
const assert = require('assert');

console.log('================================================================================');
console.log('  ArchaeoPhD Desktop — Streamlined 1-Click Update Verification');
console.log('================================================================================');

const baseDir = path.resolve(__dirname, '../dist/modules/auto-updater');
const files = ['styles.js', 'updater-modal.js', 'index.js'];

for (const f of files) {
  const filePath = path.join(baseDir, f);
  const code = fs.readFileSync(filePath, 'utf8');
  assert(code.length > 50, `File ${f} is too small`);
  const emojiRegex = /[\u{1F300}-\u{1F9FF}\u{2600}-\u{26FF}\u{2700}-\u{27BF}]/u;
  assert(!emojiRegex.test(code), `Emoji found in ${f}`);
  new Function(code);
  console.log(`  [PASS] Syntax & Zero-Emoji check for ${f}`);
}

class MockElement {
  constructor(tag, id = '', className = '') {
    this.tagName = (tag || 'DIV').toUpperCase();
    this.id = id;
    this._className = '';
    this.classList = {
      _classes: new Set(),
      add: (c) => this.classList._classes.add(c),
      remove: (c) => this.classList._classes.delete(c),
      contains: (c) => this.classList._classes.has(c)
    };
    this.className = className;
    this.children = [];
    this.parentNode = null;
    this.listeners = {};
    this._innerHTML = '';
  }

  set className(val) {
    this._className = val || '';
    if (this.classList && this.classList._classes) {
      this.classList._classes = new Set(this._className ? this._className.split(' ') : []);
    }
  }

  get className() {
    return this._className || '';
  }

  set innerHTML(val) {
    this._innerHTML = val;
  }

  get innerHTML() {
    return this._innerHTML;
  }

  appendChild(child) {
    child.parentNode = this;
    this.children.push(child);
    return child;
  }

  insertBefore(newChild, refChild) {
    newChild.parentNode = this;
    const idx = this.children.indexOf(refChild);
    if (idx !== -1) {
      this.children.splice(idx, 0, newChild);
    } else {
      this.children.push(newChild);
    }
    return newChild;
  }

  removeChild(child) {
    const idx = this.children.indexOf(child);
    if (idx !== -1) {
      this.children.splice(idx, 1);
      child.parentNode = null;
    }
    return child;
  }

  get firstChild() {
    return this.children[0] || null;
  }

  addEventListener(event, fn) {
    this.listeners[event] = this.listeners[event] || [];
    this.listeners[event].push(fn);
  }

  dispatchEvent(event) {
    const fns = this.listeners[event.type] || [];
    for (const fn of fns) fn(event);
  }
}

const mockBody = new MockElement('BODY');
const mockHead = new MockElement('HEAD');
const mockHeader = new MockElement('HEADER');
const mockHeaderRight = new MockElement('DIV', '', 'ml-auto flex items-center gap-2');
mockHeader.appendChild(mockHeaderRight);
mockBody.appendChild(mockHeader);

const mockDocument = {
  head: mockHead,
  body: mockBody,
  documentElement: new MockElement('HTML'),
  createElement: (tag) => new MockElement(tag),
  getElementById: (id) => {
    const search = (node) => {
      if (node.id === id) return node;
      for (const ch of node.children) {
        const found = search(ch);
        if (found) return found;
      }
      return null;
    };
    return search(mockBody);
  },
  querySelector: (sel) => {
    if (sel.includes('header div.ml-auto')) return mockHeaderRight;
    if (sel.includes('header')) return mockHeader;
    return null;
  },
  addEventListener: () => {}
};

const mockStorage = {
  store: {},
  getItem: (k) => mockStorage.store[k] || null,
  setItem: (k, v) => { mockStorage.store[k] = String(v); }
};

let appliedVersion = null;

const mockBridge = {
  call: async (action, payload) => {
    if (action === 'check_for_updates') {
      return {
        current_version: mockStorage.getItem('archaeophd_app_version') || '1.0.0',
        latest_version: '1.2.0',
        update_available: true
      };
    }
    if (action === 'apply_update') {
      appliedVersion = payload.target_version;
      return { success: true, to_version: appliedVersion };
    }
    return null;
  }
};

const mockWindow = {
  document: mockDocument,
  localStorage: mockStorage,
  location: { pathname: '/dashboard', reload: () => { mockWindow._reloaded = true; } },
  APD_BRIDGE: mockBridge,
  MutationObserver: class {
    constructor(cb) { this.cb = cb; }
    observe() {}
    disconnect() {}
  },
  requestAnimationFrame: (cb) => cb(),
  setTimeout: (cb, ms) => setTimeout(cb, ms),
  clearTimeout: (id) => clearTimeout(id)
};
mockWindow.window = mockWindow;

const vm = require('vm');
const context = vm.createContext(mockWindow);

const stylesCode = fs.readFileSync(path.join(baseDir, 'styles.js'), 'utf8');
vm.runInContext(stylesCode, context);

const modalCode = fs.readFileSync(path.join(baseDir, 'updater-modal.js'), 'utf8');
vm.runInContext(modalCode, context);

const indexCode = fs.readFileSync(path.join(baseDir, 'index.js'), 'utf8');
vm.runInContext(indexCode, context);

(async function runIntegration() {
  await new Promise(r => setTimeout(r, 60));

  // 1. Verify clean, simple "Update" button is injected in the top right
  const headerBtn = mockDocument.getElementById('apd-header-update-btn');
  assert(headerBtn, 'Header button must be injected');
  assert(headerBtn.className.includes('has-update'), 'State must be has-update');
  assert(headerBtn.innerHTML.includes('Update'), 'Button must say "Update"');
  assert(!headerBtn.innerHTML.includes('100% Zero Data Loss Guarantee'), 'Must not contain wall of text');
  console.log('  [PASS] Clean "Update" button injected in header (zero unwanted info)');

  // 2. Direct 1-Click Update: Clicking "Update" directly triggers the update
  headerBtn.dispatchEvent({ type: 'click', preventDefault: () => {}, stopPropagation: () => {} });
  assert(headerBtn.className.includes('updating'), 'Button immediately enters updating state');
  assert(headerBtn.innerHTML.includes('Updating...'), 'Button indicates update in progress');
  console.log('  [PASS] Clicking button directly initiates update without intermediate dialogs');

  // Allow async update to complete
  await new Promise(r => setTimeout(r, 100));
  assert(appliedVersion === '1.2.0', 'apply_update was invoked with target version');
  assert(mockStorage.getItem('archaeophd_app_version') === '1.2.0', 'New version saved to local storage');
  assert(headerBtn.className.includes('up-to-date'), 'Button switches to up-to-date state');
  assert(headerBtn.innerHTML.includes('Up to date'), 'Button text updates to "Up to date"');
  console.log('  [PASS] Direct update completes and switches to "Up to date" state');

  console.log('================================================================================');
  console.log('  ALL STREAMLINED 1-CLICK UPDATE TESTS PASSED (100% SUCCESS)');
  console.log('================================================================================');
})();
