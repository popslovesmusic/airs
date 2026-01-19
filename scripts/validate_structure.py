#!/usr/bin/env python3
"""
AIRS Structure Validation Script
Verifies the directory structure and component functionality
"""

import json
import sys
from pathlib import Path
from typing import List, Tuple

class AIRSValidator:
    def __init__(self, root_path="D:/airs"):
        self.root = Path(root_path)
        self.errors = []
        self.warnings = []
        self.checks_passed = 0
        self.checks_total = 0

    def check(self, condition: bool, message: str, critical: bool = True) -> bool:
        """Run a validation check"""
        self.checks_total += 1
        if condition:
            self.checks_passed += 1
            print(f"[OK] {message}")
            return True
        else:
            if critical:
                self.errors.append(message)
                print(f"[FAIL] {message}")
            else:
                self.warnings.append(message)
                print(f"[WARN] {message}")
            return False

    def validate_directory_structure(self):
        """Validate core directory structure"""
        print("\n" + "="*60)
        print("Validating Directory Structure")
        print("="*60)

        # Core directories
        core_dirs = [
            "ssot",
            "ssot/data",
            "ssot/api",
            "ssot/api/schemas",
            "ssot/tools",
            "ssot/bin",
            "sim",
            "sim/src/cpp",
            "sim/cli",
            "sim/cli/schemas",
            "sim/bin",
            "integration",
            "integration/schemas",
            "workspace",
            "workspace/input",
            "workspace/output",
            "config"
        ]

        for dir_path in core_dirs:
            full_path = self.root / dir_path
            self.check(
                full_path.exists() and full_path.is_dir(),
                f"Directory exists: {dir_path}"
            )

    def validate_ssot_data(self):
        """Validate SSOT data files"""
        print("\n" + "="*60)
        print("Validating SSOT Data")
        print("="*60)

        data_files = [
            ("ssot/data/ssot_parallel.db", True),
            ("ssot/data/rmap_graph.gpickle", True),
            ("ssot/data/manifest.json", True),
            ("ssot/data/SSOT_ROOT.json5", True)
        ]

        for file_path, critical in data_files:
            full_path = self.root / file_path
            exists = full_path.exists() and full_path.is_file()
            self.check(exists, f"Data file exists: {file_path}", critical)

            if exists and file_path.endswith('.db'):
                size_mb = full_path.stat().st_size / 1024 / 1024
                self.check(
                    size_mb > 100,
                    f"Database has data: {file_path} ({size_mb:.1f} MB)",
                    critical=False
                )

    def validate_ssot_schemas(self):
        """Validate SSOT JSON schemas"""
        print("\n" + "="*60)
        print("Validating SSOT Schemas")
        print("="*60)

        schemas = [
            "ssot/api/schemas/search_request.schema.json",
            "ssot/api/schemas/search_response.schema.json",
            "ssot/api/schemas/document.schema.json"
        ]

        for schema_path in schemas:
            full_path = self.root / schema_path
            exists = full_path.exists()
            self.check(exists, f"Schema exists: {schema_path}")

            if exists:
                try:
                    with open(full_path, 'r') as f:
                        schema = json.load(f)
                    self.check(
                        '$schema' in schema,
                        f"Schema is valid JSON: {schema_path}",
                        critical=False
                    )
                except json.JSONDecodeError:
                    self.check(False, f"Schema is valid JSON: {schema_path}")

    def validate_simulation_binaries(self):
        """Validate simulation binaries"""
        print("\n" + "="*60)
        print("Validating Simulation Binaries")
        print("="*60)

        binaries = [
            "sim/bin/dase_cli.exe",
            "sim/bin/libfftw3-3.dll"
        ]

        for bin_path in binaries:
            full_path = self.root / bin_path
            self.check(
                full_path.exists() and full_path.is_file(),
                f"Binary exists: {bin_path}"
            )

    def validate_simulation_schemas(self):
        """Validate simulation CLI schemas"""
        print("\n" + "="*60)
        print("Validating Simulation Schemas")
        print("="*60)

        schemas = [
            "sim/cli/schemas/command.schema.json",
            "sim/cli/schemas/response.schema.json",
            "sim/cli/schemas/state.schema.json"
        ]

        for schema_path in schemas:
            full_path = self.root / schema_path
            exists = full_path.exists()
            self.check(exists, f"Schema exists: {schema_path}")

            if exists:
                try:
                    with open(full_path, 'r') as f:
                        schema = json.load(f)
                    self.check(
                        '$schema' in schema,
                        f"Schema is valid JSON: {schema_path}",
                        critical=False
                    )
                except json.JSONDecodeError:
                    self.check(False, f"Schema is valid JSON: {schema_path}")

    def validate_configuration(self):
        """Validate configuration files"""
        print("\n" + "="*60)
        print("Validating Configuration Files")
        print("="*60)

        configs = [
            "environment.json",
            "ssot/config.json",
            "sim/config.json"
        ]

        for config_path in configs:
            full_path = self.root / config_path
            exists = full_path.exists()
            self.check(exists, f"Config exists: {config_path}")

            if exists:
                try:
                    with open(full_path, 'r') as f:
                        config = json.load(f)
                    self.check(
                        len(config) > 0,
                        f"Config has content: {config_path}",
                        critical=False
                    )
                except json.JSONDecodeError:
                    self.check(False, f"Config is valid JSON: {config_path}")

    def validate_documentation(self):
        """Validate documentation files"""
        print("\n" + "="*60)
        print("Validating Documentation")
        print("="*60)

        docs = [
            "README.md",
            "ssot/README.md",
            "sim/README.md",
            "integration/README.md"
        ]

        for doc_path in docs:
            full_path = self.root / doc_path
            self.check(
                full_path.exists() and full_path.is_file(),
                f"Documentation exists: {doc_path}",
                critical=False
            )

    def print_summary(self):
        """Print validation summary"""
        print("\n" + "="*60)
        print("Validation Summary")
        print("="*60)

        print(f"\nChecks Passed: {self.checks_passed}/{self.checks_total}")

        if self.errors:
            print(f"\n[ERRORS] ({len(self.errors)}):")
            for error in self.errors:
                print(f"  - {error}")

        if self.warnings:
            print(f"\n[WARNINGS] ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"  - {warning}")

        if not self.errors:
            print("\n[SUCCESS] All critical checks passed!")
            print("AIRS structure is valid and ready for agent use.")
            return True
        else:
            print("\n[FAILED] Validation failed. Please fix errors above.")
            return False

    def run(self):
        """Run all validations"""
        print("="*60)
        print("AIRS Structure Validation")
        print("="*60)

        self.validate_directory_structure()
        self.validate_ssot_data()
        self.validate_ssot_schemas()
        self.validate_simulation_binaries()
        self.validate_simulation_schemas()
        self.validate_configuration()
        self.validate_documentation()

        return self.print_summary()


def main():
    validator = AIRSValidator()
    success = validator.run()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
