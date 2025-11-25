"""
AST Processor for Code Analysis

This module provides AST (Abstract Syntax Tree) processing capabilities
for analyzing code structure and extracting semantic information.
"""

import ast
import re
import logging
from typing import Dict, List, Any, Optional, Union
from enum import Enum

logger = logging.getLogger(__name__)

class NodeType(Enum):
    """Enumeration of AST node types."""
    FUNCTION = "function"
    CLASS = "class"
    VARIABLE = "variable"
    IMPORT = "import"
    CONTROL_FLOW = "control_flow"
    EXPRESSION = "expression"
    LITERAL = "literal"
    COMMENT = "comment"

class ASTProcessor:
    """
    Processes AST of source code to extract structural information.
    """

    def __init__(self):
        self.supported_languages = ['python', 'cpp', 'java', 'javascript']
        self.cpp_keywords = {
            'int', 'float', 'double', 'char', 'bool', 'void', 'if', 'else', 'for', 'while',
            'do', 'switch', 'case', 'break', 'continue', 'return', 'class', 'struct', 'public',
            'private', 'protected', 'static', 'const', 'virtual', 'override'
        }

    def parse_code(self, code: str, language: str) -> Dict[str, Any]:
        """
        Parse source code and extract AST information.

        Args:
            code: Source code to parse
            language: Programming language

        Returns:
            Dict containing AST information
        """
        if language == 'python':
            return self._parse_python(code)
        else:
            # For other languages, use regex-based parsing
            return self._parse_generic(code, language)

    def _parse_python(self, code: str) -> Dict[str, Any]:
        """Parse Python code using built-in AST."""
        try:
            tree = ast.parse(code)
            return self._extract_python_ast_info(tree)
        except SyntaxError as e:
            logger.warning(f"Python syntax error: {e}")
            return self._parse_generic(code, 'python')

    def _extract_python_ast_info(self, tree: ast.AST) -> Dict[str, Any]:
        """Extract information from Python AST."""
        info = {
            'language': 'python',
            'functions': [],
            'classes': [],
            'variables': [],
            'imports': [],
            'control_flow': [],
            'complexity_metrics': {},
            'structure': self._build_structure_map(tree)
        }

        for node in ast.walk(tree):
            if isinstance(node, ast.FunctionDef):
                info['functions'].append(self._extract_function_info(node, 'python'))
            elif isinstance(node, ast.ClassDef):
                info['classes'].append(self._extract_class_info(node, 'python'))
            elif isinstance(node, ast.Import):
                for alias in node.names:
                    info['imports'].append({'name': alias.name, 'alias': alias.asname})
            elif isinstance(node, ast.ImportFrom):
                module = node.module or ''
                for alias in node.names:
                    info['imports'].append({
                        'name': f"{module}.{alias.name}",
                        'alias': alias.asname,
                        'from_module': module
                    })
            elif isinstance(node, (ast.If, ast.For, ast.While, ast.Try)):
                info['control_flow'].append({
                    'type': type(node).__name__,
                    'line': getattr(node, 'lineno', 0)
                })

        # Calculate complexity metrics
        info['complexity_metrics'] = self._calculate_complexity_metrics(info)

        return info

    def _extract_function_info(self, node, language: str) -> Dict[str, Any]:
        """Extract information about a function."""
        info = {
            'name': getattr(node, 'name', 'unknown'),
            'line': getattr(node, 'lineno', 0),
            'parameters': [],
            'return_type': None,
            'is_method': False,
            'class_name': None,
            'complexity': 1  # Base complexity
        }

        if language == 'python':
            # Extract parameters
            for arg in node.args.args:
                info['parameters'].append({
                    'name': arg.arg,
                    'type': None,  # Python doesn't have explicit type hints in AST
                    'default': None
                })

            # Check if it's a method
            if hasattr(node, 'parent_class'):
                info['is_method'] = True
                info['class_name'] = node.parent_class

        elif language in ['cpp', 'java']:
            # For C++/Java, we'd need to parse from text
            # This is a simplified implementation
            pass

        return info

    def _extract_class_info(self, node, language: str) -> Dict[str, Any]:
        """Extract information about a class."""
        info = {
            'name': getattr(node, 'name', 'unknown'),
            'line': getattr(node, 'lineno', 0),
            'methods': [],
            'properties': [],
            'inheritance': [],
            'is_abstract': False
        }

        if language == 'python':
            # Extract methods
            for item in node.body:
                if isinstance(item, ast.FunctionDef):
                    method_info = self._extract_function_info(item, language)
                    method_info['class_name'] = info['name']
                    method_info['is_method'] = True
                    info['methods'].append(method_info)

            # Check for inheritance
            for base in node.bases:
                if isinstance(base, ast.Name):
                    info['inheritance'].append(base.id)

        return info

    def _parse_generic(self, code: str, language: str) -> Dict[str, Any]:
        """
        Parse non-Python languages using regex-based approach.
        This is a simplified parser for demonstration.
        """
        info = {
            'language': language,
            'functions': [],
            'classes': [],
            'variables': [],
            'imports': [],
            'control_flow': [],
            'complexity_metrics': {},
            'warnings': ['Using simplified parser - results may be limited']
        }

        lines = code.split('\n')

        if language in ['cpp', 'java']:
            info = self._parse_cpp_java_like(code, lines, language)
        elif language == 'javascript':
            info = self._parse_javascript(code, lines)

        info['complexity_metrics'] = self._calculate_complexity_metrics(info)

        return info

    def _parse_cpp_java_like(self, code: str, lines: List[str], language: str) -> Dict[str, Any]:
        """Parse C++/Java-like languages."""
        info = {
            'language': language,
            'functions': [],
            'classes': [],
            'variables': [],
            'imports': [],
            'control_flow': [],
            'complexity_metrics': {},
            'warnings': []
        }

        in_function = False
        in_class = False
        brace_level = 0
        current_function = None
        current_class = None

        for i, line in enumerate(lines, 1):
            line = line.strip()

            # Skip empty lines and comments
            if not line or line.startswith('//') or line.startswith('/*'):
                continue

            # Extract includes/imports
            if language == 'cpp' and line.startswith('#include'):
                info['imports'].append({
                    'name': line,
                    'type': 'include',
                    'line': i
                })

            # Extract class definitions
            class_match = re.search(r'(class|struct)\s+(\w+)', line)
            if class_match and not in_class:
                class_name = class_match.group(2)
                current_class = {
                    'name': class_name,
                    'line': i,
                    'methods': [],
                    'properties': []
                }
                info['classes'].append(current_class)
                in_class = True

            # Extract function definitions
            func_pattern = r'(\w+\s+)*(\w+)\s*\([^)]*\)\s*(\{|;)'
            func_match = re.search(func_pattern, line)
            if func_match and not in_function:
                func_name = func_match.group(2)
                if func_name not in self.cpp_keywords or func_name in ['operator']:
                    current_function = {
                        'name': func_name,
                        'line': i,
                        'parameters': [],
                        'return_type': func_match.group(1).strip() if func_match.group(1) else 'void',
                        'is_method': current_class is not None,
                        'class_name': current_class['name'] if current_class else None
                    }

                    # Extract parameters (simplified)
                    param_str = re.search(r'\(([^)]*)\)', line)
                    if param_str:
                        params = param_str.group(1).split(',')
                        for param in params:
                            param = param.strip()
                            if param:
                                parts = param.split()
                                param_info = {'name': parts[-1], 'type': ' '.join(parts[:-1]) if len(parts) > 1 else 'auto'}
                                current_function['parameters'].append(param_info)

                    info['functions'].append(current_function)
                    in_function = True

            # Track brace levels for scope
            brace_level += line.count('{') - line.count('}')

            if in_class and brace_level == 0:
                in_class = False
                current_class = None

            if in_function and brace_level == 0:
                in_function = False
                current_function = None

        return info

    def _parse_javascript(self, code: str, lines: List[str]) -> Dict[str, Any]:
        """Parse JavaScript code."""
        info = {
            'language': 'javascript',
            'functions': [],
            'classes': [],
            'variables': [],
            'imports': [],
            'control_flow': [],
            'complexity_metrics': {},
            'warnings': []
        }

        for i, line in enumerate(lines, 1):
            line = line.strip()

            # Skip empty lines and comments
            if not line or line.startswith('//') or line.startswith('/*'):
                continue

            # Extract imports/require
            if line.startswith('import ') or line.startswith('const ') and 'require' in line:
                info['imports'].append({
                    'name': line,
                    'type': 'import',
                    'line': i
                })

            # Extract function definitions
            func_patterns = [
                r'function\s+(\w+)\s*\(',
                r'const\s+(\w+)\s*=\s*\([^)]*\)\s*=>',
                r'(\w+)\s*:\s*function\s*\(',
                r'async\s+function\s+(\w+)\s*\('
            ]

            for pattern in func_patterns:
                match = re.search(pattern, line)
                if match:
                    info['functions'].append({
                        'name': match.group(1),
                        'line': i,
                        'type': 'function',
                        'parameters': [],
                        'is_async': 'async' in line
                    })
                    break

            # Extract class definitions
            class_match = re.search(r'class\s+(\w+)', line)
            if class_match:
                info['classes'].append({
                    'name': class_match.group(1),
                    'line': i,
                    'methods': [],
                    'properties': []
                })

        return info

    def _build_structure_map(self, tree: ast.AST) -> List[Dict[str, Any]]:
        """Build a hierarchical structure map of the code."""
        structure = []
        # This would build a tree structure representing the code
        # Simplified implementation for now
        return structure

    def _calculate_complexity_metrics(self, info: Dict[str, Any]) -> Dict[str, Any]:
        """Calculate code complexity metrics."""
        metrics = {
            'cyclomatic_complexity': 1,  # Base complexity
            'function_count': len(info.get('functions', [])),
            'class_count': len(info.get('classes', [])),
            'nesting_depth': 0,
            'parameter_count': 0,
            'control_flow_count': len(info.get('control_flow', []))
        }

        # Calculate parameter count
        for func in info.get('functions', []):
            metrics['parameter_count'] += len(func.get('parameters', []))

        # Add complexity for control flow
        metrics['cyclomatic_complexity'] += metrics['control_flow_count']

        # Add complexity for functions and classes
        metrics['cyclomatic_complexity'] += metrics['function_count'] + metrics['class_count']

        return metrics

    def extract_dependencies(self, code: str, language: str) -> List[Dict[str, Any]]:
        """Extract dependency information from code."""
        dependencies = []

        if language == 'python':
            try:
                tree = ast.parse(code)
                for node in ast.walk(tree):
                    if isinstance(node, ast.Import):
                        for alias in node.names:
                            dependencies.append({
                                'name': alias.name,
                                'type': 'import',
                                'alias': alias.asname
                            })
                    elif isinstance(node, ast.ImportFrom):
                        dependencies.append({
                            'name': node.module or '',
                            'type': 'from_import',
                            'aliases': [alias.name for alias in node.names]
                        })
            except SyntaxError:
                pass

        return dependencies

    def find_code_smells(self, info: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Find potential code smells based on AST analysis."""
        smells = []

        # Long functions
        for func in info.get('functions', []):
            if func.get('complexity', 1) > 10:
                smells.append({
                    'type': 'high_complexity',
                    'location': f"function {func.get('name', 'unknown')} at line {func.get('line', 0)}",
                    'severity': 'warning'
                })

        # Too many parameters
        for func in info.get('functions', []):
            if len(func.get('parameters', [])) > 7:
                smells.append({
                    'type': 'too_many_parameters',
                    'location': f"function {func.get('name', 'unknown')} at line {func.get('line', 0)}",
                    'severity': 'info'
                })

        return smells