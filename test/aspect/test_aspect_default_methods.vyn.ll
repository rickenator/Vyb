; ModuleID = 'VynModule'
source_filename = "VynModule"

%Robot = type { i64 }
%Person = type { { ptr, i64 } }

@0 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1
@1 = private unnamed_addr constant [18 x i8] c"All tests passed!\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @main() !dbg !4 {
entry:
  %robot = alloca %Robot, align 8, !dbg !13
  %person = alloca %Person, align 8, !dbg !13
  %Person_obj = alloca %Person, align 8, !dbg !13
  %name_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 0, !dbg !13
  store { ptr, i64 } { ptr @0, i64 5 }, ptr %name_ptr, align 8, !dbg !13
  %Person_val = load %Person, ptr %Person_obj, align 8, !dbg !13
  store %Person %Person_val, ptr %person, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %person, metadata !9, metadata !DIExpression()), !dbg !14
  %Robot_obj = alloca %Robot, align 8, !dbg !13
  %id_ptr = getelementptr inbounds %Robot, ptr %Robot_obj, i32 0, i32 0, !dbg !13
  store i64 42, ptr %id_ptr, align 4, !dbg !13
  %Robot_val = load %Robot, ptr %Robot_obj, align 4, !dbg !13
  store %Robot %Robot_val, ptr %robot, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %robot, metadata !11, metadata !DIExpression()), !dbg !15
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @1, i64 17 }, ptr %serialize_temp, align 8, !dbg !13
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !13
  call void @__vyn_println(ptr %serialized_json), !dbg !13
  ret i64 0, !dbg !13
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

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
