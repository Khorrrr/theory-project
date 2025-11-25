"""
Code Validator for ML Translation Output

This module provides validation utilities to check the correctness
and completeness of ML-generated code translations.
"""

import re
import ast
import logging
from typing import Dict, List, Any, Optional, Tuple
from enum import Enum

logger = logging.getLogger(__name__)

class ValidationLevel(Enum):
    """Validation severity levels."""
    ERROR = "error"
    WARNING = "warning"
    INFO = "info"

class ValidationResult:
    """Container for validation results."""
    def __init__(self):
        self.is_valid = True
        self.issues = {
            ValidationLevel.ERROR: [],
            ValidationLevel.WARNING: [],
            ValidationLevel.INFO: []
        }
        self.suggestions = []
        self.confidence = 1.0

    def add_issue(self, level: ValidationLevel, message: str, line: Optional[int] = None):
        """Add a validation issue."""
        self.issues[level].append({
            'message': message,
            'line': line,
            'level': level.value
        })

        if level == ValidationLevel.ERROR:
            self.is_valid = False
            self.confidence = max(0.0, self.confidence - 0.2)
        elif level == ValidationLevel.WARNING:
            self.confidence = max(0.0, self.confidence - 0.1)

    def add_suggestion(self, suggestion: str):
        """Add a formatting or improvement suggestion."""
        self.suggestions.append(suggestion)

    def to_dict(self) -> Dict[str, Any]:
        """Convert validation result to dictionary."""
        return {
            'is_valid': self.is_valid,
            'confidence': self.confidence,
            'errors': self.issues[ValidationLevel.ERROR],
            'warnings': self.issues[ValidationLevel.WARNING],
            'info': self.issues[ValidationLevel.INFO],
            'suggestions': self.suggestions
        }

