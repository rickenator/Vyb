; ModuleID = 'VynModule'
source_filename = "VynModule"

%TreeNode = type { i64, ptr, ptr, ptr, i1, i1 }
%BinaryTree = type { ptr, i1, i64 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@1 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@2 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@3 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@4 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@5 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@6 = private unnamed_addr constant [26 x i8] c"Tree contents (in-order):\00", align 1
@7 = private unnamed_addr constant [14 x i8] c"Tree is empty\00", align 1
@8 = private unnamed_addr constant [6 x i8] c"Key: \00", align 1
@9 = private unnamed_addr constant [10 x i8] c", Value: \00", align 1
@10 = private unnamed_addr constant [40 x i8] c"Binary Search Tree Example - Vyn v0.4.0\00", align 1
@11 = private unnamed_addr constant [43 x i8] c"==========================================\00", align 1
@12 = private unnamed_addr constant [20 x i8] c"Inserting values...\00", align 1
@13 = private unnamed_addr constant [6 x i8] c"fifty\00", align 1
@14 = private unnamed_addr constant [7 x i8] c"thirty\00", align 1
@15 = private unnamed_addr constant [8 x i8] c"seventy\00", align 1
@16 = private unnamed_addr constant [7 x i8] c"twenty\00", align 1
@17 = private unnamed_addr constant [6 x i8] c"forty\00", align 1
@18 = private unnamed_addr constant [6 x i8] c"sixty\00", align 1
@19 = private unnamed_addr constant [7 x i8] c"eighty\00", align 1
@20 = private unnamed_addr constant [12 x i8] c"Tree size: \00", align 1
@21 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@22 = private unnamed_addr constant [19 x i8] c"Search operations:\00", align 1
@23 = private unnamed_addr constant [16 x i8] c"Search for 40: \00", align 1
@24 = private unnamed_addr constant [16 x i8] c"Search for 99: \00", align 1
@25 = private unnamed_addr constant [16 x i8] c"Search for 20: \00", align 1
@26 = private unnamed_addr constant [16 x i8] c"Search for 70: \00", align 1
@27 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

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
  %TreeNode_obj5 = alloca %TreeNode, align 8, !dbg !13
  %key_ptr6 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj5, i32 0, i32 0, !dbg !13
  store i64 0, ptr %key_ptr6, align 4, !dbg !13
  %value_ptr7 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj5, i32 0, i32 1, !dbg !13
  store ptr @0, ptr %value_ptr7, align 8, !dbg !13
  %TreeNode_obj8 = alloca %TreeNode, align 8, !dbg !13
  %TreeNode_val = load %TreeNode, ptr %TreeNode_obj8, align 8, !dbg !13
  %malloc_struct = call ptr @malloc(i64 40), !dbg !13
  store %TreeNode %TreeNode_val, ptr %malloc_struct, align 8, !dbg !13
  %left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj5, i32 0, i32 2, !dbg !13
  store ptr %malloc_struct, ptr %left_ptr, align 8, !dbg !13
  %TreeNode_obj9 = alloca %TreeNode, align 8, !dbg !13
  %TreeNode_val10 = load %TreeNode, ptr %TreeNode_obj9, align 8, !dbg !13
  %malloc_struct11 = call ptr @malloc(i64 40), !dbg !13
  store %TreeNode %TreeNode_val10, ptr %malloc_struct11, align 8, !dbg !13
  %right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj5, i32 0, i32 3, !dbg !13
  store ptr %malloc_struct11, ptr %right_ptr, align 8, !dbg !13
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj5, i32 0, i32 4, !dbg !13
  store i1 false, ptr %has_left_ptr, align 1, !dbg !13
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj5, i32 0, i32 5, !dbg !13
  store i1 false, ptr %has_right_ptr, align 1, !dbg !13
  %TreeNode_val12 = load %TreeNode, ptr %TreeNode_obj5, align 8, !dbg !13
  %malloc_struct13 = call ptr @malloc(i64 40), !dbg !13
  store %TreeNode %TreeNode_val12, ptr %malloc_struct13, align 8, !dbg !13
  %left_ptr14 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 2, !dbg !13
  store ptr %malloc_struct13, ptr %left_ptr14, align 8, !dbg !13
  %TreeNode_obj15 = alloca %TreeNode, align 8, !dbg !13
  %key_ptr16 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj15, i32 0, i32 0, !dbg !13
  store i64 0, ptr %key_ptr16, align 4, !dbg !13
  %value_ptr17 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj15, i32 0, i32 1, !dbg !13
  store ptr @1, ptr %value_ptr17, align 8, !dbg !13
  %TreeNode_obj18 = alloca %TreeNode, align 8, !dbg !13
  %TreeNode_val19 = load %TreeNode, ptr %TreeNode_obj18, align 8, !dbg !13
  %malloc_struct20 = call ptr @malloc(i64 40), !dbg !13
  store %TreeNode %TreeNode_val19, ptr %malloc_struct20, align 8, !dbg !13
  %left_ptr21 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj15, i32 0, i32 2, !dbg !13
  store ptr %malloc_struct20, ptr %left_ptr21, align 8, !dbg !13
  %TreeNode_obj22 = alloca %TreeNode, align 8, !dbg !13
  %TreeNode_val23 = load %TreeNode, ptr %TreeNode_obj22, align 8, !dbg !13
  %malloc_struct24 = call ptr @malloc(i64 40), !dbg !13
  store %TreeNode %TreeNode_val23, ptr %malloc_struct24, align 8, !dbg !13
  %right_ptr25 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj15, i32 0, i32 3, !dbg !13
  store ptr %malloc_struct24, ptr %right_ptr25, align 8, !dbg !13
  %has_left_ptr26 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj15, i32 0, i32 4, !dbg !13
  store i1 false, ptr %has_left_ptr26, align 1, !dbg !13
  %has_right_ptr27 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj15, i32 0, i32 5, !dbg !13
  store i1 false, ptr %has_right_ptr27, align 1, !dbg !13
  %TreeNode_val28 = load %TreeNode, ptr %TreeNode_obj15, align 8, !dbg !13
  %malloc_struct29 = call ptr @malloc(i64 40), !dbg !13
  store %TreeNode %TreeNode_val28, ptr %malloc_struct29, align 8, !dbg !13
  %right_ptr30 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 3, !dbg !13
  store ptr %malloc_struct29, ptr %right_ptr30, align 8, !dbg !13
  %has_left_ptr31 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 4, !dbg !13
  store i1 false, ptr %has_left_ptr31, align 1, !dbg !13
  %has_right_ptr32 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 5, !dbg !13
  store i1 false, ptr %has_right_ptr32, align 1, !dbg !13
  %TreeNode_val33 = load %TreeNode, ptr %TreeNode_obj, align 8, !dbg !13
  ret %TreeNode %TreeNode_val33, !dbg !13
}

