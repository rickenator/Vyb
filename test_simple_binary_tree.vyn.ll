; ModuleID = 'VynModule'
source_filename = "VynModule"

%SimpleTree = type { i1, i64 }

@0 = private unnamed_addr constant [20 x i8] c"Initial tree state:\00", align 1
@1 = private unnamed_addr constant [11 x i8] c"Has root: \00", align 1
@2 = private unnamed_addr constant [7 x i8] c"Size: \00", align 1
@3 = private unnamed_addr constant [6 x i8] c"fifty\00", align 1
@4 = private unnamed_addr constant [7 x i8] c"thirty\00", align 1
@5 = private unnamed_addr constant [18 x i8] c"After insertions:\00", align 1
@6 = private unnamed_addr constant [11 x i8] c"Has root: \00", align 1
@7 = private unnamed_addr constant [7 x i8] c"Size: \00", align 1

define %SimpleTree @create_simple_tree() !dbg !4 {
entry:
  %SimpleTree_obj = alloca %SimpleTree, align 8, !dbg !8
  %has_root_ptr = getelementptr inbounds %SimpleTree, ptr %SimpleTree_obj, i32 0, i32 0, !dbg !8
  store i1 false, ptr %has_root_ptr, align 1, !dbg !8
  %size_ptr = getelementptr inbounds %SimpleTree, ptr %SimpleTree_obj, i32 0, i32 1, !dbg !8
  store i64 0, ptr %size_ptr, align 4, !dbg !8
  %SimpleTree_val = load %SimpleTree, ptr %SimpleTree_obj, align 4, !dbg !8
  ret %SimpleTree %SimpleTree_val, !dbg !8
}

define void @insert_value(ptr %tree, i64 %key, ptr %value) !dbg !9 {
entry:
  %value3 = alloca ptr, align 8
  %key2 = alloca i64, align 8
  %tree1 = alloca ptr, align 8
  store ptr %tree, ptr %tree1, align 8, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %tree1, metadata !15, metadata !DIExpression()), !dbg !19
  store i64 %key, ptr %key2, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %key2, metadata !16, metadata !DIExpression()), !dbg !20
  store ptr %value, ptr %value3, align 8, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %value3, metadata !17, metadata !DIExpression()), !dbg !21
  %tree4 = load ptr, ptr %tree1, align 8, !dbg !18
  %has_root_ptr = getelementptr inbounds %SimpleTree, ptr %tree4, i32 0, i32 0, !dbg !18
  store i1 true, ptr %has_root_ptr, align 1, !dbg !18
  %tree5 = load ptr, ptr %tree1, align 8, !dbg !18
  %size_ptr = getelementptr inbounds %SimpleTree, ptr %tree5, i32 0, i32 1, !dbg !18
  %tree6 = load ptr, ptr %tree1, align 8, !dbg !18
  %size_ptr7 = getelementptr inbounds %SimpleTree, ptr %tree6, i32 0, i32 1, !dbg !18
  %size_val = load i64, ptr %size_ptr7, align 4, !dbg !18
  %addtmp = add i64 %size_val, 1, !dbg !18
  store i64 %addtmp, ptr %size_ptr, align 4, !dbg !18
  ret void, !dbg !18
}

