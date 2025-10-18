; ModuleID = 'VynModule'
source_filename = "VynModule"

%TreeNode = type { i64, ptr, i1, i1 }
%BinaryTree = type { %TreeNode, i1, i64, { ptr, i64, i64 }, i64 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@1 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@2 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@3 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@4 = private unnamed_addr constant [26 x i8] c"Tree contents (in-order):\00", align 1
@5 = private unnamed_addr constant [14 x i8] c"Tree is empty\00", align 1
@6 = private unnamed_addr constant [6 x i8] c"Key: \00", align 1
@7 = private unnamed_addr constant [10 x i8] c", Value: \00", align 1
@8 = private unnamed_addr constant [40 x i8] c"Binary Search Tree Example - Vyn v0.4.0\00", align 1
@9 = private unnamed_addr constant [43 x i8] c"==========================================\00", align 1
@10 = private unnamed_addr constant [20 x i8] c"Inserting values...\00", align 1
@11 = private unnamed_addr constant [6 x i8] c"fifty\00", align 1
@12 = private unnamed_addr constant [7 x i8] c"thirty\00", align 1
@13 = private unnamed_addr constant [8 x i8] c"seventy\00", align 1
@14 = private unnamed_addr constant [7 x i8] c"twenty\00", align 1
@15 = private unnamed_addr constant [6 x i8] c"forty\00", align 1
@16 = private unnamed_addr constant [6 x i8] c"sixty\00", align 1
@17 = private unnamed_addr constant [7 x i8] c"eighty\00", align 1
@18 = private unnamed_addr constant [12 x i8] c"Tree size: \00", align 1
@19 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@20 = private unnamed_addr constant [19 x i8] c"Search operations:\00", align 1
@21 = private unnamed_addr constant [16 x i8] c"Search for 40: \00", align 1
@22 = private unnamed_addr constant [16 x i8] c"Search for 99: \00", align 1
@23 = private unnamed_addr constant [16 x i8] c"Search for 20: \00", align 1
@24 = private unnamed_addr constant [16 x i8] c"Search for 70: \00", align 1
@25 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define %TreeNode @create_node(i64 %key, ptr %value) !dbg !4 {
entry:
  %value2 = alloca ptr, align 8
  %key1 = alloca i64, align 8
  store i64 %key, ptr %key1, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %key1, metadata !11, metadata !DIExpression()), !dbg !14
  store ptr %value, ptr %value2, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %value2, metadata !12, metadata !DIExpression()), !dbg !15
  %TreeNode_obj = alloca %TreeNode, align 8, !dbg !13
  %key3 = load i64, ptr %key1, align 4, !dbg !13
  %key_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 0, !dbg !13
  store i64 %key3, ptr %key_ptr, align 4, !dbg !13
  %value4 = load ptr, ptr %value2, align 8, !dbg !13
  %value_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 1, !dbg !13
  store ptr %value4, ptr %value_ptr, align 8, !dbg !13
  ret %TreeNode undef, !dbg !13
}

define %BinaryTree @new_tree() !dbg !16 {
entry:
  %TreeNode_obj = alloca %TreeNode, align 8, !dbg !20
  %key_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 0, !dbg !20
  store i64 0, ptr %key_ptr, align 4, !dbg !20
  %value_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 1, !dbg !20
  store ptr @0, ptr %value_ptr, align 8, !dbg !20
  %BinaryTree_obj = alloca %BinaryTree, align 8, !dbg !20
  ret %BinaryTree undef, !dbg !20
}

define void @insert(ptr %tree, i64 %key, ptr %value) !dbg !21 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca ptr, align 8
  store ptr %tree, ptr %tree1, align 8, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !25, metadata !DIExpression()), !dbg !29
  store i64 %key, ptr %key2, align 4, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !26, metadata !DIExpression()), !dbg !30
  store ptr %value, ptr %value3, align 8, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !27, metadata !DIExpression()), !dbg !31
  %tree4 = load ptr, ptr %tree1, align 8, !dbg !28
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %tree4, i32 0, i32 1, !dbg !28
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !28
  %nottmp = xor i1 %has_root_val, true, !dbg !28
  br i1 %nottmp, label %then, label %ifcont, !dbg !28

