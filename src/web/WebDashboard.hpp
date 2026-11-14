#pragma once

#include <string_view>

namespace smartcampus::web {

inline constexpr std::string_view DASHBOARD_HTML = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Campus Event &amp; Facility Booking Dashboard</title>
  <style>
    :root {
      --bg-primary: #0b0f19;
      --bg-secondary: #111827;
      --bg-card: #1f2937;
      --bg-card-hover: #283548;
      --border-color: #374151;
      --border-light: #4b5563;
      --text-main: #f9fafb;
      --text-muted: #9ca3af;
      --accent-blue: #3b82f6;
      --accent-blue-hover: #2563eb;
      --accent-green: #10b981;
      --accent-green-hover: #059669;
      --accent-red: #ef4444;
      --accent-red-hover: #dc2626;
      --accent-amber: #f59e0b;
      --accent-purple: #8b5cf6;
      --accent-cyan: #06b6d4;
      --radius: 8px;
      --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.3), 0 2px 4px -1px rgba(0, 0, 0, 0.2);
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
      background-color: var(--bg-primary);
      color: var(--text-main);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      overflow-x: hidden;
    }

    /* Top Navigation */
    header {
      background-color: var(--bg-secondary);
      border-bottom: 1px solid var(--border-color);
      padding: 0.75rem 1.5rem;
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 1rem;
      position: sticky;
      top: 0;
      z-index: 100;
    }

    .brand {
      display: flex;
      align-items: center;
      gap: 0.75rem;
    }

    .brand-icon {
      width: 34px;
      height: 34px;
      background: linear-gradient(135deg, #3b82f6, #8b5cf6);
      border-radius: 8px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1.2rem;
      font-weight: bold;
    }

    .brand h1 {
      font-size: 1.15rem;
      font-weight: 700;
      letter-spacing: -0.025em;
    }

    .live-badge {
      display: inline-flex;
      align-items: center;
      gap: 0.4rem;
      font-size: 0.75rem;
      font-weight: 600;
      padding: 0.2rem 0.6rem;
      border-radius: 9999px;
      background-color: rgba(16, 185, 129, 0.15);
      color: var(--accent-green);
      border: 1px solid rgba(16, 185, 129, 0.3);
    }

    .pulse-dot {
      width: 8px;
      height: 8px;
      background-color: var(--accent-green);
      border-radius: 50%;
      animation: pulse 1.5s infinite;
    }

    @keyframes pulse {
      0% { transform: scale(0.95); opacity: 0.8; }
      50% { transform: scale(1.3); opacity: 1; }
      100% { transform: scale(0.95); opacity: 0.8; }
    }

    /* Clock & Quick Actions */
    .header-controls {
      display: flex;
      align-items: center;
      flex-wrap: wrap;
      gap: 0.75rem;
    }

    .clock-display {
      background-color: #0f172a;
      border: 1px solid var(--border-color);
      border-radius: var(--radius);
      padding: 0.4rem 0.8rem;
      display: flex;
      align-items: center;
      gap: 0.5rem;
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      font-size: 0.9rem;
      color: #38bdf8;
    }

    .btn {
      font-size: 0.82rem;
      font-weight: 600;
      padding: 0.45rem 0.85rem;
      border-radius: var(--radius);
      border: none;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 0.4rem;
      transition: all 0.15s ease-in-out;
    }

    .btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }

    .btn-primary { background-color: var(--accent-blue); color: white; }
    .btn-primary:hover:not(:disabled) { background-color: var(--accent-blue-hover); }

    .btn-success { background-color: var(--accent-green); color: white; }
    .btn-success:hover:not(:disabled) { background-color: var(--accent-green-hover); }

    .btn-danger { background-color: var(--accent-red); color: white; }
    .btn-danger:hover:not(:disabled) { background-color: var(--accent-red-hover); }

    .btn-warning { background-color: var(--accent-amber); color: #111827; }
    .btn-warning:hover:not(:disabled) { background-color: #d97706; }

    .btn-secondary {
      background-color: var(--bg-card);
      color: var(--text-main);
      border: 1px solid var(--border-color);
    }
    .btn-secondary:hover:not(:disabled) {
      background-color: var(--bg-card-hover);
      border-color: var(--border-light);
    }

    .btn-sm {
      font-size: 0.75rem;
      padding: 0.25rem 0.5rem;
    }

    /* Scenario Runner Bar */
    .scenario-strip {
      background-color: #172033;
      border-bottom: 1px solid var(--border-color);
      padding: 0.6rem 1.5rem;
      display: flex;
      align-items: center;
      flex-wrap: wrap;
      gap: 0.5rem;
    }

    .scenario-title {
      font-size: 0.8rem;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      color: #94a3b8;
      margin-right: 0.5rem;
    }

    /* Main Container */
    main {
      flex: 1;
      padding: 1.5rem;
      display: flex;
      flex-direction: column;
      gap: 1.5rem;
      max-width: 1800px;
      margin: 0 auto;
      width: 100%;
    }

    .section-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 0.75rem;
    }

    .section-title {
      font-size: 1.05rem;
      font-weight: 700;
      display: flex;
      align-items: center;
      gap: 0.5rem;
      color: #f3f4f6;
    }

    /* Facility Grid */
    .facility-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
      gap: 1rem;
    }

    .facility-card {
      background-color: var(--bg-card);
      border: 1px solid var(--border-color);
      border-radius: var(--radius);
      padding: 1.1rem;
      display: flex;
      flex-direction: column;
      gap: 0.75rem;
      box-shadow: var(--shadow);
      transition: transform 0.15s ease, border-color 0.15s ease;
    }

    .facility-card:hover {
      border-color: var(--border-light);
      transform: translateY(-2px);
    }

    .facility-header {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      gap: 0.5rem;
    }

    .facility-name {
      font-size: 1rem;
      font-weight: 700;
      color: white;
    }

    .facility-id {
      font-size: 0.75rem;
      font-family: ui-monospace, monospace;
      color: #94a3b8;
    }

    .status-badge {
      font-size: 0.72rem;
      font-weight: 700;
      padding: 0.25rem 0.6rem;
      border-radius: 9999px;
      text-transform: uppercase;
      letter-spacing: 0.05em;
    }

    .status-Available    { background-color: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid rgba(16, 185, 129, 0.4); }
    .status-Reserved     { background-color: rgba(59, 130, 246, 0.2); color: #60a5fa; border: 1px solid rgba(59, 130, 246, 0.4); }
    .status-Setup        { background-color: rgba(245, 158, 11, 0.2); color: #fbbf24; border: 1px solid rgba(245, 158, 11, 0.4); }
    .status-Occupied     { background-color: rgba(139, 92, 246, 0.2); color: #c084fc; border: 1px solid rgba(139, 92, 246, 0.4); }
    .status-Turnover     { background-color: rgba(6, 182, 212, 0.2); color: #22d3ee; border: 1px solid rgba(6, 182, 212, 0.4); }
    .status-OutOfService { background-color: rgba(244, 63, 94, 0.2); color: #fb7185; border: 1px solid rgba(244, 63, 94, 0.4); }
    .status-EmergencyHold{ background-color: rgba(220, 38, 38, 0.35); color: #fca5a5; border: 1px solid #ef4444; animation: pulse 1.2s infinite; }

    .door-indicator {
      display: inline-flex;
      align-items: center;
      gap: 0.35rem;
      font-size: 0.78rem;
      font-weight: 600;
    }

    .door-locked { color: #f87171; }
    .door-unlocked { color: #34d399; }

    .facility-meta {
      display: flex;
      flex-wrap: wrap;
      gap: 0.5rem 1rem;
      font-size: 0.8rem;
      color: var(--text-muted);
    }

    .facility-actions {
      display: flex;
      flex-wrap: wrap;
      gap: 0.4rem;
      margin-top: 0.25rem;
      padding-top: 0.5rem;
      border-top: 1px solid rgba(255, 255, 255, 0.08);
    }

    /* Kanban Board */
    .kanban-wrapper {
      overflow-x: auto;
      padding-bottom: 0.75rem;
    }

    .kanban-board {
      display: grid;
      grid-template-columns: repeat(11, minmax(240px, 1fr));
      gap: 0.75rem;
      min-width: 2700px;
    }

    .kanban-column {
      background-color: var(--bg-secondary);
      border: 1px solid var(--border-color);
      border-radius: var(--radius);
      display: flex;
      flex-direction: column;
      max-height: 520px;
    }

    .column-header {
      padding: 0.6rem 0.8rem;
      background-color: rgba(255, 255, 255, 0.03);
      border-bottom: 1px solid var(--border-color);
      font-size: 0.82rem;
      font-weight: 700;
      display: flex;
      justify-content: space-between;
      align-items: center;
      position: sticky;
      top: 0;
    }

    .column-count {
      background-color: rgba(255, 255, 255, 0.1);
      padding: 0.15rem 0.45rem;
      border-radius: 9999px;
      font-size: 0.72rem;
      color: #94a3b8;
    }

    .column-cards {
      padding: 0.6rem;
      display: flex;
      flex-direction: column;
      gap: 0.6rem;
      overflow-y: auto;
      flex: 1;
    }

    .kanban-card {
      background-color: var(--bg-card);
      border: 1px solid var(--border-color);
      border-radius: 6px;
      padding: 0.75rem;
      display: flex;
      flex-direction: column;
      gap: 0.4rem;
      font-size: 0.8rem;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
      transition: border-color 0.15s ease;
    }

    .kanban-card:hover {
      border-color: var(--border-light);
    }

    .card-id {
      font-family: ui-monospace, monospace;
      font-size: 0.75rem;
      font-weight: 700;
      color: #38bdf8;
    }

    .card-title {
      font-weight: 600;
      color: white;
      font-size: 0.88rem;
    }

    .card-detail {
      font-size: 0.75rem;
      color: var(--text-muted);
    }

    .card-alert {
      display: inline-block;
      font-size: 0.68rem;
      font-weight: 700;
      color: #fbbf24;
      background: rgba(245, 158, 11, 0.15);
      border: 1px solid rgba(245, 158, 11, 0.3);
      border-radius: 4px;
      padding: 0.15rem 0.4rem;
      margin-top: 0.2rem;
    }

    .card-actions {
      display: flex;
      flex-wrap: wrap;
      gap: 0.35rem;
      margin-top: 0.4rem;
      padding-top: 0.4rem;
      border-top: 1px solid rgba(255, 255, 255, 0.06);
    }

    /* Audit Log Stream */
    .log-panel {
      background-color: #0b0f19;
      border: 1px solid var(--border-color);
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      display: flex;
      flex-direction: column;
      height: 240px;
    }

    .log-header {
      background-color: #111827;
      border-bottom: 1px solid var(--border-color);
      padding: 0.5rem 1rem;
      font-size: 0.8rem;
      font-weight: 700;
      color: #94a3b8;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .log-stream {
      padding: 0.75rem;
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      font-size: 0.78rem;
      color: #e2e8f0;
      overflow-y: auto;
      flex: 1;
      display: flex;
      flex-direction: column;
      gap: 0.35rem;
    }

    .log-entry {
      line-height: 1.4;
      border-left: 2px solid #3b82f6;
      padding-left: 0.5rem;
    }

    .log-entry.danger {
      border-left-color: #ef4444;
      color: #fca5a5;
    }

    .log-entry.success {
      border-left-color: #10b981;
      color: #86efac;
    }

    /* Modal */
    .modal-backdrop {
      position: fixed;
      inset: 0;
      background: rgba(0, 0, 0, 0.75);
      backdrop-filter: blur(4px);
      display: none;
      align-items: center;
      justify-content: center;
      z-index: 1000;
      padding: 1rem;
    }

    .modal-backdrop.active {
      display: flex;
    }

    .modal-window {
      background-color: var(--bg-secondary);
      border: 1px solid var(--border-light);
      border-radius: 10px;
      width: 100%;
      max-width: 550px;
      box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.5);
      display: flex;
      flex-direction: column;
      overflow: hidden;
    }

    .modal-header {
      padding: 1rem 1.25rem;
      background-color: rgba(255, 255, 255, 0.03);
      border-bottom: 1px solid var(--border-color);
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .modal-body {
      padding: 1.25rem;
      display: flex;
      flex-direction: column;
      gap: 0.85rem;
    }

    .form-group {
      display: flex;
      flex-direction: column;
      gap: 0.35rem;
    }

    .form-group label {
      font-size: 0.8rem;
      font-weight: 600;
      color: #cbd5e1;
    }

    .form-control {
      background-color: var(--bg-card);
      border: 1px solid var(--border-color);
      border-radius: 6px;
      padding: 0.5rem 0.75rem;
      color: white;
      font-size: 0.85rem;
      outline: none;
      transition: border-color 0.15s ease;
    }

    .form-control:focus {
      border-color: var(--accent-blue);
    }

    .modal-footer {
      padding: 0.75rem 1.25rem;
      background-color: rgba(255, 255, 255, 0.02);
      border-top: 1px solid var(--border-color);
      display: flex;
      justify-content: flex-end;
      gap: 0.5rem;
    }
  </style>
</head>
<body>

  <!-- Top Navigation Bar -->
  <header>
    <div class="brand">
      <div class="brand-icon">🏛️</div>
      <div>
        <h1>SmartCampus</h1>
        <div style="font-size: 0.72rem; color: #94a3b8;">Event &amp; Facility Booking Hub</div>
      </div>
      <div class="live-badge">
        <span class="pulse-dot"></span>
        <span id="pollStatus">LIVE (1s poll)</span>
      </div>
    </div>

    <div class="header-controls">
      <!-- Simulation Clock -->
      <div class="clock-display" title="Simulated Campus Clock">
        <span>🕒</span>
        <span id="clockValue">2026-09-01 09:00:00</span>
      </div>

      <!-- Time Advancement Controls -->
      <button class="btn btn-secondary btn-sm" onclick="advanceTime(900)">+15m</button>
      <button class="btn btn-secondary btn-sm" onclick="advanceTime(3600)">+1h</button>
      <button class="btn btn-secondary btn-sm" onclick="advanceTime(86400)">+24h</button>
      <button class="btn btn-secondary btn-sm" onclick="resetSimulation()" title="Reset simulation clock and default seed data">↺ Reset</button>

      <!-- Global Campus Emergency Controls -->
      <button class="btn btn-danger btn-sm" onclick="declareEmergency()">🚨 Declare Emergency</button>
      <button class="btn btn-success btn-sm" onclick="clearEmergency()">🛡️ All Clear</button>

      <!-- New Booking Action -->
      <button class="btn btn-primary" onclick="openBookingModal()">+ New Reservation</button>
    </div>
  </header>

  <!-- Scenario Execution Strip -->
  <div class="scenario-strip">
    <span class="scenario-title">⚡ Interactive Scenarios:</span>
    <button class="btn btn-secondary btn-sm" onclick="runScenario(1)">1. Happy Path</button>
    <button class="btn btn-secondary btn-sm" onclick="runScenario(2)">2. Security Gate</button>
    <button class="btn btn-secondary btn-sm" onclick="runScenario(3)">3. Revision Loop</button>
    <button class="btn btn-secondary btn-sm" onclick="runScenario(4)">4. Facility Staging</button>
    <button class="btn btn-secondary btn-sm" onclick="runScenario(5)">5. Emergency Preempt</button>
    <button class="btn btn-secondary btn-sm" onclick="runScenario(6)">6. 24h Cancel Guard</button>
    <button class="btn btn-primary btn-sm" onclick="runScenario('all')">▶ Run All (1-6)</button>
  </div>

  <!-- Main View -->
  <main>
    <!-- Section 1: Facilities -->
    <section>
      <div class="section-header">
        <h2 class="section-title">
          <span>🏢</span> Campus Facilities Directory &amp; Real-Time Space Status
        </h2>
        <span id="facilityCount" style="font-size: 0.8rem; color: #94a3b8;">4 Facilities Active</span>
      </div>
      <div class="facility-grid" id="facilityGrid">
        <!-- Rendered dynamically via fetchState -->
      </div>
    </section>

    <!-- Section 2: Reservation Kanban Pipeline -->
    <section>
      <div class="section-header">
        <h2 class="section-title">
          <span>📋</span> Reservation Lifecycle Pipeline (11 Formal States)
        </h2>
        <span id="reservationCount" style="font-size: 0.8rem; color: #94a3b8;">Loading...</span>
      </div>
      <div class="kanban-wrapper">
        <div class="kanban-board" id="kanbanBoard">
          <!-- 11 columns populated dynamically -->
        </div>
      </div>
    </section>

    <!-- Section 3: Live Audit Log -->
    <section>
      <div class="log-panel">
        <div class="log-header">
          <span>📡 REAL-TIME SYSTEM AUDIT LOG &amp; ACCESS DISPATCH STREAM</span>
          <span style="font-size: 0.72rem; color: #64748b;">Auto-scrolling</span>
        </div>
        <div class="log-stream" id="logStream">
          <!-- Log lines populated dynamically -->
        </div>
      </div>
    </section>
  </main>

  <!-- Modal: New Booking -->
  <div class="modal-backdrop" id="bookingModal">
    <div class="modal-window">
      <div class="modal-header">
        <h3 style="font-size: 1rem; font-weight: 700;">Create &amp; Submit Campus Reservation</h3>
        <button class="btn btn-secondary btn-sm" onclick="closeBookingModal()">✕</button>
      </div>
      <div class="modal-body">
        <div class="form-group">
          <label>Event Title</label>
          <input type="text" id="bmTitle" class="form-control" placeholder="e.g. AI Robotics Showcase">
        </div>
        <div class="form-group">
          <label>Event Type</label>
          <input type="text" id="bmType" class="form-control" placeholder="e.g. Workshop, Conference, Assembly" value="Workshop">
        </div>
        <div class="form-group">
          <label>Campus Facility</label>
          <select id="bmFacility" class="form-control">
            <option value="FAC-101">FAC-101: Centennial Auditorium (Cap 250)</option>
            <option value="FAC-102">FAC-102: Science Lecture Hall (Cap 120)</option>
            <option value="FAC-103">FAC-103: Student Union Ballroom (Cap 300)</option>
            <option value="FAC-104">FAC-104: Seminar Room (Cap 40)</option>
          </select>
        </div>
        <div class="form-group">
          <label>Requester Account</label>
          <select id="bmRequester" class="form-control">
            <option value="USR-001">USR-001: Alice Chen (Student Org Officer)</option>
            <option value="USR-002">USR-002: Bob Martinez (Student Org Officer)</option>
            <option value="USR-003">USR-003: Dr. Clara Hughes (Faculty / Staff)</option>
            <option value="USR-004">USR-004: Dan Evans (Facility Coordinator)</option>
          </select>
        </div>
        <div class="form-group">
          <label>Expected Attendance</label>
          <input type="number" id="bmAttendance" class="form-control" value="25" min="1" max="1000">
          <span style="font-size: 0.72rem; color: #94a3b8;">* Attendance &gt; 100 automatically triggers the conditional Security Review gate.</span>
        </div>
        <div class="form-group">
          <label>Start In (Hours from simulated clock)</label>
          <input type="number" id="bmStartOffset" class="form-control" value="24" min="1">
        </div>
        <div class="form-group">
          <label>Duration (Hours)</label>
          <input type="number" id="bmDuration" class="form-control" value="2" min="1" max="24">
        </div>
      </div>
      <div class="modal-footer">
        <button class="btn btn-secondary" onclick="closeBookingModal()">Cancel</button>
        <button class="btn btn-primary" onclick="submitBookingForm()">Submit &amp; Soft-Lock Slot</button>
      </div>
    </div>
  </div>

  <!-- Modal: Scenario Output -->
  <div class="modal-backdrop" id="scenarioModal">
    <div class="modal-window" style="max-width: 720px;">
      <div class="modal-header">
        <h3 id="scenarioModalTitle" style="font-size: 1rem; font-weight: 700;">Scenario Output</h3>
        <button class="btn btn-secondary btn-sm" onclick="closeScenarioModal()">✕</button>
      </div>
      <div class="modal-body">
        <pre id="scenarioModalText" style="font-family: ui-monospace, monospace; font-size: 0.78rem; background: #0b0f19; padding: 1rem; border-radius: 6px; border: 1px solid var(--border-color); color: #38bdf8; max-height: 400px; overflow-y: auto; white-space: pre-wrap;"></pre>
      </div>
      <div class="modal-footer">
        <button class="btn btn-primary" onclick="closeScenarioModal()">Close</button>
      </div>
    </div>
  </div>

  <script>
    const STAGES = [
      'Draft', 'Submitted', 'UnderReview', 'SecurityReview', 'NeedsRevision',
      'Approved', 'InProgress', 'Completed', 'Rejected', 'Cancelled', 'Expired'
    ];

    let lastState = null;

    async function apiPost(endpoint, data = {}) {
      try {
        const res = await fetch(endpoint, {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(data)
        });
        const json = await res.json();
        await fetchState();
        return json;
      } catch (err) {
        console.error('API Error:', endpoint, err);
      }
    }

    async function fetchState() {
      try {
        const res = await fetch('/api/state');
        if (!res.ok) throw new Error('Network error');
        const data = await res.json();
        lastState = data;

        document.getElementById('pollStatus').textContent = 'LIVE (1s poll)';
        document.getElementById('pollStatus').parentElement.style.color = '#10b981';

        // Update clock
        if (data.formattedTime) {
          document.getElementById('clockValue').textContent = data.formattedTime;
        }

        renderFacilities(data.facilities || []);
        renderKanban(data.reservations || []);
        renderAuditLog(data.auditLog || []);
      } catch (err) {
        document.getElementById('pollStatus').textContent = 'OFFLINE (reconnecting)';
        document.getElementById('pollStatus').parentElement.style.color = '#ef4444';
      }
    }

    function renderFacilities(facilities) {
      const grid = document.getElementById('facilityGrid');
      document.getElementById('facilityCount').textContent = facilities.length + ' Facilities Active';

      grid.innerHTML = facilities.map(fac => {
        const doorClass = fac.doorsLocked ? 'door-locked' : 'door-unlocked';
        const doorText = fac.doorsLocked ? '🔒 DOORS LOCKED' : '🔓 DOORS UNLOCKED';

        return `
          <div class="facility-card">
            <div class="facility-header">
              <div>
                <div class="facility-name">${escapeHtml(fac.name)}</div>
                <div class="facility-id">${fac.id} &bull; ${escapeHtml(fac.building)}</div>
              </div>
              <span class="status-badge status-${fac.status}">${fac.status}</span>
            </div>

            <div class="facility-meta">
              <span>👥 Cap: ${fac.capacity}</span>
              <span>💰 $${fac.hourlyRate}/hr</span>
              <span class="door-indicator ${doorClass}">${doorText}</span>
            </div>

            <div style="font-size: 0.74rem; color: #94a3b8;">
              <strong>Supported:</strong> ${(fac.equipment || []).join(', ') || 'Standard Class Setup'}
            </div>

            <div class="facility-actions">
              <button class="btn btn-secondary btn-sm" onclick="reportDamage('${fac.id}')">Report Damage</button>
              <button class="btn btn-secondary btn-sm" onclick="certifyRepair('${fac.id}')">Certify Repair</button>
            </div>
          </div>
        `;
      }).join('');
    }

    function renderKanban(reservations) {
      const board = document.getElementById('kanbanBoard');
      document.getElementById('reservationCount').textContent = reservations.length + ' Total Reservations';

      // Group by stage
      const groups = {};
      STAGES.forEach(s => groups[s] = []);
      reservations.forEach(r => {
        if (groups[r.status]) {
          groups[r.status].push(r);
        } else {
          groups['Draft'].push(r);
        }
      });

      board.innerHTML = STAGES.map(stage => {
        const cards = groups[stage] || [];
        return `
          <div class="kanban-column">
            <div class="column-header">
              <span>${stage}</span>
              <span class="column-count">${cards.length}</span>
            </div>
            <div class="column-cards">
              ${cards.map(renderCard).join('')}
            </div>
          </div>
        `;
      }).join('');
    }

    function renderCard(r) {
      let actionsHtml = '';

      if (r.status === 'Draft') {
        actionsHtml = `<button class="btn btn-primary btn-sm" onclick="routeReview('${r.id}')">Submit</button>`;
      } else if (r.status === 'Submitted') {
        actionsHtml = `<button class="btn btn-primary btn-sm" onclick="routeReview('${r.id}')">Route to Review</button>`;
      } else if (r.status === 'UnderReview') {
        actionsHtml = `
          <button class="btn btn-success btn-sm" onclick="approveRes('${r.id}')">Approve</button>
          <button class="btn btn-danger btn-sm" onclick="rejectRes('${r.id}')">Reject</button>
          <button class="btn btn-warning btn-sm" onclick="requestChanges('${r.id}')">Changes</button>
        `;
      } else if (r.status === 'SecurityReview') {
        actionsHtml = `
          <button class="btn btn-success btn-sm" onclick="securityClear('${r.id}')">Clear Security</button>
          <button class="btn btn-danger btn-sm" onclick="securityDeny('${r.id}')">Deny</button>
        `;
      } else if (r.status === 'NeedsRevision') {
        actionsHtml = `
          <button class="btn btn-primary btn-sm" onclick="resubmitRes('${r.id}')">Resubmit</button>
          <button class="btn btn-danger btn-sm" onclick="cancelRes('${r.id}')">Cancel</button>
        `;
      } else if (r.status === 'Approved') {
        actionsHtml = `
          <button class="btn btn-primary btn-sm" onclick="checkInRes('${r.id}')">Check In</button>
          <button class="btn btn-danger btn-sm" onclick="cancelRes('${r.id}')">Cancel (24h)</button>
        `;
      } else if (r.status === 'InProgress') {
        actionsHtml = `<button class="btn btn-success btn-sm" onclick="concludeRes('${r.id}')">Conclude Event</button>`;
      }

      const alertHtml = (r.requiresSecurity || r.attendance > 100)
        ? `<div class="card-alert">⚠️ Security Gate (&gt;100 attendees)</div>`
        : '';

      return `
        <div class="kanban-card">
          <div style="display:flex; justify-content:space-between; align-items:center;">
            <span class="card-id">${r.id}</span>
            <span style="font-size:0.72rem; color:#94a3b8;">${r.facilityId}</span>
          </div>
          <div class="card-title">${escapeHtml(r.title)}</div>
          <div class="card-detail">👤 ${r.requesterId} &bull; 👥 ${r.attendance} guests</div>
          <div class="card-detail">🕒 ${r.formattedStart || ''}</div>
          ${alertHtml}
          ${actionsHtml ? `<div class="card-actions">${actionsHtml}</div>` : ''}
        </div>
      `;
    }

    function renderAuditLog(logs) {
      const stream = document.getElementById('logStream');
      const atBottom = stream.scrollHeight - stream.scrollTop <= stream.clientHeight + 50;

      stream.innerHTML = logs.map(line => {
        let cls = 'log-entry';
        if (line.includes('EMERGENCY') || line.includes('rejected') || line.includes('denied') || line.includes('OutOfService')) {
          cls += ' danger';
        } else if (line.includes('approved') || line.includes('cleared') || line.includes('Available') || line.includes('PASS')) {
          cls += ' success';
        }
        return `<div class="${cls}">${escapeHtml(line)}</div>`;
      }).join('');

      if (atBottom) {
        stream.scrollTop = stream.scrollHeight;
      }
    }

    function escapeHtml(str) {
      if (!str) return '';
      return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
    }

    /* Actions */
    function advanceTime(seconds) {
      apiPost('/api/advance-time', { seconds });
    }

    function resetSimulation() {
      if (confirm('Reset simulation clock and revert facilities to initial defaults?')) {
        apiPost('/api/reset');
      }
    }

    function declareEmergency() {
      const reason = prompt('Enter Emergency Declaration Reason:', 'Campus Safety Severe Threat / Active Lockdown');
      if (reason) {
        apiPost('/api/emergency', { reason });
      }
    }

    function clearEmergency() {
      apiPost('/api/all-clear');
    }

    function reportDamage(facilityId) {
      const issue = prompt(`Report damage / blackout for ${facilityId}:`, 'HVAC unit failure and water leak');
      if (issue) {
        apiPost('/api/report-damage', { facilityId, issue });
      }
    }

    function certifyRepair(facilityId) {
      apiPost('/api/certify-repair', { facilityId });
    }

    function routeReview(id) {
      apiPost('/api/route-review', { id });
    }

    function approveRes(id) {
      apiPost('/api/approve', { id, approverId: 'USR-005', comments: 'Approved via Web Dashboard' });
    }

    function rejectRes(id) {
      const reason = prompt('Rejection justification:', 'Facility capacity exceeded / conflicting booking');
      if (reason) {
        apiPost('/api/reject', { id, approverId: 'USR-005', reason });
      }
    }

    function requestChanges(id) {
      const notes = prompt('Enter required adjustments:', 'Please submit floor plan with 36-inch fire aisle clearance');
      if (notes) {
        apiPost('/api/request-changes', { id, approverId: 'USR-005', notes });
      }
    }

    function resubmitRes(id) {
      apiPost('/api/resubmit', { id });
    }

    function securityClear(id) {
      apiPost('/api/security-clear', { id, officerId: 'USR-006', notes: 'Security plan approved; two officers assigned.' });
    }

    function securityDeny(id) {
      const reason = prompt('Security denial reason:', 'Insufficient crowd control plan for after-hours event');
      if (reason) {
        apiPost('/api/security-deny', { id, officerId: 'USR-006', reason });
      }
    }

    function checkInRes(id) {
      apiPost('/api/check-in', { id });
    }

    function concludeRes(id) {
      apiPost('/api/conclude', { id });
    }

    function cancelRes(id) {
      if (confirm('Cancel reservation? (Note: Approved reservations require >= 24h lead time)')) {
        apiPost('/api/cancel', { id, userId: 'USR-001' });
      }
    }

    async function runScenario(num) {
      const res = await apiPost('/api/scenario', { scenario: num });
      if (res) {
        document.getElementById('scenarioModalTitle').textContent = `Scenario ${num} Result: ${res.success ? 'PASSED' : 'FAILED'}`;
        document.getElementById('scenarioModalText').textContent = res.output || res.message || 'Execution completed.';
        document.getElementById('scenarioModal').classList.add('active');
      }
    }

    function closeScenarioModal() {
      document.getElementById('scenarioModal').classList.remove('active');
    }

    /* Booking Modal */
    function openBookingModal() {
      document.getElementById('bookingModal').classList.add('active');
    }

    function closeBookingModal() {
      document.getElementById('bookingModal').classList.remove('active');
    }

    function submitBookingForm() {
      const title = document.getElementById('bmTitle').value.trim() || 'New Campus Event';
      const eventType = document.getElementById('bmType').value.trim() || 'Meeting';
      const facilityId = document.getElementById('bmFacility').value;
      const requesterId = document.getElementById('bmRequester').value;
      const attendance = parseInt(document.getElementById('bmAttendance').value, 10) || 25;
      const startOffset = parseInt(document.getElementById('bmStartOffset').value, 10) || 24;
      const duration = parseInt(document.getElementById('bmDuration').value, 10) || 2;

      const baseTime = (lastState && lastState.currentTime) ? lastState.currentTime : Math.floor(Date.now() / 1000);
      const startTime = baseTime + (startOffset * 3600);
      const endTime = startTime + (duration * 3600);

      apiPost('/api/book', {
        title,
        eventType,
        facilityId,
        requesterId,
        attendance,
        startTime,
        endTime
      });

      closeBookingModal();
    }

    // Auto-poll every 1000ms
    fetchState();
    setInterval(fetchState, 1000);
  </script>
</body>
</html>
)rawhtml";

} // namespace smartcampus::web
