"""
Base class for all code translation models

This module defines the abstract interface that all translation models must implement.
"""

from abc import ABC, abstractmethod
from typing import Dict, Any, Optional

class BaseTranslationModel(ABC):
    """
    Abstract base class for code translation models.

    All concrete model implementations should inherit from this class
    and implement the required methods.
    """

    def __init__(self, model_name: str = "base-model"):
        self.model_name = model_name
        self.last_confidence = 0.0
        self.is_initialized = False

    @abstractmethod
    def load_model(self, model_path: Optional[str] = None) -> bool:
        """
        Load the translation model.

        Args:
            model_path: Optional path to the model files

        Returns:
            bool: True if model loaded successfully, False otherwise
        """
        pass

    @abstractmethod
    def translate(self, source_code: str, target_language: str) -> str:
        """
        Translate source code to target language.

        Args:
            source_code: Source code to translate
            target_language: Target programming language

        Returns:
            str: Translated code
        """
        pass

    @abstractmethod
    def get_supported_languages(self) -> list:
        """
        Get list of supported target languages.

        Returns:
            list: List of supported language codes
        """
        pass

    def get_model_info(self) -> Dict[str, Any]:
        """
        Get information about the model.

        Returns:
            Dict[str, Any]: Model information dictionary
        """
        return {
            "name": self.model_name,
            "initialized": self.is_initialized,
            "supported_languages": self.get_supported_languages(),
            "type": self.__class__.__name__
        }

    def set_confidence(self, confidence: float):
        """Set the confidence score of the last translation."""
        self.last_confidence = max(0.0, min(1.0, confidence))  # Clamp to [0, 1]

    def get_confidence(self) -> float:
        """Get the confidence score of the last translation."""
        return self.last_confidence