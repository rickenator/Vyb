; ModuleID = 'VynModule'
source_filename = "VynModule"

%TreeNode = type { i64, i64, i64 }
%BinaryTree = type { { ptr, i64, i64 } }

define i64 @main() !dbg !4 {
entry:
  %root = alloca %TreeNode, align 8, !dbg !16
  %node3 = alloca %TreeNode, align 8, !dbg !16
  %node2 = alloca %TreeNode, align 8, !dbg !16
  %node1 = alloca %TreeNode, align 8, !dbg !16
  %tree = alloca %BinaryTree, align 8, !dbg !16
  %BinaryTree_obj = alloca %BinaryTree, align 8, !dbg !16
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !16
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !16
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !16
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !16
  store i64 0, ptr %vec.size_field, align 4, !dbg !16
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !16
  store i64 0, ptr %vec.cap_field, align 4, !dbg !16
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !16
  %nodes_ptr = getelementptr inbounds %BinaryTree, ptr %BinaryTree_obj, i32 0, i32 0, !dbg !16
  store { ptr, i64, i64 } %vec.new.value, ptr %nodes_ptr, align 8, !dbg !16
  %BinaryTree_val = load %BinaryTree, ptr %BinaryTree_obj, align 8, !dbg !16
  store %BinaryTree %BinaryTree_val, ptr %tree, align 8, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %tree, metadata !9, metadata !DIExpression()), !dbg !17
  %TreeNode_obj = alloca %TreeNode, align 8, !dbg !16
  %value_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 0, !dbg !16
  store i64 10, ptr %value_ptr, align 4, !dbg !16
  %left_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 1, !dbg !16
  store i64 -1, ptr %left_ptr, align 4, !dbg !16
  %right_ptr = getelementptr inbounds %TreeNode, ptr %TreeNode_obj, i32 0, i32 2, !dbg !16
  store i64 -1, ptr %right_ptr, align 4, !dbg !16
  %TreeNode_val = load %TreeNode, ptr %TreeNode_obj, align 4, !dbg !16
  store %TreeNode %TreeNode_val, ptr %node1, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %node1, metadata !11, metadata !DIExpression()), !dbg !18
  %TreeNode_obj1 = alloca %TreeNode, align 8, !dbg !16
  %value_ptr2 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj1, i32 0, i32 0, !dbg !16
  store i64 5, ptr %value_ptr2, align 4, !dbg !16
  %left_ptr3 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj1, i32 0, i32 1, !dbg !16
  store i64 -1, ptr %left_ptr3, align 4, !dbg !16
  %right_ptr4 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj1, i32 0, i32 2, !dbg !16
  store i64 -1, ptr %right_ptr4, align 4, !dbg !16
  %TreeNode_val5 = load %TreeNode, ptr %TreeNode_obj1, align 4, !dbg !16
  store %TreeNode %TreeNode_val5, ptr %node2, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %node2, metadata !13, metadata !DIExpression()), !dbg !19
  %TreeNode_obj6 = alloca %TreeNode, align 8, !dbg !16
  %value_ptr7 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj6, i32 0, i32 0, !dbg !16
  store i64 15, ptr %value_ptr7, align 4, !dbg !16
  %left_ptr8 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj6, i32 0, i32 1, !dbg !16
  store i64 -1, ptr %left_ptr8, align 4, !dbg !16
  %right_ptr9 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj6, i32 0, i32 2, !dbg !16
  store i64 -1, ptr %right_ptr9, align 4, !dbg !16
  %TreeNode_val10 = load %TreeNode, ptr %TreeNode_obj6, align 4, !dbg !16
  store %TreeNode %TreeNode_val10, ptr %node3, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %node3, metadata !14, metadata !DIExpression()), !dbg !20
  %TreeNode_obj11 = alloca %TreeNode, align 8, !dbg !16
  %value_ptr12 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj11, i32 0, i32 0, !dbg !16
  store i64 10, ptr %value_ptr12, align 4, !dbg !16
  %left_ptr13 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj11, i32 0, i32 1, !dbg !16
  store i64 1, ptr %left_ptr13, align 4, !dbg !16
  %right_ptr14 = getelementptr inbounds %TreeNode, ptr %TreeNode_obj11, i32 0, i32 2, !dbg !16
  store i64 2, ptr %right_ptr14, align 4, !dbg !16
  %TreeNode_val15 = load %TreeNode, ptr %TreeNode_obj11, align 4, !dbg !16
  store %TreeNode %TreeNode_val15, ptr %root, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %root, metadata !15, metadata !DIExpression()), !dbg !21
  ret i64 0, !dbg !16
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @println(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "binary_tree_fixed.vyn.ll", directory: "/home/rick/Projects/Vyn/examples")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !13, !14, !15}
!9 = !DILocalVariable(name: "tree", scope: !4, file: !1, line: 2, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "BinaryTree", scope: !1, file: !1, size: 64, align: 8)
!11 = !DILocalVariable(name: "node1", scope: !4, file: !1, line: 4, type: !12)
!12 = !DICompositeType(tag: DW_TAG_structure_type, name: "TreeNode", scope: !1, file: !1, size: 192, align: 8)
!13 = !DILocalVariable(name: "node2", scope: !4, file: !1, line: 8, type: !12)
!14 = !DILocalVariable(name: "node3", scope: !4, file: !1, line: 11, type: !12)
!15 = !DILocalVariable(name: "root", scope: !4, file: !1, line: 14, type: !12)
!16 = !DILocation(line: 1, column: 1, scope: !4)
!17 = !DILocation(line: 2, column: 1, scope: !4)
!18 = !DILocation(line: 4, column: 5, scope: !4)
!19 = !DILocation(line: 8, column: 1, scope: !4)
!20 = !DILocation(line: 11, column: 1, scope: !4)
!21 = !DILocation(line: 14, column: 5, scope: !4)
