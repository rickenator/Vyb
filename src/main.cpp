#include "vyn/vyn.hpp"
#include "vyn/parser/lexer.hpp"   // For Lexer
#include "vyn/parser/parser.hpp"  // For vyn::Parser
#include <catch2/catch_session.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <set> // For test and parser verbosity specifiers
#include <algorithm> // For std::find
#include <cstdio> // For printf and fflush

// Declare the intrinsic functions from intrinsics.cpp
extern "C" {
    // This is just a declaration - implementation is in intrinsics.cpp
    void __vyn_println(const char* str);
    char* __vyn_serialize_to_json(void* obj, const char* type_name);
}

// LLVM includes for JIT compilation
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h> // Include for LLVMLinkInInterpreter
#include <llvm/ExecutionEngine/MCJIT.h>       // Include for LLVMLinkInMCJIT
#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

// Globals for test verbose control
std::set<std::string> g_verbose_test_specifiers;
bool g_make_all_tests_verbose = false;
bool g_suppress_all_debug_output = false;

// Globals for parser verbose control
namespace vyn {
    std::set<std::string> g_verbose_parser_test_specifiers;
    bool g_make_all_parser_verbose = false;
    bool g_suppress_all_parser_debug_output = false;
}

// Concrete implementation of SemanticAnalyzer
class SemanticAnalyzerImpl : public vyn::SemanticAnalyzer {
public:
    SemanticAnalyzerImpl(vyn::Driver& driver) : vyn::SemanticAnalyzer(driver) {}
    
    // Implement all pure virtual methods
    void visit(vyn::ast::BorrowExpression* node) override {}
    void visit(vyn::ast::IfExpression* node) override {}
    void visit(vyn::ast::ArrayInitializationExpression* node) override {}
    void visit(vyn::ast::TupleTypeNode* node) override {}
    void visit(vyn::ast::ExternStatement* node) override {}
    void visit(vyn::ast::YieldStatement* node) override {}
    void visit(vyn::ast::YieldReturnStatement* node) override {}
    void visit(vyn::ast::LogicalExpression* node) override {}
    void visit(vyn::ast::ConditionalExpression* node) override {}
    void visit(vyn::ast::SequenceExpression* node) override {}
    void visit(vyn::ast::FunctionExpression* node) override {}
    void visit(vyn::ast::ThisExpression* node) override {}
    void visit(vyn::ast::SuperExpression* node) override {}
    void visit(vyn::ast::AwaitExpression* node) override {}
    void visit(vyn::ast::MatchStatement* node) override {}
    void visit(vyn::ast::TraitDeclaration* node) override {}
    void visit(vyn::ast::NamespaceDeclaration* node) override {}
    void visit(vyn::ast::TypeName* node) override {}
    void visit(vyn::ast::PointerType* node) override {}
    void visit(vyn::ast::ArrayType* node) override {}
    void visit(vyn::ast::FunctionType* node) override {}
    void visit(vyn::ast::OptionalType* node) override {}
    void visit(vyn::ast::AssertStatement* node) override {}
    void visit(vyn::ast::ConstructionExpression* node) override {}
    void visit(vyn::ast::ThrowStatement* node) override {}
    void visit(vyn::ast::FieldDeclaration* node) override {}
    void visit(vyn::ast::EnumVariant* node) override {}
    void visit(vyn::ast::TemplateDeclaration* node) override {}
    void visit(vyn::ast::TypeNode* node) override {}
};

// Concrete implementation of LLVMCodegen
class LLVMCodegenImpl : public vyn::LLVMCodegen {
public:
    LLVMCodegenImpl(vyn::Driver& driver) : vyn::LLVMCodegen(driver) {}
    
    // Implement all pure virtual methods
    void visit(vyn::ast::ExternStatement* node) override {}
    void visit(vyn::ast::YieldStatement* node) override {}
    void visit(vyn::ast::YieldReturnStatement* node) override {}
    void visit(vyn::ast::BorrowExpression* node) override {}
    void visit(vyn::ast::IfExpression* node) override {}
    void visit(vyn::ast::ArrayInitializationExpression* node) override {}
    void visit(vyn::ast::TupleTypeNode* node) override {}
    void visit(vyn::ast::LogicalExpression* node) override {}
    void visit(vyn::ast::ConditionalExpression* node) override {}
    void visit(vyn::ast::SequenceExpression* node) override {}
    void visit(vyn::ast::FunctionExpression* node) override {}
    void visit(vyn::ast::ThisExpression* node) override {}
    void visit(vyn::ast::SuperExpression* node) override {}
    void visit(vyn::ast::AwaitExpression* node) override {}
    void visit(vyn::ast::EmptyStatement* node) override {}
    void visit(vyn::ast::ThrowStatement* node) override {}
    void visit(vyn::ast::MatchStatement* node) override {}
    void visit(vyn::ast::AssertStatement* node) override {}
    void visit(vyn::ast::TraitDeclaration* node) override {}
    void visit(vyn::ast::NamespaceDeclaration* node) override {}
    void visit(vyn::ast::TypeName* node) override {}
    void visit(vyn::ast::PointerType* node) override {}
    void visit(vyn::ast::ArrayType* node) override {}
    void visit(vyn::ast::FunctionType* node) override {}
    void visit(vyn::ast::OptionalType* node) override {}
    void visit(vyn::ast::ListComprehension* node) override {}
    void visit(vyn::ast::FieldDeclaration* node) override {}
    void visit(vyn::ast::EnumVariant* node) override {}
    void visit(vyn::ast::TemplateDeclaration* node) override {}
    void visit(vyn::ast::TypeNode* node) override {}
};