then:                                             ; preds = %entry
  %tree5 = load ptr, ptr %tree1, align 8, !dbg !28
  %key6 = load i64, ptr %key2, align 4, !dbg !28
  %value7 = load ptr, ptr %value3, align 8, !dbg !28
  %calltmp = call %TreeNode @create_node(i64 %key6, ptr %value7), !dbg !28
  %malloc_struct = call ptr @malloc(i64 24), !dbg !28
  store %TreeNode %calltmp, ptr %malloc_struct, align 8, !dbg !28
  %tree8 = load ptr, ptr %tree1, align 8, !dbg !28
  %has_root_ptr9 = getelementptr inbounds %BinaryTree, ptr %tree8, i32 0, i32 1, !dbg !28
  store i1 true, ptr %has_root_ptr9, align 1, !dbg !28
  %tree10 = load ptr, ptr %tree1, align 8, !dbg !28
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %tree10, i32 0, i32 2, !dbg !28
  %tree11 = load ptr, ptr %tree1, align 8, !dbg !28
  %size_ptr12 = getelementptr inbounds %BinaryTree, ptr %tree11, i32 0, i32 2, !dbg !28
  %size_val = load i64, ptr %size_ptr12, align 4, !dbg !28
  %addtmp = add i64 %size_val, 1, !dbg !28
  store i64 %addtmp, ptr %size_ptr, align 4, !dbg !28
  ret void, !dbg !28

ifcont:                                           ; preds = %entry
  %tree13 = load ptr, ptr %tree1, align 8, !dbg !28
  %tree14 = load ptr, ptr %tree1, align 8, !dbg !28
  %size_ptr15 = getelementptr inbounds %BinaryTree, ptr %tree14, i32 0, i32 2, !dbg !28
  %tree16 = load ptr, ptr %tree1, align 8, !dbg !28
  %size_ptr17 = getelementptr inbounds %BinaryTree, ptr %tree16, i32 0, i32 2, !dbg !28
  %size_val18 = load i64, ptr %size_ptr17, align 4, !dbg !28
  %addtmp19 = add i64 %size_val18, 1, !dbg !28
  store i64 %addtmp19, ptr %size_ptr15, align 4, !dbg !28
  ret void, !dbg !28
}

define void @insert_recursive(ptr %node, i64 %key, ptr %value) !dbg !32 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !34, metadata !DIExpression()), !dbg !38
  store i64 %key, ptr %key2, align 4, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !35, metadata !DIExpression()), !dbg !39
  store ptr %value, ptr %value3, align 8, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !36, metadata !DIExpression()), !dbg !40
  %key4 = load i64, ptr %key2, align 4, !dbg !37
  %node5 = load ptr, ptr %node1, align 8, !dbg !37
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node5, i32 0, i32 0, !dbg !37
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !37
  %icmpslttmp = icmp slt i64 %key4, %key_val, !dbg !37
  br i1 %icmpslttmp, label %then, label %else, !dbg !37

then:                                             ; preds = %entry
  %node6 = load ptr, ptr %node1, align 8, !dbg !37
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node6, i32 0, i32 2, !dbg !37
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !37
  %nottmp = xor i1 %has_left_val, true, !dbg !37
  br i1 %nottmp, label %then7, label %else8, !dbg !37

else:                                             ; preds = %entry
  %node15 = load ptr, ptr %node1, align 8, !dbg !37
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node15, i32 0, i32 3, !dbg !37
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !37
  %nottmp16 = xor i1 %has_right_val, true, !dbg !37
  br i1 %nottmp16, label %then17, label %else18, !dbg !37

