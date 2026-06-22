#!/usr/bin/env python3
"""
VyB Test Triage Tool

Analyzes test results to create comprehensive triage plans, identify patterns in failures,
and provide actionable insights for development priorities.

Features:
- Failure pattern analysis
- Priority-based triage plans
- Test health metrics
- Regression detection
- Performance trend analysis
"""

import json
import argparse
import sys
from pathlib import Path
from collections import defaultdict, Counter
from dataclasses import dataclass
from typing import List, Dict, Any, Optional
import re
import datetime


@dataclass
class TriageItem:
    """Represents a test failure that needs attention."""
    test_name: str
    test_file: str
    category: List[str]
    error_message: str
    failure_count: int = 1
    priority: str = "normal"
    suggested_action: str = ""
    related_tests: List[str] = None
    estimated_effort: str = "unknown"  # low, medium, high

    def __post_init__(self):
        if self.related_tests is None:
            self.related_tests = []


class TestTriageAnalyzer:
    """Analyzes test results and creates triage plans."""

    def __init__(self):
        self.error_patterns = {
            r"parse error|syntax error|unexpected token": {
                "category": "parser",
                "priority": "high",
                "action": "Fix parser grammar or syntax handling",
                "effort": "medium"
            },
            r"type mismatch|type error|cannot convert": {
                "category": "semantic",
                "priority": "high",
                "action": "Review type system and type checking",
                "effort": "medium"
            },
            r"segmentation fault|segfault|signal 11": {
                "category": "runtime",
                "priority": "critical",
                "action": "Debug memory safety issue - use valgrind/asan",
                "effort": "high"
            },
            r"assertion failed|assert": {
                "category": "runtime",
                "priority": "high",
                "action": "Review assertion and fix underlying issue",
                "effort": "medium"
            },
            r"timeout|timed out": {
                "category": "performance",
                "priority": "medium",
                "action": "Optimize performance or increase timeout",
                "effort": "medium"
            },
            r"undefined symbol|undefined reference": {
                "category": "linker",
                "priority": "high",
                "action": "Fix symbol resolution or add missing implementation",
                "effort": "low"
            },
            r"out of memory|memory allocation": {
                "category": "memory",
                "priority": "high",
                "action": "Review memory management and allocation",
                "effort": "high"
            },
            r"file not found|cannot open": {
                "category": "io",
                "priority": "low",
                "action": "Check file paths and permissions",
                "effort": "low"
            }
        }

    def analyze_results(self, results_file: str) -> Dict[str, Any]:
        """Analyze test results and create comprehensive triage plan."""
        with open(results_file, 'r') as f:
            data = json.load(f)

        failed_tests = [
            result for result in data['results']
            if not result['result']['success']
        ]

        # Basic statistics
        stats = {
            "total_tests": data['metadata']['total_tests'],
            "failed_tests": len(failed_tests),
            "pass_rate": (data['metadata']['passed'] / data['metadata']['total_tests']) * 100,
            "categories_affected": set(),
            "priority_distribution": Counter()
        }

        # Analyze failures
        triage_items = []
        error_clusters = defaultdict(list)
        category_failures = defaultdict(int)

        for result in failed_tests:
            test = result['test']
            result_data = result['result']

            # Categorize error
            error_msg = result_data.get('error_message', '')
            stderr = result_data.get('stderr', '')
            full_error = f"{error_msg} {stderr}".lower()

            # Find matching error pattern
            matched_pattern = None
            for pattern, info in self.error_patterns.items():
                if re.search(pattern, full_error, re.IGNORECASE):
                    matched_pattern = info
                    break

            if not matched_pattern:
                matched_pattern = {
                    "category": "unknown",
                    "priority": "medium",
                    "action": "Investigate test failure",
                    "effort": "medium"
                }

            # Create triage item
            triage_item = TriageItem(
                test_name=test['name'],
                test_file=test['filename'],
                category=test['category'],
                error_message=error_msg or stderr[:200],
                priority=matched_pattern['priority'],
                suggested_action=matched_pattern['action'],
                estimated_effort=matched_pattern['effort']
            )

            triage_items.append(triage_item)

            # Collect statistics
            for cat in test['category']:
                stats['categories_affected'].add(cat)
                category_failures[cat] += 1

            stats['priority_distribution'][matched_pattern['priority']] += 1

            # Group similar errors
            error_key = self._normalize_error(full_error)
            error_clusters[error_key].append(triage_item)

        # Find related test failures
        self._find_related_failures(triage_items, error_clusters)

        # Create triage plan
        triage_plan = self._create_triage_plan(triage_items)

        return {
            "stats": {
                **stats,
                "categories_affected": list(stats['categories_affected']),
                "category_failures": dict(category_failures)
            },
            "triage_items": [self._triage_item_to_dict(item) for item in triage_items],
            "triage_plan": triage_plan,
            "error_clusters": {k: len(v) for k, v in error_clusters.items()},
            "recommendations": self._generate_recommendations(stats, category_failures, triage_items)
        }

    def _normalize_error(self, error_text: str) -> str:
        """Normalize error text for clustering similar errors."""
        # Remove file paths, line numbers, and other variable parts
        normalized = re.sub(r'/[^\s]+', '<path>', error_text)
        normalized = re.sub(r'line \d+', 'line <num>', normalized)
        normalized = re.sub(r'column \d+', 'column <num>', normalized)
        normalized = re.sub(r'\d+', '<num>', normalized)
        return normalized[:100]  # Truncate for clustering

    def _find_related_failures(self, triage_items: List[TriageItem], error_clusters: Dict[str, List[TriageItem]]):
        """Find related test failures based on error patterns and categories."""
        for error_key, items in error_clusters.items():
            if len(items) > 1:
                # Group items with same error pattern
                for item in items:
                    item.related_tests = [other.test_name for other in items if other != item]

    def _create_triage_plan(self, triage_items: List[TriageItem]) -> List[Dict[str, Any]]:
        """Create a prioritized triage plan."""
        # Sort by priority and estimated effort
        priority_order = {"critical": 0, "high": 1, "medium": 2, "low": 3}
        effort_order = {"low": 0, "medium": 1, "high": 2}

        sorted_items = sorted(
            triage_items,
            key=lambda x: (priority_order.get(x.priority, 3), effort_order.get(x.estimated_effort, 2))
        )

        # Group into phases
        phases = {
            "Phase 1 - Critical Issues": [],
            "Phase 2 - High Priority": [],
            "Phase 3 - Medium Priority": [],
            "Phase 4 - Low Priority": []
        }

        for item in sorted_items:
            if item.priority == "critical":
                phases["Phase 1 - Critical Issues"].append(item)
            elif item.priority == "high":
                phases["Phase 2 - High Priority"].append(item)
            elif item.priority == "medium":
                phases["Phase 3 - Medium Priority"].append(item)
            else:
                phases["Phase 4 - Low Priority"].append(item)

        # Convert to plan format
        plan = []
        for phase_name, items in phases.items():
            if items:
                plan.append({
                    "phase": phase_name,
                    "item_count": len(items),
                    "estimated_effort": self._estimate_phase_effort(items),
                    "items": [self._triage_item_to_dict(item) for item in items]
                })

        return plan

    def _estimate_phase_effort(self, items: List[TriageItem]) -> str:
        """Estimate effort for a phase based on contained items."""
        effort_scores = {"low": 1, "medium": 3, "high": 5}
        total_score = sum(effort_scores.get(item.estimated_effort, 3) for item in items)

        if total_score <= 5:
            return "low"
        elif total_score <= 15:
            return "medium"
        else:
            return "high"

    def _generate_recommendations(self, stats: Dict, category_failures: Dict, triage_items: List[TriageItem]) -> List[str]:
        """Generate actionable recommendations based on analysis."""
        recommendations = []

        # Pass rate recommendations
        if stats['pass_rate'] < 50:
            recommendations.append("🚨 URGENT: Pass rate is critically low. Consider focusing on core functionality first.")
        elif stats['pass_rate'] < 75:
            recommendations.append("⚠️  Pass rate needs improvement. Prioritize high-impact fixes.")
        elif stats['pass_rate'] < 90:
            recommendations.append("✅ Good progress. Focus on remaining edge cases and optimizations.")

        # Category-specific recommendations
        for category, count in category_failures.items():
            if count > 10:
                recommendations.append(f"🔧 High failure rate in '{category}' category ({count} failures). Consider reviewing this subsystem.")

        # Priority-based recommendations
        critical_count = len([item for item in triage_items if item.priority == "critical"])
        if critical_count > 0:
            recommendations.append(f"🚨 {critical_count} critical issues found. These should be addressed immediately.")

        # Pattern-based recommendations
        memory_issues = len([item for item in triage_items if "memory" in item.error_message.lower() or "segfault" in item.error_message.lower()])
        if memory_issues > 5:
            recommendations.append("🧠 Multiple memory-related issues detected. Consider running tests with AddressSanitizer or Valgrind.")

        parser_issues = len([item for item in triage_items if "parser" in ' '.join(item.category)])
        if parser_issues > 10:
            recommendations.append("📝 Many parser-related failures. Consider reviewing grammar rules and parser implementation.")

        return recommendations

    def _triage_item_to_dict(self, item: TriageItem) -> Dict[str, Any]:
        """Convert triage item to dictionary format."""
        return {
            "test_name": item.test_name,
            "test_file": item.test_file,
            "category": item.category,
            "error_message": item.error_message,
            "priority": item.priority,
            "suggested_action": item.suggested_action,
            "estimated_effort": item.estimated_effort,
            "related_tests": item.related_tests
        }

    def generate_report(self, analysis: Dict[str, Any], output_format: str = "console") -> str:
        """Generate a formatted triage report."""
        if output_format == "console":
            return self._generate_console_report(analysis)
        elif output_format == "markdown":
            return self._generate_markdown_report(analysis)
        else:
            raise ValueError(f"Unknown output format: {output_format}")

    def _generate_console_report(self, analysis: Dict[str, Any]) -> str:
        """Generate console-formatted triage report."""
        stats = analysis['stats']

        report = f"""
{'='*80}
VyB Test Triage Report
{'='*80}

📊 OVERALL STATISTICS
• Total Tests: {stats['total_tests']}
• Failed Tests: {stats['failed_tests']}
• Pass Rate: {stats['pass_rate']:.1f}%
• Categories Affected: {len(stats['categories_affected'])}

📈 FAILURE BREAKDOWN BY CATEGORY
"""

        for category, count in stats['category_failures'].items():
            percentage = (count / stats['failed_tests']) * 100 if stats['failed_tests'] > 0 else 0
            report += f"• {category}: {count} failures ({percentage:.1f}%)\n"

        report += f"\n🎯 PRIORITY DISTRIBUTION\n"
        for priority, count in analysis['stats']['priority_distribution'].items():
            report += f"• {priority.upper()}: {count} items\n"

        report += f"\n📋 TRIAGE PLAN\n"
        for phase in analysis['triage_plan']:
            report += f"\n{phase['phase']} ({phase['item_count']} items, {phase['estimated_effort']} effort)\n"
            report += "-" * 60 + "\n"

            for item in phase['items'][:5]:  # Show first 5 items per phase
                report += f"• {item['test_name']}\n"
                report += f"  Priority: {item['priority'].upper()} | Effort: {item['estimated_effort']}\n"
                report += f"  Action: {item['suggested_action']}\n"
                if item['related_tests']:
                    report += f"  Related: {', '.join(item['related_tests'][:3])}\n"
                report += "\n"

            if len(phase['items']) > 5:
                report += f"  ... and {len(phase['items']) - 5} more items\n\n"

        report += "💡 RECOMMENDATIONS\n"
        for i, rec in enumerate(analysis['recommendations'], 1):
            report += f"{i}. {rec}\n"

        return report

    def _generate_markdown_report(self, analysis: Dict[str, Any]) -> str:
        """Generate markdown-formatted triage report."""
        stats = analysis['stats']
        timestamp = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        report = f"""# VyB Test Triage Report

*Generated: {timestamp}*

## 📊 Overall Statistics

| Metric | Value |
|--------|--------|
| Total Tests | {stats['total_tests']} |
| Failed Tests | {stats['failed_tests']} |
| Pass Rate | {stats['pass_rate']:.1f}% |
| Categories Affected | {len(stats['categories_affected'])} |

## 📈 Failure Breakdown by Category

| Category | Failures | Percentage |
|----------|----------|------------|
"""

        for category, count in stats['category_failures'].items():
            percentage = (count / stats['failed_tests']) * 100 if stats['failed_tests'] > 0 else 0
            report += f"| {category} | {count} | {percentage:.1f}% |\n"

        report += "\n## 🎯 Priority Distribution\n\n"
        for priority, count in analysis['stats']['priority_distribution'].items():
            report += f"- **{priority.upper()}**: {count} items\n"

        report += "\n## 📋 Triage Plan\n"
        for phase in analysis['triage_plan']:
            report += f"\n### {phase['phase']}\n"
            report += f"*{phase['item_count']} items, {phase['estimated_effort']} effort*\n\n"

            for item in phase['items']:
                report += f"#### {item['test_name']}\n"
                report += f"- **Priority**: {item['priority'].upper()}\n"
                report += f"- **Effort**: {item['estimated_effort']}\n"
                report += f"- **Action**: {item['suggested_action']}\n"
                report += f"- **File**: `{item['test_file']}`\n"
                if item['error_message']:
                    report += f"- **Error**: `{item['error_message']}`\n"
                if item['related_tests']:
                    report += f"- **Related Tests**: {', '.join(item['related_tests'])}\n"
                report += "\n"

        report += "## 💡 Recommendations\n\n"
        for i, rec in enumerate(analysis['recommendations'], 1):
            report += f"{i}. {rec}\n"

        return report


