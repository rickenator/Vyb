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
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %tree6, i32 0, i32 1, !dbg !20
  store i1 true, ptr %has_root_ptr, align 1, !dbg !20
  %tree7 = load ptr, ptr %tree1, align 8, !dbg !20
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %tree7, i32 0, i32 2, !dbg !20
  %tree8 = load ptr, ptr %tree1, align 8, !dbg !20
  %size_ptr9 = getelementptr inbounds %BinaryTree, ptr %tree8, i32 0, i32 2, !dbg !20
  %size_val = load i64, ptr %size_ptr9, align 4, !dbg !20
  %addtmp = add i64 %size_val, 1, !dbg !20
  store i64 %addtmp, ptr %size_ptr, align 4, !dbg !20
  ret void, !dbg !20
}

define ptr @search(%BinaryTree %tree, i64 %key) !dbg !25 {
entry:
  %i = alloca i64, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !32
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !29, metadata !DIExpression()), !dbg !33
  store i64 %key, ptr %key2, align 4, !dbg !32
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !30, metadata !DIExpression()), !dbg !34
  %tree3 = load %BinaryTree, ptr %tree1, align 8, !dbg !32
  %temp_struct = alloca %BinaryTree, align 8, !dbg !32
  store %BinaryTree %tree3, ptr %temp_struct, align 8, !dbg !32
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !32
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !32
  %nottmp = xor i1 %has_root_val, true, !dbg !32
  br i1 %nottmp, label %then, label %ifcont, !dbg !32

then:                                             ; preds = %entry
  ret ptr @0, !dbg !32

ifcont:                                           ; preds = %entry
  store i64 0, ptr %i, align 4, !dbg !32
  call void @llvm.dbg.declare(metadata ptr %i, metadata !31, metadata !DIExpression()), !dbg !35
  br label %loop.header, !dbg !32

loop.header:                                      ; preds = %loop.body, %ifcont
  %i4 = load i64, ptr %i, align 4, !dbg !32
  %tree5 = load %BinaryTree, ptr %tree1, align 8, !dbg !32
  %temp_struct6 = alloca %BinaryTree, align 8, !dbg !32
  store %BinaryTree %tree5, ptr %temp_struct6, align 8, !dbg !32
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct6, i32 0, i32 2, !dbg !32
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !32
  %icmpslttmp = icmp slt i64 %i4, %size_val, !dbg !32
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !32

loop.body:                                        ; preds = %loop.header
  %key7 = load i64, ptr %key2, align 4, !dbg !32
  %i8 = load i64, ptr %i, align 4, !dbg !32
  %addtmp = add i64 %i8, 1, !dbg !32
  store i64 %addtmp, ptr %i, align 4, !dbg !32
  br label %loop.header, !dbg !32

loop.exit:                                        ; preds = %loop.header
  ret ptr @1, !dbg !32
}

define void @print_tree(%BinaryTree %tree) !dbg !36 {
entry:
  %i = alloca i64, align 8
  %tree1 = alloca %BinaryTree, align 8
  store %BinaryTree %tree, ptr %tree1, align 8, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !40, metadata !DIExpression()), !dbg !43
  %tree2 = load %BinaryTree, ptr %tree1, align 8, !dbg !42
  %temp_struct = alloca %BinaryTree, align 8, !dbg !42
  store %BinaryTree %tree2, ptr %temp_struct, align 8, !dbg !42
  %has_root_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 1, !dbg !42
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !42
  %nottmp = xor i1 %has_root_val, true, !dbg !42
  br i1 %nottmp, label %then, label %ifcont, !dbg !42

then:                                             ; preds = %entry
  call void @__vyn_println(ptr @2), !dbg !42
  ret void, !dbg !42

ifcont:                                           ; preds = %entry
  call void @__vyn_println(ptr @3), !dbg !42
  store i64 0, ptr %i, align 4, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %i, metadata !41, metadata !DIExpression()), !dbg !44
  br label %loop.header, !dbg !42

loop.header:                                      ; preds = %loop.body, %ifcont
  %i3 = load i64, ptr %i, align 4, !dbg !42
  %tree4 = load %BinaryTree, ptr %tree1, align 8, !dbg !42
  %temp_struct5 = alloca %BinaryTree, align 8, !dbg !42
  store %BinaryTree %tree4, ptr %temp_struct5, align 8, !dbg !42
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct5, i32 0, i32 2, !dbg !42
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !42
  %icmpslttmp = icmp slt i64 %i3, %size_val, !dbg !42
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !42

loop.body:                                        ; preds = %loop.header
  %i6 = load i64, ptr %i, align 4, !dbg !42
  %addtmp = add i64 %i6, 1, !dbg !42
  store i64 %addtmp, ptr %i, align 4, !dbg !42
  br label %loop.header, !dbg !42

loop.exit:                                        ; preds = %loop.header
  ret void, !dbg !42
}

