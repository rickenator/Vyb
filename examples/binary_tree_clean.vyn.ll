; ModuleID = 'VynModule'
source_filename = "VynModule"

%BinaryTree = type { { ptr, i64, i64 }, i1, i64 }
%TreeNode = type { i64, ptr }

@0 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@2 = private unnamed_addr constant [14 x i8] c"Tree is empty\00", align 1
@3 = private unnamed_addr constant [15 x i8] c"Tree contents:\00", align 1
@4 = private unnamed_addr constant [6 x i8] c"Key: \00", align 1
@5 = private unnamed_addr constant [10 x i8] c", Value: \00", align 1
@6 = private unnamed_addr constant [46 x i8] c"Clean Binary Search Tree Example - Vyn v0.4.0\00", align 1
@7 = private unnamed_addr constant [47 x i8] c"==============================================\00", align 1
@8 = private unnamed_addr constant [20 x i8] c"Inserting values...\00", align 1
@9 = private unnamed_addr constant [6 x i8] c"fifty\00", align 1
@10 = private unnamed_addr constant [7 x i8] c"thirty\00", align 1
@11 = private unnamed_addr constant [8 x i8] c"seventy\00", align 1
@12 = private unnamed_addr constant [7 x i8] c"twenty\00", align 1
@13 = private unnamed_addr constant [6 x i8] c"forty\00", align 1
@14 = private unnamed_addr constant [12 x i8] c"Tree size: \00", align 1
@15 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@16 = private unnamed_addr constant [19 x i8] c"Search operations:\00", align 1
@17 = private unnamed_addr constant [16 x i8] c"Search for 40: \00", align 1
@18 = private unnamed_addr constant [16 x i8] c"Search for 99: \00", align 1
@19 = private unnamed_addr constant [16 x i8] c"Search for 20: \00", align 1
@20 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define %BinaryTree @new_tree() !dbg !4 {
entry:
  %BinaryTree_obj = alloca %BinaryTree, align 8, !dbg !8
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !8
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !8
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !8
  store i64 0, ptr %vec.size_field, align 4, !dbg !8
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !8
  store i64 0, ptr %vec.cap_field, align 4, !dbg !8
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !8
  %nodes_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 0, !dbg !8
  store { ptr, i64, i64 } %vec.new.value, ptr %nodes_ptr, align 8, !dbg !8
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 1, !dbg !8
  store i1 false, ptr %has_root_ptr, align 1, !dbg !8
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 2, !dbg !8
  store i64 0, ptr %size_ptr, align 4, !dbg !8
  %BinaryTree_val = load %BinaryTree, ptr %BinaryTree_obj, align 8, !dbg !8
  ret %BinaryTree %BinaryTree_val, !dbg !8
}

define void @insert(ptr %tree, i64 %key, ptr %value) !dbg !9 {
entry:
  %new_node = alloca %TreeNode, align 8
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca ptr, align 8
  store ptr %tree, ptr %tree1, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !15, metadata !DIExpression()), !dbg !21
  store i64 %key, ptr %key2, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !16, metadata !DIExpression()), !dbg !22
  store ptr %value, ptr %value3, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !17, metadata !DIExpression()), !dbg !23
  %TreeNode_obj = alloca %TreeNode, align 8, !dbg !20
  %key4 = load i64, ptr %key2, align 4, !dbg !20
  %key_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 0, !dbg !20
  store i64 %key4, ptr %key_ptr, align 4, !dbg !20
  %value5 = load ptr, ptr %value3, align 8, !dbg !20
  %value_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 1, !dbg !20
  store ptr %value5, ptr %value_ptr, align 8, !dbg !20
  %TreeNode_val = load %TreeNode, ptr %TreeNode_obj, align 8, !dbg !20
  store %TreeNode %TreeNode_val, ptr %new_node, align 8, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %new_node, metadata !18, metadata !DIExpression()), !dbg !24
  %tree6 = load ptr, ptr %tree1, align 8, !dbg !20
  %nodes_ptr = getelementptr inbounds %BinaryTree, ptr %tree6, i32 0, i32 0, !dbg !20
  %new_node7 = load %TreeNode, ptr %new_node, align 8, !dbg !20
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %nodes_ptr, i32 0, i32 0, !dbg !20
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %nodes_ptr, i32 0, i32 1, !dbg !20
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %nodes_ptr, i32 0, i32 2, !dbg !20
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !20
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !20
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !20
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !20
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !20
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !20
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !20

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !20
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !20
  %vec.alloc_size = mul i64 %vec.new_cap, 16, !dbg !20
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !20
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !20
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !20

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 16, !dbg !20
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !20
  br label %vec.no_copy, !dbg !20

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !20
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !20
  br label %vec.merge, !dbg !20

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !20
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !20
  %vec.offset = mul i64 %vec.reloaded_size, 16, !dbg !20
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !20
  %vec.temp_struct = alloca %TreeNode, align 8, !dbg !20
  store %TreeNode %new_node7, ptr %vec.temp_struct, align 8, !dbg !20
  %2 = call ptr @memcpy(ptr %vec.element_ptr, ptr %vec.temp_struct, i64 16), !dbg !20
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !20
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !20
  %tree8 = load ptr, ptr %tree1, align 8, !dbg !20
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %tree8, i32 0, i32 1, !dbg !20
  store i1 true, ptr %has_root_ptr, align 1, !dbg !20
  %tree9 = load ptr, ptr %tree1, align 8, !dbg !20
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %tree9, i32 0, i32 2, !dbg !20
  %tree10 = load ptr, ptr %tree1, align 8, !dbg !20
  %size_ptr11 = getelementptr inbounds %BinaryTree, ptr %tree10, i32 0, i32 2, !dbg !20
  %size_val = load i64, ptr %size_ptr11, align 4, !dbg !20
  %addtmp = add i64 %size_val, 1, !dbg !20
  store i64 %addtmp, ptr %size_ptr, align 4, !dbg !20
  ret void, !dbg !20
}

