#include <vyn/vre/llvm/codegen.hpp>
#include <llvm/IR/Value.h>
#include <vyn/parser/ast.hpp>

namespace vyn {

    void LLVMCodegen::visit(ast::EmptyStatement* node) {
        // TODO: Implement codegen for EmptyStatement
    }
    void LLVMCodegen::visit(ast::ThrowStatement* node) {
        logError(node->loc, "ThrowStatement codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::MatchStatement* node) {
        logError(node->loc, "MatchStatement codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::AssertStatement* node) {
        logError(node->loc, "AssertStatement codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::TraitDeclaration* node) {
        // TODO: Implement codegen for TraitDeclaration
    }
    void LLVMCodegen::visit(ast::NamespaceDeclaration* node) {
        // TODO: Implement codegen for NamespaceDeclaration
    }
    void LLVMCodegen::visit(ast::PointerType* node) {
        logError(node->loc, "PointerType codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::ArrayType* node) {
        logError(node->loc, "ArrayType codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::FunctionType* node) {
        logError(node->loc, "FunctionType codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::OptionalType* node) {
        logError(node->loc, "OptionalType codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::TupleTypeNode* node) {
        logError(node->loc, "TupleTypeNode codegen not implemented.");
        m_currentLLVMValue = nullptr;
    }
    void LLVMCodegen::visit(ast::LogicalExpression* node) {
        // TODO: Implement codegen for LogicalExpression
    }
    void LLVMCodegen::visit(ast::ConditionalExpression* node) {
        // TODO: Implement codegen for ConditionalExpression
    }
    void LLVMCodegen::visit(ast::SequenceExpression* node) {
        // TODO: Implement codegen for SequenceExpression
    }
    void LLVMCodegen::visit(ast::FunctionExpression* node) {
        // TODO: Implement codegen for FunctionExpression
    }
    void LLVMCodegen::visit(ast::ThisExpression* node) {
        // TODO: Implement codegen for ThisExpression
    }
    void LLVMCodegen::visit(ast::SuperExpression* node) {
        // TODO: Implement codegen for SuperExpression
    }
    void LLVMCodegen::visit(ast::AwaitExpression* node) {
        // TODO: Implement codegen for AwaitExpression
    }
    void LLVMCodegen::visit(ast::TypeName* node) {
        // TODO: Implement codegen for TypeName
    }
    void LLVMCodegen::visit(ast::ExternStatement* node) {
        // Implementation for ExternStatement to generate LLVM IR for external function declarations
        
        if (!node->name) {
            logError(node->loc, "External declaration missing name.");
            m_currentLLVMValue = nullptr;
            return;
        }

        std::vector<llvm::Type*> paramTypes;
        for (const auto& paramNode : node->parameters) {
            if (!paramNode.typeNode) {
                logError(paramNode.name->loc, "Parameter '" + paramNode.name->name + 
                         "' in external declaration '" + node->name->name + "' is missing a type annotation.");
                m_currentLLVMValue = nullptr; 
                return;
            }
            
            llvm::Type* llvmType = codegenType(paramNode.typeNode.get());
            if (!llvmType) {
                logError(paramNode.name->loc, "Could not determine LLVM type for parameter '" + 
                         paramNode.name->name + "' in external declaration '" + node->name->name + "'.");
                m_currentLLVMValue = nullptr; 
                return;
            }
            paramTypes.push_back(llvmType);
        }

        llvm::Type* returnType = nullptr;
        if (node->returnType) {
            returnType = codegenType(node->returnType.get());
            if (!returnType) {
                logError(node->loc, "Could not determine LLVM return type for external declaration '" + 
                         node->name->name + "'.");
                m_currentLLVMValue = nullptr; 
                return;
            }
        } else {
            returnType = llvm::Type::getVoidTy(*context);
        }
        
        llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false /*isVarArg*/);
        
        // Check for existing function
        llvm::Function* func = module->getFunction(node->name->name);
        if (func) {
            if (func->getFunctionType() != funcType) {
                logError(node->loc, "Redeclaration of external function '" + node->name->name + 
                         "' with different signature.");
                m_currentLLVMValue = nullptr; 
                return;
            }
            // Function already declared with matching signature, nothing more to do
        } else {
            // Create the external function declaration
            func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->name->name, module.get());
            
            // Set parameter names for better IR readability
            unsigned idx = 0;
            for (auto &arg : func->args()) {
                if (idx < node->parameters.size()) {
                    arg.setName(node->parameters[idx].name->name);
                }
                idx++;
            }
        }
        
        m_currentLLVMValue = func;
    }
    void LLVMCodegen::visit(ast::YieldStatement* node) {
        // Implementation for YieldStatement
        // Yield temporarily produces a value and suspends execution until the generator is resumed
        
        if (!getCurrentFunction()) {
            logError(node->loc, "Yield statement outside of function context.");
            m_currentLLVMValue = nullptr;
            return;
        }
        
        // For a basic implementation, we need to:
        // 1. Evaluate the expression to yield (if any)
        // 2. Save the current state of execution
        // 3. Create a suspension point
        
        llvm::Value* yieldValue = nullptr;
        if (node->expression) {
            // Visit the expression to get its value
            node->expression->accept(*this);
            yieldValue = m_currentLLVMValue;
            
            if (!yieldValue) {
                logError(node->expression->loc, "Failed to evaluate yield expression.");
                m_currentLLVMValue = nullptr;
                return;
            }
        } else {
            // If no expression provided, yield 'undefined' or a default value
            yieldValue = llvm::UndefValue::get(llvm::Type::getInt32Ty(*context));
        }
        
        // For now, we'll create a placeholder implementation that logs the yield
        // In a full implementation, this would involve coroutine transformation
        std::vector<llvm::Type*> paramTypes = { yieldValue->getType() };
        llvm::FunctionType* logYieldType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context), 
            paramTypes, 
            false
        );
        
        // Create or get the debug function for logging yields
        llvm::Function* logYieldFunc = module->getFunction("__vyn_debug_log_yield");
        if (!logYieldFunc) {
            logYieldFunc = llvm::Function::Create(
                logYieldType,
                llvm::Function::ExternalLinkage,
                "__vyn_debug_log_yield",
                module.get()
            );
        }
        
        // Call the debug function with our yield value
        std::vector<llvm::Value*> args = { yieldValue };
        builder->CreateCall(logYieldFunc, args);
        
        logWarning(node->loc, "YieldStatement partially implemented. Full generator functionality requires coroutine support.");
        
        m_currentLLVMValue = yieldValue;
    }
    void LLVMCodegen::visit(ast::YieldReturnStatement* node) {
        // Implementation for YieldReturnStatement
        // This represents the final return from a generator function
        
        if (!getCurrentFunction()) {
            logError(node->loc, "Yield return statement outside of function context.");
            m_currentLLVMValue = nullptr;
            return;
        }
        
        llvm::Value* returnValue = nullptr;
        llvm::Type* returnType = currentFunction->getReturnType();
        
        if (node->expression) {
            // Visit the expression to get its value
            node->expression->accept(*this);
            returnValue = m_currentLLVMValue;
            
            if (!returnValue) {
                logError(node->expression->loc, "Failed to evaluate yield return expression.");
                m_currentLLVMValue = nullptr;
                return;
            }
            
            // Check if the types match
            if (returnValue->getType() != returnType && !returnType->isVoidTy()) {
                // Try to cast the value to the return type
                returnValue = tryCast(returnValue, returnType, node->loc);
                if (!returnValue) {
                    logError(node->expression->loc, "Cannot convert yield return value to the function's return type.");
                    m_currentLLVMValue = nullptr;
                    return;
                }
            }
        } else if (!returnType->isVoidTy()) {
            // No expression provided but non-void return type required
            logError(node->loc, "Yield return statement missing expression for non-void return type.");
            m_currentLLVMValue = nullptr;
            return;
        }
        
        // Create a function for signaling generator completion
        std::vector<llvm::Type*> paramTypes;
        if (returnValue) {
            paramTypes.push_back(returnValue->getType());
        }
        
        llvm::FunctionType* completeGenType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context), 
            paramTypes, 
            false
        );
        
        // Create or get the debug function for generator completion
        llvm::Function* completeGenFunc = module->getFunction("__vyn_debug_complete_generator");
        if (!completeGenFunc) {
            completeGenFunc = llvm::Function::Create(
                completeGenType,
                llvm::Function::ExternalLinkage,
                "__vyn_debug_complete_generator",
                module.get()
            );
        }
        
        // Call the debug function
        std::vector<llvm::Value*> args;
        if (returnValue) {
            args.push_back(returnValue);
        }
        builder->CreateCall(completeGenFunc, args);
        
        // Add a normal return statement after the yield return
        if (returnType->isVoidTy()) {
            builder->CreateRetVoid();
        } else if (returnValue) {
            builder->CreateRet(returnValue);
        }
        
        logWarning(node->loc, "YieldReturnStatement partially implemented. Full generator functionality requires coroutine support.");
        
        m_currentLLVMValue = returnValue;
    }

} // namespace vyn