define %BinaryTree @new_tree() !dbg !16 {
entry:
  %dummy_node = alloca %TreeNode, align 8, !dbg !22
  %TreeNode_obj = alloca %TreeNode, align 8, !dbg !22
  %key_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 0, !dbg !22
  store i64 0, ptr %key_ptr, align 4, !dbg !22
  %value_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 1, !dbg !22
  store ptr @2, ptr %value_ptr, align 8, !dbg !22
  %TreeNode_obj1 = alloca %TreeNode, align 8, !dbg !22
  %TreeNode_val = load %TreeNode, ptr %TreeNode_obj1, align 8, !dbg !22
  %malloc_struct = call ptr @malloc(i64 40), !dbg !22
  store %TreeNode %TreeNode_val, ptr %malloc_struct, align 8, !dbg !22
  %left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 2, !dbg !22
  store ptr %malloc_struct, ptr %left_ptr, align 8, !dbg !22
  %TreeNode_obj2 = alloca %TreeNode, align 8, !dbg !22
  %TreeNode_val3 = load %TreeNode, ptr %TreeNode_obj2, align 8, !dbg !22
  %malloc_struct4 = call ptr @malloc(i64 40), !dbg !22
  store %TreeNode %TreeNode_val3, ptr %malloc_struct4, align 8, !dbg !22
  %right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 3, !dbg !22
  store ptr %malloc_struct4, ptr %right_ptr, align 8, !dbg !22
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 4, !dbg !22
  store i1 false, ptr %has_left_ptr, align 1, !dbg !22
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 5, !dbg !22
  store i1 false, ptr %has_right_ptr, align 1, !dbg !22
  %TreeNode_val5 = load %TreeNode, ptr %TreeNode_obj, align 8, !dbg !22
  store %TreeNode %TreeNode_val5, ptr %dummy_node, align 8, !dbg !22
  call void @llvm.dbg.declare(metadata ptr %dummy_node, metadata !21, metadata !DIExpression()), !dbg !23
  %BinaryTree_obj = alloca %BinaryTree, align 8, !dbg !22
  %dummy_node6 = load %TreeNode, ptr %dummy_node, align 8, !dbg !22
  %malloc_struct7 = call ptr @malloc(i64 40), !dbg !22
  store %TreeNode %dummy_node6, ptr %malloc_struct7, align 8, !dbg !22
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 0, !dbg !22
  store ptr %malloc_struct7, ptr %root_ptr, align 8, !dbg !22
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 1, !dbg !22
  store i1 false, ptr %has_root_ptr, align 1, !dbg !22
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 2, !dbg !22
  store i64 0, ptr %size_ptr, align 4, !dbg !22
  %BinaryTree_val = load %BinaryTree, ptr %BinaryTree_obj, align 8, !dbg !22
  ret %BinaryTree %BinaryTree_val, !dbg !22
}

