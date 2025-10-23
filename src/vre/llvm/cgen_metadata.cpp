// Vyn Type Metadata Generation
// Generates runtime type metadata for JSON serialization and reflection

#include "vyn/vre/llvm/codegen.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>

namespace vyn {

// Generate field metadata for a struct type
void LLVMCodegen::generateTypeMetadata(const std::string& typeName, ast::StructDeclaration* structDecl) {
    if (!structDecl) return;
    
    std::cout << "DEBUG: Generating type metadata for: " << typeName << std::endl;
    
    // Get the LLVM struct type
    auto structIt = monomorphizedStructs.find(typeName);
    if (structIt == monomorphizedStructs.end()) {
        std::cerr << "ERROR: Could not find struct type " << typeName << " for metadata generation" << std::endl;
        return;
    }
    llvm::StructType* structType = structIt->second;
    
    // Create arrays for field metadata
    std::vector<llvm::Constant*> fieldMetadataArray;
    
    for (size_t i = 0; i < structDecl->fields.size(); ++i) {
        const auto& fieldDecl = structDecl->fields[i];
        if (!fieldDecl || !fieldDecl->typeNode) continue;
        
        std::string fieldName = fieldDecl->name->name;
        std::string fieldTypeName = fieldDecl->typeNode->toString();
        
        // Determine if field is primitive
        bool isPrimitive = (fieldTypeName == "Int" || fieldTypeName == "Float" || 
                           fieldTypeName == "Bool" || fieldTypeName == "String");
        
        // TODO: Detect Vec types
        bool isVec = false;
        
        // Create field metadata struct
        // typedef struct VynFieldMetadata {
        //     const char* name;
        //     const char* type_name;
        //     size_t offset;
        //     size_t size;
        //     bool is_primitive;
        //     bool is_vec;
        //     const char* vec_element_type;
        // } VynFieldMetadata;
        
        // Create global string constants manually (not using builder, which requires function context)
        llvm::Constant* fieldNameStrConst = llvm::ConstantDataArray::getString(*context, fieldName, /*AddNull=*/true);
        llvm::GlobalVariable* fieldNameGlobal = new llvm::GlobalVariable(
            *module, fieldNameStrConst->getType(), true,
            llvm::GlobalValue::PrivateLinkage, fieldNameStrConst,
            "field_name_" + typeName + "_" + fieldName
        );
        llvm::Constant* fieldNameStr = llvm::ConstantExpr::getBitCast(
            fieldNameGlobal,
            llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0)
        );
        
        llvm::Constant* fieldTypeStrConst = llvm::ConstantDataArray::getString(*context, fieldTypeName, /*AddNull=*/true);
        llvm::GlobalVariable* fieldTypeGlobal = new llvm::GlobalVariable(
            *module, fieldTypeStrConst->getType(), true,
            llvm::GlobalValue::PrivateLinkage, fieldTypeStrConst,
            "field_type_" + typeName + "_" + fieldName
        );
        llvm::Constant* fieldTypeNameStr = llvm::ConstantExpr::getBitCast(
            fieldTypeGlobal,
            llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0)
        );
        
        // Get field offset and size from LLVM DataLayout
        const llvm::DataLayout& dataLayout = module->getDataLayout();
        const llvm::StructLayout* structLayout = dataLayout.getStructLayout(structType);
        uint64_t offset = structLayout->getElementOffset(i);
        llvm::Type* fieldType = structType->getElementType(i);
        uint64_t size = dataLayout.getTypeAllocSize(fieldType);
        
        // Create field metadata constant
        llvm::PointerType* int8PtrType = llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
        llvm::Type* int64Type = llvm::Type::getInt64Ty(*context);
        llvm::Type* int1Type = llvm::Type::getInt1Ty(*context);
        
        std::vector<llvm::Constant*> fieldMetadataFields = {
            fieldNameStr,
            fieldTypeNameStr,
            llvm::ConstantInt::get(int64Type, offset),
            llvm::ConstantInt::get(int64Type, size),
            llvm::ConstantInt::get(int1Type, isPrimitive),
            llvm::ConstantInt::get(int1Type, isVec),
            llvm::ConstantPointerNull::get(int8PtrType) // vec_element_type (NULL for now)
        };
        
        // Create anonymous struct type for field metadata
        llvm::StructType* fieldMetadataType = llvm::StructType::get(
            *context,
            {int8PtrType, int8PtrType, int64Type, int64Type, int1Type, int1Type, int8PtrType},
            /*isPacked=*/false
        );
        
