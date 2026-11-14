/**
 * ArchaeoPhD — Welcome Screen Template
 * 
 * Generates the DOM layout for the workstation welcome portal.
 * 100% vector SVG paths, zero emojis.
 */

(function () {
  'use strict';

  function createTemplate() {
    var screen = document.createElement('div');
    screen.id = 'apd-welcome-screen';

    screen.innerHTML = `
      <!-- Top Titlebar -->
      <div class="apd-w-topbar">
        <div class="apd-w-brand-pill">
          <svg viewBox="0 0 100 100" fill="none" xmlns="http://www.w3.org/2000/svg">
            <circle cx="50" cy="18" r="8.5" fill="#f59e0b"/>
            <path d="M50 20 L86 92 L66 92 L50 58 L34 92 L14 92 Z" fill="#9e2a18"/>
            <path d="M28 66 C42 58 58 58 72 66 L75 72 C59 64 41 64 25 72 Z" fill="#ffffff"/>
          </svg>
          <span style="font-family: Georgia, serif; font-weight: 600;">ArchaeoPhD <span style="color:#d4a359; font-style:italic;">AI</span></span>
          <span class="apd-w-version">v1.0.0</span>
        </div>
        <div>
          <button class="apd-w-skip-btn" id="apd-w-enter-workspace">
            Open Workspace &rarr;
          </button>
        </div>
      </div>

      <!-- Center Hero -->
      <div class="apd-w-center">
        <!-- Pure Transparent Vector Emblem -->
        <div class="apd-w-emblem-wrap" id="apd-w-emblem">
          <svg viewBox="0 0 100 100" fill="none" xmlns="http://www.w3.org/2000/svg" style="width: 100%; height: 100%;">
            <!-- Golden Glow -->
            <circle cx="50" cy="18" r="14" fill="url(#goldGlow)" opacity="0.35"/>
            <!-- Sun Disc -->
            <circle cx="50" cy="18" r="8.5" fill="url(#sunGrad)"/>
            <!-- Terracotta 'A' Arch -->
            <path d="M50 20 L86 92 L66 92 L50 58 L34 92 L14 92 Z" fill="url(#terraGrad)"/>
            <!-- Contour Wave Ribbon -->
            <path d="M28 66 C42 58 58 58 72 66 L75 72 C59 64 41 64 25 72 Z" fill="#ffffff" opacity="0.95"/>
            <path d="M32 75 C43 69 57 69 68 75 L70 80 C58 74 44 74 30 80 Z" fill="#ffffff" opacity="0.85"/>
            <defs>
              <linearGradient id="terraGrad" x1="14" y1="20" x2="86" y2="92" gradientUnits="userSpaceOnUse">
                <stop stop-color="#b83820"/>
                <stop offset="0.5" stop-color="#932b1a"/>
                <stop offset="1" stop-color="#67190c"/>
              </linearGradient>
              <radialGradient id="sunGrad" cx="0" cy="0" r="1" gradientUnits="userSpaceOnUse" gradientTransform="translate(48 16) scale(10)">
                <stop stop-color="#fef08a"/>
                <stop offset="0.6" stop-color="#f59e0b"/>
                <stop offset="1" stop-color="#b45309"/>
              </radialGradient>
              <radialGradient id="goldGlow" cx="0" cy="0" r="1" gradientUnits="userSpaceOnUse" gradientTransform="translate(50 18) scale(14)">
                <stop stop-color="#fde047"/>
                <stop offset="1" stop-color="#d97706" stop-opacity="0"/>
              </radialGradient>
            </defs>
          </svg>
        </div>

        <h2 class="apd-w-brand-title">ArchaeoPhD<span class="gold-ai">AI</span></h2>
        <div class="apd-w-tagline">AI-Powered Archaeological Research Workspace</div>

        <h1 class="apd-w-main-heading">Welcome to ArchaeoPhD<span class="gold-ai-heading">AI</span></h1>
        <p class="apd-w-sub-desc">
          Your intelligent companion for archaeological research, evidence analysis and scholarly writing.
        </p>

        <!-- Four Action Cards -->
        <div class="apd-w-cards-grid">
          <!-- Card 1: Explore Demo (Active/Featured) -->
          <div class="apd-w-card apd-w-card-featured" id="apd-card-demo">
            <div class="apd-w-card-icon-wrap">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" style="width:26px; height:26px;">
                <polygon points="5 3 19 12 5 21 5 3" fill="rgba(255,255,255,0.2)"></polygon>
              </svg>
            </div>
            <div class="apd-w-card-title">Explore Demo</div>
            <div class="apd-w-card-desc">Try a sample research project with real data</div>
          </div>

          <!-- Card 2: Get Started -->
          <div class="apd-w-card apd-w-card-glass" id="apd-card-start">
            <div class="apd-w-card-icon-wrap">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" style="width:24px; height:24px; color:#d4a359;">
                <rect x="3" y="3" width="18" height="18" rx="2" ry="2"></rect>
                <line x1="12" y1="8" x2="12" y2="16"></line>
                <line x1="8" y1="12" x2="16" y2="12"></line>
              </svg>
            </div>
            <div class="apd-w-card-title">Get Started</div>
            <div class="apd-w-card-desc">Create a new account and set up your workspace</div>
          </div>

          <!-- Card 3: Open Workspace -->
          <div class="apd-w-card apd-w-card-glass" id="apd-card-workspace">
            <div class="apd-w-card-icon-wrap">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" style="width:24px; height:24px; color:#d4a359;">
                <path d="M4 20h16a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-7.93a2 2 0 0 1-1.66-.9l-.82-1.2A2 2 0 0 0 7.93 3H4a2 2 0 0 0-2 2v13c0 1.1.9 2 2 2Z"></path>
              </svg>
            </div>
            <div class="apd-w-card-title">Open Workspace</div>
            <div class="apd-w-card-desc">Continue with your existing projects</div>
          </div>

          <!-- Card 4: Sign In -->
          <div class="apd-w-card apd-w-card-glass" id="apd-card-signin">
            <div class="apd-w-card-icon-wrap">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" style="width:24px; height:24px; color:#d4a359;">
                <path d="M19 21v-2a4 4 0 0 0-4-4H9a4 4 0 0 0-4 4v2"></path>
                <circle cx="12" cy="7" r="4"></circle>
              </svg>
            </div>
            <div class="apd-w-card-title">Sign In</div>
            <div class="apd-w-card-desc">Access your account and research data.</div>
          </div>
        </div>

        <!-- Middle Feature Ribbon -->
        <div class="apd-w-ribbon-wrap">
          <div class="apd-w-ribbon-node"></div>
          <div class="apd-w-ribbon-items">
            <!-- 1. AI Research -->
            <button class="apd-w-ribbon-btn" data-route="/ai-research">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                <path d="M12 2a4 4 0 0 1 4 4c0 .7-.2 1.4-.5 2H18a4 4 0 0 1 4 4c0 1.5-.8 2.8-2 3.5.3.8.4 1.6.3 2.5a4 4 0 0 1-4 4h-2a2 2 0 0 1-2-2v-4"></path>
                <path d="M12 2a4 4 0 0 0-4 4c0 .7.2 1.4.5 2H6a4 4 0 0 0-4 4c0 1.5.8 2.8 2 3.5-.3.8-.4 1.6-.3 2.5a4 4 0 0 0 4 4h2a2 2 0 0 0 2-2v-4"></path>
              </svg>
              <span>AI Research</span>
            </button>

            <!-- 2. Evidence & Argumentation -->
            <button class="apd-w-ribbon-btn" data-route="/evidence-graph">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                <circle cx="6" cy="6" r="3"></circle>
                <circle cx="18" cy="6" r="3"></circle>
                <circle cx="18" cy="18" r="3"></circle>
                <circle cx="6" cy="18" r="3"></circle>
                <line x1="8.5" y1="7.5" x2="15.5" y2="16.5"></line>
                <line x1="8.5" y1="16.5" x2="15.5" y2="7.5"></line>
              </svg>
              <span>Evidence & Argumentation</span>
            </button>

            <!-- 3. Artifacts & Sites -->
            <button class="apd-w-ribbon-btn" data-route="/sites">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                <polygon points="12 2 2 7 12 12 22 7 12 2"></polygon>
                <polyline points="2 17 12 22 22 17"></polyline>
                <polyline points="2 12 12 17 22 12"></polyline>
              </svg>
              <span>Artifacts & Sites</span>
            </button>

            <!-- 4. Chronology -->
            <button class="apd-w-ribbon-btn" data-route="/chronology">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                <circle cx="12" cy="12" r="10"></circle>
                <polyline points="12 6 12 12 16 14"></polyline>
              </svg>
              <span>Chronology</span>
            </button>

            <!-- 5. Literature Graph -->
            <button class="apd-w-ribbon-btn" data-route="/literature-graph">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                <circle cx="12" cy="5" r="3"></circle>
                <circle cx="5" cy="19" r="3"></circle>
                <circle cx="19" cy="19" r="3"></circle>
                <line x1="10" y1="7.5" x2="6.5" y2="16.5"></line>
                <line x1="14" y1="7.5" x2="17.5" y2="16.5"></line>
              </svg>
              <span>Literature Graph</span>
            </button>

            <!-- 6. Thesis & Writing -->
            <button class="apd-w-ribbon-btn" data-route="/thesis">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
                <path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"></path>
                <polyline points="14 2 14 8 20 8"></polyline>
                <line x1="16" y1="13" x2="8" y2="13"></line>
                <line x1="16" y1="17" x2="8" y2="17"></line>
                <polyline points="10 9 9 9 8 9"></polyline>
              </svg>
              <span>Thesis Editor & AI Studio</span>
            </button>
          </div>
        </div>
      </div>

      <!-- Bottom Trust Badges -->
      <div class="apd-w-trust-bar">
        <div class="apd-w-trust-item">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <line x1="1" y1="1" x2="23" y2="23"></line>
            <path d="M16.72 11.06A10.94 10.94 0 0 1 19 12.55"></path>
            <path d="M5 12.55a10.94 10.94 0 0 1 5.17-2.39"></path>
            <path d="M10.71 5.05A16 16 0 0 1 22.58 9"></path>
            <path d="M1.42 9a15.91 15.91 0 0 1 4.7-2.88"></path>
            <path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path>
            <line x1="12" y1="20" x2="12.01" y2="20"></line>
          </svg>
          <div>
            <span class="apd-w-trust-title">Offline Mode</span>
            <span class="apd-w-trust-sub">(No Internet Required)</span>
          </div>
        </div>

        <div class="apd-w-trust-item">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <ellipse cx="12" cy="5" rx="9" ry="3"></ellipse>
            <path d="M3 5V19A9 3 0 0 0 21 19V5"></path>
            <path d="M3 12A9 3 0 0 0 21 12"></path>
          </svg>
          <div>
            <span class="apd-w-trust-title">Local Database</span>
            <span class="apd-w-trust-sub">(Your Data Stays on Your PC)</span>
          </div>
        </div>

        <div class="apd-w-trust-item">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"></path>
            <path d="m9 12 2 2 4-4"></path>
          </svg>
          <div>
            <span class="apd-w-trust-title">Privacy First</span>
            <span class="apd-w-trust-sub">(No Data Upload)</span>
          </div>
        </div>
      </div>
    `;

    return screen;
  }

  window.WelcomeTemplate = {
    create: createTemplate
  };
})();

