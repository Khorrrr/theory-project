"""
ML Models for Code Translation

This package contains various models for code translation including:
- CodeGen model wrapper
- T5-based translation models
- GPT-style models
"""

from .base_model import BaseTranslationModel
from .codegen_model import CodeGenModel

__all__ = ['BaseTranslationModel', 'CodeGenModel']