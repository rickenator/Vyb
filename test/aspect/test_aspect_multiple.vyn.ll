; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { i64, i64 }

@0 = private unnamed_addr constant [12 x i8] c"Point(x, y)\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [25 x i8] c"Point { x: ..., y: ... }\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [30 x i8] c"Multiple aspects test passed!\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @main() !dbg !4 {
entry:
  %p2 = alloca %Point, align 8, !dbg !12
  %p = alloca %Point, align 8, !dbg !12
  %Point_obj = alloca %Point, align 8, !dbg !12
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0, !dbg !12
  store i64 10, ptr %x_ptr, align 4, !dbg !12
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1, !dbg !12
  store i64 20, ptr %y_ptr, align 4, !dbg !12
  %Point_val = load %Point, ptr %Point_obj, align 4, !dbg !12
  store %Point %Point_val, ptr %p, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %p, metadata !9, metadata !DIExpression()), !dbg !13
  %p.load = load %Point, ptr %p, align 4, !dbg !12
  call void @Point_show(%Point %p.load), !dbg !12
  %p.load1 = load %Point, ptr %p, align 4, !dbg !12
  call void @Point_debug(%Point %p.load1), !dbg !12
  %p.load2 = load %Point, ptr %p, align 4, !dbg !12
  %aspect.method.result = call %Point @Point_clone(%Point %p.load2), !dbg !12
  store %Point %aspect.method.result, ptr %p2, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %p2, metadata !11, metadata !DIExpression()), !dbg !14
  %p2.load = load %Point, ptr %p2, align 4, !dbg !12
  call void @Point_show(%Point %p2.load), !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @2, i64 29 }, ptr %serialize_temp, align 8, !dbg !12
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.2), !dbg !12
  call void @__vyn_println(ptr %serialized_json), !dbg !12
  ret i64 0, !dbg !12
}

define void @Point_show(%Point %self) !dbg !15 {
entry:
  %self1 = alloca %Point, align 8
  store %Point %self, ptr %self1, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !19, metadata !DIExpression()), !dbg !21
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !20
  store { ptr, i64 } { ptr @0, i64 11 }, ptr %serialize_temp, align 8, !dbg !20
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !20
  call void @__vyn_println(ptr %serialized_json), !dbg !20
  ret void, !dbg !20
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

define void @Point_debug(%Point %self) !dbg !22 {
entry:
  %self1 = alloca %Point, align 8
  store %Point %self, ptr %self1, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !24, metadata !DIExpression()), !dbg !26
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !25
  store { ptr, i64 } { ptr @1, i64 24 }, ptr %serialize_temp, align 8, !dbg !25
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.1), !dbg !25
  call void @__vyn_println(ptr %serialized_json), !dbg !25
  ret void, !dbg !25
}

define %Point @Point_clone(%Point %self) !dbg !27 {
entry:
  %result = alloca %Point, align 8
  %self1 = alloca %Point, align 8
  store %Point %self, ptr %self1, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !31, metadata !DIExpression()), !dbg !34
  %Point_obj = alloca %Point, align 8, !dbg !33
  %self2 = load %Point, ptr %self1, align 4, !dbg !33
  %temp_struct = alloca %Point, align 8, !dbg !33
  store %Point %self2, ptr %temp_struct, align 4, !dbg !33
  %x_ptr = getelementptr inbounds %Point, ptr %temp_struct, i32 0, i32 0, !dbg !33
  %x_val = load i64, ptr %x_ptr, align 4, !dbg !33
  %x_ptr3 = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0, !dbg !33
  store i64 %x_val, ptr %x_ptr3, align 4, !dbg !33
  %self4 = load %Point, ptr %self1, align 4, !dbg !33
  %temp_struct5 = alloca %Point, align 8, !dbg !33
  store %Point %self4, ptr %temp_struct5, align 4, !dbg !33
  %y_ptr = getelementptr inbounds %Point, ptr %temp_struct5, i32 0, i32 1, !dbg !33
  %y_val = load i64, ptr %y_ptr, align 4, !dbg !33
  %y_ptr6 = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1, !dbg !33
  store i64 %y_val, ptr %y_ptr6, align 4, !dbg !33
  %Point_val = load %Point, ptr %Point_obj, align 4, !dbg !33
  store %Point %Point_val, ptr %result, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %result, metadata !32, metadata !DIExpression()), !dbg !35
  %result7 = load %Point, ptr %result, align 4, !dbg !33
  ret %Point %result7, !dbg !33
}

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_aspect_multiple.vyn.ll", directory: "/home/rick/Projects/Vyn/test/aspect")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 43, type: !5, scopeLine: 43, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "p", scope: !4, file: !1, line: 44, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "Point", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocalVariable(name: "p2", scope: !4, file: !1, line: 48, type: !10)
!12 = !DILocation(line: 43, column: 1, scope: !4)
!13 = !DILocation(line: 44, column: 1, scope: !4)
!14 = !DILocation(line: 48, column: 20, scope: !4)
!15 = distinct !DISubprogram(name: "Point_show", linkageName: "Point_show", scope: !1, file: !1, line: 23, type: !16, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!16 = !DISubroutineType(types: !17)
!17 = !{null, !10}
!18 = !{!19}
!19 = !DILocalVariable(name: "self", scope: !15, file: !1, line: 23, type: !10)
!20 = !DILocation(line: 23, column: 1, scope: !15)
!21 = !DILocation(line: 23, column: 14, scope: !15)
!22 = distinct !DISubprogram(name: "Point_debug", linkageName: "Point_debug", scope: !1, file: !1, line: 30, type: !16, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !23)
!23 = !{!24}
!24 = !DILocalVariable(name: "self", scope: !22, file: !1, line: 30, type: !10)
!25 = !DILocation(line: 30, column: 1, scope: !22)
!26 = !DILocation(line: 30, column: 15, scope: !22)
!27 = distinct !DISubprogram(name: "Point_clone", linkageName: "Point_clone", scope: !1, file: !1, line: 37, type: !28, scopeLine: 37, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !30)
!28 = !DISubroutineType(types: !29)
!29 = !{!10, !10}
!30 = !{!31, !32}
!31 = !DILocalVariable(name: "self", scope: !27, file: !1, line: 37, type: !10)
!32 = !DILocalVariable(name: "result", scope: !27, file: !1, line: 38, type: !10)
!33 = !DILocation(line: 37, column: 1, scope: !27)
!34 = !DILocation(line: 37, column: 15, scope: !27)
!35 = !DILocation(line: 38, column: 1, scope: !27)
