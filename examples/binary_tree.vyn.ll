; ModuleID = 'VynModule'
source_filename = "VynModule"

%TreeNode = type { i64, ptr, ptr, ptr, i1, i1 }
%BinaryTree = type { ptr, i1, i64 }

@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@1 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@2 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@3 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@4 = private unnamed_addr constant [10 x i8] c"NOT_FOUND\00", align 1
@5 = private unnamed_addr constant [26 x i8] c"Tree contents (in-order):\00", align 1
@6 = private unnamed_addr constant [14 x i8] c"Tree is empty\00", align 1
@7 = private unnamed_addr constant [6 x i8] c"Key: \00", align 1
@8 = private unnamed_addr constant [10 x i8] c", Value: \00", align 1
@9 = private unnamed_addr constant [40 x i8] c"Binary Search Tree Example - Vyn v0.4.0\00", align 1
@10 = private unnamed_addr constant [43 x i8] c"==========================================\00", align 1
@11 = private unnamed_addr constant [20 x i8] c"Inserting values...\00", align 1
@12 = private unnamed_addr constant [6 x i8] c"fifty\00", align 1
@13 = private unnamed_addr constant [7 x i8] c"thirty\00", align 1
@14 = private unnamed_addr constant [8 x i8] c"seventy\00", align 1
@15 = private unnamed_addr constant [7 x i8] c"twenty\00", align 1
@16 = private unnamed_addr constant [6 x i8] c"forty\00", align 1
@17 = private unnamed_addr constant [6 x i8] c"sixty\00", align 1
@18 = private unnamed_addr constant [7 x i8] c"eighty\00", align 1
@19 = private unnamed_addr constant [12 x i8] c"Tree size: \00", align 1
@20 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@21 = private unnamed_addr constant [19 x i8] c"Search operations:\00", align 1
@22 = private unnamed_addr constant [16 x i8] c"Search for 40: \00", align 1
@23 = private unnamed_addr constant [16 x i8] c"Search for 99: \00", align 1
@24 = private unnamed_addr constant [16 x i8] c"Search for 20: \00", align 1
@25 = private unnamed_addr constant [16 x i8] c"Search for 70: \00", align 1
@26 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define %TreeNode @create_node(i64 %key, ptr %value) !dbg !4 {
entry:
  %dummy_node = alloca %TreeNode, align 8
  %value2 = alloca ptr, align 8
  %key1 = alloca i64, align 8
  store i64 %key, ptr %key1, align 4, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %key1, metadata !11, metadata !DIExpression()), !dbg !15
  store ptr %value, ptr %value2, align 8, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %value2, metadata !12, metadata !DIExpression()), !dbg !16
  %TreeNode_obj = alloca %TreeNode, align 8, !dbg !14
  %key_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 0, !dbg !14
  store i64 0, ptr %key_ptr, align 4, !dbg !14
  %value_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 1, !dbg !14
  store ptr @0, ptr %value_ptr, align 8, !dbg !14
  %TreeNode_obj3 = alloca %TreeNode, align 8, !dbg !14
  %TreeNode_val = load %TreeNode, ptr %TreeNode_obj3, align 8, !dbg !14
  %malloc_struct = call ptr @malloc(i64 40), !dbg !14
  store %TreeNode %TreeNode_val, ptr %malloc_struct, align 8, !dbg !14
  %left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 2, !dbg !14
  store ptr %malloc_struct, ptr %left_ptr, align 8, !dbg !14
  %TreeNode_obj4 = alloca %TreeNode, align 8, !dbg !14
  %TreeNode_val5 = load %TreeNode, ptr %TreeNode_obj4, align 8, !dbg !14
  %malloc_struct6 = call ptr @malloc(i64 40), !dbg !14
  store %TreeNode %TreeNode_val5, ptr %malloc_struct6, align 8, !dbg !14
  %right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 3, !dbg !14
  store ptr %malloc_struct6, ptr %right_ptr, align 8, !dbg !14
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 4, !dbg !14
  store i1 false, ptr %has_left_ptr, align 1, !dbg !14
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 5, !dbg !14
  store i1 false, ptr %has_right_ptr, align 1, !dbg !14
  %TreeNode_val7 = load %TreeNode, ptr %TreeNode_obj, align 8, !dbg !14
  store %TreeNode %TreeNode_val7, ptr %dummy_node, align 8, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %dummy_node, metadata !13, metadata !DIExpression()), !dbg !17
  %TreeNode_obj8 = alloca %TreeNode, align 8, !dbg !14
  %key9 = load i64, ptr %key1, align 4, !dbg !14
  %key_ptr10 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj8, i32 0, i32 0, !dbg !14
  store i64 %key9, ptr %key_ptr10, align 4, !dbg !14
  %value11 = load ptr, ptr %value2, align 8, !dbg !14
  %value_ptr12 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj8, i32 0, i32 1, !dbg !14
  store ptr %value11, ptr %value_ptr12, align 8, !dbg !14
  %dummy_node13 = load %TreeNode, ptr %dummy_node, align 8, !dbg !14
  %malloc_struct14 = call ptr @malloc(i64 40), !dbg !14
  store %TreeNode %dummy_node13, ptr %malloc_struct14, align 8, !dbg !14
  %left_ptr15 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj8, i32 0, i32 2, !dbg !14
  store ptr %malloc_struct14, ptr %left_ptr15, align 8, !dbg !14
  %dummy_node16 = load %TreeNode, ptr %dummy_node, align 8, !dbg !14
  %malloc_struct17 = call ptr @malloc(i64 40), !dbg !14
  store %TreeNode %dummy_node16, ptr %malloc_struct17, align 8, !dbg !14
  %right_ptr18 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj8, i32 0, i32 3, !dbg !14
  store ptr %malloc_struct17, ptr %right_ptr18, align 8, !dbg !14
  %has_left_ptr19 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj8, i32 0, i32 4, !dbg !14
  store i1 false, ptr %has_left_ptr19, align 1, !dbg !14
  %has_right_ptr20 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj8, i32 0, i32 5, !dbg !14
  store i1 false, ptr %has_right_ptr20, align 1, !dbg !14
  %TreeNode_val21 = load %TreeNode, ptr %TreeNode_obj8, align 8, !dbg !14
  ret %TreeNode %TreeNode_val21, !dbg !14
}

