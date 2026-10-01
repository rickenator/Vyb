// SPDX-License-Identifier: Apache-2.0
//
// Seam extracted from src/vre/llvm/cgen_expr.cpp (#347): the member definitions below
// are moved VERBATIM -- no reformatting, no renames -- so the split stays
// behaviour-neutral. Helpers that other code still calls are promoted into
// include/vyb/vre/semantic_internal.hpp as `inline`, never duplicated.

#include "vyb/vre/llvm/codegen.hpp"
#include "vyb/vre/llvm/cgen_internal.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/vre/thread_boundary.hpp"
#include "vyb/parser/token.hpp" // For TokenType in BinaryExpression
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h> // For PointerType, StructType
#include <llvm/ADT/APFloat.h>   // For APFloat in FloatLiteral
#include <llvm/ADT/APInt.h>     // For APInt in IntegerLiteral
#include <regex>                // For regex in MemberExpression loaded value handling

namespace vyb {

void LLVMCodegen::visit(vyb::ast::CallExpression *node) {

    // Kernel-mode device intrinsics (#198 P4): lowered directly, no symbol lookup.
    if (vyb::g_kernel_mode && emitKernelIntrinsic(node)) {
        return;
    }

    // Step (c) of #365: judge a spawn-site closure's captures at their *concrete*
    // types. The semantic pass ran the same structural predicate earlier, but it
    // can only see a generic function's declared type parameter (`T`) and stays
    // permissive there; here `currentTypeSubstitutions` maps each active
    // parameter to its real argument, so the verdict becomes real for generic
    // code (doc/THREAD_BOUNDARY_SCOPE.md).
    if (auto* spawnCallee = dynamic_cast<ast::Identifier*>(node->callee.get())) {
        static const std::set<std::string> spawnSites = {
            "thread_spawn", "task_spawn", "async_spawn", "agent_start",
            "vyb_thread_spawn", "vyb_task_spawn", "vyb_async_spawn",
            "agent_start_bool", "agent_start_float", "agent_start_string",
            "vyb_agent_start", "vyb_agent_start_bool", "vyb_agent_start_float",
            "vyb_agent_start_string",
        };
        if (spawnSites.count(spawnCallee->name) && !node->arguments.empty()) {
            if (auto* fe = dynamic_cast<ast::FunctionExpression*>(node->arguments[0].get())) {
                std::string siteName = spawnCallee->name;
                if (siteName.rfind("vyb_", 0) == 0) siteName = siteName.substr(4);
                if (siteName.rfind("agent_start", 0) == 0) siteName = "agent_start";
                checkSpawnHandoffWithSubstitutions(fe, node->arguments[0].get(), siteName);
            }
        }
    }

    // Bare builtin enum constructor: `Ok(x)` / `Err(e)` for Result. The semantic
    // layer injected the target enum type into this node, so the payload types
    // are recovered from its generic arguments and the value is built as the
    // corresponding tagged-union variant.
    if (auto calleeId = dynamic_cast<ast::Identifier*>(node->callee.get())) {
        const std::string& cn = calleeId->name;
        if ((cn == "Ok" || cn == "Err") && typeOfNode(node)) {
            auto* etn = dynamic_cast<ast::TypeName*>(typeOfNode(node).get());
            if (etn && etn->identifier) {
                const std::string& en = etn->identifier->name;
                bool isResultCtor = (en == "Result" && (cn == "Ok" || cn == "Err"));
                if (isResultCtor && !etn->genericArgs.empty()) {
                    // Inside a monomorphized generic bind body the enclosing type
                    // params (e.g. `T` in `Option<T>`) are active via
                    // currentTypeSubstitutions; substitute them so a bare `Some(v)`
                    // becomes `Option<Int>` rather than an unresolved `Option_T`.
                    std::vector<ast::TypeNodePtr> concreteEnumArgs;
                    concreteEnumArgs.reserve(etn->genericArgs.size());
                    for (const auto& arg : etn->genericArgs) {
                        std::string s = arg->toString();
                        if (!currentTypeSubstitutions.empty()) {
                            for (const auto& kv : currentTypeSubstitutions) {
                                s = replaceTypeTokens(s, kv.first, kv.second);
                            }
                        }
                        concreteEnumArgs.push_back(typePatternToTypeNode(TypePattern::parse(s), node->loc));
                    }
                    std::string mangled = mangleGenericTypeName(en, concreteEnumArgs);
                    if (!taggedEnumInfo.count(mangled)) monomorphizeEnum(en, concreteEnumArgs);
                    std::vector<llvm::Value*> payloadVals;
                    for (auto& arg : node->arguments) {
                        arg->accept(*this);
                        payloadVals.push_back(m_currentLLVMValue);
                    }
                    m_currentLLVMValue = buildTaggedEnumValue(mangled, cn, payloadVals);
                    return;
                }
            }
        }
    }

    // Bare builtin Vec constructor: `Vec()`, `Vec(n)`, or explicitly-typed
    // `Vec<T>()` / `Vec<T>(n)`. The callee is the identifier `Vec` (or a
    // `Vec<...>` generic instantiation) rather than the legacy `Vec::new(...)`.
    if (auto idCallee = dynamic_cast<ast::Identifier*>(node->callee.get())) {
        if (idCallee->name == "Vec") {
            emitVecConstructor(node);
            return;
        }
    }

    // Check for Vec::new() constructor calls
    // VYB_CDBG << "DEBUG: Checking if callee is MemberExpression..." << std::endl;
    if (auto memberExpr = dynamic_cast<vyb::ast::MemberExpression*>(node->callee.get())) {
        // Tagged-union enum variant constructor: Shape::Circle(x), Shape::Rect(a, b).
        // Generic data enum variant constructor: Box<Int>::Value(x).
        if (auto gi = dynamic_cast<ast::GenericInstantiationExpression*>(memberExpr->object.get())) {
            if (auto enIdent = dynamic_cast<ast::Identifier*>(gi->baseExpression.get())) {
                if (auto varIdent = dynamic_cast<ast::Identifier*>(memberExpr->property.get())) {
                    if (genericEnumTemplates.count(enIdent->name) || enIdent->name == "Result") {
                        // Inside a monomorphized generic bind body the enclosing type
                        // params (e.g. `E` in `Result<Int, E>::Err(...)`) are active via
                        // currentTypeSubstitutions; substitute them into the concrete
                        // enum instantiation so `Result<Int, E>` becomes
                        // `Result<Int, String>` instead of an unresolved `Result_Int_E`.
                        std::vector<ast::TypeNodePtr> concreteEnumArgs;
                        concreteEnumArgs.reserve(gi->genericArguments.size());
                        for (const auto& arg : gi->genericArguments) {
                            std::string s = arg->toString();
                            if (!currentTypeSubstitutions.empty()) {
                                for (const auto& kv : currentTypeSubstitutions) {
                                    s = replaceTypeTokens(s, kv.first, kv.second);
                                }
                            }
                            concreteEnumArgs.push_back(typePatternToTypeNode(TypePattern::parse(s), gi->loc));
                        }
                        std::string mangled = mangleGenericTypeName(enIdent->name, concreteEnumArgs);
                        if (!taggedEnumInfo.count(mangled)) {
                            monomorphizeEnum(enIdent->name, concreteEnumArgs);
                        }
                        std::vector<llvm::Value*> payloadVals;
                        for (auto& arg : node->arguments) {
                            arg->accept(*this);
                            payloadVals.push_back(m_currentLLVMValue);
                        }
                        m_currentLLVMValue = buildTaggedEnumValue(mangled, varIdent->name, payloadVals);
                        return;
                    }
                }
            }
        }
        if (auto enIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->object.get())) {
            if (auto varIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->property.get())) {
                auto tagIt = taggedEnumInfo.find(enIdent->name);
                if (tagIt != taggedEnumInfo.end()) {
                    std::vector<llvm::Value*> payloadVals;
                    for (auto& arg : node->arguments) {
                        arg->accept(*this);
                        payloadVals.push_back(m_currentLLVMValue);
                    }
                    m_currentLLVMValue = buildTaggedEnumValue(enIdent->name, varIdent->name, payloadVals);
                    return;
                }
            }
        }
        // VYB_CDBG << "DEBUG: Found MemberExpression callee" << std::endl;
        // VYB_CDBG << "DEBUG: MemberExpression object: " << (memberExpr->object ? memberExpr->object->toString() : "null") << std::endl;
        // VYB_CDBG << "DEBUG: MemberExpression property: " << (memberExpr->property ? memberExpr->property->toString() : "null") << std::endl;
        if (auto vecIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->object.get())) {
            VYB_CDBG << "DEBUG: MemberExpression object is Identifier: " << vecIdent->name << std::endl;
            if (auto newIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->property.get())) {
                VYB_CDBG << "DEBUG: MemberExpression property is Identifier: " << newIdent->name << std::endl;
                if (vecIdent->name == "Vec" && newIdent->name == "new") {
                    // This is Vec::new() or Vec::new(size) - create a vector
                    VYB_CDBG << "DEBUG: Creating Vec::new() constructor" << std::endl;

                    emitVecConstructor(node);
                    return;
                }

                // Handle T::from_string() static method calls
                const std::string& typeName = vecIdent->name;
                const std::string& methodName = newIdent->name;

                if (methodName == "from_string") {
                    VYB_CDBG << "DEBUG: Processing " << typeName << "::from_string() call" << std::endl;

                    // Validate arguments
                    if (node->arguments.size() != 1) {
                        logError(node->loc, typeName + "::from_string() expects exactly 1 argument (string to parse)");
                        m_currentLLVMValue = nullptr;
                        return;
                    }

                    // Evaluate the string argument
                    node->arguments[0]->accept(*this);
                    llvm::Value* stringArg = m_currentLLVMValue;
                    if (!stringArg) {
                        logError(node->loc, "Failed to evaluate string argument for " + typeName + "::from_string()");
                        m_currentLLVMValue = nullptr;
                        return;
                    }

                    // Extract the char* from the Vyb String struct { ptr, i64 }
                    llvm::Value* charPtr = builder->CreateExtractValue(stringArg, 0, "str.ptr");

                    // Handle primitive types
                    if (typeName == "Int") {
                        // Call __vyb_int_from_string(str, success_ptr)
                        llvm::FunctionType* fromStringType = llvm::FunctionType::get(
                            int64Type,
                            {int8PtrType, llvm::PointerType::get(int1Type, 0)},
                            false
                        );
                        llvm::Function* fromStringFunc = module->getFunction("__vyb_int_from_string");
                        if (!fromStringFunc) {
                            fromStringFunc = llvm::Function::Create(fromStringType,
                                llvm::Function::ExternalLinkage, "__vyb_int_from_string", module.get());
                        }

                        // Allocate success flag
                        llvm::Value* successPtr = builder->CreateAlloca(int1Type, nullptr, "success");

                        // Call from_string with char*
                        llvm::Value* result = builder->CreateCall(fromStringFunc, {charPtr, successPtr}, "from_string.result");

                        // TODO: Check success flag and handle errors
                        m_currentLLVMValue = result;
                        return;
                    } else if (typeName == "Float") {
                        // Call __vyb_float_from_string(str, success_ptr)
                        llvm::FunctionType* fromStringType = llvm::FunctionType::get(
                            doubleType,
                            {int8PtrType, llvm::PointerType::get(int1Type, 0)},
                            false
                        );
                        llvm::Function* fromStringFunc = module->getFunction("__vyb_float_from_string");
                        if (!fromStringFunc) {
                            fromStringFunc = llvm::Function::Create(fromStringType,
                                llvm::Function::ExternalLinkage, "__vyb_float_from_string", module.get());
                        }

                        llvm::Value* successPtr = builder->CreateAlloca(int1Type, nullptr, "success");
                        llvm::Value* result = builder->CreateCall(fromStringFunc, {charPtr, successPtr}, "from_string.result");
                        m_currentLLVMValue = result;
                        return;
                    } else if (typeName == "Bool") {
                        // Call __vyb_bool_from_string(str, success_ptr)
                        llvm::FunctionType* fromStringType = llvm::FunctionType::get(
                            int1Type,
                            {int8PtrType, llvm::PointerType::get(int1Type, 0)},
                            false
                        );
                        llvm::Function* fromStringFunc = module->getFunction("__vyb_bool_from_string");
                        if (!fromStringFunc) {
                            fromStringFunc = llvm::Function::Create(fromStringType,
                                llvm::Function::ExternalLinkage, "__vyb_bool_from_string", module.get());
                        }

                        llvm::Value* successPtr = builder->CreateAlloca(int1Type, nullptr, "success");
                        llvm::Value* result = builder->CreateCall(fromStringFunc, {charPtr, successPtr}, "from_string.result");
                        m_currentLLVMValue = result;
                        return;
                    } else if (typeName == "String") {
                        // String::from_string() is identity - just return a copy as Vyb String struct
                        llvm::FunctionType* fromStringType = llvm::FunctionType::get(
                            int8PtrType,
                            {int8PtrType, llvm::PointerType::get(int1Type, 0)},
                            false
                        );
                        llvm::Function* fromStringFunc = module->getFunction("__vyb_string_from_string");
                        if (!fromStringFunc) {
                            fromStringFunc = llvm::Function::Create(fromStringType,
                                llvm::Function::ExternalLinkage, "__vyb_string_from_string", module.get());
                        }

                        llvm::Value* successPtr = builder->CreateAlloca(int1Type, nullptr, "success");
                        llvm::Value* charResult = builder->CreateCall(fromStringFunc, {charPtr, successPtr}, "from_string.char_result");

                        // Convert char* to Vyb String struct { ptr, len }
                        // Declare strlen
                        llvm::FunctionType* strlenType = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                        llvm::Function* strlenFunc = module->getFunction("strlen");
                        if (!strlenFunc) {
                            strlenFunc = llvm::Function::Create(strlenType, llvm::Function::ExternalLinkage, "strlen", module.get());
                        }

                        llvm::Value* strLen = builder->CreateCall(strlenFunc, {charResult}, "str.len");

                        // Build String struct
                        llvm::StructType* stringStructType = llvm::StructType::get(*context, {int8PtrType, int64Type});
                        llvm::Value* stringStruct = llvm::UndefValue::get(stringStructType);
                        stringStruct = builder->CreateInsertValue(stringStruct, charResult, 0, "str.ptr");
                        stringStruct = builder->CreateInsertValue(stringStruct, strLen, 1, "str.len");

                        m_currentLLVMValue = stringStruct;
                        return;
                    } else {
                        // Complex type - call generic JSON deserializer
                        VYB_CDBG << "DEBUG: Generating JSON deserialization for type: " << typeName << std::endl;

                        // Check if this is a known struct type
                        auto structIt = monomorphizedStructs.find(typeName);
                        if (structIt == monomorphizedStructs.end()) {
                            logError(node->loc, "Unknown struct type for deserialization: " + typeName);
                            m_currentLLVMValue = nullptr;
                            return;
                        }

                        llvm::StructType* targetStructType = structIt->second;

                        // Declare __vyb_complex_from_json(json_str, type_name) -> void*
                        llvm::FunctionType* fromJsonType = llvm::FunctionType::get(
                            llvm::PointerType::get(*context, 0),  // returns void*
                            {int8PtrType, int8PtrType},  // (json_str, type_name)
                            false
                        );
                        llvm::Function* fromJsonFunc = module->getFunction("__vyb_complex_from_json");
                        if (!fromJsonFunc) {
                            fromJsonFunc = llvm::Function::Create(fromJsonType,
                                llvm::Function::ExternalLinkage, "__vyb_complex_from_json", module.get());
                        }

                        // Create type name string constant
                        llvm::Value* typeNameStr = builder->CreateGlobalStringPtr(typeName, "type.name");

                        // Call deserializer
                        llvm::Value* resultPtr = builder->CreateCall(fromJsonFunc, {charPtr, typeNameStr}, "from_json.ptr");

                        // Cast void* to struct type pointer
                        llvm::Value* structPtr = builder->CreateBitCast(resultPtr,
                            llvm::PointerType::get(targetStructType, 0), "struct.ptr");

                        // Load the struct value - this copies the struct to the stack
                        // but preserves internal pointers (e.g., String.data still points to heap).
                        llvm::Value* structValue = builder->CreateLoad(targetStructType, structPtr, "struct.value");
                        // The deserializer allocated the instance on the heap; free it now
                        // that its contents have been copied into the returned value. Its
                        // String fields' data pointers live on independently (registered,
                        // owned by the copied value's scope-exit cleanup).
                        builder->CreateCall(getOrCreateFreeFunction(), {resultPtr});
                        m_currentLLVMValue = structValue;
                        return;
                    }
                }

                // Check for String::from_bytes() constructor
                if (vecIdent->name == "String" && newIdent->name == "from_bytes") {
                    VYB_CDBG << "DEBUG: Creating String::from_bytes() constructor" << std::endl;

                    if (node->arguments.size() != 2) {
                        logError(node->loc, "String::from_bytes expects exactly 2 arguments (byte_ptr, length)");
                        m_currentLLVMValue = nullptr;
                        return;
                    }

                    // Evaluate byte pointer argument
                    node->arguments[0]->accept(*this);
                    llvm::Value* bytePtr = m_currentLLVMValue;
                    if (!bytePtr) {
                        logError(node->arguments[0]->loc, "Failed to evaluate byte pointer for String::from_bytes");
                        m_currentLLVMValue = nullptr;
                        return;
                    }

                    // Evaluate length argument
                    node->arguments[1]->accept(*this);
                    llvm::Value* length = m_currentLLVMValue;
                    if (!length) {
                        logError(node->arguments[1]->loc, "Failed to evaluate length for String::from_bytes");
                        m_currentLLVMValue = nullptr;
                        return;
                    }

                    // String::from_bytes produces a fresh, owned, NUL-terminated,
                    // registry-tracked String that COPIES the given bytes. The prior
                    // implementation aliased the caller's byte pointer without copying,
                    // registering, or NUL-terminating it, so the result was not a
                    // first-class owned buffer: `+` / `.concat()` (which size buffers
                    // via strlen and release via the string registry) over-read or
                    // mis-released it, corrupting or hanging the program (#179).
                    std::vector<llvm::Type*> strFields = {
                        llvm::PointerType::get(*context, 0), // ptr to bytes
                        llvm::Type::getInt64Ty(*context)     // length
                    };
                    llvm::StructType* strStructType = llvm::StructType::get(*context, strFields, false);

                    // Clamp a (misused) negative length to zero.
                    llvm::Value* lenVal = builder->CreateIntCast(
                        length, llvm::Type::getInt64Ty(*context), true, "from_bytes.len");
                    llvm::Value* lenGE0 = builder->CreateICmpSGE(lenVal,
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0));
                    lenVal = builder->CreateSelect(lenGE0, lenVal,
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0),
                        "from_bytes.len.clamp");

                    llvm::Value* allocSize = builder->CreateAdd(lenVal,
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1),
                        "from_bytes.allocsize");
                    llvm::FunctionType* mallocType = llvm::FunctionType::get(
                        llvm::PointerType::get(*context, 0),
                        {llvm::Type::getInt64Ty(*context)}, false);
                    llvm::Function* mallocFunc = module->getFunction("malloc");
                    if (!mallocFunc) {
                        mallocFunc = llvm::Function::Create(mallocType,
                            llvm::Function::ExternalLinkage, "malloc", module.get());
                    }
                    llvm::Value* newData = builder->CreateCall(mallocFunc, {allocSize}, "from_bytes.data");
                    builder->CreateCall(getOrCreateVybStringRegisterFunction(), {newData});

                    llvm::FunctionType* memcpyType = llvm::FunctionType::get(
                        llvm::PointerType::get(*context, 0),
                        {llvm::PointerType::get(*context, 0),
                         llvm::PointerType::get(*context, 0),
                         llvm::Type::getInt64Ty(*context)}, false);
                    llvm::Function* memcpyFunc = module->getFunction("memcpy");
                    if (!memcpyFunc) {
                        memcpyFunc = llvm::Function::Create(memcpyType,
                            llvm::Function::ExternalLinkage, "memcpy", module.get());
                    }
                    // Resolve the actual byte source: from_bytes accepts either a raw
                    // byte pointer or a String (whose .data pointer is used). A String
                    // argument evaluates to a {ptr, len} struct here, so extract its .ptr.
                    llvm::Value* srcPtr = bytePtr;
                    if (bytePtr->getType()->isStructTy()) {
                        srcPtr = builder->CreateExtractValue(bytePtr, 0, "from_bytes.src_ptr");
                    }
                    if (!srcPtr->getType()->isPointerTy()) {
                        logError(node->arguments[0]->loc,
                                 "String::from_bytes byte_ptr argument must be a pointer or a String");
                        m_currentLLVMValue = nullptr;
                        return;
                    }

                    builder->CreateCall(memcpyFunc, {newData, srcPtr, lenVal}, "from_bytes.memcpy");

                    // NUL-terminate the fresh buffer so strlen-based paths are safe.
                    llvm::Value* termPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context),
                        newData, lenVal, "from_bytes.term");
                    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0), termPtr);

                    llvm::Value* resultStr = llvm::UndefValue::get(strStructType);
                    resultStr = builder->CreateInsertValue(resultStr, newData, 0, "str.from_bytes_data");
                    resultStr = builder->CreateInsertValue(resultStr, lenVal, 1, "str.from_bytes_len");

                    m_currentLLVMValue = resultStr;
                    VYB_CDBG << "DEBUG: String::from_bytes() created successfully" << std::endl;
                    return;
                }

                // String::from_byte() - fresh 1-byte owned, registered, NUL-terminated
                // String built from the low byte (0..255) of b. Gives byte-oriented code
                // an idiomatic single-byte emitter instead of a lookup-table+substring
                // workaround (#180, blocks VybOS). NUL-safe: a fresh 2-byte buffer holds
                // the byte followed by a NUL terminator, so strlen-based paths are safe.
                if (vecIdent->name == "String" && newIdent->name == "from_byte") {
                    VYB_CDBG << "DEBUG: Creating String::from_byte() constructor" << std::endl;
                    if (node->arguments.size() != 1) {
                        logError(node->loc, "String::from_byte expects exactly 1 argument (byte<Int>)");
                        m_currentLLVMValue = nullptr;
                        return;
                    }
                    node->arguments[0]->accept(*this);
                    llvm::Value* byteVal = m_currentLLVMValue;
                    if (!byteVal) {
                        logError(node->arguments[0]->loc, "Failed to evaluate byte for String::from_byte");
                        m_currentLLVMValue = nullptr;
                        return;
                    }
                    byteVal = builder->CreateAnd(byteVal,
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0xFF, false),
                        "from_byte.low");
                    llvm::FunctionType* mallocType = llvm::FunctionType::get(
                        llvm::PointerType::get(*context, 0), {llvm::Type::getInt64Ty(*context)}, false);
                    llvm::Function* mallocFunc = module->getFunction("malloc");
                    if (!mallocFunc) {
                        mallocFunc = llvm::Function::Create(mallocType,
                            llvm::Function::ExternalLinkage, "malloc", module.get());
                    }
                    llvm::Value* size2 = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 2, false);
                    llvm::Value* bytePtr = builder->CreateCall(mallocFunc, {size2}, "from_byte.data");
                    builder->CreateCall(getOrCreateVybStringRegisterFunction(), {bytePtr});
                    llvm::Value* byte8 = builder->CreateTrunc(byteVal,
                        llvm::Type::getInt8Ty(*context), "from_byte.b8");
                    llvm::Value* zidx = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0, false);
                    builder->CreateStore(byte8,
                        builder->CreateGEP(llvm::Type::getInt8Ty(*context), bytePtr, {zidx}));
                    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0),
                        builder->CreateGEP(llvm::Type::getInt8Ty(*context), bytePtr,
                            {llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1, false)}));
                    std::vector<llvm::Type*> strFields2 = {
                        llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)};
                    llvm::StructType* strStructType2 = llvm::StructType::get(*context, strFields2, false);
                    llvm::Value* resultStr2 = llvm::UndefValue::get(strStructType2);
                    resultStr2 = builder->CreateInsertValue(resultStr2, bytePtr, 0, "from_byte.data");
                    resultStr2 = builder->CreateInsertValue(resultStr2,
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1, false),
                        1, "from_byte.len");
                    m_currentLLVMValue = resultStr2;
                    VYB_CDBG << "DEBUG: String::from_byte() created successfully" << std::endl;
                    return;
                }
            }
        }
    }

    // Handle mild<T>.grab() and mild<T>.released() method calls
    if (auto memberExpr = dynamic_cast<vyb::ast::MemberExpression*>(node->callee.get())) {
        if (auto objIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->object.get())) {
            if (auto methodIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->property.get())) {
                std::string methodName = methodIdent->name;

                // Check if this is a method on mild<T>
                if (methodName == "grab" || methodName == "released") {
                    // Get the object's type to verify it's mild<T>
                    std::string objectType;
                    if (typeOfNode(objIdent)) {
                        objectType = typeOfNode(objIdent)->toString();
                    } else {
                        auto namedIt = namedValues.find(objIdent->name);
                        if (namedIt != namedValues.end()) {
                            auto valueTypeIt = valueTypeMap.find(namedIt->second);
                            if (valueTypeIt != valueTypeMap.end() && valueTypeIt->second) {
                                objectType = valueTypeIt->second->toString();
                            }
                        }
                    }

                    // Check if type starts with "mild<"
                    if (objectType.find("mild<") == 0) {
                        VYB_CDBG << "DEBUG: Processing " << objectType << "." << methodName << "() call" << std::endl;

                        if (methodName == "grab") {
                            // mild<T>.grab() -> returns our<T>? (nil if object freed, strong ref if alive)
                            VYB_CDBG << "DEBUG: mild<T>.grab() - attempting to upgrade to our<T>" << std::endl;

                            // Get the mild<T> value (control block pointer)
                            auto objIt = namedValues.find(objIdent->name);
                            if (objIt == namedValues.end()) {
                                logError(node->loc, "Unknown variable: " + objIdent->name);
                                return;
                            }

                            // Load the control block pointer
                            llvm::Value* controlBlockPtr = builder->CreateLoad(
                                llvm::PointerType::get(*context, 0),
                                objIt->second,
                                objIdent->name + "_grab_cb_load"
                            );

                            // Reconstruct control block type: { i32, i32, i8, ptr }
                            std::vector<llvm::Type*> cbFields = {
                                llvm::Type::getInt32Ty(*context),  // strong_count
                                llvm::Type::getInt32Ty(*context),  // weak_count
                                llvm::Type::getInt8Ty(*context),   // object_freed (i8 for atomic)
                                llvm::PointerType::get(*context, 0) // object_ptr
                            };
                            llvm::StructType* controlBlockType = llvm::StructType::get(*context, cbFields, /*isPacked=*/false);

                            // Get pointer to object_freed (field 2)
                            llvm::Value* objectFreedPtr = builder->CreateStructGEP(
                                controlBlockType,
                                controlBlockPtr,
                                2,
                                objIdent->name + "_grab_obj_freed_ptr"
                            );

                            // Atomic load object_freed flag (acquire semantics)
                            llvm::LoadInst* freedValue = builder->CreateLoad(
                                llvm::Type::getInt8Ty(*context),
                                objectFreedPtr,
                                objIdent->name + "_grab_obj_freed"
                            );
                            freedValue->setAtomic(llvm::AtomicOrdering::Acquire);

                            // Convert i8 to bool
                            llvm::Value* isFreed = builder->CreateICmpNE(
                                freedValue,
                                llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0),
                                objIdent->name + "_grab_is_freed"
                            );

                            // Create blocks for conditional logic
                            llvm::Function* function = builder->GetInsertBlock()->getParent();
                            llvm::BasicBlock* objAliveBlock = llvm::BasicBlock::Create(*context, "grab_alive", function);
                            llvm::BasicBlock* objFreedBlock = llvm::BasicBlock::Create(*context, "grab_freed", function);
                            llvm::BasicBlock* grabContinue = llvm::BasicBlock::Create(*context, "grab_continue", function);

                            // Native optional result `our<T>?`: a `{ our<T> value, i1 hasValue }`
                            // struct with the value at index 0 and the flag at index 1 (the
                            // `codegenType` layout), present while the target is live (carrying
                            // the control-block handle) and absent once released. Consumed via
                            // `match (m.grab()) { o -> ...; ? -> ... }` or `else`.
                            llvm::StructType* optStruct = nullptr;
                            if (typeOfNode(node)) {
                                optStruct = llvm::dyn_cast<llvm::StructType>(codegenType(typeOfNode(node).get()));
                            }
                            if (!optStruct || optStruct->getNumElements() != 2) {
                                optStruct = llvm::StructType::get(
                                    *context,
                                    {controlBlockPtr->getType(), llvm::Type::getInt1Ty(*context)},
                                    false);
                            }

                            // Branch based on object_freed flag
                            builder->CreateCondBr(isFreed, objFreedBlock, objAliveBlock);

                            // Object still alive: increment strong_count and return present (our<T>)
                            builder->SetInsertPoint(objAliveBlock);

                            // Get pointer to strong_count (field 0)
                            llvm::Value* strongCountPtr = builder->CreateStructGEP(
                                controlBlockType,
                                controlBlockPtr,
                                0,
                                objIdent->name + "_grab_strong_count_ptr"
                            );

                            // Atomic increment: strong_count++
                            builder->CreateAtomicRMW(
                                llvm::AtomicRMWInst::Add,
                                strongCountPtr,
                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 1),
                                llvm::MaybeAlign(),
                                llvm::AtomicOrdering::AcquireRelease
                            );

                            VYB_CDBG << "DEBUG: mild<T>.grab() - incremented strong_count, returning present our<T>" << std::endl;

                            // Build the present optional carrying the control block handle.
                            llvm::Value* presentVal = llvm::UndefValue::get(optStruct);
                            presentVal = builder->CreateInsertValue(presentVal, controlBlockPtr, 0, "grab.val");
                            presentVal = builder->CreateInsertValue(presentVal, builder->getInt1(true), 1, "grab.present");
                            builder->CreateBr(grabContinue);

                            // Object freed: return the absent optional (`?`)
                            builder->SetInsertPoint(objFreedBlock);
                            VYB_CDBG << "DEBUG: mild<T>.grab() - object freed, returning absent optional" << std::endl;
                            llvm::Value* absentVal = llvm::UndefValue::get(optStruct);
                            absentVal = builder->CreateInsertValue(
                                absentVal, llvm::Constant::getNullValue(controlBlockPtr->getType()), 0, "grab.empty");
                            absentVal = builder->CreateInsertValue(absentVal, builder->getInt1(false), 1, "grab.absent");
                            builder->CreateBr(grabContinue);

                            // Continue block: phi node to select the optional our<T>? result
                            builder->SetInsertPoint(grabContinue);
                            llvm::PHINode* resultPhi = builder->CreatePHI(optStruct, 2, "grab_result");
                            resultPhi->addIncoming(presentVal, objAliveBlock);
                            resultPhi->addIncoming(absentVal, objFreedBlock);

                            m_currentLLVMValue = resultPhi;
                            return;
                        } else if (methodName == "released") {
                            // mild<T>.released() -> returns Bool
                            // Check object_freed flag in control block
                            VYB_CDBG << "DEBUG: mild<T>.released() - checking object_freed flag" << std::endl;

                            // Get the mild<T> value (control block pointer) from namedValues
                            auto objIt = namedValues.find(objIdent->name);
                            if (objIt == namedValues.end()) {
                                logError(node->loc, "Unknown variable: " + objIdent->name);
                                return;
                            }

                            // Load the control block pointer
                            llvm::Value* controlBlockPtr = builder->CreateLoad(
                                llvm::PointerType::get(*context, 0),
                                objIt->second,
                                objIdent->name + "_released_cb_load"
                            );

                            // Reconstruct control block type: { i32, i32, i8, ptr }
                            std::vector<llvm::Type*> cbFields = {
                                llvm::Type::getInt32Ty(*context),  // strong_count
                                llvm::Type::getInt32Ty(*context),  // weak_count
                                llvm::Type::getInt8Ty(*context),   // object_freed (i8 for atomic)
                                llvm::PointerType::get(*context, 0) // object_ptr
                            };
                            llvm::StructType* controlBlockType = llvm::StructType::get(*context, cbFields, /*isPacked=*/false);

                            // Get pointer to object_freed flag (field 2)
                            llvm::Value* objectFreedPtr = builder->CreateStructGEP(controlBlockType, controlBlockPtr, 2,
                                objIdent->name + "_released_obj_freed_ptr");

                            // Load the object_freed flag with atomic acquire semantics
                            llvm::LoadInst* objectFreedValue = builder->CreateLoad(
                                llvm::Type::getInt8Ty(*context),
                                objectFreedPtr,
                                objIdent->name + "_released_obj_freed"
                            );
                            objectFreedValue->setAtomic(llvm::AtomicOrdering::Acquire);

                            // Convert i8 to i1 (bool) for return
                            llvm::Value* boolValue = builder->CreateICmpNE(
                                objectFreedValue,
                                llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0),
                                objIdent->name + "_released_bool"
                            );

                            // Return the flag value (true if freed, false if alive)
                            m_currentLLVMValue = boolValue;
                            VYB_CDBG << "DEBUG: mild<T>.released() - returning object_freed flag" << std::endl;
                            return;
                        }
                    }
                }
            }
        }
    }

    // Check if this is an aspect method call (including generic aspects)
    if (auto memberExpr = dynamic_cast<vyb::ast::MemberExpression*>(node->callee.get())) {
        if (auto objIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->object.get())) {
            if (auto methodIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->property.get())) {
                std::string methodName = methodIdent->name;

                // Determine whether this is a qualified aspect call
                // (Aspect::method(receiver, ...)) or an unqualified method
                // call (receiver.method(...)).
                auto objIt = namedValues.find(objIdent->name);
                SemanticAnalyzer* semantic = driver_.hasSemanticAnalyzer()
                    ? driver_.getSemanticAnalyzer() : nullptr;

                bool isQualifiedAspectCall = (objIt == namedValues.end()) && semantic &&
                    semantic->getTraitRegistry().count(objIdent->name) != 0;

                llvm::Value* receiverAlloca = nullptr;
                llvm::Value* receiverValue = nullptr; // loaded receiver struct (qualified)
                std::string concreteType;
                std::string callDebug = objIdent->name + "." + methodName + "()";

                if (!isQualifiedAspectCall) {
                    // Unqualified: the receiver is a variable in namedValues.
                    if (objIt != namedValues.end()) {
                        receiverAlloca = objIt->second;
                        auto typeMapIt = valueTypeMap.find(receiverAlloca);
                        if (typeMapIt != valueTypeMap.end() && typeMapIt->second) {
                            concreteType = typeMapIt->second->toString();
                        } else if (typeOfNode(objIdent)) {
                            concreteType = typeOfNode(objIdent)->toString();
                        }
                    }
                } else {
                    // Qualified: the receiver is the first call argument.
                    callDebug = objIdent->name + "::" + methodName + "()";
                    if (node->arguments.empty()) {
                        logError(node->loc, "Qualified aspect call " + objIdent->name +
                                 "::" + methodName + "() requires a receiver as the first argument.");
                        m_currentLLVMValue = nullptr;
                        return;
                    }
                    node->arguments[0]->accept(*this);
                    receiverValue = m_currentLLVMValue;
                    if (!receiverValue) {
                        logError(node->loc, "Failed to evaluate the receiver of qualified aspect call " +
                                 objIdent->name + "::" + methodName + "().");
                        m_currentLLVMValue = nullptr;
                        return;
                    }
                    auto typeMapIt = valueTypeMap.find(receiverValue);
                    if (typeMapIt != valueTypeMap.end() && typeMapIt->second) {
                        concreteType = typeMapIt->second->toString();
                    } else if (typeOfNode(node->arguments[0])) {
                        concreteType = typeOfNode(node->arguments[0])->toString();
                    }
                }

                if (concreteType.empty()) {
                    VYB_CDBG << "DEBUG: No type for aspect method call: " << callDebug << std::endl;
                    if (!isQualifiedAspectCall) {
                        // Fall through so the general member-expression path can
                        // produce the appropriate diagnostic for non-aspect calls.
                    } else {
                        logError(node->loc, "Cannot determine the receiver type of qualified aspect call " +
                                 callDebug);
                        m_currentLLVMValue = nullptr;
                        return;
                    }
                } else {
                    VYB_CDBG << "DEBUG: Checking aspect method call: " << callDebug
                              << " on type " << concreteType << std::endl;

                    // Ownership-wrapped Vec receiver (their<Vec<T>>, my<Vec<T>>,
                    // view<Vec<T>>, our<Vec<T>>, borrow): an aspect/bind method bound
                    // to the Vec (VecOps/VecHigherOps in-place forms) is monomorphized
                    // for the *unwrapped* inner Vec type. Normalize concreteType to the
                    // inner Vec so the bind/mangle/monomorphize lookups succeed, and
                    // remember that the receiver's alloca holds a Vec* view so we pass
                    // the loaded pointer (not the alloca address) to the by-ref self.
                    bool ownershipWrappedVec = false;
                    bool isOwnershipKw = concreteType.rfind("their<", 0) == 0 ||
                                         concreteType.rfind("my<", 0) == 0 ||
                                         concreteType.rfind("our<", 0) == 0 ||
                                         concreteType.rfind("view<", 0) == 0 ||
                                         concreteType.rfind("borrow<", 0) == 0;
                    if (isOwnershipKw && !isQualifiedAspectCall) {
                        size_t lt = concreteType.find('<');
                        size_t gt = concreteType.rfind('>');
                        if (lt != std::string::npos && gt != std::string::npos && gt > lt) {
                            std::string inner = concreteType.substr(lt + 1, gt - lt - 1);
                            bool innerIsVec = inner == "Vec" || inner.rfind("Vec<", 0) == 0;
                            if (innerIsVec) {
                                concreteType = inner;
                                ownershipWrappedVec = true;
                            }
                        }
                    }

                    // Collect the candidate aspect(s) that provide this method for
                    // the concrete type. Qualified calls only consider the explicit
                    // aspect; unqualified calls consider every bound aspect.
                    std::vector<std::string> candidateTraits;
                    auto pushIfProvides = [&](const std::string& trait, bool inImpl) {
                        if (!inImpl) {
                            auto traitIt = semantic->getTraitRegistry().find(trait);
                            if (traitIt != semantic->getTraitRegistry().end()) {
                                for (const auto& tm : traitIt->second->methods) {
                                    if (tm.name == methodName && tm.hasDefaultImpl) {
                                        inImpl = true;
                                        break;
                                    }
                                }
                            }
                        }
                        if (inImpl) candidateTraits.push_back(trait);
                    };

                    if (isQualifiedAspectCall) {
                        candidateTraits.push_back(objIdent->name);
                    } else if (semantic) {
                        // Concrete impls bound to this type.
                        const auto& impls = semantic->getTraitImpls();
                        auto limpl = impls.find(concreteType);
                        if (limpl != impls.end()) {
                            for (const auto& traitEntry : limpl->second) {
                                bool has = false;
                                for (const ast::FunctionDeclaration* m : traitEntry.second) {
                                    if (m && m->id && m->id->name == methodName) { has = true; break; }
                                }
                                pushIfProvides(traitEntry.first, has);
                            }
                        }
                        // Generic impl patterns matching this concrete type.
                        TypePattern concretePattern = TypePattern::parse(concreteType);
                        const auto& genImpls = semantic->getGenericTraitImpls();
                        for (const auto& typeEntry : genImpls) {
                            TypePattern tmplPattern = TypePattern::parse(typeEntry.first);
                            std::map<std::string, std::string> sub;
                            if (!tmplPattern.matchesPattern(concretePattern, sub)) continue;
                            for (const auto& traitEntry : typeEntry.second) {
                                const GenericImplInfo* gii = traitEntry.second.get();
                                bool has = false;
                                if (gii && gii->declaration) {
                                    for (const auto& m : gii->declaration->methods) {
                                        if (m && m->id && m->id->name == methodName) { has = true; break; }
                                    }
                                }
                                pushIfProvides(traitEntry.first, has);
                            }
                        }
                    }

                    llvm::Function* implFunc = nullptr;
                    for (const std::string& trait : candidateTraits) {
                        std::string mangledName = concreteType + "_" + trait + "_" + methodName;
                        implFunc = module->getFunction(mangledName);
                        if (implFunc) {
                            VYB_CDBG << "DEBUG: Found aspect method implementation: "
                                      << mangledName << std::endl;
                            break;
                        }
                        if (semantic) {
                            implFunc = monomorphizeTraitMethod(concreteType, trait, methodName);
                            if (implFunc) {
                                VYB_CDBG << "DEBUG: Monomorphized aspect method for trait "
                                          << trait << " on " << concreteType << std::endl;
                                break;
                            }
                        }
                    }

                    if (implFunc) {
                        // Build arguments: first argument is the receiver struct.
                        // When the bind's receiver is declared by reference (an
                        // ownership-qualified receiver such as `self<their<T>>`),
                        // the specialized method's first parameter is a pointer to the
                        // receiver. Pass the receiver's address (its alloca) so
                        // in-place mutations to `self` persist on the caller's object
                        // instead of loading a by-value copy that is discarded.
                        bool selfIsByRef = implFunc->getArg(0)->getType()->isPointerTy();
                        std::vector<llvm::Value*> argValues;
                        if (isQualifiedAspectCall) {
                            argValues.push_back(receiverValue);
                        } else if (ownershipWrappedVec && receiverAlloca) {
                            // The wrapper alloca holds a single Vec* view. A by-ref
                            // `self<their<Vec<T>>>` parameter expects that pointer, but a
                            // by-value `self<Vec<T>>` bind (e.g. `bind<T> Bag -> Vec<T>`)
                            // expects the pointee loaded by value -- passing the raw view
                            // pointer made the call's parameter type disagree with the
                            // callee signature (#254).
                            llvm::Value* viewPtr = nullptr;
                            if (auto at = llvm::dyn_cast<llvm::AllocaInst>(receiverAlloca)) {
                                viewPtr = builder->CreateLoad(
                                    at->getAllocatedType(), receiverAlloca,
                                    objIdent->name + ".view.load");
                            } else {
                                viewPtr = receiverAlloca;
                            }
                            if (selfIsByRef) {
                                argValues.push_back(viewPtr);
                            } else {
                                argValues.push_back(builder->CreateLoad(
                                    implFunc->getArg(0)->getType(), viewPtr,
                                    objIdent->name + ".self.load"));
                            }
                        } else if (selfIsByRef && receiverAlloca) {
                            argValues.push_back(receiverAlloca);
                        } else if (auto allocaType = llvm::dyn_cast<llvm::AllocaInst>(receiverAlloca)) {
                            argValues.push_back(builder->CreateLoad(
                                allocaType->getAllocatedType(),
                                receiverAlloca,
                                objIdent->name + ".load"));
                        } else {
                            argValues.push_back(receiverAlloca);
                        }

                        size_t argStart = isQualifiedAspectCall ? 1u : 0u;
                        std::vector<llvm::Value*> transientClosures;
                        for (size_t i = argStart; i < node->arguments.size(); ++i) {
                            auto& arg = node->arguments[i];
                            arg->accept(*this);
                            if (!m_currentLLVMValue) {
                                logError(arg->loc, "Argument codegen failed for aspect method " + methodName);
                                m_currentLLVMValue = nullptr;
                                return;
                            }
                            llvm::Value* argVal = m_currentLLVMValue;
                            // A fresh closure literal passed as an argument starts
                            // with a zero refcount; an aspect/trait method borrows it
                            // but does not retain/release the env (it only calls the
                            // fn). Retain each such argument before the call and
                            // release it after so the transient environment is
                            // reclaimed instead of leaking.
                            if (dynamic_cast<ast::FunctionExpression*>(arg.get()) &&
                                argVal->getType()->isStructTy() &&
                                isClosureStructType(argVal->getType())) {
                                retainClosureValue(argVal);
                                transientClosures.push_back(argVal);
                            }
                            // When the parameter is a Vyb String { ptr, len } and the
                            // evaluated argument is a raw char* (e.g. to_string(),
                            // substring, concat), wrap it into the String struct;
                            // literals and String variables already arrive as a struct.
                            if (argVal->getType()->isPointerTy()) {
                                llvm::Type* expectedParam = implFunc->getFunctionType()->getParamType(argValues.size());
                                if (expectedParam && expectedParam->isStructTy()) {
                                    llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(expectedParam);
                                    if (st && st->getNumElements() == 2 &&
                                        st->getElementType(0)->isPointerTy() &&
                                        st->getElementType(1)->isIntegerTy(64)) {
                                        llvm::Value* wrapped = tryCast(argVal, expectedParam, arg->loc);
                                        if (wrapped) argVal = wrapped;
                                    }
                                }
                            }
                            argValues.push_back(argVal);
                        }
                        // #255: an aspect method called with the wrong number of
                        // arguments must not reach the verifier -- an unmatched call
                        // fails LLVM verification and the compiler died on the invalid
                        // IR (SIGSEGV). Diagnose and halt instead. argValues includes
                        // the receiver as parameter 0.
                        if (argValues.size() != implFunc->getFunctionType()->getNumParams()) {
                            const size_t want = implFunc->getFunctionType()->getNumParams() - 1;
                            logError(node->loc, concreteType + "::" + methodName + " expects exactly " +
                                     std::to_string(want) + " argument" + (want == 1 ? "" : "s") + ", got " +
                                     std::to_string(argValues.size() - 1) + ".");
                            flagHardCodegenError();
                            m_currentLLVMValue = nullptr;
                            return;
                        }

                        if (implFunc->getReturnType()->isVoidTy()) {
                            builder->CreateCall(implFunc, argValues);
                            m_currentLLVMValue = nullptr;
                        } else {
                            m_currentLLVMValue = builder->CreateCall(implFunc, argValues, "aspect.method.result");
                        }
                        for (auto* tc : transientClosures) {
                            releaseClosureValue(tc);
                        }

                        VYB_CDBG << "DEBUG: Successfully generated call to aspect method: "
                                  << methodName << " for type " << concreteType << std::endl;
                        return;
                    }

                    VYB_CDBG << "DEBUG: No aspect implementation found for " << callDebug
                              << " on type " << concreteType << std::endl;
                }
            }
        }
    }

    // First, check if this is an intrinsic function call
    auto identCallee = dynamic_cast<vyb::ast::Identifier*>(node->callee.get());
    std::string calleeName = node->callee->toString();



    // Special handling for intrinsic functions with potential variable name conflicts
    if (identCallee && node->arguments.size() == 1) {
        // Check for addr() intrinsic
        if (identCallee->name == "addr") {
            // First evaluate the argument expression
            node->arguments[0]->accept(*this);
            if (!m_currentLLVMValue) {
                logError(node->arguments[0]->loc, "Argument to addr() evaluated to null");
                return;
            }

            // If the argument is a pointer type
            if (m_currentLLVMValue->getType()->isPointerTy()) {
                llvm::Value* pointerValue = m_currentLLVMValue;

                // For pointer-to-pointer types (like loc<T>), load the pointer first
                if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(m_currentLLVMValue)) {
                    if (allocaInst->getAllocatedType()->isPointerTy()) {
                        pointerValue = builder->CreateLoad(allocaInst->getAllocatedType(),
                                                        m_currentLLVMValue,
                                                        "ptr_load_for_addr");
                    }
                }

                // Convert to integer value (address)
                m_currentLLVMValue = builder->CreatePtrToInt(pointerValue, int64Type, "addr_cast");

                // If we're not in an assignment context, create an alloca to store the result
                if (!m_isLHSOfAssignment) {
                    llvm::Value* tempAlloca = builder->CreateAlloca(int64Type, nullptr, "addr_temp");
                    builder->CreateStore(m_currentLLVMValue, tempAlloca);
                    m_currentLLVMValue = tempAlloca;
                }
                return;
            }
        }
        // Check for at() intrinsic
        else if (identCallee->name == "at") {
            // First evaluate the argument expression to get the pointer
            node->arguments[0]->accept(*this);
            if (!m_currentLLVMValue) {
                logError(node->arguments[0]->loc, "Argument to at() evaluated to null");
                return;
            }

            // If we have a valid pointer, handle it appropriately
            if (m_currentLLVMValue->getType()->isPointerTy()) {
                // For at(p) where p is a loc<T> (i.e., a pointer-to-pointer),
                // we need to load the pointer value first
                llvm::Value* pointerValue = m_currentLLVMValue;

                // AllocaInst for a loc<T> produces a pointer-to-pointer.
                // Check if we're dealing with a pointer-to-pointer (i.e., loc<T>)
                bool isPointerToPointer = false;
                if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(m_currentLLVMValue)) {
                    if (allocaInst->getAllocatedType()->isPointerTy()) {
                        isPointerToPointer = true;
                        // Load the pointer value
                        pointerValue = builder->CreateLoad(allocaInst->getAllocatedType(),
                                               m_currentLLVMValue,
                                               "ptr_val");
                    }
                }

                // Add a null check for the pointer value
                llvm::Function* currentFn = builder->GetInsertBlock()->getParent();
                llvm::BasicBlock* nonNullBB = llvm::BasicBlock::Create(*context, "ptr.not_null", currentFn);
                llvm::BasicBlock* nullBB = llvm::BasicBlock::Create(*context, "ptr.null", currentFn);
                llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "ptr.merge", currentFn);

                // Create the null check condition
                llvm::Value* isNotNull = builder->CreateIsNotNull(pointerValue, "ptr.is_not_null");
                builder->CreateCondBr(isNotNull, nonNullBB, nullBB);

                // Set up the non-null block
                builder->SetInsertPoint(nonNullBB);

                if (m_isLHSOfAssignment) {
                    // In an assignment context, return the pointer itself
                    m_currentLLVMValue = pointerValue;
                } else {
                    // Otherwise dereference the pointer (load)
                    llvm::Type* loadTy = int64Type; // Default to Int for tests

                    // Try to determine the appropriate load type
                    if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(pointerValue)) {
                        loadTy = allocaInst->getAllocatedType();
                    } else if (typeOfNode(node->arguments[0])) {
                        // Try to determine type from AST node
                        if (auto ptrType = dynamic_cast<ast::PointerType*>(typeOfNode(node->arguments[0]).get())) {
                            if (ptrType->pointeeType) {
                                loadTy = codegenType(ptrType->pointeeType.get());
                            }
                        } else if (auto typeName = dynamic_cast<ast::TypeName*>(typeOfNode(node->arguments[0]).get())) {
                            // Handle loc<T> type
                            if (typeName->identifier->name == "loc" && !typeName->genericArgs.empty()) {
                                loadTy = codegenType(typeName->genericArgs[0].get());
                            }
                        }
                    }

                    m_currentLLVMValue = builder->CreateLoad(loadTy, pointerValue, "deref.load");
                }

                builder->CreateBr(mergeBB);

                // Set up the null block
                builder->SetInsertPoint(nullBB);
                builder->CreateUnreachable();

                // Set up the merge block
                builder->SetInsertPoint(mergeBB);
                return;
            } else {
                logError(node->loc, "at() called on non-pointer type. Got: " + getTypeName(m_currentLLVMValue->getType()));
                m_currentLLVMValue = nullptr;
                return;
            }
        }
        // Check for loc() intrinsic
        else if (identCallee->name == "loc") {
            // First evaluate the argument to get its address
            auto ident = dynamic_cast<vyb::ast::Identifier*>(node->arguments[0].get());
            if (ident) {
                auto it = namedValues.find(ident->name);
                if (it != namedValues.end()) {
                    // Return the address of the alloca directly
                    m_currentLLVMValue = it->second;
                    return;
                }
            }

            // If we couldn't find the variable directly, try evaluating the argument generically
            node->arguments[0]->accept(*this);
            if (!m_currentLLVMValue) {
                logError(node->arguments[0]->loc, "Argument to loc() evaluated to null");
                return;
            }

            // The result of loc() is the pointer to the value
            if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(m_currentLLVMValue)) {
                // If the value is already an alloca instruction, just use it directly
                return;
            } else {
                // For non-alloca values, we need to create temporary storage
                llvm::Type* valType = m_currentLLVMValue->getType();
                llvm::Value* tempAlloca = builder->CreateAlloca(valType, nullptr, "loc_temp");
                builder->CreateStore(m_currentLLVMValue, tempAlloca);
                m_currentLLVMValue = tempAlloca;
            }
            return;
        }
    }

    // Handle from<T>() intrinsic - more complex because it requires a type parameter
    if (identCallee && identCallee->name == "from" && node->arguments.size() == 1) {
        // Evaluate the address expression
        node->arguments[0]->accept(*this);
        if (!m_currentLLVMValue) {
            logError(node->arguments[0]->loc, "from<T>() operand evaluated to null");
            return;
        }

        if (!m_currentLLVMValue->getType()->isIntegerTy()) {
            logError(node->arguments[0]->loc, "from<T>() requires an integer address argument. Got: " +
                                           getTypeName(m_currentLLVMValue->getType()));
            return;
        }

        // Default to i64* pointer type
        llvm::Type* ptrTy = llvm::PointerType::getUnqual(int64Type);

        // Convert integer to pointer
        m_currentLLVMValue = builder->CreateIntToPtr(m_currentLLVMValue, ptrTy, "from_cast");
        return;
    }

    // Handle ownership constructors: my(), their(), our()
    if (identCallee && (identCallee->name == "my" || identCallee->name == "their" || identCallee->name == "our") && node->arguments.size() == 1) {
        VYB_CDBG << "DEBUG: Processing ownership constructor " << identCallee->name << "() in LLVM codegen" << std::endl;

        // Evaluate the argument to get the struct value
        node->arguments[0]->accept(*this);
        if (!m_currentLLVMValue) {
            logError(node->arguments[0]->loc, "Argument to " + identCallee->name + "() evaluated to null");
            return;
        }

        llvm::Value* structValue = m_currentLLVMValue;
        llvm::Type* structType = structValue->getType();

        // Primitive ownership values are stored inline (e.g. my<Int> is just an
        // i64) - pass the value through rather than allocating a heap box or
        // control block, matching the value-typed ownership representation.
        if (structType->isIntegerTy() || structType->isFloatTy() || structType->isDoubleTy()) {
            m_currentLLVMValue = structValue;
            VYB_CDBG << "DEBUG: Ownership constructor " << identCallee->name
                      << "() over primitive passes value through" << std::endl;
            return;
        }

        if (identCallee->name == "my") {
            // For my(), allocate memory on heap and store the struct value
            if (!structType->isStructTy()) {
                logError(node->loc, "my() can only be used with struct types");
                return;
            }

            // Allocate memory for the struct on the heap
            llvm::Function* mallocFunc = module->getFunction("malloc");
            if (!mallocFunc) {
                // Declare malloc if not already declared
                llvm::FunctionType* mallocType = llvm::FunctionType::get(
                    llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0),
                    {llvm::Type::getInt64Ty(*context)},
                    false
                );
                mallocFunc = llvm::Function::Create(
                    mallocType,
                    llvm::Function::ExternalLinkage,
                    "malloc",
                    module.get()
                );
            }

            // Calculate size of struct
            llvm::DataLayout dataLayout(module.get());
            uint64_t structSize = dataLayout.getTypeAllocSize(structType);
            llvm::Value* sizeValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), structSize);

            // Call malloc
            llvm::Value* mallocPtr = builder->CreateCall(mallocFunc, {sizeValue}, "malloc_struct");

            // Cast malloc result to struct pointer type
            llvm::Type* structPtrType = llvm::PointerType::get(structType, 0);
            llvm::Value* structPtr = builder->CreateBitCast(mallocPtr, structPtrType, "struct_ptr");

            // Store the struct value into allocated memory
            builder->CreateStore(structValue, structPtr);

            // Return the pointer
            m_currentLLVMValue = structPtr;
            VYB_CDBG << "DEBUG: Successfully processed ownership constructor my() - allocated and returned pointer" << std::endl;
        } else if (identCallee->name == "our") {
            // For our(), allocate control block + object on heap
            if (!structType->isStructTy()) {
                logError(node->loc, "our() can only be used with struct types");
                return;
            }

            // Get malloc function
            llvm::Function* mallocFunc = getOrCreateMallocFunction();
            llvm::DataLayout dataLayout(module.get());

            // 1. Allocate memory for the object
            uint64_t objectSize = dataLayout.getTypeAllocSize(structType);
            llvm::Value* objectSizeValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), objectSize);
            llvm::Value* mallocObjectPtr = builder->CreateCall(mallocFunc, {objectSizeValue}, "malloc_object");
            llvm::Type* objectPtrType = llvm::PointerType::get(structType, 0);
            llvm::Value* objectPtr = builder->CreateBitCast(mallocObjectPtr, objectPtrType, "object_ptr");

            // Store the struct value into allocated memory
            builder->CreateStore(structValue, objectPtr);

            // 2. Allocate memory for control block
            llvm::StructType* controlBlockType = getControlBlockType(objectPtrType);
            uint64_t cbSize = dataLayout.getTypeAllocSize(controlBlockType);
            llvm::Value* cbSizeValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), cbSize);
            llvm::Value* mallocCBPtr = builder->CreateCall(mallocFunc, {cbSizeValue}, "malloc_cb");
            llvm::Type* cbPtrType = llvm::PointerType::get(controlBlockType, 0);
            llvm::Value* cbPtr = builder->CreateBitCast(mallocCBPtr, cbPtrType, "cb_ptr");

            // 3. Initialize control block fields: { strong_count=1, weak_count=0, object_freed=false, object_ptr }
            llvm::Value* strongCountPtr = builder->CreateStructGEP(controlBlockType, cbPtr, 0, "strong_count_ptr");
            builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 1), strongCountPtr);

            llvm::Value* weakCountPtr = builder->CreateStructGEP(controlBlockType, cbPtr, 1, "weak_count_ptr");
            builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0), weakCountPtr);

            llvm::Value* objectFreedPtr = builder->CreateStructGEP(controlBlockType, cbPtr, 2, "object_freed_ptr");
            builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0), objectFreedPtr);

            llvm::Value* objectPtrFieldPtr = builder->CreateStructGEP(controlBlockType, cbPtr, 3, "object_ptr_field_ptr");
            builder->CreateStore(objectPtr, objectPtrFieldPtr);

            // 4. Return the control block pointer (our<T> is represented as control block pointer)
            m_currentLLVMValue = cbPtr;
            VYB_CDBG << "DEBUG: Successfully processed ownership constructor our() - allocated control block and object" << std::endl;
        } else if (identCallee->name == "their") {
            // For their(), just pass through the value for now
            // In a real implementation, these would have different semantics
            VYB_CDBG << "DEBUG: Successfully processed ownership constructor their()" << std::endl;
        } else {
            // For their() and our(), just pass through the value for now
            // In a real implementation, these would have different semantics
            VYB_CDBG << "DEBUG: Successfully processed ownership constructor " << identCallee->name << "()" << std::endl;
        }
        return;
    }

    // Handle borrowing operations: borrow(), view()
    if (identCallee && (identCallee->name == "borrow" || identCallee->name == "view") && node->arguments.size() == 1) {
        VYB_CDBG << "DEBUG: Processing borrowing operation " << identCallee->name << "() in LLVM codegen" << std::endl;

        // Evaluate the argument to get the value to borrow
        node->arguments[0]->accept(*this);
        if (!m_currentLLVMValue) {
            logError(node->arguments[0]->loc, "Argument to " + identCallee->name + "() evaluated to null");
            return;
        }

        llvm::Value* valueToBorrow = m_currentLLVMValue;

        // A borrow addresses the OBJECT, not the variable slot that holds a
        // pointer to it (see borrowTargetPointer, #365 step 0).
        if (!valueToBorrow || !valueToBorrow->getType()->isPointerTy()) {
            logError(node->loc, identCallee->name + "() requires an lvalue (something that can be borrowed)");
            return;
        }
        m_currentLLVMValue = borrowTargetPointer(valueToBorrow, typeOfNode(node->arguments[0]).get(),
                                                 identCallee->name);
        return;
    }

    // Handle soft() operation: creates mild<T> from our<T>
    if (identCallee && identCallee->name == "soft" && node->arguments.size() == 1) {
        VYB_CDBG << "DEBUG: Processing soft() operation in LLVM codegen" << std::endl;

        // Evaluate the argument to get the our<T> value (control block pointer)
        node->arguments[0]->accept(*this);
        if (!m_currentLLVMValue) {
            logError(node->arguments[0]->loc, "Argument to soft() evaluated to null");
            return;
        }

        llvm::Value* controlBlockPtr = m_currentLLVMValue;

        // Soft() increments weak_count in the control block and returns mild<T>
        // Control block: { i32 strong_count, i32 weak_count, i8 object_freed, ptr object_ptr }

        // Reconstruct control block type
        std::vector<llvm::Type*> cbFields = {
            llvm::Type::getInt32Ty(*context),  // strong_count
            llvm::Type::getInt32Ty(*context),  // weak_count
            llvm::Type::getInt8Ty(*context),   // object_freed (i8 for atomic)
            llvm::PointerType::get(*context, 0) // object_ptr
        };
        llvm::StructType* controlBlockType = llvm::StructType::get(*context, cbFields, /*isPacked=*/false);

        // Get pointer to weak_count (field 1)
        llvm::Value* weakCountPtr = builder->CreateStructGEP(controlBlockType, controlBlockPtr, 1,
            "soft_weak_count_ptr");

        // Atomic increment: weak_count++
        builder->CreateAtomicRMW(
            llvm::AtomicRMWInst::Add,
            weakCountPtr,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 1),
            llvm::MaybeAlign(),
            llvm::AtomicOrdering::AcquireRelease
        );

        // Return the control block pointer as mild<T>
        // Both our<T> and mild<T> are represented as control block pointers
        m_currentLLVMValue = controlBlockPtr;
        // A temp `our(...)` fed straight into soft() owns a strong ref this
        // operation does not take. Drop it so the object is released once the
        // strong count reaches zero (and a returned weak sees .released()).
        if (freshOwningCallArgKind(node->arguments[0].get()) == 1) {
            const vyb::ast::TypeNode* pointeeAst = nullptr;
            llvm::Type* pointeeLlvm = nullptr;
            if (typeOfNode(node->arguments[0])) {
                pointeeAst = ourPointeeOf(typeOfNode(node->arguments[0]).get());
                if (pointeeAst) {
                    pointeeLlvm = codegenType(const_cast<vyb::ast::TypeNode*>(pointeeAst));
                }
            }
            releaseOurControlBlock(controlBlockPtr, "soft.temparg", pointeeAst, pointeeLlvm);
        }
        VYB_CDBG << "DEBUG: Successfully processed soft() operation - incremented weak_count and returned mild<T> pointer" << std::endl;
        return;
    }

    // Special handling for println with auto-serialization
    if (identCallee && identCallee->name == "println" && node->arguments.size() >= 1) {
        // Helper lambda to serialize one argument to char* for printing
        auto serializeOneArg = [&](ast::ExprPtr& argExpr, bool* outHeap, bool* outStringOwned) -> llvm::Value* {
            if (outHeap) *outHeap = false;
            if (outStringOwned) *outStringOwned = false;
            llvm::Value* arg = nullptr;
            // For array arguments, get pointer directly
            if (typeOfNode(argExpr) && dynamic_cast<ast::ArrayType*>(typeOfNode(argExpr).get())) {
                if (auto* identArg = dynamic_cast<ast::Identifier*>(argExpr.get())) {
                    auto it = namedValues.find(identArg->name);
                    if (it != namedValues.end()) arg = it->second;
                    else {
                        auto funcIt = m_currentFunctionNamedValues.find(identArg->name);
                        if (funcIt != m_currentFunctionNamedValues.end()) arg = funcIt->second;
                    }
                }
            }
            if (!arg) {
                argExpr->accept(*this);
                arg = m_currentLLVMValue;
            }
            if (!arg) return nullptr;
            llvm::Value* serialized = nullptr;
            if (typeOfNode(argExpr)) {
                auto* argType = typeOfNode(argExpr).get();
                std::string typeStr = argType->toString();
                if (typeStr == "string" || typeStr == "String") {
                    serialized = arg->getType()->isStructTy()
                        ? builder->CreateExtractValue(arg, 0, "str.ptr")
                        : arg;
                    if (outStringOwned) *outStringOwned = exprProducesOwnedStringTemp(argExpr.get());
                } else if (auto* arrayType = dynamic_cast<ast::ArrayType*>(argType)) {
                    serialized = generateArraySerialization(arg, arrayType);
                } else if (arg->getType()->isPointerTy() && arg->getType() == int8PtrType) {
                    serialized = arg;
                } else {
                    serialized = generateToStringCall(arg, arg->getType(), argType, argExpr->loc);
                    if (!serialized) serialized = generateGenericSerialization(arg, argType);
                    if (outHeap) *outHeap = true;
                }
            } else {
                if (arg->getType()->isPointerTy() && arg->getType() == int8PtrType) {
                    serialized = arg;
                } else if (arg->getType()->isStructTy() && arg->getType()->getStructNumElements() == 2 &&
                           arg->getType()->getStructElementType(0)->isPointerTy()) {
                    serialized = builder->CreateExtractValue(arg, 0, "str.ptr");
                } else {
                    serialized = generateToStringCall(arg, arg->getType(), nullptr, argExpr->loc);
                    if (!serialized) serialized = generateGenericSerialization(arg, nullptr);
                    if (outHeap) *outHeap = true;
                }
            }
            return serialized;
        };

        if (node->arguments.size() == 1) {
            // Single argument: use existing serialization logic
            llvm::Value* arg = nullptr;
            // For array arguments, we need the pointer, not the loaded value
            if (typeOfNode(node->arguments[0])) {
            auto* argType = typeOfNode(node->arguments[0]).get();
            if (dynamic_cast<ast::ArrayType*>(argType)) {
                // For arrays, get the alloca pointer directly instead of loading
                if (auto* identArg = dynamic_cast<ast::Identifier*>(node->arguments[0].get())) {
                    auto it = namedValues.find(identArg->name);
                    if (it != namedValues.end()) {
                        arg = it->second; // This is the alloca pointer
                    } else {
                        auto funcIt = m_currentFunctionNamedValues.find(identArg->name);
                        if (funcIt != m_currentFunctionNamedValues.end()) {
                            arg = funcIt->second; // This is the alloca pointer
                        }
                    }
                }
            }
        }

        // If we didn't get the array pointer above, evaluate normally
        if (!arg) {
            node->arguments[0]->accept(*this);
            arg = m_currentLLVMValue;
        }

        if (!arg) {
            logError(node->arguments[0]->loc, "Argument to println() evaluated to null");
            return;
        }

        llvm::Value* serializedValue = nullptr;
        bool serializedTmpIsHeap = false;
        bool stringArgOwnedTemp = exprProducesOwnedStringTemp(node->arguments[0].get());

        // Check for string type first (Vyb string struct {ptr, len})
        if (typeOfNode(node->arguments[0])) {
            auto* argType = typeOfNode(node->arguments[0]).get();
            std::string typeStr = argType->toString();

            // Priority 1: Check if it's a Vyb string type
            if (typeStr == "string" || typeStr == "String") {
                // It's a Vyb string struct {ptr, len} - extract the ptr field
                if (arg->getType()->isStructTy()) {
                    // Extract the ptr field (index 0) from the string struct
                    serializedValue = builder->CreateExtractValue(arg, 0, "str.ptr");
                } else if (arg->getType()->isPointerTy()) {
                    // Already a char* (e.g. result of string concatenation or to_string())
                    serializedValue = arg;
                } else {
                    // Fallback for other representations
                    serializedValue = generateToStringCall(arg, arg->getType(), argType, node->loc);
                    if (!serializedValue) {
                        serializedValue = generateGenericSerialization(arg, argType);
                    }
                }
            }
            // Priority 2: Check for arrays
            else if (auto* arrayType = dynamic_cast<ast::ArrayType*>(argType)) {
                // Generate array serialization code
                serializedValue = generateArraySerialization(arg, arrayType);
            }
            // Priority 3: Check if the argument is already a string (char*) for non-array types
            else if (arg->getType()->isPointerTy() && arg->getType() == int8PtrType) {
                // It's already a string pointer (char*), use it directly
                serializedValue = arg;
            }
            // Priority 4: Convert to string via to_string() for any type (Int, Float, Bool, etc.)
            else {
                serializedValue = generateToStringCall(arg, arg->getType(), argType, node->loc);
                if (!serializedValue) {
                    // Fall back to generic serialization for complex/unrecognized types
                    serializedValue = generateGenericSerialization(arg, argType);
                }
                // #409: the struct branch of generateToStringCall returns a Vyb String
                // ({ ptr, i64 }) -- but __vyb_println is declared (const char*) and
                // __vyb_string_free likewise, so handing over the struct produced a
                // module the LLVM verifier rejects ("Call parameter type does not match
                // function signature"), and `println(<struct>)` did not compile at all.
                // Field 0 is the registered heap buffer the JSON branch returns
                // (cgen_string.cpp), so it is the right argument for *both* the print
                // below and the free that serializedTmpIsHeap triggers -- one extraction
                // used twice, with no change to ownership.
                if (serializedValue && serializedValue->getType()->isStructTy() &&
                    isVybStringStructType(serializedValue->getType())) {
                    serializedValue = builder->CreateExtractValue(serializedValue, 0, "str.ptr");
                }
                serializedTmpIsHeap = true;
            }
        }
        else {
            // No type info - check if the argument is already a string (char*)
            if (arg->getType()->isPointerTy() && arg->getType() == int8PtrType) {
                // It's already a string pointer (char*), use it directly
                serializedValue = arg;
            }
            // Check if it's a struct (might be a string struct)
            else if (arg->getType()->isStructTy() && arg->getType()->getStructNumElements() == 2 &&
                     arg->getType()->getStructElementType(0)->isPointerTy()) {
                // Might be a string struct {ptr, len} - extract ptr
                serializedValue = builder->CreateExtractValue(arg, 0, "str.ptr");
            }
            else {
                // Try to_string() first, fall back to generic serialization
                serializedValue = generateToStringCall(arg, arg->getType(), nullptr, node->loc);
                if (!serializedValue) {
                    serializedValue = generateGenericSerialization(arg, nullptr);
                }
                serializedTmpIsHeap = true;
            }
        }

        // Call println with the serialized string
        llvm::Function* printlnFunc = getVybPrintlnFunction();
        std::vector<llvm::Value*> printlnArgs = {serializedValue};
        builder->CreateCall(printlnFunc, printlnArgs);

        // Free a heap-backed serialization temporary (scalar/struct/array
        // produced by to_string/serialize helpers). String paths yield the
        // value's own buffer; only a freshly-allocated concat/.to_string() temp
        // is reclaimed (through the registry, which makes it a safe no-op for
        // .rodata literals and named-var borrows).
        if (serializedTmpIsHeap) {
            builder->CreateCall(getOrCreateVybStringFreeFunction(), {serializedValue});
        }
        if (stringArgOwnedTemp) {
            builder->CreateCall(getOrCreateVybStringFreeFunction(), {serializedValue});
        }

        m_currentLLVMValue = nullptr; // println returns void
        return;
        } else {
            // Multiple arguments: print each with space separator, last with newline
            llvm::Function* printFunc = getVybPrintFunction();
            llvm::Function* printlnFunc = getVybPrintlnFunction();
            // Space constant
            llvm::Value* spaceStr = builder->CreateGlobalStringPtr(" ", "println.space");
            for (size_t i = 0; i < node->arguments.size(); ++i) {
                bool serializedIsHeap = false;
                bool serializedStringOwned = false;
                llvm::Value* serialized = serializeOneArg(node->arguments[i], &serializedIsHeap, &serializedStringOwned);
                if (!serialized) {
                    logError(node->arguments[i]->loc, "Argument to println() evaluated to null");
                    m_currentLLVMValue = nullptr;
                    return;
                }
                if (i < node->arguments.size() - 1) {
                    // Print arg then space
                    builder->CreateCall(printFunc, {serialized});
                    builder->CreateCall(printFunc, {spaceStr});
                } else {
                    // Last arg: println (adds newline)
                    builder->CreateCall(printlnFunc, {serialized});
                }
                if (serializedIsHeap) {
                    builder->CreateCall(getOrCreateVybStringFreeFunction(), {serialized});
                }
                if (serializedStringOwned) {
                    builder->CreateCall(getOrCreateVybStringFreeFunction(), {serialized});
                }
            }
            m_currentLLVMValue = nullptr;
            return;
        }
    }

    // Handle print() intrinsic (no newline)
    if (identCallee && identCallee->name == "print" && node->arguments.size() >= 1) {
        llvm::Function* printFunc = getVybPrintFunction();
        llvm::Value* spaceStr = nullptr;
        if (node->arguments.size() > 1) {
            spaceStr = builder->CreateGlobalStringPtr(" ", "print.space");
        }
        for (size_t argIdx = 0; argIdx < node->arguments.size(); ++argIdx) {
            node->arguments[argIdx]->accept(*this);
            llvm::Value* arg = m_currentLLVMValue;
            if (!arg) {
                logError(node->arguments[argIdx]->loc, "Argument to print() evaluated to null");
                return;
            }
            bool argSerializedHeap = false;
            bool argStringOwned = exprProducesOwnedStringTemp(node->arguments[argIdx].get());
            // Serialize to string then print without newline
            llvm::Value* serializedValue = nullptr;
            if (typeOfNode(node->arguments[argIdx])) {
                auto* argType = typeOfNode(node->arguments[argIdx]).get();
                std::string typeStr = argType->toString();
                if (typeStr == "String" || typeStr == "string") {
                    serializedValue = arg->getType()->isStructTy()
                        ? builder->CreateExtractValue(arg, 0, "str.ptr")
                        : arg;
                } else {
                    serializedValue = generateToStringCall(arg, arg->getType(), argType, node->loc);
                    if (!serializedValue) {
                        serializedValue = generateGenericSerialization(arg, argType);
                    }
                    // #409: same extraction as the single-argument path -- a struct
                    // argument serializes to a Vyb String ({ ptr, i64 }), and the print
                    // and free calls that follow are declared (const char*). Field 0 is
                    // the registered heap buffer, so one extraction serves both.
                    if (serializedValue && serializedValue->getType()->isStructTy() &&
                        isVybStringStructType(serializedValue->getType())) {
                        serializedValue = builder->CreateExtractValue(serializedValue, 0, "str.ptr");
                    }
                    argSerializedHeap = true;
                }
            } else {
                if (arg->getType()->isPointerTy()) {
                    serializedValue = arg;
                } else if (arg->getType()->isStructTy() && arg->getType()->getStructNumElements() == 2 &&
                           arg->getType()->getStructElementType(0)->isPointerTy()) {
                    serializedValue = builder->CreateExtractValue(arg, 0, "str.ptr");
                } else {
                    serializedValue = generateToStringCall(arg, arg->getType(), nullptr, node->loc);
                    if (!serializedValue) {
                        serializedValue = generateGenericSerialization(arg, nullptr);
                    }
                    argSerializedHeap = true;
                }
            }
            if (serializedValue) {
                builder->CreateCall(printFunc, {serializedValue});
                if (argSerializedHeap) {
                    builder->CreateCall(getOrCreateVybStringFreeFunction(), {serializedValue});
                }
                if (argStringOwned) {
                    builder->CreateCall(getOrCreateVybStringFreeFunction(), {serializedValue});
                }
            }
            // Print space between args (not after last)
            if (spaceStr && argIdx < node->arguments.size() - 1) {
                builder->CreateCall(printFunc, {spaceStr});
            }
        }
        m_currentLLVMValue = nullptr;
        return;
    }

    // Handle println_int() / print_int() intrinsics - route through generic to_string path
    if (identCallee && (identCallee->name == "println_int" || identCallee->name == "print_int") && node->arguments.size() == 1) {
        node->arguments[0]->accept(*this);
        llvm::Value* arg = m_currentLLVMValue;
        if (!arg) {
            logError(node->arguments[0]->loc, "Argument to " + identCallee->name + "() evaluated to null");
            return;
        }
        if (!arg->getType()->isIntegerTy(64)) {
            // Sign-extend signed integer types; zero-extend unsigned ones so a
            // UInt8 value with its high bit set prints as itself, not as a
            // negative signed value.
            bool unsign = false;
            if (typeOfNode(node->arguments[0])) {
                if (auto tn = dynamic_cast<ast::TypeName*>(typeOfNode(node->arguments[0]).get())) {
                    if (tn->identifier) unsign = isUnsignedIntName(tn->identifier->name);
                }
            }
            arg = builder->CreateIntCast(arg, llvm::Type::getInt64Ty(*context), !unsign, "cast_i64");
        }
        llvm::Value* strVal = generateToStringCall(arg, arg->getType(), nullptr, node->loc);
        if (identCallee->name == "println_int") {
            builder->CreateCall(getVybPrintlnFunction(), {strVal});
        } else {
            builder->CreateCall(getVybPrintFunction(), {strVal});
        }
        builder->CreateCall(getOrCreateVybStringFreeFunction(), {strVal});
        m_currentLLVMValue = nullptr;
        return;
    }

    // Handle println_bool() / print_bool() intrinsics - route through generic to_string path
    if (identCallee && (identCallee->name == "println_bool" || identCallee->name == "print_bool") && node->arguments.size() == 1) {
        node->arguments[0]->accept(*this);
        llvm::Value* arg = m_currentLLVMValue;
        if (!arg) {
            logError(node->arguments[0]->loc, "Argument to " + identCallee->name + "() evaluated to null");
            return;
        }
        // Ensure i1 for bool to_string conversion
        if (!arg->getType()->isIntegerTy(1)) {
            arg = builder->CreateICmpNE(arg, llvm::ConstantInt::get(arg->getType(), 0), "to_bool");
        }
        llvm::Value* strVal = generateToStringCall(arg, arg->getType(), nullptr, node->loc);
        if (identCallee->name == "println_bool") {
            builder->CreateCall(getVybPrintlnFunction(), {strVal});
        } else {
            builder->CreateCall(getVybPrintFunction(), {strVal});
        }
        builder->CreateCall(getOrCreateVybStringFreeFunction(), {strVal});
        m_currentLLVMValue = nullptr;
        return;
    }

    // Handle math library intrinsics
    if (identCallee) {
        const std::string& mathName = identCallee->name;
        auto getLibmFunc1 = [&](const std::string& fname) -> llvm::Function* {
            llvm::Function* mf = module->getFunction(fname);
            if (!mf) {
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(*context),
                    {llvm::Type::getDoubleTy(*context)}, false);
                mf = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, fname, module.get());
            }
            return mf;
        };
        auto getLibmFunc2 = [&](const std::string& fname) -> llvm::Function* {
            llvm::Function* mf = module->getFunction(fname);
            if (!mf) {
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    llvm::Type::getDoubleTy(*context),
                    {llvm::Type::getDoubleTy(*context), llvm::Type::getDoubleTy(*context)}, false);
                mf = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, fname, module.get());
            }
            return mf;
        };
        auto toDouble = [&](llvm::Value* v) -> llvm::Value* {
            if (!v) return v;
            if (v->getType()->isDoubleTy()) return v;
            if (v->getType()->isIntegerTy())
                return builder->CreateSIToFP(v, llvm::Type::getDoubleTy(*context), "to_dbl");
            if (v->getType()->isFloatingPointTy())
                return builder->CreateFPCast(v, llvm::Type::getDoubleTy(*context), "fp_cast");
            return v;
        };
        auto evalMathArg = [&](size_t i) -> llvm::Value* {
            if (i >= node->arguments.size()) return nullptr;
            node->arguments[i]->accept(*this);
            return m_currentLLVMValue;
        };
        // #256: a kernel (NVPTX) module has no libm, so these builtins were emitted as
        // `.extern .func` calls with no definition -- the PTX carried undefined device
        // symbols and the CUDA toolchain rejected it, yet the compiler reported success.
        // Reject the construct instead, naming the callable.
        if (g_kernel_mode &&
            (mathName == "sqrt" || mathName == "sin" || mathName == "cos" || mathName == "tan" ||
             mathName == "exp" || mathName == "log" || mathName == "log2" || mathName == "log10" ||
             mathName == "floor" || mathName == "ceil" || mathName == "round" || mathName == "pow")) {
            logError(node->loc, "math builtin '" + mathName + "' is not available in NVPTX device code: "
                     "the device module has no libm, so this would emit an unresolved extern call. Use a "
                     "pure-arithmetic device implementation (see VybForge native/kernels/vmath.vyb) or "
                     "supply your own device function.");
            flagHardCodegenError();
            m_currentLLVMValue = nullptr;
            return;
        }
        if (mathName == "sqrt" || mathName == "sin" || mathName == "cos" || mathName == "tan" ||
            mathName == "exp" || mathName == "log" || mathName == "log2" || mathName == "log10" ||
            mathName == "floor" || mathName == "ceil" || mathName == "round") {
            llvm::Value* a = toDouble(evalMathArg(0)); if (!a) return;
            m_currentLLVMValue = builder->CreateCall(getLibmFunc1(mathName), {a}, mathName);
            return;
        } else if (mathName == "pow") {
            llvm::Value* a = toDouble(evalMathArg(0)); if (!a) return;
            llvm::Value* b = toDouble(evalMathArg(1)); if (!b) return;
            m_currentLLVMValue = builder->CreateCall(getLibmFunc2("pow"), {a, b}, "pow");
            return;
        } else if (mathName == "abs") {
            llvm::Value* a = evalMathArg(0); if (!a) return;
            if (a->getType()->isIntegerTy()) {
                llvm::Value* neg = builder->CreateNeg(a, "neg");
                llvm::Value* cmp = builder->CreateICmpSGT(a, neg, "abs_cmp");
                m_currentLLVMValue = builder->CreateSelect(cmp, a, neg, "abs");
            } else {
                llvm::Value* d = toDouble(a);
                // #256: `abs` on a Float lowers to a `fabs` libm call, which device code
                // cannot resolve either (integer abs stays pure arithmetic and is fine).
                if (g_kernel_mode) {
                    logError(node->loc, "math builtin 'fabs' (Float abs) is not available in NVPTX "
                             "device code: the device module has no libm. Use `select`/comparison "
                             "arithmetic instead.");
                    flagHardCodegenError();
                    m_currentLLVMValue = nullptr;
                    return;
                }
                m_currentLLVMValue = builder->CreateCall(getLibmFunc1("fabs"), {d}, "fabs");
            }
            return;
        } else if (mathName == "min") {
            llvm::Value* a = evalMathArg(0); if (!a) return;
            llvm::Value* b = evalMathArg(1); if (!b) return;
            if (a->getType()->isIntegerTy() && b->getType()->isIntegerTy()) {
                llvm::Value* cmp = builder->CreateICmpSLT(a, b, "min_cmp");
                m_currentLLVMValue = builder->CreateSelect(cmp, a, b, "min");
            } else {
                llvm::Value* da = toDouble(a), *db = toDouble(b);
                llvm::Value* cmp = builder->CreateFCmpOLT(da, db, "min_cmp");
                m_currentLLVMValue = builder->CreateSelect(cmp, da, db, "min");
            }
            return;
        } else if (mathName == "max") {
            llvm::Value* a = evalMathArg(0); if (!a) return;
            llvm::Value* b = evalMathArg(1); if (!b) return;
            if (a->getType()->isIntegerTy() && b->getType()->isIntegerTy()) {
                llvm::Value* cmp = builder->CreateICmpSGT(a, b, "max_cmp");
                m_currentLLVMValue = builder->CreateSelect(cmp, a, b, "max");
            } else {
                llvm::Value* da = toDouble(a), *db = toDouble(b);
                llvm::Value* cmp = builder->CreateFCmpOGT(da, db, "max_cmp");
                m_currentLLVMValue = builder->CreateSelect(cmp, da, db, "max");
            }
            return;
        }
    }

    // Handle File I/O intrinsics (io stdlib module). Each maps a Vyb-level call
    // (`vyb_io_*`) to an exported runtime symbol (`__vyb_file_*`) in
    // `runtime/vyb_runtime.c`; Vyb identifiers cannot start with `_`, hence the
    // separate callable name vs. resolved symbol name. `path`/`data` come in as
    // Vyb String structs { ptr, len } and their data pointer (and for writes,
    // length) is extracted at the boundary. `read_all` and the error message
    // return an owned heap buffer registered by the runtime, so the Vyb String
    // built over it is freed by normal reference-counted cleanup.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        std::string rtName;
        if (fname == "vyb_io_open") rtName = "__vyb_file_open";
        else if (fname == "vyb_io_close") rtName = "__vyb_file_close";
        else if (fname == "vyb_io_write") rtName = "__vyb_file_write";
        else if (fname == "vyb_io_write_bytes") rtName = "__vyb_file_write";
        else if (fname == "vyb_io_write_at") rtName = "__vyb_file_write_at";
        else if (fname == "vyb_io_read_all") rtName = "__vyb_file_read_all";
        else if (fname == "vyb_io_error_code") rtName = "__vyb_file_error_code";
        else if (fname == "vyb_io_error_message") rtName = "__vyb_file_error_message";
        else if (fname == "vyb_fs_mkdir") rtName = "__vyb_mkdir";
        else if (fname == "vyb_crypto_sha256") rtName = "__vyb_sha256_hex";
        else if (fname == "vyb_crypto_ed25519_publickey") rtName = "__vyb_ed25519_publickey";
        else if (fname == "vyb_crypto_ed25519_sign") rtName = "__vyb_ed25519_sign";
        else if (fname == "vyb_crypto_ed25519_verify") rtName = "__vyb_ed25519_verify";
        if (!rtName.empty()) {
            auto getFileFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f = module->getFunction(rtName);
                if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
                return f;
            };
            auto toI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy())
                    return builder->CreateSExt(v, int64Type, "file.toi64");
                return v;
            };
            auto toStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                // A Vyb String value { ptr, len } -> its char* data pointer.
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 0, "file.strptr");
                return v;
            };
            // Emit a Vyb String argument at index i, returning its { ptr, len }.
            auto emitStrArg = [&](unsigned i, llvm::Value** outPtr,
                                  llvm::Value** outLen) -> bool {
                if (i >= node->arguments.size()) return false;
                node->arguments[i]->accept(*this);
                llvm::Value* v = m_currentLLVMValue;
                if (!v) return false;
                *outPtr = toStrPtr(v);
                *outLen = llvm::ConstantInt::get(int64Type, 0);
                if (v->getType()->isStructTy())
                    *outLen = builder->CreateExtractValue(v, 1, "arg.len");
                return true;
            };

            if (fname == "vyb_io_open") {
                if (node->arguments.size() != 2) {
                    logError(node->loc, "__vyb_file_open expects 2 arguments (path, flags)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* path = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* flags = m_currentLLVMValue;
                if (!path || !flags) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(
                    getFileFn(ft), {toStrPtr(path), toI64(flags)}, "file.fd");
                return;
            } else if (fname == "vyb_fs_mkdir") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, "__vyb_mkdir expects 1 argument (path)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* path = m_currentLLVMValue;
                if (!path) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(
                    getFileFn(ft), {toStrPtr(path)}, "fs.mkdir");
                return;
            } else if (fname == "vyb_io_close") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, "__vyb_file_close expects 1 argument (fd)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
                if (!fd) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getFileFn(ft), {toI64(fd)}, "file.closed");
                return;
            } else if (fname == "vyb_io_write") {
                if (node->arguments.size() != 2) {
                    logError(node->loc, "__vyb_file_write expects 2 arguments (fd, data)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* data = m_currentLLVMValue;
                if (!fd || !data) { m_currentLLVMValue = nullptr; return; }
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "file.strlen");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(
                    getFileFn(ft), {toI64(fd), dataPtr, dataLen}, "file.written");
                return;
            } else if (fname == "vyb_io_write_bytes") {
                if (node->arguments.size() != 2) {
                    logError(node->loc, "__vyb_file_write expects 2 arguments (fd, data<Vec<UInt8>>)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* data = m_currentLLVMValue;
                if (!fd || !data) { m_currentLLVMValue = nullptr; return; }
                // `data` is a Vec<UInt8> = { ptr, size, cap }; write the
                // `size` live bytes raw (no NUL-termination needed for binary).
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "vec.datasize");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(
                    getFileFn(ft), {toI64(fd), dataPtr, dataLen}, "file.written_bytes");
                return;
            } else if (fname == "vyb_io_write_at") {
                if (node->arguments.size() != 3) {
                    logError(node->loc, "__vyb_file_write_at expects 3 arguments (fd, off, data<Vec<UInt8>>)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* off = m_currentLLVMValue;
                node->arguments[2]->accept(*this); llvm::Value* data = m_currentLLVMValue;
                if (!fd || !off || !data) { m_currentLLVMValue = nullptr; return; }
                // `data` is a Vec<UInt8> = { ptr, size, cap }; write `size`
                // live bytes at absolute `off` (stateless pwrite).
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "vec.datasize");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(
                    getFileFn(ft), {toI64(fd), toI64(off), dataPtr, dataLen}, "file.written_at");
                return;
            } else if (fname == "vyb_io_read_all") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, "__vyb_file_read_all expects 1 argument (fd)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
                if (!fd) { m_currentLLVMValue = nullptr; return; }
                std::vector<llvm::Type*> strFields = {int8PtrType, int64Type};
                llvm::StructType* strStructType = llvm::StructType::get(*context, strFields, false);
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getFileFn(ft), {toI64(fd)}, "file.content");
                return;
            } else if (fname == "vyb_crypto_sha256") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, "__vyb_sha256_hex expects 1 argument (data)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* data = m_currentLLVMValue;
                if (!data) { m_currentLLVMValue = nullptr; return; }
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "crypto.len");
                std::vector<llvm::Type*> strFields = {int8PtrType, int64Type};
                llvm::StructType* strStructType = llvm::StructType::get(*context, strFields, false);
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType, {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getFileFn(ft), {dataPtr, dataLen}, "crypto.sha256");
                return;
            } else if (fname == "vyb_crypto_ed25519_publickey") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, "__vyb_ed25519_publickey expects 1 argument (seed)");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::Value* seedP, *seedL;
                if (!emitStrArg(0, &seedP, &seedL)) { m_currentLLVMValue = nullptr; return; }
                std::vector<llvm::Type*> strFields = {int8PtrType, int64Type};
                llvm::StructType* st = llvm::StructType::get(*context, strFields, false);
                llvm::FunctionType* ft = llvm::FunctionType::get(st, {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getFileFn(ft), {seedP, seedL}, "crypto.pub");
                return;
            } else if (fname == "vyb_crypto_ed25519_sign") {
                if (node->arguments.size() != 2) {
                    logError(node->loc, "__vyb_ed25519_sign expects 2 arguments (seed, message)");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::Value* seedP, *seedL, *msgP, *msgL;
                if (!emitStrArg(0, &seedP, &seedL) || !emitStrArg(1, &msgP, &msgL)) {
                    m_currentLLVMValue = nullptr; return;
                }
                std::vector<llvm::Type*> strFields = {int8PtrType, int64Type};
                llvm::StructType* st = llvm::StructType::get(*context, strFields, false);
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    st, {int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getFileFn(ft), {seedP, seedL, msgP, msgL}, "crypto.sig");
                return;
            } else if (fname == "vyb_crypto_ed25519_verify") {
                if (node->arguments.size() != 3) {
                    logError(node->loc, "__vyb_ed25519_verify expects 3 arguments (public, message, signature)");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::Value* pubP, *pubL, *msgP, *msgL, *sigP, *sigL;
                if (!emitStrArg(0, &pubP, &pubL) || !emitStrArg(1, &msgP, &msgL) ||
                    !emitStrArg(2, &sigP, &sigL)) {
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {
                    int8PtrType, int64Type, int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getFileFn(ft),
                    {pubP, pubL, msgP, msgL, sigP, sigL}, "crypto.verify");
                return;
            } else if (fname == "vyb_io_error_code") {
                if (!node->arguments.empty()) {
                    logError(node->loc, "__vyb_file_error_code expects no arguments");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getFileFn(ft), {}, "file.errcode");
                return;
            } else { // __vyb_file_error_message
                if (!node->arguments.empty()) {
                    logError(node->loc, "__vyb_file_error_message expects no arguments");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::FunctionType* fmsg = llvm::FunctionType::get(int8PtrType, {}, false);
                llvm::Value* msgPtr = builder->CreateCall(getFileFn(fmsg), {}, "file.errmsg");
                std::vector<llvm::Type*> strFields = {int8PtrType, int64Type};
                llvm::StructType* strStructType = llvm::StructType::get(*context, strFields, false);
                llvm::FunctionType* strlenType = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                llvm::Function* strlenFunc = module->getFunction("strlen");
                if (!strlenFunc)
                    strlenFunc = llvm::Function::Create(strlenType, llvm::Function::ExternalLinkage, "strlen", module.get());
                llvm::Value* msgLen = builder->CreateCall(strlenFunc, {msgPtr}, "file.errlen");
                llvm::Value* outStr = llvm::UndefValue::get(strStructType);
                outStr = builder->CreateInsertValue(outStr, msgPtr, 0, "file.msg.data");
                outStr = builder->CreateInsertValue(outStr, msgLen, 1, "file.msg.len");
                m_currentLLVMValue = outStr;
                return;
            }
        }
    }

    // Terminal + stdin intrinsics (term stdlib module): interactive console I/O.
    // Mirrors the File/Network mapping — Vyb-level calls (`vyb_stdin_*`,
    // `vyb_eprint*`, `vyb_stdout_flush`, `vyb_stderr_flush`, `vyb_term_*`) resolve
    // to `__vyb_*` runtime symbols in `runtime/vyb_runtime.c`. The stdin readers
    // return an owned, registry-registered heap buffer { ptr, len } that the Vyb
    // String built over it will free on last reference; all other helpers return
    // an Int status. String arguments arrive as Vyb Strings { ptr, len } and their
    // data pointer (and length) are extracted at the boundary.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        std::string rtName;
        if (fname == "vyb_stdin_read") rtName = "__vyb_stdin_read";
        else if (fname == "vyb_stdin_read_line") rtName = "__vyb_stdin_read_line";
        else if (fname == "vyb_stdin_isatty") rtName = "__vyb_stdin_isatty";
        else if (fname == "vyb_stdin_raw_enable") rtName = "__vyb_stdin_raw_enable";
        else if (fname == "vyb_stdin_raw_disable") rtName = "__vyb_stdin_raw_disable";
        else if (fname == "vyb_eprint") rtName = "__vyb_eprint";
        else if (fname == "vyb_eprintln") rtName = "__vyb_eprintln";
        else if (fname == "vyb_stdout_flush") rtName = "__vyb_stdout_flush";
        else if (fname == "vyb_stderr_flush") rtName = "__vyb_stderr_flush";
        else if (fname == "vyb_term_cols") rtName = "__vyb_term_cols";
        else if (fname == "vyb_term_rows") rtName = "__vyb_term_rows";
        else if (fname == "vyb_term_clear") rtName = "__vyb_term_clear";
        else if (fname == "vyb_term_move_cursor") rtName = "__vyb_term_move_cursor";
        else if (fname == "vyb_term_hide_cursor") rtName = "__vyb_term_hide_cursor";
        else if (fname == "vyb_term_show_cursor") rtName = "__vyb_term_show_cursor";
        if (!rtName.empty()) {
            auto getTermFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f = module->getFunction(rtName);
                if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
                return f;
            };
            auto toI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy()) return builder->CreateSExt(v, int64Type, "term.toi64");
                return v;
            };
            auto toStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy()) return builder->CreateExtractValue(v, 0, "term.strptr");
                return v;
            };
            auto strLen = [&](llvm::Value* v) -> llvm::Value* {
                return (v && v->getType()->isStructTy())
                    ? builder->CreateExtractValue(v, 1, "term.strlen")
                    : llvm::ConstantInt::get(int64Type, 0);
            };
            auto strStruct = [&]() {
                std::vector<llvm::Type*> f = {int8PtrType, int64Type};
                return llvm::StructType::get(*context, f, false);
            };

            if (fname == "vyb_stdin_read") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, rtName + " expects 1 argument (maxlen)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* maxlen = m_currentLLVMValue;
                if (!maxlen) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(strStruct(), {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getTermFn(ft), {toI64(maxlen)}, "stdin.data");
                return;
            } else if (fname == "vyb_stdin_read_line") {
                if (!node->arguments.empty()) {
                    logError(node->loc, rtName + " expects no arguments");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::FunctionType* ft = llvm::FunctionType::get(strStruct(), {}, false);
                m_currentLLVMValue = builder->CreateCall(getTermFn(ft), {}, "stdin.line");
                return;
            } else if (fname == "vyb_eprint" || fname == "vyb_eprintln") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, rtName + " expects 1 argument (s)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* s = m_currentLLVMValue;
                if (!s) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getTermFn(ft), {toStrPtr(s), strLen(s)}, "term.eprint");
                return;
            } else if (fname == "vyb_term_move_cursor") {
                if (node->arguments.size() != 2) {
                    logError(node->loc, rtName + " expects 2 arguments (row, col)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* row = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* col = m_currentLLVMValue;
                if (!row || !col) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getTermFn(ft), {toI64(row), toI64(col)}, "term.move");
                return;
            } else {
                if (!node->arguments.empty()) {
                    logError(node->loc, rtName + " expects no arguments");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getTermFn(ft), {}, "term.ret");
                return;
            }
        }
    }

    // Curses TUI intrinsics (curses stdlib module): thin facades over the
    // `__vyb_curses_*` runtime shims in `runtime/vyb_runtime.c` (which wrap
    // ncursesw). Every helper returns an Int (status, keycode, or attribute);
    // a String argument crosses as its { ptr, len } struct and the data pointer
    // and length are extracted at the boundary. Arity follows the module surface.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        std::string rtName;
        if (fname == "vyb_curses_init") rtName = "__vyb_curses_init";
        else if (fname == "vyb_curses_close") rtName = "__vyb_curses_close";
        else if (fname == "vyb_curses_ok") rtName = "__vyb_curses_ok";
        else if (fname == "vyb_curses_rows") rtName = "__vyb_curses_rows";
        else if (fname == "vyb_curses_cols") rtName = "__vyb_curses_cols";
        else if (fname == "vyb_curses_refresh") rtName = "__vyb_curses_refresh";
        else if (fname == "vyb_curses_clear") rtName = "__vyb_curses_clear";
        else if (fname == "vyb_curses_move") rtName = "__vyb_curses_move";
        else if (fname == "vyb_curses_addstr") rtName = "__vyb_curses_addstr";
        else if (fname == "vyb_curses_move_addstr") rtName = "__vyb_curses_move_addstr";
        else if (fname == "vyb_curses_has_color") rtName = "__vyb_curses_has_color";
        else if (fname == "vyb_curses_start_color") rtName = "__vyb_curses_start_color";
        else if (fname == "vyb_curses_color_pair") rtName = "__vyb_curses_color_pair";
        else if (fname == "vyb_curses_init_pair") rtName = "__vyb_curses_init_pair";
        else if (fname == "vyb_curses_attr_on") rtName = "__vyb_curses_attr_on";
        else if (fname == "vyb_curses_attr_off") rtName = "__vyb_curses_attr_off";
        else if (fname == "vyb_curses_attr_normal") rtName = "__vyb_curses_attr_normal";
        else if (fname == "vyb_curses_attr_bold") rtName = "__vyb_curses_attr_bold";
        else if (fname == "vyb_curses_attr_underline") rtName = "__vyb_curses_attr_underline";
        else if (fname == "vyb_curses_attr_reverse") rtName = "__vyb_curses_attr_reverse";
        else if (fname == "vyb_curses_attr_blink") rtName = "__vyb_curses_attr_blink";
        else if (fname == "vyb_curses_getch") rtName = "__vyb_curses_getch";
        else if (fname == "vyb_curses_nodelay") rtName = "__vyb_curses_nodelay";
        else if (fname == "vyb_curses_timeout") rtName = "__vyb_curses_timeout";
        else if (fname == "vyb_curses_keypad") rtName = "__vyb_curses_keypad";
        else if (fname == "vyb_curses_show_cursor") rtName = "__vyb_curses_show_cursor";
        else if (fname == "vyb_curses_hide_cursor") rtName = "__vyb_curses_hide_cursor";
        if (!rtName.empty()) {
            auto getCurseFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f = module->getFunction(rtName);
                if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
                return f;
            };
            auto toI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy()) return builder->CreateSExt(v, int64Type, "curses.toi64");
                return v;
            };
            auto toStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy()) return builder->CreateExtractValue(v, 0, "curses.strptr");
                return v;
            };
            auto strLen = [&](llvm::Value* v) -> llvm::Value* {
                return (v && v->getType()->isStructTy())
                    ? builder->CreateExtractValue(v, 1, "curses.strlen")
                    : llvm::ConstantInt::get(int64Type, 0);
            };
            auto callNoArgs = [&](const char* tag) { return builder->CreateCall(
                getCurseFn(llvm::FunctionType::get(int64Type, {}, false)), {}, tag); };

            if (fname == "vyb_curses_move") {
                if (node->arguments.size() != 2) {
                    logError(node->loc, rtName + " expects 2 arguments (row, col)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* row = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* col = m_currentLLVMValue;
                if (!row || !col) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getCurseFn(ft), {toI64(row), toI64(col)}, "curses.move");
                return;
            } else if (fname == "vyb_curses_addstr") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, rtName + " expects 1 argument (s)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* s = m_currentLLVMValue;
                if (!s) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getCurseFn(ft), {toStrPtr(s), strLen(s)}, "curses.addstr");
                return;
            } else if (fname == "vyb_curses_move_addstr") {
                if (node->arguments.size() != 3) {
                    logError(node->loc, rtName + " expects 3 arguments (row, col, s)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* row = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* col = m_currentLLVMValue;
                node->arguments[2]->accept(*this); llvm::Value* s = m_currentLLVMValue;
                if (!row || !col || !s) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    int64Type, {int64Type, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(
                    getCurseFn(ft), {toI64(row), toI64(col), toStrPtr(s), strLen(s)}, "curses.moveadd");
                return;
            } else if (fname == "vyb_curses_init_pair") {
                if (node->arguments.size() != 3) {
                    logError(node->loc, rtName + " expects 3 arguments (pair, fg, bg)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this); llvm::Value* pair = m_currentLLVMValue;
                node->arguments[1]->accept(*this); llvm::Value* fg = m_currentLLVMValue;
                node->arguments[2]->accept(*this); llvm::Value* bg = m_currentLLVMValue;
                if (!pair || !fg || !bg) { m_currentLLVMValue = nullptr; return; }
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    int64Type, {int64Type, int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(
                    getCurseFn(ft), {toI64(pair), toI64(fg), toI64(bg)}, "curses.initpair");
                return;
            } else {
                // 1-arg Int helpers: color_pair, attr_on, attr_off, nodelay, timeout, keypad
                if (fname == "vyb_curses_color_pair" || fname == "vyb_curses_attr_on" ||
                    fname == "vyb_curses_attr_off" || fname == "vyb_curses_nodelay" ||
                    fname == "vyb_curses_timeout" || fname == "vyb_curses_keypad") {
                    if (node->arguments.size() != 1) {
                        logError(node->loc, rtName + " expects 1 argument");
                        m_currentLLVMValue = nullptr; return;
                    }
                    node->arguments[0]->accept(*this); llvm::Value* arg = m_currentLLVMValue;
                    if (!arg) { m_currentLLVMValue = nullptr; return; }
                    llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                    m_currentLLVMValue = builder->CreateCall(getCurseFn(ft), {toI64(arg)}, "curses.arg1");
                    return;
                }
                if (!node->arguments.empty()) {
                    logError(node->loc, rtName + " expects no arguments");
                    m_currentLLVMValue = nullptr; return;
                }
                m_currentLLVMValue = callNoArgs("curses.ret");
                return;
            }
        }
    }

    // Handle UTF-8 / env / rand / process / regex intrinsics (the utf8, env,
    // rand, process, regex stdlib modules) plus the socket-timeout helper added
    // to the network module. Same FFI shape as the terminal block: String args
    // cross as { ptr, len } structs and their data pointer (and, where the helper
    // cares about the exact bytes, the length) is extracted; the string-returning
    // helpers hand back an owned, registry-registered buffer the Vyb String
    // adopts, so reference-counted cleanup releases it.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        std::string rtName;
        if (fname == "vyb_utf8_len" || fname == "vyb_utf8_at" ||
            fname == "vyb_utf8_index" || fname == "vyb_utf8_valid") rtName = "__vyb_" + fname.substr(4);
        else if (fname == "vyb_net_set_timeout") rtName = "__vyb_net_set_timeout";
        else if (fname == "vyb_env_get") rtName = "__vyb_env_get";
        else if (fname == "vyb_env_set") rtName = "__vyb_env_set";
        else if (fname == "vyb_env_unset") rtName = "__vyb_env_unset";
        else if (fname == "vyb_rand") rtName = "__vyb_rand";
        else if (fname == "vyb_rand_range") rtName = "__vyb_rand_range";
        else if (fname == "vyb_rand_seed") rtName = "__vyb_rand_seed";
        else if (fname == "vyb_exec_run") rtName = "__vyb_exec_run";
        else if (fname == "vyb_exec_output") rtName = "__vyb_exec_output";
        else if (fname == "vyb_exec_status") rtName = "__vyb_exec_status";
        else if (fname == "vyb_regex_match" || fname == "vyb_regex_find" ||
                 fname == "vyb_regex_capture_match" || fname == "vyb_regex_capture" ||
                 fname == "vyb_regex_replace" || fname == "vyb_regex_replace_all") rtName = "__vyb_" + fname.substr(4);
        if (!rtName.empty()) {
            auto getXFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f = module->getFunction(rtName);
                if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
                return f;
            };
            auto toI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy())
                    return builder->CreateSExt(v, int64Type, "x.toi64");
                return v;
            };
            auto toStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 0, "x.strptr");
                return v;
            };
            auto strLenOf = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 1, "x.strlen");
                return llvm::ConstantInt::get(int64Type, 0);
            };
            auto xsStrStructType = [&]() {
                std::vector<llvm::Type*> f = {int8PtrType, int64Type};
                return llvm::StructType::get(*context, f, false);
            };
            auto needArg = [&](size_t idx) -> llvm::Value* {
                if (idx >= node->arguments.size()) { m_currentLLVMValue = nullptr; return nullptr; }
                node->arguments[idx]->accept(*this);
                return m_currentLLVMValue;
            };
            auto checkArity = [&](size_t n) -> bool {
                if (node->arguments.size() != n) {
                    logError(node->loc, rtName + " expects " + std::to_string(n) + " argument(s)");
                    m_currentLLVMValue = nullptr;
                    return false;
                }
                return true;
            };

            if (fname == "vyb_utf8_len" || fname == "vyb_utf8_valid") {
                if (!checkArity(1)) return;
                llvm::Value* str = needArg(0); if (!str) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toStrPtr(str), strLenOf(str)}, "x.utf8");
                return;
            } else if (fname == "vyb_utf8_at" || fname == "vyb_utf8_index") {
                if (!checkArity(2)) return;
                llvm::Value* str = needArg(0); llvm::Value* idx = needArg(1);
                if (!str || !idx) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType, int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toStrPtr(str), strLenOf(str), toI64(idx)}, "x.utf8");
                return;
            } else if (fname == "vyb_net_set_timeout") {
                if (!checkArity(2)) return;
                llvm::Value* fd = needArg(0); llvm::Value* ms = needArg(1);
                if (!fd || !ms) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toI64(fd), toI64(ms)}, "x.timeout");
                return;
            } else if (fname == "vyb_env_get") {
                if (!checkArity(1)) return;
                llvm::Value* name = needArg(0); if (!name) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(xsStrStructType(), {int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toStrPtr(name)}, "x.envget");
                return;
            } else if (fname == "vyb_env_set") {
                if (!checkArity(2)) return;
                llvm::Value* name = needArg(0); llvm::Value* value = needArg(1);
                if (!name || !value) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType, int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toStrPtr(name), toStrPtr(value)}, "x.envset");
                return;
            } else if (fname == "vyb_env_unset") {
                if (!checkArity(1)) return;
                llvm::Value* name = needArg(0); if (!name) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toStrPtr(name)}, "x.envunset");
                return;
            } else if (fname == "vyb_rand") {
                if (!checkArity(0)) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {}, "x.rand");
                return;
            } else if (fname == "vyb_rand_range") {
                if (!checkArity(2)) return;
                llvm::Value* lo = needArg(0); llvm::Value* hi = needArg(1);
                if (!lo || !hi) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toI64(lo), toI64(hi)}, "x.randrange");
                return;
            } else if (fname == "vyb_rand_seed") {
                if (!checkArity(1)) return;
                llvm::Value* seed = needArg(0); if (!seed) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(voidType, {int64Type}, false);
                builder->CreateCall(getXFn(ft), {toI64(seed)});
                m_currentLLVMValue = llvm::ConstantInt::get(int64Type, 0);
                return;
            } else if (fname == "vyb_exec_run") {
                if (!checkArity(1)) return;
                llvm::Value* cmd = needArg(0); if (!cmd) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toStrPtr(cmd)}, "x.exec");
                return;
            } else if (fname == "vyb_exec_output") {
                if (!checkArity(1)) return;
                llvm::Value* cmd = needArg(0); if (!cmd) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(xsStrStructType(), {int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {toStrPtr(cmd)}, "x.execout");
                return;
            } else if (fname == "vyb_exec_status") {
                if (!checkArity(0)) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft), {}, "x.execstatus");
                return;
            } else if (fname == "vyb_regex_match" || fname == "vyb_regex_find") {
                if (!checkArity(2)) return;
                llvm::Value* pat = needArg(0); llvm::Value* str = needArg(1);
                if (!pat || !str) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type,
                    {int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft),
                    {toStrPtr(pat), strLenOf(pat), toStrPtr(str), strLenOf(str)}, "x.regex");
                return;
            } else if (fname == "vyb_regex_capture_match" || fname == "vyb_regex_capture") {
                if (!checkArity(2)) return;
                llvm::Value* pat = needArg(0); llvm::Value* str = needArg(1);
                if (!pat || !str) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(xsStrStructType(),
                    {int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft),
                    {toStrPtr(pat), strLenOf(pat), toStrPtr(str), strLenOf(str)}, "x.regex");
                return;
            } else { // vyb_regex_replace / vyb_regex_replace_all
                if (!checkArity(3)) return;
                llvm::Value* pat = needArg(0); llvm::Value* str = needArg(1); llvm::Value* repl = needArg(2);
                if (!pat || !str || !repl) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(xsStrStructType(),
                    {int8PtrType, int64Type, int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getXFn(ft),
                    {toStrPtr(pat), strLenOf(pat), toStrPtr(str), strLenOf(str),
                     toStrPtr(repl), strLenOf(repl)}, "x.regex");
                return;
            }
        }
    }

    // Handle Qt5 native GUI intrinsics (qt stdlib module). Thin facades over the
    // `__vyb_qt_*` extern "C" shims in `runtime/vyb_qt_bridge.cpp` (a C++ Qt5
    // Widgets bridge). Vyb windows/labels are opaque Int handles (qintptr-sized);
    // String args cross as { ptr, len } structs whose data pointer (and length)
    // are extracted at the boundary, and String-returning getters hand back an
    // owned, registry-registered buffer the Vyb String adopts (same convention
    // as env_exec/regex). Skewed from the network block so the FFI stays
    // Int/String-shaped and deterministic under a headless QPA platform.
    if (identCallee) {
        // gen_qt[cgen]: begin
        const std::string& fname = identCallee->name;
        std::string rtName;
        if (fname == "vyb_qt_web_create") rtName = "__vyb_qt_web_create";
        else if (fname == "vyb_qt_web_load") rtName = "__vyb_qt_web_load";
        else if (fname == "vyb_qt_web_url") rtName = "__vyb_qt_web_url";
        else if (fname == "vyb_qt_web_title") rtName = "__vyb_qt_web_title";
        else if (fname == "vyb_qt_web_loading") rtName = "__vyb_qt_web_loading";
        else if (fname == "vyb_qt_web_back") rtName = "__vyb_qt_web_back";
        else if (fname == "vyb_qt_web_forward") rtName = "__vyb_qt_web_forward";
        else if (fname == "vyb_qt_web_reload") rtName = "__vyb_qt_web_reload";
        else if (fname == "vyb_qt_web_zoom_in") rtName = "__vyb_qt_web_zoom_in";
        else if (fname == "vyb_qt_web_zoom_out") rtName = "__vyb_qt_web_zoom_out";
        else if (fname == "vyb_qt_init") rtName = "__vyb_qt_init";
        else if (fname == "vyb_qt_quit") rtName = "__vyb_qt_quit";
        else if (fname == "vyb_qt_active") rtName = "__vyb_qt_active";
        else if (fname == "vyb_qt_process_events") rtName = "__vyb_qt_process_events";
        else if (fname == "vyb_qt_set_timer") rtName = "__vyb_qt_set_timer";
        else if (fname == "vyb_qt_timer_fired") rtName = "__vyb_qt_timer_fired";
        else if (fname == "vyb_qt_window_create") rtName = "__vyb_qt_window_create";
        else if (fname == "vyb_qt_window_close") rtName = "__vyb_qt_window_close";
        else if (fname == "vyb_qt_window_set_title") rtName = "__vyb_qt_window_set_title";
        else if (fname == "vyb_qt_window_title") rtName = "__vyb_qt_window_title";
        else if (fname == "vyb_qt_window_resize") rtName = "__vyb_qt_window_resize";
        else if (fname == "vyb_qt_window_width") rtName = "__vyb_qt_window_width";
        else if (fname == "vyb_qt_window_height") rtName = "__vyb_qt_window_height";
        else if (fname == "vyb_qt_window_show") rtName = "__vyb_qt_window_show";
        else if (fname == "vyb_qt_window_hide") rtName = "__vyb_qt_window_hide";
        else if (fname == "vyb_qt_window_visible") rtName = "__vyb_qt_window_visible";
        else if (fname == "vyb_qt_screen_width") rtName = "__vyb_qt_screen_width";
        else if (fname == "vyb_qt_screen_height") rtName = "__vyb_qt_screen_height";
        else if (fname == "vyb_qt_screen_dpi") rtName = "__vyb_qt_screen_dpi";
        else if (fname == "vyb_qt_label_create") rtName = "__vyb_qt_label_create";
        else if (fname == "vyb_qt_label_set_text") rtName = "__vyb_qt_label_set_text";
        else if (fname == "vyb_qt_label_text") rtName = "__vyb_qt_label_text";
        else if (fname == "vyb_qt_button_create") rtName = "__vyb_qt_button_create";
        else if (fname == "vyb_qt_button_set_text") rtName = "__vyb_qt_button_set_text";
        else if (fname == "vyb_qt_button_text") rtName = "__vyb_qt_button_text";
        else if (fname == "vyb_qt_button_set_enabled") rtName = "__vyb_qt_button_set_enabled";
        else if (fname == "vyb_qt_edit_create") rtName = "__vyb_qt_edit_create";
        else if (fname == "vyb_qt_edit_text") rtName = "__vyb_qt_edit_text";
        else if (fname == "vyb_qt_edit_set_text") rtName = "__vyb_qt_edit_set_text";
        else if (fname == "vyb_qt_edit_set_placeholder") rtName = "__vyb_qt_edit_set_placeholder";
        else if (fname == "vyb_qt_checkbox_create") rtName = "__vyb_qt_checkbox_create";
        else if (fname == "vyb_qt_checkbox_checked") rtName = "__vyb_qt_checkbox_checked";
        else if (fname == "vyb_qt_checkbox_set_checked") rtName = "__vyb_qt_checkbox_set_checked";
        else if (fname == "vyb_qt_progress_create") rtName = "__vyb_qt_progress_create";
        else if (fname == "vyb_qt_progress_set_value") rtName = "__vyb_qt_progress_set_value";
        else if (fname == "vyb_qt_vbox") rtName = "__vyb_qt_vbox";
        else if (fname == "vyb_qt_hbox") rtName = "__vyb_qt_hbox";
        else if (fname == "vyb_qt_layout_add") rtName = "__vyb_qt_layout_add";
        else if (fname == "vyb_qt_layout_add_layout") rtName = "__vyb_qt_layout_add_layout";
        else if (fname == "vyb_qt_layout_set_stretch") rtName = "__vyb_qt_layout_set_stretch";
        else if (fname == "vyb_qt_kind") rtName = "__vyb_qt_kind";
        else if (fname == "vyb_qt_event_count") rtName = "__vyb_qt_event_count";
        else if (fname == "vyb_qt_event_handle") rtName = "__vyb_qt_event_handle";
        else if (fname == "vyb_qt_event_kind") rtName = "__vyb_qt_event_kind";
        else if (fname == "vyb_qt_event_pop") rtName = "__vyb_qt_event_pop";
        else if (fname == "vyb_qt_wait_event") rtName = "__vyb_qt_wait_event";
        else if (fname == "vyb_qt_run") rtName = "__vyb_qt_run";
        else if (fname == "vyb_qt_run_stop") rtName = "__vyb_qt_run_stop";
        else if (fname == "vyb_qt_on_event") rtName = "__vyb_qt_on_event";
        else if (fname == "vyb_qt_post_event") rtName = "__vyb_qt_post_event";
        else if (fname == "vyb_qt_combo_create") rtName = "__vyb_qt_combo_create";
        else if (fname == "vyb_qt_combo_add_item") rtName = "__vyb_qt_combo_add_item";
        else if (fname == "vyb_qt_combo_count") rtName = "__vyb_qt_combo_count";
        else if (fname == "vyb_qt_combo_current_index") rtName = "__vyb_qt_combo_current_index";
        else if (fname == "vyb_qt_combo_set_current_index") rtName = "__vyb_qt_combo_set_current_index";
        else if (fname == "vyb_qt_combo_item_text") rtName = "__vyb_qt_combo_item_text";
        else if (fname == "vyb_qt_spin_create") rtName = "__vyb_qt_spin_create";
        else if (fname == "vyb_qt_spin_value") rtName = "__vyb_qt_spin_value";
        else if (fname == "vyb_qt_spin_set_value") rtName = "__vyb_qt_spin_set_value";
        else if (fname == "vyb_qt_slider_create") rtName = "__vyb_qt_slider_create";
        else if (fname == "vyb_qt_slider_value") rtName = "__vyb_qt_slider_value";
        else if (fname == "vyb_qt_slider_set_value") rtName = "__vyb_qt_slider_set_value";
        else if (fname == "vyb_qt_dial_create") rtName = "__vyb_qt_dial_create";
        else if (fname == "vyb_qt_dial_value") rtName = "__vyb_qt_dial_value";
        else if (fname == "vyb_qt_dial_set_value") rtName = "__vyb_qt_dial_set_value";
        else if (fname == "vyb_qt_group_create") rtName = "__vyb_qt_group_create";
        else if (fname == "vyb_qt_text_edit_create") rtName = "__vyb_qt_text_edit_create";
        else if (fname == "vyb_qt_text_edit_text") rtName = "__vyb_qt_text_edit_text";
        else if (fname == "vyb_qt_text_edit_set_text") rtName = "__vyb_qt_text_edit_set_text";
        else if (fname == "vyb_qt_radio_create") rtName = "__vyb_qt_radio_create";
        else if (fname == "vyb_qt_radio_checked") rtName = "__vyb_qt_radio_checked";
        else if (fname == "vyb_qt_radio_set_checked") rtName = "__vyb_qt_radio_set_checked";
        else if (fname == "vyb_qt_widget_set_enabled") rtName = "__vyb_qt_widget_set_enabled";
        else if (fname == "vyb_qt_widget_enabled") rtName = "__vyb_qt_widget_enabled";
        else if (fname == "vyb_qt_grid") rtName = "__vyb_qt_grid";
        else if (fname == "vyb_qt_grid_add") rtName = "__vyb_qt_grid_add";
        else if (fname == "vyb_qt_widget_set_visible") rtName = "__vyb_qt_widget_set_visible";
        else if (fname == "vyb_qt_widget_visible") rtName = "__vyb_qt_widget_visible";
        else if (fname == "vyb_qt_tabs_create") rtName = "__vyb_qt_tabs_create";
        else if (fname == "vyb_qt_tabs_add") rtName = "__vyb_qt_tabs_add";
        else if (fname == "vyb_qt_tabs_count") rtName = "__vyb_qt_tabs_count";
        else if (fname == "vyb_qt_tabs_current") rtName = "__vyb_qt_tabs_current";
        else if (fname == "vyb_qt_tabs_set_current") rtName = "__vyb_qt_tabs_set_current";
        else if (fname == "vyb_qt_list_create") rtName = "__vyb_qt_list_create";
        else if (fname == "vyb_qt_list_add") rtName = "__vyb_qt_list_add";
        else if (fname == "vyb_qt_list_count") rtName = "__vyb_qt_list_count";
        else if (fname == "vyb_qt_list_current") rtName = "__vyb_qt_list_current";
        else if (fname == "vyb_qt_list_set_current") rtName = "__vyb_qt_list_set_current";
        else if (fname == "vyb_qt_list_item_text") rtName = "__vyb_qt_list_item_text";
        else if (fname == "vyb_qt_main_window_create") rtName = "__vyb_qt_main_window_create";
        else if (fname == "vyb_qt_menubar") rtName = "__vyb_qt_menubar";
        else if (fname == "vyb_qt_menu_add") rtName = "__vyb_qt_menu_add";
        else if (fname == "vyb_qt_action_add") rtName = "__vyb_qt_action_add";
        else if (fname == "vyb_qt_action_count") rtName = "__vyb_qt_action_count";
        else if (fname == "vyb_qt_statusbar_message") rtName = "__vyb_qt_statusbar_message";
        else if (fname == "vyb_qt_statusbar_text") rtName = "__vyb_qt_statusbar_text";
        else if (fname == "vyb_qt_toolbar_create") rtName = "__vyb_qt_toolbar_create";
        else if (fname == "vyb_qt_msg_info") rtName = "__vyb_qt_msg_info";
        else if (fname == "vyb_qt_msg_warn") rtName = "__vyb_qt_msg_warn";
        else if (fname == "vyb_qt_msg_error") rtName = "__vyb_qt_msg_error";
        else if (fname == "vyb_qt_msg_about") rtName = "__vyb_qt_msg_about";
        else if (fname == "vyb_qt_msg_question") rtName = "__vyb_qt_msg_question";
        else if (fname == "vyb_qt_file_open") rtName = "__vyb_qt_file_open";
        else if (fname == "vyb_qt_file_save") rtName = "__vyb_qt_file_save";
        else if (fname == "vyb_qt_dir_select") rtName = "__vyb_qt_dir_select";
        else if (fname == "vyb_qt_dlg_info") rtName = "__vyb_qt_dlg_info";
        else if (fname == "vyb_qt_dlg_warn") rtName = "__vyb_qt_dlg_warn";
        else if (fname == "vyb_qt_dlg_error") rtName = "__vyb_qt_dlg_error";
        else if (fname == "vyb_qt_dlg_about") rtName = "__vyb_qt_dlg_about";
        else if (fname == "vyb_qt_dlg_question") rtName = "__vyb_qt_dlg_question";
        else if (fname == "vyb_qt_dlg_open") rtName = "__vyb_qt_dlg_open";
        else if (fname == "vyb_qt_dlg_save") rtName = "__vyb_qt_dlg_save";
        else if (fname == "vyb_qt_dlg_dir") rtName = "__vyb_qt_dlg_dir";
        else if (fname == "vyb_qt_dlg_close") rtName = "__vyb_qt_dlg_close";
        else if (fname == "vyb_qt_dlg_selected") rtName = "__vyb_qt_dlg_selected";
        else if (fname == "vyb_qt_event_result") rtName = "__vyb_qt_event_result";
        else if (fname == "vyb_qt_rich_create") rtName = "__vyb_qt_rich_create";
        else if (fname == "vyb_qt_rich_set_html") rtName = "__vyb_qt_rich_set_html";
        else if (fname == "vyb_qt_rich_html") rtName = "__vyb_qt_rich_html";
        else if (fname == "vyb_qt_rich_set_plain") rtName = "__vyb_qt_rich_set_plain";
        else if (fname == "vyb_qt_rich_plain") rtName = "__vyb_qt_rich_plain";
        else if (fname == "vyb_qt_rich_append") rtName = "__vyb_qt_rich_append";
        else if (fname == "vyb_qt_rich_clear") rtName = "__vyb_qt_rich_clear";
        else if (fname == "vyb_qt_rich_set_text_color") rtName = "__vyb_qt_rich_set_text_color";
        else if (fname == "vyb_qt_widget_set_font_size") rtName = "__vyb_qt_widget_set_font_size";
        else if (fname == "vyb_qt_widget_set_font_bold") rtName = "__vyb_qt_widget_set_font_bold";
        else if (fname == "vyb_qt_widget_set_text_color") rtName = "__vyb_qt_widget_set_text_color";
        else if (fname == "vyb_qt_file_open_opt") rtName = "__vyb_qt_file_open_opt";
        else if (fname == "vyb_qt_file_save_opt") rtName = "__vyb_qt_file_save_opt";
        else if (fname == "vyb_qt_dir_select_opt") rtName = "__vyb_qt_dir_select_opt";
        else if (fname == "vyb_qt_dlg_selected_opt") rtName = "__vyb_qt_dlg_selected_opt";
        if (!rtName.empty()) {
            auto getQtFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f2 = module->getFunction(rtName);
                if (!f2) f2 = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
                return f2;
            };
            auto toI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy())
                    return builder->CreateSExt(v, int64Type, "qt.toi64");
                return v;
            };
            auto toStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 0, "qt.strptr");
                return v;
            };
            auto strLenOf = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 1, "qt.strlen");
                return llvm::ConstantInt::get(int64Type, 0);
            };
            auto qtStrRet = [&]() -> llvm::StructType* {
                std::vector<llvm::Type*> f = {int8PtrType, int64Type};
                return llvm::StructType::get(*context, f, false);
            };
            auto needArg = [&](size_t idx) -> llvm::Value* {
                if (idx >= node->arguments.size()) { m_currentLLVMValue = nullptr; return nullptr; }
                node->arguments[idx]->accept(*this);
                return m_currentLLVMValue;
            };
            auto checkArity = [&](size_t n) -> bool {
                if (node->arguments.size() != n) {
                    logError(node->loc, rtName + " expects " + std::to_string(n) + " argument(s)");
                    m_currentLLVMValue = nullptr;
                    return false;
                }
                return true;
            };

            if (fname == "vyb_qt_init" || fname == "vyb_qt_quit" || fname == "vyb_qt_active" ||
                fname == "vyb_qt_process_events" ||
                fname == "vyb_qt_timer_fired" ||
                fname == "vyb_qt_window_create" ||
                fname == "vyb_qt_screen_width" ||
                fname == "vyb_qt_screen_height" ||
                fname == "vyb_qt_screen_dpi" ||
                fname == "vyb_qt_event_count" ||
                fname == "vyb_qt_event_handle" ||
                fname == "vyb_qt_event_kind" ||
                fname == "vyb_qt_event_pop" ||
                fname == "vyb_qt_run" ||
                fname == "vyb_qt_run_stop" ||
                fname == "vyb_qt_main_window_create" ||
                fname == "vyb_qt_event_result") {
                if (!checkArity(0)) return;
                llvm::FunctionType* ft0 = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft0), {}, "qt.ret");
                return;
            } else if (fname == "vyb_qt_web_create" || fname == "vyb_qt_web_loading" ||
                fname == "vyb_qt_web_back" ||
                fname == "vyb_qt_web_forward" ||
                fname == "vyb_qt_web_reload" ||
                fname == "vyb_qt_web_zoom_in" ||
                fname == "vyb_qt_web_zoom_out" ||
                fname == "vyb_qt_set_timer" ||
                fname == "vyb_qt_window_close" ||
                fname == "vyb_qt_window_width" ||
                fname == "vyb_qt_window_height" ||
                fname == "vyb_qt_window_show" ||
                fname == "vyb_qt_window_hide" ||
                fname == "vyb_qt_window_visible" ||
                fname == "vyb_qt_checkbox_checked" ||
                fname == "vyb_qt_vbox" ||
                fname == "vyb_qt_hbox" ||
                fname == "vyb_qt_kind" ||
                fname == "vyb_qt_wait_event" ||
                fname == "vyb_qt_combo_create" ||
                fname == "vyb_qt_combo_count" ||
                fname == "vyb_qt_combo_current_index" ||
                fname == "vyb_qt_spin_value" ||
                fname == "vyb_qt_slider_value" ||
                fname == "vyb_qt_dial_value" ||
                fname == "vyb_qt_text_edit_create" ||
                fname == "vyb_qt_radio_checked" ||
                fname == "vyb_qt_widget_enabled" ||
                fname == "vyb_qt_grid" ||
                fname == "vyb_qt_widget_visible" ||
                fname == "vyb_qt_tabs_create" ||
                fname == "vyb_qt_tabs_count" ||
                fname == "vyb_qt_tabs_current" ||
                fname == "vyb_qt_list_create" ||
                fname == "vyb_qt_list_count" ||
                fname == "vyb_qt_list_current" ||
                fname == "vyb_qt_menubar" ||
                fname == "vyb_qt_action_count" ||
                fname == "vyb_qt_dlg_close" ||
                fname == "vyb_qt_rich_create" ||
                fname == "vyb_qt_rich_clear") {
                if (!checkArity(1)) return;
                llvm::Value* a = needArg(0); if (!a) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a)}, "qt.i1");
                return;
            } else if (fname == "vyb_qt_web_url" || fname == "vyb_qt_web_title" ||
                fname == "vyb_qt_window_title" ||
                fname == "vyb_qt_label_text" ||
                fname == "vyb_qt_button_text" ||
                fname == "vyb_qt_edit_text" ||
                fname == "vyb_qt_text_edit_text" ||
                fname == "vyb_qt_statusbar_text" ||
                fname == "vyb_qt_dlg_selected" ||
                fname == "vyb_qt_rich_html" ||
                fname == "vyb_qt_rich_plain") {
                if (!checkArity(1)) return;
                llvm::Value* a = needArg(0); if (!a) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(qtStrRet(), {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a)}, "qt.s1");
                return;
            } else if (fname == "vyb_qt_combo_item_text" || fname == "vyb_qt_list_item_text") {
                if (!checkArity(2)) return;
                llvm::Value* a = needArg(0); llvm::Value* b = needArg(1);
                if (!a || !b) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(qtStrRet(), {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toI64(b)}, "qt.s2");
                return;
            } else if (fname == "vyb_qt_file_open" || fname == "vyb_qt_file_save") {
                if (!checkArity(3)) return;
                llvm::Value* a = needArg(0); llvm::Value* s = needArg(1); llvm::Value* t = needArg(2);
                if (!a || !s || !t) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(qtStrRet(), {int64Type, int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toStrPtr(s), strLenOf(s), toStrPtr(t), strLenOf(t)}, "qt.s3");
                return;
            } else if (fname == "vyb_qt_web_load" || fname == "vyb_qt_window_set_title" ||
                fname == "vyb_qt_label_create" ||
                fname == "vyb_qt_label_set_text" ||
                fname == "vyb_qt_button_create" ||
                fname == "vyb_qt_button_set_text" ||
                fname == "vyb_qt_edit_create" ||
                fname == "vyb_qt_edit_set_text" ||
                fname == "vyb_qt_edit_set_placeholder" ||
                fname == "vyb_qt_checkbox_create" ||
                fname == "vyb_qt_combo_add_item" ||
                fname == "vyb_qt_group_create" ||
                fname == "vyb_qt_text_edit_set_text" ||
                fname == "vyb_qt_radio_create" ||
                fname == "vyb_qt_tabs_add" ||
                fname == "vyb_qt_list_add" ||
                fname == "vyb_qt_menu_add" ||
                fname == "vyb_qt_action_add" ||
                fname == "vyb_qt_statusbar_message" ||
                fname == "vyb_qt_toolbar_create" ||
                fname == "vyb_qt_dlg_dir" ||
                fname == "vyb_qt_rich_set_html" ||
                fname == "vyb_qt_rich_set_plain" ||
                fname == "vyb_qt_rich_append") {
                if (!checkArity(2)) return;
                llvm::Value* a = needArg(0); llvm::Value* s = needArg(1);
                if (!a || !s) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toStrPtr(s), strLenOf(s)}, "qt.text");
                return;
            } else if (fname == "vyb_qt_msg_info" || fname == "vyb_qt_msg_warn" ||
                fname == "vyb_qt_msg_error" ||
                fname == "vyb_qt_msg_about" ||
                fname == "vyb_qt_msg_question" ||
                fname == "vyb_qt_dlg_info" ||
                fname == "vyb_qt_dlg_warn" ||
                fname == "vyb_qt_dlg_error" ||
                fname == "vyb_qt_dlg_about" ||
                fname == "vyb_qt_dlg_question" ||
                fname == "vyb_qt_dlg_open" ||
                fname == "vyb_qt_dlg_save") {
                if (!checkArity(3)) return;
                llvm::Value* a = needArg(0); llvm::Value* s = needArg(1); llvm::Value* t = needArg(2);
                if (!a || !s || !t) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toStrPtr(s), strLenOf(s), toStrPtr(t), strLenOf(t)}, "qt.t2");
                return;
            } else if (fname == "vyb_qt_dir_select") {
                if (!checkArity(2)) return;
                llvm::Value* a = needArg(0); llvm::Value* s = needArg(1);
                if (!a || !s) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(qtStrRet(), {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toStrPtr(s), strLenOf(s)}, "qt.st");
                return;
            } else if (fname == "vyb_qt_button_set_enabled" || fname == "vyb_qt_checkbox_set_checked" ||
                fname == "vyb_qt_progress_create" ||
                fname == "vyb_qt_progress_set_value" ||
                fname == "vyb_qt_layout_add" ||
                fname == "vyb_qt_layout_add_layout" ||
                fname == "vyb_qt_post_event" ||
                fname == "vyb_qt_combo_set_current_index" ||
                fname == "vyb_qt_spin_set_value" ||
                fname == "vyb_qt_slider_set_value" ||
                fname == "vyb_qt_dial_set_value" ||
                fname == "vyb_qt_radio_set_checked" ||
                fname == "vyb_qt_widget_set_enabled" ||
                fname == "vyb_qt_widget_set_visible" ||
                fname == "vyb_qt_tabs_set_current" ||
                fname == "vyb_qt_list_set_current" ||
                fname == "vyb_qt_widget_set_font_size" ||
                fname == "vyb_qt_widget_set_font_bold") {
                if (!checkArity(2)) return;
                llvm::Value* a = needArg(0); llvm::Value* b = needArg(1);
                if (!a || !b) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toI64(b)}, "qt.v2");
                return;
            } else if (fname == "vyb_qt_window_resize" || fname == "vyb_qt_layout_set_stretch" ||
                fname == "vyb_qt_spin_create" ||
                fname == "vyb_qt_slider_create" ||
                fname == "vyb_qt_dial_create") {
                if (!checkArity(3)) return;
                llvm::Value* a = needArg(0); llvm::Value* b = needArg(1); llvm::Value* c = needArg(2);
                if (!a || !b || !c) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toI64(b), toI64(c)}, "qt.i3");
                return;
            } else if (fname == "vyb_qt_grid_add" || fname == "vyb_qt_rich_set_text_color" ||
                fname == "vyb_qt_widget_set_text_color") {
                if (!checkArity(4)) return;
                llvm::Value* a=needArg(0); llvm::Value* b=needArg(1); llvm::Value* c=needArg(2); llvm::Value* d=needArg(3);
                if (!a || !b || !c || !d) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type, int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {toI64(a), toI64(b), toI64(c), toI64(d)}, "qt.i4");
                return;
            } else if (fname == "vyb_qt_on_event") {
                if (!checkArity(1)) return;
                node->arguments[0]->accept(*this);
                llvm::Value* cl = m_currentLLVMValue; if (!cl) return;
                llvm::StructType* closureTy = getClosureStructType();
                llvm::Value* envPtr = nullptr; llvm::Value* fnPtr = nullptr;
                if (cl->getType()->isStructTy()) {
                    envPtr = builder->CreateExtractValue(cl, 0, "qt.env");
                    fnPtr = builder->CreateExtractValue(cl, 1, "qt.fn");
                } else if (cl->getType()->isPointerTy()) {
                    llvm::Value* closureVal = builder->CreateLoad(closureTy, cl, "qt.closure");
                    envPtr = builder->CreateExtractValue(closureVal, 0, "qt.env");
                    fnPtr = builder->CreateExtractValue(closureVal, 1, "qt.fn");
                } else {
                    logError(node->loc, "vyb_qt_on_event argument is not a fn() closure");
                    m_currentLLVMValue = nullptr; return;
                }
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType, int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getQtFn(ft), {envPtr, fnPtr}, "qt.cb");
                return;
            }
            if (fname == "vyb_qt_file_open_opt" || fname == "vyb_qt_file_save_opt") {
                if (!checkArity(3)) return;
                llvm::Value* a = needArg(0); llvm::Value* s = needArg(1); llvm::Value* t = needArg(2);
                if (!a || !s || !t) return;
                llvm::StructType* strTy = qtStrRet();
                llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
                llvm::Value* slotA = builder->CreateAlloca(strTy, nullptr, "qt.opt.slot");
                builder->CreateStore(llvm::Constant::getNullValue(strTy), slotA, "qt.opt.zero");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type, int8PtrType, int64Type, llvm::PointerType::get(*context, 0)}, false);
                llvm::Value* has = builder->CreateCall(getQtFn(ft), {toI64(a), toStrPtr(s), strLenOf(s), toStrPtr(t), strLenOf(t), slotA}, "qt.opt.has");
                llvm::Value* val = builder->CreateLoad(strTy, slotA, "qt.opt.val");
                llvm::Value* opts = llvm::UndefValue::get(optTy);
                opts = builder->CreateInsertValue(opts, val, 0, "qt.opt.v");
                opts = builder->CreateInsertValue(opts, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "qt.opt.h"), 1);
                m_currentLLVMValue = opts;
                return;
            } else if (fname == "vyb_qt_dir_select_opt") {
                if (!checkArity(2)) return;
                llvm::Value* a = needArg(0); llvm::Value* s = needArg(1);
                if (!a || !s) return;
                llvm::StructType* strTy = qtStrRet();
                llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
                llvm::Value* slotA = builder->CreateAlloca(strTy, nullptr, "qt.opt.slot");
                builder->CreateStore(llvm::Constant::getNullValue(strTy), slotA, "qt.opt.zero");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type, llvm::PointerType::get(*context, 0)}, false);
                llvm::Value* has = builder->CreateCall(getQtFn(ft), {toI64(a), toStrPtr(s), strLenOf(s), slotA}, "qt.opt.has");
                llvm::Value* val = builder->CreateLoad(strTy, slotA, "qt.opt.val");
                llvm::Value* opts = llvm::UndefValue::get(optTy);
                opts = builder->CreateInsertValue(opts, val, 0, "qt.opt.v");
                opts = builder->CreateInsertValue(opts, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "qt.opt.h"), 1);
                m_currentLLVMValue = opts;
                return;
            } else if (fname == "vyb_qt_dlg_selected_opt") {
                if (!checkArity(1)) return;
                llvm::Value* a = needArg(0); if (!a) return;
                llvm::StructType* strTy = qtStrRet();
                llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
                llvm::Value* slotA = builder->CreateAlloca(strTy, nullptr, "qt.opt.slot");
                builder->CreateStore(llvm::Constant::getNullValue(strTy), slotA, "qt.opt.zero");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
                llvm::Value* has = builder->CreateCall(getQtFn(ft), {toI64(a), slotA}, "qt.opt.has");
                llvm::Value* val = builder->CreateLoad(strTy, slotA, "qt.opt.val");
                llvm::Value* opts = llvm::UndefValue::get(optTy);
                opts = builder->CreateInsertValue(opts, val, 0, "qt.opt.v");
                opts = builder->CreateInsertValue(opts, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "qt.opt.h"), 1);
                m_currentLLVMValue = opts;
                return;
            }
        }
