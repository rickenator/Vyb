// SPDX-License-Identifier: Apache-2.0
//
// Seam extracted from src/vre/semantic.cpp (#347): the member definitions below are moved
// VERBATIM -- no reformatting, no renames -- so the split stays behaviour-neutral.
// Everything that is file-local (static helpers, macros) travels with them or is
// promoted into the shared internal header, never duplicated.

#include "vyb/semantic.hpp"
#include "vyb/parser/token.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/driver.hpp"
#include "vyb/vre/semantic_internal.hpp"
#include "vyb/vre/thread_boundary.hpp"
#include <stdexcept>
#include <memory>
#include <unordered_set>
#include <string>
#include <map>
#include <set>
#include <functional>
#include <algorithm>

namespace vyb {

void SemanticAnalyzer::visit(ast::AspectDeclaration* node) {
    if (!node || !node->name) {
        addError("Malformed aspect declaration.", node);
        return;
    }

    const std::string& traitName = node->name->name;

    // Check if aspect is already registered
    if (traitRegistry.find(traitName) != traitRegistry.end()) {
        addError("Aspect '" + traitName + "' is already defined.", node);
        return;
    }

    // Validate generic parameters
    for (const auto& param : node->genericParams) {
        if (!param || !param->name) {
            addError("Invalid generic parameter in aspect '" + traitName + "'.", node);
            return;
        }
    }

    std::unordered_set<std::string> associatedTypeNames;
    for (const auto& associatedType : node->associatedTypes) {
        if (!associatedType) {
            addError("Invalid associated type in aspect '" + traitName + "'.", node);
            return;
        }

        const std::string& associatedTypeName = associatedType->name;
        if (!associatedTypeNames.insert(associatedTypeName).second) {
            addError("Duplicate associated type '" + associatedTypeName + "' in aspect '" + traitName + "'.", associatedType.get());
            return;
        }
    }

    // Validate aspect methods
    for (const auto& method : node->methods) {
        if (!method || !method->id) {
            addError("Invalid method in aspect '" + traitName + "'.", node);
            return;
        }

        // Check for self parameter
        bool hasSelfParam = false;
        if (!method->params.empty() && method->params[0].name) {
            if (method->params[0].name->name == "self") {
                hasSelfParam = true;
            }
        }

        if (!hasSelfParam) {
            addError("Aspect method '" + method->id->name + "' must have 'self' as first parameter.", method.get());
        }

        // Validate return type
        if (!method->returnTypeNode) {
            addError("Aspect method '" + method->id->name + "' must declare a return type.", method.get());
        }

        VYB_CDBG << "DEBUG:   Method: " << method->id->name
                  << " (default impl: " << (method->body ? "yes" : "no") << ")" << std::endl;
    }

    // Register the aspect
    registerTrait(node);

    // Register aspect as a type in the symbol table
    SymbolInfo traitSym;
    traitSym.name = traitName;
    traitSym.kind = SymbolInfo::Kind::Type;
    traitSym.type = nullptr; // Aspects are interface types, not concrete
    currentScope->add(traitSym);
}
void SemanticAnalyzer::visit(ast::BindDeclaration* node) {
    if (!node || !node->selfType) {
        addError("Malformed bind declaration.", node);
        return;
    }

    // Handle generic parameters if present (e.g., impl<T> ...)
    bool hasGenericParams = !node->genericParams.empty();
    std::vector<std::string> typeParamNames;

    if (hasGenericParams) {


        // Enter a new scope for type parameters
        enterScope();

        // Register each type parameter as a valid type in this scope
        for (const auto& param : node->genericParams) {
            if (param && param->name) {
                std::string paramName = param->name->name;
                typeParamNames.push_back(paramName);

                // Validate aspect bounds (if any)
                for (const auto& bound : param->bounds) {
                    if (bound) {
                        std::string boundName = bound->toString();
                        // Check that the bound is actually an aspect
                        if (!findTrait(boundName)) {
                            addError("Bound '" + boundName + "' on type parameter '" + paramName + "' is not a defined aspect.", param.get());
                        }
                    }
                }

                // Register the type parameter as a TYPE_PARAMETER symbol
                SymbolInfo typeParamSymbol;
                typeParamSymbol.name = paramName;
                typeParamSymbol.kind = SymbolInfo::Kind::TYPE_PARAMETER;
                typeParamSymbol.type = nullptr; // Generic type parameter has no concrete type yet

                // Store bounds for this type parameter
                for (const auto& bound : param->bounds) {
                    if (bound) {
                        typeParamSymbol.bounds.push_back(bound->toString());
                    }
                }

                currentScope->add(typeParamSymbol);

                VYB_CDBG << "DEBUG: Registered type parameter: " << paramName << std::endl;
            }
        }
    }

    std::string typeName = node->selfType->toString();
    std::string traitName;

    // Set current impl type for Self resolution (will be restored later)
    ast::TypeNode* previousImplType = currentImplType;
    currentImplType = node->selfType.get();
    VYB_CDBG << "DEBUG: Set currentImplType to " << typeName << " for Self resolution" << std::endl;

    if (node->traitType) {
        // This is an aspect implementation: bind Aspect -> Type
        traitName = node->traitType->toString();

        VYB_CDBG << "DEBUG: Processing impl " << traitName << " for " << typeName << std::endl;

        // Check if trait exists
        TraitInfo* traitInfo = findTrait(traitName);
        if (!traitInfo) {
            addError("Trait '" + traitName + "' is not defined.", node);
            if (hasGenericParams) exitScope();
            return;
        }

        // Check if type exists (for user-defined types)
        // Primitives like Int, Float, String are always valid
        bool isBuiltinType = (typeName == "Int" || typeName == "Float" ||
                             typeName == "Bool" || typeName == "String" ||
                             typeName == "Char" || typeName == "Rune");

        // For generic impls, the type might contain type parameters (e.g., Vec<T>)
        // which should be allowed if T is a type parameter
        bool isGenericType = false;
        if (hasGenericParams) {
            // Check if typeName contains any type parameters
            for (const auto& paramName : typeParamNames) {
                if (typeName.find(paramName) != std::string::npos) {
                    isGenericType = true;
                    VYB_CDBG << "DEBUG: Type " << typeName << " uses type parameter " << paramName << std::endl;
                    break;
                }
            }
        }

        bool isBuiltinGenericType = isBuiltinVecType(node->selfType.get());

        if (!isBuiltinType && !isBuiltinGenericType && !isGenericType) {
            SymbolInfo* typeSym = currentScope->lookup(typeName);
            if (!typeSym || typeSym->kind != SymbolInfo::Kind::Type) {
                // Allow binding to a concrete instantiation of a generic struct
                // (e.g. bind Display -> Box<Int>): resolve the base template and
                // validate the type-argument count against its generic parameters.
                bool resolvedInstantiation = false;
                auto* selfT = dynamic_cast<ast::TypeName*>(node->selfType.get());
                if (selfT && selfT->identifier) {
                    const std::string& baseId = selfT->identifier->name;
                    const std::vector<std::string>* paramOrder = nullptr;
                    auto structIt = structGenericParamOrder.find(baseId);
                    auto enumIt = enumGenericParamOrder.find(baseId);
                    if (structIt != structGenericParamOrder.end()) {
                        SymbolInfo* baseSym = currentScope->lookup(baseId);
                        if (baseSym && baseSym->kind == SymbolInfo::Kind::Type) {
                            resolvedInstantiation = true;
                            paramOrder = &structIt->second;
                        }
                    } else if (enumIt != enumGenericParamOrder.end()) {
                        // Generic enum bind target: a user-defined generic enum
                        // (e.g. bind ToStr -> Box<Int>) or the built-in generic
                        // enum (Result<T,E>).
                        if (enumIt->first == "Result") {
                            resolvedInstantiation = true;  // built-in: no scope symbol
                            paramOrder = &enumIt->second;
                        } else {
                            SymbolInfo* baseSym = currentScope->lookup(baseId);
                            if (baseSym && baseSym->kind == SymbolInfo::Kind::Type) {
                                resolvedInstantiation = true;
                                paramOrder = &enumIt->second;
                            }
                        }
                    }
                    if (resolvedInstantiation && paramOrder) {
                        size_t expectedParams = paramOrder->size();
                        if (!selfT->genericArgs.empty() && expectedParams != 0 &&
                            selfT->genericArgs.size() != expectedParams) {
                            addError("Type '" + typeName + "' has " + std::to_string(selfT->genericArgs.size()) +
                                     " type argument(s) but '" + baseId + "' expects " +
                                     std::to_string(expectedParams) + ".", node);
                            if (hasGenericParams) exitScope();
                            return;
                        }
                    }
                }
                if (!resolvedInstantiation) {
                    addError("Type '" + typeName + "' is not defined.", node);
                    if (hasGenericParams) exitScope();
                    return;
                }
            }
        }

        // Validate that all required trait methods are implemented
        if (!validateTraitImpl(typeName, traitName, node->methods, node->associatedTypeBindings, node)) {
            addError("Incomplete implementation of trait '" + traitName + "' for type '" + typeName + "'.", node);
            if (hasGenericParams) exitScope();
            return;
        }

        // Register the trait implementation
        registerTraitImpl(node);

        // Visit all methods to validate their bodies (while type params are still in scope)
        std::string previousImplTraitName = currentImplTraitName;
        auto previousAssociatedTypeBindings = currentImplAssociatedTypeBindings;
        currentImplTraitName = traitName;
        currentImplAssociatedTypeBindings.clear();
        for (const auto& assocBinding : node->associatedTypeBindings) {
            if (assocBinding.name && assocBinding.valueType) {
                currentImplAssociatedTypeBindings[assocBinding.name->name] = assocBinding.valueType.get();
            }
        }

        processingTraitOrBindMethod = true;  // Don't add bind methods to global scope
        for (const auto& method : node->methods) {
            if (method) {
                method->accept(*this);
            }
        }
        processingTraitOrBindMethod = false;
        currentImplTraitName = previousImplTraitName;
        currentImplAssociatedTypeBindings = previousAssociatedTypeBindings;

        VYB_CDBG << "DEBUG: Successfully registered impl " << traitName << " for " << typeName
                  << " with " << node->methods.size() << " methods" << std::endl;
    } else {
        // This is an inherent bind: bind Type { ... }
        // Just adds methods directly to the type without an aspect
        VYB_CDBG << "DEBUG: Processing inherent bind for " << typeName << std::endl;

        // Visit all methods to validate them
        processingTraitOrBindMethod = true;  // Don't add inherent bind methods to global scope
        for (const auto& method : node->methods) {
            if (method) {
                method->accept(*this);
            }
        }
        processingTraitOrBindMethod = false;
    }

    // Exit the type parameter scope if we entered one
    if (hasGenericParams) {
        exitScope();

    }

    // Restore previous impl type
    currentImplType = previousImplType;
}
void SemanticAnalyzer::handleQualifiedAspectCall(ast::CallExpression* node,
                                                  const std::string& aspectName,
                                                  const std::string& methodName) {
    if (node->arguments.empty()) {
        addError("Qualified aspect call " + aspectName + "::" + methodName +
                 "() requires a receiver as the first argument.", node);
        return;
    }

    // Visit the receiver to resolve its type.
    ast::Expression* receiver = node->arguments[0].get();
    if (receiver) receiver->accept(*this);

    ast::TypeNode* receiverType = nullptr;
    if (receiver) {
        auto it = expressionTypes.find(exprKey(receiver));
        if (it != expressionTypes.end() && it->second.get()) receiverType = it->second.get();
        if (!receiverType && typeOf(receiver)) receiverType = typeOf(receiver).get();
    }
    if (!receiverType) {
        addError("Cannot determine the type of the receiver argument to " +
                 aspectName + "::" + methodName + "().", node);
        return;
    }

    // Visit the remaining arguments for typing.
    for (size_t i = 1; i < node->arguments.size(); ++i) {
        if (node->arguments[i]) node->arguments[i]->accept(*this);
    }

    const std::string typeStr = receiverType->toString();

    TraitInfo* traitInfo = findTrait(aspectName);
    if (!traitInfo) {
        addError("Aspect '" + aspectName + "' is not defined.", node);
        return;
    }

    // A receiver may be a bounded type parameter (e.g. thing<T> where T has a
    // Display bound). Such a receiver has no concrete bind entry; the return
    // type is resolved from the bound aspect's declared method signature.
    bool isBoundTypeParameter = false;
    if (SymbolInfo* sym = currentScope->lookup(typeStr)) {
        if (sym->kind == SymbolInfo::Kind::TYPE_PARAMETER) {
            for (const std::string& bound : sym->bounds) {
                if (boundAspectProvides(bound, aspectName)) {
                    isBoundTypeParameter = true;
                    break;
                }
            }
        }
    }

    // Locate an explicit bind implementation for this concrete type.
    ast::FunctionDeclaration* methodDecl = nullptr;
    bool typeMatchesImpl = false;
    if (!isBoundTypeParameter) {
        auto typeImplsIt = traitImpls.find(typeStr);
        if (typeImplsIt != traitImpls.end()) {
            auto traitIt = typeImplsIt->second.find(aspectName);
            if (traitIt != typeImplsIt->second.end()) {
                typeMatchesImpl = true;
                for (ast::FunctionDeclaration* m : traitIt->second) {
                    if (m && m->id && m->id->name == methodName) {
                        methodDecl = m;
                        break;
                    }
                }
            }
        }

        // Fall back to a generic impl pattern (e.g., bind<T> Container -> Vec<T>).
        if (!typeMatchesImpl) {
            for (const auto& typeEntry : genericTraitImpls) {
                if (!matchesPattern(typeStr, typeEntry.first)) continue;
                const auto& traitMap = typeEntry.second;
                auto traitIt = traitMap.find(aspectName);
                if (traitIt == traitMap.end()) continue;
                const GenericImplInfo* implInfo = traitIt->second.get();
                if (!implInfo || !implInfo->declaration) continue;
                typeMatchesImpl = true;
                for (const auto& m : implInfo->declaration->methods) {
                    if (m && m->id && m->id->name == methodName) {
                        methodDecl = m.get();
                        break;
                    }
                }
                break;
            }
        }
    }

    // Determine whether the method is declared by the aspect itself or any of
    // its transitive super-aspects (inherited methods are dispatchable too).
    const TraitInfo* declaringAspect = nullptr;
    const TraitMethod* declaredMethod = nullptr;
    findAspectMethod(aspectName, methodName, declaringAspect, declaredMethod);
    if (!declaredMethod && !methodDecl) {
        addError("Aspect '" + aspectName + "' does not define a method named '" +
                 methodName + "'.", node);
        return;
    }
    if (!isBoundTypeParameter && !typeMatchesImpl) {
        addError("Type '" + typeStr + "' does not implement aspect '" + aspectName +
                 "' (no bind found).", node);
        return;
    }

    ast::TypeNode* returnTypeNode = methodDecl ? methodDecl->returnTypeNode.get()
                                               : declaredMethod->returnType;
    if (!returnTypeNode) {
        addError("Method '" + methodName + "' in aspect '" + aspectName +
                 "' has no return type.", node);
        return;
    }

    ast::TypeNode* actualReturnType = substituteSelfType(returnTypeNode, typeStr);
    setType(node,  retainType(actualReturnType));
    setType(node,  std::shared_ptr<ast::TypeNode>(actualReturnType->clone()));
    VYB_CDBG << "DEBUG: Qualified aspect call " << aspectName << "::" << methodName
              << " on " << typeStr << " returns " << actualReturnType->toString()
              << std::endl;
}
void SemanticAnalyzer::registerTrait(ast::AspectDeclaration* traitDecl) {
    if (!traitDecl || !traitDecl->name) {
        return;
    }

    auto traitInfo = std::make_unique<TraitInfo>(traitDecl);
    const std::string& traitName = traitInfo->name;

    traitRegistry[traitName] = std::move(traitInfo);
}
void SemanticAnalyzer::registerTraitImpl(ast::BindDeclaration* implDecl) {
    if (!implDecl || !implDecl->selfType || !implDecl->traitType) {
        return;
    }

    std::string typeName = implDecl->selfType->toString();
    std::string traitName = implDecl->traitType->toString();

    // Check if this is a generic implementation (has type parameters)
    bool isGeneric = !implDecl->genericParams.empty();

    if (isGeneric) {
        // Store generic implementation separately
        auto genericInfo = std::make_unique<GenericImplInfo>(implDecl);
        bool newIsBounded = genericInfo->isBounded;

        // Bind selection precedence: when both a bounded and an unbounded generic
        // bind exist for the same trait and type shape, the bounded (more
        // specialized) bind wins regardless of declaration order.
        auto& traitMap = genericTraitImpls[typeName];
        auto existing = traitMap.find(traitName);
        if (existing != traitMap.end()) {
            if (!newIsBounded && existing->second->isBounded) {
                VYB_CDBG << "DEBUG: Keeping bounded generic trait impl for " << traitName
                         << " " << typeName << " over unbounded duplicate" << std::endl;
                return;
            }
        }

        VYB_CDBG << "DEBUG: Storing generic trait impl: " << traitName << " for " << typeName << std::endl;
        traitMap[traitName] = std::move(genericInfo);
    } else {
        // Store concrete implementation
        VYB_CDBG << "DEBUG: Storing concrete trait impl: " << traitName << " for " << typeName << std::endl;

        std::vector<ast::FunctionDeclaration*> implMethods;
        for (const auto& method : implDecl->methods) {
            if (method) {
                implMethods.push_back(method.get());
            }
        }

        traitImpls[typeName][traitName] = implMethods;

        // #365 curated escape hatch. An explicit `bind Handoff -> T` (or
        // `bind Viewable -> T`) states that a value of `T` may cross a thread
        // boundary even though the structural derivation says otherwise -- the
        // reviewed path for shapes the compiler cannot see through (FFI
        // `ptr<T>`-holding structs, opaque C handles, `loc<T>` carriers). The bind
        // wins; a contradiction is a warning, not an error, precisely because
        // overriding is the point. The claim is then the programmer's to keep.
        if (traitName == "Handoff" || traitName == "Viewable") {
            const bool structural =
                traitName == "Handoff"
                    ? thread_boundary::handoffCapable(implDecl->selfType.get(),
                                                      &structFieldTypes, &enumVariantPayloadTypes)
                    : thread_boundary::viewable(implDecl->selfType.get(),
                                                &structFieldTypes, &enumVariantPayloadTypes);
            if (!structural) {
                addWarning("curated thread-boundary bind: `bind " + traitName + " -> " + typeName +
                           "` claims the type may cross a thread boundary, but its structural shape "
                           "is not " +
                           (traitName == "Handoff" ? "handoff-capable" : "viewable") +
                           " -- the explicit bind wins, so the program is accepted; the payload then "
                           "carries the programmer's guarantee (doc/THREAD_BOUNDARY_SCOPE.md).",
                           implDecl);
            }
        }

        auto& associatedMap = traitAssociatedTypeImpls[typeName][traitName];
        associatedMap.clear();
        for (const auto& assocBinding : implDecl->associatedTypeBindings) {
            if (assocBinding.name && assocBinding.valueType) {
                associatedMap[assocBinding.name->name] = assocBinding.valueType.get();
            }
        }

        // Fill in aspect-declared default types for any associated types the
        // bind did not explicitly assign.
        auto traitIt = traitRegistry.find(traitName);
        if (traitIt != traitRegistry.end() && traitIt->second) {
            for (const auto& declaredAssoc : traitIt->second->associatedTypes) {
                if (associatedMap.find(declaredAssoc) != associatedMap.end()) {
                    continue;
                }
                ast::TypeNode* assocDefault = traitIt->second->getAssociatedTypeDefault(declaredAssoc);
                if (assocDefault) {
                    associatedMap[declaredAssoc] = assocDefault;
                }
            }
        }
    }
}
bool SemanticAnalyzer::boundAspectProvides(const std::string& boundAspect,
                                            const std::string& requestedAspect) {
    if (boundAspect == requestedAspect) {
        return true;
    }
    std::unordered_set<std::string> visited;
    std::vector<std::string> stack{boundAspect};
    while (!stack.empty()) {
        std::string cur = stack.back();
        stack.pop_back();
        if (!visited.insert(cur).second) {
            continue;
        }
        if (cur == requestedAspect) {
            return true;
        }
        TraitInfo* info = findTrait(cur);
        if (!info) {
            continue;
        }
        for (const auto& super : info->superTraits) {
            if (!visited.count(super)) {
                stack.push_back(super);
            }
        }
    }
    return false;
}
bool SemanticAnalyzer::findAspectMethod(const std::string& aspectName,
                                        const std::string& methodName,
                                        const TraitInfo*& declaringAspect,
                                        const TraitMethod*& outMethod) {
    std::function<bool(const TraitInfo*, std::unordered_set<const TraitInfo*>&)> search;
    search = [&](const TraitInfo* info, std::unordered_set<const TraitInfo*>& visited) -> bool {
        if (!info || !visited.insert(info).second) {
            return false;
        }
        for (const auto& tm : info->methods) {
            if (tm.name == methodName) {
                declaringAspect = info;
                outMethod = &tm;
                return true;
            }
        }
        for (const auto& superName : info->superTraits) {
            TraitInfo* super = findTrait(superName);
            if (super && search(super, visited)) {
                return true;
            }
        }
        return false;
    };
    std::unordered_set<const TraitInfo*> visited;
    return search(findTrait(aspectName), visited);
}
bool SemanticAnalyzer::hasAspectBinding(const std::string& typeStr, const std::string& aspectName) {
    auto it = traitImpls.find(typeStr);
    if (it != traitImpls.end() && it->second.count(aspectName)) {
        return true;
    }
    for (const auto& typeEntry : genericTraitImpls) {
        if (matchesPattern(typeStr, typeEntry.first) && typeEntry.second.count(aspectName)) {
            return true;
        }
    }
    return false;
}
void SemanticAnalyzer::validateAspectInheritance() {
    // Run after every aspect and bind has been registered. Validates that every
    // super-aspect names a defined aspect and that no inheritance cycle exists.
    for (const auto& entry : traitRegistry) {
        TraitInfo* info = entry.second.get();
        if (!info || info->superTraits.empty()) {
            continue;
        }

        std::vector<std::string> stack;
        std::function<bool(const std::string&, const std::string&, int)> findCycle;
        findCycle = [&](const std::string& start, const std::string& current, int depth) -> bool {
            if (depth > 0 && current == start) {
                return true;
            }
            auto curIt = traitRegistry.find(current);
            if (curIt == traitRegistry.end()) {
                // Unknown super-aspect; reported separately below.
                return false;
            }
            for (const auto& next : curIt->second->superTraits) {
                if (findCycle(start, next, depth + 1)) {
                    return true;
                }
            }
            return false;
        };

        for (const auto& superName : info->superTraits) {
            if (traitRegistry.find(superName) == traitRegistry.end()) {
                addError("Super-aspect '" + superName + "' of aspect '" + info->name +
                         "' is not a defined aspect.", info->declaration);
                continue;
            }
            if (findCycle(info->name, superName, 1)) {
                addError("Aspect '" + info->name + "' has a cyclic super-aspect dependency.", info->declaration);
            }
        }

        // Bind requirement: every type (concrete or generic) that binds this
        // sub-aspect must also bind each of its super-aspects. Runs after all
        // binds are registered so declaration order does not matter.
        for (const auto& superName : info->superTraits) {
            for (const auto& typeEntry : traitImpls) {
                if (typeEntry.second.count(info->name) &&
                    !hasAspectBinding(typeEntry.first, superName)) {
                    addError("Type '" + typeEntry.first + "' binds aspect '" + info->name +
                             "' which requires also binding super-aspect '" + superName +
                             "'.", info->declaration);
                }
            }
            for (const auto& typeEntry : genericTraitImpls) {
                if (typeEntry.second.count(info->name) &&
                    !hasAspectBinding(typeEntry.first, superName)) {
                    addError("Generic type '" + typeEntry.first + "' binds aspect '" + info->name +
                             "' which requires also binding super-aspect '" + superName +
                             "'.", info->declaration);
                }
            }
        }
    }
}

} // namespace vyb
