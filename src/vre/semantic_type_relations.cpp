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

bool SemanticAnalyzer::isIntegerType(ast::TypeNode* type) {
    if (!type) return false;

    // Handle TypeName nodes (most common case)
    if (auto tn = dynamic_cast<ast::TypeName*>(type)) {
        if (!tn->identifier) return false;
        const std::string& name = tn->identifier->name;
        return name == "Int" || name == "i8" || name == "i16" || name == "i32" || name == "i64" ||
               name == "u8" || name == "u16" || name == "u32" || name == "u64" || name == "size_t" ||
               name == "isize" || name == "usize" ||
               name == "Int8" || name == "Int16" || name == "Int32" || name == "Int64" ||
               name == "UInt8" || name == "UInt16" || name == "UInt32" || name == "UInt64" ||
               name == "Byte" || name == "Char" || name == "Rune" ||
               name == "CChar" || name == "CUChar" || name == "CShort" || name == "CUShort" ||
               name == "CInt" || name == "CUInt" || name == "CLong" || name == "CULong" ||
               name == "CSize" || name == "CSSize";
    }

    // Handle array size expressions which might be integer literals
    if (auto arrayType = dynamic_cast<ast::ArrayType*>(type)) {
        // If it has a size expression that's a literal, it might be integer type
        if (arrayType->sizeExpression) {
            auto it = expressionTypes.find(exprKey(arrayType->sizeExpression.get()));
            if (it != expressionTypes.end() && it->second) {
                return isIntegerType(it->second.get());
            }
        }
    }

    // Add any other type checks that could represent integer types
    // For example, if there are typedef'ed types or alias types

    return false;
}
bool SemanticAnalyzer::isFloatType(ast::TypeNode* type) {
    if (!type) return false;
    if (auto tn = dynamic_cast<ast::TypeName*>(type)) {
        if (!tn->identifier) return false;
        const std::string& name = tn->identifier->name;
        return name == "Float" || name == "Float64" || name == "Float32" ||
               name == "f32" || name == "f64" || name == "float32" || name == "float64";
    }
    return false;
}
std::shared_ptr<ast::TypeNode> SemanticAnalyzer::cloneTypeNode(ast::TypeNode* type) {
    if (!type) return nullptr;

    // Use the existing clone() method on TypeNode
    return std::shared_ptr<ast::TypeNode>(type->clone());
}
ast::TypeNode* SemanticAnalyzer::substituteSelfType(ast::TypeNode* returnType, const std::string& concreteType) {
    if (!returnType) return nullptr;

    // Check if return type is Self
    if (auto typeName = dynamic_cast<ast::TypeName*>(returnType)) {
        if (typeName->identifier && typeName->identifier->name == "Self") {
            // Replace Self with the concrete type
            // Need to parse concreteType to extract base name and generic arguments
            // E.g., "Box<Point>" -> base="Box", genericArgs=["Point"]

            size_t anglePos = concreteType.find('<');
            if (anglePos != std::string::npos) {
                // Has generic arguments
                std::string baseName = concreteType.substr(0, anglePos);
                std::string argsStr = concreteType.substr(anglePos + 1);
                // Remove trailing '>'
                if (!argsStr.empty() && argsStr.back() == '>') {
                    argsStr.pop_back();
                }

                // Parse generic arguments (simple comma-separated list for now)
                std::vector<ast::TypeNodePtr> genericArgs;
                size_t start = 0;
                while (start < argsStr.length()) {
                    size_t commaPos = argsStr.find(',', start);
                    std::string argName;
                    if (commaPos != std::string::npos) {
                        argName = argsStr.substr(start, commaPos - start);
                        start = commaPos + 1;
                    } else {
                        argName = argsStr.substr(start);
                        start = argsStr.length();
                    }

                    // Trim whitespace
                    argName.erase(0, argName.find_first_not_of(" \t"));
                    argName.erase(argName.find_last_not_of(" \t") + 1);

                    if (!argName.empty()) {
                        // Create TypeName for this argument
                        auto argId = std::make_unique<ast::Identifier>(typeName->loc, argName);
                        genericArgs.push_back(std::make_unique<ast::TypeName>(typeName->loc, std::move(argId)));
                    }
                }

                // Create TypeName with base and generic args
                auto baseId = std::make_unique<ast::Identifier>(typeName->loc, baseName);
                return new ast::TypeName(typeName->loc, std::move(baseId), std::move(genericArgs));
            } else {
                // No generic arguments
                return new ast::TypeName(typeName->loc, std::make_unique<ast::Identifier>(typeName->loc, concreteType));
            }
        }
    }

    // If not Self, return an owned copy so callers can always retain the result
    // without risk of double-freeing a borrowed pointer into the module AST.
    return returnType->clone().release();
}