// gen_qt[cgen]: end
    }

    // Handle Network I/O intrinsics (network stdlib module). Mirrors the File I/O
    // mapping: each Vyb-level call (`vyb_net_*`) resolves to an exported runtime
    // symbol (`__vyb_net_*`) in `runtime/vyb_runtime.c`. IP addresses and payloads
    // arrive as Vyb String structs { ptr, len } and their data pointer (and length
    // for send) is extracted at the boundary. `recv` and the error message return
    // an owned, registry-registered heap buffer so the Vyb String built over it is
    // freed by normal reference-counted cleanup.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        std::string rtName;
        if (fname == "vyb_net_open") rtName = "__vyb_net_open";
        else if (fname == "vyb_net_close") rtName = "__vyb_net_close";
        else if (fname == "vyb_net_bind") rtName = "__vyb_net_bind";
        else if (fname == "vyb_net_listen") rtName = "__vyb_net_listen";
        else if (fname == "vyb_net_accept") rtName = "__vyb_net_accept";
        else if (fname == "vyb_net_connect") rtName = "__vyb_net_connect";
        else if (fname == "vyb_net_send") rtName = "__vyb_net_send";
        else if (fname == "vyb_net_recv") rtName = "__vyb_net_recv";
        else if (fname == "vyb_net_local_port") rtName = "__vyb_net_local_port";
        else if (fname == "vyb_net_error_code") rtName = "__vyb_net_error_code";
        else if (fname == "vyb_net_error_message") rtName = "__vyb_net_error_message";
        else if (fname == "vyb_net_sendto") rtName = "__vyb_net_sendto";
        else if (fname == "vyb_net_recvfrom") rtName = "__vyb_net_recvfrom";
        else if (fname == "vyb_net_last_peer_ip") rtName = "__vyb_net_last_peer_ip";
        else if (fname == "vyb_net_last_peer_port") rtName = "__vyb_net_last_peer_port";
        else if (fname == "vyb_net_last_peer_ip_opt") rtName = "__vyb_net_last_peer_ip_opt";
        else if (fname == "vyb_net_last_peer_port_opt") rtName = "__vyb_net_last_peer_port_opt";
        else if (fname == "vyb_net_resolve") rtName = "__vyb_net_resolve";
        else if (fname == "vyb_base64_encode") rtName = "__vyb_base64_encode";
        else if (fname == "vyb_ws_accept_key") rtName = "__vyb_ws_accept_key";
        if (!rtName.empty()) {
            auto getNetFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f = module->getFunction(rtName);
                if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
                return f;
            };
            auto toI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy())
                    return builder->CreateSExt(v, int64Type, "net.toi64");
                return v;
            };
            auto toStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 0, "net.strptr");
                return v;
            };
            auto strStructType = [&]() {
                std::vector<llvm::Type*> f = {int8PtrType, int64Type};
                return llvm::StructType::get(*context, f, false);
            };

            auto needArg = [&](size_t idx) -> llvm::Value* {
                if (idx >= node->arguments.size()) { m_currentLLVMValue = nullptr; return nullptr; }
                node->arguments[idx]->accept(*this);
                return m_currentLLVMValue;
            };
            auto checkArity = [&](size_t n) -> bool {
                if (node->arguments.size() != n) {
                    logError(node->loc, rtName + " expects " + std::to_string(n) + " argument(s)");
                    m_currentLLVMValue = nullptr;
                    return false;
                }
                return true;
            };

            if (fname == "vyb_net_open") {
                if (!checkArity(3)) return;
                llvm::Value* d = needArg(0); llvm::Value* t = needArg(1); llvm::Value* pr = needArg(2);
                if (!d || !t || !pr) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(d), toI64(t), toI64(pr)}, "net.fd");
                return;
            } else if (fname == "vyb_net_close") {
                if (!checkArity(1)) return;
                llvm::Value* fd = needArg(0); if (!fd) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd)}, "net.closed");
                return;
            } else if (fname == "vyb_net_bind" || fname == "vyb_net_connect") {
                if (!checkArity(3)) return;
                llvm::Value* fd = needArg(0); llvm::Value* ip = needArg(1); llvm::Value* port = needArg(2);
                if (!fd || !ip || !port) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd), toStrPtr(ip), toI64(port)},
                    fname == "vyb_net_bind" ? "net.bound" : "net.connected");
                return;
            } else if (fname == "vyb_net_listen") {
                if (!checkArity(2)) return;
                llvm::Value* fd = needArg(0); llvm::Value* backlog = needArg(1);
                if (!fd || !backlog) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd), toI64(backlog)}, "net.listened");
                return;
            } else if (fname == "vyb_net_accept") {
                if (!checkArity(1)) return;
                llvm::Value* fd = needArg(0); if (!fd) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd)}, "net.accepted");
                return;
            } else if (fname == "vyb_net_send") {
                if (!checkArity(2)) return;
                llvm::Value* fd = needArg(0); llvm::Value* data = needArg(1);
                if (!fd || !data) return;
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "net.strlen");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd), dataPtr, dataLen}, "net.sent");
                return;
            } else if (fname == "vyb_net_recv") {
                if (!checkArity(2)) return;
                llvm::Value* fd = needArg(0); llvm::Value* maxlen = needArg(1);
                if (!fd || !maxlen) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType(), {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd), toI64(maxlen)}, "net.recved");
                return;
            } else if (fname == "vyb_net_sendto") {
                if (!checkArity(4)) return;
                llvm::Value* fd = needArg(0); llvm::Value* data = needArg(1);
                llvm::Value* ip = needArg(2); llvm::Value* port = needArg(3);
                if (!fd || !data || !ip || !port) return;
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "net.udplen");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type,
                    {int64Type, int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft),
                    {toI64(fd), dataPtr, dataLen, toStrPtr(ip), toI64(port)}, "net.udpsent");
                return;
            } else if (fname == "vyb_net_recvfrom") {
                if (!checkArity(2)) return;
                llvm::Value* fd = needArg(0); llvm::Value* maxlen = needArg(1);
                if (!fd || !maxlen) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType(), {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd), toI64(maxlen)}, "net.udprecved");
                return;
            } else if (fname == "vyb_net_last_peer_ip") {
                if (!checkArity(0)) return;
                llvm::FunctionType* fip = llvm::FunctionType::get(int8PtrType, {}, false);
                llvm::Value* ipPtr = builder->CreateCall(getNetFn(fip), {}, "net.peerip");
                llvm::FunctionType* strlenType2 = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                llvm::Function* strlenF2 = module->getFunction("strlen");
                if (!strlenF2)
                    strlenF2 = llvm::Function::Create(strlenType2, llvm::Function::ExternalLinkage, "strlen", module.get());
                llvm::Value* ipLen = builder->CreateCall(strlenF2, {ipPtr}, "net.peerilen");
                llvm::Value* outIp = llvm::UndefValue::get(strStructType());
                outIp = builder->CreateInsertValue(outIp, ipPtr, 0, "net.peerip.data");
                outIp = builder->CreateInsertValue(outIp, ipLen, 1, "net.peerip.len");
                m_currentLLVMValue = outIp;
                return;
            } else if (fname == "vyb_net_last_peer_port") {
                if (!checkArity(0)) return;
                llvm::FunctionType* fpt = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(fpt), {}, "net.peerport");
                return;
            } else if (fname == "vyb_net_last_peer_ip_opt") {
                // Lossless peer IP -> native `String?` ({ String, has }); absent
                // until a datagram is received. The out-slot is zero-initialized
                // so the absent path leaves the payload defined (#137).
                if (!checkArity(0)) return;
                llvm::StructType* strTy = strStructType();
                llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
                llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "net.peerip.slot");
                builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "net.peerip.zero");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {llvm::PointerType::get(*context, 0)}, false);
                llvm::Value* has = builder->CreateCall(getNetFn(ft), {slot}, "net.peerip.has");
                llvm::Value* val = builder->CreateLoad(strTy, slot, "net.peerip.val");
                llvm::Value* opt = llvm::UndefValue::get(optTy);
                opt = builder->CreateInsertValue(opt, val, 0, "net.peerip.v");
                opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "net.peerip.h"), 1);
                m_currentLLVMValue = opt;
                return;
            } else if (fname == "vyb_net_last_peer_port_opt") {
                // Lossless peer port -> native `Int?` ({ i64, has }); absent until
                // a datagram is received. Zero-initialized out-slot (#137).
                if (!checkArity(0)) return;
                llvm::StructType* optTy = llvm::StructType::get(*context, {int64Type, llvm::Type::getInt1Ty(*context)}, false);
                llvm::Value* slot = builder->CreateAlloca(int64Type, nullptr, "net.peerport.slot");
                builder->CreateStore(llvm::ConstantInt::get(int64Type, 0), slot, "net.peerport.zero");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {llvm::PointerType::get(*context, 0)}, false);
                llvm::Value* has = builder->CreateCall(getNetFn(ft), {slot}, "net.peerport.has");
                llvm::Value* val = builder->CreateLoad(int64Type, slot, "net.peerport.val");
                llvm::Value* opt = llvm::UndefValue::get(optTy);
                opt = builder->CreateInsertValue(opt, val, 0, "net.peerport.v");
                opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "net.peerport.h"), 1);
                m_currentLLVMValue = opt;
                return;
            } else if (fname == "vyb_net_local_port") {
                if (!checkArity(1)) return;
                llvm::Value* fd = needArg(0); if (!fd) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toI64(fd)}, "net.port");
                return;
            } else if (fname == "vyb_net_error_code") {
                if (!checkArity(0)) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {}, "net.errcode");
                return;
            } else if (fname == "vyb_net_resolve") {
                if (!checkArity(1)) return;
                llvm::Value* host = needArg(0); if (!host) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType(), {int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {toStrPtr(host)}, "net.resolved");
                return;
            } else if (fname == "vyb_base64_encode") {
                if (!checkArity(1)) return;
                llvm::Value* data = needArg(0); if (!data) return;
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "b64.len");
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType(), {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {dataPtr, dataLen}, "b64.encoded");
                return;
            } else if (fname == "vyb_ws_accept_key") {
                if (!checkArity(1)) return;
                llvm::Value* key = needArg(0); if (!key) return;
                llvm::Value* keyPtr = toStrPtr(key);
                llvm::Value* keyLen = llvm::ConstantInt::get(int64Type, 0);
                if (key->getType()->isStructTy())
                    keyLen = builder->CreateExtractValue(key, 1, "ws.keylen");
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType(), {int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getNetFn(ft), {keyPtr, keyLen}, "ws.accept");
                return;
            } else { // vyb_net_error_message
                if (!checkArity(0)) return;
                llvm::FunctionType* fmsg = llvm::FunctionType::get(int8PtrType, {}, false);
                llvm::Value* msgPtr = builder->CreateCall(getNetFn(fmsg), {}, "net.errmsg");
                llvm::FunctionType* strlenType = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                llvm::Function* strlenFunc = module->getFunction("strlen");
                if (!strlenFunc)
                    strlenFunc = llvm::Function::Create(strlenType, llvm::Function::ExternalLinkage, "strlen", module.get());
                llvm::Value* msgLen = builder->CreateCall(strlenFunc, {msgPtr}, "net.errlen");
                llvm::Value* outStr = llvm::UndefValue::get(strStructType());
                outStr = builder->CreateInsertValue(outStr, msgPtr, 0, "net.msg.data");
                outStr = builder->CreateInsertValue(outStr, msgLen, 1, "net.msg.len");
                m_currentLLVMValue = outStr;
                return;
            }
        }
    }

    // TLS intrinsics (tls stdlib module). Mirrors the net mapping: each Vyb-level
    // call (`vyb_tls_*`) resolves to an exported runtime symbol (`__vyb_tls_*`).
    // SSL/SSL_CTX handles are Ints; PEM cert/key strings arrive as Vyb String
    // structs whose data ptr is extracted at the boundary. `read` and the error
    // message return a String built over an owned, registry-registered buffer.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        std::string rtName;
        if (fname == "vyb_tls_client_context") rtName = "__vyb_tls_client_context";
        else if (fname == "vyb_tls_client_context_verified") rtName = "__vyb_tls_client_context_verified";
        else if (fname == "vyb_tls_server_context") rtName = "__vyb_tls_server_context";
        else if (fname == "vyb_tls_ctx_free") rtName = "__vyb_tls_ctx_free";
        else if (fname == "vyb_tls_stream") rtName = "__vyb_tls_stream";
        else if (fname == "vyb_tls_connect") rtName = "__vyb_tls_connect";
        else if (fname == "vyb_tls_accept") rtName = "__vyb_tls_accept";
        else if (fname == "vyb_tls_write") rtName = "__vyb_tls_write";
        else if (fname == "vyb_tls_read") rtName = "__vyb_tls_read";
        else if (fname == "vyb_tls_close") rtName = "__vyb_tls_close";
        else if (fname == "vyb_tls_error_code") rtName = "__vyb_tls_error_code";
        else if (fname == "vyb_tls_error_message") rtName = "__vyb_tls_error_message";
        if (!rtName.empty()) {
            auto getTlsFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f = module->getFunction(rtName);
                if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
                return f;
            };
            auto toI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy())
                    return builder->CreateSExt(v, int64Type, "tls.toi64");
                return v;
            };
            auto toStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 0, "tls.strptr");
                return v;
            };
            auto strStructType = [&]() {
                std::vector<llvm::Type*> f = {int8PtrType, int64Type};
                return llvm::StructType::get(*context, f, false);
            };
            auto needArg = [&](size_t idx) -> llvm::Value* {
                if (idx >= node->arguments.size()) { m_currentLLVMValue = nullptr; return nullptr; }
                node->arguments[idx]->accept(*this);
                return m_currentLLVMValue;
            };
            auto checkArity = [&](size_t n) -> bool {
                if (node->arguments.size() != n) {
                    logError(node->loc, rtName + " expects " + std::to_string(n) + " argument(s)");
                    m_currentLLVMValue = nullptr;
                    return false;
                }
                return true;
            };

            if (fname == "vyb_tls_client_context") {
                if (!checkArity(0)) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft), {}, "tls.clientctx");
                return;
            } else if (fname == "vyb_tls_client_context_verified") {
                if (!checkArity(1)) return;
                llvm::Value* ca = needArg(0); if (!ca) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft), {toStrPtr(ca)}, "tls.clientctxverify");
                return;
            } else if (fname == "vyb_tls_server_context") {
                if (!checkArity(2)) return;
                llvm::Value* cert = needArg(0); llvm::Value* key = needArg(1);
                if (!cert || !key) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type,
                    {int8PtrType, int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft),
                    {toStrPtr(cert), toStrPtr(key)}, "tls.serverctx");
                return;
            } else if (fname == "vyb_tls_ctx_free") {
                if (!checkArity(1)) return;
                llvm::Value* ctx = needArg(0); if (!ctx) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(voidType, {int64Type}, false);
                builder->CreateCall(getTlsFn(ft), {toI64(ctx)});
                m_currentLLVMValue = nullptr;
                return;
            } else if (fname == "vyb_tls_stream") {
                if (!checkArity(3)) return;
                llvm::Value* ctx = needArg(0); llvm::Value* fd = needArg(1); llvm::Value* host = needArg(2);
                if (!ctx || !fd || !host) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type,
                    {int64Type, int64Type, int8PtrType}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft),
                    {toI64(ctx), toI64(fd), toStrPtr(host)}, "tls.ssl");
                return;
            } else if (fname == "vyb_tls_connect" || fname == "vyb_tls_accept") {
                if (!checkArity(1)) return;
                llvm::Value* ssl = needArg(0); if (!ssl) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft), {toI64(ssl)},
                    fname == "vyb_tls_connect" ? "tls.connected" : "tls.accepted");
                return;
            } else if (fname == "vyb_tls_write") {
                if (!checkArity(2)) return;
                llvm::Value* ssl = needArg(0); llvm::Value* data = needArg(1);
                if (!ssl || !data) return;
                llvm::Value* dataPtr = toStrPtr(data);
                llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dataLen = builder->CreateExtractValue(data, 1, "tls.writelen");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type,
                    {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft),
                    {toI64(ssl), dataPtr, dataLen}, "tls.sent");
                return;
            } else if (fname == "vyb_tls_read") {
                if (!checkArity(2)) return;
                llvm::Value* ssl = needArg(0); llvm::Value* maxlen = needArg(1);
                if (!ssl || !maxlen) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(strStructType(), {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft),
                    {toI64(ssl), toI64(maxlen)}, "tls.read");
                return;
            } else if (fname == "vyb_tls_close") {
                if (!checkArity(2)) return;
                llvm::Value* ssl = needArg(0); llvm::Value* fd = needArg(1);
                if (!ssl || !fd) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft),
                    {toI64(ssl), toI64(fd)}, "tls.closed");
                return;
            } else if (fname == "vyb_tls_error_code") {
                if (!checkArity(0)) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                m_currentLLVMValue = builder->CreateCall(getTlsFn(ft), {}, "tls.errcode");
                return;
            } else { // vyb_tls_error_message
                if (!checkArity(0)) return;
                llvm::FunctionType* fmsg = llvm::FunctionType::get(int8PtrType, {}, false);
                llvm::Value* msgPtr = builder->CreateCall(getTlsFn(fmsg), {}, "tls.errmsg");
                llvm::FunctionType* strlenType = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
                llvm::Function* strlenFunc = module->getFunction("strlen");
                if (!strlenFunc)
                    strlenFunc = llvm::Function::Create(strlenType, llvm::Function::ExternalLinkage, "strlen", module.get());
                llvm::Value* msgLen = builder->CreateCall(strlenFunc, {msgPtr}, "tls.errlen");
                llvm::Value* outStr = llvm::UndefValue::get(strStructType());
                outStr = builder->CreateInsertValue(outStr, msgPtr, 0, "tls.msg.data");
                outStr = builder->CreateInsertValue(outStr, msgLen, 1, "tls.msg.len");
                m_currentLLVMValue = outStr;
                return;
            }
        }
    }

    // Async I/O intrinsics (async stdlib module): non-blocking socket ops that
    // suspend the calling fiber until the fd is ready. accept / send / connect /
    // io_wait return Int; recv returns a String. Mirrors the net mapping: IPs
    // and payloads arrive as Vyb String structs and their data ptr (and len for
    // send) is extracted at the boundary.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        std::string rtAsync;
        if (fname == "vyb_async_io_wait") rtAsync = "__vyb_async_io_wait";
        else if (fname == "vyb_async_accept") rtAsync = "__vyb_async_accept";
        else if (fname == "vyb_async_connect") rtAsync = "__vyb_async_connect";
        else if (fname == "vyb_async_send") rtAsync = "__vyb_async_send";
        else if (fname == "vyb_async_recv") rtAsync = "__vyb_async_recv";
        else if (fname == "vyb_async_recv_opt") rtAsync = "__vyb_async_recv_opt";
        else if (fname == "vyb_async_recvfrom_opt") rtAsync = "__vyb_async_recvfrom_opt";
        else if (fname == "vyb_async_sendto") rtAsync = "__vyb_async_sendto";
        if (!rtAsync.empty()) {
            auto asyncFn = [&](llvm::FunctionType* ft) -> llvm::Function* {
                llvm::Function* f = module->getFunction(rtAsync);
                if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtAsync, module.get());
                return f;
            };
            auto aToI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v) return v;
                if (v->getType()->isIntegerTy(64)) return v;
                if (v->getType()->isIntegerTy())
                    return builder->CreateSExt(v, int64Type, "aio.toi64");
                return v;
            };
            auto aStrPtr = [&](llvm::Value* v) -> llvm::Value* {
                if (v && v->getType()->isStructTy())
                    return builder->CreateExtractValue(v, 0, "aio.strptr");
                return v;
            };
            auto aStrType = [&]() {
                std::vector<llvm::Type*> f = {int8PtrType, int64Type};
                return llvm::StructType::get(*context, f, false);
            };
            auto aNeed = [&](size_t idx) -> llvm::Value* {
                if (idx >= node->arguments.size()) { m_currentLLVMValue = nullptr; return nullptr; }
                node->arguments[idx]->accept(*this);
                return m_currentLLVMValue;
            };
            auto aArity = [&](size_t n) -> bool {
                if (node->arguments.size() != n) {
                    logError(node->loc, rtAsync + " expects " + std::to_string(n) + " argument(s)");
                    m_currentLLVMValue = nullptr;
                    return false;
                }
                return true;
            };

            if (fname == "vyb_async_io_wait") {
                if (!aArity(2)) return;
                llvm::Value* fd = aNeed(0); llvm::Value* write = aNeed(1);
                if (!fd || !write) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(asyncFn(ft), {aToI64(fd), aToI64(write)}, "aio.wait");
                return;
            } else if (fname == "vyb_async_accept") {
                if (!aArity(1)) return;
                llvm::Value* fd = aNeed(0); if (!fd) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(asyncFn(ft), {aToI64(fd)}, "aio.accepted");
                return;
            } else if (fname == "vyb_async_connect") {
                if (!aArity(3)) return;
                llvm::Value* fd = aNeed(0); llvm::Value* ip = aNeed(1); llvm::Value* port = aNeed(2);
                if (!fd || !ip || !port) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(asyncFn(ft), {aToI64(fd), aStrPtr(ip), aToI64(port)}, "aio.connected");
                return;
            } else if (fname == "vyb_async_send") {
                if (!aArity(2)) return;
                llvm::Value* fd = aNeed(0); llvm::Value* data = aNeed(1);
                if (!fd || !data) return;
                llvm::Value* dp = aStrPtr(data);
                llvm::Value* dl = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dl = builder->CreateExtractValue(data, 1, "aio.strlen");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(asyncFn(ft), {aToI64(fd), dp, dl}, "aio.sent");
                return;
            } else if (fname == "vyb_async_recv") {
                if (!aArity(2)) return;
                llvm::Value* fd = aNeed(0); llvm::Value* maxlen = aNeed(1);
                if (!fd || !maxlen) return;
                llvm::FunctionType* ft = llvm::FunctionType::get(aStrType(), {int64Type, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(asyncFn(ft), {aToI64(fd), aToI64(maxlen)},
                    "aio.recved");
                return;
            } else if (fname == "vyb_async_recv_opt" || fname == "vyb_async_recvfrom_opt") {
                // Lossless async receive -> native `String?` ({ String, has });
                // absent on error, present holding the bytes read (a present-
                // empty datagram/EOF is distinct from failure). Only the return
                // shape differs from the bare forms; the underlying read still
                // suspends the calling fiber.
                if (!aArity(2)) return;
                llvm::Value* fd = aNeed(0); llvm::Value* maxlen = aNeed(1);
                if (!fd || !maxlen) return;
                llvm::StructType* strTy = aStrType();
                llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
                llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "aio.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "aio.zero");
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    int64Type, {int64Type, int64Type, llvm::PointerType::get(*context, 0)}, false);
                llvm::Value* has = builder->CreateCall(asyncFn(ft), {aToI64(fd), aToI64(maxlen), slot}, "aio.has");
                llvm::Value* val = builder->CreateLoad(strTy, slot, "aio.val");
                llvm::Value* opt = llvm::UndefValue::get(optTy);
                opt = builder->CreateInsertValue(opt, val, 0, "aio.v");
                opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "aio.h"), 1);
                m_currentLLVMValue = opt;
                return;
            } else { // vyb_async_sendto
                if (!aArity(4)) return;
                llvm::Value* fd = aNeed(0); llvm::Value* data = aNeed(1);
                llvm::Value* ip = aNeed(2); llvm::Value* port = aNeed(3);
                if (!fd || !data || !ip || !port) return;
                llvm::Value* dp = aStrPtr(data);
                llvm::Value* dl = llvm::ConstantInt::get(int64Type, 0);
                if (data->getType()->isStructTy())
                    dl = builder->CreateExtractValue(data, 1, "aio.udplen");
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type,
                    {int64Type, int8PtrType, int64Type, int8PtrType, int64Type}, false);
                m_currentLLVMValue = builder->CreateCall(asyncFn(ft),
                    {aToI64(fd), dp, dl, aStrPtr(ip), aToI64(port)}, "aio.udpsent");
                return;
            }
        }
    }

    // Time module intrinsics: each vyb_time_* maps to a no-arg (or one-arg
    // sleep) Int-returning runtime symbol in `runtime/vyb_runtime.c`.
    if (identCallee) {
        const std::string& tname = identCallee->name;
        std::string rtTime;
        if (tname == "vyb_time_epoch_secs") rtTime = "__vyb_time_epoch_secs";
        else if (tname == "vyb_time_epoch_millis") rtTime = "__vyb_time_epoch_millis";
        else if (tname == "vyb_time_nanos") rtTime = "__vyb_time_nanos";
        else if (tname == "vyb_time_mono_millis") rtTime = "__vyb_time_mono_millis";
        else if (tname == "vyb_time_sleep_ms") rtTime = "__vyb_time_sleep_ms";
        if (!rtTime.empty()) {
            llvm::Function* f = module->getFunction(rtTime);
            if (!f) {
                llvm::FunctionType* ft = (tname == "vyb_time_sleep_ms")
                    ? llvm::FunctionType::get(int64Type, {int64Type}, false)
                    : llvm::FunctionType::get(int64Type, {}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtTime, module.get());
            }
            if (tname == "vyb_time_sleep_ms") {
                if (node->arguments.size() != 1) {
                    logError(node->loc, rtTime + " expects 1 argument (millis)");
                    m_currentLLVMValue = nullptr; return;
                }
                node->arguments[0]->accept(*this);
                llvm::Value* ms = m_currentLLVMValue;
                if (!ms) return;
                if (ms->getType()->isIntegerTy(64)) m_currentLLVMValue = builder->CreateCall(f, {ms}, "time.slept");
                else if (ms->getType()->isIntegerTy()) m_currentLLVMValue = builder->CreateCall(f, {builder->CreateSExt(ms, int64Type, "time.ms")}, "time.slept");
                else m_currentLLVMValue = nullptr;
                return;
            }
            if (!node->arguments.empty()) {
                logError(node->loc, rtTime + " expects no arguments");
                m_currentLLVMValue = nullptr; return;
            }
            m_currentLLVMValue = builder->CreateCall(f, {}, "time.value");
            return;
        }
    }

    // Handle Threads intrinsics (threads stdlib module). `vyb_thread_spawn`
    // takes a `fn() -> Int` closure value `{ env, fn }`: unpack the two
    // pointers and hand them to the pthread trampoline so it can run the
    // closure with its hidden environment parameter. The rest take/return plain
    // Int handles.
    if (identCallee) {
        const std::string& fname = identCallee->name;
        if (fname == "vyb_thread_spawn") {
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_thread_spawn expects 1 argument (fn() -> Int)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* cl = m_currentLLVMValue;
            if (!cl) return;
            llvm::StructType* closureTy = getClosureStructType();
            llvm::Value* envPtr = nullptr;
            llvm::Value* fnPtr = nullptr;
            if (cl->getType()->isStructTy()) {
                envPtr = builder->CreateExtractValue(cl, 0, "thr.env");
                fnPtr = builder->CreateExtractValue(cl, 1, "thr.fn");
            } else if (cl->getType()->isPointerTy()) {
                llvm::Value* closureVal = builder->CreateLoad(closureTy, cl, "thr.closure");
                envPtr = builder->CreateExtractValue(closureVal, 0, "thr.env");
                fnPtr = builder->CreateExtractValue(closureVal, 1, "thr.fn");
            } else {
                logError(node->loc, "vyb_thread_spawn argument is not a fn() closure");
                m_currentLLVMValue = nullptr; return;
            }
            llvm::Function* f = module->getFunction("__vyb_thread_spawn");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    int64Type, {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0)}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_thread_spawn", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {envPtr, fnPtr}, "thr.handle");
            return;
        } else if (fname == "agent_start" || fname == "agent_start_bool" ||
                   fname == "agent_start_float" || fname == "agent_start_string" ||
                   fname == "vyb_agent_start" || fname == "vyb_agent_start_bool" ||
                   fname == "vyb_agent_start_float" || fname == "vyb_agent_start_string") {
            // Agents: `agent_start*` takes a `fn(Payload?) -> Void` behavior closure
            // value `{ env, fn }`. The two pointers go to the runtime, which creates
            // the mailbox (Int/Bool/Float share the int-slot channel; String uses a
            // strchan) and runs the behavior loop on a worker thread. A behavior
            // that contains `fail` is compiled with the failable `{i1, i8*}` return
            // ABI; we pass that flag so the runtime captures (rather than drops)
            // the propagated error (failure channeling, Stage 4). The agent handle
            // is an Int (a runtime table index).
            const char* rtName = (fname == "agent_start" || fname == "vyb_agent_start") ? "__vyb_agent_start"
                              : (fname == "agent_start_bool" || fname == "vyb_agent_start_bool") ? "__vyb_agent_start_bool"
                              : (fname == "agent_start_float" || fname == "vyb_agent_start_float") ? "__vyb_agent_start_float"
                              : "__vyb_agent_start_string";
            std::string rt(rtName);
            if (node->arguments.size() != 1 && node->arguments.size() != 2) {
                logError(node->loc, fname + " expects the behavior closure and an optional mailbox capacity");
                m_currentLLVMValue = nullptr; return;
            }
            // The failable flag is a compile-time property of the behavior lambda
            // (1 when it can propagate a failure, 0 for a plain Void behavior).
            int failable = 0;
            if (auto* fe = dynamic_cast<ast::FunctionExpression*>(node->arguments[0].get())) {
                auto* optionalPayload = fe->params.size() == 1 && fe->params[0].typeNode
                    ? dynamic_cast<ast::OptionalType*>(fe->params[0].typeNode.get()) : nullptr;
                const char* expectedPayload = (rtName == std::string("__vyb_agent_start")) ? "Int"
                    : (rtName == std::string("__vyb_agent_start_bool")) ? "Bool"
                    : (rtName == std::string("__vyb_agent_start_float")) ? "Float" : "String";
                if (!optionalPayload || !optionalPayload->containedType ||
                    optionalPayload->containedType->toString() != expectedPayload) {
                    logError(node->loc, fname + " behavior must accept exactly one " +
                        std::string(expectedPayload) + "? parameter");
                    m_currentLLVMValue = nullptr; return;
                }
                failable = nodeCanFail(fe) ? 1 : 0;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* cl = m_currentLLVMValue;
            if (!cl) return;
            llvm::StructType* closureTy = getClosureStructType();
            llvm::Value* envPtr = nullptr;
            llvm::Value* fnPtr = nullptr;
            if (cl->getType()->isStructTy()) {
                envPtr = builder->CreateExtractValue(cl, 0, "agent.env");
                fnPtr = builder->CreateExtractValue(cl, 1, "agent.fn");
            } else if (cl->getType()->isPointerTy()) {
                llvm::Value* closureVal = builder->CreateLoad(closureTy, cl, "agent.closure");
                envPtr = builder->CreateExtractValue(closureVal, 0, "agent.env");
                fnPtr = builder->CreateExtractValue(closureVal, 1, "agent.fn");
            } else {
                logError(node->loc, "vyb_agent_start argument is not a fn() closure");
                m_currentLLVMValue = nullptr; return;
            }
            llvm::Function* f = module->getFunction(rt);
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type,
                    {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0),
                     int64Type, int64Type}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rt, module.get());
            }
            // Optional bounded mailbox capacity (0 = unbounded, like chan_new).
            llvm::Value* capV = llvm::ConstantInt::get(int64Type, 0);
            if (node->arguments.size() == 2) {
                node->arguments[1]->accept(*this);
                llvm::Value* capArg = m_currentLLVMValue;
                if (capArg) {
                    if (capArg->getType()->isIntegerTy() && !capArg->getType()->isIntegerTy(64))
                        capArg = builder->CreateSExt(capArg, int64Type, "agent.cap.toi64");
                    capV = capArg;
                }
            }
            llvm::Value* failableV = llvm::ConstantInt::get(int64Type, failable);
            llvm::Value* handle = builder->CreateCall(f, {envPtr, fnPtr, failableV, capV}, "agent.handle");
            // Return a native `Int?`: present holding the live agent handle,
            // absent when the runtime failed to spawn (the old 0 sentinel).
            llvm::StructType* optTy = llvm::StructType::get(*context,
                {int64Type, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* present = builder->CreateICmpNE(handle,
                llvm::ConstantInt::get(int64Type, 0), "agent.has");
            llvm::Value* optVal = llvm::UndefValue::get(optTy);
            optVal = builder->CreateInsertValue(optVal, handle, 0, "agent.v");
            optVal = builder->CreateInsertValue(optVal, present, 1, "agent.h");
            m_currentLLVMValue = optVal;
            return;
        } else if (fname == "vyb_task_spawn") {
            // Tasks (tasks stdlib module): same closure { env, fn } unpack as
            // thread_spawn, but the task delivers its result to a private
            // capacity-1 channel; the returned handle is that channel.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_task_spawn expects 1 argument (fn() -> Int)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* cl = m_currentLLVMValue;
            if (!cl) return;
            llvm::StructType* closureTy = getClosureStructType();
            llvm::Value* envPtr = nullptr;
            llvm::Value* fnPtr = nullptr;
            if (cl->getType()->isStructTy()) {
                envPtr = builder->CreateExtractValue(cl, 0, "task.env");
                fnPtr = builder->CreateExtractValue(cl, 1, "task.fn");
            } else if (cl->getType()->isPointerTy()) {
                llvm::Value* closureVal = builder->CreateLoad(closureTy, cl, "task.closure");
                envPtr = builder->CreateExtractValue(closureVal, 0, "task.env");
                fnPtr = builder->CreateExtractValue(closureVal, 1, "task.fn");
            } else {
                logError(node->loc, "vyb_task_spawn argument is not a fn() closure");
                m_currentLLVMValue = nullptr; return;
            }
            llvm::Function* f = module->getFunction("__vyb_task_spawn");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    int64Type, {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0)}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_task_spawn", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {envPtr, fnPtr}, "task.handle");
            return;
        } else if (fname == "vyb_async_spawn") {
            // Async (async stdlib module): enqueue a `fn() -> Int` closure as an
            // event-loop fiber. Same { env, fn } unpack as task_spawn.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_async_spawn expects 1 argument (fn() -> Int)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* cl = m_currentLLVMValue;
            if (!cl) return;
            llvm::StructType* closureTy = getClosureStructType();
            llvm::Value* envPtr = nullptr;
            llvm::Value* fnPtr = nullptr;
            if (cl->getType()->isStructTy()) {
                envPtr = builder->CreateExtractValue(cl, 0, "async.env");
                fnPtr = builder->CreateExtractValue(cl, 1, "async.fn");
            } else if (cl->getType()->isPointerTy()) {
                llvm::Value* closureVal = builder->CreateLoad(closureTy, cl, "async.closure");
                envPtr = builder->CreateExtractValue(closureVal, 0, "async.env");
                fnPtr = builder->CreateExtractValue(closureVal, 1, "async.fn");
            } else {
                logError(node->loc, "vyb_async_spawn argument is not a fn() closure");
                m_currentLLVMValue = nullptr; return;
            }
            llvm::Function* f = module->getFunction("__vyb_async_spawn");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    int64Type, {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0)}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_async_spawn", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {envPtr, fnPtr}, "async.handle");
            return;
        } else if (fname == "vyb_thread_join" || fname == "vyb_thread_detach" ||
                   fname == "vyb_mutex_lock" || fname == "vyb_mutex_unlock" ||
                   fname == "vyb_mutex_free") {
            if (node->arguments.size() != 1) {
                logError(node->loc, fname + " expects 1 argument");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* arg = m_currentLLVMValue;
            if (!arg) return;
            llvm::Value* a = arg;
            if (a->getType()->isIntegerTy() && !a->getType()->isIntegerTy(64))
                a = builder->CreateSExt(a, int64Type, "thr.toi64");
            std::string rtName = (fname == "vyb_thread_join") ? "__vyb_thread_join"
                              : (fname == "vyb_thread_detach") ? "__vyb_thread_detach"
                              : (fname == "vyb_mutex_lock") ? "__vyb_mutex_lock"
                              : (fname == "vyb_mutex_unlock") ? "__vyb_mutex_unlock" : "__vyb_mutex_free";
            llvm::Function* f = module->getFunction(rtName);
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {a}, "thr.ret");
            return;
        } else if (fname == "vyb_mutex_new") {
            if (!node->arguments.empty()) {
                logError(node->loc, "vyb_mutex_new expects no arguments");
                m_currentLLVMValue = nullptr; return;
            }
            llvm::Function* f = module->getFunction("__vyb_mutex_new");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_mutex_new", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {}, "mutex.handle");
            return;
        }

        // CondVar + AtomicInt intrinsics (threads stdlib module). Each takes a
        // fixed number of Int arguments (handles / values) and returns an Int.
        // A small helper evaluates/sexts the args and lowers to the `__vyb_*`
        // runtime function, mirroring the mutex/thread_join pattern.
        auto emitHandleIntrinsic = [&](const std::string& rtName, size_t nArgs) -> void {
            if (node->arguments.size() != nArgs) {
                logError(node->loc, fname + " expects " + std::to_string(nArgs) + " argument(s)");
                m_currentLLVMValue = nullptr; return;
            }
            llvm::Function* f = module->getFunction(rtName);
            if (!f) {
                std::vector<llvm::Type*> params(nArgs, int64Type);
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, params, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtName, module.get());
            }
            std::vector<llvm::Value*> args;
            args.reserve(nArgs);
            for (size_t i = 0; i < nArgs; ++i) {
                node->arguments[i]->accept(*this);
                llvm::Value* a = m_currentLLVMValue;
                if (!a) { m_currentLLVMValue = nullptr; return; }
                if (a->getType()->isIntegerTy() && !a->getType()->isIntegerTy(64))
                    a = builder->CreateSExt(a, int64Type, "thr.toi64");
                args.push_back(a);
            }
            m_currentLLVMValue = builder->CreateCall(f, args, "thr.ret");
        };

        if (fname == "vyb_cond_new") {
            emitHandleIntrinsic("__vyb_cond_new", 0);
            return;
        } else if (fname == "vyb_cond_signal" || fname == "vyb_cond_broadcast" ||
                   fname == "vyb_cond_free") {
            emitHandleIntrinsic((fname == "vyb_cond_signal") ? "__vyb_cond_signal"
                             : (fname == "vyb_cond_broadcast") ? "__vyb_cond_broadcast"
                             : "__vyb_cond_free", 1);
            return;
        } else if (fname == "vyb_cond_wait") {
            emitHandleIntrinsic("__vyb_cond_wait", 2);
            return;
        } else if (fname == "vyb_atomic_new") {
            emitHandleIntrinsic("__vyb_atomic_new", 1);
            return;
        } else if (fname == "vyb_atomic_load" || fname == "vyb_atomic_free") {
            emitHandleIntrinsic((fname == "vyb_atomic_load") ? "__vyb_atomic_load" : "__vyb_atomic_free", 1);
            return;
        } else if (fname == "vyb_atomic_store") {
            emitHandleIntrinsic("__vyb_atomic_store", 2);
            return;
        } else if (fname == "vyb_atomic_add") {
            emitHandleIntrinsic("__vyb_atomic_add", 2);
            return;
        } else if (fname == "vyb_atomic_cas") {
            emitHandleIntrinsic("__vyb_atomic_cas", 3);
            return;
        } else if (fname == "vyb_agent_send") {
            // agents: post an Int message (non-blocking).
            emitHandleIntrinsic("__vyb_agent_send", 2);
            return;
        } else if (fname == "vyb_agent_send_bool") {
            // Post a Bool message: sign-extend the i1 payload to its i64 slot.
            if (node->arguments.size() != 2) {
                logError(node->loc, "vyb_agent_send_bool expects 2 arguments (agent, bool)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* bh = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* bv = m_currentLLVMValue;
            if (!bh || !bv) return;
            llvm::Value* h64 = bh;
            if (h64->getType()->isIntegerTy() && !h64->getType()->isIntegerTy(64))
                h64 = builder->CreateSExt(h64, int64Type, "agent.toi64");
            llvm::Value* b64 = builder->CreateZExt(bv, int64Type, "agent.bool.bits");
            llvm::Function* f = module->getFunction("__vyb_agent_send_bool");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_agent_send_bool", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {h64, b64}, "agent.sent.bool");
            return;
        } else if (fname == "vyb_agent_send_float") {
            // Post a Float message: bitcast the f64 payload into its i64 slot.
            if (node->arguments.size() != 2) {
                logError(node->loc, "vyb_agent_send_float expects 2 arguments (agent, float)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* fh = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* fv = m_currentLLVMValue;
            if (!fh || !fv) return;
            llvm::Value* h64 = fh;
            if (h64->getType()->isIntegerTy() && !h64->getType()->isIntegerTy(64))
                h64 = builder->CreateSExt(h64, int64Type, "agent.toi64");
            llvm::Value* bits = builder->CreateBitCast(fv, int64Type, "agent.float.bits");
            llvm::Function* f = module->getFunction("__vyb_agent_send_float");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int64Type}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_agent_send_float", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {h64, bits}, "agent.sent.float");
            return;
        } else if (fname == "vyb_agent_send_string") {
            // Post a String message: retain the data ptr and store { ptr, len }
            // in the strchan mailbox. Lowers to (int64, i8*, i64).
            if (node->arguments.size() != 2) {
                logError(node->loc, "vyb_agent_send_string expects 2 arguments (agent, string)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* sh = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* sv = m_currentLLVMValue;
            if (!sh || !sv) return;
            llvm::Value* h64 = sh;
            if (h64->getType()->isIntegerTy() && !h64->getType()->isIntegerTy(64))
                h64 = builder->CreateSExt(h64, int64Type, "agent.toi64");
            llvm::Value* dataPtr = sv;
            llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
            if (sv->getType()->isStructTy()) {
                dataPtr = builder->CreateExtractValue(sv, 0, "agent.strptr");
                dataLen = builder->CreateExtractValue(sv, 1, "agent.strlen");
            }
            llvm::Function* f = module->getFunction("__vyb_agent_send_string");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type, int8PtrType, int64Type}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_agent_send_string", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {h64, dataPtr, dataLen}, "agent.sent.str");
            return;
        } else if (fname == "vyb_agent_len") {
            emitHandleIntrinsic("__vyb_agent_len", 1);
            return;
        } else if (fname == "vyb_agent_alive") {
            emitHandleIntrinsic("__vyb_agent_alive", 1);
            return;
        } else if (fname == "vyb_agent_close") {
            emitHandleIntrinsic("__vyb_agent_close", 1);
            return;
        } else if (fname == "vyb_agent_free") {
            emitHandleIntrinsic("__vyb_agent_free", 1);
            return;
        } else if (fname == "vyb_agent_mailbox") {
            emitHandleIntrinsic("__vyb_agent_mailbox", 1);
            return;
        } else if (fname == "vyb_agent_status") {
            emitHandleIntrinsic("__vyb_agent_status", 1);
            return;
        } else if (fname == "vyb_agent_error_code") {
            emitHandleIntrinsic("__vyb_agent_error_code", 1);
            return;
        } else if (fname == "vyb_agent_set_dead_letter") {
            emitHandleIntrinsic("__vyb_agent_set_dead_letter", 2);
            return;
        } else if (fname == "vyb_agent_error") {
            // Failure descriptor: the runtime returns a registry-owned char*
            // buffer; wrap it (with strlen) as a Vyb String, mirroring the
            // net/tls error-message handling.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_agent_error expects 1 argument (agent)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* h = m_currentLLVMValue;
            if (!h) return;
            if (h->getType()->isIntegerTy() && !h->getType()->isIntegerTy(64))
                h = builder->CreateSExt(h, int64Type, "agenterr.toi64");
            llvm::FunctionType* ft = llvm::FunctionType::get(int8PtrType, {int64Type}, false);
            llvm::Function* f = module->getFunction("__vyb_agent_error");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_agent_error", module.get());
            llvm::Value* msgPtr = builder->CreateCall(f, {h}, "agent.errmsg");
            llvm::FunctionType* strlenTy = llvm::FunctionType::get(int64Type, {int8PtrType}, false);
            llvm::Function* strlenFn = module->getFunction("strlen");
            if (!strlenFn) strlenFn = llvm::Function::Create(strlenTy, llvm::Function::ExternalLinkage, "strlen", module.get());
            llvm::Value* msgLen = builder->CreateCall(strlenFn, {msgPtr}, "agent.errlen");
            llvm::StructType* agentStrTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::Value* out = llvm::UndefValue::get(agentStrTy);
            out = builder->CreateInsertValue(out, msgPtr, 0, "agent.err.data");
            out = builder->CreateInsertValue(out, msgLen, 1, "agent.err.len");
            m_currentLLVMValue = out;
            return;
        } else if (fname == "vyb_chan_new") {
            emitHandleIntrinsic("__vyb_chan_new", 1);   // capacity (0 = unbounded)
            return;
        } else if (fname == "vyb_chan_recv") {
            emitHandleIntrinsic("__vyb_chan_recv", 1);  // blocking
            return;
        } else if (fname == "vyb_chan_recv_opt") {
            // Lossless blocking recv returning a native `Int?` (`{ value, has }`);
            // absent only when the channel is closed and drained.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_chan_recv_opt expects 1 argument (ch)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* ch = m_currentLLVMValue;
            if (!ch) return;
            llvm::Value* ch64 = ch;
            if (ch64->getType()->isIntegerTy() && !ch64->getType()->isIntegerTy(64))
                ch64 = builder->CreateSExt(ch64, int64Type, "chanopt.toi64");
            llvm::StructType* optTy = llvm::StructType::get(*context,
                {int64Type, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(int64Type, nullptr, "chanopt.slot");
            builder->CreateStore(llvm::Constant::getNullValue(int64Type), slot, "chanopt.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_chan_recv_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_chan_recv_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {ch64, slot}, "chanopt.has");
            llvm::Value* val = builder->CreateLoad(int64Type, slot, "chanopt.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "chanopt.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "chanopt.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_chan_try") {
            emitHandleIntrinsic("__vyb_chan_try", 1);   // non-blocking
            return;
        } else if (fname == "vyb_chan_try_opt") {
            // Lossless non-blocking try returning a native `Int?` (`{ value, has }`);
            // absent when the channel is empty or closed. Mirrors vyb_chan_recv_opt.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_chan_try_opt expects 1 argument (ch)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* ch = m_currentLLVMValue;
            if (!ch) return;
            llvm::Value* ch64 = ch;
            if (ch64->getType()->isIntegerTy() && !ch64->getType()->isIntegerTy(64))
                ch64 = builder->CreateSExt(ch64, int64Type, "chantry.toi64");
            llvm::StructType* optTy = llvm::StructType::get(*context,
                {int64Type, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(int64Type, nullptr, "chantry.slot");
            builder->CreateStore(llvm::Constant::getNullValue(int64Type), slot, "chantry.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_chan_try_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_chan_try_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {ch64, slot}, "chantry.has");
            llvm::Value* val = builder->CreateLoad(int64Type, slot, "chantry.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "chantry.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "chantry.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_chan_len") {
            emitHandleIntrinsic("__vyb_chan_len", 1);
            return;
        } else if (fname == "vyb_chan_close") {
            emitHandleIntrinsic("__vyb_chan_close", 1);  // mark closed, wake waiters
            return;
        } else if (fname == "vyb_chan_free") {
            emitHandleIntrinsic("__vyb_chan_free", 1);
            return;
        } else if (fname == "vyb_chan_send") {
            emitHandleIntrinsic("__vyb_chan_send", 2);  // (chan, value) -> 1/0
            return;
        } else if (fname == "vyb_chan_select") {
            // `chan_select(handles<Vec<Int>>)` blocks until one of the listed
            // channels is ready and returns its index (without consuming).
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_chan_select expects 1 argument (Vec<Int> of handles)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* vec = m_currentLLVMValue;
            if (!vec) return;
            llvm::Value* dataPtr = nullptr;
            llvm::Value* sizeVal = nullptr;
            if (vec->getType()->isStructTy()) {
                dataPtr = builder->CreateExtractValue(vec, 0, "sel.data");
                sizeVal = builder->CreateExtractValue(vec, 1, "sel.size");
            } else {
                logError(node->loc, "vyb_chan_select expects a Vec<Int> of channel handles");
                m_currentLLVMValue = nullptr; return;
            }
            dataPtr = builder->CreateBitCast(dataPtr, llvm::PointerType::get(*context, 0), "sel.dataptr");
            llvm::Function* f = module->getFunction("__vyb_chan_select");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(
                    int64Type, {llvm::PointerType::get(*context, 0), int64Type}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_chan_select", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {dataPtr, sizeVal}, "sel.idx");
            return;
        } else if (fname == "vyb_strchan_new") {
            emitHandleIntrinsic("__vyb_strchan_new", 1);   // capacity (0 = unbounded)
            return;
        } else if (fname == "vyb_strchan_send") {
            // `strchan_send(ch, s<String>)`: retain the string data ptr, store
            // { ptr, len } in the channel. Lowers to (int64, i8*, i64).
            if (node->arguments.size() != 2) {
                logError(node->loc, "vyb_strchan_send expects 2 arguments (ch, string)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* ch = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* s = m_currentLLVMValue;
            if (!ch || !s) return;
            llvm::Value* ch64 = ch;
            if (ch64->getType()->isIntegerTy() && !ch64->getType()->isIntegerTy(64))
                ch64 = builder->CreateSExt(ch64, int64Type, "strchan.toi64");
            llvm::Value* dataPtr = s;
            llvm::Value* dataLen = llvm::ConstantInt::get(int64Type, 0);
            if (s->getType()->isStructTy()) {
                dataPtr = builder->CreateExtractValue(s, 0, "strchan.strptr");
                dataLen = builder->CreateExtractValue(s, 1, "strchan.strlen");
            }
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, int8PtrType, int64Type}, false);
            llvm::Function* f = module->getFunction("__vyb_strchan_send");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_strchan_send", module.get());
            m_currentLLVMValue = builder->CreateCall(f, {ch64, dataPtr, dataLen}, "strchan.sent");
            return;
        } else if (fname == "vyb_strchan_recv_opt") {
            // Lossless blocking recv returning a native `String?` (`{ String, has }`);
            // transfers the channel's reference to the caller on a present result.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_strchan_recv_opt expects 1 argument (ch)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* ch = m_currentLLVMValue;
            if (!ch) return;
            llvm::Value* ch64 = ch;
            if (ch64->getType()->isIntegerTy() && !ch64->getType()->isIntegerTy(64))
                ch64 = builder->CreateSExt(ch64, int64Type, "stropt.toi64");
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "stropt.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "stropt.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_strchan_recv_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_strchan_recv_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {ch64, slot}, "stropt.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "stropt.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "stropt.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "stropt.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_strchan_try_opt") {
            // Lossless non-blocking try returning a native `String?` (`{ String, has }`);
            // absent when the channel is empty or closed. Mirrors vyb_strchan_recv_opt.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_strchan_try_opt expects 1 argument (ch)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* ch = m_currentLLVMValue;
            if (!ch) return;
            llvm::Value* ch64 = ch;
            if (ch64->getType()->isIntegerTy() && !ch64->getType()->isIntegerTy(64))
                ch64 = builder->CreateSExt(ch64, int64Type, "strtry.toi64");
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "strtry.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "strtry.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_strchan_try_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_strchan_try_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {ch64, slot}, "strtry.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "strtry.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "strtry.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "strtry.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_env_get_opt") {
            // Lossless env lookup returning a native `String?` (`{ String, has }`);
            // absent when the variable is unset, present holding its value (which
            // may legitimately be empty). Mirrors vyb_strchan_recv_opt.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_env_get_opt expects 1 argument (name)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* nm = m_currentLLVMValue;
            if (!nm) return;
            llvm::Value* namePtr = nm;
            if (namePtr->getType()->isStructTy())
                namePtr = builder->CreateExtractValue(namePtr, 0, "envopt.name");
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "envopt.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "envopt.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int8PtrType, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_env_get_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_env_get_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {namePtr, slot}, "envopt.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "envopt.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "envopt.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "envopt.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_regex_capture_match_opt" || fname == "vyb_regex_capture_opt") {
            // Lossless regex capture returning a native `String?` (`{ String, has }`);
            // present holding the captured text (which may be empty) on a match,
            // absent when the pattern does not match. Mirrors vyb_env_get_opt.
            if (node->arguments.size() != 2) {
                logError(node->loc, fname + " expects 2 arguments (pattern, s)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* pat = m_currentLLVMValue; if (!pat) return;
            node->arguments[1]->accept(*this);
            llvm::Value* str = m_currentLLVMValue; if (!str) return;
            llvm::Value* patPtr = pat;
            if (patPtr->getType()->isStructTy())
                patPtr = builder->CreateExtractValue(patPtr, 0, "capopt.pat");
            llvm::Value* patLen = pat;
            if (pat->getType()->isStructTy())
                patLen = builder->CreateExtractValue(pat, 1, "capopt.patlen");
            else
                patLen = llvm::ConstantInt::get(int64Type, 0);
            llvm::Value* strPtr = str;
            if (strPtr->getType()->isStructTy())
                strPtr = builder->CreateExtractValue(strPtr, 0, "capopt.str");
            llvm::Value* strLen = str;
            if (str->getType()->isStructTy())
                strLen = builder->CreateExtractValue(str, 1, "capopt.strlen");
            else
                strLen = llvm::ConstantInt::get(int64Type, 0);
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "capopt.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "capopt.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int8PtrType, int64Type, int8PtrType, int64Type, llvm::PointerType::get(*context, 0)}, false);
            std::string rtN = (fname == "vyb_regex_capture_match_opt") ? "__vyb_regex_capture_match_opt" : "__vyb_regex_capture_opt";
            llvm::Function* f = module->getFunction(rtN);
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, rtN, module.get());
            llvm::Value* has = builder->CreateCall(f, {patPtr, patLen, strPtr, strLen, slot}, "capopt.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "capopt.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "capopt.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "capopt.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_exec_output_opt") {
            // Lossless stdout capture returning a native `String?` (`{ String, has }`);
            // absent only when the command could not be launched. Mirrors the
            // env_get_opt / regex capture_opt handlers.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_exec_output_opt expects 1 argument (cmd)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* cmd = m_currentLLVMValue;
            if (!cmd) return;
            llvm::Value* cmdPtr = cmd;
            if (cmdPtr->getType()->isStructTy())
                cmdPtr = builder->CreateExtractValue(cmdPtr, 0, "execopt.cmd");
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "execopt.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "execopt.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int8PtrType, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_exec_output_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_exec_output_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {cmdPtr, slot}, "execopt.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "execopt.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "execopt.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "execopt.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_io_read_all_opt") {
            // Lossless whole-file read returning a native `String?` (`{ String,
            // has }`); absent only when the read failed. Same shape as the
            // env_get_opt / exec_output_opt handlers, but the argument is an fd.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_io_read_all_opt expects 1 argument (fd)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* fd = m_currentLLVMValue;
            if (!fd) return;
            llvm::Value* fdi64 = fd;
            if (fdi64->getType()->isIntegerTy(64)) {
                // already i64
            } else if (fdi64->getType()->isIntegerTy()) {
                fdi64 = builder->CreateSExt(fdi64, int64Type, "readall.toi64");
            }
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "readall.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "readall.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_io_read_all_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_io_read_all_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {fdi64, slot}, "readall.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "readall.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "readall.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "readall.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_io_read_at") {
            // Bounded/offset read returning a native `String?` -- absent on failure
            // (bad fd / negative offset), present (possibly short) holding up to
            // `maxlen` bytes at absolute byte `off`. Mirrors vyb_io_read_all_opt /
            // vyb_net_recv_opt, with (fd, off, maxlen) integer arguments.
            if (node->arguments.size() != 3) {
                logError(node->loc, "vyb_io_read_at expects 3 arguments (fd, off, maxlen)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* off = m_currentLLVMValue;
            node->arguments[2]->accept(*this); llvm::Value* maxlen = m_currentLLVMValue;
            if (!fd || !off || !maxlen) return;
            auto toReadAtI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (v->getType()->isIntegerTy(64)) return v;
                return builder->CreateSExt(v, int64Type, "readat.toi64");
            };
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "readat.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "readat.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, int64Type, int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_file_read_at");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_file_read_at", module.get());
            llvm::Value* has = builder->CreateCall(f, {toReadAtI64(fd), toReadAtI64(off), toReadAtI64(maxlen), slot}, "readat.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "readat.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "readat.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "readat.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_io_read_bytes_opt") {
            // Whole-file BINARY read returning a native `Vec<UInt8>?`
            // (`{ Vec<UInt8>, has }`); absent only when the read failed. The
            // byte-buffer counterpart of read_all (String? carries text, not raw
            // bytes) (#213).
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_io_read_bytes_opt expects 1 argument (fd)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* fd = m_currentLLVMValue;
            if (!fd) return;
            llvm::Value* fdi64 = fd;
            if (!fdi64->getType()->isIntegerTy(64)) fdi64 = builder->CreateSExt(fdi64, int64Type, "readbytes.toi64");
            // Vec<UInt8> = { ptr, size, cap }
            llvm::StructType* vecByteTy = llvm::StructType::get(*context, {int8PtrType, int64Type, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {vecByteTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(vecByteTy, nullptr, "readbytes.slot");
            builder->CreateStore(llvm::Constant::getNullValue(vecByteTy), slot, "readbytes.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_io_read_bytes_all");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_io_read_bytes_all", module.get());
            llvm::Value* has = builder->CreateCall(f, {fdi64, slot}, "readbytes.has");
            llvm::Value* val = builder->CreateLoad(vecByteTy, slot, "readbytes.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "readbytes.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "readbytes.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_io_read_bytes_at_opt") {
            // Bounded/offset BINARY read returning a native `Vec<UInt8>?` --
            // the byte-buffer counterpart of read_at (#213). Absent on failure
            // (bad fd / negative offset), present (possibly short) holding up to
            // `maxlen` raw bytes at absolute byte `off`.
            if (node->arguments.size() != 3) {
                logError(node->loc, "vyb_io_read_bytes_at_opt expects 3 arguments (fd, off, maxlen)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* off = m_currentLLVMValue;
            node->arguments[2]->accept(*this); llvm::Value* maxlen = m_currentLLVMValue;
            if (!fd || !off || !maxlen) return;
            auto toRBI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (v->getType()->isIntegerTy(64)) return v;
                return builder->CreateSExt(v, int64Type, "readbytes.toi64");
            };
            llvm::StructType* vecByteTy = llvm::StructType::get(*context, {int8PtrType, int64Type, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {vecByteTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(vecByteTy, nullptr, "readbytes.slot");
            builder->CreateStore(llvm::Constant::getNullValue(vecByteTy), slot, "readbytes.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, int64Type, int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_io_read_bytes_at");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_io_read_bytes_at", module.get());
            llvm::Value* has = builder->CreateCall(f, {toRBI64(fd), toRBI64(off), toRBI64(maxlen), slot}, "readbytes.has");
            llvm::Value* val = builder->CreateLoad(vecByteTy, slot, "readbytes.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "readbytes.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "readbytes.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_net_recv_opt") {
            // Lossless stream read over a socket returning a native `String?`.
            // Mirrors vyb_io_read_all_opt, with (fd, maxlen) integer arguments.
            if (node->arguments.size() != 2) {
                logError(node->loc, "vyb_net_recv_opt expects 2 arguments (fd, maxlen)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* maxlen = m_currentLLVMValue;
            if (!fd || !maxlen) return;
            auto toRecvI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v->getType()->isIntegerTy(64)) return builder->CreateSExt(v, int64Type, "recv.toi64");
                return v;
            };
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "recv.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "recv.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_net_recv_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_net_recv_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {toRecvI64(fd), toRecvI64(maxlen), slot}, "recv.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "recv.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "recv.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "recv.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_net_recvfrom_opt") {
            // Lossless datagram receive returning a native `String?`.
            // Mirrors vyb_net_recv_opt over the recvfrom code path.
            if (node->arguments.size() != 2) {
                logError(node->loc, "vyb_net_recvfrom_opt expects 2 arguments (fd, maxlen)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* fd = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* maxlen = m_currentLLVMValue;
            if (!fd || !maxlen) return;
            auto udpToI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v->getType()->isIntegerTy(64)) return builder->CreateSExt(v, int64Type, "udp.toi64");
                return v;
            };
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "udp.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "udp.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_net_recvfrom_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_net_recvfrom_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {udpToI64(fd), udpToI64(maxlen), slot}, "udp.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "udp.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "udp.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "udp.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_thread_join_opt") {
            // Lossless thread join returning a native `Int?` ({ Int, has });
            // present holding the joined result (which may legitimately be -2),
            // absent when the handle is unknown/already-joined/detached.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_thread_join_opt expects 1 argument (handle)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* th = m_currentLLVMValue;
            if (!th) return;
            llvm::Value* th64 = th;
            if (th64->getType()->isIntegerTy() && !th64->getType()->isIntegerTy(64))
                th64 = builder->CreateSExt(th64, int64Type, "tjoin.toi64");
            llvm::StructType* optTy = llvm::StructType::get(*context, {int64Type, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(int64Type, nullptr, "tjoin.slot");
            builder->CreateStore(llvm::Constant::getNullValue(int64Type), slot, "tjoin.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_thread_join_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_thread_join_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {th64, slot}, "tjoin.has");
            llvm::Value* val = builder->CreateLoad(int64Type, slot, "tjoin.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "tjoin.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "tjoin.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_net_resolve_opt") {
            // Lossless hostname->IPv4 resolution returning a native `String?`.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_net_resolve_opt expects 1 argument (host)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* host = m_currentLLVMValue;
            if (!host) return;
            llvm::Value* hostPtr = host;
            if (hostPtr->getType()->isStructTy())
                hostPtr = builder->CreateExtractValue(hostPtr, 0, "resolve.host");
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "resolve.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "resolve.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int8PtrType, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_net_resolve_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_net_resolve_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {hostPtr, slot}, "resolve.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "resolve.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "resolve.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "resolve.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_tls_read_opt") {
            // Lossless TLS read returning a native `String?` ({ String, has });
            // absent on an error/EOF read, present holding the decrypted bytes.
            // Mirrors vyb_net_recv_opt: (sslp, maxlen) integer arguments, and
            // __vyb_tls_read_opt deposits the bytes into a caller buffer.
            if (node->arguments.size() != 2) {
                logError(node->loc, "vyb_tls_read_opt expects 2 arguments (sslp, maxlen)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* sslp = m_currentLLVMValue;
            node->arguments[1]->accept(*this); llvm::Value* maxlen = m_currentLLVMValue;
            if (!sslp || !maxlen) return;
            auto tlsToI64 = [&](llvm::Value* v) -> llvm::Value* {
                if (!v->getType()->isIntegerTy(64)) return builder->CreateSExt(v, int64Type, "tls.toi64");
                return v;
            };
            llvm::StructType* strTy = llvm::StructType::get(*context, {int8PtrType, int64Type}, false);
            llvm::StructType* optTy = llvm::StructType::get(*context, {strTy, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(strTy, nullptr, "tls.slot");
            builder->CreateStore(llvm::Constant::getNullValue(strTy), slot, "tls.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_tls_read_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_tls_read_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {tlsToI64(sslp), tlsToI64(maxlen), slot}, "tls.has");
            llvm::Value* val = builder->CreateLoad(strTy, slot, "tls.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "tls.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "tls.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_strchan_recv") {
            // Blocking recv returning a String { ptr, len } that the caller owns.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_strchan_recv expects 1 argument (ch)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* ch = m_currentLLVMValue;
            if (!ch) return;
            llvm::Value* ch64 = ch;
            if (ch64->getType()->isIntegerTy() && !ch64->getType()->isIntegerTy(64))
                ch64 = builder->CreateSExt(ch64, int64Type, "strchan.toi64");
            std::vector<llvm::Type*> strFields = {int8PtrType, int64Type};
            llvm::StructType* strStructType = llvm::StructType::get(*context, strFields, false);
            llvm::FunctionType* ft = llvm::FunctionType::get(strStructType, {int64Type}, false);
            llvm::Function* f = module->getFunction("__vyb_strchan_recv");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_strchan_recv", module.get());
            m_currentLLVMValue = builder->CreateCall(f, {ch64}, "strchan.recved");
            return;
        } else if (fname == "vyb_strchan_try") {
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_strchan_try expects 1 argument (ch)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this); llvm::Value* ch = m_currentLLVMValue;
            if (!ch) return;
            llvm::Value* ch64 = ch;
            if (ch64->getType()->isIntegerTy() && !ch64->getType()->isIntegerTy(64))
                ch64 = builder->CreateSExt(ch64, int64Type, "strchan.toi64");
            std::vector<llvm::Type*> strFields = {int8PtrType, int64Type};
            llvm::StructType* strStructType = llvm::StructType::get(*context, strFields, false);
            llvm::FunctionType* ft = llvm::FunctionType::get(strStructType, {int64Type}, false);
            llvm::Function* f = module->getFunction("__vyb_strchan_try");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_strchan_try", module.get());
            m_currentLLVMValue = builder->CreateCall(f, {ch64}, "strchan.tryed");
            return;
        } else if (fname == "vyb_strchan_len") {
            emitHandleIntrinsic("__vyb_strchan_len", 1);
            return;
        } else if (fname == "vyb_strchan_close") {
            emitHandleIntrinsic("__vyb_strchan_close", 1);  // mark closed, wake waiters
            return;
        } else if (fname == "vyb_strchan_free") {
            emitHandleIntrinsic("__vyb_strchan_free", 1);
            return;
        } else if (fname == "vyb_task_await") {
            emitHandleIntrinsic("__vyb_task_await", 1);  // blocking recv
            return;
        } else if (fname == "vyb_task_poll") {
            emitHandleIntrinsic("__vyb_task_poll", 1);   // non-blocking try
            return;
        } else if (fname == "vyb_task_poll_opt") {
            // Lossless non-blocking task poll -> native `Int?` ({ value, has });
            // absent while the task is still running, present holding the result
            // (which may legitimately be -1). Mirrors vyb_chan_recv_opt.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_task_poll_opt expects 1 argument (task)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* tk = m_currentLLVMValue;
            if (!tk) return;
            if (tk->getType()->isIntegerTy() && !tk->getType()->isIntegerTy(64))
                tk = builder->CreateSExt(tk, int64Type, "taskopt.toi64");
            llvm::StructType* optTy = llvm::StructType::get(*context,
                {int64Type, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(int64Type, nullptr, "taskopt.slot");
            builder->CreateStore(llvm::Constant::getNullValue(int64Type), slot, "taskopt.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_task_poll_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_task_poll_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {tk, slot}, "taskopt.has");
            llvm::Value* val = builder->CreateLoad(int64Type, slot, "taskopt.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "taskopt.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "taskopt.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_task_free") {
            emitHandleIntrinsic("__vyb_task_free", 1);
            return;
        } else if (fname == "vyb_async_run_all") {
            emitHandleIntrinsic("__vyb_async_run_all", 0);
            return;
        } else if (fname == "vyb_async_await") {
            emitHandleIntrinsic("__vyb_async_await", 1);
            return;
        } else if (fname == "vyb_async_poll") {
            emitHandleIntrinsic("__vyb_async_poll", 1);
            return;
        } else if (fname == "vyb_async_poll_opt") {
            // Lossless non-blocking async poll -> native `Int?` ({ value, has });
            // absent while the fiber is still running, present holding the result
            // (which may legitimately be -1). Mirrors vyb_chan_recv_opt.
            if (node->arguments.size() != 1) {
                logError(node->loc, "vyb_async_poll_opt expects 1 argument (task)");
                m_currentLLVMValue = nullptr; return;
            }
            node->arguments[0]->accept(*this);
            llvm::Value* tk = m_currentLLVMValue;
            if (!tk) return;
            if (tk->getType()->isIntegerTy() && !tk->getType()->isIntegerTy(64))
                tk = builder->CreateSExt(tk, int64Type, "asyncopt.toi64");
            llvm::StructType* optTy = llvm::StructType::get(*context,
                {int64Type, llvm::Type::getInt1Ty(*context)}, false);
            llvm::Value* slot = builder->CreateAlloca(int64Type, nullptr, "asyncopt.slot");
            builder->CreateStore(llvm::Constant::getNullValue(int64Type), slot, "asyncopt.zero");
            llvm::FunctionType* ft = llvm::FunctionType::get(
                int64Type, {int64Type, llvm::PointerType::get(*context, 0)}, false);
            llvm::Function* f = module->getFunction("__vyb_async_poll_opt");
            if (!f) f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_async_poll_opt", module.get());
            llvm::Value* has = builder->CreateCall(f, {tk, slot}, "asyncopt.has");
            llvm::Value* val = builder->CreateLoad(int64Type, slot, "asyncopt.val");
            llvm::Value* opt = llvm::UndefValue::get(optTy);
            opt = builder->CreateInsertValue(opt, val, 0, "asyncopt.v");
            opt = builder->CreateInsertValue(opt, builder->CreateTrunc(has, llvm::Type::getInt1Ty(*context), "asyncopt.h"), 1);
            m_currentLLVMValue = opt;
            return;
        } else if (fname == "vyb_async_detach") {
            emitHandleIntrinsic("__vyb_async_detach", 1);
            return;
        } else if (fname == "vyb_async_yield") {
            emitHandleIntrinsic("__vyb_async_yield", 0);
            return;
        } else if (fname == "vyb_async_sleep_ms") {
            emitHandleIntrinsic("__vyb_async_sleep_ms", 1);
            return;
        }
    }

    // Handle serialization mode intrinsics: lit(), notype(), bare(), deserial()
    if (identCallee && identCallee->name == "lit" && node->arguments.size() >= 1) {
        // lit() intrinsic - convert value(s) to their raw string/JSON literal representation
        // Single arg: returns the literal string
        // Multiple args: returns a JSON array string like [arg1, arg2, ...]
        VYB_CDBG << "DEBUG: Processing lit() intrinsic with " << node->arguments.size() << " args" << std::endl;

        auto serializeLitArg = [&](ast::ExprPtr& argExpr) -> llvm::Value* {
            argExpr->accept(*this);
            llvm::Value* arg = m_currentLLVMValue;
            if (!arg) return nullptr;
            llvm::Type* argType = arg->getType();

            // If the argument is a Vyb string struct { ptr, i64 }, extract the ptr field
            if (argType->isStructTy() && argType->getStructNumElements() == 2 &&
                argType->getStructElementType(1)->isIntegerTy(64)) {
                arg = builder->CreateExtractValue(arg, 0, "lit.strptr");
                argType = arg->getType();
            }

            // If the argument is a non-pointer scalar (Int, Float, Bool), convert to string first
            if (argType->isIntegerTy() && !argType->isIntegerTy(8)) {
                // Boolean: output "true" or "false"
                if (argType->isIntegerTy(1)) {
                    llvm::Value* trueStr = builder->CreateGlobalStringPtr("true", "lit.bool.true");
                    llvm::Value* falseStr = builder->CreateGlobalStringPtr("false", "lit.bool.false");
                    return builder->CreateSelect(arg, trueStr, falseStr, "lit.bool_result");
                }
                std::string toStringFuncName = "__vyb_int_to_string";
                llvm::FunctionType* toStringFuncType = llvm::FunctionType::get(int8PtrType, {int64Type}, false);
                llvm::Function* toStringFunc = module->getFunction(toStringFuncName);
                if (!toStringFunc) {
                    toStringFunc = llvm::Function::Create(toStringFuncType, llvm::Function::ExternalLinkage, toStringFuncName, module.get());
                }
                if (argType != int64Type) {
                    arg = builder->CreateSExt(arg, int64Type, "lit.int64");
                }
                return builder->CreateCall(toStringFunc, {arg}, "lit_result");
            } else if (argType->isFloatingPointTy()) {
                std::string toStringFuncName = "__vyb_float_to_string";
                llvm::FunctionType* toStringFuncType = llvm::FunctionType::get(int8PtrType, {doubleType}, false);
                llvm::Function* toStringFunc = module->getFunction(toStringFuncName);
                if (!toStringFunc) {
                    toStringFunc = llvm::Function::Create(toStringFuncType, llvm::Function::ExternalLinkage, toStringFuncName, module.get());
                }
                if (argType != doubleType) {
                    arg = builder->CreateFPExt(arg, doubleType, "lit.double");
                }
                return builder->CreateCall(toStringFunc, {arg}, "lit_result");
            }

            // For pointer types (strings), call the lit conversion function
            llvm::Function* litFunc = getLitConversionFunction();
            if (!litFunc) {
                logError(argExpr->loc, "Failed to get lit conversion function");
                return nullptr;
            }
            return builder->CreateCall(litFunc, {arg}, "lit_result");
        };

        if (node->arguments.size() == 1) {
            // Single argument: return the literal string directly
            llvm::Value* result = serializeLitArg(node->arguments[0]);
            if (!result) return;
            m_currentLLVMValue = result;
            return;
        } else {
            // Multiple arguments: produce a JSON array string like [arg1, arg2, ...]
            // Helper to get or declare __vyb_string_concat(char*, char*) -> char*
            auto getConcatFn = [&]() -> llvm::Function* {
                llvm::Function* f = module->getFunction("__vyb_string_concat");
                if (!f) {
                    llvm::FunctionType* ft = llvm::FunctionType::get(int8PtrType, {int8PtrType, int8PtrType}, false);
                    f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__vyb_string_concat", module.get());
                }
                return f;
            };
            llvm::Function* concatFn = getConcatFn();

            // Start with "["
            llvm::Value* jsonStr = builder->CreateGlobalStringPtr("[", "lit.arr.open");

            for (size_t i = 0; i < node->arguments.size(); ++i) {
                llvm::Value* serialized = serializeLitArg(node->arguments[i]);
                if (!serialized) return;

                // Add comma separator before all but first element
                if (i > 0) {
                    llvm::Value* sep = builder->CreateGlobalStringPtr(", ", "lit.arr.sep");
                    jsonStr = builder->CreateCall(concatFn, {jsonStr, sep}, "lit.arr.sep.call");
                }

                // Append the serialized argument
                jsonStr = builder->CreateCall(concatFn, {jsonStr, serialized}, "lit.arr.elem");
            }

            // Close with "]"
            llvm::Value* closeBracket = builder->CreateGlobalStringPtr("]", "lit.arr.close");
            jsonStr = builder->CreateCall(concatFn, {jsonStr, closeBracket}, "lit.arr.close.call");

            m_currentLLVMValue = jsonStr;
            return;
        }
    }
    if (identCallee && node->arguments.size() == 1) {
        if (identCallee->name == "lit") {
            // lit() intrinsic - convert value to its raw string/JSON literal representation
            VYB_CDBG << "DEBUG: Processing lit() intrinsic" << std::endl;
            node->arguments[0]->accept(*this);
            llvm::Value* arg = m_currentLLVMValue;
            if (!arg) {
                logError(node->arguments[0]->loc, "Argument to lit() evaluated to null");
                return;
            }

            llvm::Type* argType = arg->getType();

            // If the argument is a Vyb string struct { ptr, i64 }, extract the ptr field
            if (argType->isStructTy() && argType->getStructNumElements() == 2 &&
                argType->getStructElementType(1)->isIntegerTy(64)) {
                arg = builder->CreateExtractValue(arg, 0, "lit.strptr");
                argType = arg->getType();
            }

            // If the argument is a non-pointer scalar (Int, Float, Bool), convert to string first
            if (argType->isIntegerTy() && !argType->isIntegerTy(8)) {
                // Boolean: output "true" or "false"
                if (argType->isIntegerTy(1)) {
                    llvm::Value* trueStr = builder->CreateGlobalStringPtr("true", "lit.bool.true");
                    llvm::Value* falseStr = builder->CreateGlobalStringPtr("false", "lit.bool.false");
                    m_currentLLVMValue = builder->CreateSelect(arg, trueStr, falseStr, "lit.bool_result");
                    return;
                }
                // Integer: use __vyb_int_to_string (always cast to i64)
                std::string toStringFuncName = "__vyb_int_to_string";
                llvm::FunctionType* toStringFuncType = llvm::FunctionType::get(
                    int8PtrType, {int64Type}, false);
                llvm::Function* toStringFunc = module->getFunction(toStringFuncName);
                if (!toStringFunc) {
                    toStringFunc = llvm::Function::Create(toStringFuncType,
                        llvm::Function::ExternalLinkage, toStringFuncName, module.get());
                }
                // Cast to i64 if needed
                if (argType != int64Type) {
                    // Use ZExt for i1 (bool), SExt for other integers
                    if (argType->isIntegerTy(1)) {
                        arg = builder->CreateZExt(arg, int64Type, "lit.int64");
                    } else {
                        arg = builder->CreateSExt(arg, int64Type, "lit.int64");
                    }
                }
                m_currentLLVMValue = builder->CreateCall(toStringFunc, {arg}, "lit_result");
                VYB_CDBG << "DEBUG: Created call to lit conversion function" << std::endl;
                return;
            } else if (argType->isFloatingPointTy()) {
                // Float: use __vyb_float_to_string
                std::string toStringFuncName = "__vyb_float_to_string";
                llvm::FunctionType* toStringFuncType = llvm::FunctionType::get(
                    int8PtrType, {doubleType}, false);
                llvm::Function* toStringFunc = module->getFunction(toStringFuncName);
                if (!toStringFunc) {
                    toStringFunc = llvm::Function::Create(toStringFuncType,
                        llvm::Function::ExternalLinkage, toStringFuncName, module.get());
                }
                if (argType != doubleType) {
                    arg = builder->CreateFPExt(arg, doubleType, "lit.double");
                }
                m_currentLLVMValue = builder->CreateCall(toStringFunc, {arg}, "lit_result");
                VYB_CDBG << "DEBUG: Created call to lit conversion function" << std::endl;
                return;
            }

            // For pointer types (strings), call the lit conversion function
            VYB_CDBG << "DEBUG: Getting lit conversion function..." << std::endl;
            llvm::Function* litFunc = getLitConversionFunction();
            if (!litFunc) {
                VYB_CDBG << "DEBUG: Failed to get lit conversion function!" << std::endl;
                logError(node->loc, "Failed to get lit conversion function");
                return;
            }
            VYB_CDBG << "DEBUG: Got lit conversion function: " << litFunc->getName().str() << std::endl;
            std::vector<llvm::Value*> args = {arg};
            m_currentLLVMValue = builder->CreateCall(litFunc, args, "lit_result");
            VYB_CDBG << "DEBUG: Created call to lit conversion function" << std::endl;
            return;
        }
        else if (identCallee->name == "notype" || identCallee->name == "bare") {
            // notype() and bare() intrinsics - forward the inner value
            node->arguments[0]->accept(*this);
            // m_currentLLVMValue is already set to the inner expression's result
            if (identCallee->name == "bare" && m_currentLLVMValue) {
                // #383: `bare(x)` hands the value on untouched, so the request for raw
                // field values has to travel with the value itself -- record it here
                // and let the serialization site (cgen_string.cpp) see it. This is the
                // only place that knows the expression was wrapped in bare().
                bareSerializationValues.push_back(m_currentLLVMValue);
            }
            return;
        }
        else if (identCallee->name == "deserial") {
            // deserial() intrinsic - forward the inner value for now
            node->arguments[0]->accept(*this);
            // m_currentLLVMValue is already set to the inner expression's result
            return;
        }
    }

    // Check for method calls before trying function lookup
    if (auto memberExpr = dynamic_cast<vyb::ast::MemberExpression*>(node->callee.get())) {
        if (auto methodIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->property.get())) {
            std::string methodName = methodIdent->name;

            // Handle to_string method calls
            if (methodName == "to_string") {
                VYB_CDBG << "DEBUG: Processing to_string method call" << std::endl;

                // Evaluate the object (the thing we're calling to_string on)
                memberExpr->object->accept(*this);
                llvm::Value* objectValue = m_currentLLVMValue;
                if (!objectValue) {
                    logError(memberExpr->object->loc, "Failed to evaluate object for to_string method call");
                    m_currentLLVMValue = nullptr;
                    return;
                }

                // Get the type of the object
                llvm::Type* objectType = objectValue->getType();
                vyb::ast::TypeNode* objectASTType = nullptr;

                // Try to get AST type information for better type resolution
                auto valueTypeIter = valueTypeMap.find(objectValue);
                if (valueTypeIter != valueTypeMap.end()) {
                    objectASTType = valueTypeIter->second.get();
                }

                // Generate the to_string call
                llvm::Value* result = generateToStringCall(objectValue, objectType, objectASTType, node->loc);
                if (result) {
                    // #216: enforce a canonical String result shape at expression
                    // boundaries. `generateToStringCall` returns a raw char* for
                    // scalar/String receivers, but a `.to_string()` expression must be
                    // a full Vyb String struct {ptr, i64} so operand-position uses
                    // (String ==/!=, concat, C-string params, println) all get the
                    // same shape. Otherwise `x.to_string() == "22"` lowers to an
                    // icmp of a ptr against a {ptr,i64} literal and module
                    // verification aborts. (The JSON/enum paths already return the
                    // struct; only a raw char* result needs wrapping.) Length is
                    // computed with strlen exactly as the char*->String conversion
                    // in the fromString/assignment paths does.
                    if (result->getType()->isPointerTy() && result->getType() == int8PtrType) {
                        llvm::StructType* strTy = llvm::StructType::get(*context, {
                            int8PtrType, llvm::Type::getInt64Ty(*context)});
                        llvm::FunctionType* strlenType = llvm::FunctionType::get(
                            llvm::Type::getInt64Ty(*context), {int8PtrType}, false);
                        llvm::Function* strlenFunc = module->getFunction("strlen");
                        if (!strlenFunc) {
                            strlenFunc = llvm::Function::Create(strlenType,
                                llvm::Function::ExternalLinkage, "strlen", module.get());
                        }
                        llvm::Value* len = builder->CreateCall(strlenFunc, {result}, "tostring.len");
                        llvm::Value* s = llvm::UndefValue::get(strTy);
                        s = builder->CreateInsertValue(s, result, 0, "tostring.data");
                        s = builder->CreateInsertValue(s, len, 1, "tostring.len");
                        result = s;
                    }
                    VYB_CDBG << "DEBUG: Successfully generated to_string call" << std::endl;
                    m_currentLLVMValue = result;
                    return;
                } else {
                    logError(node->loc, "Failed to generate to_string call for type");
                    m_currentLLVMValue = nullptr;
                    return;
                }
            }

            // Handle Vec, String, and Tuple method calls
            if (auto objIdent = dynamic_cast<vyb::ast::Identifier*>(memberExpr->object.get())) {
                std::string objectName = objIdent->name;

                // Check if this is a Tuple, String, or Vec variable by looking at AST type info first
                auto varIt = namedValues.find(objectName);
                bool isTupleVar = false;
                bool isStringVar = false;
                bool isVecVar = false;
                bool isByRefVec = false;
                bool isChanVar = false;
                bool chanIsString = false;
                const vyb::ast::TypeNode* chanElem = nullptr;
                unsigned tupleSize = 0;

                if (varIt != namedValues.end()) {
                    // First check valueTypeMap for AST type information (most reliable)
                    auto typeIt = valueTypeMap.find(varIt->second);
                    if (typeIt != valueTypeMap.end() && typeIt->second) {
                        vyb::ast::TypeNode* astType = typeIt->second.get();
                        // Check for TupleTypeNode
                        if (auto tupleType = dynamic_cast<vyb::ast::TupleTypeNode*>(astType)) {
                            isTupleVar = true;
                            tupleSize = tupleType->memberTypes.size();
                        }
                        // Check for VecType
                        else if (dynamic_cast<vyb::ast::VecType*>(astType)) {
                            isVecVar = true;
                        }
                        // Check for String (represented as TypeName with identifier "String")
                        else if (auto typeName = dynamic_cast<vyb::ast::TypeName*>(astType)) {
                            if (typeName->identifier && typeName->identifier->name == "String") {
                                isStringVar = true;
                            } else if (typeName->identifier &&
                                       typeName->identifier->name == "chan" &&
                                       typeName->genericArgs.size() == 1) {
                                // Built-in channel receiver `chan<T>`: a single i64
                                // handle; the element type picks the runtime path.
                                isChanVar = true;
                                chanElem = typeName->genericArgs[0].get();
                                chanIsString = chanElementIsString(chanElem);
                            } else if (typeName->identifier &&
                                       (typeName->identifier->name == "their" ||
                                        typeName->identifier->name == "my" ||
                                        typeName->identifier->name == "our" ||
                                        typeName->identifier->name == "view" ||
                                        typeName->identifier->name == "borrow") &&
                                       typeName->genericArgs.size() == 1) {
                                // Ownership-wrapped Vec receiver: the slot stores a
                                // `Vec*` (mutable/by-ref borrow), deref'd at the call.
                                ast::TypeNode* inner = typeName->genericArgs[0].get();
                                isByRefVec = dynamic_cast<vyb::ast::VecType*>(inner) != nullptr;
                                if (!isByRefVec) {
                                    if (auto innerTN = dynamic_cast<ast::TypeName*>(inner)) {
                                        isByRefVec = innerTN->identifier && innerTN->identifier->name == "Vec";
                                    }
                                }
                                if (isByRefVec) isVecVar = true;
                            }
                        }
                    }

                    // Fall back to LLVM type analysis if no AST type info available
                    if (!isTupleVar && !isStringVar && !isVecVar) {
                        if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(varIt->second)) {
                            llvm::Type* allocatedType = allocaInst->getAllocatedType();
                            if (allocatedType->isStructTy()) {
                                llvm::StructType* structType = llvm::cast<llvm::StructType>(allocatedType);
                                unsigned numElements = structType->getNumElements();
                                // String struct has 2 fields: { ptr, len }
                                // Vec struct has 3 fields: { ptr, size, capacity }
                                // This is a heuristic fallback only
                                if (numElements == 2) {
                                    isStringVar = true;
                                } else if (numElements == 3) {
                                    isVecVar = true;
                                } else {
                                    // Assume tuple for other struct sizes
                                    isTupleVar = true;
                                    tupleSize = numElements;
                                }
                            }
                        }
                    }
                }

                // Handle Tuple methods first (highest priority)
                if (isChanVar) {
                    llvm::Value* handle = builder->CreateLoad(
                        llvm::Type::getInt64Ty(*context), varIt->second, "chan.load");
                    emitChannelMethod(node, handle, methodName, chanIsString, chanElem);
                    return;
                }

                if (isTupleVar && methodName == "len") {
                    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), tupleSize);
                    return;
                }

                // Handle String methods
                if (isStringVar) {
                    if (methodName == "len" || methodName == "length" || methodName == "concat" || methodName == "substring" ||
                        methodName == "substr" || methodName == "char_at" || methodName == "to_bytes" ||
                        methodName == "from_bytes" || methodName == "starts_with" || methodName == "ends_with" ||
                        methodName == "contains" || methodName == "to_upper" || methodName == "to_lower" ||
                        methodName == "trim" || methodName == "strip" || methodName == "replace" ||
                        methodName == "format") {
                        handleStringMethod(node, objectName, methodName);
                        return;
                    }
                }

                // Handle Vec methods
                if (isVecVar || (!isTupleVar && !isStringVar && (methodName == "push" || methodName == "pop" || methodName == "get" ||
                    methodName == "push_array" || methodName == "to_array" || methodName == "get_array" ||
                    methodName == "clear" || methodName == "is_empty" || methodName == "capacity" ||
                    methodName == "remove_at" || methodName == "get_vec"))) {
                    // These methods are Vec-specific or we couldn't determine type
                    if (methodName == "push" || methodName == "pop" || methodName == "len" || methodName == "get" || methodName == "set" ||
                        methodName == "push_array" || methodName == "to_array" || methodName == "get_array" ||
                        methodName == "clear" || methodName == "is_empty" || methodName == "capacity" ||
                        methodName == "concat" || methodName == "contains" || methodName == "remove_at" ||
                        methodName == "last" || methodName == "peek" ||
                        methodName == "get_vec") {
                        // By-ref receiver (their<Vec<T>> / my<Vec<T>> / ...): the
                        // named slot holds a `Vec*` (a mutable borrow), so load that
                        // pointer and operate on it directly, rather than treating the
                        // slot itself as the Vec struct.
                        if (isByRefVec && varIt != namedValues.end()) {
                            llvm::Value* slot = varIt->second;
                            llvm::Value* vecPtr = builder->CreateLoad(
                                llvm::PointerType::get(*context, 0), slot, "byref.vec.load");
                            handleVecMethodOnValue(node, vecPtr, methodName, memberExpr->object.get());
                            return;
                        }
                        handleVecMethod(node, objectName, methodName);
                        return;
                    }
                }
            }

            // String method on a non-identifier receiver: a string literal, a
            // function/expression result, or a struct/array field whose type is
            // String (e.g. "hello {}!".format(...), get().substring(...),
            // p.name.to_upper()). Runs before the Vec-member fallback below so a
            // String method like `contains`/`concat`/`len` isn't misrouted to the
            // Vec path. Evaluate the receiver once, materialize a pointer to its
            // { ptr, len } struct, then dispatch to either the built-in String
            // handlers or a String-bound aspect method (e.g. split).
            bool objectAstIsString = false;
            if (typeOfNode(memberExpr->object)) {
                if (auto objTn = dynamic_cast<ast::TypeName*>(typeOfNode(memberExpr->object).get())) {
                    objectAstIsString = objTn->identifier && objTn->identifier->name == "String";
                }
            }
            if (objectAstIsString) {
                memberExpr->object->accept(*this);
                llvm::Value* strVal = m_currentLLVMValue;
                if (!strVal) {
                    logError(memberExpr->object->loc, "Failed to evaluate String method receiver");
                    m_currentLLVMValue = nullptr;
                    return;
                }
                llvm::Value* strPtr = nullptr;
                llvm::Type* strStructTy = nullptr;
                if (strVal->getType()->isStructTy()) {
                    strStructTy = strVal->getType();
                }
                if (strVal->getType()->isStructTy()) {
                    llvm::AllocaInst* tmp = builder->CreateAlloca(strVal->getType(), nullptr, "str.rcv.tmp");
                    builder->CreateStore(strVal, tmp);
                    strPtr = tmp;
                } else if (strVal->getType()->isPointerTy()) {
                    strPtr = strVal;
                }
                if (!strPtr) {
                    logError(memberExpr->object->loc, "String method receiver is not a String struct");
                    m_currentLLVMValue = nullptr;
                    return;
                }

                if (methodName == "len" || methodName == "length" || methodName == "concat" ||
                    methodName == "substring" || methodName == "substr" || methodName == "char_at" ||
                    methodName == "to_bytes" || methodName == "starts_with" || methodName == "ends_with" ||
                    methodName == "contains" || methodName == "to_upper" || methodName == "to_lower" ||
                    methodName == "trim" || methodName == "strip" || methodName == "replace" ||
                    methodName == "format") {
                    handleStringMethodOnValue(node, strPtr, methodName);
                    // A computed String receiver (e.g. `header.substring(0,c).trim()`)
                    // owns a fresh heap buffer with no named binding; drop it here or
                    // it leaks for the life of the process.
                    if (exprProducesOwnedStringTemp(memberExpr->object.get())) {
                        llvm::Value* rcvData = strVal;
                        if (strVal->getType()->isStructTy()) {
                            rcvData = builder->CreateExtractValue(strVal, 0, "str.rcv.data");
                        }
                        builder->CreateCall(getOrCreateVybStringFreeFunction(), {rcvData});
                    }
                    return;
                }

                // Non-built-in: this may be a String-bound aspect method (e.g. split).
                SemanticAnalyzer* semantic = driver_.hasSemanticAnalyzer()
                    ? driver_.getSemanticAnalyzer() : nullptr;
                if (semantic) {
                    const std::string concreteType = "String";
                    const auto& impls = semantic->getTraitImpls();
                    auto limpl = impls.find(concreteType);
                    if (limpl != impls.end()) {
                        for (const auto& traitEntry : limpl->second) {
                            bool hasMethod = false;
                            for (const ast::FunctionDeclaration* m : traitEntry.second) {
                                if (m && m->id && m->id->name == methodName) { hasMethod = true; break; }
                            }
                            if (!hasMethod) continue;
                            const std::string& trait = traitEntry.first;
                            llvm::Function* implFunc = module->getFunction(concreteType + "_" + trait + "_" + methodName);
                            if (!implFunc) implFunc = monomorphizeTraitMethod(concreteType, trait, methodName);
                            if (!implFunc) continue;

                            bool selfIsByRef = implFunc->getArg(0)->getType()->isPointerTy();
                            std::vector<llvm::Value*> argValues;
                            if (selfIsByRef) {
                                argValues.push_back(strPtr);
                            } else if (strStructTy) {
                                argValues.push_back(builder->CreateLoad(strStructTy, strPtr, "str.rcv.load"));
                            } else {
                                argValues.push_back(strPtr);
                            }
                            for (size_t a = 0; a < node->arguments.size(); ++a) {
                                auto& arg = node->arguments[a];
                                arg->accept(*this);
                                if (!m_currentLLVMValue) {
                                    logError(arg->loc, "Argument codegen failed for aspect method " + methodName);
                                    m_currentLLVMValue = nullptr;
                                    return;
                                }
                                llvm::Value* argVal = m_currentLLVMValue;
                                if (argVal->getType()->isPointerTy()) {
                                    llvm::Type* expected = implFunc->getFunctionType()->getParamType(argValues.size());
                                    if (expected && expected->isStructTy()) {
                                        llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(expected);
                                        if (st && st->getNumElements() == 2 &&
                                            st->getElementType(0)->isPointerTy() &&
                                            st->getElementType(1)->isIntegerTy(64)) {
                                            llvm::Value* wrapped = tryCast(argVal, expected, arg->loc);
                                            if (wrapped) argVal = wrapped;
                                        }
                                    }
                                }
                                argValues.push_back(argVal);
                            }
                            // #255: an aspect method called with the wrong number of
                            // arguments must not reach the verifier -- an unmatched call
                            // fails LLVM verification and the compiler died on the
                            // invalid IR (SIGSEGV). Diagnose and halt instead. argValues
                            // includes the receiver as parameter 0.
                            if (argValues.size() != implFunc->getFunctionType()->getNumParams()) {
                                const size_t want = implFunc->getFunctionType()->getNumParams() - 1;
                                logError(node->loc, implFunc->getName().str() + " expects exactly " +
                                         std::to_string(want) + " argument" + (want == 1 ? "" : "s") + ", got " +
                                         std::to_string(argValues.size() - 1) + ".");
                                flagHardCodegenError();
                                m_currentLLVMValue = nullptr;
                                return;
                            }
                            if (implFunc->getReturnType()->isVoidTy()) {
                                builder->CreateCall(implFunc, argValues);
                                m_currentLLVMValue = nullptr;
                            } else {
                                m_currentLLVMValue = builder->CreateCall(implFunc, argValues, "aspect.method.result");
                            }
                            if (exprProducesOwnedStringTemp(memberExpr->object.get())) {
                                llvm::Value* rcvData = strVal;
                                if (strVal->getType()->isStructTy()) {
                                    rcvData = builder->CreateExtractValue(strVal, 0, "str.aspect.rcv.data");
                                }
                                builder->CreateCall(getOrCreateVybStringFreeFunction(), {rcvData});
                            }
                            return;
                        }
                    }
                }
            }

            // Built-in channel receiver reached through a non-identifier receiver
            // (a chan returned by a function, a chan<T> struct field, or a chained
            // member access). chan<T> is a single i64 handle, spatially identical to
            // Int, so evaluating the receiver yields the handle directly; dispatch
            // by element type through emitChannelMethod exactly like the identifier
            // path above.
            if (typeOfNode(memberExpr->object)) {
                if (auto objTn = dynamic_cast<ast::TypeName*>(typeOfNode(memberExpr->object).get())) {
                    if (objTn->identifier && objTn->identifier->name == "chan" &&
                        objTn->genericArgs.size() == 1) {
                        memberExpr->object->accept(*this);
                        llvm::Value* chanHandle = m_currentLLVMValue;
                        if (!chanHandle) {
                            logError(memberExpr->object->loc, "Failed to evaluate channel method receiver");
                            m_currentLLVMValue = nullptr;
                            return;
                        }
                        const vyb::ast::TypeNode* chanElem = objTn->genericArgs[0].get();
                        bool chanIsString = chanElementIsString(chanElem);
                        emitChannelMethod(node, chanHandle, methodName, chanIsString, chanElem);
                        return;
                    }
                }
            }

            // Handle Vec method calls on member expressions (e.g., tree.nodes.push())
            // The object is itself a member expression. The plain Vec-exclusive
            // builtins route by name; last()/peek() (NOT Vec-exclusive, may collide
            // with a user bind method named peek/last) are routed to the Vec handler
            // only when the member's object type is genuinely a Vec.
            ast::TypeNode* mcObjTy = typeOfNode(memberExpr->object) ? typeOfNode(memberExpr->object).get() : nullptr;
            bool mcObjIsVec = false;
            if (mcObjTy) {
                if (dynamic_cast<ast::VecType*>(mcObjTy)) mcObjIsVec = true;
                else if (auto mo = dynamic_cast<ast::TypeName*>(mcObjTy))
                    mcObjIsVec = mo->identifier && mo->identifier->name == "Vec";
            }
            if (methodName == "push" || methodName == "pop" || methodName == "len" || methodName == "get" || methodName == "set" ||
                methodName == "push_array" || methodName == "to_array" || methodName == "get_array" ||
                methodName == "clear" || methodName == "is_empty" || methodName == "capacity" ||
                methodName == "concat" || methodName == "contains" || methodName == "remove_at" ||
                methodName == "get_vec" ||
                ((methodName == "last" || methodName == "peek") && mcObjIsVec)) {
                // Evaluate the object in "LHS mode" (pointer mode) to get a pointer to the Vec field.
                // Without this, evaluating `s.items` loads a copy of the Vec struct and mutations
                // (like push) would be applied to a temporary, losing the changes.
                bool savedLHS = m_isLHSOfAssignment;
                m_isLHSOfAssignment = true;
                memberExpr->object->accept(*this);
                m_isLHSOfAssignment = savedLHS;
                llvm::Value* vecValue = m_currentLLVMValue;
                if (!vecValue) {
                    logError(memberExpr->object->loc, "Failed to evaluate object for Vec method call");
                    m_currentLLVMValue = nullptr;
                    return;
                }

                // A struct field typed as a by-ref Vec (`their<Vec<T>>` / `my<...>` /
                // `view<...>` / `our<...>` / `borrow<...>`) is laid out as a single
                // pointer slot holding the borrowed Vec's address. LHS-mode evaluation
                // of `obj.field` above yields the *slot* address (a `Vec**`); load once
                // to recover the `Vec*` before operating on it. A plain `Vec<T>` field
                // is emitted directly as the Vec struct, so this only applies when the
                // field's declared type is an ownership wrapper around a Vec.
                // #297: a receiver that is itself a call (a `get` borrow in flight,
                // e.g. `outer.get(0).get(0)`) is ALREADY that `Vec*`, so it is not
                // loaded again.
                if (!receiverIsBorrowValue(memberExpr->object.get())) {
                    if (auto objTn = dynamic_cast<ast::TypeName*>(typeOfNode(memberExpr->object).get())) {
                        if (objTn->identifier &&
                            (objTn->identifier->name == "their" || objTn->identifier->name == "my" ||
                             objTn->identifier->name == "our" || objTn->identifier->name == "view" ||
                             objTn->identifier->name == "borrow") &&
                            objTn->genericArgs.size() == 1) {
                            ast::TypeNode* inner = objTn->genericArgs[0].get();
                            bool innerIsVec = dynamic_cast<ast::VecType*>(inner) != nullptr;
                            if (!innerIsVec) {
                                if (auto innerTN = dynamic_cast<ast::TypeName*>(inner)) {
                                    innerIsVec = innerTN->identifier && innerTN->identifier->name == "Vec";
                                }
                            }
                            if (innerIsVec) {
                                llvm::Type* vecPtrTy = llvm::PointerType::get(*context, 0);
                                vecValue = builder->CreateLoad(vecPtrTy, vecValue, "byref.vec.field.load");
                            }
                        }
                    }
                }

                // Handle the Vec method with the evaluated value directly
                handleVecMethodOnValue(node, vecValue, methodName, memberExpr->object.get());
                // A fresh Vec temp receiver (`payload.split("\n").len()`) owns a
                // buffer nobody can reach once a pure read returns; release it here
                // or it leaks for the life of the program (TODO.md:248). Only the
                // pure readers qualify -- `get`/`last`/`peek`/`get_vec` hand back a
                // view of the receiver's buffer and the mutators hand back its
                // address, so freeing the temp under them would dangle (the same
                // reason exprProducesOwnedStringTemp excludes Vec element access).
                // #382: the element accessors qualify too, but only when what they
                // return does not alias the receiver's storage. A `Vec<Vec<T>>` slot is
                // the exception: `get` on it is typed as a borrow and yields the slot
                // address (#297), so `borrowedVecInnerNode` on the accessor's *result*
                // type is exactly that case and stays excluded. Primitive elements are
                // loaded by value, struct elements are deep-copied, and String elements
                // are retained by the accessor (`handleVecGet`/`handleVecLast`).
                const bool accessorYieldsOwnValue =
                    vecElementAccessorValueMethod(methodName) &&
                    borrowedVecInnerNode(typeOfNode(node).get()) == nullptr;
                if ((vecReadMethodSafeForTempReceiver(methodName) || accessorYieldsOwnValue) &&
                    exprProducesFreshVecTemp(memberExpr->object.get())) {
                    // The classifier already required a recorded type; the guard is
                    // for `codegenType`, not for the reclaim (a null AST type is a
                    // legitimate "free the buffer, nothing to unwind" case).
                    auto rcvAstNode = typeOfNode(memberExpr->object);
                    if (rcvAstNode) {
                        llvm::Type* rcvTy = vecValue->getType()->isStructTy()
                            ? vecValue->getType()
                            : codegenType(const_cast<vyb::ast::TypeNode*>(rcvAstNode.get()));
                        reclaimVecStorage(vecValue, rcvTy, rcvAstNode.get(), "vecrecv.tmp");
                    }
                }
                return;
            }

            // General aspect/bind method dispatch for a member-expression receiver
            // (e.g. `h.c.bump()` on a struct field, or `self.data.sort_in_place()`
            // through an ownership-wrapped their<Vec<T>> field). Built-in String/Vec
            // methods returned above; anything else that resolves to a bind lands here.
            {
                SemanticAnalyzer* semantic = driver_.hasSemanticAnalyzer()
                    ? driver_.getSemanticAnalyzer() : nullptr;
                ast::TypeNode* objTy2 = typeOfNode(memberExpr->object)
                    ? typeOfNode(memberExpr->object).get() : nullptr;
                if (semantic && objTy2 && (dynamic_cast<ast::TypeName*>(objTy2) ||
                                           dynamic_cast<ast::VecType*>(objTy2))) {
                    std::string concreteType = objTy2->toString();
                    bool receiverIsByRef = false;
                    if (auto objTn2 = dynamic_cast<ast::TypeName*>(objTy2)) {
                        const std::string kw = objTn2->identifier ? objTn2->identifier->name : "";
                        if ((kw == "their" || kw == "my" || kw == "our" ||
                             kw == "view" || kw == "borrow") && objTn2->genericArgs.size() == 1) {
                            ast::TypeNode* inner = objTn2->genericArgs[0].get();
                            // Dispatch bind/aspect methods against the unwrapped base type
                            // for ANY ownership-wrapped receiver (their<Counter> / my<X> /
                            // our<X> / view<X> / borrow<X>), not just wrapped Vecs. The slot
                            // of such a variable holds a by-ref borrow (a pointer to the
                            // pointee), so the self argument must be the loaded pointer.
                            concreteType = inner->toString();
                            receiverIsByRef = true;
                        }
                    }

                    // Find a bound aspect providing this method for concreteType.
                    std::string foundTrait;
                    bool found = false;
                    const auto& impls = semantic->getTraitImpls();
                    auto limpl = impls.find(concreteType);
                    if (limpl != impls.end()) {
                        for (const auto& traitEntry : limpl->second) {
                            for (const ast::FunctionDeclaration* m : traitEntry.second) {
                                if (m && m->id && m->id->name == methodName) {
                                    foundTrait = traitEntry.first; found = true; break;
                                }
                            }
                            if (found) break;
                        }
                    }
                    if (!found) {
                        TypePattern concretePattern = TypePattern::parse(concreteType);
                        const auto& genImpls = semantic->getGenericTraitImpls();
                        for (const auto& typeEntry : genImpls) {
                            TypePattern tmpl = TypePattern::parse(typeEntry.first);
                            std::map<std::string, std::string> sub;
                            if (!tmpl.matchesPattern(concretePattern, sub)) continue;
                            for (const auto& traitEntry : typeEntry.second) {
                                const GenericImplInfo* gii = traitEntry.second.get();
                                bool has = false;
                                if (gii && gii->declaration) {
                                    for (const auto& m : gii->declaration->methods) {
                                        if (m && m->id && m->id->name == methodName) { has = true; break; }
                                    }
                                }
                                if (has) { foundTrait = traitEntry.first; found = true; break; }
                            }
                            if (found) break;
                        }
                    }

                    if (found) {
                        llvm::Function* implFunc = module->getFunction(concreteType + "_" + foundTrait + "_" + methodName);
                        if (!implFunc) implFunc = monomorphizeTraitMethod(concreteType, foundTrait, methodName);
                        if (implFunc) {
                            // Evaluate the receiver in LHS (pointer) mode to get the
                            // address of the member field / object.
                            bool savedLHS = m_isLHSOfAssignment;
                            m_isLHSOfAssignment = true;
                            memberExpr->object->accept(*this);
                            m_isLHSOfAssignment = savedLHS;
                            llvm::Value* recvPtr = m_currentLLVMValue;
                            if (!recvPtr) {
                                logError(memberExpr->object->loc, "Failed to evaluate member-expression receiver for aspect method " + methodName);
                                m_currentLLVMValue = nullptr;
                                return;
                            }
                            // Ownership-wrapped field: recvPtr is the slot address (a
                            // pointer-to-pointer for a by-ref borrow); load once to recover
                            // the pointee pointer the by-ref self expects. #297: a
                            // call receiver already IS the borrowed pointer (a `get`
                            // borrow in flight), so it is not loaded again.
                            llvm::Value* selfArg = recvPtr;
                            if (receiverIsByRef && !receiverIsBorrowValue(memberExpr->object.get())) {
                                selfArg = builder->CreateLoad(llvm::PointerType::get(*context, 0), recvPtr, "byref.aspect.recv.load");
                            } else if (!selfArg->getType()->isPointerTy()) {
                                // A temporary receiver (e.g. `v.get(0)`, a method chain, or a
                                // struct-returning call) yields the struct value itself and has
                                // no address of its own. Materialize it in a stack slot so the
                                // receiver load below has a pointer operand instead of
                                // emitting `load %T, %T <value>` (#253).
                                llvm::AllocaInst* recvSlot =
                                    builder->CreateAlloca(selfArg->getType(), nullptr, "aspect.recv.temp");
                                builder->CreateStore(selfArg, recvSlot, "aspect.recv.temp.store");
                                selfArg = recvSlot;
                            }
                            bool selfIsByRef = implFunc->getArg(0)->getType()->isPointerTy();
                            std::vector<llvm::Value*> argValues;
                            if (selfIsByRef) {
                                argValues.push_back(selfArg);
                            } else {
                                argValues.push_back(builder->CreateLoad(implFunc->getArg(0)->getType(), selfArg, "aspect.recv.load"));
                            }
                            std::vector<llvm::Value*> transientClosures;
                            for (size_t a = 0; a < node->arguments.size(); ++a) {
                                auto& arg = node->arguments[a];
                                arg->accept(*this);
                                if (!m_currentLLVMValue) {
                                    logError(arg->loc, "Argument codegen failed for aspect method " + methodName);
                                    m_currentLLVMValue = nullptr;
                                    return;
                                }
                                llvm::Value* argVal = m_currentLLVMValue;
                                if (dynamic_cast<ast::FunctionExpression*>(arg.get()) &&
                                    argVal->getType()->isStructTy() &&
                                    isClosureStructType(argVal->getType())) {
                                    retainClosureValue(argVal);
                                    transientClosures.push_back(argVal);
                                }
                                if (argVal->getType()->isPointerTy()) {
                                    llvm::Type* expected = implFunc->getFunctionType()->getParamType(argValues.size());
                                    if (expected && expected->isStructTy()) {
                                        llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(expected);
                                        if (st && st->getNumElements() == 2 &&
                                            st->getElementType(0)->isPointerTy() &&
                                            st->getElementType(1)->isIntegerTy(64)) {
                                            llvm::Value* wrapped = tryCast(argVal, expected, arg->loc);
                                            if (wrapped) argVal = wrapped;
                                        }
                                    }
                                }
                                argValues.push_back(argVal);
                            }
                            // #255: an aspect method called with the wrong number of
                            // arguments must not reach the verifier -- an unmatched call
                            // fails LLVM verification and the compiler died on the
                            // invalid IR (SIGSEGV). Diagnose and halt instead. argValues
                            // includes the receiver as parameter 0.
                            if (argValues.size() != implFunc->getFunctionType()->getNumParams()) {
                                const size_t want = implFunc->getFunctionType()->getNumParams() - 1;
                                logError(node->loc, implFunc->getName().str() + " expects exactly " +
                                         std::to_string(want) + " argument" + (want == 1 ? "" : "s") + ", got " +
                                         std::to_string(argValues.size() - 1) + ".");
                                flagHardCodegenError();
                                m_currentLLVMValue = nullptr;
                                return;
                            }
                            if (implFunc->getReturnType()->isVoidTy()) {
                                builder->CreateCall(implFunc, argValues);
                                m_currentLLVMValue = nullptr;
                            } else {
                                m_currentLLVMValue = builder->CreateCall(implFunc, argValues, "aspect.method.result");
                            }
                            for (auto* tc : transientClosures) releaseClosureValue(tc);
                            return;
                        }
                    }
                }
            }
        }
    }

    // Check if this is a call to a generic function that needs monomorphization
    if (identCallee && genericFunctionTemplates.find(identCallee->name) != genericFunctionTemplates.end()) {
        VYB_CDBG << "DEBUG: Detected call to generic function: " << identCallee->name << std::endl;

        // Get the template to know how many type parameters we need
        ast::FunctionDeclaration* templateFunc = genericFunctionTemplates[identCallee->name];
        size_t numTypeParams = templateFunc->genericParams.size();

        // Choose concrete type arguments: explicit (probe<Int>(0, 0)) if the
        // caller wrote them, otherwise infer them from the call-site argument types.
        std::vector<std::string> concreteTypeArgs(numTypeParams, "");
        // Type-parameter names by slot, so structural inference can map a name it
        // meets inside a declared parameter type back to its concrete-args slot.
        std::vector<std::string> typeParamNames;
        for (const auto& gp : templateFunc->genericParams) {
            typeParamNames.push_back(gp && gp->name ? gp->name->name : std::string());
        }

        if (!node->explicitTypeArgs.empty()) {
            if (node->explicitTypeArgs.size() != numTypeParams) {
                logError(node->loc, "Explicit type argument count mismatch for generic function " + identCallee->name +
                         " (expected " + std::to_string(numTypeParams) + ", got " +
                         std::to_string(node->explicitTypeArgs.size()) + ")");
                m_currentLLVMValue = nullptr;
                return;
            }
            for (size_t i = 0; i < numTypeParams; ++i) {
                concreteTypeArgs[i] = node->explicitTypeArgs[i]->toString();
            }
            VYB_CDBG << "DEBUG: Using explicit type arguments for " << identCallee->name << std::endl;
        } else {
        // Infer concrete type arguments from call site
        // Strategy: match each argument to its corresponding type parameter
        // by checking if the argument's type matches the parameter's declared type pattern
        for (size_t i = 0; i < node->arguments.size(); ++i) {
            // Evaluate argument to infer type
            node->arguments[i]->accept(*this);
            llvm::Value* argValue = m_currentLLVMValue;
            if (!argValue) {
                logError(node->arguments[i]->loc, "Failed to evaluate argument for generic function call");
                m_currentLLVMValue = nullptr;
                return;
            }

            // Get type name from AST type annotation if available
            std::string argTypeName;
            if (typeOfNode(node->arguments[i])) {
                argTypeName = typeOfNode(node->arguments[i])->toString();
            } else if (auto* identArg = dynamic_cast<ast::Identifier*>(node->arguments[i].get())) {
                // Look up variable's type from valueTypeMap
                auto varIt = namedValues.find(identArg->name);
                if (varIt != namedValues.end()) {
                    auto typeIt = valueTypeMap.find(varIt->second);
                    if (typeIt != valueTypeMap.end()) {
                        argTypeName = typeIt->second->toString();
                    }
                }
            }

            // Fallback: use LLVM type name
            if (argTypeName.empty()) {
                argTypeName = getTypeName(argValue->getType());
            }

            VYB_CDBG << "DEBUG: Argument " << i << " has type: " << argTypeName << std::endl;

            // issue #343: infer the type parameter from WHERE it appears in the
            // declared parameter type, not by argument position. `f<T>(b<Box<T>>)`
            // called with `Box<Int>` must bind T = Int; binding T = Box<Int> (the
            // whole argument type) monomorphizes a signature of `Box<Box<Int>>`
            // and every call fails with "Argument type mismatch ... Expected
            // Box_Box_Int but got Box_Int". Positional matching stays as the
            // fallback for arguments that carried no AST type.
            if (i < templateFunc->params.size() && templateFunc->params[i].typeNode) {
                inferGenericArgsFromPattern(templateFunc->params[i].typeNode.get(),
                                            typeOfNode(node->arguments[i]).get(),
                                            typeParamNames, concreteTypeArgs);
            }
            // Fallback: map arg[i] positionally onto param[i]; if the function
            // has fewer type params than args, fill whatever slot is still empty.
            if (i < numTypeParams && concreteTypeArgs[i].empty()) {
                concreteTypeArgs[i] = argTypeName;
            } else if (numTypeParams > 0 && i >= numTypeParams) {
                // Extra arguments beyond type params — check if any remaining params are still empty
                for (size_t p = 0; p < numTypeParams; ++p) {
                    if (concreteTypeArgs[p].empty()) {
                        concreteTypeArgs[p] = argTypeName;
                        break;
                    }
                }
            }
        }
        // Fill any remaining empty type params with a default (shouldn't happen for valid code)
        for (size_t p = 0; p < numTypeParams; ++p) {
            if (concreteTypeArgs[p].empty()) {
                logError(node->loc, "Could not infer type for generic parameter '" +
                         (templateFunc->genericParams[p] && templateFunc->genericParams[p]->name ?
                          templateFunc->genericParams[p]->name->name : "unknown") + "'");
                m_currentLLVMValue = nullptr;
                return;
            }
        }
        // Resolve bind/generic-body type parameters (e.g. K -> String) in the
        // inferred/explicit concrete type args before monomorphizing, so a helper
        // called with a type-param argument (e.g. `hashkey(ck)` with `ck<K>` in a
        // `bind<K<Hashable>, V>` body) monomorphizes against the bind's concrete
        // instantiation instead of an unresolved `K`.
        for (auto& argStr : concreteTypeArgs) {
            for (const auto& kv : currentTypeSubstitutions) {
                argStr = replaceTypeTokens(argStr, kv.first, kv.second);
            }
        }
        }
        // Monomorphize the generic function
        llvm::Function* monomorphizedFunc = monomorphizeGenericFunction(identCallee->name, concreteTypeArgs);
        if (!monomorphizedFunc) {
            logError(node->loc, "Failed to monomorphize generic function " + identCallee->name);
            m_currentLLVMValue = nullptr;
            return;
        }

        VYB_CDBG << "DEBUG: Successfully monomorphized " << identCallee->name << std::endl;

        // Now proceed with the monomorphized function
        calleeName = monomorphizedFunc->getName().str();
    }

    // Lookup the function in the module - standard function call handling
    llvm::Function *calleeFunc = module->getFunction(calleeName);
    if (!calleeFunc) {
        // Try special functions that might not be in the module yet
        if (identCallee && identCallee->name == "println") {
            calleeFunc = getPrintlnFunction();
        } else if (identCallee) {
            // Check if it's a local lambda variable or a function-typed value.
            auto varIt = namedValues.find(identCallee->name);
            if (varIt != namedValues.end()) {
                // A function-typed parameter (`f<fn(T,T) -> T>`) or an explicitly
                // fn-annotated lambda variable has a FunctionType in valueTypeMap;
                // those are handled by the indirect-call path below (they share the
                // same `{ env, fn }` closure layout). Only a bare local lambda
                // (inferred closure-struct type) uses localLambdaTypes here.
                bool isFnTyped = false;
                auto vtmIt = valueTypeMap.find(varIt->second);
                isFnTyped = vtmIt != valueTypeMap.end() && vtmIt->second &&
                    dynamic_cast<ast::FunctionType*>(vtmIt->second.get()) != nullptr;
                auto lambdaTypeIt = localLambdaTypes.find(identCallee->name);
            if (!isFnTyped && lambdaTypeIt != localLambdaTypes.end()) {
                // It's a lambda stored in a local variable - use indirect call
                {
                    llvm::Value* closureAlloca = varIt->second;
                    llvm::FunctionType* lambdaFuncType = lambdaTypeIt->second;

                    // Load the closure struct { env, fn } and unpack it: call the
                    // fn pointer with the captured environment first, then args.
                    llvm::StructType* closureTy = getClosureStructType();
                    llvm::Value* closureVal = builder->CreateLoad(closureTy, closureAlloca, "lambda.closure");
                    llvm::Value* envPtr = builder->CreateExtractValue(closureVal, 0, "lambda.env");
                    llvm::Value* fnPtr = builder->CreateExtractValue(closureVal, 1, "lambda.fn");
                    // The callee's real signature also carries the environment.
                    std::vector<llvm::Type*> calleeParamTypes;
                    calleeParamTypes.push_back(llvm::PointerType::get(*context, 0));
                    for (auto* pt : lambdaFuncType->params()) calleeParamTypes.push_back(pt);
                    llvm::FunctionType* calleeType = llvm::FunctionType::get(
                        lambdaFuncType->getReturnType(), calleeParamTypes, false);
                    llvm::Value* funcPtr = builder->CreateBitCast(fnPtr, calleeType->getPointerTo(), "lambda.fptr");

                    // Build argument values
                    std::vector<llvm::Value*> lambdaArgValues;
                    lambdaArgValues.push_back(envPtr);  // hidden environment parameter
                    for (size_t i = 0; i < node->arguments.size(); ++i) {
                        node->arguments[i]->accept(*this);
                        llvm::Value* argVal = m_currentLLVMValue;
                        if (!argVal) {
                            logError(node->arguments[i]->loc, "Argument codegen failed for lambda call");
                            m_currentLLVMValue = nullptr;
                            return;
                        }
                        // Cast if needed
                        if (i < lambdaFuncType->getNumParams()) {
                            llvm::Type* expectedType = lambdaFuncType->getParamType(i);
                            if (argVal->getType() != expectedType) {
                                if (expectedType->isIntegerTy() && argVal->getType()->isIntegerTy()) {
                                    argVal = builder->CreateSExtOrTrunc(argVal, expectedType, "lambda.argcast");
                                } else if (expectedType->isFloatingPointTy() && argVal->getType()->isIntegerTy()) {
                                    argVal = builder->CreateSIToFP(argVal, expectedType, "lambda.argcast");
                                }
                            }
                        }
                        lambdaArgValues.push_back(argVal);
                    }

                    // Create indirect call with the full (env-inclusive) signature.
                    // A void-returning callee must not name the call result (LLVM
                    // refuses to give a name to a void value).
                    m_currentLLVMValue = builder->CreateCall(
                        calleeType, funcPtr, lambdaArgValues,
                        lambdaFuncType->getReturnType()->isVoidTy() ? "" : "lambda.result");
                    return;
                }
            }
            }
            // It may be a function-typed parameter (`f<fn(Int) -> Int>`, a closure)
            // or a bare C function pointer (`cb<loc<fn(Int) -> Int>>`), both lowered
            // to an indirect call through a pointer stored in the variable's alloca.
            // The closure carries a hidden environment parameter; the bare pointer
            // (a C callback) calls straight through with the declared signature.
            if (identCallee) {
                auto varIt = namedValues.find(identCallee->name);
                if (varIt != namedValues.end()) {
                    auto vtmIt = valueTypeMap.find(varIt->second);
                    ast::TypeNode* valTypeNode =
                        vtmIt != valueTypeMap.end() ? vtmIt->second.get() : nullptr;
                    ast::FunctionType* fnTypeNode = dynamic_cast<ast::FunctionType*>(valTypeNode);
                    bool bareFnPtr = false;
                    if (!fnTypeNode) {
                        // A bare C function pointer arrives as `loc<fn(...)>` / `CPtr<fn(...)>`,
                        // i.e. a `loc` TypeName wrapping a FunctionType generic argument.
                        if (auto* tn = dynamic_cast<ast::TypeName*>(valTypeNode)) {
                            if (tn->identifier &&
                                (tn->identifier->name == "loc" || tn->identifier->name == "CPtr") &&
                                !tn->genericArgs.empty()) {
                                if ((fnTypeNode = dynamic_cast<ast::FunctionType*>(tn->genericArgs[0].get()))) {
                                    bareFnPtr = true; // a bare code pointer
                                }
                            }
                        }
                    }
                    if (fnTypeNode) {
                        // Rebuild the LLVM function type from the declared signature
                        // (opaque-pointer mode has no typed PointerType to inspect).
                        std::vector<llvm::Type*> ptypes;
                        for (const auto& pn : fnTypeNode->parameterTypes) {
                            llvm::Type* t = pn ? codegenType(pn.get()) : nullptr;
                            if (t) ptypes.push_back(t);
                        }
                        llvm::Type* rt = fnTypeNode->returnType
                            ? codegenType(fnTypeNode->returnType.get())
                            : llvm::Type::getVoidTy(*context);
                        llvm::FunctionType* fnParamType = llvm::FunctionType::get(rt, ptypes, false);

                        llvm::Value* envPtr = nullptr;
                        llvm::Value* funcPtr = nullptr;
                        std::vector<llvm::Type*> calleeParamTypes;
                        if (bareFnPtr) {
                            // A bare C function pointer is a single code pointer; load
                            // it from the alloca and call with the declared signature only.
                            llvm::Type* ptrTy = llvm::PointerType::get(*context, 0);
                            funcPtr = builder->CreateLoad(ptrTy, varIt->second, "fnptr.val");
                            for (auto* pt : fnParamType->params()) calleeParamTypes.push_back(pt);
                        } else {
                            // The `fn` value is a closure struct { env, fn }.
                            llvm::StructType* closureTy = getClosureStructType();
                            llvm::Value* closureVal = builder->CreateLoad(closureTy, varIt->second, "fnparam.closure");
                            envPtr = builder->CreateExtractValue(closureVal, 0, "fnparam.env");
                            funcPtr = builder->CreateExtractValue(closureVal, 1, "fnparam.fn");
                            // The callee's real signature carries the environment too.
                            calleeParamTypes.push_back(llvm::PointerType::get(*context, 0));
                            for (auto* pt : fnParamType->params()) calleeParamTypes.push_back(pt);
                        }
                        llvm::FunctionType* calleeType = llvm::FunctionType::get(rt, calleeParamTypes, false);
                        funcPtr = builder->CreateBitCast(funcPtr, calleeType->getPointerTo(), "fnparam.fptr");
                        std::vector<llvm::Value*> fnArgValues;
                        if (envPtr) fnArgValues.push_back(envPtr);  // hidden environment parameter
                        for (size_t i = 0; i < node->arguments.size(); ++i) {
                            node->arguments[i]->accept(*this);
                            llvm::Value* argVal = m_currentLLVMValue;
                            if (!argVal) {
                                logError(node->arguments[i]->loc, "Argument codegen failed for function-typed param call");
                                m_currentLLVMValue = nullptr;
                                return;
                            }
                            if (i < fnParamType->getNumParams() && argVal->getType() != fnParamType->getParamType(i)) {
                                llvm::Type* expectedType = fnParamType->getParamType(i);
                                if (expectedType->isIntegerTy() && argVal->getType()->isIntegerTy()) {
                                    argVal = builder->CreateSExtOrTrunc(argVal, expectedType, "fnparam.argcast");
                                } else if (expectedType->isFloatingPointTy() && argVal->getType()->isIntegerTy()) {
                                    argVal = builder->CreateSIToFP(argVal, expectedType, "fnparam.argcast");
                                }
                            }
                            fnArgValues.push_back(argVal);
                        }
                        m_currentLLVMValue = builder->CreateCall(
                            calleeType, funcPtr, fnArgValues,
                            rt->isVoidTy() ? "" : "fnparam.result");
                        return;
                    }
                }
            }
            // Try mangled name if it's a method or from a namespace
            // This part needs a robust name mangling and lookup scheme
            logError(node->callee->loc, "Function " + calleeName + " not found.");
            // #344: an unresolved `self.<verb>()` (a sibling verb inside a bind) used to
            // fall through to undef and the program still ran, exiting 0 with a garbage
            // value. Refuse to run instead of silently miscompiling.
            if (calleeName.rfind("self.", 0) == 0) {
                flagHardCodegenError();
            }
            m_currentLLVMValue = nullptr;
            return;
        } else {
            logError(node->callee->loc, "Function " + calleeName + " not found.");
            if (calleeName.rfind("self.", 0) == 0) {
                flagHardCodegenError();
            }
            m_currentLLVMValue = nullptr;
            return;
        }
    }

    // Check argument count. Variadic functions accept any number of extra
    // arguments, but at least as many as their fixed parameters.
    size_t fixedParamCount = calleeFunc->getFunctionType()->getNumParams();
    if (calleeFunc->isVarArg()) {
        if (node->arguments.size() < fixedParamCount) {
            logError(node->loc, "Variadic function " + calleeName + " requires at least " +
                     std::to_string(fixedParamCount) + " argument(s), got " +
                     std::to_string(node->arguments.size()) + ".");
            m_currentLLVMValue = nullptr;
            return;
        }
    } else if (fixedParamCount != node->arguments.size()) {
        logError(node->loc, "Incorrect number of arguments passed to function " + calleeName);
        m_currentLLVMValue = nullptr;
        return;
    }

    std::vector<llvm::Value *> argValues;
    // Fresh owning temporaries passed as arguments are owned solely by this call
    // site; by-value params retain their own independent count, so each temp must
    // be released once the callee returns (see freshOwningCallArgKind).
    struct TempRelease { int kind; llvm::Value* cb; const vyb::ast::TypeNode* pointeeAst; llvm::Type* pointeeLlvm; };
    std::vector<TempRelease> pendingTemporaryReleases;
    // Freshly-built heap String (substring/concat/format) passed by value with no
    // named binding: the callee retains/releases its own copy, so the caller must
    // drop the temp's own reference after the call (see exprProducesOwnedStringTemp).
    std::vector<llvm::Value*> pendingStringTempFrees;
    // A fresh owned struct passed by value as an argument is deep-copied into the
    // callee's param by the callee (cgen_decl.cpp ownedStructParam), which reclaims
    // its own copy at scope exit; the caller still owns the temp's ORIGINAL buffers
    // and must reclaim them after the call or they leak (#192: decode_run(build_huff(
    // ...),...) leaked the temp Huff's count/symbol Vec buffers).
    struct StructTempReclaim { llvm::Value* ptr; const vyb::ast::TypeNode* ast; llvm::StructType* ll; };
    std::vector<StructTempReclaim> pendingStructTempReclaims;
    // Raw NUL-terminated heap copies made to satisfy a C string param (#209). A
    // Vyb String is `{ptr, len}` with no guaranteed terminator; a strlen-based
    // callee (cuModuleLoadData reading PTX text, etc.) would over-read past the
    // buffer, intermittently corrupting the image. Freed with `free` right after
    // the call (the callee only borrows the pointer during the call).
    std::vector<llvm::Value*> pendingCArgFrees;
    auto emitTemporaryReleases = [&]() {
        for (auto& pr : pendingTemporaryReleases) {
            if (pr.kind == 1) releaseOurControlBlock(pr.cb, "temparg.our", pr.pointeeAst, pr.pointeeLlvm);
            else if (pr.kind == 2) releaseMildControlBlock(pr.cb, "temparg.mild");
            else if (pr.kind == 3) {
                // A fresh `my(...)` payload (a new, caller-owned heap allocation)
                // with no named binding: reclaim any owned fields inside the struct
                // and free the block. The callee's `my` parameter borrows rather
                // than frees, so ownership returns to the call site.
                llvm::PointerType* rawPtr = llvm::PointerType::get(*context, 0);
                llvm::Constant* nullPtr = llvm::ConstantPointerNull::get(rawPtr);
                llvm::Value* isNull = builder->CreateICmpEQ(pr.cb, nullPtr, "temparg.my_null");
                llvm::BasicBlock* freeBB = llvm::BasicBlock::Create(*context, "temparg.my_free", currentFunction);
                llvm::BasicBlock* contBB = llvm::BasicBlock::Create(*context, "temparg.my_cont", currentFunction);
                builder->CreateCondBr(isNull, contBB, freeBB);
                builder->SetInsertPoint(freeBB);
                if (pr.pointeeAst && pr.pointeeLlvm) {
                    if (auto* structTy = llvm::dyn_cast<llvm::StructType>(pr.pointeeLlvm)) {
                        std::set<std::string> visited;
                        reclaimStructOwnedFieldsAt(pr.cb, pr.pointeeAst, structTy, visited);
                    }
                }
                builder->CreateCall(getOrCreateFreeFunction(), {pr.cb});
                builder->CreateBr(contBB);
                builder->SetInsertPoint(contBB);
            }
        }
        for (llvm::Value* sd : pendingStringTempFrees) {
            builder->CreateCall(getOrCreateVybStringFreeFunction(), {sd});
        }
        for (llvm::Value* cdata : pendingCArgFrees) {
            builder->CreateCall(getOrCreateFreeFunction(), {cdata});
        }
        for (auto& sr : pendingStructTempReclaims) {
            if (sr.ptr && sr.ast && sr.ll) {
                std::set<std::string> visited;
                reclaimStructOwnedFieldsAt(sr.ptr, sr.ast, sr.ll, visited);
            }
        }
    };
    // Build a heap NUL-terminated copy of a Vyb String `{ptr, len}` so a
    // strlen-based C callee (e.g. cuModuleLoadData reading PTX text) never
    // over-reads past the buffer into adjacent memory (#209). Registered into
    // pendingCArgFrees so the copy is freed with `free` right after the call
    // (the callee only borrows the pointer during the call).
    auto buildNulTermCStr = [&](llvm::Value* strStruct) -> llvm::Value* {
        llvm::Value* dataPtr = builder->CreateExtractValue(strStruct, 0, "cstr.data");
        llvm::Value* dataLen = builder->CreateExtractValue(strStruct, 1, "cstr.len");
        llvm::Value* allocSize = builder->CreateAdd(dataLen,
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "cstr.alloc");
        llvm::FunctionType* mallocType = llvm::FunctionType::get(
            llvm::PointerType::get(*context, 0), {llvm::Type::getInt64Ty(*context)}, false);
        llvm::Function* mallocFunc = module->getFunction("malloc");
        if (!mallocFunc) mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
        llvm::Value* newData = builder->CreateCall(mallocFunc, {allocSize}, "cstr.new");
        llvm::FunctionType* memcpyType = llvm::FunctionType::get(
            llvm::PointerType::get(*context, 0),
            {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)}, false);
        llvm::Function* memcpyFunc = module->getFunction("memcpy");
        if (!memcpyFunc) memcpyFunc = llvm::Function::Create(memcpyType, llvm::Function::ExternalLinkage, "memcpy", module.get());
        builder->CreateCall(memcpyFunc, {newData, dataPtr, dataLen});
        llvm::Value* nullPos = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, dataLen, "cstr.nul");
        builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0), nullPos);
        pendingCArgFrees.push_back(newData);
        return newData;
    };

    for (size_t i = 0; i < node->arguments.size(); ++i) {
        node->arguments[i]->accept(*this);
        llvm::Value* argValue = m_currentLLVMValue;
        if (!argValue) {
            logError(node->arguments[i]->loc, "Argument codegen failed for call to " + calleeName);
            m_currentLLVMValue = nullptr;
            return;
        }

        // A freshly-built String temp handed by value is released right after the
        // call; the callee retains/releases its own copy, leaving the temp's own
        // reference for us to drop.
        if (exprProducesOwnedStringTemp(node->arguments[i].get())) {
            llvm::Value* sd = argValue;
            if (argValue->getType() && argValue->getType()->isStructTy()) {
                sd = builder->CreateExtractValue(argValue, 0, "strtemparg.data");
            }
            pendingStringTempFrees.push_back(sd);
        }

        if (i < fixedParamCount) {
            // Fixed parameters: implicit cast to the declared parameter type.
            llvm::Type* expectedArgType = calleeFunc->getFunctionType()->getParamType(i);
            if (argValue->getType() != expectedArgType) {
                if (expectedArgType->isPointerTy()) {
                    auto* stringStructType = llvm::dyn_cast<llvm::StructType>(argValue->getType());
                    if (stringStructType && stringStructType->getNumElements() == 2 &&
                        stringStructType->getElementType(0)->isPointerTy() &&
                        stringStructType->getElementType(1)->isIntegerTy()) {
                        // A Vyb String `{ptr,len}` passed to a C string param.
                        // If the param is `char*` (a C string the C side reads
                        // with strlen), hand the callee a NUL-terminated COPY so
                        // it never over-reads past the (non-NUL-terminated)
                        // buffer -- e.g. cuModuleLoadData reading PTX text #209.
                        if (expectedArgType == int8PtrType) {
                            argValue = buildNulTermCStr(argValue);
                        } else {
                            argValue = builder->CreateExtractValue(argValue, {0}, "cstrarg");
                        }
                    }
                } else if (expectedArgType->isFloatingPointTy() && argValue->getType()->isIntegerTy()) {
                    argValue = builder->CreateSIToFP(argValue, expectedArgType, "callargcast");
                } else if (expectedArgType->isIntegerTy() && argValue->getType()->isFloatingPointTy()) {
                    argValue = builder->CreateFPToSI(argValue, expectedArgType, "callargcast");
                } else if (expectedArgType->isIntegerTy() && argValue->getType()->isIntegerTy()) {
                    // Handle integer width mismatches (e.g., i64 to i32)
                    llvm::IntegerType* expectedIntType = llvm::cast<llvm::IntegerType>(expectedArgType);
                    llvm::IntegerType* actualIntType = llvm::cast<llvm::IntegerType>(argValue->getType());

                    if (expectedIntType->getBitWidth() < actualIntType->getBitWidth()) {
                        // Truncate to smaller width (e.g., i64 to i32)
                        argValue = builder->CreateTrunc(argValue, expectedArgType, "callargtrunc");
                    } else if (expectedIntType->getBitWidth() > actualIntType->getBitWidth()) {
                        // Sign-extend to larger width (e.g., i32 to i64)
                        argValue = builder->CreateSExt(argValue, expectedArgType, "callargsext");
                    }
                } else if (expectedArgType->isStructTy() && argValue->getType()->isPointerTy()) {
                    // Expected a Vyb String { ptr, len } and got a raw char* (e.g.
                    // the result of to_string() / substring / concat). Fair
                    // String-struct shapes only: wrap the pointer into the struct
                    // using tryCast (which computes strlen for the length field).
                    llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(expectedArgType);
                    if (st && st->getNumElements() == 2 &&
                        st->getElementType(0)->isPointerTy() &&
                        st->getElementType(1)->isIntegerTy(64)) {
                        llvm::Value* wrapped = tryCast(argValue, expectedArgType, node->arguments[i]->loc);
                        if (wrapped) argValue = wrapped;
                    } else if (st && st->getNumElements() == 3 &&
                               st->getElementType(0)->isPointerTy() &&
                               st->getElementType(1)->isIntegerTy(64) &&
                               st->getElementType(2)->isIntegerTy(64)) {
                        // #297: a `their<Vec<T>>` argument (a nested `get` borrow in
                        // flight) reaching a by-value `Vec<T>` parameter is passed as
                        // the borrowed header. The callee deep-copies by-value Vec
                        // parameters on entry, so the parameter owns its own buffer
                        // and nothing aliases the container's element.
                        argValue = builder->CreateLoad(expectedArgType, argValue, "callarg.vecborrow");
                    }
                }
                // Add more sophisticated casting rules as needed
            }

            if (argValue->getType() != expectedArgType) {
                 logError(node->arguments[i]->loc, "Argument type mismatch for call to " + calleeName + ". Expected " + getTypeName(expectedArgType) + " but got " + getTypeName(argValue->getType()));
                 m_currentLLVMValue = nullptr;
                 return;
            }
        } else {
            // Variadic arguments have no declared parameter type, so pass the raw
            // value. As a convenience, auto-extract a Vyb String's data pointer so
            // C varargs such as `printf("%s", s)` receive the expected `char*`.
            if (auto* stringStructType = llvm::dyn_cast<llvm::StructType>(argValue->getType())) {
                if (stringStructType->getNumElements() == 2 &&
                    stringStructType->getElementType(0)->isPointerTy() &&
                    stringStructType->getElementType(1)->isIntegerTy()) {
                    argValue = builder->CreateExtractValue(argValue, {0}, "cvararg.str");
                }
            }
        }
        int tempKind = freshOwningCallArgKind(node->arguments[i].get());
        if (tempKind != 0) {
            TempRelease tr{tempKind, argValue, nullptr, nullptr};
            if (typeOfNode(node->arguments[i])) {
                if (tempKind == 1) {
                    tr.pointeeAst = ourPointeeOf(typeOfNode(node->arguments[i]).get());
                } else if (tempKind == 3) {
                    // Only free a fresh `my(...)` temp when it owns a new struct
                    // payload. `my` over a Vec/String shares its source, so those
                    // are released by the originating binding (never freed here).
                    if (isMyOwnedStructTypeNode(typeOfNode(node->arguments[i]).get())) {
                        tr.pointeeAst = myPointeeOf(typeOfNode(node->arguments[i]).get());
                    } else {
                        tempKind = 0;  // shared my<Vec>/my<String> temp: nothing to free
                    }
                }
                if (tr.pointeeAst) {
                    tr.pointeeLlvm = codegenType(const_cast<vyb::ast::TypeNode*>(tr.pointeeAst));
                }
            }
            if (tempKind != 0) {
                pendingTemporaryReleases.push_back(tr);
            }
        }
        // A freshly-built owned struct passed by value as an argument (a call/literal
        // whose result is a struct with owned Vec/String fields -- e.g.
        // `decode_run(build_huff(...), build_huff(...), ...)`). The callee deep-copies
        // the param (cgen_decl.cpp ownedStructParam) and reclaims its own copy at
        // scope exit; the caller still owns the temp's ORIGINAL buffers and must
        // reclaim them after the call. Must NOT fire on a named variable/borrow arg
        // (the callee or a sibling binding also owns it) -- only on an expression that
        // creates the struct fresh at this call site.
        bool freshOwnedStructArg =
            (dynamic_cast<ast::CallExpression*>(node->arguments[i].get()) != nullptr ||
             dynamic_cast<ast::ObjectLiteral*>(node->arguments[i].get()) != nullptr);
        if (tempKind == 0 && freshOwnedStructArg && typeOfNode(node->arguments[i]) &&
            argValue->getType() && argValue->getType()->isStructTy()) {
            const vyb::ast::TypeNode* at = typeOfNode(node->arguments[i]).get();
            if (isKnownStructTypeNode(at) && structTypeHasOwnedFields(at)) {
                if (auto* st = llvm::dyn_cast<llvm::StructType>(argValue->getType())) {
                    llvm::Value* tmp = builder->CreateAlloca(st, nullptr, "ownedstructarg.tmp");
                    builder->CreateStore(argValue, tmp);
                    pendingStructTempReclaims.push_back(StructTempReclaim{tmp, at, st});
                }
            }
        }
        // #228: a fresh owned Vec passed by value as an argument (e.g. the
        // `Vec<Record>` returned by `facts()` into `new_chain(origin, facts())`).
        // The callee deep-copies the by-value param and reclaims its own copy at
        // scope exit; the caller still owns the temp's ORIGINAL element buffer and
        // must reclaim it or it leaks (chain module: 1x128 B per seal). Register
        // the temp so cleanupVariable frees the buffer + owned elements at scope
        // exit -- but ONLY for an expression that creates the Vec fresh here
        // (a call/literal), never a named-variable or borrow arg (a sibling
        // binding also owns it -> double-free).
        bool freshOwnedVecArg =
            (dynamic_cast<ast::CallExpression*>(node->arguments[i].get()) != nullptr ||
             dynamic_cast<ast::ObjectLiteral*>(node->arguments[i].get()) != nullptr);
        if (tempKind == 0 && freshOwnedVecArg && typeOfNode(node->arguments[i]) &&
            isVecTypeNode(typeOfNode(node->arguments[i]).get())) {
            if (auto* vt = llvm::dyn_cast<llvm::StructType>(argValue->getType())) {
                llvm::Value* tmp = builder->CreateAlloca(vt, nullptr, "ownedvecarg.tmp");
                builder->CreateStore(argValue, tmp);
                valueTypeMap[tmp] = typeOfNode(node->arguments[i]);
                registerVariable("call.tmp.vec." + std::to_string(i), tmp, argValue,
                                 ast::OwnershipKind::MY, vt, true);
            }
        }
        argValues.push_back(argValue);
    }

    if (calleeFunc->getReturnType()->isVoidTy()) {
        builder->CreateCall(calleeFunc, argValues);
        emitTemporaryReleases();
        m_currentLLVMValue = nullptr; // No value for void calls
    } else {
        llvm::Value* callResult = builder->CreateCall(calleeFunc, argValues, "calltmp");
        emitTemporaryReleases();

        // Track Future<T> result type for opaque pointer handling in await expressions
        if (calleeFunc->getReturnType()->isStructTy()) {
            auto* structRetType = llvm::cast<llvm::StructType>(calleeFunc->getReturnType());
            // Future<T> struct has 4 fields: {T*, i32, i64, i8*}
            if (structRetType->getNumElements() == 4) {
                // Look up the function declaration to get the return type annotation
                if (auto* calleeIdent = dynamic_cast<ast::Identifier*>(node->callee.get())) {
                    if (m_currentVybModule) {
                        for (const auto& stmt : m_currentVybModule->body) {
                            auto* decl = dynamic_cast<ast::FunctionDeclaration*>(stmt.get());
                            if (decl && decl->id && decl->id->name == calleeIdent->name && decl->returnTypeNode) {
                                // Check if return type is Future<T>
                                if (auto* futureType = dynamic_cast<ast::FutureType*>(decl->returnTypeNode.get())) {
                                    if (futureType->resultType) {
                                        m_currentCallResultType = codegenType(futureType->resultType.get());
                                        VYB_CDBG << "DEBUG: Set m_currentCallResultType to " 
                                                 << getTypeName(m_currentCallResultType)
                                                 << " for Future call to " << calleeIdent->name << std::endl;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Phase 4: Check if this is a call to a semantically failable function.
        bool calleeNeedsErrorReturn = false;
        if (auto* calleeIdent = dynamic_cast<ast::Identifier*>(node->callee.get())) {
            if (m_currentVybModule) {
                for (const auto& stmt : m_currentVybModule->body) {
                    auto* decl = dynamic_cast<ast::FunctionDeclaration*>(stmt.get());
                    if (decl && decl->id && decl->id->name == calleeIdent->name && nodeNeedsErrorReturn(decl)) {
                        calleeNeedsErrorReturn = true;
                        break;
                    }
                }
            }
        }

        if (calleeNeedsErrorReturn) {
            if (llvm::StructType* structRetType = llvm::dyn_cast<llvm::StructType>(calleeFunc->getReturnType())) {
            if (structRetType->getNumElements() == 2 &&
                structRetType->getElementType(1)->isPointerTy()) {
                // This looks like a {T, ptr} return from a failable function
                VYB_CDBG << "DEBUG: Extracting error from failable function call to " << calleeName << std::endl;

                // Extract the value (position 0)
                llvm::Value* returnedValue = builder->CreateExtractValue(callResult, {0}, "call.value");

                // Extract the error pointer (position 1)
                llvm::Value* errorPtr = builder->CreateExtractValue(callResult, {1}, "call.error");

                // Check if error occurred (error != NULL)
                llvm::Value* hasError = builder->CreateICmpNE(
                    errorPtr,
                    llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(errorPtr->getType())),
                    "has.error"
                );

                // Create basic blocks for error handling
                llvm::Function* currentFunc = getCurrentFunction();
                llvm::BasicBlock* errorBB = llvm::BasicBlock::Create(*context, "call.error", currentFunc);
                llvm::BasicBlock* successBB = llvm::BasicBlock::Create(*context, "call.success", currentFunc);

                builder->CreateCondBr(hasError, errorBB, successBB);

                // Error block: check if we have a trap handler or need to propagate
                builder->SetInsertPoint(errorBB);
                VYB_CDBG << "DEBUG: Error detected, trapStack.size() = " << trapStack.size() << std::endl;
                if (!trapStack.empty()) {
                    // We have a trap handler - store error and jump to landing pad
                    // TODO(error-phase-3): keep this branch as the dedicated trap binding hook
                    // as trap payload typing/dispatch wiring is completed.
                    VYB_CDBG << "DEBUG: Storing error to trap.errorSlot and branching to landing pad" << std::endl;
                    TrapContext& trap = trapStack.back();
                    builder->CreateStore(errorPtr, trap.errorSlot);
                    builder->CreateBr(trap.landingPad);
                } else if ((currentFunctionAST && nodeNeedsErrorReturn(currentFunctionAST)) || m_currentFunctionFailable) {
                    // No trap but we're in a failable function - propagate to our caller
                    emitPropagatingErrorReturn(errorPtr);
                } else {
                    // No trap and not a failable function - call untrapped error handler
                    VYB_CDBG << "DEBUG: Error reaching untrapped handler" << std::endl;
                    llvm::Function* untrappedFn = getVybUntrappedErrorFunction();

                    // Pass the actual error pointer
                    builder->CreateCall(untrappedFn, {errorPtr});
                    builder->CreateUnreachable();
                }

                // Success block: continue with the actual value
                builder->SetInsertPoint(successBB);
                m_currentLLVMValue = returnedValue;
                return;
            }
            }
        }

        // Not a failable function call - use result directly
        m_currentLLVMValue = callResult;
    }
}

} // namespace vyb
