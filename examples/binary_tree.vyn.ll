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

define void @insert(ptr %tree, i64 %key, ptr %value) !dbg !24 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca ptr, align 8
  store ptr %tree, ptr %tree1, align 8, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !28, metadata !DIExpression()), !dbg !32
  store i64 %key, ptr %key2, align 4, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !29, metadata !DIExpression()), !dbg !33
  store ptr %value, ptr %value3, align 8, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !30, metadata !DIExpression()), !dbg !34
  %tree4 = load ptr, ptr %tree1, align 8, !dbg !31
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %tree4, i32 0, i32 1, !dbg !31
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !31
  %nottmp = xor i1 %has_root_val, true, !dbg !31
  br i1 %nottmp, label %then, label %ifcont, !dbg !31

then:                                             ; preds = %entry
  %tree5 = load ptr, ptr %tree1, align 8, !dbg !31
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %tree5, i32 0, i32 0, !dbg !31
  %key6 = load i64, ptr %key2, align 4, !dbg !31
  %value7 = load ptr, ptr %value3, align 8, !dbg !31
  %calltmp = call %TreeNode @create_node(i64 %key6, ptr %value7), !dbg !31
  %malloc_struct = call ptr @malloc(i64 40), !dbg !31
  store %TreeNode %calltmp, ptr %malloc_struct, align 8, !dbg !31
  store ptr %malloc_struct, ptr %root_ptr, align 8, !dbg !31
  %tree8 = load ptr, ptr %tree1, align 8, !dbg !31
  %has_root_ptr9 = getelementptr inbounds %BinaryTree, ptr %tree8, i32 0, i32 1, !dbg !31
  store i1 true, ptr %has_root_ptr9, align 1, !dbg !31
  %tree10 = load ptr, ptr %tree1, align 8, !dbg !31
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %tree10, i32 0, i32 2, !dbg !31
  %tree11 = load ptr, ptr %tree1, align 8, !dbg !31
  %size_ptr12 = getelementptr inbounds %BinaryTree, ptr %tree11, i32 0, i32 2, !dbg !31
  %size_val = load i64, ptr %size_ptr12, align 4, !dbg !31
  %addtmp = add i64 %size_val, 1, !dbg !31
  store i64 %addtmp, ptr %size_ptr, align 4, !dbg !31
  ret void, !dbg !31

ifcont:                                           ; preds = %entry
  %tree13 = load ptr, ptr %tree1, align 8, !dbg !31
  %root_ptr14 = getelementptr inbounds %BinaryTree, ptr %tree13, i32 0, i32 0, !dbg !31
  %key15 = load i64, ptr %key2, align 4, !dbg !31
  %value16 = load ptr, ptr %value3, align 8, !dbg !31
  call void @insert_recursive(ptr %root_ptr14, i64 %key15, ptr %value16), !dbg !31
  %tree17 = load ptr, ptr %tree1, align 8, !dbg !31
  %size_ptr18 = getelementptr inbounds %BinaryTree, ptr %tree17, i32 0, i32 2, !dbg !31
  %tree19 = load ptr, ptr %tree1, align 8, !dbg !31
  %size_ptr20 = getelementptr inbounds %BinaryTree, ptr %tree19, i32 0, i32 2, !dbg !31
  %size_val21 = load i64, ptr %size_ptr20, align 4, !dbg !31
  %addtmp22 = add i64 %size_val21, 1, !dbg !31
  store i64 %addtmp22, ptr %size_ptr18, align 4, !dbg !31
  ret void, !dbg !31
}

define void @insert_recursive(ptr %node, i64 %key, ptr %value) !dbg !35 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !40
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !37, metadata !DIExpression()), !dbg !41
  store i64 %key, ptr %key2, align 4, !dbg !40
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !38, metadata !DIExpression()), !dbg !42
  store ptr %value, ptr %value3, align 8, !dbg !40
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !39, metadata !DIExpression()), !dbg !43
  %key4 = load i64, ptr %key2, align 4, !dbg !40
  %node5 = load ptr, ptr %node1, align 8, !dbg !40
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node5, i32 0, i32 0, !dbg !40
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !40
  %icmpslttmp = icmp slt i64 %key4, %key_val, !dbg !40
  br i1 %icmpslttmp, label %then, label %else, !dbg !40

