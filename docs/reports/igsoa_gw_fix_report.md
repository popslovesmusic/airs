# IGSOA GW Engine - CLI Integration Report

## Summary
- Fixed GW engine integration build errors by moving IGSOAGWEngine below GW includes and adding `<complex>`.
- Rebuilt `dase_cli.exe` and validated GW engine via CLI commands.
- Updated validation outputs to mark GW family as passing.

## Changes
- D:\airs\Simulation\dase_cli\src\engine_manager.cpp
  - Added `<complex>` include.
  - Moved IGSOAGWEngine definition below GW includes to resolve type visibility.

## Validation
- CLI command sequence (N_x=N_y=N_z=8, dt=0.01): create_engine -> run_mission(1) -> get_metrics -> destroy_engine.
- Results: all commands returned status=success; get_state returns `psi_real`, `psi_imag`, `phi`.

## Notes
- The existing test_cli_harness does not cover the igsoa_gw engine type; GW validation used direct CLI JSON commands.
