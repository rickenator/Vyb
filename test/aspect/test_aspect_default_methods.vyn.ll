; ModuleID = 'VynModule'
source_filename = "VynModule"

%Robot = type { i64 }
%Person = type { { ptr, i64 } }

@0 = private unnamed_addr constant [20 x i8] c"Person says goodbye\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [17 x i8] c"Robot says hello\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [19 x i8] c"Robot says goodbye\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@3 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1
@4 = private unnamed_addr constant [18 x i8] c"All tests passed!\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @main() !dbg !4 {
entry:
  %robot = alloca %Robot, align 8, !dbg !13
  %person = alloca %Person, align 8, !dbg !13
  %Person_obj = alloca %Person, align 8, !dbg !13
  %name_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 0, !dbg !13
  store { ptr, i64 } { ptr @3, i64 5 }, ptr %name_ptr, align 8, !dbg !13
  %Person_val = load %Person, ptr %Person_obj, align 8, !dbg !13
  store %Person %Person_val, ptr %person, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %person, metadata !9, metadata !DIExpression()), !dbg !14
  %Robot_obj = alloca %Robot, align 8, !dbg !13
  %id_ptr = getelementptr inbounds %Robot, ptr %Robot_obj, i32 0, i32 0, !dbg !13
  store i64 42, ptr %id_ptr, align 4, !dbg !13
  %Robot_val = load %Robot, ptr %Robot_obj, align 4, !dbg !13
  store %Robot %Robot_val, ptr %robot, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %robot, metadata !11, metadata !DIExpression()), !dbg !15
  %person.load = load %Person, ptr %person, align 8, !dbg !13
  call void @Person_goodbye(%Person %person.load), !dbg !13
  %robot.load = load %Robot, ptr %robot, align 4, !dbg !13
  call void @Robot_hello(%Robot %robot.load), !dbg !13
  %robot.load1 = load %Robot, ptr %robot, align 4, !dbg !13
  call void @Robot_goodbye(%Robot %robot.load1), !dbg !13
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @4, i64 17 }, ptr %serialize_temp, align 8, !dbg !13
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.3), !dbg !13
  call void @__vyn_println(ptr %serialized_json), !dbg !13
  ret i64 0, !dbg !13
}

define void @Person_goodbye(%Person %self) !dbg !16 {
entry:
  %self1 = alloca %Person, align 8
  store %Person %self, ptr %self1, align 8, !dbg !21
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !20, metadata !DIExpression()), !dbg !22
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !21
  store { ptr, i64 } { ptr @0, i64 19 }, ptr %serialize_temp, align 8, !dbg !21
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !21
  call void @__vyn_println(ptr %serialized_json), !dbg !21
  ret void, !dbg !21
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

define void @Robot_hello(%Robot %self) !dbg !23 {
entry:
  %self1 = alloca %Robot, align 8
  store %Robot %self, ptr %self1, align 4, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !27, metadata !DIExpression()), !dbg !29
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !28
  store { ptr, i64 } { ptr @1, i64 16 }, ptr %serialize_temp, align 8, !dbg !28
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.1), !dbg !28
  call void @__vyn_println(ptr %serialized_json), !dbg !28
  ret void, !dbg !28
}

define void @Robot_goodbye(%Robot %self) !dbg !30 {
entry:
  %self1 = alloca %Robot, align 8
  store %Robot %self, ptr %self1, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !32, metadata !DIExpression()), !dbg !34
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !33
  store { ptr, i64 } { ptr @2, i64 18 }, ptr %serialize_temp, align 8, !dbg !33
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.2), !dbg !33
  call void @__vyn_println(ptr %serialized_json), !dbg !33
  ret void, !dbg !33
}

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_aspect_default_methods.vyn.ll", directory: "/home/rick/Projects/Vyn/test/aspect")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 38, type: !5, scopeLine: 38, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "person", scope: !4, file: !1, line: 39, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "Person", scope: !1, file: !1, size: 64, align: 8)
!11 = !DILocalVariable(name: "robot", scope: !4, file: !1, line: 40, type: !12)
!12 = !DICompositeType(tag: DW_TAG_structure_type, name: "Robot", scope: !1, file: !1, size: 64, align: 8)
!13 = !DILocation(line: 38, column: 1, scope: !4)
!14 = !DILocation(line: 39, column: 1, scope: !4)
!15 = !DILocation(line: 40, column: 1, scope: !4)
!16 = distinct !DISubprogram(name: "Person_goodbye", linkageName: "Person_goodbye", scope: !1, file: !1, line: 22, type: !17, scopeLine: 22, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !19)
!17 = !DISubroutineType(types: !18)
!18 = !{null, !10}
!19 = !{!20}
!20 = !DILocalVariable(name: "self", scope: !16, file: !1, line: 22, type: !10)
!21 = !DILocation(line: 22, column: 1, scope: !16)
!22 = !DILocation(line: 22, column: 17, scope: !16)
!23 = distinct !DISubprogram(name: "Robot_hello", linkageName: "Robot_hello", scope: !1, file: !1, line: 29, type: !24, scopeLine: 29, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !26)
!24 = !DISubroutineType(types: !25)
!25 = !{null, !12}
!26 = !{!27}
!27 = !DILocalVariable(name: "self", scope: !23, file: !1, line: 29, type: !12)
!28 = !DILocation(line: 29, column: 1, scope: !23)
!29 = !DILocation(line: 29, column: 15, scope: !23)
!30 = distinct !DISubprogram(name: "Robot_goodbye", linkageName: "Robot_goodbye", scope: !1, file: !1, line: 33, type: !24, scopeLine: 33, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !31)
!31 = !{!32}
!32 = !DILocalVariable(name: "self", scope: !30, file: !1, line: 33, type: !12)
!33 = !DILocation(line: 33, column: 5, scope: !30)
!34 = !DILocation(line: 33, column: 17, scope: !30)