then:                                             ; preds = %entry
  %node6 = load ptr, ptr %node1, align 8, !dbg !40
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node6, i32 0, i32 4, !dbg !40
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !40
  %nottmp = xor i1 %has_left_val, true, !dbg !40
  br i1 %nottmp, label %then7, label %else8, !dbg !40

else:                                             ; preds = %entry
  %node18 = load ptr, ptr %node1, align 8, !dbg !40
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node18, i32 0, i32 5, !dbg !40
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !40
  %nottmp19 = xor i1 %has_right_val, true, !dbg !40
  br i1 %nottmp19, label %then20, label %else21, !dbg !40

then7:                                            ; preds = %then
  %node9 = load ptr, ptr %node1, align 8, !dbg !40
  %left_ptr = getelementptr inbounds %TreeNode, ptr %node9, i32 0, i32 2, !dbg !40
  %key10 = load i64, ptr %key2, align 4, !dbg !40
  %value11 = load ptr, ptr %value3, align 8, !dbg !40
  %calltmp = call %TreeNode @create_node(i64 %key10, ptr %value11), !dbg !40
  %malloc_struct = call ptr @malloc(i64 40), !dbg !40
  store %TreeNode %calltmp, ptr %malloc_struct, align 8, !dbg !40
  store ptr %malloc_struct, ptr %left_ptr, align 8, !dbg !40
  %node12 = load ptr, ptr %node1, align 8, !dbg !40
  %has_left_ptr13 = getelementptr inbounds %TreeNode, ptr %node12, i32 0, i32 4, !dbg !40
  store i1 true, ptr %has_left_ptr13, align 1, !dbg !40
  br label %ifcont, !dbg !40

else8:                                            ; preds = %then
  %node14 = load ptr, ptr %node1, align 8, !dbg !40
  %left_ptr15 = getelementptr inbounds %TreeNode, ptr %node14, i32 0, i32 2, !dbg !40
  %key16 = load i64, ptr %key2, align 4, !dbg !40
  %value17 = load ptr, ptr %value3, align 8, !dbg !40
  call void @insert_recursive(ptr %left_ptr15, i64 %key16, ptr %value17), !dbg !40
  br label %ifcont, !dbg !40

ifcont:                                           ; preds = %else8, %then7
  br label %ifcont34, !dbg !40

then20:                                           ; preds = %else
  %node22 = load ptr, ptr %node1, align 8, !dbg !40
  %right_ptr = getelementptr inbounds %TreeNode, ptr %node22, i32 0, i32 3, !dbg !40
  %key23 = load i64, ptr %key2, align 4, !dbg !40
  %value24 = load ptr, ptr %value3, align 8, !dbg !40
  %calltmp25 = call %TreeNode @create_node(i64 %key23, ptr %value24), !dbg !40
  %malloc_struct26 = call ptr @malloc(i64 40), !dbg !40
  store %TreeNode %calltmp25, ptr %malloc_struct26, align 8, !dbg !40
  store ptr %malloc_struct26, ptr %right_ptr, align 8, !dbg !40
  %node27 = load ptr, ptr %node1, align 8, !dbg !40
  %has_right_ptr28 = getelementptr inbounds %TreeNode, ptr %node27, i32 0, i32 5, !dbg !40
  store i1 true, ptr %has_right_ptr28, align 1, !dbg !40
  br label %ifcont33, !dbg !40

else21:                                           ; preds = %else
  %node29 = load ptr, ptr %node1, align 8, !dbg !40
  %right_ptr30 = getelementptr inbounds %TreeNode, ptr %node29, i32 0, i32 3, !dbg !40
  %key31 = load i64, ptr %key2, align 4, !dbg !40
  %value32 = load ptr, ptr %value3, align 8, !dbg !40
  call void @insert_recursive(ptr %right_ptr30, i64 %key31, ptr %value32), !dbg !40
  br label %ifcont33, !dbg !40

ifcont33:                                         ; preds = %else21, %then20
  br label %ifcont34, !dbg !40

ifcont34:                                         ; preds = %ifcont33, %ifcont
  ret void, !dbg !40
}

define ptr @search(%BinaryTree %tree, i64 %key) !dbg !44 {
entry:
  %key2 = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !50
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !48, metadata !DIExpression()), !dbg !51
  store i64 %key, ptr %key2, align 4, !dbg !50
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !49, metadata !DIExpression()), !dbg !52
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !50
  %temp_struct = alloca %BinaryTree, align 8, !dbg !50
  store %BinaryTree %tree3, ptr %temp_struct, align 8, !dbg !50
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !50
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !50
  %nottmp = xor i1 %has_root_val, true, !dbg !50
  br i1 %nottmp, label %then, label %ifcont, !dbg !50

