# Legacy Web Stack Documentation Archive

**Archived Date**: January 18, 2026
**Migration**: Full-stack → CLI-only
**Backup Tag**: `pre-cli-only-migration`

---

## Overview

This directory contains documentation from the legacy **full-stack web application** version of IGSOA-SIM, archived during the CLI-only migration. These documents describe the previous architecture that included:

- React 18 Command Center web UI
- Node.js Express/WebSocket backend server
- Python web bridge components
- REST API and WebSocket API layers
- Multi-tier deployment architecture

---

## Archive Contents

### `/api` - API Reference
- `API_REFERENCE.md` - REST and WebSocket API documentation for the web backend

### `/architecture` - Architecture Documentation
- `PROJECT_STRUCTURE.md` - Full-stack project structure
- `STRUCTURAL_ANALYSIS.md` - Multi-tier architecture analysis
- `CODEBASE_STRUCTURE.md` - Complete codebase organization

### `/components` - Component Documentation
- `backend/` - Node.js backend server documentation
  - `README.md` - Backend component overview
  - `SERVER_SECURITY_FIXES.md` - Security fixes for web server

### `/command-center` - Web UI Documentation
- `glossary/` - Command Center terminology
- Complete Command Center specifications (archived separately)

### `/getting-started` - Legacy Setup Guides
- `INSTRUCTIONS.md` - Full-stack setup instructions (web + backend + CLI)
- `QUICK_REFERENCE.md` - Legacy quick reference with web UI commands

### `/implementation` - Implementation Reports
- `INTEGRATION_STATUS.md` - Web integration status
- `SESSION_SUMMARY.md` - Web development session summaries
- `GOOGLE_COLAB_TRAINING_PLAN.md` - Colab integration plan

### `/reviews` - Code Reviews
- `2025-03-Stage1-Command-Center.md` - Command Center implementation review
- `2025-03-bridge-backend-code-review.md` - Backend bridge code review
- `stage2-playground-review.md` - Playground feature review

### `/roadmap` - Legacy Roadmaps
- `POLISH_AND_UX_ROADMAP.md` - Web UI/UX enhancement roadmap

### Root Files
- `README_legacy.md` - Original project README describing full-stack architecture

---

## Why Archived?

These documents became obsolete when the project migrated from a full-stack web application to a streamlined CLI-only architecture. They are preserved here for:

1. **Historical Reference**: Understanding the evolution of the project
2. **Rollback Support**: If reverting to the web stack is needed
3. **Architecture Lessons**: Learning from the full-stack design decisions
4. **Component Reuse**: Reference for re-implementing web features if needed

---

## Restoration

To restore the full-stack version with all web components:

```bash
git checkout pre-cli-only-migration
```

This will restore:
- Complete React Command Center UI (`web/`)
- Node.js backend server (`backend/server.js`, `backend/api/`)
- Python web bridges (`src/python/bridge_server*.py`)
- All legacy documentation (moved back from archive)

---

## Current Architecture

The current **CLI-only architecture** is documented in:

- `README.md` (project root) - CLI-focused overview
- `CLI_USAGE_GUIDE.md` - Complete CLI reference
- `CLI_MIGRATION_SUMMARY.md` - Migration report
- `docs/implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md` - CLI design

---

## Notes

- These documents may reference components that no longer exist in the current codebase
- File paths and directory structures described may not match the current CLI-only layout
- Web-specific features (WebSocket, REST API, React components) are not available in CLI-only mode
- The CLI retains all **computational engine capabilities** - only the web interface was removed

---

**Migration Details**: See `/CLI_MIGRATION_SUMMARY.md` in project root for complete migration report.