define void @insert(%BinaryTree %tree, i64 %key, ptr %value) !dbg !24 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !28, metadata !DIExpression()), !dbg !32
  store i64 %key, ptr %key2, align 4, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !29, metadata !DIExpression()), !dbg !33
  store ptr %value, ptr %value3, align 8, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !30, metadata !DIExpression()), !dbg !34
  %tree4 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree4, ptr %temp_struct, align 8, !dbg !31
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !31
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !31
  %nottmp = xor i1 %has_root_val, true, !dbg !31
  br i1 %nottmp, label %then, label %ifcont, !dbg !31

then:                                             ; preds = %entry
  %tree5 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct6 = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree5, ptr %temp_struct6, align 8, !dbg !31
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct6, i32 0, i32 0, !dbg !31
  %key7 = load i64, ptr %key2, align 4, !dbg !31
  %value8 = load ptr, ptr %value3, align 8, !dbg !31
  %calltmp = call %TreeNode @create_node(i64 %key7, ptr %value8), !dbg !31
  %malloc_struct = call ptr @malloc(i64 40), !dbg !31
  store %TreeNode %calltmp, ptr %malloc_struct, align 8, !dbg !31
  store ptr %malloc_struct, ptr %root_ptr, align 8, !dbg !31
  %tree9 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct10 = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree9, ptr %temp_struct10, align 8, !dbg !31
  %has_root_ptr11 = getelementptr inbounds %BinaryTree, ptr %temp_struct10, i32 0, i32 1, !dbg !31
  store i1 true, ptr %has_root_ptr11, align 1, !dbg !31
  %tree12 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct13 = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree12, ptr %temp_struct13, align 8, !dbg !31
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct13, i32 0, i32 2, !dbg !31
  %tree14 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct15 = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree14, ptr %temp_struct15, align 8, !dbg !31
  %size_ptr16 = getelementptr inbounds %BinaryTree, ptr %temp_struct15, i32 0, i32 2, !dbg !31
  %size_val = load i64, ptr %size_ptr16, align 4, !dbg !31
  %addtmp = add i64 %size_val, 1, !dbg !31
  store i64 %addtmp, ptr %size_ptr, align 4, !dbg !31
  ret void, !dbg !31

ifcont:                                           ; preds = %entry
  %tree17 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct18 = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree17, ptr %temp_struct18, align 8, !dbg !31
  %root_ptr19 = getelementptr inbounds %BinaryTree, ptr %temp_struct18, i32 0, i32 0, !dbg !31
  %key20 = load i64, ptr %key2, align 4, !dbg !31
  %value21 = load ptr, ptr %value3, align 8, !dbg !31
  call void @insert_recursive(ptr %root_ptr19, i64 %key20, ptr %value21), !dbg !31
  %tree22 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct23 = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree22, ptr %temp_struct23, align 8, !dbg !31
  %size_ptr24 = getelementptr inbounds %BinaryTree, ptr %temp_struct23, i32 0, i32 2, !dbg !31
  %tree25 = load %BinaryTree, ptr %tree1, align 8, !dbg !31
  %temp_struct26 = alloca %BinaryTree, align 8, !dbg !31
  store %BinaryTree %tree25, ptr %temp_struct26, align 8, !dbg !31
  %size_ptr27 = getelementptr inbounds %BinaryTree, ptr %temp_struct26, i32 0, i32 2, !dbg !31
  %size_val28 = load i64, ptr %size_ptr27, align 4, !dbg !31
  %addtmp29 = add i64 %size_val28, 1, !dbg !31
  store i64 %addtmp29, ptr %size_ptr24, align 4, !dbg !31
  ret void, !dbg !31
}