define ptr @search(%BinaryTree %tree, i64 %key) !dbg !25 {
entry:
  %node = alloca %TreeNode, align 8
  %i = alloca i64, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !29, metadata !DIExpression()), !dbg !34
  store i64 %key, ptr %key2, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !30, metadata !DIExpression()), !dbg !35
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !33
  %temp_struct = alloca %BinaryTree, align 8, !dbg !33
  store %BinaryTree %tree3, ptr %temp_struct, align 8, !dbg !33
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !33
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !33
  %nottmp = xor i1 %has_root_val, true, !dbg !33
  br i1 %nottmp, label %then, label %ifcont, !dbg !33

then:                                             ; preds = %entry
  ret ptr @0, !dbg !33

ifcont:                                           ; preds = %entry
  store i64 0, ptr %i, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %i, metadata !31, metadata !DIExpression()), !dbg !36
  br label %loop.header, !dbg !33

loop.header:                                      ; preds = %ifcont16, %ifcont
  %i4 = load i64, ptr %i, align 4, !dbg !33
  %tree5 = load %BinaryTree, ptr %tree1, align 8, !dbg !33
  %temp_struct6 = alloca %BinaryTree, align 8, !dbg !33
  store %BinaryTree %tree5, ptr %temp_struct6, align 8, !dbg !33
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct6, i32 0, i32 2, !dbg !33
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !33
  %icmpslttmp = icmp slt i64 %i4, %size_val, !dbg !33
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !33

loop.body:                                        ; preds = %loop.header
  %tree7 = load %BinaryTree, ptr %tree1, align 8, !dbg !33
  %temp_struct8 = alloca %BinaryTree, align 8, !dbg !33
  store %BinaryTree %tree7, ptr %temp_struct8, align 8, !dbg !33
  %nodes_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct8, i32 0, i32 0, !dbg !33
  %i9 = load i64, ptr %i, align 4, !dbg !33
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %nodes_ptr, i32 0, i32 0, !dbg !33
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %nodes_ptr, i32 0, i32 1, !dbg !33
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !33
  %vec.size = load i64, ptr %vec.size_ptr, align 4, !dbg !33
  %vec.offset = mul i64 %i9, 16, !dbg !33
  %vec.element_ptr = getelementptr i8, ptr %vec.data, i64 %vec.offset, !dbg !33
  %vec.element_struct = load %TreeNode, ptr %vec.element_ptr, align 8, !dbg !33
  store %TreeNode %vec.element_struct, ptr %node, align 8, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %node, metadata !32, metadata !DIExpression()), !dbg !37
  %node10 = load %TreeNode, ptr %node, align 8, !dbg !33
  %temp_struct11 = alloca %TreeNode, align 8, !dbg !33
  store %TreeNode %node10, ptr %temp_struct11, align 8, !dbg !33
  %key_ptr = getelementptr inbounds %TreeNode, ptr %temp_struct11, i32 0, i32 0, !dbg !33
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !33
  %key12 = load i64, ptr %key2, align 4, !dbg !33
  %icmpeqtmp = icmp eq i64 %key_val, %key12, !dbg !33
  br i1 %icmpeqtmp, label %then13, label %ifcont16, !dbg !33

