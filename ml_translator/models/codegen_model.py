"""
CodeGen Model Wrapper for Code Translation

This module provides a wrapper around the Salesforce CodeGen model
for C++ to multiple target language translation.
"""

import os
import re
import logging
from typing import Dict, Any, Optional, List
from .base_model import BaseTranslationModel

try:
    import torch
    from transformers import AutoTokenizer, AutoModelForCausalLM
    TRANSFORMERS_AVAILABLE = True
except ImportError:
    TRANSFORMERS_AVAILABLE = False
    logging.warning("transformers library not available. CodeGen model will use fallback mode.")

logger = logging.getLogger(__name__)

class CodeGenModel(BaseTranslationModel):
    """
    Wrapper for Salesforce CodeGen model for code translation.

    This model provides translation capabilities from C++ to:
    - Python
    - Java
    - JavaScript
    - Assembly (limited support)
    """

    def __init__(self, model_name: str = "codegen-350m-multi"):
        super().__init__(model_name)
        self.model = None
        self.tokenizer = None
        self.device = "cuda" if torch.cuda.is_available() else "cpu"
        self.max_length = 1024
        self.temperature = 0.7
        self.top_p = 0.95
        self.top_k = 50

        # Language-specific prompts
        self.translation_prompts = {
            "python": "Translate the following C++ code to Python:\n\nC++:\n{code}\n\nPython:\n",
            "java": "Translate the following C++ code to Java:\n\nC++:\n{code}\n\nJava:\n",
            "javascript": "Translate the following C++ code to JavaScript:\n\nC++:\n{code}\n\nJavaScript:\n",
            "assembly": "Convert the following C++ code to x86 assembly:\n\nC++:\n{code}\n\nAssembly:\n"
        }

        # Initialize in fallback mode if transformers not available
        if not TRANSFORMERS_AVAILABLE:
            self._init_fallback_mode()
        else:
            self.model_path = self._get_model_path()

    def _get_model_path(self) -> str:
        """Get the appropriate model path based on available options."""
        # Prefer local model if available, otherwise use HuggingFace
        local_path = os.path.join(os.path.dirname(__file__), "..", "..", "models", "codegen")
        if os.path.exists(local_path):
            logger.info(f"Using local model: {local_path}")
            return local_path

        # Use HuggingFace model
        hf_models = {
            "codegen-350m-multi": "Salesforce/codegen-350M-multi",
            "codegen-2b-multi": "Salesforce/codegen-2B-multi",
            "codegen-6b-multi": "Salesforce/codegen-6B-multi"
        }

        model_key = self.model_name if self.model_name in hf_models else "codegen-350m-multi"
        hf_path = hf_models[model_key]
        logger.info(f"Using HuggingFace model: {hf_path}")
        return hf_path

    def _init_fallback_mode(self):
        """Initialize fallback mode for when transformers is not available."""
        logger.warning("Initializing CodeGen model in fallback mode (rule-based)")
        self.fallback_mode = True
        self.is_initialized = True

    def load_model(self, model_path: Optional[str] = None) -> bool:
        """Load the CodeGen model and tokenizer."""
        if not TRANSFORMERS_AVAILABLE:
            logger.info("Using fallback mode - no model loading required")
            return True

        try:
            logger.info(f"Loading CodeGen model on {self.device}...")

            # Use provided path or default
            path = model_path or self.model_path

            # Load tokenizer
            self.tokenizer = AutoTokenizer.from_pretrained(path)
            if self.tokenizer.pad_token is None:
                self.tokenizer.pad_token = self.tokenizer.eos_token

            # Load model
            self.model = AutoModelForCausalLM.from_pretrained(
                path,
                torch_dtype=torch.float16 if self.device == "cuda" else torch.float32,
                device_map="auto" if self.device == "cuda" else None,
                low_cpu_mem_usage=True
            )

            if self.device == "cpu":
                self.model = self.model.to(self.device)

            self.model.eval()
            self.is_initialized = True
            logger.info("CodeGen model loaded successfully")
            return True

        except Exception as e:
            logger.error(f"Failed to load CodeGen model: {e}")
            logger.info("Falling back to rule-based translation")
            self._init_fallback_mode()
            return True

    def translate(self, source_code: str, target_language: str) -> str:
        """Translate C++ code to target language."""
        if not self.is_initialized:
            raise RuntimeError("Model not initialized. Call load_model() first.")

        # Validate target language
        target_language = target_language.lower()
        if target_language not in self.get_supported_languages():
            raise ValueError(f"Unsupported target language: {target_language}")

        # Use fallback mode if available
        if hasattr(self, 'fallback_mode') and self.fallback_mode:
            return self._fallback_translate(source_code, target_language)

        # Use ML model
        return self._ml_translate(source_code, target_language)

    def _ml_translate(self, source_code: str, target_language: str) -> str:
        """Translate using the actual ML model."""
        try:
            # Create prompt
            prompt = self.translation_prompts[target_language].format(code=source_code.strip())

            # Tokenize input
            inputs = self.tokenizer.encode(prompt, return_tensors="pt", truncation=True, max_length=self.max_length).to(self.device)

            # Generate translation
            with torch.no_grad():
                outputs = self.model.generate(
                    inputs,
                    max_new_tokens=min(512, self.max_length - len(inputs[0])),
                    temperature=self.temperature,
                    top_p=self.top_p,
                    top_k=self.top_k,
                    do_sample=True,
                    pad_token_id=self.tokenizer.eos_token_id,
                    num_return_sequences=1
                )

            # Decode the generated text
            generated_text = self.tokenizer.decode(outputs[0], skip_special_tokens=True)

            # Extract the translated code
            translated = self._extract_translation(generated_text, target_language)

            # Set a reasonable confidence score
            self.set_confidence(0.85)

            return translated

        except Exception as e:
            logger.error(f"ML translation failed: {e}")
            # Fallback to rule-based translation
            return self._fallback_translate(source_code, target_language)

    def _fallback_translate(self, source_code: str, target_language: str) -> str:
        """Fallback rule-based translation."""
        logger.info(f"Using fallback rule-based translation to {target_language}")
        self.set_confidence(0.6)  # Lower confidence for fallback

        if target_language == "python":
            return self._cpp_to_python_fallback(source_code)
        elif target_language == "java":
            return self._cpp_to_java_fallback(source_code)
        elif target_language == "javascript":
            return self._cpp_to_javascript_fallback(source_code)
        elif target_language == "assembly":
            return self._cpp_to_assembly_fallback(source_code)
        else:
            raise ValueError(f"Unsupported target language: {target_language}")

    def _extract_translation(self, generated_text: str, target_language: str) -> str:
        """Extract the actual translated code from generated text."""
        # Look for the language label and extract everything after it
        language_labels = {
            "python": ["Python:", "Python code:", "Output:"],
            "java": ["Java:", "Java code:", "Output:"],
            "javascript": ["JavaScript:", "JavaScript code:", "JS:", "Output:"],
            "assembly": ["Assembly:", "Assembly code:", "ASM:", "Output:"]
        }

        labels = language_labels.get(target_language, [f"{target_language.capitalize}:"])

        for label in labels:
            if label in generated_text:
                # Find the label and extract everything after it
                parts = generated_text.split(label, 1)
                if len(parts) > 1:
                    translated = parts[1].strip()
                    # Remove any remaining prompt artifacts
                    lines = translated.split('\n')
                    clean_lines = []
                    for line in lines:
                        line = line.strip()
                        if line and not line.startswith('C++:') and not line.startswith('Input:'):
                            clean_lines.append(line)
                    return '\n'.join(clean_lines)

        # If no clear separation found, return the latter part of the text
        lines = generated_text.split('\n')
        mid_point = len(lines) // 2
        return '\n'.join(lines[mid_point:]).strip()

    def _cpp_to_python_fallback(self, cpp_code: str) -> str:
        """Simple C++ to Python fallback translation."""
        # Basic variable declarations
        code = re.sub(r'int\s+(\w+)\s*=\s*(\d+);', r'\1 = \2', cpp_code)
        code = re.sub(r'float\s+(\w+)\s*=\s*([\d.]+);', r'\1 = \2', cpp_code)
        code = re.sub(r'double\s+(\w+)\s*=\s*([\d.]+);', r'\1 = \2', cpp_code)
        code = re.sub(r'char\s+(\w+)\s*=\s*\'([^\']*)\';', r'\1 = "\2"', cpp_code)
        code = re.sub(r'bool\s+(\w+)\s*=\s*(true|false);', r'\1 = \2', cpp_code)

        # Basic I/O
        code = re.sub(r'cout\s*<<\s*(.*?);', r'print(\1)', code)
        code = re.sub(r'cin\s*>>\s*(\w+);', r'\1 = input("Enter \1: ")', code)

        # Remove semicolons
        code = re.sub(r';', '', code)

        # Add comments about limitations
        return f"# Fallback translation (limited functionality)\n{code}"

    def _cpp_to_java_fallback(self, src_code: str) -> str:
        """Simple C++ to Java fallback translation."""
        # Add class wrapper and main method
        lines = src_code.strip().split('\n')
        method_lines = []

        for line in lines:
            if 'int main()' in line or 'void main()' in line:
                method_lines.append('    public static void main(String[] args) {')
            elif line.strip() == 'return 0;':
                continue  # Skip return in Java main
            else:
                method_lines.append('    ' + line)

        # Simple variable type conversions
        method_code = '\n'.join(method_lines)
        method_code = re.sub(r'int\s+(\w+);', r'int \1;', method_code)

        return f"public class TranslatedProgram {{\n{method_code}\n}}"

    def _cpp_to_javascript_fallback(self, src_code: str) -> str:
        """Simple C++ to JavaScript fallback translation."""
        code = src_code
        # Variable declarations (let/const instead of types)
        code = re.sub(r'int\s+(const\s+)?(\w+)', r'let \2', code)
        code = re.sub(r'float\s+(const\s+)?(\w+)', r'let \2', code)
        code = re.sub(r'double\s+(const\s+)?(\w+)', r'let \2', code)

        # I/O
        code = re.sub(r'cout\s*<<\s*(.*?);', r'console.log(\1);', code)
        code = re.sub(r'cin\s*>>\s*(\w+);', r'\1 = prompt("Enter value:");', code)

        # Remove semicolons (JavaScript is more lenient)
        return f"// Fallback JavaScript translation\n{code}"

    def _cpp_to_assembly_fallback(self, src_code: str) -> str:
        """Very limited C++ to assembly fallback."""
        return f"; Fallback assembly translation\n; Limited C++ to assembly conversion\n; Original C++ code:\n; {src_code}\nsection .text\n    global _start\n_start:\n    ; Minimal assembly stub\n    mov eax, 1      ; sys_exit\n    mov ebx, 0      ; exit code 0\n    int 0x80"

    def get_supported_languages(self) -> List[str]:
        """Get list of supported target languages."""
        return ["python", "java", "javascript", "assembly"]

    def get_model_info(self) -> Dict[str, Any]:
        """Get detailed information about the model."""
        info = super().get_model_info()
        info.update({
            "device": self.device,
            "max_length": self.max_length,
            "temperature": self.temperature,
            "transformers_available": TRANSFORMERS_AVAILABLE,
            "fallback_mode": getattr(self, 'fallback_mode', False)
        })

        if self.model is not None:
            info.update({
                "model_parameters": sum(p.numel() for p in self.model.parameters()),
                "model_type": type(self.model).__name__
            })

        return info