define void @insert_recursive(ptr %node, i64 %key, ptr %value) !dbg !35 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !39, metadata !DIExpression()), !dbg !43
  store i64 %key, ptr %key2, align 4, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !40, metadata !DIExpression()), !dbg !44
  store ptr %value, ptr %value3, align 8, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !41, metadata !DIExpression()), !dbg !45
  %key4 = load i64, ptr %key2, align 4, !dbg !42
  %node5 = load ptr, ptr %node1, align 8, !dbg !42
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node5, i32 0, i32 0, !dbg !42
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !42
  %icmpslttmp = icmp slt i64 %key4, %key_val, !dbg !42
  br i1 %icmpslttmp, label %then, label %else, !dbg !42

then:                                             ; preds = %entry
  %node6 = load ptr, ptr %node1, align 8, !dbg !42
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node6, i32 0, i32 4, !dbg !42
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !42
  %nottmp = xor i1 %has_left_val, true, !dbg !42
  br i1 %nottmp, label %then7, label %else8, !dbg !42

else:                                             ; preds = %entry
  %node18 = load ptr, ptr %node1, align 8, !dbg !42
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node18, i32 0, i32 5, !dbg !42
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !42
  %nottmp19 = xor i1 %has_right_val, true, !dbg !42
  br i1 %nottmp19, label %then20, label %else21, !dbg !42

then7:                                            ; preds = %then
  %node9 = load ptr, ptr %node1, align 8, !dbg !42
  %left_ptr = getelementptr inbounds %TreeNode, ptr %node9, i32 0, i32 2, !dbg !42
  %key10 = load i64, ptr %key2, align 4, !dbg !42
  %value11 = load ptr, ptr %value3, align 8, !dbg !42
  %calltmp = call %TreeNode @create_node(i64 %key10, ptr %value11), !dbg !42
  %malloc_struct = call ptr @malloc(i64 40), !dbg !42
  store %TreeNode %calltmp, ptr %malloc_struct, align 8, !dbg !42
  store ptr %malloc_struct, ptr %left_ptr, align 8, !dbg !42
  %node12 = load ptr, ptr %node1, align 8, !dbg !42
  %has_left_ptr13 = getelementptr inbounds %TreeNode, ptr %node12, i32 0, i32 4, !dbg !42
  store i1 true, ptr %has_left_ptr13, align 1, !dbg !42
  br label %ifcont, !dbg !42

else8:                                            ; preds = %then
  %node14 = load ptr, ptr %node1, align 8, !dbg !42
  %left_ptr15 = getelementptr inbounds %TreeNode, ptr %node14, i32 0, i32 2, !dbg !42
  %key16 = load i64, ptr %key2, align 4, !dbg !42
  %value17 = load ptr, ptr %value3, align 8, !dbg !42
  call void @insert_recursive(ptr %left_ptr15, i64 %key16, ptr %value17), !dbg !42
  br label %ifcont, !dbg !42

ifcont:                                           ; preds = %else8, %then7
  br label %ifcont34, !dbg !42

then20:                                           ; preds = %else
  %node22 = load ptr, ptr %node1, align 8, !dbg !42
  %right_ptr = getelementptr inbounds %TreeNode, ptr %node22, i32 0, i32 3, !dbg !42
  %key23 = load i64, ptr %key2, align 4, !dbg !42
  %value24 = load ptr, ptr %value3, align 8, !dbg !42
  %calltmp25 = call %TreeNode @create_node(i64 %key23, ptr %value24), !dbg !42
  %malloc_struct26 = call ptr @malloc(i64 40), !dbg !42
  store %TreeNode %calltmp25, ptr %malloc_struct26, align 8, !dbg !42
  store ptr %malloc_struct26, ptr %right_ptr, align 8, !dbg !42
  %node27 = load ptr, ptr %node1, align 8, !dbg !42
  %has_right_ptr28 = getelementptr inbounds %TreeNode, ptr %node27, i32 0, i32 5, !dbg !42
  store i1 true, ptr %has_right_ptr28, align 1, !dbg !42
  br label %ifcont33, !dbg !42

else21:                                           ; preds = %else
  %node29 = load ptr, ptr %node1, align 8, !dbg !42
  %right_ptr30 = getelementptr inbounds %TreeNode, ptr %node29, i32 0, i32 3, !dbg !42
  %key31 = load i64, ptr %key2, align 4, !dbg !42
  %value32 = load ptr, ptr %value3, align 8, !dbg !42
  call void @insert_recursive(ptr %right_ptr30, i64 %key31, ptr %value32), !dbg !42
  br label %ifcont33, !dbg !42

ifcont33:                                         ; preds = %else21, %then20
  br label %ifcont34, !dbg !42

ifcont34:                                         ; preds = %ifcont33, %ifcont
  ret void, !dbg !42
}