then:                                             ; preds = %entry
  ret ptr @3, !dbg !50

ifcont:                                           ; preds = %entry
  %tree4 = load %BinaryTree, ptr %tree1, align 8, !dbg !50
  %temp_struct5 = alloca %BinaryTree, align 8, !dbg !50
  store %BinaryTree %tree4, ptr %temp_struct5, align 8, !dbg !50
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct5, i32 0, i32 0, !dbg !50
  %key6 = load i64, ptr %key2, align 4, !dbg !50
  %calltmp = call ptr @search_recursive(ptr %root_ptr, i64 %key6), !dbg !50
  ret ptr %calltmp, !dbg !50
}

define ptr @search_recursive(ptr %node, i64 %key) !dbg !53 {
entry:
  %key2 = alloca i64, align 8
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !59
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !57, metadata !DIExpression()), !dbg !60
  store i64 %key, ptr %key2, align 4, !dbg !59
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !58, metadata !DIExpression()), !dbg !61
  %node3 = load ptr, ptr %node1, align 8, !dbg !59
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node3, i32 0, i32 0, !dbg !59
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !59
  %key4 = load i64, ptr %key2, align 4, !dbg !59
  %icmpeqtmp = icmp eq i64 %key_val, %key4, !dbg !59
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !59

then:                                             ; preds = %entry
  %node5 = load ptr, ptr %node1, align 8, !dbg !59
  %value_ptr = getelementptr inbounds %TreeNode, ptr %node5, i32 0, i32 1, !dbg !59
  ret ptr %value_ptr, !dbg !59

ifcont:                                           ; preds = %entry
  %key6 = load i64, ptr %key2, align 4, !dbg !59
  %node7 = load ptr, ptr %node1, align 8, !dbg !59
  %key_ptr8 = getelementptr inbounds %TreeNode, ptr %node7, i32 0, i32 0, !dbg !59
  %key_val9 = load i64, ptr %key_ptr8, align 4, !dbg !59
  %icmpslttmp = icmp slt i64 %key6, %key_val9, !dbg !59
  br i1 %icmpslttmp, label %then10, label %else, !dbg !59

then10:                                           ; preds = %ifcont
  %node11 = load ptr, ptr %node1, align 8, !dbg !59
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node11, i32 0, i32 4, !dbg !59
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !59
  br i1 %has_left_val, label %then12, label %else13, !dbg !59

else:                                             ; preds = %ifcont
  %node16 = load ptr, ptr %node1, align 8, !dbg !59
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node16, i32 0, i32 5, !dbg !59
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !59
  br i1 %has_right_val, label %then17, label %else18, !dbg !59

then12:                                           ; preds = %then10
  %node14 = load ptr, ptr %node1, align 8, !dbg !59
  %left_ptr = getelementptr inbounds %TreeNode, ptr %node14, i32 0, i32 2, !dbg !59
  %key15 = load i64, ptr %key2, align 4, !dbg !59
  %calltmp = call ptr @search_recursive(ptr %left_ptr, i64 %key15), !dbg !59
  ret ptr %calltmp, !dbg !59

else13:                                           ; preds = %then10
  ret ptr @4, !dbg !59

then17:                                           ; preds = %else
  %node19 = load ptr, ptr %node1, align 8, !dbg !59
  %right_ptr = getelementptr inbounds %TreeNode, ptr %node19, i32 0, i32 3, !dbg !59
  %key20 = load i64, ptr %key2, align 4, !dbg !59
  %calltmp21 = call ptr @search_recursive(ptr %right_ptr, i64 %key20), !dbg !59
  ret ptr %calltmp21, !dbg !59

else18:                                           ; preds = %else
  ret ptr @5, !dbg !59
}

define void @print_tree(%BinaryTree %tree) !dbg !62 {
entry:
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !67
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !66, metadata !DIExpression()), !dbg !68
  %tree2 = load %BinaryTree, ptr %tree1, align 8, !dbg !67
  %temp_struct = alloca %BinaryTree, align 8, !dbg !67
  store %BinaryTree %tree2, ptr %temp_struct, align 8, !dbg !67
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !67
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !67
  br i1 %has_root_val, label %then, label %else, !dbg !67