define %BinaryTree @new_tree() !dbg !18 {
entry:
  %dummy_node = alloca %TreeNode, align 8, !dbg !24
  %TreeNode_obj = alloca %TreeNode, align 8, !dbg !24
  %key_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 0, !dbg !24
  store i64 0, ptr %key_ptr, align 4, !dbg !24
  %value_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 1, !dbg !24
  store ptr @1, ptr %value_ptr, align 8, !dbg !24
  %TreeNode_obj1 = alloca %TreeNode, align 8, !dbg !24
  %TreeNode_val = load %TreeNode, ptr %TreeNode_obj1, align 8, !dbg !24
  %malloc_struct = call ptr @malloc(i64 40), !dbg !24
  store %TreeNode %TreeNode_val, ptr %malloc_struct, align 8, !dbg !24
  %left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 2, !dbg !24
  store ptr %malloc_struct, ptr %left_ptr, align 8, !dbg !24
  %TreeNode_obj2 = alloca %TreeNode, align 8, !dbg !24
  %TreeNode_val3 = load %TreeNode, ptr %TreeNode_obj2, align 8, !dbg !24
  %malloc_struct4 = call ptr @malloc(i64 40), !dbg !24
  store %TreeNode %TreeNode_val3, ptr %malloc_struct4, align 8, !dbg !24
  %right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 3, !dbg !24
  store ptr %malloc_struct4, ptr %right_ptr, align 8, !dbg !24
  %has_left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 4, !dbg !24
  store i1 false, ptr %has_left_ptr, align 1, !dbg !24
  %has_right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 5, !dbg !24
  store i1 false, ptr %has_right_ptr, align 1, !dbg !24
  %TreeNode_val5 = load %TreeNode, ptr %TreeNode_obj, align 8, !dbg !24
  store %TreeNode %TreeNode_val5, ptr %dummy_node, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %dummy_node, metadata !23, metadata !DIExpression()), !dbg !25
  %BinaryTree_obj = alloca %BinaryTree, align 8, !dbg !24
  %dummy_node6 = load %TreeNode, ptr %dummy_node, align 8, !dbg !24
  %malloc_struct7 = call ptr @malloc(i64 40), !dbg !24
  store %TreeNode %dummy_node6, ptr %malloc_struct7, align 8, !dbg !24
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 0, !dbg !24
  store ptr %malloc_struct7, ptr %root_ptr, align 8, !dbg !24
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 1, !dbg !24
  store i1 false, ptr %has_root_ptr, align 1, !dbg !24
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 2, !dbg !24
  store i64 0, ptr %size_ptr, align 4, !dbg !24
  %BinaryTree_val = load %BinaryTree, ptr %BinaryTree_obj, align 8, !dbg !24
  ret %BinaryTree %BinaryTree_val, !dbg !24
}