define ptr @search(%BinaryTree %tree, i64 %key) !dbg !46 {
entry:
  %key2 = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !52
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !50, metadata !DIExpression()), !dbg !53
  store i64 %key, ptr %key2, align 4, !dbg !52
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !51, metadata !DIExpression()), !dbg !54
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !52
  %temp_struct = alloca %BinaryTree, align 8, !dbg !52
  store %BinaryTree %tree3, ptr %temp_struct, align 8, !dbg !52
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !52
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !52
  %nottmp = xor i1 %has_root_val, true, !dbg !52
  br i1 %nottmp, label %then, label %ifcont, !dbg !52

then:                                             ; preds = %entry
  ret ptr @3, !dbg !52

ifcont:                                           ; preds = %entry
  %tree4 = load %BinaryTree, ptr %tree1, align 8, !dbg !52
  %temp_struct5 = alloca %BinaryTree, align 8, !dbg !52
  store %BinaryTree %tree4, ptr %temp_struct5, align 8, !dbg !52
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct5, i32 0, i32 0, !dbg !52
  %key6 = load i64, ptr %key2, align 4, !dbg !52
  %calltmp = call ptr @search_recursive(ptr %root_ptr, i64 %key6), !dbg !52
  ret ptr %calltmp, !dbg !52
}

define ptr @search_recursive(ptr %node, i64 %key) !dbg !55 {
entry:
  %key2 = alloca i64, align 8
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !61
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !59, metadata !DIExpression()), !dbg !62
  store i64 %key, ptr %key2, align 4, !dbg !61
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !60, metadata !DIExpression()), !dbg !63
  %node3 = load ptr, ptr %node1, align 8, !dbg !61
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node3, i32 0, i32 0, !dbg !61
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !61
  %key4 = load i64, ptr %key2, align 4, !dbg !61
  %icmpeqtmp = icmp eq i64 %key_val, %key4, !dbg !61
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !61

then:                                             ; preds = %entry
  %node5 = load ptr, ptr %node1, align 8, !dbg !61
  %value_ptr = getelementptr inbounds %TreeNode, ptr %node5, i32 0, i32 1, !dbg !61
  ret ptr %value_ptr, !dbg !61

ifcont:                                           ; preds = %entry
  %key6 = load i64, ptr %key2, align 4, !dbg !61
  %node7 = load ptr, ptr %node1, align 8, !dbg !61
  %key_ptr8 = getelementptr inbounds %TreeNode, ptr %node7, i32 0, i32 0, !dbg !61
  %key_val9 = load i64, ptr %key_ptr8, align 4, !dbg !61
  %icmpslttmp = icmp slt i64 %key6, %key_val9, !dbg !61
  br i1 %icmpslttmp, label %then10, label %else, !dbg !61

then10:                                           ; preds = %ifcont
  %node11 = load ptr, ptr %node1, align 8, !dbg !61
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node11, i32 0, i32 4, !dbg !61
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !61
  br i1 %has_left_val, label %then12, label %else13, !dbg !61

else:                                             ; preds = %ifcont
  %node16 = load ptr, ptr %node1, align 8, !dbg !61
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node16, i32 0, i32 5, !dbg !61
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !61
  br i1 %has_right_val, label %then17, label %else18, !dbg !61

then12:                                           ; preds = %then10
  %node14 = load ptr, ptr %node1, align 8, !dbg !61
  %left_ptr = getelementptr inbounds %TreeNode, ptr %node14, i32 0, i32 2, !dbg !61
  %key15 = load i64, ptr %key2, align 4, !dbg !61
  %calltmp = call ptr @search_recursive(ptr %left_ptr, i64 %key15), !dbg !61
  ret ptr %calltmp, !dbg !61

else13:                                           ; preds = %then10
  ret ptr @4, !dbg !61

then17:                                           ; preds = %else
  %node19 = load ptr, ptr %node1, align 8, !dbg !61
  %right_ptr = getelementptr inbounds %TreeNode, ptr %node19, i32 0, i32 3, !dbg !61
  %key20 = load i64, ptr %key2, align 4, !dbg !61
  %calltmp21 = call ptr @search_recursive(ptr %right_ptr, i64 %key20), !dbg !61
  ret ptr %calltmp21, !dbg !61

else18:                                           ; preds = %else
  ret ptr @5, !dbg !61
}

define void @print_tree(%BinaryTree %tree) !dbg !64 {
entry:
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !69
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !68, metadata !DIExpression()), !dbg !70
  %tree2 = load %BinaryTree, ptr %tree1, align 8, !dbg !69
  %temp_struct = alloca %BinaryTree, align 8, !dbg !69
  store %BinaryTree %tree2, ptr %temp_struct, align 8, !dbg !69
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !69
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !69
  br i1 %has_root_val, label %then, label %else, !dbg !69