        llvm::Constant* fieldMetadata = llvm::ConstantStruct::get(fieldMetadataType, fieldMetadataFields);
        fieldMetadataArray.push_back(fieldMetadata);
    }
    
    // Create global array of field metadata
    if (fieldMetadataArray.empty()) {
        std::cout << "DEBUG: No fields to generate metadata for " << typeName << std::endl;
        return;
    }
    
    llvm::ArrayType* fieldArrayType = llvm::ArrayType::get(
        fieldMetadataArray[0]->getType(),
        fieldMetadataArray.size()
    );
    
    llvm::Constant* fieldArrayInit = llvm::ConstantArray::get(fieldArrayType, fieldMetadataArray);
    
    llvm::GlobalVariable* fieldArrayGlobal = new llvm::GlobalVariable(
        *module,
        fieldArrayType,
        true, // isConstant
        llvm::GlobalValue::PrivateLinkage,
        fieldArrayInit,
        "__vyn_fields_" + typeName
    );
    
    // Create VynTypeMetadata struct
    // typedef struct VynTypeMetadata {
    //     const char* type_name;
    //     size_t struct_size;
    //     size_t num_fields;
    //     VynFieldMetadata* fields;
    //     size_t num_aspects;
    //     VynAspectBinding* aspects;
    // } VynTypeMetadata;
    
    llvm::PointerType* int8PtrType = llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
    llvm::Type* int64Type = llvm::Type::getInt64Ty(*context);
    
    // Create type name string constant
    llvm::Constant* typeNameStrConst = llvm::ConstantDataArray::getString(*context, typeName, /*AddNull=*/true);
    llvm::GlobalVariable* typeNameGlobal = new llvm::GlobalVariable(
        *module, typeNameStrConst->getType(), true,
        llvm::GlobalValue::PrivateLinkage, typeNameStrConst,
        "type_name_" + typeName
    );
    llvm::Constant* typeNameStr = llvm::ConstantExpr::getBitCast(
        typeNameGlobal,
        int8PtrType
    );
    
    const llvm::DataLayout& dataLayout = module->getDataLayout();
    uint64_t structSize = dataLayout.getTypeAllocSize(structType);
    
    std::vector<llvm::Constant*> typeMetadataFields = {
        typeNameStr,
        llvm::ConstantInt::get(int64Type, structSize),
        llvm::ConstantInt::get(int64Type, fieldMetadataArray.size()),
        llvm::ConstantExpr::getBitCast(fieldArrayGlobal, int8PtrType), // Cast array to pointer
        llvm::ConstantInt::get(int64Type, 0), // num_aspects (TODO: implement aspect metadata)
        llvm::ConstantPointerNull::get(int8PtrType) // aspects (NULL for now)
    };
    
    llvm::StructType* typeMetadataStructType = llvm::StructType::get(
        *context,
        {int8PtrType, int64Type, int64Type, int8PtrType, int64Type, int8PtrType},
        /*isPacked=*/false
    );
    
    llvm::Constant* typeMetadataInit = llvm::ConstantStruct::get(typeMetadataStructType, typeMetadataFields);
    
    llvm::GlobalVariable* typeMetadataGlobal = new llvm::GlobalVariable(
        *module,
        typeMetadataStructType,
        true, // isConstant
        llvm::GlobalValue::ExternalLinkage, // Make it visible for runtime registration
        typeMetadataInit,
        "__vyn_metadata_" + typeName
    );
    
    // Add registration call (will be called at program startup)
    // We need to generate a constructor function that calls __vyn_register_type
    // For now, we'll just store the metadata global and register it later
    typeMetadataGlobals[typeName] = typeMetadataGlobal;
    
    std::cout << "DEBUG: Generated metadata for " << typeName 
              << " with " << fieldMetadataArray.size() << " fields" << std::endl;
}

// Register all type metadata at runtime
void LLVMCodegen::registerTypeMetadata() {
    if (typeMetadataGlobals.empty()) {
        std::cout << "DEBUG: No type metadata to register" << std::endl;
        return;
    }
    
    std::cout << "DEBUG: Registering " << typeMetadataGlobals.size() << " type metadata entries" << std::endl;
    
    // Declare __vyn_register_type function
    llvm::PointerType* int8PtrType = llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
    llvm::FunctionType* registerFuncType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*context),
        {int8PtrType}, // Takes VynTypeMetadata* (as i8*)
        false
    );
    
    llvm::Function* registerFunc = module->getFunction("__vyn_register_type");
    if (!registerFunc) {
        registerFunc = llvm::Function::Create(
            registerFuncType,
            llvm::Function::ExternalLinkage,
            "__vyn_register_type",
            module.get()
        );
    }
    
    // Create a global constructor function to register all types at startup
    llvm::FunctionType* ctorType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*context),
        false
    );
    
    llvm::Function* ctorFunc = llvm::Function::Create(
        ctorType,
        llvm::Function::ExternalLinkage,  // Make it visible to JIT
        "__vyn_register_all_types",
        module.get()
    );
    
    llvm::BasicBlock* ctorBB = llvm::BasicBlock::Create(*context, "entry", ctorFunc);
    llvm::IRBuilder<> ctorBuilder(ctorBB);
    
    // Call __vyn_register_type for each type
    for (const auto& pair : typeMetadataGlobals) {
        llvm::Value* metadataPtr = ctorBuilder.CreateBitCast(pair.second, int8PtrType);
        ctorBuilder.CreateCall(registerFunc, {metadataPtr});
        std::cout << "DEBUG: Added registration call for type: " << pair.first << std::endl;
    }
    
    ctorBuilder.CreateRetVoid();
    
    // Add this function to the global constructors list manually
    // This ensures it runs before main()
    // The llvm.global_ctors array contains {i32 priority, void()* ctor, i8* associated data or null}
    llvm::StructType* ctorStructType = llvm::StructType::get(
        *context,
        {
            llvm::Type::getInt32Ty(*context),  // priority
            ctorType->getPointerTo(),           // function pointer
            int8PtrType                         // associated data (null)
        }
    );
    
    std::vector<llvm::Constant*> ctorArray;
    ctorArray.push_back(llvm::ConstantStruct::get(
        ctorStructType,
        {
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 65535),  // priority
            ctorFunc,
            llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0))
        }
    ));
    
    llvm::ArrayType* ctorArrayType = llvm::ArrayType::get(ctorStructType, ctorArray.size());
    new llvm::GlobalVariable(
        *module,
        ctorArrayType,
        false,  // not constant
        llvm::GlobalValue::AppendingLinkage,
        llvm::ConstantArray::get(ctorArrayType, ctorArray),
        "llvm.global_ctors"
    );
    
    std::cout << "DEBUG: Type metadata registration complete" << std::endl;
}

} // namespace vyn