then7:                                            ; preds = %then
  %node9 = load ptr, ptr %node1, align 8, !dbg !37
  %key10 = load i64, ptr %key2, align 4, !dbg !37
  %value11 = load ptr, ptr %value3, align 8, !dbg !37
  %calltmp = call %TreeNode @create_node(i64 %key10, ptr %value11), !dbg !37
  %malloc_struct = call ptr @malloc(i64 24), !dbg !37
  store %TreeNode %calltmp, ptr %malloc_struct, align 8, !dbg !37
  %node12 = load ptr, ptr %node1, align 8, !dbg !37
  %has_left_ptr13 = getelementptr inbounds %TreeNode, ptr %node12, i32 0, i32 2, !dbg !37
  store i1 true, ptr %has_left_ptr13, align 1, !dbg !37
  br label %ifcont, !dbg !37

else8:                                            ; preds = %then
  %node14 = load ptr, ptr %node1, align 8, !dbg !37
  br label %ifcont, !dbg !37

ifcont:                                           ; preds = %else8, %then7
  br label %ifcont28, !dbg !37

then17:                                           ; preds = %else
  %node19 = load ptr, ptr %node1, align 8, !dbg !37
  %key20 = load i64, ptr %key2, align 4, !dbg !37
  %value21 = load ptr, ptr %value3, align 8, !dbg !37
  %calltmp22 = call %TreeNode @create_node(i64 %key20, ptr %value21), !dbg !37
  %malloc_struct23 = call ptr @malloc(i64 24), !dbg !37
  store %TreeNode %calltmp22, ptr %malloc_struct23, align 8, !dbg !37
  %node24 = load ptr, ptr %node1, align 8, !dbg !37
  %has_right_ptr25 = getelementptr inbounds %TreeNode, ptr %node24, i32 0, i32 3, !dbg !37
  store i1 true, ptr %has_right_ptr25, align 1, !dbg !37
  br label %ifcont27, !dbg !37

else18:                                           ; preds = %else
  %node26 = load ptr, ptr %node1, align 8, !dbg !37
  br label %ifcont27, !dbg !37

ifcont27:                                         ; preds = %else18, %then17
  br label %ifcont28, !dbg !37

ifcont28:                                         ; preds = %ifcont27, %ifcont
  ret void, !dbg !37
}

define ptr @search(%BinaryTree %tree, i64 %key) !dbg !41 {
entry:
  %key2 = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !47
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !45, metadata !DIExpression()), !dbg !48
  store i64 %key, ptr %key2, align 4, !dbg !47
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !46, metadata !DIExpression()), !dbg !49
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !47
  %temp_struct = alloca %BinaryTree, align 8, !dbg !47
  store %BinaryTree %tree3, ptr %temp_struct, align 8, !dbg !47
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !47
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !47
  %nottmp = xor i1 %has_root_val, true, !dbg !47
  br i1 %nottmp, label %then, label %ifcont, !dbg !47

then:                                             ; preds = %entry
  ret ptr @1, !dbg !47

ifcont:                                           ; preds = %entry
  %tree4 = load %BinaryTree, ptr %tree1, align 8, !dbg !47
  %temp_struct5 = alloca %BinaryTree, align 8, !dbg !47
  store %BinaryTree %tree4, ptr %temp_struct5, align 8, !dbg !47
  ret ptr undef, !dbg !47
}

define ptr @search_recursive(ptr %node, i64 %key) !dbg !50 {
entry:
  %key2 = alloca i64, align 8
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !56
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !54, metadata !DIExpression()), !dbg !57
  store i64 %key, ptr %key2, align 4, !dbg !56
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !55, metadata !DIExpression()), !dbg !58
  %node3 = load ptr, ptr %node1, align 8, !dbg !56
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node3, i32 0, i32 0, !dbg !56
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !56
  %key4 = load i64, ptr %key2, align 4, !dbg !56
  %icmpeqtmp = icmp eq i64 %key_val, %key4, !dbg !56
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !56

