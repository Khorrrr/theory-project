"""
Configuration settings for ML Code Translator

This module contains configuration constants and settings for the
ML code translation system.
"""

import os
from typing import Dict, Any, List

class Config:
    """Configuration class for ML translation system."""

    # Server Configuration
    HOST = os.getenv('ML_HOST', '127.0.0.1')
    PORT = int(os.getenv('ML_PORT', 5000))
    DEBUG = os.getenv('ML_DEBUG', 'false').lower() == 'true'

    # Model Configuration
    DEFAULT_MODEL = os.getenv('ML_DEFAULT_MODEL', 'codegen-350m-multi')
    MODEL_CACHE_DIR = os.getenv('ML_MODEL_CACHE', './models')
    MAX_MODEL_SIZE_GB = 8  # Maximum model size to download

    # Translation Configuration
    MAX_SOURCE_LENGTH = 2048  # Maximum source code length
    MAX_TARGET_LENGTH = 1024  # Maximum translated code length
    TEMPERATURE = 0.7
    TOP_P = 0.95
    TOP_K = 50
    NUM_RETURN_SEQUENCES = 1

    # Performance Configuration
    REQUEST_TIMEOUT = 30  # seconds
    MAX_CONCURRENT_REQUESTS = 5
    BATCH_SIZE = 1  # For batch processing

    # Supported Languages
    SUPPORTED_LANGUAGES: List[str] = ['python', 'java', 'javascript', 'assembly']
    DEFAULT_LANGUAGE = 'python'

    # HuggingFace Model Mappings
    HUGGINGFACE_MODELS: Dict[str, str] = {
        'codegen-350m-multi': 'Salesforce/codegen-350M-multi',
        'codegen-2b-multi': 'Salesforce/codegen-2B-multi',
        'codegen-6b-multi': 'Salesforce/codegen-6B-multi',
        'codegen-16b-multi': 'Salesforce/codegen-16B-multi'
    }

    # Model-specific configurations
    MODEL_CONFIGS: Dict[str, Dict[str, Any]] = {
        'codegen-350m-multi': {
            'max_length': 1024,
            'temperature': 0.7,
            'top_p': 0.95,
            'top_k': 50,
            'requires_gpu': False,
            'estimated_size_gb': 2
        },
        'codegen-2b-multi': {
            'max_length': 1024,
            'temperature': 0.6,
            'top_p': 0.9,
            'top_k': 40,
            'requires_gpu': True,
            'estimated_size_gb': 4
        },
        'codegen-6b-multi': {
            'max_length': 2048,
            'temperature': 0.6,
            'top_p': 0.9,
            'top_k': 40,
            'requires_gpu': True,
            'estimated_size_gb': 6
        }
    }

    # Logging Configuration
    LOG_LEVEL = os.getenv('ML_LOG_LEVEL', 'INFO')
    LOG_FORMAT = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    LOG_FILE = os.getenv('ML_LOG_FILE', 'ml_translator.log')

    # API Configuration
    API_PREFIX = '/api/v1'
    CORS_ORIGINS = ['*']  # Configure for production
    RATE_LIMIT = '100 per hour'

    # Translation Prompts
    TRANSLATION_PROMPTS: Dict[str, str] = {
        'python': 'Translate the following C++ code to Python:\n\nC++:\n{source}\n\nPython:\n',
        'java': 'Translate the following C++ code to Java:\n\nC++:\n{source}\n\nJava:\n',
        'javascript': 'Translate the following C++ code to JavaScript:\n\nC++:\n{source}\n\nJavaScript:\n',
        'assembly': 'Convert the following C++ code to x86 assembly:\n\nC++:\n{source}\n\nAssembly:\n'
    }

    # Quality Thresholds
    MIN_CONFIDENCE_THRESHOLD = 0.5
    MAX_ERRORS_ALLOWED = 3
    MAX_WARNINGS_ALLOWED = 10

    # Fallback Configuration
    ENABLE_FALLBACK = True
    FALLBACK_CONFIDENCE = 0.6  # Confidence for rule-based fallback

    @classmethod
    def get_model_config(cls, model_name: str) -> Dict[str, Any]:
        """Get configuration for a specific model."""
        return cls.MODEL_CONFIGS.get(model_name, cls.MODEL_CONFIGS['codegen-350m-multi'])

    @classmethod
    def get_huggingface_model(cls, model_name: str) -> str:
        """Get HuggingFace model name."""
        return cls.HUGGINGFACE_MODELS.get(model_name, cls.HUGGINGFACE_MODELS['codegen-350m-multi'])

    @classmethod
    def is_model_supported(cls, model_name: str) -> bool:
        """Check if a model is supported."""
        return model_name in cls.HUGGINGFACE_MODELS

    @classmethod
    def get_translation_prompt(cls, target_language: str) -> str:
        """Get translation prompt for target language."""
        return cls.TRANSLATION_PROMPTS.get(target_language.lower(), cls.TRANSLATION_PROMPTS['python'])

    @classmethod
    def validate_config(cls) -> List[str]:
        """Validate configuration and return any issues."""
        issues = []

        # Check model availability
        if cls.DEFAULT_MODEL not in cls.HUGGINGFACE_MODELS:
            issues.append(f"Default model '{cls.DEFAULT_MODEL}' is not supported")

        # Check port validity
        if not (1 <= cls.PORT <= 65535):
            issues.append(f"Invalid port number: {cls.PORT}")

        # Check model cache directory
        try:
            os.makedirs(cls.MODEL_CACHE_DIR, exist_ok=True)
        except Exception as e:
            issues.append(f"Cannot create model cache directory: {e}")

        return issues

class DevelopmentConfig(Config):
    """Development configuration overrides."""
    DEBUG = True
    LOG_LEVEL = 'DEBUG'
    REQUEST_TIMEOUT = 60  # Longer timeout for debugging

class ProductionConfig(Config):
    """Production configuration overrides."""
    DEBUG = False
    LOG_LEVEL = 'WARNING'
    CORS_ORIGINS = ['http://localhost:8080', 'http://127.0.0.1:8080']  # Qt app
    REQUEST_TIMEOUT = 30

def get_config() -> Config:
    """Get appropriate configuration based on environment."""
    env = os.getenv('ML_ENV', 'development')

    if env == 'production':
        return ProductionConfig()
    elif env == 'development':
        return DevelopmentConfig()
    else:
        return Config()