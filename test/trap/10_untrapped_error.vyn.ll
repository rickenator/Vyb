; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [22 x i8] c"This should not print\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define { i64, ptr } @risky_operation() !dbg !4 {
entry:
  ret { i64, ptr } zeroinitializer, !dbg !8
}

define i64 @main() !dbg !9 {
entry:
  %calltmp = call { i64, ptr } @risky_operation(), !dbg !13
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !13
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !13
  %has.error = icmp ne ptr %call.error, null, !dbg !13
  br i1 %has.error, label %call.error1, label %call.success, !dbg !13

call.error1:                                      ; preds = %entry
  call void @__vyn_runtime_untrapped_error(ptr null), !dbg !13
  unreachable, !dbg !13

call.success:                                     ; preds = %entry
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @0, i64 21 }, ptr %serialize_temp, align 8, !dbg !13
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !13
  call void @__vyn_println(ptr %serialized_json), !dbg !13
  ret i64 0, !dbg !13
}

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #0

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "10_untrapped_error.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "risky_operation", linkageName: "risky_operation", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 4, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 9, type: !10, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!13 = !DILocation(line: 9, column: 1, scope: !9)
