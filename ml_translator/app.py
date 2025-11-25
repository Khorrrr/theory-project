#!/usr/bin/env python3
"""
Flask Web Server for ML Code Translation

This server provides HTTP endpoints for machine learning-based code translation
from C++ to multiple target languages using pre-trained models.
"""

import os
import sys
import json
import logging
from flask import Flask, request, jsonify
from flask_cors import CORS
import traceback

# Add the current directory to Python path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    from models.codegen_model import CodeGenModel
    from preprocessing.tokenizer import CodeTokenizer
    from postprocessing.formatter import CodeFormatter
    from postprocessing.validator import CodeValidator
except ImportError as e:
    print(f"Import error: {e}")
    print("Make sure all dependencies are installed: pip install -r requirements.txt")
    sys.exit(1)

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Initialize Flask app
app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Global variables for models
translator = None
tokenizer = None
formatter = None
validator = None

def initialize_models():
    """Initialize ML models and preprocessing components"""
    global translator, tokenizer, formatter, validator

    try:
        logger.info("Initializing ML components...")

        # Initialize tokenizer
        tokenizer = CodeTokenizer()
        logger.info("Tokenizer initialized")

        # Initialize translator
        translator = CodeGenModel()
        logger.info("CodeGen model initialized")

        # Initialize formatter
        formatter = CodeFormatter()
        logger.info("Code formatter initialized")

        # Initialize validator
        validator = CodeValidator()
        logger.info("Code validator initialized")

        logger.info("All ML components initialized successfully")
        return True

    except Exception as e:
        logger.error(f"Failed to initialize ML components: {e}")
        logger.error(traceback.format_exc())
        return False

def format_error_response(message, details=None, status_code=500):
    """Format error response consistently"""
    response = {
        "error": message,
        "success": False
    }
    if details:
        response["details"] = details

    return jsonify(response), status_code

def validate_translation_request(data):
    """Validate translation request data"""
    errors = []

    if not data:
        errors.append("Request body is empty")
        return errors

    if "source_code" not in data or not data["source_code"].strip():
        errors.append("source_code is required and cannot be empty")

    if "target_language" not in data or not data["target_language"].strip():
        errors.append("target_language is required and cannot be empty")
    else:
        valid_languages = ["python", "java", "javascript", "assembly"]
        if data["target_language"].lower() not in valid_languages:
            errors.append(f"target_language must be one of: {', '.join(valid_languages)}")

    # Tokens are optional but useful for context
    if "tokens" in data and not isinstance(data["tokens"], list):
        errors.append("tokens must be a list if provided")

    return errors

@app.route('/health', methods=['GET'])
def health_check():
    """Health check endpoint to verify server status"""
    try:
        status = {
            "status": "healthy",
            "models_loaded": translator is not None,
            "server": "ml-translator",
            "version": "1.0.0"
        }
        return jsonify(status), 200
    except Exception as e:
        logger.error(f"Health check failed: {e}")
        return jsonify({
            "status": "unhealthy",
            "error": str(e),
            "models_loaded": False
        }), 500

@app.route('/translate', methods=['POST'])
def translate_code():
    """Main translation endpoint"""
    try:
        # Validate request
        data = request.get_json()
        if not data:
            return format_error_response("Invalid JSON request", status_code=400)

        validation_errors = validate_translation_request(data)
        if validation_errors:
            return format_error_response("Validation failed", "; ".join(validation_errors), 400)

        # Extract request data
        source_code = data["source_code"]
        target_language = data["target_language"].lower()
        tokens = data.get("tokens", [])

        logger.info(f"Translation request: C++ -> {target_language}")
        logger.debug(f"Source code length: {len(source_code)} characters")
        logger.debug(f"Tokens provided: {len(tokens)} tokens")

        # Check if models are initialized
        if not translator or not tokenizer:
            return format_error_response(
                "ML models not initialized",
                "Server may still be starting up. Please try again in a moment.",
                503
            )

        # Preprocess the code
        logger.info("Preprocessing source code...")
        processed_input = tokenizer.preprocess(source_code, tokens)

        # Perform ML translation
        logger.info(f"Translating to {target_language}...")
        translated = translator.translate(processed_input, target_language)

        if not translated:
            return format_error_response(
                "Translation failed",
                "ML model returned empty result",
                500
            )

        # Postprocess the result
        logger.info("Postprocessing translation result...")
        formatted_output = formatter.format_code(translated, target_language)

        # Validate the output (optional)
        validation_result = validator.validate(formatted_output, target_language)

        # Prepare response
        response = {
            "translated_code": formatted_output,
            "confidence": getattr(translator, 'last_confidence', 0.0),
            "model_used": getattr(translator, 'model_name', 'codegen-default'),
            "validation": validation_result,
            "success": True
        }

        logger.info(f"Translation completed successfully (confidence: {response['confidence']:.2f})")
        return jsonify(response), 200

    except json.JSONDecodeError:
        return format_error_response("Invalid JSON format", status_code=400)
    except Exception as e:
        logger.error(f"Translation error: {e}")
        logger.error(traceback.format_exc())

        # Don't expose internal error details to client in production
        error_msg = "Internal server error during translation"
        if app.debug:
            error_msg = f"Translation error: {str(e)}"

        return format_error_response(error_msg, status_code=500)

@app.route('/models/info', methods=['GET'])
def models_info():
    """Get information about available models"""
    try:
        info = {
            "available_models": [],
            "current_model": None,
            "supported_languages": ["python", "java", "javascript", "assembly"]
        }

        if translator:
            info["current_model"] = getattr(translator, 'model_name', 'unknown')
            info["model_loaded"] = True
            info["model_details"] = translator.get_model_info()
        else:
            info["model_loaded"] = False

        return jsonify(info), 200

    except Exception as e:
        logger.error(f"Models info error: {e}")
        return format_error_response("Failed to get models information")

@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors"""
    return format_error_response("Endpoint not found", status_code=404)

@app.errorhandler(405)
def method_not_allowed(error):
    """Handle 405 errors"""
    return format_error_response("Method not allowed", status_code=405)

@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors"""
    logger.error(f"Internal server error: {error}")
    return format_error_response("Internal server error", status_code=500)

def main():
    """Main function to start the server"""
    import argparse

    parser = argparse.ArgumentParser(description='ML Code Translation Server')
    parser.add_argument('--host', default='127.0.0.1', help='Host to bind to')
    parser.add_argument('--port', type=int, default=5000, help='Port to bind to')
    parser.add_argument('--debug', action='store_true', help='Enable debug mode')
    parser.add_argument('--model', default='codegen-350m', help='Model to use for translation')

    args = parser.parse_args()

    logger.info("Starting ML Code Translation Server...")
    logger.info(f"Server will run on http://{args.host}:{args.port}")

    # Initialize models
    if not initialize_models():
        logger.error("Failed to initialize models. Exiting.")
        sys.exit(1)

    try:
        # Start Flask server
        app.run(
            host=args.host,
            port=args.port,
            debug=args.debug,
            threaded=True
        )
    except KeyboardInterrupt:
        logger.info("Server stopped by user")
    except Exception as e:
        logger.error(f"Server error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()