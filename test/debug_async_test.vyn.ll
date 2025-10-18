; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [17 x i8] c"Fetching data...\00", align 1
@1 = private unnamed_addr constant [25 x i8] c"=== Debug Async Test ===\00", align 1

define ptr @fetch_data() !dbg !4 {
entry:
  %result = alloca i64, align 8
  store i64 10, ptr %result, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %result, metadata !9, metadata !DIExpression()), !dbg !12
  call void @__vyn_println(ptr @0), !dbg !11
  %result1 = load i64, ptr %result, align 4, !dbg !11
  %inttoptr_cast = inttoptr i64 %result1 to ptr, !dbg !11
  ret ptr %inttoptr_cast, !dbg !11
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @__vyn_println(ptr)

define ptr @process_data() !dbg !13 {
entry:
  %combined = alloca i64, align 8, !dbg !18
  %data2 = alloca i64, align 8, !dbg !18
  %data1 = alloca i64, align 8, !dbg !18
  %calltmp = call ptr @fetch_data(), !dbg !18
  call void @vyn_await_task(i64 0), !dbg !18
  br label %await_continuation_1, !dbg !18

await_continuation_1:                             ; preds = %entry
  %ptrtoint_cast = ptrtoint ptr %calltmp to i64, !dbg !18
  store i64 %ptrtoint_cast, ptr %data1, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %data1, metadata !15, metadata !DIExpression()), !dbg !19
  %calltmp1 = call ptr @fetch_data(), !dbg !20
  call void @vyn_await_task(i64 0), !dbg !20
  br label %await_continuation_2, !dbg !20

await_continuation_2:                             ; preds = %await_continuation_1
  %ptrtoint_cast2 = ptrtoint ptr %calltmp1 to i64, !dbg !20
  store i64 %ptrtoint_cast2, ptr %data2, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %data2, metadata !16, metadata !DIExpression()), !dbg !21
  %data13 = load i64, ptr %data1, align 4, !dbg !20
  %data24 = load i64, ptr %data2, align 4, !dbg !20
  %addtmp = add i64 %data13, %data24, !dbg !20
  store i64 %addtmp, ptr %combined, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %combined, metadata !17, metadata !DIExpression()), !dbg !22
  %combined5 = load i64, ptr %combined, align 4, !dbg !20
  %inttoptr_cast = inttoptr i64 %combined5 to ptr, !dbg !20
  ret ptr %inttoptr_cast, !dbg !20
}

define void @vyn_await_task(i64 %0) {
entry:
  ret void
}

define i64 @main() !dbg !23 {
entry:
  %result = alloca ptr, align 8, !dbg !28
  call void @__vyn_println(ptr @1), !dbg !28
  %calltmp = call ptr @process_data(), !dbg !28
  store ptr %calltmp, ptr %result, align 8, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %result, metadata !27, metadata !DIExpression()), !dbg !29
  ret i64 0, !dbg !28
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "debug_async_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "async fetch_data", linkageName: "fetch_data", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!8 = !{!9}
!9 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 5, type: !10)
!10 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!11 = !DILocation(line: 4, column: 1, scope: !4)
!12 = !DILocation(line: 5, column: 5, scope: !4)
!13 = distinct !DISubprogram(name: "async process_data", linkageName: "process_data", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!14 = !{!15, !16, !17}
!15 = !DILocalVariable(name: "data1", scope: !13, file: !1, line: 12, type: !10)
!16 = !DILocalVariable(name: "data2", scope: !13, file: !1, line: 13, type: !10)
!17 = !DILocalVariable(name: "combined", scope: !13, file: !1, line: 15, type: !10)
!18 = !DILocation(line: 12, column: 18, scope: !13)
!19 = !DILocation(line: 12, column: 1, scope: !13)
!20 = !DILocation(line: 13, column: 18, scope: !13)
!21 = !DILocation(line: 13, column: 1, scope: !13)
!22 = !DILocation(line: 15, column: 1, scope: !13)
!23 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 19, type: !24, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !26)
!24 = !DISubroutineType(types: !25)
!25 = !{!10}
!26 = !{!27}
!27 = !DILocalVariable(name: "result", scope: !23, file: !1, line: 21, type: !7)
!28 = !DILocation(line: 19, column: 1, scope: !23)
!29 = !DILocation(line: 21, column: 1, scope: !23)
