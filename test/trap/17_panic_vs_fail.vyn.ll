; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [47 x i8] c"INVARIANT VIOLATED: Value cannot be negative: \00", align 1
@1 = private unnamed_addr constant [25 x i8] c"=== Recoverable Fail ===\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [9 x i8] c"Result: \00", align 1
@3 = private unnamed_addr constant [30 x i8] c"\\n=== Unrecoverable Panic ===\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@4 = private unnamed_addr constant [22 x i8] c"This will never print\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define void @check_invariant(i64 %value) !dbg !4 {
entry:
  %value1 = alloca i64, align 8
  store i64 %value, ptr %value1, align 4, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %value1, metadata !9, metadata !DIExpression()), !dbg !11
  %value2 = load i64, ptr %value1, align 4, !dbg !10
  %icmpslttmp = icmp slt i64 %value2, 0, !dbg !10
  br i1 %icmpslttmp, label %then, label %ifcont, !dbg !10

then:                                             ; preds = %entry
  %value3 = load i64, ptr %value1, align 4, !dbg !10
  %tostring = call ptr @__vyn_toString_int(i64 %value3), !dbg !10
  %strcattmp = call ptr @__vyn_string_concat(ptr @0, ptr %tostring), !dbg !10
  br label %ifcont, !dbg !10

ifcont:                                           ; preds = %then, %entry
  ret void, !dbg !10
}

define i64 @divide(i64 %a, i64 %b) !dbg !12 {
entry:
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %a1, metadata !16, metadata !DIExpression()), !dbg !19
  store i64 %b, ptr %b2, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %b2, metadata !17, metadata !DIExpression()), !dbg !20
  %b3 = load i64, ptr %b2, align 4, !dbg !18
  %icmpeqtmp = icmp eq i64 %b3, 0, !dbg !18
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !18

then:                                             ; preds = %entry
  br label %ifcont, !dbg !18

ifcont:                                           ; preds = %then, %entry
  %a4 = load i64, ptr %a1, align 4, !dbg !18
  %b5 = load i64, ptr %b2, align 4, !dbg !18
  %sdivtmp = sdiv i64 %a4, %b5, !dbg !18
  ret i64 %sdivtmp, !dbg !18
}

define i64 @main() !dbg !21 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !24
  store { ptr, i64 } { ptr @1, i64 24 }, ptr %serialize_temp, align 8, !dbg !24
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !24
  call void @__vyn_println(ptr %serialized_json), !dbg !24
  %calltmp = call i64 @divide(i64 10, i64 0), !dbg !24
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !24
  store { ptr, i64 } { ptr @3, i64 29 }, ptr %serialize_temp1, align 8, !dbg !24
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.1), !dbg !24
  call void @__vyn_println(ptr %serialized_json2), !dbg !24
  call void @check_invariant(i64 -5), !dbg !24
  %serialize_temp3 = alloca { ptr, i64 }, align 8, !dbg !24
  store { ptr, i64 } { ptr @4, i64 21 }, ptr %serialize_temp3, align 8, !dbg !24
  %serialized_json4 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp3, ptr @type_name.2), !dbg !24
  call void @__vyn_println(ptr %serialized_json4), !dbg !24
  ret i64 0, !dbg !24
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "17_panic_vs_fail.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "check_invariant", linkageName: "check_invariant", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{null, !7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "value", scope: !4, file: !1, line: 4, type: !7)
!10 = !DILocation(line: 4, column: 1, scope: !4)
!11 = !DILocation(line: 4, column: 22, scope: !4)
!12 = distinct !DISubprogram(name: "divide", linkageName: "divide", scope: !1, file: !1, line: 11, type: !13, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!13 = !DISubroutineType(types: !14)
!14 = !{!7, !7, !7}
!15 = !{!16, !17}
!16 = !DILocalVariable(name: "a", scope: !12, file: !1, line: 11, type: !7)
!17 = !DILocalVariable(name: "b", scope: !12, file: !1, line: 11, type: !7)
!18 = !DILocation(line: 11, column: 1, scope: !12)
!19 = !DILocation(line: 11, column: 9, scope: !12)
!20 = !DILocation(line: 11, column: 17, scope: !12)
!21 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 19, type: !22, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!22 = !DISubroutineType(types: !23)
!23 = !{!7}
!24 = !DILocation(line: 19, column: 1, scope: !21)
