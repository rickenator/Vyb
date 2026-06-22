#!/usr/bin/env python3
"""
VyB Syntax Migration Tool v1.0
Automatically converts legacy ownership syntax to canonical syntax.

Usage:
    python3 migrate_syntax.py [options]

Options:
    --scan           Show what would be changed (dry run)
    --migrate        Apply changes to files
    --file PATH      Process single file
    --directory PATH Process directory recursively
    --backup         Create .backup files before changes
    --report         Generate migration report

Examples:
    # Scan entire project for legacy syntax
    python3 migrate_syntax.py --scan

    # Migrate specific file with backup
    python3 migrate_syntax.py --migrate --file examples/main.vyb --backup

    # Migrate entire examples directory
    python3 migrate_syntax.py --migrate --directory examples/ --backup --report
"""

import os
import re
import sys
import argparse
import json
import shutil
from pathlib import Path
from typing import List, Dict, Tuple, Optional
from dataclasses import dataclass
from datetime import datetime

@dataclass
class Migration:
    """Represents a single syntax migration."""
    file_path: str
    line_number: int
    old_text: str
    new_text: str
    rule_name: str
    context: str  # Surrounding context for verification

@dataclass
class MigrationReport:
    """Contains complete migration results."""
    files_scanned: int = 0
    files_modified: int = 0
    total_changes: int = 0
    migrations: List[Migration] = None
    warnings: List[str] = None
    errors: List[str] = None

    def __post_init__(self):
        if self.migrations is None:
            self.migrations = []
        if self.warnings is None:
            self.warnings = []
        if self.errors is None:
            self.errors = []