// #212 made an omitted return signature canonical implicit Void. For an aspect
// implementation the signature is not absent — it is declared by the aspect — so
// the canonical-Void rule must not override it (#250). This resolves the return
// type a bind body inherits from the aspect method it implements.
ast::TypeNode* SemanticAnalyzer::aspectMethodReturnTypeFor(const std::string& aspectName,
                                                           const std::string& methodName) {
    if (aspectName.empty() || methodName.empty()) return nullptr;

    TraitInfo* info = findTrait(aspectName);
    if (!info) return nullptr;

    ast::TypeNode* declared = nullptr;
    for (const TraitMethod& method : info->methods) {
        if (method.name == methodName && method.returnType) {
            declared = method.returnType;
            break;
        }
    }
    if (!declared) return nullptr;

    // `type T = Int` in the bind supplies the aspect's associated type, so a
    // signature that names it (`-> T`) means the bind's assignment.
    if (auto* typeName = dynamic_cast<ast::TypeName*>(declared)) {
        if (typeName->identifier && typeName->genericArgs.empty()) {
            auto bindingIt = currentImplAssociatedTypeBindings.find(typeName->identifier->name);
            if (bindingIt != currentImplAssociatedTypeBindings.end() && bindingIt->second) {
                return bindingIt->second->clone().release();
            }
        }
    }

    // `Self` in the signature is the bind target (e.g. bind Display -> Box<Int>).
    if (currentImplType) {
        return substituteSelfType(declared, currentImplType->toString());
    }
    return declared->clone().release();
}
static std::string normalizeTypeName(const std::string& name) {
    if (name == "i64" || name == "int" || name == "long")  return "Int";
    if (name == "i32" || name == "int32")                  return "Int32";
    if (name == "i16" || name == "int16" || name == "short") return "Int16";
    if (name == "i8"  || name == "int8"  || name == "char") return "Int8";
    if (name == "CChar")                                   return "Int8";
    if (name == "CUChar")                                  return "UInt8";
    if (name == "CShort")                                  return "Int16";
    if (name == "CUShort")                                 return "UInt16";
    if (name == "CInt")                                    return "Int32";
    if (name == "CUInt")                                   return "UInt32";
    if (name == "CLong" || name == "CSSize")               return "Int";
    if (name == "CULong" || name == "CSize")               return "UInt64";
    if (name == "CFloat")                                  return "Float32";
    if (name == "CDouble")                                 return "Float";
    if (name == "CVoid")                                   return "Void";
    if (name == "u64" || name == "uint64")                 return "UInt64";
    if (name == "u32" || name == "uint32")                 return "UInt32";
    if (name == "u16" || name == "uint16")                 return "UInt16";
    if (name == "u8"  || name == "uint8" || name == "byte") return "UInt8";
    if (name == "f64" || name == "double")                 return "Float";
    if (name == "f32" || name == "float")                  return "Float32";
    if (name == "bool")                                    return "Bool";
    if (name == "void")                                    return "Void";
    return name; // already canonical
}
bool SemanticAnalyzer::areTypesCompatible(ast::TypeNode* targetType, ast::TypeNode* valueType) {
    if (!targetType || !valueType) {
        // Both null → same unresolved type; otherwise incompatible
        return (!targetType && !valueType);
    }

    // Resolve type aliases: if either type is a TypeName that's defined as a type alias
    // in the current scope, use the aliased type instead.
    auto resolveAlias = [this](ast::TypeNode* t) -> ast::TypeNode* {
        if (t && t->getCategory() == ast::TypeNode::Category::IDENTIFIER) {
            auto* tn = static_cast<ast::TypeName*>(t);
            if (tn->identifier && tn->genericArgs.empty()) {
                auto* sym = currentScope->lookup(tn->identifier->name);
                if (sym && sym->kind == SymbolInfo::Kind::Type && sym->type) {
                    return sym->type.get();
                }
            }
        }
        return t;
    };
    ast::TypeNode* resolvedTarget = resolveAlias(targetType);
    ast::TypeNode* resolvedValue = resolveAlias(valueType);
    // If aliases resolved to different nodes, recurse with resolved types
    if (resolvedTarget != targetType || resolvedValue != valueType) {
        return areTypesCompatible(resolvedTarget, resolvedValue);
    }

    // If they are the exact same type object.
    if (targetType == valueType) {
        return true;
    }

    // Ownership wrappers over primitives are transparent: my<Int>, Int, and
    // our<Int> all lower to the same scalar at codegen time.
    ast::TypeNode* unwrappedTarget = unwrapPrimitiveOwnershipType(targetType);
    ast::TypeNode* unwrappedValue = unwrapPrimitiveOwnershipType(valueType);
    if (unwrappedTarget != targetType || unwrappedValue != valueType) {
        return areTypesCompatible(unwrappedTarget, unwrappedValue);
    }

    ast::TypeNode::Category categoryTarget = targetType->getCategory();
    ast::TypeNode::Category categoryValue = valueType->getCategory();

    // Special Case 1: Assigning 'nil' to an Optional type
    if (categoryValue == ast::TypeNode::Category::IDENTIFIER) {
        if (auto* tnValue = dynamic_cast<ast::TypeName*>(valueType)) {
            if (tnValue->identifier && tnValue->identifier->name == "nil") {
                if (categoryTarget == ast::TypeNode::Category::OPTIONAL) {
                    return true;
                }
                // Vyb might also allow assigning 'nil' to raw pointer types.
                // if (categoryTarget == ast::TypeNode::Category::POINTER) return true;
            }
        }
    }

    // Special Case 2: Integer literal (typed as "Int" or "int") can be assigned to any integer type.
    if (categoryTarget == ast::TypeNode::Category::IDENTIFIER &&
        categoryValue == ast::TypeNode::Category::IDENTIFIER) {
        auto* tnTarget = static_cast<ast::TypeName*>(targetType);
        auto* tnValue = static_cast<ast::TypeName*>(valueType);
        if (tnTarget->identifier && tnValue->identifier) {
            bool isSpecificIntTarget = isIntegerType(tnTarget);
            const std::string& valName = tnValue->identifier->name;
            // Integer literals are typed as "Int" (64-bit) in the semantic analyser
            bool isGenericIntValue = (valName == "Int" || valName == "int");
            if (isSpecificIntTarget && isGenericIntValue) {
                return true; // e.g., x<i32> = 10  or  x<Int8> = 255
            }

            // Also allow any integer-alias → any other integer-alias (e.g. i32 → Int32)
            if (isIntegerType(tnTarget) && isIntegerType(tnValue)) {
                return normalizeTypeName(tnTarget->identifier->name) ==
                       normalizeTypeName(tnValue->identifier->name);
            }

            // Float literal (typed as "Float") can be assigned to any float type
            bool isFloatTarget = (tnTarget->identifier->name == "Float" ||
                                  tnTarget->identifier->name == "Float32" ||
                                  tnTarget->identifier->name == "Float64" ||
                                  tnTarget->identifier->name == "f32" ||
                                  tnTarget->identifier->name == "f64");
            bool isGenericFloatValue = (valName == "Float" || valName == "Float64" || valName == "f64");
            if (isFloatTarget && isGenericFloatValue) {
                return true; // e.g., x<Float32> = 3.14
            }

            // Special Case: Ownership wrappers. my<T>, their<T>, our<T>, mild<T> are compatible
            // with the inner type T for initialization (e.g., x<my<Int>> = 42).
            static const std::set<std::string> ownershipWrappers = {"my", "their", "our", "mild", "view"};
            if (tnTarget->identifier && ownershipWrappers.count(tnTarget->identifier->name) > 0
                && tnTarget->genericArgs.size() == 1) {
                // Check if value type is compatible with the inner type
                if (areTypesCompatible(tnTarget->genericArgs[0].get(), valueType)) {
                    return true;
                }
            }
        }
    }

    // If categories are different and not covered by the above special cases,
    // they are generally not compatible without an explicit cast.
    // Exception: if both types produce the same string representation, treat them as
    // compatible. This handles cases like Vec<Int> TypeName vs VecType, Future<T>
    // TypeName vs FutureType, etc., which arise from different paths through the
    // semantic analyzer and parser that produce structurally identical types in
    // different internal representations.
    if (categoryTarget != categoryValue) {
        if (targetType->toString() == valueType->toString()) {
            return true;
        }
        // Cross-category: Tuple<T,U> TypeName vs TupleTypeNode
        auto isTupleTypeName = [](ast::TypeNode* t) -> ast::TypeName* {
            if (t->getCategory() != ast::TypeNode::Category::IDENTIFIER) return nullptr;
            auto* tn = static_cast<ast::TypeName*>(t);
            if (tn->identifier && tn->identifier->name == "Tuple") return tn;
            return nullptr;
        };
        ast::TypeName* tupleTarget = isTupleTypeName(targetType);
        ast::TypeName* tupleValue = isTupleTypeName(valueType);
        if (tupleTarget && categoryValue == ast::TypeNode::Category::TUPLE) {
            auto* tt = static_cast<ast::TupleTypeNode*>(valueType);
            if (tupleTarget->genericArgs.size() == tt->memberTypes.size()) {
                bool ok = true;
                for (size_t i = 0; i < tt->memberTypes.size(); ++i) {
                    if (!areTypesCompatible(tupleTarget->genericArgs[i].get(), tt->memberTypes[i].get())) { ok = false; break; }
                }
                if (ok) return true;
            }
        }
        if (tupleValue && categoryTarget == ast::TypeNode::Category::TUPLE) {
            auto* tt = static_cast<ast::TupleTypeNode*>(targetType);
            if (tupleValue->genericArgs.size() == tt->memberTypes.size()) {
                bool ok = true;
                for (size_t i = 0; i < tt->memberTypes.size(); ++i) {
                    if (!areTypesCompatible(tt->memberTypes[i].get(), tupleValue->genericArgs[i].get())) { ok = false; break; }
                }
                if (ok) return true;
            }
        }
        return false;
    }

    // Categories are the same, proceed with category-specific checks.
    switch (categoryTarget) {
        case ast::TypeNode::Category::IDENTIFIER: {
            auto* tnTarget = static_cast<ast::TypeName*>(targetType);
            auto* tnValue = static_cast<ast::TypeName*>(valueType);
            if (!tnTarget->identifier || !tnValue->identifier) return false;

            // Normalize type names: treat LLVM aliases (i32, i64, …) as their
            // canonical Vyb equivalents (Int32, Int, …) for compatibility checks.
            std::string nameTarget = normalizeTypeName(tnTarget->identifier->name);
            std::string nameValue  = normalizeTypeName(tnValue->identifier->name);

            // For integer types, require the same normalised name.
            // (isIntegerType checks the original name; recheck after normalisation)
            if (nameTarget == nameValue) {
                if (tnTarget->genericArgs.size() != tnValue->genericArgs.size()) return false;
                for (size_t i = 0; i < tnTarget->genericArgs.size(); ++i) {
                    if (!areTypesCompatible(tnTarget->genericArgs[i].get(), tnValue->genericArgs[i].get())) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
        case ast::TypeNode::Category::POINTER: {
            auto* ptTarget = static_cast<ast::PointerType*>(targetType);
            auto* ptValue = static_cast<ast::PointerType*>(valueType);
            // T* is compatible with U* if T is compatible with U (invariant for now).
            // Vyb might have rules for void* or covariance/contravariance.
            return areTypesCompatible(ptTarget->pointeeType.get(), ptValue->pointeeType.get());
        }
        case ast::TypeNode::Category::ARRAY: {
            auto* atTarget = static_cast<ast::ArrayType*>(targetType);
            auto* atValue = static_cast<ast::ArrayType*>(valueType);
            // T[N] is compatible with U[M] if T is compatible with U.
            // For simplicity, ignoring size compatibility for now (atTarget->sizeExpression vs atValue->sizeExpression).
            // A full check would compare constant sizes if available.
            return areTypesCompatible(atTarget->elementType.get(), atValue->elementType.get());
        }
        case ast::TypeNode::Category::VEC: {
            auto* vtTarget = static_cast<ast::VecType*>(targetType);
            auto* vtValue = static_cast<ast::VecType*>(valueType);
            // Vec<T> is compatible with Vec<U> if T is compatible with U.
            return areTypesCompatible(vtTarget->elementType.get(), vtValue->elementType.get());
        }
        case ast::TypeNode::Category::FUNCTION: {
            auto* ftTarget = static_cast<ast::FunctionType*>(targetType);
            auto* ftValue = static_cast<ast::FunctionType*>(valueType);

            // Return types: a null return type means "no value returned" (void),
            // which is compatible with an explicit `void` annotation. Two explicit
            // return types are compared structurally.
            auto isVoidReturn = [](ast::TypeNode* t) {
                auto* tn = dynamic_cast<ast::TypeName*>(t);
                return tn && tn->identifier && tn->identifier->name == "void";
            };
            ast::TypeNode* targetRet = ftTarget->returnType.get();
            ast::TypeNode* valueRet = ftValue->returnType.get();
            if (targetRet && valueRet) {
                if (!areTypesCompatible(targetRet, valueRet)) return false;
            } else if (targetRet) {
                if (!isVoidReturn(targetRet)) return false;
            } else if (valueRet) {
                if (!isVoidReturn(valueRet)) return false;
            }

            // Parameter types: invariant comparison; unknown types ("?") are always compatible
            if (ftTarget->parameterTypes.size() != ftValue->parameterTypes.size()) return false;
            for (size_t i = 0; i < ftTarget->parameterTypes.size(); ++i) {
                auto* pt = ftTarget->parameterTypes[i].get();
                auto* pv = ftValue->parameterTypes[i].get();
                // If either side is the "?" placeholder, skip the comparison
                auto* ptName = dynamic_cast<ast::TypeName*>(pt);
                auto* pvName = dynamic_cast<ast::TypeName*>(pv);
                bool targetIsUnknown = ptName && ptName->identifier && ptName->identifier->name == "?";
                bool valueIsUnknown  = pvName && pvName->identifier && pvName->identifier->name == "?";
                if (targetIsUnknown || valueIsUnknown) continue;
                if (!areTypesCompatible(pt, pv)) return false;
            }
            return true;
        }
        case ast::TypeNode::Category::OPTIONAL: {
            auto* otTarget = static_cast<ast::OptionalType*>(targetType);
            auto* otValue = static_cast<ast::OptionalType*>(valueType);
            // Optional<T> is compatible with Optional<U> if T is compatible with U.
            return areTypesCompatible(otTarget->containedType.get(), otValue->containedType.get());
        }
        case ast::TypeNode::Category::TUPLE: {
            auto* ttTarget = static_cast<ast::TupleTypeNode*>(targetType);
            auto* ttValue = static_cast<ast::TupleTypeNode*>(valueType);
            if (ttTarget->memberTypes.size() != ttValue->memberTypes.size()) return false;
            for (size_t i = 0; i < ttTarget->memberTypes.size(); ++i) {
                if (!areTypesCompatible(ttTarget->memberTypes[i].get(), ttValue->memberTypes[i].get())) {
                    return false;
                }
            }
            return true;
        }
        // case ast::TypeNode::Category::STRUCT:
            // Struct compatibility would typically be nominal (same definition) or structural.
            // Nominal is partly handled by IDENTIFIER if struct names are unique and resolved.
            // Structural would require comparing field types and names.
        // case ast::TypeNode::Category::REFERENCE: // Not fully defined in provided AST
        // case ast::TypeNode::Category::SLICE:     // Not fully defined in provided AST
        default:
            // Fallback for unhandled categories or complex types.
            // This is a weak check and ideally should be replaced with more specific rules
            // or by ensuring all types are resolved to canonical forms before comparison.
            if (targetType->toString() == valueType->toString()) { // Basic structural check via string representation
                return true;
            }
            // If they are TypeName, it might have been missed by earlier checks
            if (categoryTarget == ast::TypeNode::Category::IDENTIFIER) {
                 auto* tnTarget = static_cast<ast::TypeName*>(targetType);
                 auto* tnValue = static_cast<ast::TypeName*>(valueType); // Already know category is IDENTIFIER
                 if (tnTarget->identifier && tnValue->identifier && tnTarget->identifier->name == tnValue->identifier->name) {
                     if (tnTarget->genericArgs.size() == tnValue->genericArgs.size()) {
                         bool allArgsCompatible = true;
                         for (size_t i = 0; i < tnTarget->genericArgs.size(); ++i) {
                            if (!areTypesCompatible(tnTarget->genericArgs[i].get(), tnValue->genericArgs[i].get())) {
                                allArgsCompatible = false;
                                break;
                            }
                         }
                         if (allArgsCompatible) return true;
                     }
                 }
            }
            return false;
    }
    return false; // Should be unreachable if all cases are handled
}
bool SemanticAnalyzer::matchesPattern(const std::string& concreteType, const std::string& pattern) {
    // Simple pattern matching: Box<Int> matches Box<T>
    // Extract base type from both

    size_t concreteAngle = concreteType.find('<');
    size_t patternAngle = pattern.find('<');

    // If pattern has no angle brackets, must match exactly
    if (patternAngle == std::string::npos) {
        return concreteType == pattern;
    }

    // If concrete type has no angle brackets but pattern does, no match
    if (concreteAngle == std::string::npos) {
        return false;
    }

    // Check base types match (e.g., "Box" == "Box")
    std::string concreteBase = concreteType.substr(0, concreteAngle);
    std::string patternBase = pattern.substr(0, patternAngle);

    if (concreteBase != patternBase) {
        return false;
    }

    // For now, if base types match and both have angle brackets, consider it a match
    // A more sophisticated implementation would validate type argument counts
    return true;
}

} // namespace vyb