define void @insert(ptr %tree, i64 %key, ptr %value) !dbg !26 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca ptr, align 8
  store ptr %tree, ptr %tree1, align 8, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !30, metadata !DIExpression()), !dbg !34
  store i64 %key, ptr %key2, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !31, metadata !DIExpression()), !dbg !35
  store ptr %value, ptr %value3, align 8, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !32, metadata !DIExpression()), !dbg !36
  %tree4 = load ptr, ptr %tree1, align 8, !dbg !33
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %tree4, i32 0, i32 1, !dbg !33
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !33
  %nottmp = xor i1 %has_root_val, true, !dbg !33
  br i1 %nottmp, label %then, label %ifcont, !dbg !33

then:                                             ; preds = %entry
  %tree5 = load ptr, ptr %tree1, align 8, !dbg !33
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %tree5, i32 0, i32 0, !dbg !33
  %key6 = load i64, ptr %key2, align 4, !dbg !33
  %value7 = load ptr, ptr %value3, align 8, !dbg !33
  %calltmp = call %TreeNode @create_node(i64 %key6, ptr %value7), !dbg !33
  %malloc_struct = call ptr @malloc(i64 40), !dbg !33
  store %TreeNode %calltmp, ptr %malloc_struct, align 8, !dbg !33
  store ptr %malloc_struct, ptr %root_ptr, align 8, !dbg !33
  %tree8 = load ptr, ptr %tree1, align 8, !dbg !33
  %has_root_ptr9 = getelementptr inbounds %BinaryTree, ptr %tree8, i32 0, i32 1, !dbg !33
  store i1 true, ptr %has_root_ptr9, align 1, !dbg !33
  %tree10 = load ptr, ptr %tree1, align 8, !dbg !33
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %tree10, i32 0, i32 2, !dbg !33
  %tree11 = load ptr, ptr %tree1, align 8, !dbg !33
  %size_ptr12 = getelementptr inbounds %BinaryTree, ptr %tree11, i32 0, i32 2, !dbg !33
  %size_val = load i64, ptr %size_ptr12, align 4, !dbg !33
  %addtmp = add i64 %size_val, 1, !dbg !33
  store i64 %addtmp, ptr %size_ptr, align 4, !dbg !33
  ret void, !dbg !33

ifcont:                                           ; preds = %entry
  %tree13 = load ptr, ptr %tree1, align 8, !dbg !33
  %root_ptr14 = getelementptr inbounds %BinaryTree, ptr %tree13, i32 0, i32 0, !dbg !33
  %key15 = load i64, ptr %key2, align 4, !dbg !33
  %value16 = load ptr, ptr %value3, align 8, !dbg !33
  call void @insert_recursive(ptr %root_ptr14, i64 %key15, ptr %value16), !dbg !33
  %tree17 = load ptr, ptr %tree1, align 8, !dbg !33
  %size_ptr18 = getelementptr inbounds %BinaryTree, ptr %tree17, i32 0, i32 2, !dbg !33
  %tree19 = load ptr, ptr %tree1, align 8, !dbg !33
  %size_ptr20 = getelementptr inbounds %BinaryTree, ptr %tree19, i32 0, i32 2, !dbg !33
  %size_val21 = load i64, ptr %size_ptr20, align 4, !dbg !33
  %addtmp22 = add i64 %size_val21, 1, !dbg !33
  store i64 %addtmp22, ptr %size_ptr18, align 4, !dbg !33
  ret void, !dbg !33
}

