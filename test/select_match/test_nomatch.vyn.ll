; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [16 x i8] c"Hello from test\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [16 x i8] c"Hello from main\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [5 x i8] c"Done\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define void @test() !dbg !4 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !7
  store { ptr, i64 } { ptr @0, i64 15 }, ptr %serialize_temp, align 8, !dbg !7
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !7
  call void @__vyn_println(ptr %serialized_json), !dbg !7
  ret void, !dbg !7
}

define void @main() !dbg !8 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !9
  store { ptr, i64 } { ptr @1, i64 15 }, ptr %serialize_temp, align 8, !dbg !9
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.1), !dbg !9
  call void @__vyn_println(ptr %serialized_json), !dbg !9
  call void @test(), !dbg !9
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !9
  store { ptr, i64 } { ptr @2, i64 4 }, ptr %serialize_temp1, align 8, !dbg !9
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.2), !dbg !9
  call void @__vyn_println(ptr %serialized_json2), !dbg !9
  ret void, !dbg !9
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_nomatch.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test", linkageName: "test", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !DILocation(line: 3, column: 1, scope: !4)
!8 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 7, type: !5, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!9 = !DILocation(line: 7, column: 1, scope: !8)