class SyntaxMigrator:
    """Handles migration from legacy to canonical VyB syntax."""

    def __init__(self):
        self.migration_rules = self._build_migration_rules()
        self.report = MigrationReport()

    def _build_migration_rules(self) -> List[Tuple[str, str, str, str]]:
        """Build list of (pattern, replacement, rule_name, description) tuples."""
        return [
            # Legacy function calls to canonical constructors
            (r'\bmake_my\s*\(\s*([^)]+)\s*\)',
             r'my(\1)',
             'make_my_to_my',
             'Convert make_my() calls to my() constructors'),

            (r'\bmake_our\s*\(\s*([^)]+)\s*\)',
             r'our(\1)',
             'make_our_to_our',
             'Convert make_our() calls to our() constructors'),

            # Legacy their creation patterns (if any exist)
            (r'\bmake_their\s*\(\s*([^)]+)\s*\)',
             r'view(\1)',
             'make_their_to_view',
             'Convert make_their() calls to view() operations'),

            # Prefix borrow/view syntax to function-call syntax
            (r'\bview\s+([A-Za-z_][A-Za-z0-9_]*)\b(?=\s*[,.;)\]}]|$)',
             r'view(\1)',
             'view_prefix_to_call',
             'Convert view expr to view(expr) syntax'),

            (r'\bborrow\s+([A-Za-z_][A-Za-z0-9_]*)\b(?=\s*[,.;)\]}]|$)',
             r'borrow(\1)',
             'borrow_prefix_to_call',
             'Convert borrow expr to borrow(expr) syntax'),

            # Documentation inconsistencies
            (r'\bmake_my\s+borrow\s+view',
             r'my() constructor with view/borrow operators',
             'doc_syntax_update',
             'Update documentation syntax descriptions'),

            # Comment inconsistencies
            (r'//.*make_my\s*\(',
             lambda m: m.group(0).replace('make_my(', 'my('),
             'comment_make_my_fix',
             'Fix make_my references in comments'),

            (r'//.*make_our\s*\(',
             lambda m: m.group(0).replace('make_our(', 'our('),
             'comment_make_our_fix',
             'Fix make_our references in comments'),
        ]

    def scan_file(self, file_path: str) -> List[Migration]:
        """Scan a single file for legacy syntax patterns."""
        migrations = []

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                lines = f.readlines()
        except (OSError, UnicodeDecodeError) as e:
            self.report.errors.append(f"Failed to read {file_path}: {e}")
            return []

        for line_num, line in enumerate(lines, 1):
            for pattern, replacement, rule_name, description in self.migration_rules:
                matches = re.finditer(pattern, line)
                for match in matches:
                    # Apply replacement
                    if callable(replacement):
                        new_text = replacement(match)
                    else:
                        new_text = re.sub(pattern, replacement, match.group(0))

                    # Get context (line before and after)
                    context_lines = []
                    if line_num > 1:
                        context_lines.append(f"{line_num-1}: {lines[line_num-2].rstrip()}")
                    context_lines.append(f"{line_num}: {line.rstrip()}")
                    if line_num < len(lines):
                        context_lines.append(f"{line_num+1}: {lines[line_num].rstrip()}")

                    migration = Migration(
                        file_path=file_path,
                        line_number=line_num,
                        old_text=match.group(0),
                        new_text=new_text,
                        rule_name=rule_name,
                        context='\n'.join(context_lines)
                    )
                    migrations.append(migration)

        return migrations

    def apply_migrations(self, file_path: str, migrations: List[Migration], create_backup: bool = True) -> bool:
        """Apply migrations to a file."""
        if not migrations:
            return False

        try:
            # Read original file
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()

            # Create backup if requested
            if create_backup:
                backup_path = f"{file_path}.backup"
                shutil.copy2(file_path, backup_path)
                print(f"Created backup: {backup_path}")

            # Apply migrations in reverse line order to preserve line numbers
            sorted_migrations = sorted(migrations, key=lambda m: m.line_number, reverse=True)
            lines = content.splitlines(keepends=True)

            for migration in sorted_migrations:
                line_idx = migration.line_number - 1
                if line_idx < len(lines):
                    old_line = lines[line_idx]
                    new_line = old_line.replace(migration.old_text, migration.new_text, 1)
                    lines[line_idx] = new_line

            # Write updated file
            with open(file_path, 'w', encoding='utf-8') as f:
                f.writelines(lines)

            return True

        except (OSError, UnicodeDecodeError) as e:
            self.report.errors.append(f"Failed to apply migrations to {file_path}: {e}")
            return False

    def scan_directory(self, directory: str, recursive: bool = True) -> Dict[str, List[Migration]]:
        """Scan directory for files needing migration."""
        all_migrations = {}

        path = Path(directory)
        if not path.exists():
            self.report.errors.append(f"Directory not found: {directory}")
            return {}

        # Define file patterns to scan
        patterns = ['*.vyb', '*.md', '*.txt', '*.cpp', '*.hpp']

        for pattern in patterns:
            if recursive:
                files = path.rglob(pattern)
            else:
                files = path.glob(pattern)

            for file_path in files:
                if file_path.is_file():
                    migrations = self.scan_file(str(file_path))
                    if migrations:
                        all_migrations[str(file_path)] = migrations
                    self.report.files_scanned += 1

        return all_migrations

    def generate_report(self, all_migrations: Dict[str, List[Migration]], output_file: Optional[str] = None) -> str:
        """Generate detailed migration report."""
        total_migrations = sum(len(migrations) for migrations in all_migrations.values())

        report_lines = [
            f"# VyB Syntax Migration Report",
            f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
            f"",
            f"## Summary",
            f"- Files scanned: {self.report.files_scanned}",
            f"- Files with legacy syntax: {len(all_migrations)}",
            f"- Total migrations needed: {total_migrations}",
            f"",
        ]

        if self.report.errors:
            report_lines.extend([
                f"## Errors ({len(self.report.errors)})",
                ""
            ])
            for error in self.report.errors:
                report_lines.append(f"- {error}")
            report_lines.append("")

        if self.report.warnings:
            report_lines.extend([
                f"## Warnings ({len(self.report.warnings)})",
                ""
            ])
            for warning in self.report.warnings:
                report_lines.append(f"- {warning}")
            report_lines.append("")

        # Group migrations by rule type
        by_rule = {}
        for file_path, migrations in all_migrations.items():
            for migration in migrations:
                if migration.rule_name not in by_rule:
                    by_rule[migration.rule_name] = []
                by_rule[migration.rule_name].append((file_path, migration))

        if by_rule:
            report_lines.extend([
                f"## Migration Details",
                ""
            ])

            for rule_name, rule_migrations in by_rule.items():
                report_lines.extend([
                    f"### {rule_name} ({len(rule_migrations)} instances)",
                    ""
                ])

                for file_path, migration in rule_migrations:
                    report_lines.extend([
                        f"**{file_path}:{migration.line_number}**",
                        f"```vyb",
                        f"// OLD:",
                        f"{migration.old_text}",
                        f"// NEW:",
                        f"{migration.new_text}",
                        f"```",
                        ""
                    ])

        # File-by-file breakdown
        if all_migrations:
            report_lines.extend([
                f"## Files Requiring Migration",
                ""
            ])

            for file_path, migrations in sorted(all_migrations.items()):
                report_lines.extend([
                    f"### {file_path} ({len(migrations)} changes)",
                    ""
                ])

                for migration in migrations:
                    report_lines.extend([
                        f"**Line {migration.line_number}:** {migration.rule_name}",
                        f"```",
                        migration.context,
                        f"```",
                        f"**Change:** `{migration.old_text}` → `{migration.new_text}`",
                        ""
                    ])

        report_content = '\n'.join(report_lines)

        if output_file:
            try:
                with open(output_file, 'w', encoding='utf-8') as f:
                    f.write(report_content)
                print(f"Report saved to: {output_file}")
            except OSError as e:
                self.report.errors.append(f"Failed to save report: {e}")

        return report_content

