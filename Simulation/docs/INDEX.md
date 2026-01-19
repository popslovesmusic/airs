# IGSOA-SIM Documentation Index

**Last Updated**: 2026-01-18 (CLI Migration)
**Architecture**: CLI-Only (migrated from full-stack)
**Active Documents**: ~120 files | **Archived**: ~40 files (legacy web stack)

---

## 🚨 Important: CLI-Only Migration

**IGSOA-SIM has been migrated to CLI-only architecture** (January 2026).

- ✅ **Current**: CLI-based simulation framework with JSON API
- ❌ **Removed**: React web UI, Node.js backend, WebSocket API
- 📦 **Archived**: Legacy web stack documentation → `archive/legacy-web-stack/`

**Key Documentation**:
- [CLI Usage Guide](../CLI_USAGE_GUIDE.md) - Complete CLI reference
- [Migration Summary](../CLI_MIGRATION_SUMMARY.md) - What changed and why
- [Legacy Archive](archive/legacy-web-stack/README.md) - Old web stack docs

---

## Quick Start

**New to CLI-only IGSOA-SIM?** Start here:
1. [README.md](../README.md) - Project overview and build instructions
2. [CLI Usage Guide](../CLI_USAGE_GUIDE.md) - Complete command reference
3. [Headless CLI Architecture](implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md) - CLI design

---

## Documentation Categories

### 📚 Getting Started (docs/getting-started/)
User-facing guides for onboarding and quick reference.

**Key Documents**:
- `INSTRUCTIONS.md` - Main getting started guide
- `QUICK_REFERENCE.md` - Quick reference for common tasks
- `QUICK_START_NEXT_SESSION.md` - Resume work quickly

### 🏗️ Architecture & Design (docs/architecture-design/)
System architecture, design patterns, and structural analysis.

**Key Documents**:
- `STRUCTURAL_ANALYSIS.md` - Comprehensive project structural analysis
- `PROJECT_ORGANIZATION.md` - Project organization and conventions
- `AGENTS.md` - Agent system architecture

### 🔧 Implementation (docs/implementation/)
Implementation reports, session summaries, and phase completions.

**Key Documents**:
- `CACHE_PHASE_C_COMPLETE.md` - Phase C cache system report
- `GOVERNANCE_FEATURE_COMPLETE.md` - Feature 10 governance agent
- `FEATURE_IMPLEMENTATION_STATUS_UPDATED.md` - Overall feature status
- `SESSION_*_SUMMARY.md` - Session summaries
- `WEEK*_*.md` - Weekly progress reports
- `PHASE_3_COMPLETE.md` - Phase 3 completion report
- `ERROR_HANDLING_*.md` - Error handling improvements
- `INTEGRATION_STATUS.md` - Integration status

### 📊 Reports & Analysis (docs/reports-analysis/)
Analysis reports, benchmarks, and audit results.

**Key Documents**:
- `PROJECT_METRICS_SUMMARY.md` - Quick metrics overview
- `STATIC_ANALYSIS_*.md` - Code quality analysis
- `ERROR_HANDLING_AUDIT.md` - Error handling audit
- `ERROR_HANDLING_VERIFICATION.md` - Verification results
- `RELEASE_PREP_SUMMARY.md` - Release preparation summary

### 🗺️ Roadmap (docs/roadmap/)
Project roadmap, planning documents, and remaining work.

**Key Documents**:
- `DO_NOT_FORGET.md` - **Remaining features list (3 of 12)**
- `INTEGRATION_PLAN.md` - Integration roadmap

### 📝 API Reference (docs/api-reference/)
JSON CLI API documentation.

**Note**: REST/WebSocket API docs archived → `archive/legacy-web-stack/api/`
**Current**: CLI uses JSON-based API (see [CLI Usage Guide](../CLI_USAGE_GUIDE.md))

### 🔍 Reviews (docs/reviews/)
Code reviews and checkpoint documents.

**Key Documents**:
- `REVIEW_SAVEPOINT.md` - Review savepoint
- `cmb-imprint-analysis/` - CMB analysis reviews
- `echo-searches/` - Echo search reviews
- Code review documents

**Note**: Web-related reviews archived → `archive/legacy-web-stack/reviews/`

### 🐛 Issues & Fixes (docs/issues-fixes/)
Issue tracking and fix documentation.

