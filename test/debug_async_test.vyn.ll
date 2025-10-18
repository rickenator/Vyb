; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [17 x i8] c"Fetching data...\00", align 1
@1 = private unnamed_addr constant [25 x i8] c"=== Debug Async Test ===\00", align 1

define ptr @fetch_data() !dbg !4 {
entry:
  %result = alloca i64, align 8
  store i64 10, ptr %result, align 4, !dbg !8
  call void @__vyn_println(ptr @0), !dbg !8
  %result1 = load i64, ptr %result, align 4, !dbg !8
  %inttoptr_cast = inttoptr i64 %result1 to ptr, !dbg !8
  ret ptr %inttoptr_cast, !dbg !8
}

declare void @__vyn_println(ptr)

define ptr @process_data() !dbg !9 {
entry:
  %combined = alloca i64, align 8, !dbg !10
  %data2 = alloca i64, align 8, !dbg !10
  %data1 = alloca i64, align 8, !dbg !10
  %calltmp = call ptr @fetch_data(), !dbg !10
  call void @vyn_await_task(i64 0), !dbg !10
  br label %await_continuation_1, !dbg !10

await_continuation_1:                             ; preds = %entry
  %ptrtoint_cast = ptrtoint ptr %calltmp to i64, !dbg !10
  store i64 %ptrtoint_cast, ptr %data1, align 4, !dbg !10
  %calltmp1 = call ptr @fetch_data(), !dbg !11
  call void @vyn_await_task(i64 0), !dbg !11
  br label %await_continuation_2, !dbg !11

await_continuation_2:                             ; preds = %await_continuation_1
  %ptrtoint_cast2 = ptrtoint ptr %calltmp1 to i64, !dbg !11
  store i64 %ptrtoint_cast2, ptr %data2, align 4, !dbg !11
  %data13 = load i64, ptr %data1, align 4, !dbg !11
  %data24 = load i64, ptr %data2, align 4, !dbg !11
  %addtmp = add i64 %data13, %data24, !dbg !11
  store i64 %addtmp, ptr %combined, align 4, !dbg !11
  %combined5 = load i64, ptr %combined, align 4, !dbg !11
  %inttoptr_cast = inttoptr i64 %combined5 to ptr, !dbg !11
  ret ptr %inttoptr_cast, !dbg !11
}

define void @vyn_await_task(i64 %0) {
entry:
  ret void
}

define i64 @main() !dbg !12 {
entry:
  %result = alloca ptr, align 8, !dbg !16
  call void @__vyn_println(ptr @1), !dbg !16
  %calltmp = call ptr @process_data(), !dbg !16
  store ptr %calltmp, ptr %result, align 8, !dbg !16
  ret i64 0, !dbg !16
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "debug_async_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "async fetch_data", linkageName: "fetch_data", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!8 = !DILocation(line: 4, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "async process_data", linkageName: "process_data", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DILocation(line: 12, column: 18, scope: !9)
!11 = !DILocation(line: 13, column: 18, scope: !9)
!12 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 19, type: !13, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!13 = !DISubroutineType(types: !14)
!14 = !{!15}
!15 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!16 = !DILocation(line: 19, column: 1, scope: !12)
