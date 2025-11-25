#!/usr/bin/env python3
"""
Startup script for the ML Code Translation Server

This script provides an easy way to start the translation server
with proper configuration and error handling.
"""

import sys
import os
import argparse
import logging
import time
import signal
from pathlib import Path

# Add the current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    from app import app, initialize_models
    from config import get_config, Config
    from models.codegen_model import TRANSFORMERS_AVAILABLE
except ImportError as e:
    print(f"Import error: {e}")
    print("Please install dependencies: pip install -r requirements.txt")
    sys.exit(1)

# Configure logging
def setup_logging(log_level: str, log_file: str = None):
    """Setup logging configuration."""
    level = getattr(logging, log_level.upper(), logging.INFO)

    # Create formatter
    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )

    # Setup root logger
    root_logger = logging.getLogger()
    root_logger.setLevel(level)

    # Console handler
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(level)
    console_handler.setFormatter(formatter)
    root_logger.addHandler(console_handler)

    # File handler (if specified)
    if log_file:
        file_handler = logging.FileHandler(log_file)
        file_handler.setLevel(level)
        file_handler.setFormatter(formatter)
        root_logger.addHandler(file_handler)

def check_dependencies():
    """Check if all required dependencies are available."""
    issues = []

    if not TRANSFORMERS_AVAILABLE:
        issues.append("transformers library not available - will use fallback mode")

    # Check for torch
    try:
        import torch
        if torch.cuda.is_available():
            print("✓ CUDA available - GPU acceleration enabled")
        else:
            print("ℹ CUDA not available - will use CPU (slower)")
    except ImportError:
        issues.append("torch library not available")

    # Check for Flask
    try:
        import flask
        print("✓ Flask available")
    except ImportError:
        issues.append("Flask not available")

    return issues

def validate_configuration():
    """Validate the configuration."""
    config = get_config()
    issues = config.validate_config()

    if issues:
        print("Configuration issues found:")
        for issue in issues:
            print(f"  ❌ {issue}")
        return False

    print("✓ Configuration validated")
    return True

def initialize_translation_system():
    """Initialize the ML translation system."""
    print("Initializing ML translation system...")

    if not initialize_models():
        print("❌ Failed to initialize ML models")
        return False

    print("✓ ML translation system initialized successfully")
    return True

def print_startup_info(host: str, port: int, model_name: str):
    """Print startup information."""
    print("\n" + "="*60)
    print("🤖 ML Code Translation Server")
    print("="*60)
    print(f"📡 Server running at: http://{host}:{port}")
    print(f"🧠 Model: {model_name}")
    print(f"🔧 Health check: http://{host}:{port}/health")
    print(f"📚 API docs: http://{host}:{port}/models/info")
    print("\n📋 Available endpoints:")
    print("  GET  /health              - Server health check")
    print("  POST /translate           - Translate code")
    print("  GET  /models/info         - Model information")
    print("\n💡 Press Ctrl+C to stop the server")
    print("="*60)

def signal_handler(signum, frame):
    """Handle shutdown signals."""
    print("\n\n🛑 Shutting down ML translation server...")
    sys.exit(0)

def main():
    """Main startup function."""
    parser = argparse.ArgumentParser(
        description='Start the ML Code Translation Server',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python start_server.py                          # Start with defaults
  python start_server.py --port 8080              # Custom port
  python start_server.py --model codegen-2b-multi  # Custom model
  python start_server.py --debug                  # Debug mode
        """
    )

    parser.add_argument(
        '--host',
        default='127.0.0.1',
        help='Host to bind to (default: 127.0.0.1)'
    )

    parser.add_argument(
        '--port',
        type=int,
        default=5000,
        help='Port to bind to (default: 5000)'
    )

    parser.add_argument(
        '--model',
        default='codegen-350m-multi',
        choices=['codegen-350m-multi', 'codegen-2b-multi', 'codegen-6b-multi'],
        help='Model to use for translation (default: codegen-350m-multi)'
    )

    parser.add_argument(
        '--debug',
        action='store_true',
        help='Enable debug mode'
    )

    parser.add_argument(
        '--log-level',
        choices=['DEBUG', 'INFO', 'WARNING', 'ERROR'],
        default='INFO',
        help='Logging level (default: INFO)'
    )

    parser.add_argument(
        '--log-file',
        help='Log file path (default: stdout only)'
    )

    parser.add_argument(
        '--check-only',
        action='store_true',
        help='Only check dependencies and configuration, do not start server'
    )

    args = parser.parse_args()

    # Setup logging
    setup_logging(args.log_level, args.log_file)

    # Set up signal handler
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    print("🔍 Checking dependencies...")
    dep_issues = check_dependencies()
    if dep_issues:
        print("⚠️  Dependency issues:")
        for issue in dep_issues:
            print(f"   {issue}")
        print("   (Server will still start with limited functionality)")

    print("\n🔧 Validating configuration...")
    if not validate_configuration():
        sys.exit(1)

    if args.check_only:
        print("\n✅ All checks passed. Ready to start server.")
        sys.exit(0)

    print("\n🚀 Initializing translation system...")
    if not initialize_translation_system():
        print("⚠️  Server will start with fallback mode only")

    # Override config with command line arguments
    config = get_config()
    config.HOST = args.host
    config.PORT = args.port
    config.DEFAULT_MODEL = args.model
    config.DEBUG = args.debug

    # Print startup information
    print_startup_info(config.HOST, config.PORT, config.DEFAULT_MODEL)

    try:
        # Start the Flask server
        app.run(
            host=config.HOST,
            port=config.PORT,
            debug=config.DEBUG,
            threaded=True,
            use_reloader=False  # Prevent duplicate initialization
        )
    except Exception as e:
        print(f"\n❌ Failed to start server: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()