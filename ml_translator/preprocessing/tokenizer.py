"""
Code Tokenizer for ML Models

This module provides tokenization and preprocessing utilities for code
that will be fed to machine learning models.
"""

import re
import logging
from typing import List, Dict, Any, Optional
import ast
import tokenize
import io

logger = logging.getLogger(__name__)

class CodeTokenizer:
    """
    Tokenizes and preprocesses source code for ML model consumption.
    """

    def __init__(self):
        # Language-specific comment patterns
        self.comment_patterns = {
            'cpp': r'//.*?$|/\*.*?\*/',
            'python': r'#.*?$|""".*?"""|\'\'\'.*?\'\'\'',
            'java': r'//.*?$|/\*.*?\*/',
            'javascript': r'//.*?$|/\*.*?\*/'
        }

        # Common code patterns
        self.string_patterns = [
            r'"([^"\\]|\\.)*"',  # Double quoted strings
            r"'([^'\\]|\\.)*'"   # Single quoted strings
        ]

        self.whitespace_pattern = r'\s+'

    def preprocess(self, source_code: str, tokens: Optional[List[Dict[str, Any]]] = None,
                   source_language: str = 'cpp') -> str:
        """
        Preprocess source code for ML model input.

        Args:
            source_code: Raw source code
            tokens: Optional list of tokens from lexer
            source_language: Source programming language

        Returns:
            str: Preprocessed code ready for ML model
        """
        if not source_code or not source_code.strip():
            return ""

        # Normalize line endings
        code = self._normalize_line_endings(source_code)

        # Remove excessive whitespace
        code = self._normalize_whitespace(code)

        # Handle comments (preserve some context)
        code = self._process_comments(code, source_language)

        # Normalize string literals
        code = self._normalize_strings(code)

        # If external tokens are provided, use them for enhanced preprocessing
        if tokens:
            code = self._enhance_with_tokens(code, tokens)

        # Add structural markers
        code = self._add_structural_markers(code)

        # Ensure the code ends with a newline
        if not code.endswith('\n'):
            code += '\n'

        return code

    def _normalize_line_endings(self, code: str) -> str:
        """Normalize different line ending styles."""
        return code.replace('\r\n', '\n').replace('\r', '\n')

    def _normalize_whitespace(self, code: str) -> str:
        """Normalize whitespace patterns."""
        # Replace tabs with spaces (4 spaces per tab)
        code = code.replace('\t', '    ')

        # Remove multiple consecutive empty lines
        code = re.sub(r'\n\s*\n\s*\n+', '\n\n', code)

        # Trim leading/trailing whitespace
        code = code.strip()

        return code

    def _process_comments(self, code: str, language: str) -> str:
        """Process comments - preserve important ones, remove noise."""
        if language not in self.comment_patterns:
            return code

        comment_pattern = self.comment_patterns[language]
        comment_regex = re.compile(comment_pattern, re.MULTILINE | re.DOTALL)

        def comment_replacer(match):
            comment = match.group(0)
            # Preserve TODO/FIXME/IMPORTANT comments
            if any(keyword in comment.upper() for keyword in ['TODO', 'FIXME', 'IMPORTANT', 'NOTE']):
                return comment
            # Replace other comments with minimal marker
            return ' /* comment */ '

        return comment_regex.sub(comment_replacer, code)

    def _normalize_strings(self, code: str) -> str:
        """Normalize string literals for consistent processing."""
        # This is a simplified approach - in production, you'd want more sophisticated handling
        for pattern in self.string_patterns:
            # Replace string contents with placeholder to avoid model confusion
            code = re.sub(pattern, '<STRING>', code)
        return code

    def _enhance_with_tokens(self, code: str, tokens: List[Dict[str, Any]]) -> str:
        """
        Enhance preprocessing using external token information.

        Args:
            code: Source code
            tokens: List of token dictionaries from the C++ lexer

        Returns:
            str: Enhanced code with token information
        """
        try:
            # Group tokens by line for better processing
            tokens_by_line = {}
            for token in tokens:
                line_num = token.get('line', 1)
                if line_num not in tokens_by_line:
                    tokens_by_line[line_num] = []
                tokens_by_line[line_num].append(token)

            # Add token type information as comments
            lines = code.split('\n')
            enhanced_lines = []

            for i, line in enumerate(lines, 1):
                enhanced_lines.append(line)

                # Add token information for this line if available
                if i in tokens_by_line:
                    line_tokens = tokens_by_line[i]
                    # Add a comment with token types for this line
                    token_info = ' '.join([f"{t.get('type', 'UNKNOWN')}" for t in line_tokens[:5]])  # Limit to first 5 tokens
                    if token_info:
                        enhanced_lines.append(f"// Tokens: {token_info}")

            return '\n'.join(enhanced_lines)

        except Exception as e:
            logger.warning(f"Failed to enhance with tokens: {e}")
            return code

    def _add_structural_markers(self, code: str) -> str:
        """Add structural markers to help the model understand code structure."""
        # Mark function boundaries
        code = re.sub(r'(\w+\s+\w+\s*\([^)]*\)\s*\{)', r'\n// FUNCTION_START\n\1', code)
        code = re.sub(r'(\})\s*(?=\n|$)', r'\1\n// FUNCTION_END\n', code)

        # Mark class/struct boundaries
        code = re.sub(r'(class\s+\w+\s*\{)', r'\n// CLASS_START\n\1', code)
        code = re.sub(r'(struct\s+\w+\s*\{)', r'\n// STRUCT_START\n\1', code)

        # Mark include statements
        code = re.sub(r'#include\s*[<"][^>"]*[>"]', r'// INCLUDE\n\g<0>', code)

        return code

    def extract_language_features(self, code: str, language: str = 'cpp') -> Dict[str, Any]:
        """
        Extract language-specific features from code.

        Args:
            code: Source code
            language: Programming language

        Returns:
            Dict containing language features
        """
        features = {
            'language': language,
            'line_count': len(code.split('\n')),
            'character_count': len(code),
            'token_estimate': len(code.split()),
            'has_comments': False,
            'has_strings': False,
            'has_functions': False,
            'has_classes': False,
            'complexity_indicators': {}
        }

        # Check for comments
        comment_pattern = self.comment_patterns.get(language, '')
        if comment_pattern and re.search(comment_pattern, code, re.MULTILINE | re.DOTALL):
            features['has_comments'] = True

        # Check for strings
        for pattern in self.string_patterns:
            if re.search(pattern, code):
                features['has_strings'] = True
                break

        # Language-specific feature detection
        if language == 'cpp' or language == 'java' or language == 'javascript':
            # Functions
            if re.search(r'\w+\s+\w+\s*\([^)]*\)\s*\{', code):
                features['has_functions'] = True

            # Classes/structs
            if re.search(r'(class|struct)\s+\w+', code):
                features['has_classes'] = True

        elif language == 'python':
            # Functions
            if re.search(r'def\s+\w+\s*\(', code):
                features['has_functions'] = True

            # Classes
            if re.search(r'class\s+\w+', code):
                features['has_classes'] = True

        # Complexity indicators
        features['complexity_indicators'] = {
            'nested_blocks': len(re.findall(r'\{[^{}]*\{', code)),
            'control_flow_count': len(re.findall(r'\b(if|else|for|while|switch|case)\b', code)),
            'function_calls': len(re.findall(r'\w+\s*\(', code))
        }

        return features

    def tokenize_for_analysis(self, code: str) -> List[str]:
        """
        Simple tokenization for analysis purposes.

        Args:
            code: Source code

        Returns:
            List of tokens
        """
        # Remove strings and comments first
        clean_code = code
        for pattern in self.string_patterns:
            clean_code = re.sub(pattern, '<STRING>', clean_code)

        # Split by whitespace and punctuation
        tokens = re.findall(r'\w+|[^\w\s]', clean_code)
        return tokens

    def validate_code_structure(self, code: str, language: str) -> Dict[str, Any]:
        """
        Validate basic code structure.

        Args:
            code: Source code
            language: Programming language

        Returns:
            Dict containing validation results
        """
        validation = {
            'is_valid': True,
            'errors': [],
            'warnings': [],
            'suggestions': []
        }

        # Basic checks
        if not code or not code.strip():
            validation['is_valid'] = False
            validation['errors'].append("Empty code provided")
            return validation

        # Check for unmatched braces/parentheses
        if language in ['cpp', 'java', 'javascript']:
            brace_count = code.count('{') - code.count('}')
            paren_count = code.count('(') - code.count(')')

            if brace_count != 0:
                validation['warnings'].append(f"Unmatched braces: {brace_count}")

            if paren_count != 0:
                validation['warnings'].append(f"Unmatched parentheses: {paren_count}")

        # Language-specific validation
        if language == 'python':
            try:
                ast.parse(code)
            except SyntaxError as e:
                validation['is_valid'] = False
                validation['errors'].append(f"Python syntax error: {e}")

        return validation