then:                                             ; preds = %entry
  call void @__vyn_println(ptr @6), !dbg !69
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !69
  %temp_struct4 = alloca %BinaryTree, align 8, !dbg !69
  store %BinaryTree %tree3, ptr %temp_struct4, align 8, !dbg !69
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct4, i32 0, i32 0, !dbg !69
  call void @print_in_order(ptr %root_ptr), !dbg !69
  br label %ifcont, !dbg !69

else:                                             ; preds = %entry
  call void @__vyn_println(ptr @7), !dbg !69
  br label %ifcont, !dbg !69

ifcont:                                           ; preds = %else, %then
  ret void, !dbg !69
}

define void @print_in_order(ptr %node) !dbg !71 {
entry:
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !76
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !75, metadata !DIExpression()), !dbg !77
  %node2 = load ptr, ptr %node1, align 8, !dbg !76
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node2, i32 0, i32 4, !dbg !76
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !76
  br i1 %has_left_val, label %then, label %ifcont, !dbg !76

then:                                             ; preds = %entry
  %node3 = load ptr, ptr %node1, align 8, !dbg !76
  %left_ptr = getelementptr inbounds %TreeNode, ptr %node3, i32 0, i32 2, !dbg !76
  call void @print_in_order(ptr %left_ptr), !dbg !76
  br label %ifcont, !dbg !76

ifcont:                                           ; preds = %then, %entry
  %node4 = load ptr, ptr %node1, align 8, !dbg !76
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node4, i32 0, i32 0, !dbg !76
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !76
  %tostring = call ptr @__vyn_toString_int(i64 %key_val), !dbg !76
  %strcattmp = call ptr @__vyn_string_concat(ptr @8, ptr %tostring), !dbg !76
  %strcattmp5 = call ptr @__vyn_string_concat(ptr %strcattmp, ptr @9), !dbg !76
  %node6 = load ptr, ptr %node1, align 8, !dbg !76
  %value_ptr = getelementptr inbounds %TreeNode, ptr %node6, i32 0, i32 1, !dbg !76
  %strcattmp7 = call ptr @__vyn_string_concat(ptr %strcattmp5, ptr %value_ptr), !dbg !76
  call void @__vyn_println(ptr %strcattmp7), !dbg !76
  %node8 = load ptr, ptr %node1, align 8, !dbg !76
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node8, i32 0, i32 5, !dbg !76
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !76
  br i1 %has_right_val, label %then9, label %ifcont11, !dbg !76

then9:                                            ; preds = %ifcont
  %node10 = load ptr, ptr %node1, align 8, !dbg !76
  %right_ptr = getelementptr inbounds %TreeNode, ptr %node10, i32 0, i32 3, !dbg !76
  call void @print_in_order(ptr %right_ptr), !dbg !76
  br label %ifcont11, !dbg !76

ifcont11:                                         ; preds = %then9, %ifcont
  ret void, !dbg !76
}