**Key Documents**:
- `ERROR_HANDLING_IMPROVEMENT_PLAN.md` - Error handling improvements

### 🧪 Testing (docs/testing/)
Test documentation and test plans.

**Contents**: Test documentation files

### 🔒 Security (docs/security/)
Security reviews and audits.

**Contents**: Security documentation files

### 📖 Guides & Manuals (docs/guides-manuals/)
User and developer manuals.

**Contents**: User guides and manuals

### 🧩 Components (docs/components/)
Component-specific documentation.

**Structure**:
- `cli/` - CLI tool documentation

**Note**: Web UI and backend component docs archived → `archive/legacy-web-stack/`

### 📦 Archive (docs/archive/)
Historical documentation and legacy components.

**Structure**:
- `legacy-web-stack/` - **Full-stack web application documentation** (archived 2026-01-18)
  - React Command Center UI docs
  - Node.js backend server docs
  - REST/WebSocket API documentation
  - Web-specific architecture and implementation docs
  - See [Archive README](archive/legacy-web-stack/README.md) for details
- Other archived documents (refactoring reports, old analyses, etc.)

---

## Documentation by Purpose

### For New Users (CLI)
1. [README.md](../README.md) - Project overview
2. [CLI Usage Guide](../CLI_USAGE_GUIDE.md) - Complete CLI reference
3. [Headless CLI Architecture](implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md) - CLI design
4. [Quick Start](getting-started/QUICK_START_NEXT_SESSION.md)

### For Developers
1. [Project Organization](architecture-design/PROJECT_ORGANIZATION.md)
2. [File Organization](architecture-design/FILE_ORGANIZATION.md)
3. [Component Docs](components/)
4. [CLI Architecture](implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md)

### For Project Managers
1. [Feature Status](implementation/FEATURE_IMPLEMENTATION_STATUS_UPDATED.md)
2. [Remaining Work](roadmap/DO_NOT_FORGET.md)
3. [Project Metrics](reports-analysis/PROJECT_METRICS_SUMMARY.md)
4. [Migration Summary](../CLI_MIGRATION_SUMMARY.md) - CLI migration report

### For Contributors
1. [Integration Plan](roadmap/INTEGRATION_PLAN.md)
2. [Architecture Design](architecture-design/)
3. [Testing Docs](testing/)
4. [Issue Tracking](issues-fixes/)

---

## Project Status at a Glance

**Current Status** (as of 2026-01-18):
- **Architecture**: ✅ CLI-Only (migrated from full-stack)
- **CLI Build**: ✅ Verified and tested
- **Code Reduction**: 39% smaller (removed ~10K LOC web stack)
- **Build Time**: 60% faster (2 min vs 5 min)
- **Production Status**: ✅ CLI production-ready
- **Documentation**: ~120 active files, ~40 archived (legacy web)

**CLI Status**:
- ✅ JSON-based headless interface
- ✅ All computational engines functional
- ✅ FFT analysis built-in
- ✅ Batch processing support
- ✅ Python cache utilities (optional)

See [CLI Migration Summary](../CLI_MIGRATION_SUMMARY.md) for complete details.

---

## Documentation by Feature

### Cache System (Features 1-7, 10-11)
- Phase completion: [CACHE_PHASE_C_COMPLETE.md](implementation/CACHE_PHASE_C_COMPLETE.md)
- Governance: [GOVERNANCE_FEATURE_COMPLETE.md](implementation/GOVERNANCE_FEATURE_COMPLETE.md)
- Status: [FEATURE_IMPLEMENTATION_STATUS_UPDATED.md](implementation/FEATURE_IMPLEMENTATION_STATUS_UPDATED.md)

### CLI Architecture
- Design: [HEADLESS_JSON_CLI_ARCHITECTURE.md](implementation/HEADLESS_JSON_CLI_ARCHITECTURE.md)
- Usage: [CLI_USAGE_GUIDE.md](../CLI_USAGE_GUIDE.md)
- Migration: [CLI_MIGRATION_SUMMARY.md](../CLI_MIGRATION_SUMMARY.md)