def main():
    parser = argparse.ArgumentParser(
        description="Migrate legacy VyB syntax to canonical syntax",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )

    parser.add_argument('--scan', action='store_true',
                       help='Show what would be changed (dry run)')
    parser.add_argument('--migrate', action='store_true',
                       help='Apply changes to files')
    parser.add_argument('--file', type=str,
                       help='Process single file')
    parser.add_argument('--directory', type=str,
                       help='Process directory recursively')
    parser.add_argument('--backup', action='store_true',
                       help='Create .backup files before changes')
    parser.add_argument('--report', action='store_true',
                       help='Generate migration report')

    args = parser.parse_args()

    # Validate arguments
    if not args.scan and not args.migrate:
        print("Error: Must specify either --scan or --migrate")
        return 1

    if not args.file and not args.directory:
        # Default to current directory
        args.directory = '.'

    migrator = SyntaxMigrator()

    # Determine what to process
    if args.file:
        print(f"Processing file: {args.file}")
        migrations = {args.file: migrator.scan_file(args.file)}
        migrator.report.files_scanned = 1
    else:
        print(f"Processing directory: {args.directory}")
        migrations = migrator.scan_directory(args.directory)

    # Show scan results
    total_changes = sum(len(file_migrations) for file_migrations in migrations.values())

    if total_changes == 0:
        print("✅ No legacy syntax found - all files use canonical syntax!")
        return 0

    print(f"\n📊 Scan Results:")
    print(f"   Files scanned: {migrator.report.files_scanned}")
    print(f"   Files with legacy syntax: {len(migrations)}")
    print(f"   Total changes needed: {total_changes}")

    # Show changes by file
    for file_path, file_migrations in migrations.items():
        print(f"\n📝 {file_path} ({len(file_migrations)} changes):")
        for migration in file_migrations:
            print(f"   Line {migration.line_number}: {migration.old_text} → {migration.new_text}")

    # Apply migrations if requested
    if args.migrate:
        print(f"\n🔧 Applying migrations...")

        for file_path, file_migrations in migrations.items():
            if migrator.apply_migrations(file_path, file_migrations, args.backup):
                print(f"   ✅ Updated {file_path}")
                migrator.report.files_modified += 1
                migrator.report.total_changes += len(file_migrations)
            else:
                print(f"   ❌ Failed to update {file_path}")

        print(f"\n✅ Migration complete:")
        print(f"   Files modified: {migrator.report.files_modified}")
        print(f"   Total changes applied: {migrator.report.total_changes}")

    # Generate report if requested
    if args.report:
        report_file = f"migration_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.md"
        migrator.generate_report(migrations, report_file)

    return 0

if __name__ == '__main__':
    sys.exit(main())