// Function to execute Vyn code using LLVM JIT
int run_vyn_code(const std::string& source, const std::string& fileName, bool generateLLVMIR) {
    std::cout << "Starting run_vyn_code for file: " << fileName << std::endl;
    
    // Initialize LLVM targets for JIT
    std::cout << "Initializing LLVM targets..." << std::endl;
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    // Explicitly initialize the required components for JIT execution
    std::cout << "Setting up JIT options..." << std::endl;
    try {
        // Try manually linking the required components
        LLVMLinkInMCJIT();
        LLVMLinkInInterpreter();
        std::cout << "LLVM components linked successfully." << std::endl;
    } catch(const std::exception& e) {
        std::cerr << "Error linking LLVM components: " << e.what() << std::endl;
    }

    try {
        std::cout << "Creating driver instance..." << std::endl;
        vyn::Driver driver;

        std::cout << "Tokenizing source code..." << std::endl;
        Lexer lexer(source, fileName);
        std::vector<vyn::token::Token> tokens = lexer.tokenize();
        std::cout << "Tokens generated: " << tokens.size() << " tokens" << std::endl;

        std::cout << "Parsing tokens into AST..." << std::endl;
        vyn::Parser parser(tokens, fileName);
        auto ast = parser.parse_module();
        if (!ast) {
            throw std::runtime_error("Failed to parse source code");
        }
        std::cout << "AST created successfully" << std::endl;

        std::cout << "Running semantic analysis..." << std::endl;
        SemanticAnalyzerImpl semanticAnalyzer(driver);
        semanticAnalyzer.analyze(ast.get());
        std::cout << "Semantic analysis completed" << std::endl;

        std::cout << "Generating LLVM IR code..." << std::endl;
        LLVMCodegenImpl codegen(driver);
        codegen.generate(ast.get(), fileName + ".ll");
        std::cout << "LLVM IR generation completed" << std::endl;

        if (generateLLVMIR) {
            // Generate LLVM IR to a file if requested
            std::string irFilename = fileName + ".ll";
            std::error_code EC;
            llvm::raw_fd_ostream irFile(irFilename, EC);
            if (EC) {
                throw std::runtime_error("Failed to open file for IR output: " + EC.message());
            }
            codegen.getModule()->print(irFile, nullptr);
            irFile.flush();
            std::cout << "Generated LLVM IR to " << irFilename << std::endl;
        }

        // Get the LLVM module from the code generator
        std::unique_ptr<llvm::Module> module = codegen.releaseModule();
        
        std::cout << "Setting up execution engine..." << std::endl;
        std::string errStr;
        
        // Print module information
        std::cout << "Module name: " << module->getName().str() << std::endl;
        unsigned int funcCount = 0;
        for(const auto& f : *module) {
            funcCount++;
        }
        std::cout << "Function count: " << funcCount << std::endl;
        
        // Print all functions in the module
        std::cout << "Functions in module:" << std::endl;
        for (auto &F : module->functions()) {
            std::cout << "  - " << F.getName().str() << std::endl;
        }
        
        // Check functions before moving the module
        llvm::Function* printlnFunc = module->getFunction("__vyn_println");
        llvm::Function* serializeFunc = module->getFunction("__vyn_serialize_to_json");
        
        // Explicitly link in the interpreter and MCJIT
        std::cout << "Linking Interpreter and MCJIT..." << std::endl;
        LLVMLinkInInterpreter();
        LLVMLinkInMCJIT();
        
        std::cout << "Creating execution engine..." << std::endl;
        llvm::ExecutionEngine* executionEngine = 
            llvm::EngineBuilder(std::move(module))
            .setErrorStr(&errStr)
            .create();
        
        if (!executionEngine) {
            throw std::runtime_error("Failed to create execution engine: " + errStr);
        }
        
        // Register our intrinsic functions with the execution engine using the function pointers we saved
        if (printlnFunc) {
            executionEngine->addGlobalMapping(printlnFunc, reinterpret_cast<void*>(__vyn_println));
            std::cout << "Successfully registered __vyn_println function" << std::endl;
        } else {
            std::cout << "Warning: __vyn_println function not found in module" << std::endl;
        }
        
        // Also register the serialization function
        if (serializeFunc) {
            executionEngine->addGlobalMapping(serializeFunc, reinterpret_cast<void*>(__vyn_serialize_to_json));
            std::cout << "Successfully registered __vyn_serialize_to_json function" << std::endl;
        } else {
            std::cout << "Warning: __vyn_serialize_to_json function not found in module" << std::endl;
        }
        
        std::cout << "Execution engine created successfully" << std::endl;
        
        std::cout << "Finding main function..." << std::endl;
        llvm::Function* mainFunc = executionEngine->FindFunctionNamed("main");
        if (!mainFunc) {
            throw std::runtime_error("No 'main' function found in the program");
        }
        std::cout << "Found main function" << std::endl;
        
        // Execute the main function
        std::vector<llvm::GenericValue> noArgs;
        llvm::GenericValue result = executionEngine->runFunction(mainFunc, noArgs);
        
        // Clean up
        delete executionEngine;
        
        // Return the integer result
        return result.IntVal.getSExtValue();
    } catch (const std::exception& e) {
        std::cerr << "Error running Vyn code: " << e.what() << std::endl;
        throw; // Re-throw the exception to allow calling code to handle errors
    }
}

