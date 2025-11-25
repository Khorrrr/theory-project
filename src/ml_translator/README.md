# ML Code Translator

A machine learning-powered code translation system that integrates with the Theory of Computation project to provide C++ to multiple target language translation.

## Features

- **Neural Translation**: Uses Salesforce CodeGen models for high-quality code translation
- **Multiple Target Languages**: Python, Java, JavaScript, and Assembly support
- **Fallback Mode**: Rule-based translation when ML models are unavailable
- **REST API**: HTTP interface for easy integration with Qt application
- **Code Validation**: Post-processing and validation of translated code
- **Extensible Architecture**: Easy to add new models and target languages

## Setup

### Prerequisites

- Python 3.8 or higher
- pip package manager
- (Optional) CUDA-enabled GPU for faster inference

### Installation

1. **Navigate to the ML translator directory:**
   ```bash
   cd ml_translator
   ```

2. **Install Python dependencies:**
   ```bash
   pip install -r requirements.txt
   ```

3. **Verify installation:**
   ```bash
   python start_server.py --check-only
   ```

### Running the Server

#### Development Mode
```bash
python start_server.py --debug
```

#### Production Mode
```bash
python start_server.py --port 5000
```

#### Custom Model
```bash
python start_server.py --model codegen-2b-multi
```

## API Endpoints

### Health Check
```
GET /health
```

Response:
```json
{
  "status": "healthy",
  "models_loaded": true,
  "server": "ml-translator",
  "version": "1.0.0"
}
```

### Translate Code
```
POST /translate
```

Request:
```json
{
  "source_code": "int x = 10; cout << x;",
  "target_language": "python",
  "tokens": [
    {"type": "INT", "value": "int", "line": 1, "column": 1},
    {"type": "IDENTIFIER", "value": "x", "line": 1, "column": 5}
  ]
}
```

Response:
```json
{
  "translated_code": "x = 10\nprint(x)",
  "confidence": 0.92,
  "model_used": "codegen-350m-multi",
  "validation": {
    "is_valid": true,
    "errors": [],
    "warnings": []
  },
  "success": true
}
```

### Model Information
```
GET /models/info
```

Response:
```json
{
  "available_models": ["codegen-350m-multi", "codegen-2b-multi"],
  "current_model": "codegen-350m-multi",
  "model_loaded": true,
  "supported_languages": ["python", "java", "javascript", "assembly"],
  "model_details": {
    "device": "cpu",
    "parameters": 350000000,
    "fallback_mode": false
  }
}
```

## Supported Models

| Model | Size | GPU Required | Description |
|-------|------|--------------|-------------|
| codegen-350m-multi | 350M | No | Good balance of speed and quality |
| codegen-2b-multi | 2B | Recommended | Higher quality translations |
| codegen-6b-multi | 6B | Recommended | Best quality for complex code |

## Configuration

Configuration can be set via environment variables:

```bash
export ML_HOST=127.0.0.1
export ML_PORT=5000
export ML_DEFAULT_MODEL=codegen-350m-multi
export ML_DEBUG=false
export ML_LOG_LEVEL=INFO
```

## Integration with Qt Application

The ML translator integrates with the Qt application through the `MLTranslationBridge` class:

1. **Start the ML server** before running the Qt application
2. **Select "ML Translation"** radio button in the semantic analyzer tab
3. **Translate code** using the existing translation button

The bridge handles:
- HTTP communication with the Python server
- Error handling and fallback to rule-based translation
- Status updates and progress indication

## Troubleshooting

### Common Issues

1. **Server won't start**:
   - Check Python version: `python --version`
   - Install dependencies: `pip install -r requirements.txt`
   - Check port availability: `netstat -an | grep 5000`

2. **Model loading fails**:
   - Check internet connection for model download
   - Verify disk space (models are 2-6GB)
   - Try smaller model: `--model codegen-350m-multi`

3. **Translation errors**:
   - Check server logs for error messages
   - Verify input format is correct
   - Try simpler code examples

4. **GPU not detected**:
   - Install CUDA toolkit if using NVIDIA GPU
   - Verify PyTorch CUDA support: `python -c "import torch; print(torch.cuda.is_available())"`

### Debug Mode

Run with debug logging:
```bash
python start_server.py --debug --log-level DEBUG
```

### Logs

Server logs are written to:
- Console (default)
- File: `ml_translator.log` (when `--log-file` is specified)

## Development

### Adding New Models

1. Create a new model class in `models/` directory
2. Implement the `BaseTranslationModel` interface
3. Register the model in `config.py`
4. Add model-specific configuration

### Adding New Languages

1. Add language to `SUPPORTED_LANGUAGES` in `config.py`
2. Add translation prompt in `TRANSLATION_PROMPTS`
3. Add language-specific validation in `validator.py`
4. Add formatting rules in `formatter.py`

### Testing

Run tests:
```bash
python -m pytest tests/
```

## Performance

### CPU vs GPU

- **CPU**: 2-5 seconds per translation (350M model)
- **GPU**: 0.5-2 seconds per translation (350M model)

### Optimization Tips

- Use appropriate model size for your hardware
- Enable batch processing for multiple translations
- Consider model quantization for faster inference

## Security

- Input validation and sanitization
- Request rate limiting
- No code execution (translation only)
- Error message sanitization

## License

This ML translator component is part of the Theory of Computation project. Please refer to the main project license for terms and conditions.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## Support

For issues and questions:
- Check the troubleshooting section
- Review server logs for error messages
- Create an issue in the project repository