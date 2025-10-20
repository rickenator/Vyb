; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { i64, i64 }

@0 = private unnamed_addr constant [12 x i8] c"Point(x, y)\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [27 x i8] c"Aspect bounds test passed!\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @main() !dbg !4 {
entry:
  %p = alloca %Point, align 8, !dbg !11
  %Point_obj = alloca %Point, align 8, !dbg !11
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0, !dbg !11
  store i64 5, ptr %x_ptr, align 4, !dbg !11
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1, !dbg !11
  store i64 10, ptr %y_ptr, align 4, !dbg !11
  %Point_val = load %Point, ptr %Point_obj, align 4, !dbg !11
  store %Point %Point_val, ptr %p, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %p, metadata !9, metadata !DIExpression()), !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !11
  store { ptr, i64 } { ptr @1, i64 26 }, ptr %serialize_temp, align 8, !dbg !11
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.1), !dbg !11
  call void @__vyn_println(ptr %serialized_json), !dbg !11
  ret i64 0, !dbg !11
}

define void @Point_show(%Point %self) !dbg !13 {
entry:
  %self1 = alloca %Point, align 8
  store %Point %self, ptr %self1, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !17, metadata !DIExpression()), !dbg !19
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !18
  store { ptr, i64 } { ptr @0, i64 11 }, ptr %serialize_temp, align 8, !dbg !18
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !18
  call void @__vyn_println(ptr %serialized_json), !dbg !18
  ret void, !dbg !18
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

define %Point @Point_clone(%Point %self) !dbg !20 {
entry:
  %result = alloca %Point, align 8
  %self1 = alloca %Point, align 8
  store %Point %self, ptr %self1, align 4, !dbg !26
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !24, metadata !DIExpression()), !dbg !27
  %Point_obj = alloca %Point, align 8, !dbg !26
  %self2 = load %Point, ptr %self1, align 4, !dbg !26
  %temp_struct = alloca %Point, align 8, !dbg !26
  store %Point %self2, ptr %temp_struct, align 4, !dbg !26
  %x_ptr = getelementptr inbounds %Point, ptr %temp_struct, i32 0, i32 0, !dbg !26
  %x_val = load i64, ptr %x_ptr, align 4, !dbg !26
  %x_ptr3 = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0, !dbg !26
  store i64 %x_val, ptr %x_ptr3, align 4, !dbg !26
  %self4 = load %Point, ptr %self1, align 4, !dbg !26
  %temp_struct5 = alloca %Point, align 8, !dbg !26
  store %Point %self4, ptr %temp_struct5, align 4, !dbg !26
  %y_ptr = getelementptr inbounds %Point, ptr %temp_struct5, i32 0, i32 1, !dbg !26
  %y_val = load i64, ptr %y_ptr, align 4, !dbg !26
  %y_ptr6 = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1, !dbg !26
  store i64 %y_val, ptr %y_ptr6, align 4, !dbg !26
  %Point_val = load %Point, ptr %Point_obj, align 4, !dbg !26
  store %Point %Point_val, ptr %result, align 4, !dbg !26
  call void @llvm.dbg.declare(metadata ptr %result, metadata !25, metadata !DIExpression()), !dbg !28
  %result7 = load %Point, ptr %result, align 4, !dbg !26
  ret %Point %result7, !dbg !26
}

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_aspect_bounds.vyn.ll", directory: "/home/rick/Projects/Vyn/test/aspect")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 146, type: !5, scopeLine: 146, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "p", scope: !4, file: !1, line: 147, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "Point", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocation(line: 146, column: 1, scope: !4)
!12 = !DILocation(line: 147, column: 1, scope: !4)
!13 = distinct !DISubprogram(name: "Point_show", linkageName: "Point_show", scope: !1, file: !1, line: 86, type: !14, scopeLine: 86, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{null, !10}
!16 = !{!17}
!17 = !DILocalVariable(name: "self", scope: !13, file: !1, line: 86, type: !10)
!18 = !DILocation(line: 86, column: 1, scope: !13)
!19 = !DILocation(line: 86, column: 14, scope: !13)
!20 = distinct !DISubprogram(name: "Point_clone", linkageName: "Point_clone", scope: !1, file: !1, line: 93, type: !21, scopeLine: 93, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !23)
!21 = !DISubroutineType(types: !22)
!22 = !{!10, !10}
!23 = !{!24, !25}
!24 = !DILocalVariable(name: "self", scope: !20, file: !1, line: 93, type: !10)
!25 = !DILocalVariable(name: "result", scope: !20, file: !1, line: 94, type: !10)
!26 = !DILocation(line: 93, column: 1, scope: !20)
!27 = !DILocation(line: 93, column: 15, scope: !20)
!28 = !DILocation(line: 94, column: 1, scope: !20)
