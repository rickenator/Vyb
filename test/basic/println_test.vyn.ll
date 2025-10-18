; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [18 x i8] c"Hello, Vyn World!\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [13 x i8] c"The sum is: \00", align 1

define i64 @main() !dbg !4 {
entry:
  call void @__vyn_println(ptr @0), !dbg !8
  %serialize_temp = alloca i64, align 8, !dbg !8
  store i64 3, ptr %serialize_temp, align 4, !dbg !8
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !8
  call void @__vyn_println(ptr %serialized_json), !dbg !8
  %tostring = call ptr @__vyn_toString_int(i64 12), !dbg !8
  %strcattmp = call ptr @__vyn_string_concat(ptr @1, ptr %tostring), !dbg !8
  call void @__vyn_println(ptr %strcattmp), !dbg !8
  ret i64 0, !dbg !8
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "println_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/basic")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 2, column: 1, scope: !4)