then:                                             ; preds = %entry
  %node5 = load ptr, ptr %node1, align 8, !dbg !56
  %value_ptr = getelementptr inbounds %TreeNode, ptr %node5, i32 0, i32 1, !dbg !56
  ret ptr %value_ptr, !dbg !56

ifcont:                                           ; preds = %entry
  %key6 = load i64, ptr %key2, align 4, !dbg !56
  %node7 = load ptr, ptr %node1, align 8, !dbg !56
  %key_ptr8 = getelementptr inbounds %TreeNode, ptr %node7, i32 0, i32 0, !dbg !56
  %key_val9 = load i64, ptr %key_ptr8, align 4, !dbg !56
  %icmpslttmp = icmp slt i64 %key6, %key_val9, !dbg !56
  br i1 %icmpslttmp, label %then10, label %else, !dbg !56

then10:                                           ; preds = %ifcont
  %node11 = load ptr, ptr %node1, align 8, !dbg !56
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node11, i32 0, i32 2, !dbg !56
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !56
  br i1 %has_left_val, label %then12, label %else13, !dbg !56

else:                                             ; preds = %ifcont
  %node15 = load ptr, ptr %node1, align 8, !dbg !56
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node15, i32 0, i32 3, !dbg !56
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !56
  br i1 %has_right_val, label %then16, label %else17, !dbg !56

then12:                                           ; preds = %then10
  %node14 = load ptr, ptr %node1, align 8, !dbg !56
  ret ptr undef, !dbg !56

else13:                                           ; preds = %then10
  ret ptr @2, !dbg !56

then16:                                           ; preds = %else
  %node18 = load ptr, ptr %node1, align 8, !dbg !56
  ret ptr undef, !dbg !56

else17:                                           ; preds = %else
  ret ptr @3, !dbg !56
}

define void @print_tree(%BinaryTree %tree) !dbg !59 {
entry:
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !64
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !63, metadata !DIExpression()), !dbg !65
  %tree2 = load %BinaryTree, ptr %tree1, align 8, !dbg !64
  %temp_struct = alloca %BinaryTree, align 8, !dbg !64
  store %BinaryTree %tree2, ptr %temp_struct, align 8, !dbg !64
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !64
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !64
  br i1 %has_root_val, label %then, label %else, !dbg !64

then:                                             ; preds = %entry
  call void @__vyn_println(ptr @4), !dbg !64
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !64
  %temp_struct4 = alloca %BinaryTree, align 8, !dbg !64
  store %BinaryTree %tree3, ptr %temp_struct4, align 8, !dbg !64
  br label %ifcont, !dbg !64

else:                                             ; preds = %entry
  call void @__vyn_println(ptr @5), !dbg !64
  br label %ifcont, !dbg !64

ifcont:                                           ; preds = %else, %then
  ret void, !dbg !64
}

define void @print_in_order(ptr %node) !dbg !66 {
entry:
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !71
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !70, metadata !DIExpression()), !dbg !72
  %node2 = load ptr, ptr %node1, align 8, !dbg !71
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node2, i32 0, i32 2, !dbg !71
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !71
  br i1 %has_left_val, label %then, label %ifcont, !dbg !71

then:                                             ; preds = %entry
  %node3 = load ptr, ptr %node1, align 8, !dbg !71
  br label %ifcont, !dbg !71

ifcont:                                           ; preds = %then, %entry
  %node4 = load ptr, ptr %node1, align 8, !dbg !71
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node4, i32 0, i32 0, !dbg !71
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !71
  %tostring = call ptr @__vyn_toString_int(i64 %key_val), !dbg !71
  %strcattmp = call ptr @__vyn_string_concat(ptr @6, ptr %tostring), !dbg !71
  %strcattmp5 = call ptr @__vyn_string_concat(ptr %strcattmp, ptr @7), !dbg !71
  %node6 = load ptr, ptr %node1, align 8, !dbg !71
  %value_ptr = getelementptr inbounds %TreeNode, ptr %node6, i32 0, i32 1, !dbg !71
  %strcattmp7 = call ptr @__vyn_string_concat(ptr %strcattmp5, ptr %value_ptr), !dbg !71
  call void @__vyn_println(ptr %strcattmp7), !dbg !71
  %node8 = load ptr, ptr %node1, align 8, !dbg !71
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node8, i32 0, i32 3, !dbg !71
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !71
  br i1 %has_right_val, label %then9, label %ifcont11, !dbg !71