then:                                             ; preds = %entry
  call void @__vyn_println(ptr @6), !dbg !67
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !67
  %temp_struct4 = alloca %BinaryTree, align 8, !dbg !67
  store %BinaryTree %tree3, ptr %temp_struct4, align 8, !dbg !67
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct4, i32 0, i32 0, !dbg !67
  call void @print_in_order(ptr %root_ptr), !dbg !67
  br label %ifcont, !dbg !67

else:                                             ; preds = %entry
  call void @__vyn_println(ptr @7), !dbg !67
  br label %ifcont, !dbg !67

ifcont:                                           ; preds = %else, %then
  ret void, !dbg !67
}

define void @print_in_order(ptr %node) !dbg !69 {
entry:
  %node1 = alloca ptr, align 8
  store ptr %node, ptr %node1, align 8, !dbg !74
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !73, metadata !DIExpression()), !dbg !75
  %node2 = load ptr, ptr %node1, align 8, !dbg !74
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %node2, i32 0, i32 4, !dbg !74
  %has_left_val = load i1, ptr %has_left_ptr, align 1, !dbg !74
  br i1 %has_left_val, label %then, label %ifcont, !dbg !74

then:                                             ; preds = %entry
  %node3 = load ptr, ptr %node1, align 8, !dbg !74
  %left_ptr = getelementptr inbounds %TreeNode, ptr %node3, i32 0, i32 2, !dbg !74
  call void @print_in_order(ptr %left_ptr), !dbg !74
  br label %ifcont, !dbg !74

ifcont:                                           ; preds = %then, %entry
  %node4 = load ptr, ptr %node1, align 8, !dbg !74
  %key_ptr = getelementptr inbounds %TreeNode, ptr %node4, i32 0, i32 0, !dbg !74
  %key_val = load i64, ptr %key_ptr, align 4, !dbg !74
  %tostring = call ptr @__vyn_toString_int(i64 %key_val), !dbg !74
  %strcattmp = call ptr @__vyn_string_concat(ptr @8, ptr %tostring), !dbg !74
  %strcattmp5 = call ptr @__vyn_string_concat(ptr %strcattmp, ptr @9), !dbg !74
  %node6 = load ptr, ptr %node1, align 8, !dbg !74
  %value_ptr = getelementptr inbounds %TreeNode, ptr %node6, i32 0, i32 1, !dbg !74
  %strcattmp7 = call ptr @__vyn_string_concat(ptr %strcattmp5, ptr %value_ptr), !dbg !74
  call void @__vyn_println(ptr %strcattmp7), !dbg !74
  %node8 = load ptr, ptr %node1, align 8, !dbg !74
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %node8, i32 0, i32 5, !dbg !74
  %has_right_val = load i1, ptr %has_right_ptr, align 1, !dbg !74
  br i1 %has_right_val, label %then9, label %ifcont11, !dbg !74

then9:                                            ; preds = %ifcont
  %node10 = load ptr, ptr %node1, align 8, !dbg !74
  %right_ptr = getelementptr inbounds %TreeNode, ptr %node10, i32 0, i32 3, !dbg !74
  call void @print_in_order(ptr %right_ptr), !dbg !74
  br label %ifcont11, !dbg !74

ifcont11:                                         ; preds = %then9, %ifcont
  ret void, !dbg !74
}