def main():
    """Main entry point for the triage tool."""
    parser = argparse.ArgumentParser(
        description='VyB Test Triage Tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Analyze test results and display console report
  ./triage_tool.py results.json

  # Generate markdown report
  ./triage_tool.py results.json --format markdown --output triage.md

  # Display only high-priority items
  ./triage_tool.py results.json --priority high,critical
        """
    )

    parser.add_argument('results_file', help='JSON test results file')
    parser.add_argument('--format', choices=['console', 'markdown'], default='console', help='Output format')
    parser.add_argument('--output', '-o', help='Output file (default: stdout)')
    parser.add_argument('--priority', help='Filter by priority (comma-separated)')

    args = parser.parse_args()

    if not Path(args.results_file).exists():
        print(f"Error: Results file {args.results_file} not found")
        sys.exit(1)

    # Analyze results
    analyzer = TestTriageAnalyzer()
    analysis = analyzer.analyze_results(args.results_file)

    # Filter by priority if specified
    if args.priority:
        priorities = [p.strip().lower() for p in args.priority.split(',')]
        filtered_plan = []
        for phase in analysis['triage_plan']:
            filtered_items = [
                item for item in phase['items']
                if item['priority'] in priorities
            ]
            if filtered_items:
                phase_copy = phase.copy()
                phase_copy['items'] = filtered_items
                phase_copy['item_count'] = len(filtered_items)
                filtered_plan.append(phase_copy)
        analysis['triage_plan'] = filtered_plan

    # Generate report
    report = analyzer.generate_report(analysis, args.format)

    # Output report
    if args.output:
        with open(args.output, 'w') as f:
            f.write(report)
        print(f"Triage report saved to: {args.output}")
    else:
        print(report)


if __name__ == "__main__":
    main()