loop.exit:                                        ; preds = %loop.header
  ret ptr @1, !dbg !33

then13:                                           ; preds = %loop.body
  %node14 = load %TreeNode, ptr %node, align 8, !dbg !33
  %temp_struct15 = alloca %TreeNode, align 8, !dbg !33
  store %TreeNode %node14, ptr %temp_struct15, align 8, !dbg !33
  %value_ptr = getelementptr inbounds %TreeNode, ptr %temp_struct15, i32 0, i32 1, !dbg !33
  ret ptr %value_ptr, !dbg !33

ifcont16:                                         ; preds = %loop.body
  %i17 = load i64, ptr %i, align 4, !dbg !33
  %addtmp = add i64 %i17, 1, !dbg !33
  store i64 %addtmp, ptr %i, align 4, !dbg !33
  br label %loop.header, !dbg !33
}

define void @print_tree(%BinaryTree %tree) !dbg !38 {
entry:
  %node = alloca %TreeNode, align 8
  %i = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !45
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !42, metadata !DIExpression()), !dbg !46
  %tree2 = load %BinaryTree, ptr %tree1, align 8, !dbg !45
  %temp_struct = alloca %BinaryTree, align 8, !dbg !45
  store %BinaryTree %tree2, ptr %temp_struct, align 8, !dbg !45
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !45
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !45
  %nottmp = xor i1 %has_root_val, true, !dbg !45
  br i1 %nottmp, label %then, label %ifcont, !dbg !45

then:                                             ; preds = %entry
  call void @__vyn_println(ptr @2), !dbg !45
  ret void, !dbg !45

ifcont:                                           ; preds = %entry
  call void @__vyn_println(ptr @3), !dbg !45
  store i64 0, ptr %i, align 4, !dbg !45
  call void @llvm.dbg.declare(metadata ptr %i, metadata !43, metadata !DIExpression()), !dbg !47
  br label %loop.header, !dbg !45

loop.header:                                      ; preds = %loop.body, %ifcont
  %i3 = load i64, ptr %i, align 4, !dbg !45
  %tree4 = load %BinaryTree, ptr %tree1, align 8, !dbg !45
  %temp_struct5 = alloca %BinaryTree, align 8, !dbg !45
  store %BinaryTree %tree4, ptr %temp_struct5, align 8, !dbg !45
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct5, i32 0, i32 2, !dbg !45
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !45
  %icmpslttmp = icmp slt i64 %i3, %size_val, !dbg !45
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !45

loop.body:                                        ; preds = %loop.header
  %tree6 = load %BinaryTree, ptr %tree1, align 8, !dbg !45
  %temp_struct7 = alloca %BinaryTree, align 8, !dbg !45
  store %BinaryTree %tree6, ptr %temp_struct7, align 8, !dbg !45
  %nodes_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct7, i32 0, i32 0, !dbg !45
  %i8 = load i64, ptr %i, align 4, !dbg !45
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %nodes_ptr, i32 0, i32 0, !dbg !45
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %nodes_ptr, i32 0, i32 1, !dbg !45
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !45
  %vec.size = load i64, ptr %vec.size_ptr, align 4, !dbg !45
  %vec.offset = mul i64 %i8, 16, !dbg !45
  %vec.element_ptr = getelementptr i8, ptr %vec.data, i64 %vec.offset, !dbg !45
  %vec.element_struct = load %TreeNode, ptr %vec.element_ptr, align 8, !dbg !45
  store %TreeNode %vec.element_struct, ptr %node, align 8, !dbg !45
  call void @llvm.dbg.declare(metadata ptr %node, metadata !44, metadata !DIExpression()), !dbg !48
  %node9 = load %TreeNode, ptr %node, align 8, !dbg !45
  %temp_struct10 = alloca %TreeNode, align 8, !dbg !45
  store %TreeNode %node9, ptr %temp_struct10, align 8, !dbg !45
  %key_ptr = getelementptr inbounds %TreeNode, ptr %temp_struct10, i32 0, i32 0, !dbg !45
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !45
  %tostring = call ptr @__vyn_toString_int(i64 %key_val), !dbg !45
  %strcattmp = call ptr @__vyn_string_concat(ptr @4, ptr %tostring), !dbg !45
  %strcattmp11 = call ptr @__vyn_string_concat(ptr %strcattmp, ptr @5), !dbg !45
  %node12 = load %TreeNode, ptr %node, align 8, !dbg !45
  %temp_struct13 = alloca %TreeNode, align 8, !dbg !45
  store %TreeNode %node12, ptr %temp_struct13, align 8, !dbg !45
  %value_ptr = getelementptr inbounds %TreeNode, ptr %temp_struct13, i32 0, i32 1, !dbg !45
  %strcattmp14 = call ptr @__vyn_string_concat(ptr %strcattmp11, ptr %value_ptr), !dbg !45
  call void @__vyn_println(ptr %strcattmp14), !dbg !45
  %i15 = load i64, ptr %i, align 4, !dbg !45
  %addtmp = add i64 %i15, 1, !dbg !45
  store i64 %addtmp, ptr %i, align 4, !dbg !45
  br label %loop.header, !dbg !45