class CodeValidator:
    """
    Validates ML-generated code translations for correctness and completeness.
    """

    def __init__(self):
        # Language-specific validation patterns
        self.patterns = {
            'python': {
                'syntax_keywords': ['def', 'class', 'if', 'else', 'elif', 'for', 'while', 'try', 'except', 'with'],
                'invalid_patterns': [
                    r'end$',  # Ruby-style end
                    r'\{$',  # Unclosed brace
                    r'\}$',  # Unmatched closing brace
                ],
                'required_imports': {},  # Context-dependent
            },
            'java': {
                'syntax_keywords': ['public', 'private', 'protected', 'static', 'class', 'interface', 'if', 'else', 'for', 'while', 'do', 'try', 'catch', 'finally'],
                'invalid_patterns': [
                    r'def\s+\w+',  # Python function definition
                    r'print\s*\(',  # Python print
                    r'end$',  # Ruby-style end
                ],
                'required_imports': {},
            },
            'javascript': {
                'syntax_keywords': ['function', 'var', 'let', 'const', 'class', 'if', 'else', 'for', 'while', 'do', 'try', 'catch', 'finally'],
                'invalid_patterns': [
                    r'def\s+\w+',  # Python function definition
                    r'print\s*\(',  # Python print
                    r'end$',  # Ruby-style end
                ],
                'required_imports': {},
            },
            'assembly': {
                'syntax_keywords': ['mov', 'add', 'sub', 'jmp', 'call', 'push', 'pop', 'ret'],
                'invalid_patterns': [
                    r'function\s+\w+',  # Function keyword in assembly
                    r'class\s+\w+',  # Class keyword in assembly
                ],
                'required_imports': {},
            }
        }

    def validate(self, code: str, target_language: str, source_info: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Validate generated code against language-specific rules.

        Args:
            code: Generated code to validate
            target_language: Target programming language
            source_info: Optional information about the source code

        Returns:
            Dict containing validation results
        """
        result = ValidationResult()

        if not code or not code.strip():
            result.add_issue(ValidationLevel.ERROR, "Generated code is empty")
            return result.to_dict()

        # Basic structural validation
        self._validate_basic_structure(code, target_language, result)

        # Language-specific validation
        if target_language.lower() == 'python':
            self._validate_python(code, result)
        elif target_language.lower() in ['java', 'javascript']:
            self._validate_c_like(code, target_language, result)
        elif target_language.lower() == 'assembly':
            self._validate_assembly(code, result)

        # Cross-language consistency validation
        if source_info:
            self._validate_consistency(code, target_language, source_info, result)

        # Quality assessment
        self._assess_quality(code, target_language, result)

        return result.to_dict()

    def _validate_basic_structure(self, code: str, target_language: str, result: ValidationResult):
        """Validate basic code structure."""
        lines = code.split('\n')
        non_empty_lines = [line for line in lines if line.strip()]

        if not non_empty_lines:
            result.add_issue(ValidationLevel.ERROR, "Code contains no meaningful content")
            return

        # Check for obvious ML artifacts
        artifact_patterns = [
            r'here is the',
            r'the following',
            r'as you can see',
            r'note that',
            r'please note',
            r'translation:',
            r'generated code:'
        ]

        for i, line in enumerate(lines, 1):
            line_lower = line.lower()
            for pattern in artifact_patterns:
                if re.search(pattern, line_lower):
                    result.add_issue(ValidationLevel.WARNING, f"Possible ML artifact detected", i)

        # Check for code block markers
        if re.search(r'```\w*', code):
            result.add_issue(ValidationLevel.WARNING, "Code contains markdown formatting markers")

    def _validate_python(self, code: str, result: ValidationResult):
        """Validate Python code."""
        try:
            # Try to parse as Python AST
            ast.parse(code)
        except SyntaxError as e:
            result.add_issue(ValidationLevel.ERROR, f"Python syntax error: {e.msg}", e.lineno)

        # Check for common Python-specific issues
        lines = code.split('\n')
        for i, line in enumerate(lines, 1):
            # Check for semicolons (uncommon in Python)
            if line.strip().endswith(';') and not line.strip().endswith("';"):
                result.add_issue(ValidationLevel.INFO, "Unnecessary semicolon detected", i)

            # Check for braces (not Pythonic)
            if '{' in line or '}' in line:
                result.add_issue(ValidationLevel.WARNING, "Braces detected in Python code", i)

            # Check indentation consistency
            if line.startswith(' ') and not line.startswith('    ') and not line.startswith('        '):
                if line.strip():  # Not an empty line
                    result.add_issue(ValidationLevel.WARNING, "Inconsistent indentation (should be 4 spaces)", i)

    def _validate_c_like(self, code: str, target_language: str, result: ValidationResult):
        """Validate Java/JavaScript (C-like) code."""
        lines = code.split('\n')
        brace_count = 0
        paren_count = 0

        for i, line in enumerate(lines, 1):
            brace_count += line.count('{') - line.count('}')
            paren_count += line.count('(') - line.count(')')

            # Check for missing semicolons (should end most statements)
            stripped = line.strip()
            if (stripped and
                not stripped.startswith('//') and
                not stripped.startswith('/*') and
                not stripped.endswith('{') and
                not stripped.endswith('}') and
                not stripped.endswith(':') and
                not any(keyword in stripped for keyword in ['if', 'for', 'while', 'switch', 'else', 'do', 'try', 'catch', 'finally']) and
                not stripped.endswith(';')):

                # Check if it looks like a statement that should end with semicolon
                if (re.search(r'\w+\s*=', stripped) or  # Assignment
                    re.search(r'\w+\(.*\)', stripped) or  # Function call
                    re.search(r'return\s+', stripped) or  # Return statement
                    re.search(r'break\b', stripped) or  # Break
                    re.search(r'continue\b', stripped)):  # Continue

                    result.add_issue(ValidationLevel.WARNING, "Possible missing semicolon", i)

        # Check brace balance
        if brace_count != 0:
            result.add_issue(ValidationLevel.ERROR, f"Unmatched braces: {brace_count}")

        # Check parenthesis balance
        if paren_count != 0:
            result.add_issue(ValidationLevel.WARNING, f"Unmatched parentheses: {paren_count}")

    def _validate_assembly(self, code: str, result: ValidationResult):
        """Validate assembly code."""
        lines = code.split('\n')
        valid_instructions = [
            'mov', 'add', 'sub', 'mul', 'div', 'jmp', 'call', 'push', 'pop', 'ret',
            'cmp', 'test', 'je', 'jne', 'jg', 'jl', 'jge', 'jle', 'inc', 'dec',
            'and', 'or', 'xor', 'not', 'shl', 'shr', 'nop', 'int', 'hlt'
        ]

        for i, line in enumerate(lines, 1):
            stripped = line.strip().lower()

            if not stripped or stripped.startswith(';'):
                continue

            # Check if line starts with a valid instruction
            parts = stripped.split(None, 1)
            if not parts:
                continue

            instruction = parts[0]

            # Skip labels (end with colon)
            if instruction.endswith(':'):
                continue

            # Check for unknown instructions
            if instruction not in valid_instructions:
                result.add_issue(ValidationLevel.WARNING, f"Unknown assembly instruction: {instruction}", i)

    def _validate_consistency(self, code: str, target_language: str, source_info: Dict[str, Any], result: ValidationResult):
        """Validate consistency with source code information."""
        # Check if functions from source are present in translation
        if 'functions' in source_info:
            source_functions = [func['name'] for func in source_info['functions']]

            for func_name in source_functions:
                # Simple check - look for function name in translated code
                if func_name not in code:
                    result.add_issue(ValidationLevel.WARNING, f"Function '{func_name}' from source not found in translation")

        # Check if classes from source are present in translation
        if 'classes' in source_info:
            source_classes = [cls['name'] for cls in source_info['classes']]

            for class_name in source_classes:
                if class_name not in code:
                    result.add_issue(ValidationLevel.WARNING, f"Class '{class_name}' from source not found in translation")

    def _assess_quality(self, code: str, target_language: str, result: ValidationResult):
        """Assess overall code quality."""
        lines = code.split('\n')
        non_empty_lines = [line for line in lines if line.strip()]

        # Code length assessment
        if len(non_empty_lines) == 1 and len(lines[0]) < 50:
            result.add_issue(ValidationLevel.INFO, "Very short translation - may be incomplete")
            result.confidence = max(0.5, result.confidence - 0.1)

        # Comment density
        comment_lines = sum(1 for line in lines if re.search(r'^\s*(#|//|/\*)', line))
        if comment_lines == 0 and len(non_empty_lines) > 10:
            result.add_suggestion("Consider adding comments to explain the translation")

        # Repetition detection
        if len(set(non_empty_lines)) < len(non_empty_lines) * 0.7:
            result.add_issue(ValidationLevel.WARNING, "High code repetition detected")
            result.confidence = max(0.7, result.confidence - 0.1)

        # Language-specific quality checks
        if target_language.lower() == 'python':
            self._assess_python_quality(code, result)

    def _assess_python_quality(self, code: str, result: ValidationResult):
        """Assess Python-specific code quality."""
        # Check for Pythonic practices
        if re.search(r'range\s*\(\s*len\s*\(', code):
            result.add_suggestion("Consider using enumerate() instead of range(len())")

        if re.search(r'for\s+\w+\s+in\s+range\s*\(', code) and 'while' not in code:
            result.add_suggestion("Consider if a more Pythonic iteration method is available")

    def check_syntax_compatibility(self, code: str, target_language: str) -> Tuple[bool, List[str]]:
        """
        Check if code is syntactically compatible with target language.

        Args:
            code: Code to check
            target_language: Target programming language

        Returns:
            Tuple of (is_compatible, error_messages)
        """
        error_messages = []

        if target_language.lower() == 'python':
            try:
                ast.parse(code)
                return True, error_messages
            except SyntaxError as e:
                error_messages.append(f"Python syntax error: {e.msg}")
                return False, error_messages

        # For other languages, do basic pattern checking
        patterns = self.patterns.get(target_language.lower(), {})
        invalid_patterns = patterns.get('invalid_patterns', [])

        for pattern in invalid_patterns:
            if re.search(pattern, code):
                error_messages.append(f"Invalid pattern detected: {pattern}")

        return len(error_messages) == 0, error_messages

    def estimate_translation_quality(self, source_info: Optional[Dict[str, Any]],
                                    translation_result: Dict[str, Any]) -> float:
        """
        Estimate overall translation quality.

        Args:
            source_info: Information about source code
            translation_result: Validation result for translation

        Returns:
            float: Quality score between 0.0 and 1.0
        """
        base_quality = translation_result.get('confidence', 0.8)

        # Adjust based on validation issues
        error_count = len(translation_result.get('errors', []))
        warning_count = len(translation_result.get('warnings', []))

        quality = base_quality
        quality -= (error_count * 0.2)  # Penalize errors heavily
        quality -= (warning_count * 0.05)  # Penalize warnings lightly

        # Bonus for no issues
        if error_count == 0 and warning_count == 0:
            quality = min(1.0, quality + 0.1)

        return max(0.0, min(1.0, quality))

    def generate_improvement_suggestions(self, code: str, target_language: str,
                                       validation_result: Dict[str, Any]) -> List[str]:
        """
        Generate suggestions for improving the code.

        Args:
            code: The generated code
            target_language: Target programming language
            validation_result: Validation results

        Returns:
            List of improvement suggestions
        """
        suggestions = []

        # Add validation suggestions
        suggestions.extend(validation_result.get('suggestions', []))

        # Add language-specific suggestions
        if target_language.lower() == 'python':
            suggestions.extend(self._get_python_suggestions(code))
        elif target_language.lower() in ['java', 'javascript']:
            suggestions.extend(self._get_c_like_suggestions(code))

        return list(set(suggestions))  # Remove duplicates

    def _get_python_suggestions(self, code: str) -> List[str]:
        """Get Python-specific improvement suggestions."""
        suggestions = []

        if 'print' in code and 'import sys' not in code:
            suggestions.append("Consider using logging module instead of print for production code")

        if re.search(r'len\s*\(\s*\w+\s*\)', code):
            suggestions.append("Consider using more Pythonic alternatives to len() checks")

        return suggestions

    def _get_c_like_suggestions(self, code: str) -> List[str]:
        """Get C-like language improvement suggestions."""
        suggestions = []

        if 'var ' in code:  # JavaScript
            suggestions.append("Consider using 'let' or 'const' instead of 'var'")

        return suggestions