int main(int argc, char* argv[]) {
    Catch::Session session; // Catch2 entry point

    std::vector<std::string> catch_args;
    catch_args.push_back(argv[0]); // Program name

    bool next_arg_is_test_specifier_for_verbose = false;
    bool test_mode_active = false;
    bool parse_only_mode = false;
    bool semantic_only_mode = false;
    bool emit_llvm_ir = false;
    bool execute_jit = true;  // By default, execute the code with JIT

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test") {
            test_mode_active = true;
            // Enter test mode for Catch2; do not forward our own flag
            continue;
        } else if (arg == "--parse-only") {
            parse_only_mode = true;
            execute_jit = false;  // Don't execute if parse-only
            continue;
        } else if (arg == "--semantic-only") {
            semantic_only_mode = true;
            execute_jit = false;  // Don't execute if semantic-only
            continue;
        } else if (arg == "--emit-llvm") {
            emit_llvm_ir = true;
            continue;
        } else if (arg == "--no-execute") {
            execute_jit = false;  // Explicitly disable JIT execution
            continue;
        } else if (arg == "--debug-verbose") {
            if (i + 1 < argc) {
                std::string specifiers_str = argv[++i];
                if (specifiers_str == "all") {
                    g_make_all_tests_verbose = true;
                } else {
                    // Parse comma-separated specifiers
                    size_t start = 0;
                    size_t end = specifiers_str.find(',');
                    while (end != std::string::npos) {
                        g_verbose_test_specifiers.insert(specifiers_str.substr(start, end - start));
                        start = end + 1;
                        end = specifiers_str.find(',', start);
                    }
                    g_verbose_test_specifiers.insert(specifiers_str.substr(start));
                }
            } else {
                std::cerr << "Warning: --debug-verbose requires an argument (e.g., \"all\" or test_name,[tag])." << std::endl;
            }
        } else if (arg == "--no-debug-output") {
            g_suppress_all_debug_output = true;
        } else if (arg == "--debug-parser-verbose") {
            if (i + 1 < argc) {
                std::string spec_str = argv[++i];
                if (spec_str == "all") {
                    vyn::g_make_all_parser_verbose = true;
                } else {
                    size_t start = 0;
                    size_t end = spec_str.find(',');
                    while (end != std::string::npos) {
                        vyn::g_verbose_parser_test_specifiers.insert(spec_str.substr(start, end - start));
                        start = end + 1;
                        end = spec_str.find(',', start);
                    }
                    vyn::g_verbose_parser_test_specifiers.insert(spec_str.substr(start));
                }
            } else {
                std::cerr << "Warning: --debug-parser-verbose requires an argument." << std::endl;
            }
        } else if (arg == "--no-parser-debug-output") {
            vyn::g_suppress_all_parser_debug_output = true;
        }
        else {
            // If in test mode, or it\'s a general Catch2 arg, pass it along
             catch_args.push_back(arg);
        }
    }

    if (!test_mode_active && (g_make_all_tests_verbose || !g_verbose_test_specifiers.empty() || g_suppress_all_debug_output ||
                              vyn::g_make_all_parser_verbose || !vyn::g_verbose_parser_test_specifiers.empty() || vyn::g_suppress_all_parser_debug_output)) {
         std::cerr << "Warning: Debug verbosity flags (--debug-verbose, --no-debug-output, --debug-parser-verbose, --no-parser-debug-output) are intended for use with --test mode." << std::endl;
    }
    
    // Convert std::vector<std::string> to char* array for Catch2
    std::vector<char*> C_catch_args;
    for(const auto& s : catch_args) {
        C_catch_args.push_back(const_cast<char*>(s.c_str()));
    }

    int result = session.run(C_catch_args.size(), C_catch_args.data());

    // If tests were run, we might want to exit here.
    if (test_mode_active) {
        return result;
    }

    // If not in test mode, proceed with original file processing logic
    if (argc > 1) {
        // Find the file name to process (skip options)
        std::string filename;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            // Skip known option flags and their arguments
            if (arg == "--debug-verbose" || arg == "--debug-parser-verbose" || arg == "--emit-llvm") {
                i++; // Skip the next argument for debug flags, skip for emit-llvm
                continue;
            }
            // Not an option (doesn't start with --), assume it's the file
            if (arg.substr(0, 2) != "--") {
                filename = arg;
                break;
            }
        }
        
        if (filename.empty()) {
            std::cerr << "Error: No input file specified" << std::endl;
            return 1;
        }            std::cout << "Processing file: " << filename << std::endl;
        try {
            // Read the source file
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file " << filename << std::endl;
                return 1;
            }
            std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            // In parse-only mode, just tokenize and parse
            if (parse_only_mode) {
                Lexer lexer(source, filename);
                auto tokens = lexer.tokenize();
                
                // Optional: Print tokens if verbose mode is enabled
                if (g_make_all_tests_verbose || !g_verbose_test_specifiers.empty()) {
                    std::cout << "Tokenization results:" << std::endl;
                    for (const auto& token : tokens) {
                        std::cout << vyn::token_type_to_string(token.type) << " (" << token.lexeme << ") at " 
                                << token.location.filePath << ":" << token.location.line << ":" << token.location.column << std::endl;
                    }
                }

                vyn::Parser parser(tokens, filename);
                std::unique_ptr<vyn::ast::Module> ast = parser.parse_module();
                
                std::cout << "Parse completed successfully" << std::endl;
                return 0;
            }
            
            // Generate LLVM IR to a file if requested
            if (emit_llvm_ir) {
                Lexer lexer(source, filename);
                auto tokens = lexer.tokenize();
                vyn::Parser parser(tokens, filename);
                auto ast = parser.parse_module();
                
                vyn::Driver driver;
                vyn::LLVMCodegen codegen(driver);
                
                // Output file: <input>.ll
                std::string out_ll = filename;
                size_t dot = out_ll.find_last_of('.');
                if (dot != std::string::npos) out_ll = out_ll.substr(0, dot);
                out_ll += ".ll";
                
                codegen.generate(ast.get(), out_ll);
                std::cout << "LLVM IR generated to " << out_ll << std::endl;
                return 0;
            }
            
            // In semantic-only mode, run semantic analysis without execution
            if (semantic_only_mode) {
                Lexer lexer(source, filename);
                auto tokens = lexer.tokenize();
                vyn::Parser parser(tokens, filename);
                auto ast = parser.parse_module();
                
                vyn::Driver driver;
                SemanticAnalyzerImpl semanticAnalyzer(driver);
                semanticAnalyzer.analyze(ast.get());
                
                std::cout << "Semantic analysis completed successfully" << std::endl;
                return 0;
            }
            
            // Default behavior: JIT compile and execute the code
            if (execute_jit) {
                try {
                    std::cout << "Starting JIT execution of " << filename << std::endl;
                    int result = run_vyn_code(source, filename, emit_llvm_ir);
                    std::cout << "Program execution completed with return code: " << result << std::endl;
                    return result;
                } catch (const std::exception& e) {
                    std::cerr << "Error during code execution: " << e.what() << std::endl;
                    return 1;
                }
            }


        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            return 1;
        }
    } else {
        std::cout << "Vyn Compiler - Usage: " << argv[0] << " <filename> [options] | --test [catch2_options]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --parse-only          Stop after parsing (validates syntax only)" << std::endl;
        std::cout << "  --semantic-only       Stop after semantic analysis" << std::endl;
        std::cout << "  --emit-llvm           Generate LLVM IR to a .ll file" << std::endl;
        std::cout << "  --no-execute          Do not execute the code (JIT is on by default)" << std::endl;
        std::cout << std::endl;
        std::cout << "Test Mode Options:" << std::endl;
        std::cout << "  --test                Run test suite" << std::endl;
        std::cout << "  --debug-verbose <all|test_name,[tag],...]> Enable verbose output for tests" << std::endl;
        std::cout << "  --no-debug-output     Suppress all debug output" << std::endl;
        std::cout << "  --debug-parser-verbose <all|test_name,[tag],...]> Enable verbose parser output" << std::endl;
        std::cout << "  --no-parser-debug-output Suppress parser debug output" << std::endl;
    }

    return result; // Or 0 if not running tests and successful
}