define i64 @main() !dbg !45 {
entry:
  %result3 = alloca ptr, align 8, !dbg !53
  %result2 = alloca ptr, align 8, !dbg !53
  %result1 = alloca ptr, align 8, !dbg !53
  %tree = alloca %BinaryTree, align 8, !dbg !53
  %calltmp = call %BinaryTree @new_tree(), !dbg !53
  store %BinaryTree %calltmp, ptr %tree, align 8, !dbg !53
  call void @llvm.dbg.declare(metadata ptr %tree, metadata !49, metadata !DIExpression()), !dbg !54
  call void @__vyn_println(ptr @6), !dbg !53
  call void @__vyn_println(ptr @7), !dbg !53
  call void @__vyn_println(ptr @8), !dbg !53
  call void @insert(ptr %tree, i64 50, ptr @9), !dbg !53
  call void @insert(ptr %tree, i64 30, ptr @10), !dbg !53
  call void @insert(ptr %tree, i64 70, ptr @11), !dbg !53
  call void @insert(ptr %tree, i64 20, ptr @12), !dbg !53
  call void @insert(ptr %tree, i64 40, ptr @13), !dbg !53
  %tree1 = load %BinaryTree, ptr %tree, align 8, !dbg !53
  %temp_struct = alloca %BinaryTree, align 8, !dbg !53
  store %BinaryTree %tree1, ptr %temp_struct, align 8, !dbg !53
  %size_ptr = getelementptr inbounds %BinaryTree, ptr %temp_struct, i32 0, i32 2, !dbg !53
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !53
  %tostring = call ptr @__vyn_toString_int(i64 %size_val), !dbg !53
  %strcattmp = call ptr @__vyn_string_concat(ptr @14, ptr %tostring), !dbg !53
  call void @__vyn_println(ptr %strcattmp), !dbg !53
  call void @__vyn_println(ptr @15), !dbg !53
  call void @__vyn_println(ptr @16), !dbg !53
  %tree2 = load %BinaryTree, ptr %tree, align 8, !dbg !53
  %calltmp3 = call ptr @search(%BinaryTree %tree2, i64 40), !dbg !53
  store ptr %calltmp3, ptr %result1, align 8, !dbg !53
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !50, metadata !DIExpression()), !dbg !55
  %result14 = load ptr, ptr %result1, align 8, !dbg !53
  %strcattmp5 = call ptr @__vyn_string_concat(ptr @17, ptr %result14), !dbg !53
  call void @__vyn_println(ptr %strcattmp5), !dbg !53
  %tree6 = load %BinaryTree, ptr %tree, align 8, !dbg !53
  %calltmp7 = call ptr @search(%BinaryTree %tree6, i64 99), !dbg !53
  store ptr %calltmp7, ptr %result2, align 8, !dbg !53
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !51, metadata !DIExpression()), !dbg !56
  %result28 = load ptr, ptr %result2, align 8, !dbg !53
  %strcattmp9 = call ptr @__vyn_string_concat(ptr @18, ptr %result28), !dbg !53
  call void @__vyn_println(ptr %strcattmp9), !dbg !53
  %tree10 = load %BinaryTree, ptr %tree, align 8, !dbg !53
  %calltmp11 = call ptr @search(%BinaryTree %tree10, i64 20), !dbg !53
  store ptr %calltmp11, ptr %result3, align 8, !dbg !53
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !52, metadata !DIExpression()), !dbg !57
  %result312 = load ptr, ptr %result3, align 8, !dbg !53
  %strcattmp13 = call ptr @__vyn_string_concat(ptr @19, ptr %result312), !dbg !53
  call void @__vyn_println(ptr %strcattmp13), !dbg !53
  call void @__vyn_println(ptr @20), !dbg !53
  %tree14 = load %BinaryTree, ptr %tree, align 8, !dbg !53
  call void @print_tree(%BinaryTree %tree14), !dbg !53
  ret i64 0, !dbg !53
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

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
!28 = !{!29, !30, !31}
!29 = !DILocalVariable(name: "tree", scope: !25, file: !1, line: 35, type: !7)
!30 = !DILocalVariable(name: "key", scope: !25, file: !1, line: 35, type: !13)
!31 = !DILocalVariable(name: "i", scope: !25, file: !1, line: 40, type: !13)
!32 = !DILocation(line: 34, column: 1, scope: !25)
!33 = !DILocation(line: 35, column: 12, scope: !25)
!34 = !DILocation(line: 35, column: 29, scope: !25)
!35 = !DILocation(line: 40, column: 1, scope: !25)
!36 = distinct !DISubprogram(name: "print_tree", linkageName: "print_tree", scope: !1, file: !1, line: 52, type: !37, scopeLine: 52, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !39)
!37 = !DISubroutineType(types: !38)
!38 = !{null, !7}
!39 = !{!40, !41}
!40 = !DILocalVariable(name: "tree", scope: !36, file: !1, line: 53, type: !7)
!41 = !DILocalVariable(name: "i", scope: !36, file: !1, line: 60, type: !13)
!42 = !DILocation(line: 52, column: 1, scope: !36)
!43 = !DILocation(line: 53, column: 16, scope: !36)
!44 = !DILocation(line: 60, column: 1, scope: !36)
!45 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 68, type: !46, scopeLine: 68, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !48)
!46 = !DISubroutineType(types: !47)
!47 = !{!13}
!48 = !{!49, !50, !51, !52}
!49 = !DILocalVariable(name: "tree", scope: !45, file: !1, line: 70, type: !7)
!50 = !DILocalVariable(name: "result1", scope: !45, file: !1, line: 89, type: !12)
!51 = !DILocalVariable(name: "result2", scope: !45, file: !1, line: 92, type: !12)
!52 = !DILocalVariable(name: "result3", scope: !45, file: !1, line: 95, type: !12)
!53 = !DILocation(line: 68, column: 1, scope: !45)
!54 = !DILocation(line: 70, column: 5, scope: !45)
!55 = !DILocation(line: 89, column: 1, scope: !45)
!56 = !DILocation(line: 92, column: 1, scope: !45)
!57 = !DILocation(line: 95, column: 1, scope: !45)