define i64 @main() !dbg !76 {
entry:
  %result4 = alloca ptr, align 8, !dbg !85
  %result3 = alloca ptr, align 8, !dbg !85
  %result2 = alloca ptr, align 8, !dbg !85
  %result1 = alloca ptr, align 8, !dbg !85
  %tree = alloca %BinaryTree, align 8, !dbg !85
  %calltmp = call %BinaryTree @new_tree(), !dbg !85
  store %BinaryTree %calltmp, ptr %tree, align 8, !dbg !85
  call void @llvm.dbg.declare(metadata ptr %tree, metadata !80, metadata !DIExpression()), !dbg !86
  call void @__vyn_println(ptr @10), !dbg !85
  call void @__vyn_println(ptr @11), !dbg !85
  call void @__vyn_println(ptr @12), !dbg !85
  call void @insert(ptr %tree, i64 50, ptr @13), !dbg !85
  call void @insert(ptr %tree, i64 30, ptr @14), !dbg !85
  call void @insert(ptr %tree, i64 70, ptr @15), !dbg !85
  call void @insert(ptr %tree, i64 20, ptr @16), !dbg !85
  call void @insert(ptr %tree, i64 40, ptr @17), !dbg !85
  call void @insert(ptr %tree, i64 60, ptr @18), !dbg !85
  call void @insert(ptr %tree, i64 80, ptr @19), !dbg !85
  %tree1 = load %BinaryTree, ptr %tree, align 8, !dbg !85
  %temp_struct = alloca %BinaryTree, align 8, !dbg !85
  store %BinaryTree %tree1, ptr %temp_struct, align 8, !dbg !85
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 2, !dbg !85
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !85
  %tostring = call ptr @__vyn_toString_int(i64 %size_val), !dbg !85
  %strcattmp = call ptr @__vyn_string_concat(ptr @20, ptr %tostring), !dbg !85
  call void @__vyn_println(ptr %strcattmp), !dbg !85
  call void @__vyn_println(ptr @21), !dbg !85
  call void @__vyn_println(ptr @22), !dbg !85
  %tree2 = load %BinaryTree, ptr %tree, align 8, !dbg !85
  %calltmp3 = call ptr @search(%BinaryTree %tree2, i64 40), !dbg !85
  store ptr %calltmp3, ptr %result1, align 8, !dbg !85
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !81, metadata !DIExpression()), !dbg !87
  %result14 = load ptr, ptr %result1, align 8, !dbg !85
  %strcattmp5 = call ptr @__vyn_string_concat(ptr @23, ptr %result14), !dbg !85
  call void @__vyn_println(ptr %strcattmp5), !dbg !85
  %tree6 = load %BinaryTree, ptr %tree, align 8, !dbg !85
  %calltmp7 = call ptr @search(%BinaryTree %tree6, i64 99), !dbg !85
  store ptr %calltmp7, ptr %result2, align 8, !dbg !85
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !82, metadata !DIExpression()), !dbg !88
  %result28 = load ptr, ptr %result2, align 8, !dbg !85
  %strcattmp9 = call ptr @__vyn_string_concat(ptr @24, ptr %result28), !dbg !85
  call void @__vyn_println(ptr %strcattmp9), !dbg !85
  %tree10 = load %BinaryTree, ptr %tree, align 8, !dbg !85
  %calltmp11 = call ptr @search(%BinaryTree %tree10, i64 20), !dbg !85
  store ptr %calltmp11, ptr %result3, align 8, !dbg !85
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !83, metadata !DIExpression()), !dbg !89
  %result312 = load ptr, ptr %result3, align 8, !dbg !85
  %strcattmp13 = call ptr @__vyn_string_concat(ptr @25, ptr %result312), !dbg !85
  call void @__vyn_println(ptr %strcattmp13), !dbg !85
  %tree14 = load %BinaryTree, ptr %tree, align 8, !dbg !85
  %calltmp15 = call ptr @search(%BinaryTree %tree14, i64 70), !dbg !85
  store ptr %calltmp15, ptr %result4, align 8, !dbg !85
  call void @llvm.dbg.declare(metadata ptr %result4, metadata !84, metadata !DIExpression()), !dbg !90
  %result416 = load ptr, ptr %result4, align 8, !dbg !85
  %strcattmp17 = call ptr @__vyn_string_concat(ptr @26, ptr %result416), !dbg !85
  call void @__vyn_println(ptr %strcattmp17), !dbg !85
  call void @__vyn_println(ptr @27), !dbg !85
  %tree18 = load %BinaryTree, ptr %tree, align 8, !dbg !85
  call void @print_tree(%BinaryTree %tree18), !dbg !85
  ret i64 0, !dbg !85
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
!26 = !{null, !9, !8, !9}
!27 = !{!28, !29, !30}
!28 = !DILocalVariable(name: "tree", scope: !24, file: !1, line: 53, type: !9)
!29 = !DILocalVariable(name: "key", scope: !24, file: !1, line: 53, type: !8)
!30 = !DILocalVariable(name: "value", scope: !24, file: !1, line: 53, type: !9)
!31 = !DILocation(line: 52, column: 1, scope: !24)
!32 = !DILocation(line: 53, column: 12, scope: !24)
!33 = !DILocation(line: 53, column: 36, scope: !24)
!34 = !DILocation(line: 53, column: 48, scope: !24)
!35 = distinct !DISubprogram(name: "insert_recursive", linkageName: "insert_recursive", scope: !1, file: !1, line: 65, type: !25, scopeLine: 65, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !36)
!36 = !{!37, !38, !39}
!37 = !DILocalVariable(name: "node", scope: !35, file: !1, line: 66, type: !9)
!38 = !DILocalVariable(name: "key", scope: !35, file: !1, line: 66, type: !8)
!39 = !DILocalVariable(name: "value", scope: !35, file: !1, line: 66, type: !9)
!40 = !DILocation(line: 65, column: 1, scope: !35)
!41 = !DILocation(line: 66, column: 22, scope: !35)
!42 = !DILocation(line: 66, column: 44, scope: !35)
!43 = !DILocation(line: 66, column: 56, scope: !35)
!44 = distinct !DISubprogram(name: "search", linkageName: "search", scope: !1, file: !1, line: 84, type: !45, scopeLine: 84, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !47)
!45 = !DISubroutineType(types: !46)
!46 = !{!9, !19, !8}
!47 = !{!48, !49}
!48 = !DILocalVariable(name: "tree", scope: !44, file: !1, line: 85, type: !19)
!49 = !DILocalVariable(name: "key", scope: !44, file: !1, line: 85, type: !8)
!50 = !DILocation(line: 84, column: 1, scope: !44)
!51 = !DILocation(line: 85, column: 12, scope: !44)
!52 = !DILocation(line: 85, column: 29, scope: !44)
!53 = distinct !DISubprogram(name: "search_recursive", linkageName: "search_recursive", scope: !1, file: !1, line: 93, type: !54, scopeLine: 93, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !56)
!54 = !DISubroutineType(types: !55)
!55 = !{!9, !9, !8}
!56 = !{!57, !58}
!57 = !DILocalVariable(name: "node", scope: !53, file: !1, line: 94, type: !9)
!58 = !DILocalVariable(name: "key", scope: !53, file: !1, line: 94, type: !8)
!59 = !DILocation(line: 93, column: 1, scope: !53)
!60 = !DILocation(line: 94, column: 22, scope: !53)
!61 = !DILocation(line: 94, column: 50, scope: !53)
!62 = distinct !DISubprogram(name: "print_tree", linkageName: "print_tree", scope: !1, file: !1, line: 114, type: !63, scopeLine: 114, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !65)
!63 = !DISubroutineType(types: !64)
!64 = !{null, !19}
!65 = !{!66}
!66 = !DILocalVariable(name: "tree", scope: !62, file: !1, line: 115, type: !19)
!67 = !DILocation(line: 114, column: 1, scope: !62)
!68 = !DILocation(line: 115, column: 16, scope: !62)
!69 = distinct !DISubprogram(name: "print_in_order", linkageName: "print_in_order", scope: !1, file: !1, line: 124, type: !70, scopeLine: 124, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !72)
!70 = !DISubroutineType(types: !71)
!71 = !{null, !9}
!72 = !{!73}
!73 = !DILocalVariable(name: "node", scope: !69, file: !1, line: 125, type: !9)
!74 = !DILocation(line: 124, column: 1, scope: !69)
!75 = !DILocation(line: 125, column: 20, scope: !69)
!76 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 137, type: !77, scopeLine: 137, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !79)
!77 = !DISubroutineType(types: !78)
!78 = !{!8}
!79 = !{!80, !81, !82, !83, !84}
!80 = !DILocalVariable(name: "tree", scope: !76, file: !1, line: 139, type: !19)
!81 = !DILocalVariable(name: "result1", scope: !76, file: !1, line: 160, type: !9)
!82 = !DILocalVariable(name: "result2", scope: !76, file: !1, line: 163, type: !9)
!83 = !DILocalVariable(name: "result3", scope: !76, file: !1, line: 166, type: !9)
!84 = !DILocalVariable(name: "result4", scope: !76, file: !1, line: 169, type: !9)
!85 = !DILocation(line: 137, column: 1, scope: !76)
!86 = !DILocation(line: 139, column: 5, scope: !76)
!87 = !DILocation(line: 160, column: 1, scope: !76)
!88 = !DILocation(line: 163, column: 1, scope: !76)
!89 = !DILocation(line: 166, column: 1, scope: !76)
!90 = !DILocation(line: 169, column: 1, scope: !76)