define i64 @main() !dbg !22 {
entry:
  %tree = alloca %SimpleTree, align 8, !dbg !27
  %calltmp = call %SimpleTree @create_simple_tree(), !dbg !27
  store %SimpleTree %calltmp, ptr %tree, align 4, !dbg !27
  call void @llvm.dbg.declare(metadata ptr %tree, metadata !26, metadata !DIExpression()), !dbg !28
  call void @__vyn_println(ptr @0), !dbg !27
  %tree1 = load %SimpleTree, ptr %tree, align 4, !dbg !27
  %temp_struct = alloca %SimpleTree, align 8, !dbg !27
  store %SimpleTree %tree1, ptr %temp_struct, align 4, !dbg !27
  %has_root_ptr = getelementptr inbounds %SimpleTree, ptr %temp_struct, i32 0, i32 0, !dbg !27
  %has_root_val = load i1, ptr %has_root_ptr, align 1, !dbg !27
  %tostring = call ptr @__vyn_toString_bool(i1 %has_root_val), !dbg !27
  %strcattmp = call ptr @__vyn_string_concat(ptr @1, ptr %tostring), !dbg !27
  call void @__vyn_println(ptr %strcattmp), !dbg !27
  %tree2 = load %SimpleTree, ptr %tree, align 4, !dbg !27
  %temp_struct3 = alloca %SimpleTree, align 8, !dbg !27
  store %SimpleTree %tree2, ptr %temp_struct3, align 4, !dbg !27
  %size_ptr = getelementptr inbounds %SimpleTree, ptr %temp_struct3, i32 0, i32 1, !dbg !27
  %size_val = load i64, ptr %size_ptr, align 4, !dbg !27
  %tostring4 = call ptr @__vyn_toString_int(i64 %size_val), !dbg !27
  %strcattmp5 = call ptr @__vyn_string_concat(ptr @2, ptr %tostring4), !dbg !27
  call void @__vyn_println(ptr %strcattmp5), !dbg !27
  call void @insert_value(ptr %tree, i64 50, ptr @3), !dbg !27
  call void @insert_value(ptr %tree, i64 30, ptr @4), !dbg !27
  call void @__vyn_println(ptr @5), !dbg !27
  %tree6 = load %SimpleTree, ptr %tree, align 4, !dbg !27
  %temp_struct7 = alloca %SimpleTree, align 8, !dbg !27
  store %SimpleTree %tree6, ptr %temp_struct7, align 4, !dbg !27
  %has_root_ptr8 = getelementptr inbounds %SimpleTree, ptr %temp_struct7, i32 0, i32 0, !dbg !27
  %has_root_val9 = load i1, ptr %has_root_ptr8, align 1, !dbg !27
  %tostring10 = call ptr @__vyn_toString_bool(i1 %has_root_val9), !dbg !27
  %strcattmp11 = call ptr @__vyn_string_concat(ptr @6, ptr %tostring10), !dbg !27
  call void @__vyn_println(ptr %strcattmp11), !dbg !27
  %tree12 = load %SimpleTree, ptr %tree, align 4, !dbg !27
  %temp_struct13 = alloca %SimpleTree, align 8, !dbg !27
  store %SimpleTree %tree12, ptr %temp_struct13, align 4, !dbg !27
  %size_ptr14 = getelementptr inbounds %SimpleTree, ptr %temp_struct13, i32 0, i32 1, !dbg !27
  %size_val15 = load i64, ptr %size_ptr14, align 4, !dbg !27
  %tostring16 = call ptr @__vyn_toString_int(i64 %size_val15), !dbg !27
  %strcattmp17 = call ptr @__vyn_string_concat(ptr @7, ptr %tostring16), !dbg !27
  call void @__vyn_println(ptr %strcattmp17), !dbg !27
  ret i64 0, !dbg !27
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @__vyn_println(ptr)

declare ptr @__vyn_toString_bool(i1)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_simple_binary_tree.vyn.ll", directory: "/home/rick/Projects/Vyn")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "create_simple_tree", linkageName: "create_simple_tree", scope: !1, file: !1, line: 14, type: !5, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "SimpleTree", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 14, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "insert_value", linkageName: "insert_value", scope: !1, file: !1, line: 21, type: !10, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!10 = !DISubroutineType(types: !11)
!11 = !{null, !12, !13, !12}
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!13 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!14 = !{!15, !16, !17}
!15 = !DILocalVariable(name: "tree", scope: !9, file: !1, line: 21, type: !12)
!16 = !DILocalVariable(name: "key", scope: !9, file: !1, line: 21, type: !13)
!17 = !DILocalVariable(name: "value", scope: !9, file: !1, line: 21, type: !12)
!18 = !DILocation(line: 21, column: 1, scope: !9)
!19 = !DILocation(line: 21, column: 18, scope: !9)
!20 = !DILocation(line: 21, column: 42, scope: !9)
!21 = !DILocation(line: 21, column: 54, scope: !9)
!22 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 26, type: !23, scopeLine: 26, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !25)
!23 = !DISubroutineType(types: !24)
!24 = !{!13}
!25 = !{!26}
!26 = !DILocalVariable(name: "tree", scope: !22, file: !1, line: 27, type: !7)
!27 = !DILocation(line: 26, column: 1, scope: !22)
!28 = !DILocation(line: 27, column: 1, scope: !22)