define i64 @main() !dbg !78 {
entry:
  %result4 = alloca ptr, align 8, !dbg !87
  %result3 = alloca ptr, align 8, !dbg !87
  %result2 = alloca ptr, align 8, !dbg !87
  %result1 = alloca ptr, align 8, !dbg !87
  %tree = alloca %BinaryTree, align 8, !dbg !87
  %calltmp = call %BinaryTree @new_tree(), !dbg !87
  store %BinaryTree %calltmp, ptr %tree, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %tree, metadata !82, metadata !DIExpression()), !dbg !88
  call void @__vyn_println(ptr @10), !dbg !87
  call void @__vyn_println(ptr @11), !dbg !87
  call void @__vyn_println(ptr @12), !dbg !87
  %tree1 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @insert(%BinaryTree %tree1, i64 50, ptr @13), !dbg !87
  %tree2 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @insert(%BinaryTree %tree2, i64 30, ptr @14), !dbg !87
  %tree3 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @insert(%BinaryTree %tree3, i64 70, ptr @15), !dbg !87
  %tree4 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @insert(%BinaryTree %tree4, i64 20, ptr @16), !dbg !87
  %tree5 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @insert(%BinaryTree %tree5, i64 40, ptr @17), !dbg !87
  %tree6 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @insert(%BinaryTree %tree6, i64 60, ptr @18), !dbg !87
  %tree7 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @insert(%BinaryTree %tree7, i64 80, ptr @19), !dbg !87
  %tree8 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %temp_struct = alloca %BinaryTree, align 8, !dbg !87
  store %BinaryTree %tree8, ptr %temp_struct, align 8, !dbg !87
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 2, !dbg !87
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !87
  %tostring = call ptr @__vyn_toString_int(i64 %size_val), !dbg !87
  %strcattmp = call ptr @__vyn_string_concat(ptr @20, ptr %tostring), !dbg !87
  call void @__vyn_println(ptr %strcattmp), !dbg !87
  call void @__vyn_println(ptr @21), !dbg !87
  call void @__vyn_println(ptr @22), !dbg !87
  %tree9 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp10 = call ptr @search(%BinaryTree %tree9, i64 40), !dbg !87
  store ptr %calltmp10, ptr %result1, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !83, metadata !DIExpression()), !dbg !89
  %result111 = load ptr, ptr %result1, align 8, !dbg !87
  %strcattmp12 = call ptr @__vyn_string_concat(ptr @23, ptr %result111), !dbg !87
  call void @__vyn_println(ptr %strcattmp12), !dbg !87
  %tree13 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp14 = call ptr @search(%BinaryTree %tree13, i64 99), !dbg !87
  store ptr %calltmp14, ptr %result2, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !84, metadata !DIExpression()), !dbg !90
  %result215 = load ptr, ptr %result2, align 8, !dbg !87
  %strcattmp16 = call ptr @__vyn_string_concat(ptr @24, ptr %result215), !dbg !87
  call void @__vyn_println(ptr %strcattmp16), !dbg !87
  %tree17 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp18 = call ptr @search(%BinaryTree %tree17, i64 20), !dbg !87
  store ptr %calltmp18, ptr %result3, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !85, metadata !DIExpression()), !dbg !91
  %result319 = load ptr, ptr %result3, align 8, !dbg !87
  %strcattmp20 = call ptr @__vyn_string_concat(ptr @25, ptr %result319), !dbg !87
  call void @__vyn_println(ptr %strcattmp20), !dbg !87
  %tree21 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp22 = call ptr @search(%BinaryTree %tree21, i64 70), !dbg !87
  store ptr %calltmp22, ptr %result4, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result4, metadata !86, metadata !DIExpression()), !dbg !92
  %result423 = load ptr, ptr %result4, align 8, !dbg !87
  %strcattmp24 = call ptr @__vyn_string_concat(ptr @26, ptr %result423), !dbg !87
  call void @__vyn_println(ptr %strcattmp24), !dbg !87
  call void @__vyn_println(ptr @27), !dbg !87
  %tree25 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @print_tree(%BinaryTree %tree25), !dbg !87
  ret i64 0, !dbg !87
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
!4 = distinct !DISubprogram(name: "create_node", linkageName: "create_node", scope: !1, file: !1, line: 22, type: !5, scopeLine: 22, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !10)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8, !9}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "TreeNode", scope: !1, file: !1, size: 384, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!10 = !{!11, !12}
!11 = !DILocalVariable(name: "key", scope: !4, file: !1, line: 23, type: !8)
!12 = !DILocalVariable(name: "value", scope: !4, file: !1, line: 23, type: !9)
!13 = !DILocation(line: 22, column: 1, scope: !4)
!14 = !DILocation(line: 23, column: 16, scope: !4)
!15 = !DILocation(line: 23, column: 28, scope: !4)
!16 = distinct !DISubprogram(name: "new_tree", linkageName: "new_tree", scope: !1, file: !1, line: 34, type: !17, scopeLine: 34, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !20)
!17 = !DISubroutineType(types: !18)
!18 = !{!19}
!19 = !DICompositeType(tag: DW_TAG_structure_type, name: "BinaryTree", scope: !1, file: !1, size: 192, align: 8)
!20 = !{!21}
!21 = !DILocalVariable(name: "dummy_node", scope: !16, file: !1, line: 36, type: !7)
!22 = !DILocation(line: 34, column: 1, scope: !16)
!23 = !DILocation(line: 36, column: 1, scope: !16)
!24 = distinct !DISubprogram(name: "insert", linkageName: "insert", scope: !1, file: !1, line: 52, type: !25, scopeLine: 52, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !27)
!25 = !DISubroutineType(types: !26)
!26 = !{null, !19, !8, !9}
!27 = !{!28, !29, !30}
!28 = !DILocalVariable(name: "tree", scope: !24, file: !1, line: 53, type: !19)
!29 = !DILocalVariable(name: "key", scope: !24, file: !1, line: 53, type: !8)
!30 = !DILocalVariable(name: "value", scope: !24, file: !1, line: 53, type: !9)
!31 = !DILocation(line: 52, column: 1, scope: !24)
!32 = !DILocation(line: 53, column: 12, scope: !24)
!33 = !DILocation(line: 53, column: 29, scope: !24)
!34 = !DILocation(line: 53, column: 41, scope: !24)
!35 = distinct !DISubprogram(name: "insert_recursive", linkageName: "insert_recursive", scope: !1, file: !1, line: 65, type: !36, scopeLine: 65, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !38)
!36 = !DISubroutineType(types: !37)
!37 = !{null, !9, !8, !9}
!38 = !{!39, !40, !41}
!39 = !DILocalVariable(name: "node", scope: !35, file: !1, line: 66, type: !9)
!40 = !DILocalVariable(name: "key", scope: !35, file: !1, line: 66, type: !8)
!41 = !DILocalVariable(name: "value", scope: !35, file: !1, line: 66, type: !9)
!42 = !DILocation(line: 65, column: 1, scope: !35)
!43 = !DILocation(line: 66, column: 22, scope: !35)
!44 = !DILocation(line: 66, column: 44, scope: !35)
!45 = !DILocation(line: 66, column: 56, scope: !35)
!46 = distinct !DISubprogram(name: "search", linkageName: "search", scope: !1, file: !1, line: 84, type: !47, scopeLine: 84, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !49)
!47 = !DISubroutineType(types: !48)
!48 = !{!9, !19, !8}
!49 = !{!50, !51}
!50 = !DILocalVariable(name: "tree", scope: !46, file: !1, line: 85, type: !19)
!51 = !DILocalVariable(name: "key", scope: !46, file: !1, line: 85, type: !8)
!52 = !DILocation(line: 84, column: 1, scope: !46)
!53 = !DILocation(line: 85, column: 12, scope: !46)
!54 = !DILocation(line: 85, column: 29, scope: !46)
!55 = distinct !DISubprogram(name: "search_recursive", linkageName: "search_recursive", scope: !1, file: !1, line: 93, type: !56, scopeLine: 93, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !58)
!56 = !DISubroutineType(types: !57)
!57 = !{!9, !9, !8}
!58 = !{!59, !60}
!59 = !DILocalVariable(name: "node", scope: !55, file: !1, line: 94, type: !9)
!60 = !DILocalVariable(name: "key", scope: !55, file: !1, line: 94, type: !8)
!61 = !DILocation(line: 93, column: 1, scope: !55)
!62 = !DILocation(line: 94, column: 22, scope: !55)
!63 = !DILocation(line: 94, column: 50, scope: !55)
!64 = distinct !DISubprogram(name: "print_tree", linkageName: "print_tree", scope: !1, file: !1, line: 114, type: !65, scopeLine: 114, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !67)
!65 = !DISubroutineType(types: !66)
!66 = !{null, !19}
!67 = !{!68}
!68 = !DILocalVariable(name: "tree", scope: !64, file: !1, line: 115, type: !19)
!69 = !DILocation(line: 114, column: 1, scope: !64)
!70 = !DILocation(line: 115, column: 16, scope: !64)
!71 = distinct !DISubprogram(name: "print_in_order", linkageName: "print_in_order", scope: !1, file: !1, line: 124, type: !72, scopeLine: 124, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !74)
!72 = !DISubroutineType(types: !73)
!73 = !{null, !9}
!74 = !{!75}
!75 = !DILocalVariable(name: "node", scope: !71, file: !1, line: 125, type: !9)
!76 = !DILocation(line: 124, column: 1, scope: !71)
!77 = !DILocation(line: 125, column: 20, scope: !71)
!78 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 137, type: !79, scopeLine: 137, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !81)
!79 = !DISubroutineType(types: !80)
!80 = !{!8}
!81 = !{!82, !83, !84, !85, !86}
!82 = !DILocalVariable(name: "tree", scope: !78, file: !1, line: 139, type: !19)
!83 = !DILocalVariable(name: "result1", scope: !78, file: !1, line: 160, type: !9)
!84 = !DILocalVariable(name: "result2", scope: !78, file: !1, line: 163, type: !9)
!85 = !DILocalVariable(name: "result3", scope: !78, file: !1, line: 166, type: !9)
!86 = !DILocalVariable(name: "result4", scope: !78, file: !1, line: 169, type: !9)
!87 = !DILocation(line: 137, column: 1, scope: !78)
!88 = !DILocation(line: 139, column: 5, scope: !78)
!89 = !DILocation(line: 160, column: 1, scope: !78)
!90 = !DILocation(line: 163, column: 1, scope: !78)
!91 = !DILocation(line: 166, column: 1, scope: !78)
!92 = !DILocation(line: 169, column: 1, scope: !78)
