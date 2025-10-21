; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [14 x i8] c"Starting main\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [9 x i8] c"In block\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [16 x i8] c"In trap handler\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@3 = private unnamed_addr constant [12 x i8] c"After block\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @main() !dbg !4 {
entry:
  %trap_error = alloca ptr, align 8, !dbg !8
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @0, i64 13 }, ptr %serialize_temp, align 8, !dbg !8
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !8
  call void @__vyn_println(ptr %serialized_json), !dbg !8
  br label %block.normal, !dbg !8

block.normal:                                     ; preds = %entry
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @1, i64 8 }, ptr %serialize_temp1, align 8, !dbg !8
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.1), !dbg !8
  call void @__vyn_println(ptr %serialized_json2), !dbg !8
  store i64 42, ptr %trap_error, align 4, !dbg !8
  br label %trap.landing, !dbg !8

block.continue:                                   ; No predecessors!
  %serialize_temp5 = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @3, i64 11 }, ptr %serialize_temp5, align 8, !dbg !8
  %serialized_json6 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp5, ptr @type_name.3), !dbg !8
  call void @__vyn_println(ptr %serialized_json6), !dbg !8
  ret i64 undef, !dbg !8

trap.landing:                                     ; preds = %block.normal
  %caught_error = load ptr, ptr %trap_error, align 8, !dbg !8
  %serialize_temp3 = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @2, i64 15 }, ptr %serialize_temp3, align 8, !dbg !8
  %serialized_json4 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp3, ptr @type_name.2), !dbg !8
  call void @__vyn_println(ptr %serialized_json4), !dbg !8
  ret i64 -1, !dbg !8
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "debug_trap.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 3, column: 1, scope: !4)