define void @insert_recursive(ptr %node, i64 %key, ptr %value) !dbg !37 {
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
  ret ptr @2, !dbg !52

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
  ret ptr @3, !dbg !61

then17:                                           ; preds = %else
  %node19 = load ptr, ptr %node1, align 8, !dbg !61
  %right_ptr = getelementptr inbounds %TreeNode, ptr %node19, i32 0, i32 3, !dbg !61
  %key20 = load i64, ptr %key2, align 4, !dbg !61
  %calltmp21 = call ptr @search_recursive(ptr %right_ptr, i64 %key20), !dbg !61
  ret ptr %calltmp21, !dbg !61

else18:                                           ; preds = %else
  ret ptr @4, !dbg !61
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
  call void @__vyn_println(ptr @5), !dbg !69
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !69
  %temp_struct4 = alloca %BinaryTree, align 8, !dbg !69
  store %BinaryTree %tree3, ptr %temp_struct4, align 8, !dbg !69
  %root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct4, i32 0, i32 0, !dbg !69
  call void @print_in_order(ptr %root_ptr), !dbg !69
  br label %ifcont, !dbg !69

else:                                             ; preds = %entry
  call void @__vyn_println(ptr @6), !dbg !69
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
  %strcattmp = call ptr @__vyn_string_concat(ptr @7, ptr %tostring), !dbg !76
  %strcattmp5 = call ptr @__vyn_string_concat(ptr %strcattmp, ptr @8), !dbg !76
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
  call void @__vyn_println(ptr @9), !dbg !87
  call void @__vyn_println(ptr @10), !dbg !87
  call void @__vyn_println(ptr @11), !dbg !87
  call void @insert(ptr %tree, i64 50, ptr @12), !dbg !87
  call void @insert(ptr %tree, i64 30, ptr @13), !dbg !87
  call void @insert(ptr %tree, i64 70, ptr @14), !dbg !87
  call void @insert(ptr %tree, i64 20, ptr @15), !dbg !87
  call void @insert(ptr %tree, i64 40, ptr @16), !dbg !87
  call void @insert(ptr %tree, i64 60, ptr @17), !dbg !87
  call void @insert(ptr %tree, i64 80, ptr @18), !dbg !87
  %tree1 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %temp_struct = alloca %BinaryTree, align 8, !dbg !87
  store %BinaryTree %tree1, ptr %temp_struct, align 8, !dbg !87
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 2, !dbg !87
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !87
  %tostring = call ptr @__vyn_toString_int(i64 %size_val), !dbg !87
  %strcattmp = call ptr @__vyn_string_concat(ptr @19, ptr %tostring), !dbg !87
  call void @__vyn_println(ptr %strcattmp), !dbg !87
  call void @__vyn_println(ptr @20), !dbg !87
  call void @__vyn_println(ptr @21), !dbg !87
  %tree2 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp3 = call ptr @search(%BinaryTree %tree2, i64 40), !dbg !87
  store ptr %calltmp3, ptr %result1, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !83, metadata !DIExpression()), !dbg !89
  %result14 = load ptr, ptr %result1, align 8, !dbg !87
  %strcattmp5 = call ptr @__vyn_string_concat(ptr @22, ptr %result14), !dbg !87
  call void @__vyn_println(ptr %strcattmp5), !dbg !87
  %tree6 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp7 = call ptr @search(%BinaryTree %tree6, i64 99), !dbg !87
  store ptr %calltmp7, ptr %result2, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !84, metadata !DIExpression()), !dbg !90
  %result28 = load ptr, ptr %result2, align 8, !dbg !87
  %strcattmp9 = call ptr @__vyn_string_concat(ptr @23, ptr %result28), !dbg !87
  call void @__vyn_println(ptr %strcattmp9), !dbg !87
  %tree10 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp11 = call ptr @search(%BinaryTree %tree10, i64 20), !dbg !87
  store ptr %calltmp11, ptr %result3, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !85, metadata !DIExpression()), !dbg !91
  %result312 = load ptr, ptr %result3, align 8, !dbg !87
  %strcattmp13 = call ptr @__vyn_string_concat(ptr @24, ptr %result312), !dbg !87
  call void @__vyn_println(ptr %strcattmp13), !dbg !87
  %tree14 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  %calltmp15 = call ptr @search(%BinaryTree %tree14, i64 70), !dbg !87
  store ptr %calltmp15, ptr %result4, align 8, !dbg !87
  call void @llvm.dbg.declare(metadata ptr %result4, metadata !86, metadata !DIExpression()), !dbg !92
  %result416 = load ptr, ptr %result4, align 8, !dbg !87
  %strcattmp17 = call ptr @__vyn_string_concat(ptr @25, ptr %result416), !dbg !87
  call void @__vyn_println(ptr %strcattmp17), !dbg !87
  call void @__vyn_println(ptr @26), !dbg !87
  %tree18 = load %BinaryTree, ptr %tree, align 8, !dbg !87
  call void @print_tree(%BinaryTree %tree18), !dbg !87
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
!10 = !{!11, !12, !13}
!11 = !DILocalVariable(name: "key", scope: !4, file: !1, line: 23, type: !8)
!12 = !DILocalVariable(name: "value", scope: !4, file: !1, line: 23, type: !9)
!13 = !DILocalVariable(name: "dummy_node", scope: !4, file: !1, line: 24, type: !7)
!14 = !DILocation(line: 22, column: 1, scope: !4)
!15 = !DILocation(line: 23, column: 16, scope: !4)
!16 = !DILocation(line: 23, column: 28, scope: !4)
!17 = !DILocation(line: 24, column: 1, scope: !4)
!18 = distinct !DISubprogram(name: "new_tree", linkageName: "new_tree", scope: !1, file: !1, line: 43, type: !19, scopeLine: 43, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !22)
!19 = !DISubroutineType(types: !20)
!20 = !{!21}
!21 = !DICompositeType(tag: DW_TAG_structure_type, name: "BinaryTree", scope: !1, file: !1, size: 192, align: 8)
!22 = !{!23}
!23 = !DILocalVariable(name: "dummy_node", scope: !18, file: !1, line: 45, type: !7)
!24 = !DILocation(line: 43, column: 1, scope: !18)
!25 = !DILocation(line: 45, column: 1, scope: !18)
!26 = distinct !DISubprogram(name: "insert", linkageName: "insert", scope: !1, file: !1, line: 61, type: !27, scopeLine: 61, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !29)
!27 = !DISubroutineType(types: !28)
!28 = !{null, !9, !8, !9}
!29 = !{!30, !31, !32}
!30 = !DILocalVariable(name: "tree", scope: !26, file: !1, line: 62, type: !9)
!31 = !DILocalVariable(name: "key", scope: !26, file: !1, line: 62, type: !8)
!32 = !DILocalVariable(name: "value", scope: !26, file: !1, line: 62, type: !9)
!33 = !DILocation(line: 61, column: 1, scope: !26)
!34 = !DILocation(line: 62, column: 12, scope: !26)
!35 = !DILocation(line: 62, column: 36, scope: !26)
!36 = !DILocation(line: 62, column: 48, scope: !26)
!37 = distinct !DISubprogram(name: "insert_recursive", linkageName: "insert_recursive", scope: !1, file: !1, line: 74, type: !27, scopeLine: 74, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !38)
!38 = !{!39, !40, !41}
!39 = !DILocalVariable(name: "node", scope: !37, file: !1, line: 75, type: !9)
!40 = !DILocalVariable(name: "key", scope: !37, file: !1, line: 75, type: !8)
!41 = !DILocalVariable(name: "value", scope: !37, file: !1, line: 75, type: !9)
!42 = !DILocation(line: 74, column: 1, scope: !37)
!43 = !DILocation(line: 75, column: 22, scope: !37)
!44 = !DILocation(line: 75, column: 44, scope: !37)
!45 = !DILocation(line: 75, column: 56, scope: !37)
!46 = distinct !DISubprogram(name: "search", linkageName: "search", scope: !1, file: !1, line: 93, type: !47, scopeLine: 93, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !49)
!47 = !DISubroutineType(types: !48)
!48 = !{!9, !21, !8}
!49 = !{!50, !51}
!50 = !DILocalVariable(name: "tree", scope: !46, file: !1, line: 94, type: !21)
!51 = !DILocalVariable(name: "key", scope: !46, file: !1, line: 94, type: !8)
!52 = !DILocation(line: 93, column: 1, scope: !46)
!53 = !DILocation(line: 94, column: 12, scope: !46)
!54 = !DILocation(line: 94, column: 29, scope: !46)
!55 = distinct !DISubprogram(name: "search_recursive", linkageName: "search_recursive", scope: !1, file: !1, line: 102, type: !56, scopeLine: 102, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !58)
!56 = !DISubroutineType(types: !57)
!57 = !{!9, !9, !8}
!58 = !{!59, !60}
!59 = !DILocalVariable(name: "node", scope: !55, file: !1, line: 103, type: !9)
!60 = !DILocalVariable(name: "key", scope: !55, file: !1, line: 103, type: !8)
!61 = !DILocation(line: 102, column: 1, scope: !55)
!62 = !DILocation(line: 103, column: 22, scope: !55)
!63 = !DILocation(line: 103, column: 50, scope: !55)
!64 = distinct !DISubprogram(name: "print_tree", linkageName: "print_tree", scope: !1, file: !1, line: 123, type: !65, scopeLine: 123, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !67)
!65 = !DISubroutineType(types: !66)
!66 = !{null, !21}
!67 = !{!68}
!68 = !DILocalVariable(name: "tree", scope: !64, file: !1, line: 124, type: !21)
!69 = !DILocation(line: 123, column: 1, scope: !64)
!70 = !DILocation(line: 124, column: 16, scope: !64)
!71 = distinct !DISubprogram(name: "print_in_order", linkageName: "print_in_order", scope: !1, file: !1, line: 133, type: !72, scopeLine: 133, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !74)
!72 = !DISubroutineType(types: !73)
!73 = !{null, !9}
!74 = !{!75}
!75 = !DILocalVariable(name: "node", scope: !71, file: !1, line: 134, type: !9)
!76 = !DILocation(line: 133, column: 1, scope: !71)
!77 = !DILocation(line: 134, column: 20, scope: !71)
!78 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 146, type: !79, scopeLine: 146, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !81)
!79 = !DISubroutineType(types: !80)
!80 = !{!8}
!81 = !{!82, !83, !84, !85, !86}
!82 = !DILocalVariable(name: "tree", scope: !78, file: !1, line: 148, type: !21)
!83 = !DILocalVariable(name: "result1", scope: !78, file: !1, line: 169, type: !9)
!84 = !DILocalVariable(name: "result2", scope: !78, file: !1, line: 172, type: !9)
!85 = !DILocalVariable(name: "result3", scope: !78, file: !1, line: 175, type: !9)
!86 = !DILocalVariable(name: "result4", scope: !78, file: !1, line: 178, type: !9)
!87 = !DILocation(line: 146, column: 1, scope: !78)
!88 = !DILocation(line: 148, column: 5, scope: !78)
!89 = !DILocation(line: 169, column: 1, scope: !78)
!90 = !DILocation(line: 172, column: 1, scope: !78)
!91 = !DILocation(line: 175, column: 1, scope: !78)
!92 = !DILocation(line: 178, column: 1, scope: !78)