### Error Handling
- Audit: [ERROR_HANDLING_AUDIT.md](reports-analysis/ERROR_HANDLING_AUDIT.md)
- Implementation: [ERROR_HANDLING_IMPLEMENTATION_PROGRESS.md](implementation/ERROR_HANDLING_IMPLEMENTATION_PROGRESS.md)
- Final Summary: [ERROR_HANDLING_FINAL_SUMMARY.md](implementation/ERROR_HANDLING_FINAL_SUMMARY.md)
- Improvement Plan: [ERROR_HANDLING_IMPROVEMENT_PLAN.md](issues-fixes/ERROR_HANDLING_IMPROVEMENT_PLAN.md)
- Verification: [ERROR_HANDLING_VERIFICATION.md](reports-analysis/ERROR_HANDLING_VERIFICATION.md)

### Integration
- Plan: [INTEGRATION_PLAN.md](roadmap/INTEGRATION_PLAN.md)

### Legacy Web Stack (Archived)
- Archive: [legacy-web-stack/](archive/legacy-web-stack/)
- See [Archive README](archive/legacy-web-stack/README.md) for full-stack documentation

---

## File Naming Conventions

### Prefixes
- `PHASE_*` - Phase completion reports
- `SESSION_*` - Session summaries
- `WEEK*_*` - Weekly progress reports
- `ERROR_HANDLING_*` - Error handling related docs

### Suffixes
- `_COMPLETE.md` - Completion reports
- `_STATUS.md` - Status documents
- `_SUMMARY.md` - Summary reports
- `_PLAN.md` - Planning documents
- `_AUDIT.md` - Audit reports

---

## Contributing to Documentation

### Adding New Documentation

1. **Choose Appropriate Category**:
   - User guides → `getting-started/`
   - Architecture docs → `architecture-design/`
   - Implementation reports → `implementation/`
   - Analysis → `reports-analysis/`
   - Planning → `roadmap/`

2. **Follow Naming Conventions**:
   - Use UPPERCASE for main documents
   - Use descriptive names
   - Include dates if session-specific

3. **Update This Index**:
   - Add new document to relevant section
   - Update document counts

4. **Cross-Reference Properly**:
   - Use full relative paths: `docs/category/FILE.md`
   - Not root-relative paths: `/FILE.md`

---

## Search Tips

### By Keyword
Use file search in your editor or:
```bash
grep -r "keyword" docs/
```

### By Category
Navigate to specific category directory:
```bash
ls docs/implementation/
ls docs/architecture-design/
```

### By Date
Session summaries and weekly reports are dated:
- `SESSION_SUMMARY_2025-11-10.md`
- `WEEK3_SUCCESS.md`, `WEEK4_COMPLETE.md`

---

## Related Resources

### Code Documentation
- Python docstrings: 100% coverage
- C++ code comments
- TypeScript JSDoc comments

### External Links
- GitHub repository (if applicable)
- Project wiki (if applicable)
- Issue tracker (if applicable)

---

## Maintenance

This index is manually maintained. When adding new documentation:
1. Place file in appropriate category directory
2. Update this index
3. Update cross-references in related documents
4. Update document counts

**Last Index Update**: 2025-11-12
**Index Maintainer**: Project documentation team

---

## Quick Links

| Document | Location | Purpose |
|----------|----------|---------|
| Getting Started | [INSTRUCTIONS.md](getting-started/INSTRUCTIONS.md) | New user guide |
| Remaining Work | [DO_NOT_FORGET.md](roadmap/DO_NOT_FORGET.md) | TODO list |
| Project Structure | [STRUCTURAL_ANALYSIS.md](architecture-design/STRUCTURAL_ANALYSIS.md) | Full analysis |
| Metrics Summary | [PROJECT_METRICS_SUMMARY.md](reports-analysis/PROJECT_METRICS_SUMMARY.md) | Quick stats |
| Feature Status | [FEATURE_IMPLEMENTATION_STATUS_UPDATED.md](implementation/FEATURE_IMPLEMENTATION_STATUS_UPDATED.md) | Feature completion |
| Cache System | [CACHE_PHASE_C_COMPLETE.md](implementation/CACHE_PHASE_C_COMPLETE.md) | Phase C report |
| Governance | [GOVERNANCE_FEATURE_COMPLETE.md](implementation/GOVERNANCE_FEATURE_COMPLETE.md) | Feature 10 |

---

**Documentation Status**: ✅ Organized and indexed
**Total Categories**: 23 directories
**Total Documents**: 156+ markdown files
**All documentation now in `docs/` following project conventions**