then9:                                            ; preds = %ifcont
  %node10 = load ptr, ptr %node1, align 8, !dbg !71
  br label %ifcont11, !dbg !71

ifcont11:                                         ; preds = %then9, %ifcont
  ret void, !dbg !71
}

define i64 @main() !dbg !73 {
entry:
  %result4 = alloca ptr, align 8, !dbg !82
  %result3 = alloca ptr, align 8, !dbg !82
  %result2 = alloca ptr, align 8, !dbg !82
  %result1 = alloca ptr, align 8, !dbg !82
  %tree = alloca %BinaryTree, align 8, !dbg !82
  %calltmp = call %BinaryTree @new_tree(), !dbg !82
  store %BinaryTree %calltmp, ptr %tree, align 8, !dbg !82
  call void @llvm.dbg.declare(metadata ptr %tree, metadata !77, metadata !DIExpression()), !dbg !83
  call void @__vyn_println(ptr @8), !dbg !82
  call void @__vyn_println(ptr @9), !dbg !82
  call void @__vyn_println(ptr @10), !dbg !82
  call void @insert(ptr %tree, i64 50, ptr @11), !dbg !82
  call void @insert(ptr %tree, i64 30, ptr @12), !dbg !82
  call void @insert(ptr %tree, i64 70, ptr @13), !dbg !82
  call void @insert(ptr %tree, i64 20, ptr @14), !dbg !82
  call void @insert(ptr %tree, i64 40, ptr @15), !dbg !82
  call void @insert(ptr %tree, i64 60, ptr @16), !dbg !82
  call void @insert(ptr %tree, i64 80, ptr @17), !dbg !82
  %tree1 = load %BinaryTree, ptr %tree, align 8, !dbg !82
  %temp_struct = alloca %BinaryTree, align 8, !dbg !82
  store %BinaryTree %tree1, ptr %temp_struct, align 8, !dbg !82
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 2, !dbg !82
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !82
  %tostring = call ptr @__vyn_toString_int(i64 %size_val), !dbg !82
  %strcattmp = call ptr @__vyn_string_concat(ptr @18, ptr %tostring), !dbg !82
  call void @__vyn_println(ptr %strcattmp), !dbg !82
  call void @__vyn_println(ptr @19), !dbg !82
  call void @__vyn_println(ptr @20), !dbg !82
  %tree2 = load %BinaryTree, ptr %tree, align 8, !dbg !82
  %calltmp3 = call ptr @search(%BinaryTree %tree2, i64 40), !dbg !82
  store ptr %calltmp3, ptr %result1, align 8, !dbg !82
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !78, metadata !DIExpression()), !dbg !84
  %result14 = load ptr, ptr %result1, align 8, !dbg !82
  %strcattmp5 = call ptr @__vyn_string_concat(ptr @21, ptr %result14), !dbg !82
  call void @__vyn_println(ptr %strcattmp5), !dbg !82
  %tree6 = load %BinaryTree, ptr %tree, align 8, !dbg !82
  %calltmp7 = call ptr @search(%BinaryTree %tree6, i64 99), !dbg !82
  store ptr %calltmp7, ptr %result2, align 8, !dbg !82
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !79, metadata !DIExpression()), !dbg !85
  %result28 = load ptr, ptr %result2, align 8, !dbg !82
  %strcattmp9 = call ptr @__vyn_string_concat(ptr @22, ptr %result28), !dbg !82
  call void @__vyn_println(ptr %strcattmp9), !dbg !82
  %tree10 = load %BinaryTree, ptr %tree, align 8, !dbg !82
  %calltmp11 = call ptr @search(%BinaryTree %tree10, i64 20), !dbg !82
  store ptr %calltmp11, ptr %result3, align 8, !dbg !82
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !80, metadata !DIExpression()), !dbg !86
  %result312 = load ptr, ptr %result3, align 8, !dbg !82
  %strcattmp13 = call ptr @__vyn_string_concat(ptr @23, ptr %result312), !dbg !82
  call void @__vyn_println(ptr %strcattmp13), !dbg !82
  %tree14 = load %BinaryTree, ptr %tree, align 8, !dbg !82
  %calltmp15 = call ptr @search(%BinaryTree %tree14, i64 70), !dbg !82
  store ptr %calltmp15, ptr %result4, align 8, !dbg !82
  call void @llvm.dbg.declare(metadata ptr %result4, metadata !81, metadata !DIExpression()), !dbg !87
  %result416 = load ptr, ptr %result4, align 8, !dbg !82
  %strcattmp17 = call ptr @__vyn_string_concat(ptr @24, ptr %result416), !dbg !82
  call void @__vyn_println(ptr %strcattmp17), !dbg !82
  call void @__vyn_println(ptr @25), !dbg !82
  %tree18 = load %BinaryTree, ptr %tree, align 8, !dbg !82
  call void @print_tree(%BinaryTree %tree18), !dbg !82
  ret i64 0, !dbg !82
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare void @__vyn_println(ptr)

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "binary_tree.vyn.ll", directory: "/home/rick/Projects/Vyn/examples")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "create_node", linkageName: "create_node", scope: !1, file: !1, line: 21, type: !5, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !10)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8, !9}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "TreeNode", scope: !1, file: !1, size: 256, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!10 = !{!11, !12}
!11 = !DILocalVariable(name: "key", scope: !4, file: !1, line: 22, type: !8)
!12 = !DILocalVariable(name: "value", scope: !4, file: !1, line: 22, type: !9)
!13 = !DILocation(line: 21, column: 1, scope: !4)
!14 = !DILocation(line: 22, column: 16, scope: !4)
!15 = !DILocation(line: 22, column: 28, scope: !4)
!16 = distinct !DISubprogram(name: "new_tree", linkageName: "new_tree", scope: !1, file: !1, line: 33, type: !17, scopeLine: 33, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!17 = !DISubroutineType(types: !18)
!18 = !{!19}
!19 = !DICompositeType(tag: DW_TAG_structure_type, name: "BinaryTree", scope: !1, file: !1, size: 320, align: 8)
!20 = !DILocation(line: 33, column: 1, scope: !16)
!21 = distinct !DISubprogram(name: "insert", linkageName: "insert", scope: !1, file: !1, line: 43, type: !22, scopeLine: 43, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !24)
!22 = !DISubroutineType(types: !23)
!23 = !{null, !9, !8, !9}
!24 = !{!25, !26, !27}
!25 = !DILocalVariable(name: "tree", scope: !21, file: !1, line: 44, type: !9)
!26 = !DILocalVariable(name: "key", scope: !21, file: !1, line: 44, type: !8)
!27 = !DILocalVariable(name: "value", scope: !21, file: !1, line: 44, type: !9)
!28 = !DILocation(line: 43, column: 1, scope: !21)
!29 = !DILocation(line: 44, column: 12, scope: !21)
!30 = !DILocation(line: 44, column: 36, scope: !21)
!31 = !DILocation(line: 44, column: 48, scope: !21)
!32 = distinct !DISubprogram(name: "insert_recursive", linkageName: "insert_recursive", scope: !1, file: !1, line: 56, type: !22, scopeLine: 56, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!33 = !{!34, !35, !36}
!34 = !DILocalVariable(name: "node", scope: !32, file: !1, line: 57, type: !9)
!35 = !DILocalVariable(name: "key", scope: !32, file: !1, line: 57, type: !8)
!36 = !DILocalVariable(name: "value", scope: !32, file: !1, line: 57, type: !9)
!37 = !DILocation(line: 56, column: 1, scope: !32)
!38 = !DILocation(line: 57, column: 22, scope: !32)
!39 = !DILocation(line: 57, column: 44, scope: !32)
!40 = !DILocation(line: 57, column: 56, scope: !32)
!41 = distinct !DISubprogram(name: "search", linkageName: "search", scope: !1, file: !1, line: 75, type: !42, scopeLine: 75, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !44)
!42 = !DISubroutineType(types: !43)
!43 = !{!9, !19, !8}
!44 = !{!45, !46}
!45 = !DILocalVariable(name: "tree", scope: !41, file: !1, line: 76, type: !19)
!46 = !DILocalVariable(name: "key", scope: !41, file: !1, line: 76, type: !8)
!47 = !DILocation(line: 75, column: 1, scope: !41)
!48 = !DILocation(line: 76, column: 12, scope: !41)
!49 = !DILocation(line: 76, column: 29, scope: !41)
!50 = distinct !DISubprogram(name: "search_recursive", linkageName: "search_recursive", scope: !1, file: !1, line: 84, type: !51, scopeLine: 84, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !53)
!51 = !DISubroutineType(types: !52)
!52 = !{!9, !9, !8}
!53 = !{!54, !55}
!54 = !DILocalVariable(name: "node", scope: !50, file: !1, line: 85, type: !9)
!55 = !DILocalVariable(name: "key", scope: !50, file: !1, line: 85, type: !8)
!56 = !DILocation(line: 84, column: 1, scope: !50)
!57 = !DILocation(line: 85, column: 22, scope: !50)
!58 = !DILocation(line: 85, column: 50, scope: !50)
!59 = distinct !DISubprogram(name: "print_tree", linkageName: "print_tree", scope: !1, file: !1, line: 105, type: !60, scopeLine: 105, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !62)
!60 = !DISubroutineType(types: !61)
!61 = !{null, !19}
!62 = !{!63}
!63 = !DILocalVariable(name: "tree", scope: !59, file: !1, line: 106, type: !19)
!64 = !DILocation(line: 105, column: 1, scope: !59)
!65 = !DILocation(line: 106, column: 16, scope: !59)
!66 = distinct !DISubprogram(name: "print_in_order", linkageName: "print_in_order", scope: !1, file: !1, line: 115, type: !67, scopeLine: 115, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !69)
!67 = !DISubroutineType(types: !68)
!68 = !{null, !9}
!69 = !{!70}
!70 = !DILocalVariable(name: "node", scope: !66, file: !1, line: 116, type: !9)
!71 = !DILocation(line: 115, column: 1, scope: !66)
!72 = !DILocation(line: 116, column: 20, scope: !66)
!73 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 128, type: !74, scopeLine: 128, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !76)
!74 = !DISubroutineType(types: !75)
!75 = !{!8}
!76 = !{!77, !78, !79, !80, !81}
!77 = !DILocalVariable(name: "tree", scope: !73, file: !1, line: 130, type: !19)
!78 = !DILocalVariable(name: "result1", scope: !73, file: !1, line: 151, type: !9)
!79 = !DILocalVariable(name: "result2", scope: !73, file: !1, line: 154, type: !9)
!80 = !DILocalVariable(name: "result3", scope: !73, file: !1, line: 157, type: !9)
!81 = !DILocalVariable(name: "result4", scope: !73, file: !1, line: 160, type: !9)
!82 = !DILocation(line: 128, column: 1, scope: !73)
!83 = !DILocation(line: 130, column: 5, scope: !73)
!84 = !DILocation(line: 151, column: 1, scope: !73)
!85 = !DILocation(line: 154, column: 1, scope: !73)
!86 = !DILocation(line: 157, column: 1, scope: !73)
!87 = !DILocation(line: 160, column: 1, scope: !73)