loop.exit:                                        ; preds = %loop.header
  ret void, !dbg !45
}

define i64 @main() !dbg !49 {
entry:
  %result3 = alloca ptr, align 8, !dbg !57
  %result2 = alloca ptr, align 8, !dbg !57
  %result1 = alloca ptr, align 8, !dbg !57
  %tree = alloca %BinaryTree, align 8, !dbg !57
  %calltmp = call %BinaryTree @new_tree(), !dbg !57
  store %BinaryTree %calltmp, ptr %tree, align 8, !dbg !57
  call void @llvm.dbg.declare(metadata ptr %tree, metadata !53, metadata !DIExpression()), !dbg !58
  call void @__vyn_println(ptr @6), !dbg !57
  call void @__vyn_println(ptr @7), !dbg !57
  call void @__vyn_println(ptr @8), !dbg !57
  call void @insert(ptr %tree, i64 50, ptr @9), !dbg !57
  call void @insert(ptr %tree, i64 30, ptr @10), !dbg !57
  call void @insert(ptr %tree, i64 70, ptr @11), !dbg !57
  call void @insert(ptr %tree, i64 20, ptr @12), !dbg !57
  call void @insert(ptr %tree, i64 40, ptr @13), !dbg !57
  %tree1 = load %BinaryTree, ptr %tree, align 8, !dbg !57
  %temp_struct = alloca %BinaryTree, align 8, !dbg !57
  store %BinaryTree %tree1, ptr %temp_struct, align 8, !dbg !57
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 2, !dbg !57
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !57
  %tostring = call ptr @__vyn_toString_int(i64 %size_val), !dbg !57
  %strcattmp = call ptr @__vyn_string_concat(ptr @14, ptr %tostring), !dbg !57
  call void @__vyn_println(ptr %strcattmp), !dbg !57
  call void @__vyn_println(ptr @15), !dbg !57
  call void @__vyn_println(ptr @16), !dbg !57
  %tree2 = load %BinaryTree, ptr %tree, align 8, !dbg !57
  %calltmp3 = call ptr @search(%BinaryTree %tree2, i64 40), !dbg !57
  store ptr %calltmp3, ptr %result1, align 8, !dbg !57
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !54, metadata !DIExpression()), !dbg !59
  %result14 = load ptr, ptr %result1, align 8, !dbg !57
  %strcattmp5 = call ptr @__vyn_string_concat(ptr @17, ptr %result14), !dbg !57
  call void @__vyn_println(ptr %strcattmp5), !dbg !57
  %tree6 = load %BinaryTree, ptr %tree, align 8, !dbg !57
  %calltmp7 = call ptr @search(%BinaryTree %tree6, i64 99), !dbg !57
  store ptr %calltmp7, ptr %result2, align 8, !dbg !57
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !55, metadata !DIExpression()), !dbg !60
  %result28 = load ptr, ptr %result2, align 8, !dbg !57
  %strcattmp9 = call ptr @__vyn_string_concat(ptr @18, ptr %result28), !dbg !57
  call void @__vyn_println(ptr %strcattmp9), !dbg !57
  %tree10 = load %BinaryTree, ptr %tree, align 8, !dbg !57
  %calltmp11 = call ptr @search(%BinaryTree %tree10, i64 20), !dbg !57
  store ptr %calltmp11, ptr %result3, align 8, !dbg !57
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !56, metadata !DIExpression()), !dbg !61
  %result312 = load ptr, ptr %result3, align 8, !dbg !57
  %strcattmp13 = call ptr @__vyn_string_concat(ptr @19, ptr %result312), !dbg !57
  call void @__vyn_println(ptr %strcattmp13), !dbg !57
  call void @__vyn_println(ptr @20), !dbg !57
  %tree14 = load %BinaryTree, ptr %tree, align 8, !dbg !57
  call void @print_tree(%BinaryTree %tree14), !dbg !57
  ret i64 0, !dbg !57
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare void @__vyn_println(ptr)

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "binary_tree_clean.vyn.ll", directory: "/home/rick/Projects/Vyn/examples")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "new_tree", linkageName: "new_tree", scope: !1, file: !1, line: 17, type: !5, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "BinaryTree", scope: !1, file: !1, size: 192, align: 8)
!8 = !DILocation(line: 17, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "insert", linkageName: "insert", scope: !1, file: !1, line: 26, type: !10, scopeLine: 26, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!10 = !DISubroutineType(types: !11)
!11 = !{null, !12, !13, !12}
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!13 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!14 = !{!15, !16, !17, !18}
!15 = !DILocalVariable(name: "tree", scope: !9, file: !1, line: 27, type: !12)
!16 = !DILocalVariable(name: "key", scope: !9, file: !1, line: 27, type: !13)
!17 = !DILocalVariable(name: "value", scope: !9, file: !1, line: 27, type: !12)
!18 = !DILocalVariable(name: "new_node", scope: !9, file: !1, line: 28, type: !19)
!19 = !DICompositeType(tag: DW_TAG_structure_type, name: "TreeNode", scope: !1, file: !1, size: 128, align: 8)
!20 = !DILocation(line: 26, column: 1, scope: !9)
!21 = !DILocation(line: 27, column: 12, scope: !9)
!22 = !DILocation(line: 27, column: 36, scope: !9)
!23 = !DILocation(line: 27, column: 48, scope: !9)
!24 = !DILocation(line: 28, column: 1, scope: !9)
!25 = distinct !DISubprogram(name: "search", linkageName: "search", scope: !1, file: !1, line: 34, type: !26, scopeLine: 34, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !28)
!26 = !DISubroutineType(types: !27)
!27 = !{!12, !7, !13}
!28 = !{!29, !30, !31, !32}
!29 = !DILocalVariable(name: "tree", scope: !25, file: !1, line: 35, type: !7)
!30 = !DILocalVariable(name: "key", scope: !25, file: !1, line: 35, type: !13)
!31 = !DILocalVariable(name: "i", scope: !25, file: !1, line: 40, type: !13)
!32 = !DILocalVariable(name: "node", scope: !25, file: !1, line: 42, type: !19)
!33 = !DILocation(line: 34, column: 1, scope: !25)
!34 = !DILocation(line: 35, column: 12, scope: !25)
!35 = !DILocation(line: 35, column: 29, scope: !25)
!36 = !DILocation(line: 40, column: 1, scope: !25)
!37 = !DILocation(line: 42, column: 1, scope: !25)
!38 = distinct !DISubprogram(name: "print_tree", linkageName: "print_tree", scope: !1, file: !1, line: 52, type: !39, scopeLine: 52, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !41)
!39 = !DISubroutineType(types: !40)
!40 = !{null, !7}
!41 = !{!42, !43, !44}
!42 = !DILocalVariable(name: "tree", scope: !38, file: !1, line: 53, type: !7)
!43 = !DILocalVariable(name: "i", scope: !38, file: !1, line: 60, type: !13)
!44 = !DILocalVariable(name: "node", scope: !38, file: !1, line: 62, type: !19)
!45 = !DILocation(line: 52, column: 1, scope: !38)
!46 = !DILocation(line: 53, column: 16, scope: !38)
!47 = !DILocation(line: 60, column: 1, scope: !38)
!48 = !DILocation(line: 62, column: 1, scope: !38)
!49 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 68, type: !50, scopeLine: 68, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !52)
!50 = !DISubroutineType(types: !51)
!51 = !{!13}
!52 = !{!53, !54, !55, !56}
!53 = !DILocalVariable(name: "tree", scope: !49, file: !1, line: 70, type: !7)
!54 = !DILocalVariable(name: "result1", scope: !49, file: !1, line: 89, type: !12)
!55 = !DILocalVariable(name: "result2", scope: !49, file: !1, line: 92, type: !12)
!56 = !DILocalVariable(name: "result3", scope: !49, file: !1, line: 95, type: !12)
!57 = !DILocation(line: 68, column: 1, scope: !49)
!58 = !DILocation(line: 70, column: 5, scope: !49)
!59 = !DILocation(line: 89, column: 1, scope: !49)
!60 = !DILocation(line: 92, column: 1, scope: !49)
!61 = !DILocation(line: 95, column: 1, scope: !49)
